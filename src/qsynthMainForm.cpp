// qsynthMainForm.cpp
//
/****************************************************************************
   Copyright (C) 2003-2011, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qsynthAbout.h"
#include "qsynthMainForm.h"

#include "qsynthEngine.h"

#include "qsynthTabBar.h"
#include "qsynthSystemTray.h"

#include "qsynthAboutForm.h"
#include "qsynthSetupForm.h"
#include "qsynthOptionsForm.h"
#include "qsynthMessagesForm.h"
#include "qsynthChannelsForm.h"

#include "qsynthDialClassicStyle.h"
#include "qsynthDialVokiStyle.h"
#include "qsynthDialPeppinoStyle.h"
#include "qsynthDialSkulptureStyle.h"

#include <QApplication>
#include <QSocketNotifier>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>
#include <QRegExp>
#include <QTimer>
#include <QMenu>
#include <QUrl>

#include <QCloseEvent>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDropEvent>

#if QT_VERSION < 0x040500
namespace Qt {
const WindowFlags WindowCloseButtonHint = WindowFlags(0x08000000);
#if QT_VERSION < 0x040200
const WindowFlags CustomizeWindowHint   = WindowFlags(0x02000000);
#endif
}
#endif


// Timer constant stuff.
#define QSYNTH_TIMER_MSECS  100
#define QSYNTH_DELAY_MSECS  300

// Scale factors.
#define QSYNTH_MASTER_GAIN_SCALE    100.0f

#define QSYNTH_REVERB_ROOM_SCALE    100.0f
#define QSYNTH_REVERB_DAMP_SCALE	100.0f
#define QSYNTH_REVERB_WIDTH_SCALE   1.0f
#define QSYNTH_REVERB_LEVEL_SCALE   100.0f

#define QSYNTH_CHORUS_NR_SCALE      1.0f
#define QSYNTH_CHORUS_LEVEL_SCALE   100.0f
#define QSYNTH_CHORUS_SPEED_SCALE   100.0f
#define QSYNTH_CHORUS_DEPTH_SCALE   10.0f

#if defined(WIN32)
#undef HAVE_SIGNAL_H
#else
#include <unistd.h>
// Notification pipe descriptors
#define QSYNTH_FDNIL     -1
#define QSYNTH_FDREAD     0
#define QSYNTH_FDWRITE    1
static int g_fdStdout[2] = { QSYNTH_FDNIL, QSYNTH_FDNIL };
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

// Needed for lroundf()
#ifdef CONFIG_ROUND
#include <math.h>
#else
static inline long lroundf ( float x )
{
	if (x >= 0.0f)
		return long(x + 0.5f);
	else
		return long(x - 0.5f);
}
#endif


// The current selected engine.
static qsynthEngine *g_pCurrentEngine = NULL;


#ifdef CONFIG_FLUID_SERVER

// Hold last shell/server port in use.
static int g_iLastShellPort = 0;

// Needed for server mode.
static fluid_cmd_handler_t* qsynth_newclient ( void* data, char* )
{
	return ::new_fluid_cmd_handler((fluid_synth_t*) data);
}

#endif


//-------------------------------------------------------------------------
// Audio driver processing stub.

int qsynth_process ( void *pvData, int len,
	int nin, float **in, int nout, float **out )
{
	qsynthEngine *pEngine = (qsynthEngine *) pvData;
	// Call the synthesizer process function to fill
	// the output buffers with its audio output.
	if (::fluid_synth_process(pEngine->pSynth, len, nin, in, nout, out) != 0)
		return -1;
	// Now find the peak level for this buffer run...
	if (pEngine == g_pCurrentEngine) {
		for (int i = 0; i < nout; i++) {
			float *out_i = out[i];
			for (int j = 0; j < len; j++) {
				float fValue = out_i[j];
				if (pEngine->fMeterValue[i & 1] < fValue)
					pEngine->fMeterValue[i & 1] = fValue;
			}
		}
	}
	// Surely a success :)
	return 0;
}

//-------------------------------------------------------------------------
// Midi router stubs to have some midi activity feedback.

#define QSYNTH_MIDI_NOTE_OFF            0x80
#define QSYNTH_MIDI_NOTE_ON             0x90
#define QSYNTH_MIDI_CONTROL_CHANGE      0xb0
#define QSYNTH_MIDI_PROGRAM_CHANGE      0xc0

#define QSYNTH_MIDI_CC_BANK_SELECT_MSB  0x00
#define QSYNTH_MIDI_CC_BANK_SELECT_LSB  0x20
#define QSYNTH_MIDI_CC_ALL_SOUND_OFF    0x78

struct qsynth_midi_channel
{
	int iEvent;     // Event occurrence accumulator.
	int iState;     // Activity state tracker.
	int iChange;    // Change activity accumulator.
};

static int                  g_iMidiChannels  = 0;
static qsynth_midi_channel *g_pMidiChannels  = NULL;

static void qsynth_midi_event ( qsynthEngine *pEngine,
	fluid_midi_event_t *pMidiEvent )
{
	pEngine->iMidiEvent++;

	if (g_pMidiChannels && pEngine == g_pCurrentEngine) {
		int iChan = ::fluid_midi_event_get_channel(pMidiEvent);
#ifdef CONFIG_DEBUG
		int iType = ::fluid_midi_event_get_type(pMidiEvent);
		int iKey  = ::fluid_midi_event_get_control(pMidiEvent);
		int iVal  = ::fluid_midi_event_get_value(pMidiEvent);
		fprintf(stderr, "Type=%03d (0x%02x) Chan=%02d Key=%03d (0x%02x) Val=%03d (0x%02x).\n",
			iType, iType, iChan, iKey, iKey, iVal, iVal);
#endif
		if (iChan >= 0 && iChan < g_iMidiChannels) {
			int iCC;
			switch (::fluid_midi_event_get_type(pMidiEvent)) {
			case QSYNTH_MIDI_CONTROL_CHANGE:
				// Avoid bank selects or global control changes...
				iCC = ::fluid_midi_event_get_control(pMidiEvent);
				if (iCC == QSYNTH_MIDI_CC_BANK_SELECT_MSB ||
					iCC == QSYNTH_MIDI_CC_BANK_SELECT_LSB ||
					iCC >= QSYNTH_MIDI_CC_ALL_SOUND_OFF)
					break;
				// Fall thru...
			case QSYNTH_MIDI_PROGRAM_CHANGE:
				g_pMidiChannels[iChan].iChange++;
				// Fall thru, again...
			case QSYNTH_MIDI_NOTE_ON:
			case QSYNTH_MIDI_NOTE_OFF:
				g_pMidiChannels[iChan].iEvent++;
				break;
			}
		}
	}
}

static int qsynth_dump_postrouter ( void *pvData,
	fluid_midi_event_t *pMidiEvent )
{
	qsynthEngine *pEngine = (qsynthEngine *) pvData;
	qsynth_midi_event(pEngine, pMidiEvent);
	return ::fluid_midi_dump_postrouter(pEngine->pSynth, pMidiEvent);
}

static int qsynth_handle_midi_event ( void *pvData,
	fluid_midi_event_t *pMidiEvent )
{
	qsynthEngine *pEngine = (qsynthEngine *) pvData;
	qsynth_midi_event(pEngine, pMidiEvent);
	return ::fluid_synth_handle_midi_event(pEngine->pSynth, pMidiEvent);
}


//-------------------------------------------------------------------------
// Scaling & Clipping helpers.

static int qsynth_set_range_value ( QDial *pDial, float fScale, float fValue )
{
	int iValue = (int) ::lroundf(fScale * fValue);

	if (iValue < pDial->minimum())
		iValue = pDial->minimum();
	else
	if (iValue > pDial->maximum())
		iValue = pDial->maximum();

	pDial->setValue(iValue);

	return iValue;
}

static float qsynth_get_range_value ( QSpinBox *pSpinBox, float fScale )
{
	float fValue = float(pSpinBox->value()) / fScale;

	float fMinimum = float(pSpinBox->minimum()) / fScale;
	float fMaximum = float(pSpinBox->maximum()) / fScale;

	if (fValue < fMinimum)
		fValue = fMinimum;
	else
	if (fValue > fMaximum)
		fValue = fMaximum;

	return fValue;
}


#ifdef QSYNTH_CUSTOM_LOADER
//-------------------------------------------------------------------------
// (EXPERIMENTAL) Soundfont loader: feature to avoid loading
// duplicate soundfonts for multiple engines.

static
struct qsynthEngineNode
{
	qsynthEngine     *pEngine;
	qsynthEngineNode *pPrev;
	qsynthEngineNode *pNext;

} *g_pEngineList = NULL;


static int qsynth_sfont_free ( fluid_sfont_t *pSoundFont )
{
#ifdef CONFIG_DEBUG
	fprintf(stderr, "qsynth_sfont_free(%p)\n", pSoundFont);
#endif
	if (pSoundFont)	::free(pSoundFont);
	return 0;
}

static int qsynth_sfloader_free ( fluid_sfloader_t * pLoader )
{
#ifdef CONFIG_DEBUG
	fprintf(stderr, "qsynth_sfloader_free(%p)\n", pLoader);
#endif
	if (pLoader) ::free(pLoader);
	return 0;
}


static fluid_sfont_t *qsynth_sfloader_load (
	fluid_sfloader_t *pLoader, const char *pszFilename )
{
#ifdef CONFIG_DEBUG
	fprintf(stderr, "qsynth_sfloader_load(%p, \"%s\")\n", pLoader, pszFilename);
#endif

	if (pLoader == NULL)
		return NULL;

	// Look thru all the synths' sfonts for the requested one...
	qsynthEngineNode *pNode = g_pEngineList;
	while (pNode) {
		fluid_synth_t *pSynth = (pNode->pEngine)->pSynth;
		int cSoundFonts = ::fluid_synth_sfcount(pSynth);
		for (int i = 0; i < cSoundFonts; i++) {
			fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(pSynth, i);
			// Somehow get the name of this sfont...
			char *pszName = pSoundFont->get_name(pSoundFont);
			// Create a dup sfont node with our 'free' routine,
			// when we have a match
			if (::strcmp(pszName, pszFilename) == 0) {
				fluid_sfont_t *pNewSoundFont
					= (fluid_sfont_t *) ::malloc(sizeof(fluid_sfont_t));
				::memcpy(pNewSoundFont, pSoundFont, sizeof(fluid_sfont_t));
				pNewSoundFont->free = qsynth_sfont_free;
				return pNewSoundFont;
			}
		}
		pNode = pNode->pNext;
	}

	// fluidsynth will call next (or default) loader...
	return NULL;
}

#endif	// QSYNTH_CUSTOM_LOADER


//----------------------------------------------------------------------------
// qsynthMainForm -- UI wrapper form.

// Kind of singleton reference.
qsynthMainForm *qsynthMainForm::g_pMainForm = NULL;

// Constructor.
qsynthMainForm::qsynthMainForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QWidget(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Pseudo-singleton reference setup.
	g_pMainForm = this;

	m_pOptions = NULL;

	m_iTimerDelay = 0;

	m_iCurrentTab = -1;

	m_pStdoutNotifier = NULL;

	m_iGainChanged   = 0;
	m_iReverbChanged = 0;
	m_iChorusChanged = 0;

	m_iGainUpdated   = 0;
	m_iReverbUpdated = 0;
	m_iChorusUpdated = 0;

	// All forms are to be created later on setup.
	m_pMessagesForm  = NULL;
	m_pChannelsForm  = NULL;

	// The eventual system tray widget.
	m_pSystemTray = NULL;
	m_iSystemTrayState = 0;

	// We're not quitting so early :)
	m_bQuitForce = false;

	// Whether we've Qt::Tool flag (from bKeepOnTop),
	// this is actually the main last application window...
	QWidget::setAttribute(Qt::WA_QuitOnClose);

#ifdef HAVE_SIGNAL_H
	// Set to ignore any fatal "Broken pipe" signals.
	::signal(SIGPIPE, SIG_IGN);
#endif

#if QT_VERSION >= 0x040200
	m_ui.GainSpinBox->setAccelerated(true);
	m_ui.ReverbRoomSpinBox->setAccelerated(true);
	m_ui.ReverbDampSpinBox->setAccelerated(true);
	m_ui.ReverbWidthSpinBox->setAccelerated(true);
	m_ui.ReverbLevelSpinBox->setAccelerated(true);
	m_ui.ChorusNrSpinBox->setAccelerated(true);
	m_ui.ChorusLevelSpinBox->setAccelerated(true);
	m_ui.ChorusSpeedSpinBox->setAccelerated(true);
	m_ui.ChorusDepthSpinBox->setAccelerated(true);
#endif

	// UI connections...
	QObject::connect(m_ui.SetupPushButton,
		SIGNAL(clicked()),
		SLOT(showSetupForm()));
	QObject::connect(m_ui.GainDial,
		SIGNAL(valueChanged(int)),
		m_ui.GainSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ReverbActiveCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(reverbActivate(bool)));
	QObject::connect(m_ui.ReverbRoomDial,
		SIGNAL(valueChanged(int)),
		m_ui.ReverbRoomSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ReverbDampDial,
		SIGNAL(valueChanged(int)),
		m_ui.ReverbDampSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ReverbWidthDial,
		SIGNAL(valueChanged(int)),
		m_ui.ReverbWidthSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ReverbLevelDial,
		SIGNAL(valueChanged(int)),
		m_ui.ReverbLevelSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ChorusActiveCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(chorusActivate(bool)));
	QObject::connect(m_ui.ChorusNrDial,
		SIGNAL(valueChanged(int)),
		m_ui.ChorusNrSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ChorusLevelDial,
		SIGNAL(valueChanged(int)),
		m_ui.ChorusLevelSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ChorusSpeedDial,
		SIGNAL(valueChanged(int)),
		m_ui.ChorusSpeedSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ChorusDepthDial,
		SIGNAL(valueChanged(int)),
		m_ui.ChorusDepthSpinBox,
		SLOT(setValue(int)));
	QObject::connect(m_ui.ChorusTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(chorusChanged(int)));
	QObject::connect(m_ui.GainSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(gainChanged(int)));
	QObject::connect(m_ui.ReverbRoomSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(reverbChanged(int)));
	QObject::connect(m_ui.ReverbDampSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(reverbChanged(int)));
	QObject::connect(m_ui.ReverbWidthSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(reverbChanged(int)));
	QObject::connect(m_ui.ReverbLevelSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(reverbChanged(int)));
	QObject::connect(m_ui.ChorusNrSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(chorusChanged(int)));
	QObject::connect(m_ui.ChorusLevelSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(chorusChanged(int)));
	QObject::connect(m_ui.ChorusSpeedSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(chorusChanged(int)));
	QObject::connect(m_ui.ChorusDepthSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(chorusChanged(int)));
	QObject::connect(m_ui.ProgramResetPushButton,
		SIGNAL(clicked()),
		SLOT(programReset()));
	QObject::connect(m_ui.SystemResetPushButton,
		SIGNAL(clicked()),
		SLOT(systemReset()));
	QObject::connect(m_ui.RestartPushButton,
		SIGNAL(clicked()),
		SLOT(promptRestart()));
	QObject::connect(m_ui.ChannelsPushButton,
		SIGNAL(clicked()),
		SLOT(toggleChannelsForm()));
	QObject::connect(m_ui.QuitPushButton,
		SIGNAL(clicked()),
		SLOT(quitMainForm()));
	QObject::connect(m_ui.OptionsPushButton,
		SIGNAL(clicked()),
		SLOT(showOptionsForm()));
	QObject::connect(m_ui.MessagesPushButton,
		SIGNAL(clicked()),
		SLOT(toggleMessagesForm()));
	QObject::connect(m_ui.AboutPushButton,
		SIGNAL(clicked()),
		SLOT(showAboutForm()));
	QObject::connect(m_ui.NewEngineToolButton,
		SIGNAL(clicked()),
		SLOT(newEngine()));
	QObject::connect(m_ui.DeleteEngineToolButton,
		SIGNAL(clicked()),
		SLOT(deleteEngine()));

	m_pKnobStyle = NULL;
}


// Destructor.
qsynthMainForm::~qsynthMainForm (void)
{
	// Stop the press!
	for (int iTab = 0; iTab < m_ui.TabBar->count(); iTab++) {
		qsynthEngine *pEngine = m_ui.TabBar->engine(iTab);
		if (pEngine)
			stopEngine(pEngine);
	}

	// No more options descriptor.
	m_pOptions = NULL;

	// Finally drop any popup widgets around...
	if (m_pMessagesForm)
		delete m_pMessagesForm;
	if (m_pChannelsForm)
		delete m_pChannelsForm;

	// Quit off system tray widget.
	if (m_pSystemTray)
		delete m_pSystemTray;

	// Pseudo-singleton reference shut-down.
	g_pMainForm = NULL;

	if (m_pKnobStyle)
		delete m_pKnobStyle;
}


// Kind of singleton reference.
qsynthMainForm *qsynthMainForm::getInstance (void)
{
	return g_pMainForm;
}


// Make and set a proper setup step.
void qsynthMainForm::setup ( qsynthOptions *pOptions )
{
	// Finally, fix settings descriptor
	// and stabilize the form.
	m_pOptions = pOptions;

	// What style do we create these forms?
	QWidget *pParent = NULL;
	Qt::WindowFlags wflags = Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowSystemMenuHint
		| Qt::WindowMinMaxButtonsHint
		| Qt::WindowCloseButtonHint;
	if (m_pOptions->bKeepOnTop) {
		pParent = this;
		wflags |= Qt::Tool;
	}

	// All forms are to be created right now.
	m_pMessagesForm = new qsynthMessagesForm(pParent, wflags);
	m_pChannelsForm = new qsynthChannelsForm(pParent, wflags);

	// Setup appropriately...
	m_pMessagesForm->setLogging(m_pOptions->bMessagesLog, m_pOptions->sMessagesLogPath);

	// Get the default setup and dummy instace tab.
	m_ui.TabBar->addEngine(new qsynthEngine(m_pOptions));
	// And all additional custom ones...
	QStringListIterator iter(m_pOptions->engines);
	while (iter.hasNext())
		m_ui.TabBar->addEngine(new qsynthEngine(m_pOptions, iter.next()));

	// Try to restore old window positioning.
	m_pOptions->loadWidgetGeometry(this, true);

	// And for the whole widget gallore...
	m_pOptions->loadWidgetGeometry(m_pMessagesForm);
	m_pOptions->loadWidgetGeometry(m_pChannelsForm);

	// Set defaults...
	updateMessagesFont();
	updateMessagesLimit();
	updateOutputMeters();
	updateSystemTray();

	// Knobs
	updateKnobs();

#if !defined(WIN32)
	// Check if we can redirect our own stdout/stderr...
	if (m_pOptions->bStdoutCapture && ::pipe(g_fdStdout) == 0) {
		::dup2(g_fdStdout[QSYNTH_FDWRITE], STDOUT_FILENO);
		::dup2(g_fdStdout[QSYNTH_FDWRITE], STDERR_FILENO);
		m_pStdoutNotifier = new QSocketNotifier(
			g_fdStdout[QSYNTH_FDREAD], QSocketNotifier::Read, this);
		QObject::connect(m_pStdoutNotifier,
			SIGNAL(activated(int)),
			SLOT(stdoutNotifySlot(int)));
	}
#endif

	// We'll accept drops from now on...
	setAcceptDrops(true);
	// Initial selection...
	tabSelect(0);
	// Final startup stabilization...
	stabilizeForm();

	// TabBar management.
	QObject::connect(m_ui.TabBar,
		SIGNAL(currentChanged(int)),
		SLOT(tabSelect(int)));
	QObject::connect(m_ui.TabBar,
		SIGNAL(contextMenuRequested(int, const QPoint &)),
		SLOT(tabContextMenu(int, const QPoint &)));

	// Register the initial timer slot.
	QTimer::singleShot(QSYNTH_TIMER_MSECS, this, SLOT(timerSlot()));
}


// Window close event handlers.
bool qsynthMainForm::queryClose (void)
{
	bool bQueryClose = true;

	// Now's the time?
	if (m_pOptions) {
#ifdef CONFIG_SYSTEM_TRAY
		// If we're not quitting explicitly and there's an
		// active system tray icon, then just hide ourselves.
		if (!m_bQuitForce && isVisible()
			&& m_pOptions->bSystemTray && m_pSystemTray) {
			m_pOptions->saveWidgetGeometry(this, true);
			const QString& sTitle = QSYNTH_TITLE ": " + tr("Information");
			const QString& sText
				= tr("The program will keep running in the system tray.\n\n"
					"To terminate the program, please choose \"Quit\"\n"
					"in the context menu of the system tray icon.");
		#ifdef QSYNTH_QT4_SYSTEM_TRAY
		#if QT_VERSION >= 0x040300
			if (QSystemTrayIcon::supportsMessages()) {
				m_pSystemTray->showMessage(
					sTitle, sText, QSystemTrayIcon::Information);
			}
			else
		#endif
		#endif
			QMessageBox::information(this, sTitle, sText);
			hide();
			bQueryClose = false;
		}
#endif
		// Dow we quit right away?
		if (bQueryClose && m_pOptions->bQueryClose) {
			for (int iTab = 0; iTab < m_ui.TabBar->count(); iTab++) {
				qsynthEngine *pEngine = m_ui.TabBar->engine(iTab);
				if (pEngine && pEngine->pSynth) {
					bQueryClose = (QMessageBox::warning(this,
						QSYNTH_TITLE ": " + tr("Warning"),
						QSYNTH_TITLE " " + tr("is about to terminate.") + "\n\n" +
						tr("Are you sure?"),
						QMessageBox::Ok | QMessageBox::Cancel)
						== QMessageBox::Ok);
					break;
				}
			}
		}
		// Some windows default fonts is here on demeand too.
		if (bQueryClose && m_pMessagesForm)
			m_pOptions->sMessagesFont = m_pMessagesForm->messagesFont().toString();
		// Try to save current positioning.
		if (bQueryClose) {
			m_pOptions->saveWidgetGeometry(m_pChannelsForm);
			m_pOptions->saveWidgetGeometry(m_pMessagesForm);
			m_pOptions->saveWidgetGeometry(this, true);
			// Close popup widgets.
			if (m_pMessagesForm)
				m_pMessagesForm->close();
			if (m_pChannelsForm)
				m_pChannelsForm->close();
			// And the system tray icon too.
			if (m_pSystemTray)
				m_pSystemTray->close();
		}
	}

#ifdef CONFIG_SYSTEM_TRAY
	// Whether we're really quitting.
	m_bQuitForce = bQueryClose;
#endif

	return bQueryClose;
}


void qsynthMainForm::closeEvent ( QCloseEvent *pCloseEvent )
{
	// Let's be sure about that...
	if (queryClose()) {
		pCloseEvent->accept();
		QApplication::quit();
	} else {
		pCloseEvent->ignore();
	}
}


// Add dropped files to playlist or soundfont stack.
void qsynthMainForm::playLoadFiles ( qsynthEngine *pEngine,
	const QStringList& files, bool bSetup )
{
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return;

	// Add each list item to Soundfont stack or MIDI player playlist...
	const QString sPrefix  = pEngine->name() + ": ";
	const QString sElipsis = "...";
	int   iSoundFonts = 0;
	int   iMidiFiles  = 0;
	QStringListIterator iter(files);
	while (iter.hasNext()) {
		const QString& sFilename = iter.next();
		// Is it a soundfont file...
		if (::fluid_is_soundfont(sFilename.toLocal8Bit().data())) {
			if (bSetup || !pSetup->soundfonts.contains(sFilename)) {
				appendMessagesColor(sPrefix +
					tr("Loading soundfont: \"%1\"")
					.arg(sFilename) + sElipsis, "#999933");
				if (::fluid_synth_sfload(
						pEngine->pSynth, sFilename.toLocal8Bit().data(), 1) >= 0) {
					iSoundFonts++;
					if (!bSetup) {
						pSetup->soundfonts.append(sFilename);
						pSetup->bankoffsets.append("0");
					}
				} else {
					appendMessagesError(sPrefix +
						tr("Failed to load the soundfont: \"%1\".")
						.arg(sFilename));
				}
			}
		}
		else  // Or is it a bare midifile?
		if (::fluid_is_midifile(sFilename.toLocal8Bit().data()) && pEngine->pPlayer) {
			appendMessagesColor(sPrefix +
				tr("Playing MIDI file: \"%1\"")
				.arg(sFilename) + sElipsis, "#99cc66");
			if (::fluid_player_add(
					pEngine->pPlayer, sFilename.toLocal8Bit().data()) >= 0) {
				iMidiFiles++;
			} else {
				appendMessagesError(sPrefix +
					tr("Failed to play MIDI file: \"%1\".")
					.arg(sFilename));
			}
		}
	}

	// Reset all presets, if applicable...
	if (!bSetup && iSoundFonts > 0) {
		resetEngine(pEngine);
		resetChannelsForm(pEngine, false);
	}

	// Start playing, if any...
	if (pEngine->pPlayer && iMidiFiles > 0)
		::fluid_player_play(pEngine->pPlayer);
}


void qsynthMainForm::dragEnterEvent ( QDragEnterEvent* pDragEnterEvent )
{
	bool bAccept = false;

	if (pDragEnterEvent->source() == NULL) {
		const QMimeData *pMimeData = pDragEnterEvent->mimeData();
		if (pMimeData && pMimeData->hasUrls()) {
			QListIterator<QUrl> iter(pMimeData->urls());
			while (iter.hasNext()) {
				const QString& sFilename
					= iter.next().toLocalFile();
				if (!sFilename.isEmpty()) {
                                        QByteArray tmp = sFilename.toLocal8Bit();
					char *pszFilename = tmp.data();
					if (::fluid_is_midifile(pszFilename) ||
						::fluid_is_soundfont(pszFilename))
						bAccept = true;
				}
			}
		}
	}

	if (bAccept)
		pDragEnterEvent->accept();
	else
		pDragEnterEvent->ignore();
}


void qsynthMainForm::dropEvent ( QDropEvent* pDropEvent )
{
	if (pDropEvent->source())
		return;

	const QMimeData *pMimeData = pDropEvent->mimeData();
	if (pMimeData && pMimeData->hasUrls()) {
		QStringList files;
		QListIterator<QUrl> iter(pMimeData->urls());
		while (iter.hasNext()) {
			const QString& sFilename
				= iter.next().toLocalFile();
			if (!sFilename.isEmpty())
				files.append(sFilename);
		}
		playLoadFiles(currentEngine(), files, false);
	}

}


// Own stdout/stderr socket notifier slot.
void qsynthMainForm::stdoutNotifySlot ( int fd )
{
#if !defined(WIN32)
	char achBuffer[1024];
	int  cchBuffer = ::read(fd, achBuffer, sizeof(achBuffer) - 1);
	if (cchBuffer > 0) {
		achBuffer[cchBuffer] = (char) 0;
		appendStdoutBuffer(achBuffer);
	}
#endif
}


// Stdout buffer handler -- now splitted by complete new-lines...
void qsynthMainForm::appendStdoutBuffer ( const QString& s )
{
	m_sStdoutBuffer.append(s);

	int iLength = m_sStdoutBuffer.lastIndexOf('\n') + 1;
	if (iLength > 0) {
		QString sTemp = m_sStdoutBuffer.left(iLength);
		m_sStdoutBuffer.remove(0, iLength);
		QStringList list = sTemp.split('\n');
		QStringListIterator iter(list);
		while (iter.hasNext())
			appendMessagesText(iter.next());
	}
}


// Stdout flusher -- show up any unfinished line...
void qsynthMainForm::flushStdoutBuffer (void)
{
	if (!m_sStdoutBuffer.isEmpty()) {
		appendMessagesText(m_sStdoutBuffer);
		m_sStdoutBuffer.truncate(0);
	}
}


// Messages output methods.
void qsynthMainForm::appendMessages( const QString& s )
{
	if (m_pMessagesForm)
		m_pMessagesForm->appendMessages(s);
}

void qsynthMainForm::appendMessagesColor( const QString& s, const QString& c )
{
	if (m_pMessagesForm)
		m_pMessagesForm->appendMessagesColor(s, c);
}

void qsynthMainForm::appendMessagesText( const QString& s )
{
	if (m_pMessagesForm)
		m_pMessagesForm->appendMessagesText(s);
}

void qsynthMainForm::appendMessagesError( const QString& s )
{
	if (m_pMessagesForm)
		m_pMessagesForm->show();

	appendMessagesColor(s.simplified(), "#ff0000");

	const QString& sTitle = QSYNTH_TITLE ": " + tr("Error");
#ifdef CONFIG_SYSTEM_TRAY
#ifdef QSYNTH_QT4_SYSTEM_TRAY
#if QT_VERSION >= 0x040300
	if (m_pOptions->bSystemTray && m_pSystemTray
		&& QSystemTrayIcon::supportsMessages()) {
		m_pSystemTray->showMessage(sTitle, s, QSystemTrayIcon::Critical);
	}
	else
#endif
#endif
#endif
	QMessageBox::critical(this, sTitle, s, QMessageBox::Cancel);
}


// Force update of the messages font.
void qsynthMainForm::updateMessagesFont (void)
{
	if (m_pOptions == NULL)
		return;

	if (m_pMessagesForm && !m_pOptions->sMessagesFont.isEmpty()) {
		QFont font;
		if (font.fromString(m_pOptions->sMessagesFont))
			m_pMessagesForm->setMessagesFont(font);
	}
}


// Update messages window line limit.
void qsynthMainForm::updateMessagesLimit (void)
{
	if (m_pOptions == NULL)
		return;

	if (m_pMessagesForm) {
		if (m_pOptions->bMessagesLimit)
			m_pMessagesForm->setMessagesLimit(m_pOptions->iMessagesLimitLines);
		else
			m_pMessagesForm->setMessagesLimit(-1);
	}
}


// Force update of the output meters visibility.
void qsynthMainForm::updateOutputMeters (void)
{
	if (m_pOptions == NULL)
		return;

	if (m_pOptions->bOutputMeters)
		m_ui.OutputGroupBox->show();
	else
		m_ui.OutputGroupBox->hide();

//	adjustSize();
}


// System tray master switcher.
void qsynthMainForm::updateSystemTray (void)
{
	if (m_pOptions == NULL)
		return;

#ifdef CONFIG_SYSTEM_TRAY
	if (!m_pOptions->bSystemTray && m_pSystemTray) {
	//  Strange enough, this would close the application too.
	//  m_pSystemTray->close();
		delete m_pSystemTray;
		m_pSystemTray = NULL;
	}
	if (m_pOptions->bSystemTray && m_pSystemTray == NULL) {
		m_pSystemTray = new qsynthSystemTray(this);
		m_pSystemTray->show();
		QObject::connect(m_pSystemTray,
			SIGNAL(clicked()),
			SLOT(toggleMainForm()));
		QObject::connect(m_pSystemTray,
			SIGNAL(contextMenuRequested(const QPoint &)),
			SLOT(systemTrayContextMenu(const QPoint &)));
	} else {
		// Make sure the main widget is visible.
		show();
		raise();
		activateWindow();
	}
#endif
}


// System tray context menu request slot.
void qsynthMainForm::systemTrayContextMenu ( const QPoint& pos )
{
	if (m_pOptions == NULL)
		return;

	QMenu menu(this);
	QAction *pAction;

	QString sHideMinimize = (m_pOptions->bSystemTray && m_pSystemTray
		? tr("&Hide") : tr("Mi&nimize"));
	QString sShowRestore  = (m_pOptions->bSystemTray && m_pSystemTray
		? tr("S&how") : tr("Rest&ore"));
	pAction = menu.addAction(isVisible()
		? sHideMinimize : sShowRestore, this, SLOT(toggleMainForm()));
	menu.addSeparator();

	qsynthEngine *pEngine = currentEngine();
	pAction = menu.addAction(QIcon(":/images/add1.png"),
		tr("&New engine..."), this, SLOT(newEngine()));
	pAction = menu.addAction(QIcon(":/images/remove1.png"),
		tr("&Delete"), this, SLOT(deleteEngine()));
	pAction->setEnabled(pEngine && !pEngine->isDefault());
	menu.addSeparator();

	bool bEnabled = (pEngine && pEngine->pSynth);
	pAction = menu.addAction(QIcon(":/images/restart1.png"),
		bEnabled ? tr("Re&start") : tr("&Start"), this, SLOT(promptRestart()));
	pAction = menu.addAction(QIcon(":/images/reset1.png"),
		tr("&Reset"), this, SLOT(programReset()));
	pAction->setEnabled(bEnabled);
	pAction = menu.addAction(QIcon(":/images/panic1.png"),
		tr("&Panic"), this, SLOT(systemReset()));
	pAction->setEnabled(bEnabled);
	menu.addSeparator();
	pAction = menu.addAction(QIcon(":/images/channels1.png"),
		tr("&Channels"), this, SLOT(toggleChannelsForm()));
	pAction->setCheckable(true);
	pAction->setChecked(m_pChannelsForm && m_pChannelsForm->isVisible());
	pAction->setEnabled(bEnabled);
	pAction = menu.addAction(QIcon(":/images/setup1.png"),
		tr("Set&up..."), this, SLOT(showSetupForm()));
	menu.addSeparator();

	// Construct the actual engines menu,
	// overriding the last one, if any...
	// Add presets menu to the main context menu...
	QMenu *pEnginesMenu = menu.addMenu(tr("Engines"));
	for (int iTab = 0; iTab < m_ui.TabBar->count(); iTab++) {
		pEngine = m_ui.TabBar->engine(iTab);
		if (pEngine) {
			pAction = pEnginesMenu->addAction(pEngine->name());
			pAction->setCheckable(true);
			pAction->setChecked(pEngine == currentEngine());
			pAction->setData(iTab);
		}
	}
	QObject::connect(pEnginesMenu,
		SIGNAL(triggered(QAction*)),
		SLOT(activateEnginesMenu(QAction*)));
	menu.addSeparator();

	pAction = menu.addAction(QIcon(":/images/messages1.png"),
		tr("&Messages"), this, SLOT(toggleMessagesForm()));
	pAction->setCheckable(true);
	pAction->setChecked(m_pMessagesForm && m_pMessagesForm->isVisible());
	pAction = menu.addAction(QIcon(":/images/options1.png"),
		tr("&Options..."), this, SLOT(showOptionsForm()));
//  pAction = menu.AddAction(QIcon(":/images/about1.png"),
//      tr("A&bout..."), this, SLOT(showAboutForm()));
	menu.addSeparator();

	pAction = menu.addAction(QIcon(":/images/quit1.png"),
		tr("&Quit"), this, SLOT(quitMainForm()));

	menu.exec(pos);
}


// Stabilize current form toggle buttons that may be astray.
void qsynthMainForm::stabilizeForm (void)
{
	qsynthEngine *pEngine = currentEngine();

	bool bEnabled = (pEngine && pEngine->pSynth);

	m_ui.GainGroupBox->setEnabled(bEnabled);
	m_ui.ReverbGroupBox->setEnabled(bEnabled);
	m_ui.ChorusGroupBox->setEnabled(bEnabled);
	m_ui.OutputGroupBox->setEnabled(bEnabled && pEngine->bMeterEnabled);
	m_ui.ProgramResetPushButton->setEnabled(bEnabled);
	m_ui.SystemResetPushButton->setEnabled(bEnabled);
	m_ui.ChannelsPushButton->setEnabled(bEnabled);

	if (bEnabled) {
		bool bReverbActive = m_ui.ReverbActiveCheckBox->isChecked();
		m_ui.ReverbRoomTextLabel->setEnabled(bReverbActive);
		m_ui.ReverbDampTextLabel->setEnabled(bReverbActive);
		m_ui.ReverbWidthTextLabel->setEnabled(bReverbActive);
		m_ui.ReverbLevelTextLabel->setEnabled(bReverbActive);
		m_ui.ReverbRoomDial->setEnabled(bReverbActive);
		m_ui.ReverbDampDial->setEnabled(bReverbActive);
		m_ui.ReverbWidthDial->setEnabled(bReverbActive);
		m_ui.ReverbLevelDial->setEnabled(bReverbActive);
		m_ui.ReverbRoomSpinBox->setEnabled(bReverbActive);
		m_ui.ReverbDampSpinBox->setEnabled(bReverbActive);
		m_ui.ReverbWidthSpinBox->setEnabled(bReverbActive);
		m_ui.ReverbLevelSpinBox->setEnabled(bReverbActive);
		bool bChorusActive = m_ui.ChorusActiveCheckBox->isChecked();
		m_ui.ChorusNrTextLabel->setEnabled(bChorusActive);
		m_ui.ChorusLevelTextLabel->setEnabled(bChorusActive);
		m_ui.ChorusSpeedTextLabel->setEnabled(bChorusActive);
		m_ui.ChorusDepthTextLabel->setEnabled(bChorusActive);
		m_ui.ChorusTypeTextLabel->setEnabled(bChorusActive);
		m_ui.ChorusNrDial->setEnabled(bChorusActive);
		m_ui.ChorusLevelDial->setEnabled(bChorusActive);
		m_ui.ChorusSpeedDial->setEnabled(bChorusActive);
		m_ui.ChorusDepthDial->setEnabled(bChorusActive);
		m_ui.ChorusNrSpinBox->setEnabled(bChorusActive);
		m_ui.ChorusLevelSpinBox->setEnabled(bChorusActive);
		m_ui.ChorusSpeedSpinBox->setEnabled(bChorusActive);
		m_ui.ChorusDepthSpinBox->setEnabled(bChorusActive);
		m_ui.ChorusTypeComboBox->setEnabled(bChorusActive);
		m_ui.RestartPushButton->setText(tr("Re&start"));
	} else {
		m_ui.RestartPushButton->setText(tr("&Start"));
	}
	m_ui.RestartPushButton->setEnabled(true);

	m_ui.DeleteEngineToolButton->setEnabled(pEngine && !pEngine->isDefault());

	m_ui.MessagesPushButton->setChecked(
		m_pMessagesForm && m_pMessagesForm->isVisible());
	m_ui.ChannelsPushButton->setChecked(
		m_pChannelsForm && m_pChannelsForm->isVisible());
}


// Program reset command slot (all channels).
void qsynthMainForm::programReset (void)
{
	m_ui.ProgramResetPushButton->setEnabled(false);

	resetEngine(currentEngine());
	if (m_pChannelsForm)
		m_pChannelsForm->resetAllChannels(true);
	stabilizeForm();
}


// System reset command slot.
void qsynthMainForm::systemReset (void)
{
	m_ui.SystemResetPushButton->setEnabled(false);

	qsynthEngine *pEngine = currentEngine();
	if (pEngine && pEngine->pSynth) {
#ifdef CONFIG_FLUID_RESET
		appendMessagesColor(pEngine->name() + ": fluid_synth_system_reset()", "#993366");
		::fluid_synth_system_reset(pEngine->pSynth);
#else
		appendMessagesColor(pEngine->name() + ": fluid_synth_program_reset()", "#996666");
		::fluid_synth_program_reset(pEngine->pSynth);
#endif
		if (m_pChannelsForm)
			m_pChannelsForm->resetAllChannels(true);
	}
	stabilizeForm();
}


// Complete engine restart.
void qsynthMainForm::promptRestart (void)
{
	restartEngine(currentEngine());
}


// Prompt and create a new engine instance.
void qsynthMainForm::newEngine (void)
{
	qsynthEngine *pEngine;
	QString sName;

	// Simple hack for finding a unused engine name...
	const QString sPrefix = QSYNTH_TITLE;
	int   iSuffix = m_ui.TabBar->count() + 1;	// One is always there, so try after...
	bool  bRetry  = true;
	while (bRetry) {
		sName  = sPrefix + QString::number(iSuffix++);
		bRetry = false;
		for (int iTab = 0; iTab < m_ui.TabBar->count() && !bRetry; iTab++) {
			pEngine = m_ui.TabBar->engine(iTab);
			if (pEngine && pEngine->name() == sName)
				bRetry = true;
		}
	}

	// Probably a good idea to prompt for the setup dialog.
	pEngine = new qsynthEngine(m_pOptions, sName);
	if (setupEngineTab(pEngine, -1)) {
		// Success, add a new tab...
		int iTab = m_ui.TabBar->addEngine(pEngine);
		// And try to be persistent...
		m_pOptions->newEngine(pEngine);
		// Update bar...
		m_ui.TabBar->setCurrentIndex(iTab);
		m_ui.TabBar->update();
	} else {
		// As this will not be mangaed by a qsynthTab instance,
		// we better free it up right now...
		delete pEngine;
	}
}


// Delete the current engine instance.
void qsynthMainForm::deleteEngine (void)
{
	// Check if we're doing everything all right.
	qsynthEngine *pEngine = m_ui.TabBar->engine(m_iCurrentTab);
	if (pEngine)
		deleteEngineTab(pEngine, m_iCurrentTab);
}


// Delete and engine instance.
bool qsynthMainForm::deleteEngineTab ( qsynthEngine *pEngine, int iTab )
{
	if (pEngine == NULL || iTab < 0)
		return false;

	// Try to prompt user if he/she really wants this...
	bool bResult = (QMessageBox::warning(this,
		QSYNTH_TITLE ": " + tr("Warning"),
		tr("Delete fluidsynth engine:") + "\n\n" +
		pEngine->name() + "\n\n" +
		tr("Are you sure?"),
		QMessageBox::Ok | QMessageBox::Cancel)
		== QMessageBox::Ok);

	if (bResult) {
		// First we try to stop the angine.
		stopEngine(pEngine);
		// Better nullify the current reference, if applicable.
		if (g_pCurrentEngine == pEngine)
			g_pCurrentEngine = NULL;
		if (m_iCurrentTab == iTab)
			m_iCurrentTab = -1;
		// Nows time to remove those crappy entries...
		m_pOptions->deleteEngine(pEngine);
		// Then, we delete the instance (note that the engine object
		// is owned by the tab instance, so it will be delete here).
		m_ui.TabBar->removeEngine(iTab);
		m_ui.TabBar->update();
		tabSelect(m_ui.TabBar->currentIndex());
	}

	return bResult;
}


// Edit settings of a given engine instance.
bool qsynthMainForm::setupEngineTab ( qsynthEngine *pEngine, int iTab )
{
	if (pEngine == NULL || pEngine->setup() == NULL)
		return false;

	qsynthSetupForm setupForm(this);
	// Load the current instance settings.
	setupForm.setup(m_pOptions, pEngine, iTab < 0);
	// Show the instance setup dialog, then ask for a engine restart?
	if (!setupForm.exec())
		return false;

	// Have we changed names? Ugly uh?
	m_pOptions->renameEngine(pEngine);
	if (iTab >= 0) {
		// Update main caption, if we're on current engine tab...
		if (iTab == m_ui.TabBar->currentIndex()) {
			setWindowTitle(QSYNTH_TITLE " - " + tr(QSYNTH_SUBTITLE)
				+ " [" + pEngine->name() + "]");
		}
		// Finally update tab text...
		m_ui.TabBar->setTabText(iTab, pEngine->name());
	}

	// Now we may restart this.
	restartEngine(pEngine);

	// Done.
	return true;
}


// Main form visibility requester slot.
void qsynthMainForm::toggleMainForm (void)
{
	if (m_pOptions == NULL)
		return;

	m_pOptions->saveWidgetGeometry(this, true);

	if (isVisible()) {
		if (m_pOptions->bSystemTray && m_pSystemTray) {
			// Hide away from sight.
			hide();
		} else {
			// Minimize (iconify) normally.
			showMinimized();
		}
	} else {
		show();
		raise();
		activateWindow();
	}
}


// Message log form requester slot.
void qsynthMainForm::toggleMessagesForm (void)
{
	if (m_pOptions == NULL)
		return;

	if (m_pMessagesForm) {
		m_pOptions->saveWidgetGeometry(m_pMessagesForm);
		if (m_pMessagesForm->isVisible()) {
			m_pMessagesForm->hide();
		} else {
			m_pMessagesForm->show();
			m_pMessagesForm->raise();
			m_pMessagesForm->activateWindow();
		}
	}
}


// Channels view form requester slot.
void qsynthMainForm::toggleChannelsForm (void)
{
	if (m_pOptions == NULL)
		return;

	if (m_pChannelsForm) {
		m_pOptions->saveWidgetGeometry(m_pChannelsForm);
		if (m_pChannelsForm->isVisible()) {
			m_pChannelsForm->hide();
		} else {
			m_pChannelsForm->show();
			m_pChannelsForm->raise();
			m_pChannelsForm->activateWindow();
		}
	}
}


// Instance dialog requester slot.
void qsynthMainForm::showSetupForm (void)
{
	int iTab = m_ui.TabBar->currentIndex();
	if (iTab >= 0)
		setupEngineTab(m_ui.TabBar->engine(iTab), iTab);
}


// Setup dialog requester slot.
void qsynthMainForm::showOptionsForm (void)
{
	if (m_pOptions == NULL)
		return;

	qsynthOptionsForm *pOptionsForm = new qsynthOptionsForm(this);
	if (pOptionsForm) {
		// Check out some initial nullities(tm)...
		if (m_pOptions->sMessagesFont.isEmpty() && m_pMessagesForm)
			m_pOptions->sMessagesFont = m_pMessagesForm->messagesFont().toString();
		// To track down deferred or immediate changes.
		bool    bOldMessagesLog = m_pOptions->bMessagesLog;
		QString sOldMessagesLogPath = m_pOptions->sMessagesLogPath;
		QString sMessagesFont  = m_pOptions->sMessagesFont;
		bool    bSystemTray    = m_pOptions->bSystemTray;
		bool    bOutputMeters  = m_pOptions->bOutputMeters;
		bool    bStdoutCapture = m_pOptions->bStdoutCapture;
		bool    bKeepOnTop     = m_pOptions->bKeepOnTop;
		int     iOldBaseFontSize = m_pOptions->iBaseFontSize;
		int     bMessagesLimit = m_pOptions->bMessagesLimit;
		int     iMessagesLimitLines = m_pOptions->iMessagesLimitLines;
		int     iKnobStyle     = m_pOptions->iKnobStyle;
		int     iKnobMotion    = m_pOptions->iKnobMotion;
		// Load the current setup settings.
		pOptionsForm->setup(m_pOptions);
		// Show the setup dialog...
		if (pOptionsForm->exec()) {
			// Warn if something will be only effective on next run.
			if (( bStdoutCapture && !m_pOptions->bStdoutCapture) ||
				(!bStdoutCapture &&  m_pOptions->bStdoutCapture) ||
				( bKeepOnTop     && !m_pOptions->bKeepOnTop)     ||
				(!bKeepOnTop     &&  m_pOptions->bKeepOnTop)     ||
				(iOldBaseFontSize != m_pOptions->iBaseFontSize)) {
				const QString& sTitle
					= QSYNTH_TITLE ": " + tr("Information");
				const QString& sText
					= tr("Some settings will be only effective\n"
						"next time you start this program.");
			#ifdef CONFIG_SYSTEM_TRAY
			#ifdef QSYNTH_QT4_SYSTEM_TRAY
			#if QT_VERSION >= 0x040300
				if (m_pOptions->bSystemTray && m_pSystemTray
					&& QSystemTrayIcon::supportsMessages()) {
					m_pSystemTray->showMessage(
						sTitle, sText, QSystemTrayIcon::Information);
				}
				else
			#endif
			#endif
			#endif
				QMessageBox::information(this, sTitle, sText);
			}
			// Check wheather something immediate has changed.
			if (( bOldMessagesLog && !m_pOptions->bMessagesLog) ||
				(!bOldMessagesLog &&  m_pOptions->bMessagesLog) ||
				(sOldMessagesLogPath != m_pOptions->sMessagesLogPath))
				m_pMessagesForm->setLogging(
					m_pOptions->bMessagesLog, m_pOptions->sMessagesLogPath);
			if (sMessagesFont != m_pOptions->sMessagesFont)
				updateMessagesFont();
			if (( bMessagesLimit && !m_pOptions->bMessagesLimit) ||
				(!bMessagesLimit &&  m_pOptions->bMessagesLimit) ||
				(iMessagesLimitLines !=  m_pOptions->iMessagesLimitLines))
				updateMessagesLimit();
			if (( bSystemTray && !m_pOptions->bSystemTray) ||
				(!bSystemTray &&  m_pOptions->bSystemTray))
				updateSystemTray();
			if (( iKnobStyle != m_pOptions->iKnobStyle) ||
				( iKnobMotion != m_pOptions->iKnobMotion))
				updateKnobs();
			// There's some option(s) that need a global restart...
			if (( bOutputMeters  && !m_pOptions->bOutputMeters) ||
				(!bOutputMeters  &&  m_pOptions->bOutputMeters)) {
				updateOutputMeters();
				restartAllEngines();
			}
		}
		// Done.
		delete pOptionsForm;
	}
}


// About dialog requester slot.
void qsynthMainForm::showAboutForm (void)
{
	qsynthAboutForm *pAboutForm = new qsynthAboutForm(this);
	if (pAboutForm) {
		pAboutForm->exec();
		delete pAboutForm;
	}
}


// Tab selection slot.
void qsynthMainForm::tabSelect ( int iTab )
{
	if (iTab == m_iCurrentTab)
		return;

	// Try to save old tab settings...
	if (m_iCurrentTab >= 0) {
		qsynthEngine *pEngine = m_ui.TabBar->engine(m_iCurrentTab);
		if (pEngine)
			savePanelSettings(pEngine);
	}

	// Make it official.
	m_iCurrentTab = iTab;

	// And set new ones, while refreshing views...
	if (m_iCurrentTab >= 0) {
		qsynthEngine *pEngine = m_ui.TabBar->engine(m_iCurrentTab);
		if (pEngine) {
			// Set current engine reference hack.
			g_pCurrentEngine = pEngine;
			// And do the change.
			setWindowTitle(QSYNTH_TITLE " - " + tr(QSYNTH_SUBTITLE)
				+ " [" + pEngine->name() + "]");
			loadPanelSettings(pEngine, false);
			resetChannelsForm(pEngine, false);
		}
	}

	// Finally, stabilize main form.
	stabilizeForm();
}


// Common context request slot.
void qsynthMainForm::tabContextMenu ( int iTab, const QPoint& pos )
{
	qsynthEngine *pEngine = m_ui.TabBar->engine(iTab);
	m_ui.TabBar->setCurrentIndex(iTab);

	QMenu menu(this);
	QAction *pAction;

	pAction = menu.addAction(QIcon(":/images/add1.png"),
		tr("&New engine..."), this, SLOT(newEngine()));
	pAction = menu.addAction(QIcon(":/images/remove1.png"),
		tr("&Delete"), this, SLOT(deleteEngine()));
	pAction->setEnabled(pEngine && !pEngine->isDefault());
	menu.addSeparator();
	pAction = menu.addAction(QIcon(":/images/setup1.png"),
		tr("Set&up..."), this, SLOT(showSetupForm()));
	pAction->setEnabled(pEngine != NULL);

	menu.exec(pos);}


// Timer callback funtion.
void qsynthMainForm::timerSlot (void)
{
	// Is it the first shot on synth start after a one slot delay?
	if (m_iTimerDelay < QSYNTH_DELAY_MSECS) {
		m_iTimerDelay += QSYNTH_TIMER_MSECS;
		if (m_iTimerDelay >= QSYNTH_DELAY_MSECS) {
			// Start the press!
			for (int iTab = 0; iTab < m_ui.TabBar->count(); iTab++)
				startEngine(m_ui.TabBar->engine(iTab));
		}
	}

	// Some global MIDI activity?
	int iTabUpdate = 0;
	for (int iTab = 0; iTab < m_ui.TabBar->count(); iTab++) {
		qsynthEngine *pEngine = m_ui.TabBar->engine(iTab);
		if (pEngine->iMidiEvent > 0) {
			pEngine->iMidiEvent = 0;
			if (pEngine->iMidiState == 0) {
				pEngine->iMidiState++;
				m_ui.TabBar->setOn(iTab, true);
				iTabUpdate++;
				// Change the system tray icon background color!
				if (m_pSystemTray && m_iSystemTrayState == 0) {
					m_iSystemTrayState++;
					m_pSystemTray->setBackground(Qt::green);
				}
			}
		}
		else if (pEngine->iMidiEvent == 0 && pEngine->iMidiState > 0) {
			pEngine->iMidiState--;
			m_ui.TabBar->setOn(iTab, false);
			iTabUpdate++;
			// Reset the system tray icon background!
			if (m_pSystemTray && m_iSystemTrayState > 0) {
				m_iSystemTrayState--;
				m_pSystemTray->setBackground(Qt::transparent);
			}
		}
	}
	// Have we an update?
	if (iTabUpdate > 0)
		m_ui.TabBar->update();

	// MIDI Channel activity breakout...
	if (m_pChannelsForm) {
		for (int iChan = 0; iChan < g_iMidiChannels; iChan++) {
			if (g_pMidiChannels[iChan].iEvent > 0) {
				g_pMidiChannels[iChan].iEvent = 0;
				// Activity tracking...
				if (g_pMidiChannels[iChan].iState == 0) {
					m_pChannelsForm->setChannelOn(iChan, true);
					g_pMidiChannels[iChan].iState++;
				}
				// Control and/or program change...
				if (g_pMidiChannels[iChan].iChange > 0) {
					g_pMidiChannels[iChan].iChange = 0;
					m_pChannelsForm->updateChannel(iChan);
				}
			}   // Activity fallback...
			else if (g_pMidiChannels[iChan].iEvent == 0 && g_pMidiChannels[iChan].iState > 0) {
				m_pChannelsForm->setChannelOn(iChan, false);
				g_pMidiChannels[iChan].iState--;
			}
		}
	}

	// Gain changes?
	if (m_iGainChanged > 0)
		updateGain();

	// Reverb changes?
	if (m_iReverbChanged > 0)
		updateReverb();

	// Chorus changes?
	if (m_iChorusChanged > 0)
		updateChorus();

	// Meter update.
	if (g_pCurrentEngine && g_pCurrentEngine->bMeterEnabled) {
		m_ui.OutputMeter->setValue(0, g_pCurrentEngine->fMeterValue[0]);
		m_ui.OutputMeter->setValue(1, g_pCurrentEngine->fMeterValue[1]);
	//	m_ui.OutputMeter->refresh();
		g_pCurrentEngine->fMeterValue[0] = 0.0f;
		g_pCurrentEngine->fMeterValue[1] = 0.0f;
	}

	// Register for the next timer slot.
	QTimer::singleShot(QSYNTH_TIMER_MSECS, this, SLOT(timerSlot()));
}


// Return the current selected engine.
qsynthEngine *qsynthMainForm::currentEngine (void) const
{
	return g_pCurrentEngine;
}


// Start the fluidsynth clone, based on given settings.
bool qsynthMainForm::startEngine ( qsynthEngine *pEngine )
{
	if (pEngine == NULL)
		return false;
	if (pEngine->pSynth)
		return true;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return false;

	// Start realizing settings...
	pSetup->realize();

	const QString sPrefix  = pEngine->name() + ": ";
	const QString sElipsis = "...";

	// Create the synthesizer.
	appendMessages(sPrefix + tr("Creating synthesizer engine") + sElipsis);
	pEngine->pSynth = ::new_fluid_synth(pSetup->fluid_settings());
	if (pEngine->pSynth == NULL) {
		appendMessagesError(sPrefix + tr("Failed to create the synthesizer.\n\nCannot continue without it."));
		return false;
	}

#ifdef QSYNTH_CUSTOM_LOADER
	// Add special loader to mirror fonts in use by another engine...
	fluid_sfloader_t *pLoader
		= (fluid_sfloader_t *) ::malloc(sizeof(fluid_sfloader_t));
	pLoader->data = (void *) pEngine;
	pLoader->load = qsynth_sfloader_load;
	pLoader->free = qsynth_sfloader_free;
	::fluid_synth_add_sfloader(pEngine->pSynth, pLoader);
	// Push on to engine list.
	qsynthEngineNode *pNode = new qsynthEngineNode;
	pNode->pEngine = pEngine;
	pNode->pNext   = g_pEngineList;
	pNode->pPrev   = NULL;
	if (g_pEngineList)
		g_pEngineList->pPrev = pNode;
	g_pEngineList = pNode;
#endif

	// Load soundfonts...
	int i = 0;
	QStringListIterator iter(pSetup->soundfonts);
	while (iter.hasNext()) {
		const QString& sFilename = iter.next();
		// Is it a soundfont file...
		if (::fluid_is_soundfont(sFilename.toLocal8Bit().data())) {
			int iBankOffset = pSetup->bankoffsets[i].toInt();
			appendMessagesColor(sPrefix +
				tr("Loading soundfont: \"%1\" (bank offset %2)")
				.arg(sFilename).arg(iBankOffset) + sElipsis, "#999933");
			int iSFID = ::fluid_synth_sfload(
				pEngine->pSynth, sFilename.toLocal8Bit().data(), 1);
			if (iSFID < 0)
				appendMessagesError(sPrefix +
					tr("Failed to load the soundfont: \"%1\".")
					.arg(sFilename));
#ifdef CONFIG_FLUID_BANK_OFFSET
			else
			if (::fluid_synth_set_bank_offset(
				pEngine->pSynth, iSFID, iBankOffset) < 0) {
				appendMessagesError(sPrefix +
					tr("Failed to set bank offset (%1) for soundfont: \"%2\".")
					.arg(iBankOffset).arg(sFilename));
			}
#endif
		}
		i++;
	}

	// Start the synthesis thread...
	appendMessages(sPrefix +
		tr("Creating audio driver (%1)")
		.arg(pSetup->sAudioDriver) + sElipsis);
	pEngine->pAudioDriver  = NULL;
	pEngine->bMeterEnabled = false;
	if (m_pOptions->bOutputMeters) {
		pEngine->pAudioDriver  = ::new_fluid_audio_driver2(
			pSetup->fluid_settings(), qsynth_process, pEngine);
		pEngine->bMeterEnabled = (pEngine->pAudioDriver != NULL);
	}
	if (pEngine->pAudioDriver == NULL)
		pEngine->pAudioDriver = ::new_fluid_audio_driver(
			pSetup->fluid_settings(), pEngine->pSynth);
	if (pEngine->pAudioDriver == NULL) {
		appendMessagesError(sPrefix +
			tr("Failed to create the audio driver (%1).\n\n"
			"Cannot continue without it.")
			.arg(pSetup->sAudioDriver));
		stopEngine(pEngine);
		return false;
	}

	// Start the midi router and link it to the synth...
	if (pSetup->bMidiIn) {
		// In dump mode, text output is generated for events going into
		// and out of the router. The example dump functions are put into
		// the chain before and after the router..
		appendMessages(sPrefix +
			tr("Creating MIDI router (%1)")
			.arg(pSetup->sMidiDriver) + sElipsis);
		pEngine->pMidiRouter = ::new_fluid_midi_router(
			pSetup->fluid_settings(), pSetup->bMidiDump
			? qsynth_dump_postrouter
			: qsynth_handle_midi_event,
			(void *) pEngine);
		if (pEngine->pMidiRouter == NULL) {
			appendMessagesError(sPrefix +
				tr("Failed to create the MIDI input router (%1).\n\n"
				"No MIDI input will be available.")
				.arg(pSetup->sMidiDriver));
		} else {
			::fluid_synth_set_midi_router(pEngine->pSynth, pEngine->pMidiRouter);
			appendMessages(sPrefix +
				tr("Creating MIDI driver (%1)")
				.arg(pSetup->sMidiDriver) + sElipsis);
			pEngine->pMidiDriver = ::new_fluid_midi_driver(
				pSetup->fluid_settings(), pSetup->bMidiDump
				? ::fluid_midi_dump_prerouter
				: ::fluid_midi_router_handle_midi_event,
				static_cast<void *> (pEngine->pMidiRouter));
			if (pEngine->pMidiDriver == NULL)
				appendMessagesError(sPrefix +
					tr("Failed to create the MIDI driver (%1).\n\n"
					"No MIDI input will be available.")
					.arg(pSetup->sMidiDriver));
		}
	}

	// Create the MIDI player.
	appendMessages(sPrefix + tr("Creating MIDI player") + sElipsis);
	pEngine->pPlayer = ::new_fluid_player(pEngine->pSynth);
	if (pEngine->pPlayer == NULL) {
		appendMessagesError(sPrefix +
			tr("Failed to create the MIDI player.\n\n"
			"Continuing without a player."));
	} else {
		// Play the midi files, if any.
		playLoadFiles(pEngine, pSetup->midifiles, false);
	}

	// Run the server, if requested.
	if (pSetup->bServer) {
#ifdef CONFIG_FLUID_SERVER
		appendMessages(sPrefix + tr("Creating server") + sElipsis);
		// Server port must be different for each engine...
		char szShellPort[] = "shell.port";
		if (g_iLastShellPort > 0) {
			g_iLastShellPort++;
		} else {
			g_iLastShellPort = 0;
			::fluid_settings_getint(
				pSetup->fluid_settings(), szShellPort, &g_iLastShellPort);
			if (g_iLastShellPort == 0) {
				g_iLastShellPort = ::fluid_settings_getint_default(
					pSetup->fluid_settings(), szShellPort);
			}
		}
		// Set the (new) server port for this engne...
		::fluid_settings_setint(
			pSetup->fluid_settings(), szShellPort, g_iLastShellPort);
		// Create the server now...
		pEngine->pServer = ::new_fluid_server(
			pSetup->fluid_settings(), qsynth_newclient, pEngine->pSynth);
		if (pEngine->pServer == NULL)
			appendMessagesError(sPrefix +
				tr("Failed to create the server.\n\n"
				"Continuing without it."));
#else
		appendMessagesError(sPrefix +
			tr("Server mode disabled.\n\n"
			"Continuing without it."));
#endif
	}

	// Make an initial program reset.
	m_pOptions->loadPreset(pEngine, pSetup->sDefPreset);

	// Show up our efforts, if we're currently selected :)
	if (pEngine == currentEngine()) {
		loadPanelSettings(pEngine, true);
		resetChannelsForm(pEngine, true);
		stabilizeForm();
	} else {
		setEngineReverbOn(pEngine,
			pSetup->bReverbActive);
		setEngineChorusOn(pEngine,
			pSetup->bChorusActive);
		setEngineGain(pEngine,
			pSetup->fGain);
		setEngineReverb(pEngine,
			pSetup->fReverbRoom,
			pSetup->fReverbDamp,
			pSetup->fReverbWidth,
			pSetup->fReverbLevel);
		setEngineChorus(pEngine,
			pSetup->iChorusNr,
			pSetup->fChorusLevel,
			pSetup->fChorusSpeed,
			pSetup->fChorusDepth,
			pSetup->iChorusType);
	}

	// All is right.
	appendMessages(sPrefix + tr("Synthesizer engine started."));

	return true;
}


// Stop the fluidsynth clone.
void qsynthMainForm::stopEngine ( qsynthEngine *pEngine )
{
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return;

	// Only if there's a legal audio driver...
	if (pEngine->pAudioDriver) {
		// Before all else save current engine panel settings...
		if (pEngine == currentEngine())
			savePanelSettings(pEngine);
		// Make those settings persist over...
		m_pOptions->saveSetup(pSetup, pEngine->isDefault()
			? QString::null : pEngine->name());
	}

	// Flush anything that maybe pending...
	flushStdoutBuffer();

	const QString sPrefix  = pEngine->name() + ": ";
	const QString sElipsis = "...";

#ifdef CONFIG_FLUID_SERVER
	// Destroy server.
	if (pEngine->pServer) {
	//  Server join is not necessary, causes seg fault when multiple engines.
	//  appendMessages(sPrefix + tr("Waiting for server to terminate") + sElipsis);
	//  ::fluid_server_join(pEngine->pServer);
		appendMessages(sPrefix + tr("Destroying server") + sElipsis);
		::delete_fluid_server(pEngine->pServer);
		pEngine->pServer = NULL;
	}
#endif

	// Destroy player.
	if (pEngine->pPlayer) {
		appendMessages(sPrefix + tr("Stopping MIDI player") + sElipsis);
		::fluid_player_stop(pEngine->pPlayer);
		appendMessages(sPrefix + tr("Waiting for MIDI player to terminate") + sElipsis);
		::fluid_player_join(pEngine->pPlayer);
		appendMessages(sPrefix + tr("Destroying MIDI player") + sElipsis);
		::delete_fluid_player(pEngine->pPlayer);
		pEngine->pPlayer = NULL;
	}

	// Destroy MIDI router.
	if (pEngine->pMidiRouter) {
		if (pEngine->pMidiDriver) {
			appendMessages(sPrefix + tr("Destroying MIDI driver") + sElipsis);
			::delete_fluid_midi_driver(pEngine->pMidiDriver);
			pEngine->pMidiDriver = NULL;
		}
		appendMessages(sPrefix + tr("Destroying MIDI router") + sElipsis);
		::delete_fluid_midi_router(pEngine->pMidiRouter);
		pEngine->pMidiRouter = NULL;
	}

	// Destroy audio driver.
	if (pEngine->pAudioDriver) {
		appendMessages(sPrefix + tr("Destroying audio driver") + sElipsis);
		::delete_fluid_audio_driver(pEngine->pAudioDriver);
		pEngine->pAudioDriver = NULL;
		pEngine->bMeterEnabled = false;
	}

	// Unload soundfonts from actual synth stack...
	int cSoundFonts = ::fluid_synth_sfcount(pEngine->pSynth);
	for (int i = 0; i < cSoundFonts; i++) {
		fluid_sfont_t *pSoundFont
			= ::fluid_synth_get_sfont(pEngine->pSynth, i);
		if (pSoundFont) {
			int iSFID = pSoundFont->id;
			const QString sName = pSoundFont->get_name(pSoundFont);
			appendMessagesColor(sPrefix +
				tr("Unloading soundfont: \"%1\" (SFID=%2)")
				.arg(sName).arg(iSFID) + sElipsis, "#999933");
			if (::fluid_synth_sfunload(pEngine->pSynth, iSFID, 0) < 0)
				appendMessagesError(sPrefix +
					tr("Failed to unload the soundfont: \"%1\".")
					.arg(sName));
		}
	}

	// And finally, destroy the synthesizer engine.
	if (pEngine->pSynth) {
		appendMessages(sPrefix + tr("Destroying synthesizer engine") + sElipsis);
		::delete_fluid_synth(pEngine->pSynth);
		pEngine->pSynth = NULL;
		// We're done.
		appendMessages(sPrefix + tr("Synthesizer engine terminated."));
	}

#ifdef QSYNTH_CUSTOM_LOADER
	// Remove engine from custom loader list, if any...
	qsynthEngineNode *pNode = g_pEngineList;
	while (pNode) {
		if (pNode->pEngine == pEngine ) {
			if (pNode->pPrev)
				(pNode->pPrev)->pNext = pNode->pNext;
			else
				g_pEngineList = pNode->pNext;
			if (pNode->pNext)
				(pNode->pNext)->pPrev = pNode->pPrev;
			delete pNode;
			break;
		}
		pNode = pNode->pNext;
	}
#endif

	// Show up our efforts, if we're currently selected :p
	if (pEngine == currentEngine()) {
		resetChannelsForm(pEngine, true);
		stabilizeForm();
	}

	// Wait a litle bit before continue...
	QTime t;
	t.start();
	while (t.elapsed() < QSYNTH_DELAY_MSECS)
		QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}


// Start all synth engines (schedule).
void qsynthMainForm::startAllEngines (void)
{
	m_iTimerDelay = 0;
}


// Restart all synth engines.
void qsynthMainForm::restartAllEngines (void)
{
	// Always prompt user...
	bool  bRestart = (QMessageBox::warning(this,
			QSYNTH_TITLE ": " + tr("Warning"),
			tr("New settings will be effective after\n"
			"restarting all fluidsynth engines.") + "\n\n" +
			tr("Please note that this operation may cause\n"
			"temporary MIDI and Audio disruption.") + "\n\n" +
			tr("Do you want to restart all engines now?"),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);

	// Just restart every engine out there...
	if (bRestart) {
		// Restarting means stopping an engine...
		for (int iTab = 0; iTab < m_ui.TabBar->count(); iTab++)
			stopEngine(m_ui.TabBar->engine(iTab));
		// Must make this one grayed out for a while...
		m_ui.RestartPushButton->setEnabled(false);
		// And making room for immediate restart...
		startAllEngines();
	}
}


// Start the fluidsynth clone, based on given settings.
void qsynthMainForm::restartEngine ( qsynthEngine *pEngine )
{
	bool bRestart = true;

	// If currently running, prompt user...
	if (pEngine && pEngine->pSynth) {
		bRestart = (QMessageBox::warning(this,
			QSYNTH_TITLE ": " + tr("Warning"),
			tr("New settings will be effective after\n"
			"restarting the fluidsynth engine:") + "\n\n" +
			pEngine->name() + "\n\n" +
			tr("Please note that this operation may cause\n"
			"temporary MIDI and Audio disruption.") + "\n\n" +
			tr("Do you want to restart the engine now?"),
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
	}

	// If allowed, just restart the engine...
	if (bRestart) {
		// Must make current one grayed out for a while...
		if (pEngine == currentEngine())
			m_ui.RestartPushButton->setEnabled(false);
		// Restarting means stopping the engine...
		stopEngine(pEngine);
		// And making room for immediate restart...
		startAllEngines();
	}
}


// Engine reset (all channels program reset).
void qsynthMainForm::resetEngine ( qsynthEngine *pEngine )
{
	if (pEngine && pEngine->pSynth) {
		appendMessagesColor(pEngine->name() + ": fluid_synth_program_reset()", "#996666");
		::fluid_synth_program_reset(pEngine->pSynth);
	}
}


// Engine gain settings.
void qsynthMainForm::setEngineGain ( qsynthEngine *pEngine, float fGain )
{
	appendMessagesColor(pEngine->name() + ": fluid_synth_set_gain(" + QString::number(fGain) + ")", "#6699cc");

	::fluid_synth_set_gain(pEngine->pSynth, fGain);
}


// Engine reverb settings.
void qsynthMainForm::setEngineReverbOn ( qsynthEngine *pEngine, bool bActive )
{
	appendMessagesColor(pEngine->name() + ": fluid_synth_set_reverb_on(" + QString::number((int) bActive) + ")", "#99cc33");

	::fluid_synth_set_reverb_on(pEngine->pSynth, (int) bActive);
}

void qsynthMainForm::setEngineReverb ( qsynthEngine *pEngine, float fRoom, float fDamp, float fWidth, float fLevel )
{
	appendMessagesColor(pEngine->name() + ": fluid_synth_set_reverb("
		+ QString::number(fRoom)  + ","
		+ QString::number(fDamp)  + ","
		+ QString::number(fWidth) + ","
		+ QString::number(fLevel) + ")", "#99cc66");

	::fluid_synth_set_reverb(pEngine->pSynth, fRoom, fDamp, fWidth, fLevel);
}


// Engine chorus settings.
void qsynthMainForm::setEngineChorusOn ( qsynthEngine *pEngine, bool bActive )
{
	appendMessagesColor(pEngine->name() + ": fluid_synth_set_chorus_on(" + QString::number((int) bActive) + ")", "#cc9933");

	::fluid_synth_set_chorus_on(pEngine->pSynth, (int) bActive);
}

void qsynthMainForm::setEngineChorus ( qsynthEngine *pEngine, int iNr, float fLevel, float fSpeed, float fDepth, int iType )
{
	appendMessagesColor(pEngine->name() + ": fluid_synth_set_chorus("
		+ QString::number(iNr)    + ","
		+ QString::number(fLevel) + ","
		+ QString::number(fSpeed) + ","
		+ QString::number(fDepth) + ","
		+ QString::number(iType)  + ")", "#cc9966");

	::fluid_synth_set_chorus(pEngine->pSynth, iNr, fLevel, fSpeed, fDepth, iType);
}


// Front panel state load routine.
void qsynthMainForm::loadPanelSettings ( qsynthEngine *pEngine, bool bUpdate )
{
	if (pEngine == NULL)
		return;
//  if (pEngine->pSynth == NULL || pEngine->pAudioDriver == NULL)
//      return;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return;

	// Reset change flags.
	m_iGainChanged   = 0;
	m_iReverbChanged = 0;
	m_iChorusChanged = 0;

	// Avoid update races: set update counters > 0)...
	m_iGainUpdated   = 1;
	m_iReverbUpdated = 1;
	m_iChorusUpdated = 1;

	m_ui.GainDial->setDefaultValue(qsynth_set_range_value(
		m_ui.GainDial, QSYNTH_MASTER_GAIN_SCALE, pSetup->fGain));

	m_ui.ReverbActiveCheckBox->setChecked(pSetup->bReverbActive);
	m_ui.ReverbRoomDial->setDefaultValue(qsynth_set_range_value(
		m_ui.ReverbRoomDial, QSYNTH_REVERB_ROOM_SCALE, pSetup->fReverbRoom));
	m_ui.ReverbDampDial->setDefaultValue(qsynth_set_range_value(
		m_ui.ReverbDampDial, QSYNTH_REVERB_DAMP_SCALE, pSetup->fReverbDamp));
	m_ui.ReverbWidthDial->setDefaultValue(qsynth_set_range_value(
		m_ui.ReverbWidthDial, QSYNTH_REVERB_WIDTH_SCALE, pSetup->fReverbWidth));
	m_ui.ReverbLevelDial->setDefaultValue(qsynth_set_range_value(
		m_ui.ReverbLevelDial, QSYNTH_REVERB_LEVEL_SCALE, pSetup->fReverbLevel));

	m_ui.ChorusActiveCheckBox->setChecked(pSetup->bChorusActive);
	m_ui.ChorusNrDial->setDefaultValue(pSetup->iChorusNr);
	m_ui.ChorusNrDial->setValue(pSetup->iChorusNr);
	m_ui.ChorusLevelDial->setDefaultValue(qsynth_set_range_value(
		m_ui.ChorusLevelDial, QSYNTH_CHORUS_LEVEL_SCALE, pSetup->fChorusLevel));
	m_ui.ChorusSpeedDial->setDefaultValue(qsynth_set_range_value(
		m_ui.ChorusSpeedDial, QSYNTH_CHORUS_SPEED_SCALE, pSetup->fChorusSpeed));
	m_ui.ChorusDepthDial->setDefaultValue(qsynth_set_range_value(
		m_ui.ChorusDepthDial, QSYNTH_CHORUS_DEPTH_SCALE, pSetup->fChorusDepth));
	m_ui.ChorusTypeComboBox->setCurrentIndex(pSetup->iChorusType);

	// Make them dirty.
	if (bUpdate) {
		m_iGainChanged++;
		m_iReverbChanged++;
		m_iChorusChanged++;
	}

	// Let them get updated, possibly on next tick.
	m_iGainUpdated   = 0;
	m_iReverbUpdated = 0;
	m_iChorusUpdated = 0;
}


// Front panel state save routine.
void qsynthMainForm::savePanelSettings ( qsynthEngine *pEngine )
{
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL || pEngine->pAudioDriver == NULL)
		return;

	qsynthSetup *pSetup = pEngine->setup();
	if (pSetup == NULL)
		return;

	pSetup->fGain = qsynth_get_range_value(
		m_ui.GainSpinBox, QSYNTH_MASTER_GAIN_SCALE);

	pSetup->bReverbActive = m_ui.ReverbActiveCheckBox->isChecked();
	pSetup->fReverbRoom   = qsynth_get_range_value(
		m_ui.ReverbRoomSpinBox, QSYNTH_REVERB_ROOM_SCALE);
	pSetup->fReverbDamp   = qsynth_get_range_value(
		m_ui.ReverbDampSpinBox, QSYNTH_REVERB_DAMP_SCALE);
	pSetup->fReverbWidth  = qsynth_get_range_value(
		m_ui.ReverbWidthSpinBox, QSYNTH_REVERB_WIDTH_SCALE);
	pSetup->fReverbLevel  = qsynth_get_range_value(
		m_ui.ReverbLevelSpinBox, QSYNTH_REVERB_LEVEL_SCALE);

	pSetup->bChorusActive = m_ui.ChorusActiveCheckBox->isChecked();
	pSetup->iChorusNr     = m_ui.ChorusNrSpinBox->value();
	pSetup->fChorusLevel  = qsynth_get_range_value(
		m_ui.ChorusLevelSpinBox, QSYNTH_CHORUS_LEVEL_SCALE);
	pSetup->fChorusSpeed  = qsynth_get_range_value(
		m_ui.ChorusSpeedSpinBox, QSYNTH_CHORUS_SPEED_SCALE);
	pSetup->fChorusDepth  = qsynth_get_range_value(
		m_ui.ChorusDepthSpinBox, QSYNTH_CHORUS_DEPTH_SCALE);
	pSetup->iChorusType   = m_ui.ChorusTypeComboBox->currentIndex();
}


// Complete refresh of the floating channels form.
void qsynthMainForm::resetChannelsForm ( qsynthEngine *pEngine, bool bPreset )
{
	if (m_pChannelsForm == NULL)
		return;

	// Setup the channels view window.
	m_pChannelsForm->setup(m_pOptions, pEngine, bPreset);

	// Reset the channel event state flaggers.
	if (g_pMidiChannels)
		delete [] g_pMidiChannels;
	g_pMidiChannels = NULL;
	g_iMidiChannels = 0;
	// Prepare the new channel event state flaggers.
	if (pEngine && pEngine->pSynth)
		g_iMidiChannels = ::fluid_synth_count_midi_channels(pEngine->pSynth);
	if (g_iMidiChannels > 0)
		g_pMidiChannels = new qsynth_midi_channel [g_iMidiChannels];
	if (g_pMidiChannels) {
		for (int iChan = 0; iChan < g_iMidiChannels; iChan++) {
			g_pMidiChannels[iChan].iEvent  = 0;
			g_pMidiChannels[iChan].iState  = 0;
			g_pMidiChannels[iChan].iChange = 0;
		}
	}
}


// Increment reverb change flag.
void qsynthMainForm::reverbActivate ( bool bActive )
{
	if (m_iReverbUpdated > 0)
		return;

	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;

	setEngineReverbOn(pEngine, bActive);

	if (bActive)
		refreshReverb();

	stabilizeForm();
}


// Increment chorus change flag.
void qsynthMainForm::chorusActivate ( bool bActive )
{
	if (m_iChorusUpdated > 0)
		return;

	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;

	setEngineChorusOn(pEngine, bActive);

	if (bActive)
		refreshChorus();

	stabilizeForm();
}


// Increment gain change flag.
void qsynthMainForm::gainChanged (int)
{
	if (m_iGainUpdated == 0)
		m_iGainChanged++;
}


// Increment reverb change flag.
void qsynthMainForm::reverbChanged (int)
{
	if (m_iReverbUpdated == 0)
		m_iReverbChanged++;
}


// Increment chorus change flag.
void qsynthMainForm::chorusChanged (int)
{
	if (m_iChorusUpdated == 0)
		m_iChorusChanged++;
}


// Update gain state.
void qsynthMainForm::updateGain (void)
{
	if (m_iGainUpdated > 0)
		return;

	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;
	m_iGainUpdated++;

	float fGain = qsynth_get_range_value(
		m_ui.GainSpinBox, QSYNTH_MASTER_GAIN_SCALE);

	setEngineGain(pEngine, fGain);
	refreshGain();

	m_iGainUpdated--;
	m_iGainChanged = 0;
}


// Update reverb state.
void qsynthMainForm::updateReverb (void)
{
	if (m_iReverbUpdated > 0)
		return;

	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;
	m_iReverbUpdated++;

	double fReverbRoom  = qsynth_get_range_value(
		m_ui.ReverbRoomSpinBox, QSYNTH_REVERB_ROOM_SCALE);
	double fReverbDamp  = qsynth_get_range_value(
		m_ui.ReverbDampSpinBox, QSYNTH_REVERB_DAMP_SCALE);
	double fReverbWidth = qsynth_get_range_value(
		m_ui.ReverbWidthSpinBox, QSYNTH_REVERB_WIDTH_SCALE);
	double fReverbLevel = qsynth_get_range_value(
		m_ui.ReverbLevelSpinBox, QSYNTH_REVERB_LEVEL_SCALE);

	setEngineReverb(pEngine, fReverbRoom, fReverbDamp, fReverbWidth, fReverbLevel);
	refreshReverb();

	m_iReverbUpdated--;
	m_iReverbChanged = 0;
}


// Update chorus state.
void qsynthMainForm::updateChorus (void)
{
	if (m_iChorusUpdated > 0)
		return;

	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;
	m_iChorusUpdated++;

	int    iChorusNr    = m_ui.ChorusNrSpinBox->value();
	double fChorusLevel = qsynth_get_range_value(
		m_ui.ChorusLevelSpinBox, QSYNTH_CHORUS_LEVEL_SCALE);
	double fChorusSpeed = qsynth_get_range_value(
		m_ui.ChorusSpeedSpinBox, QSYNTH_CHORUS_SPEED_SCALE);
	double fChorusDepth = qsynth_get_range_value(
		m_ui.ChorusDepthSpinBox, QSYNTH_CHORUS_DEPTH_SCALE);
	int    iChorusType  = m_ui.ChorusTypeComboBox->currentIndex();

	setEngineChorus(pEngine, iChorusNr, fChorusLevel, fChorusSpeed, fChorusDepth, iChorusType);
	refreshChorus();

	m_iChorusUpdated--;
	m_iChorusChanged = 0;
}


// Refresh gain panel controls.
void qsynthMainForm::refreshGain (void)
{
	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;

	float fGain = ::fluid_synth_get_gain(pEngine->pSynth);

	qsynth_set_range_value(
		m_ui.GainDial, QSYNTH_MASTER_GAIN_SCALE, fGain);
}


// Refresh reverb panel controls.
void qsynthMainForm::refreshReverb (void)
{
	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;

	double fReverbRoom  = ::fluid_synth_get_reverb_roomsize(pEngine->pSynth);
	double fReverbDamp  = ::fluid_synth_get_reverb_damp(pEngine->pSynth);
	double fReverbWidth = ::fluid_synth_get_reverb_width(pEngine->pSynth);
	double fReverbLevel = ::fluid_synth_get_reverb_level(pEngine->pSynth);

	qsynth_set_range_value(
		m_ui.ReverbRoomDial, QSYNTH_REVERB_ROOM_SCALE, fReverbRoom);
	qsynth_set_range_value(
		m_ui.ReverbDampDial, QSYNTH_REVERB_DAMP_SCALE, fReverbDamp);
	qsynth_set_range_value(
		m_ui.ReverbWidthDial, QSYNTH_REVERB_WIDTH_SCALE, fReverbWidth);
	qsynth_set_range_value(
		m_ui.ReverbLevelDial, QSYNTH_REVERB_LEVEL_SCALE, fReverbLevel);
}


// Refresh chorus panel controls.
void qsynthMainForm::refreshChorus (void)
{
	qsynthEngine *pEngine = currentEngine();
	if (pEngine == NULL)
		return;
	if (pEngine->pSynth == NULL)
		return;

	int    iChorusNr    = ::fluid_synth_get_chorus_nr(pEngine->pSynth);
	double fChorusLevel = ::fluid_synth_get_chorus_level(pEngine->pSynth);
	double fChorusSpeed = ::fluid_synth_get_chorus_speed_Hz(pEngine->pSynth);
	double fChorusDepth = ::fluid_synth_get_chorus_depth_ms(pEngine->pSynth);
	int    iChorusType  = ::fluid_synth_get_chorus_type(pEngine->pSynth);

	m_ui.ChorusNrDial->setValue(iChorusNr);
	qsynth_set_range_value(
		m_ui.ChorusLevelDial, QSYNTH_CHORUS_LEVEL_SCALE, fChorusLevel);
	qsynth_set_range_value(
		m_ui.ChorusSpeedDial, QSYNTH_CHORUS_SPEED_SCALE, fChorusSpeed);
	qsynth_set_range_value(
		m_ui.ChorusDepthDial, QSYNTH_CHORUS_DEPTH_SCALE, fChorusDepth);
	m_ui.ChorusTypeComboBox->setCurrentIndex(iChorusType);
}


// Select the current default preset name from context menu.
void qsynthMainForm::activateEnginesMenu ( QAction *pAction )
{
	int iTab = pAction->data().toInt();
	if (iTab < 0)
		return;

	m_ui.TabBar->setCurrentIndex(iTab);
}


// Close main form slot.
void qsynthMainForm::quitMainForm (void)
{
#ifdef CONFIG_SYSTEM_TRAY
	// Flag that we're quitting explicitly.
	m_bQuitForce = true;
#endif
	// And then, do the closing dance.
	close();
}


// Context menu event handler.
void qsynthMainForm::contextMenuEvent( QContextMenuEvent *pEvent )
{
	// We'll just show up the usual system tray menu.
	systemTrayContextMenu(pEvent->globalPos());
}


void qsynthMainForm::updateKnobs()
{
	if (m_pOptions == NULL)
		return;
	qsynthKnob::DialMode mode = qsynthKnob::DialMode(m_pOptions->iKnobMotion);
	KnobStyle style = KnobStyle(m_pOptions->iKnobStyle);
	if (m_pKnobStyle) {
		delete m_pKnobStyle;
	}
	switch (style) {
	case Classic:
		m_pKnobStyle = new qsynthDialClassicStyle();
		break;
	case Vokimon:
		m_pKnobStyle = new qsynthDialVokiStyle();
		break;
	case Peppino:
		m_pKnobStyle = new qsynthDialPeppinoStyle();
		break;
	case Skulpture:
	    m_pKnobStyle = new qsynthDialSkulptureStyle();
	    break;
	default:
		m_pKnobStyle = NULL;
		break;
	}
	QList<qsynthKnob *> allKnobs = findChildren<qsynthKnob *>();
	foreach(qsynthKnob* knob, allKnobs) {
		knob->setStyle(m_pKnobStyle);
		knob->setDialMode(mode);
	}
}


// Session (desktop) shutdown signal handler.
void qsynthMainForm::setQuitForce ( bool bQuitForce )
{
	m_bQuitForce = bQuitForce;
}

bool qsynthMainForm::isQuitForce (void) const
{
	return m_bQuitForce;
}


// end of qsynthMainForm.cpp
