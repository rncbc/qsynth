// qsynthMainForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2003-2004, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include <qapplication.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qregexp.h>
#include <qtimer.h>

#include <math.h>

#include "config.h"

#include "qsynthAbout.h"

// Timer constant stuff.
#define QSYNTH_TIMER_MSECS  200
#define QSYNTH_DELAY_MSECS  200

// Scale factors.
#define QSYNTH_GAIN_SCALE   10.0
#define QSYNTH_REVERB_SCALE 100.0
#define QSYNTH_CHORUS_SCALE 10.0

// Notification pipe descriptors
#define QSYNTH_FDNIL     -1
#define QSYNTH_FDREAD     0
#define QSYNTH_FDWRITE    1

static int g_fdStdout[2] = { QSYNTH_FDNIL, QSYNTH_FDNIL };

//-------------------------------------------------------------------------
// Needed for server mode.
static fluid_cmd_handler_t* qsynth_newclient ( void* data, char* )
{
    return ::new_fluid_cmd_handler((fluid_synth_t*) data);
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

static int g_iMidiEvent = 0;
static int g_iMidiState = 0;

struct qsynth_midi_channel
{
    int iEvent;     // Event occurrence accumulator.
    int iState;     // Activity state tracker.
    int iChange;    // Change activity accumulator.
};

static int                  g_iMidiChannels = 0;
static qsynth_midi_channel *g_pMidiChannels = NULL;

static void qsynth_midi_event ( fluid_midi_event_t *pMidiEvent )
{
    g_iMidiEvent++;
    
    if (g_pMidiChannels) {
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

static int qsynth_dump_postrouter (void *pvData, fluid_midi_event_t *pMidiEvent)
{
    qsynth_midi_event(pMidiEvent);
    return ::fluid_midi_dump_postrouter(pvData, pMidiEvent);
}

static int qsynth_handle_midi_event (void *pvData, fluid_midi_event_t *pMidiEvent)
{
    qsynth_midi_event(pMidiEvent);
    return ::fluid_synth_handle_midi_event(pvData, pMidiEvent);
}


//-------------------------------------------------------------------------
// Scaling & Clipping helpers.

static int qsynth_iscale_clip ( double fScale, double fValue )
{
#ifdef CONFIG_ROUND
    int iValue = (int) ::round(fScale * fValue);
#else
    double fIPart = 0.0;
    double fFPart = ::modf(fScale * fValue, &fIPart);
    int iValue = (int) fIPart;
    if (fFPart >= +0.5)
        iValue++;
    else
    if (fFPart <= -0.5)
        iValue--;
#endif

    if (iValue < 0)
        iValue = 0;
    else if (iValue > 100)
        iValue = 100;
        
    return iValue;
}

static double qsynth_fscale_clip ( int iValue, double fScale )
{
    double fValue = ((double) iValue / fScale);

    const double fMax = (100.0 / fScale);
    if (fValue < 0.0)
        fValue = 0.0;
    else if (fValue > fMax)
        fValue = fMax;
        
    return fValue;
}


//-------------------------------------------------------------------------
// qsynthMainForm -- Main window form implementation.

// Kind of constructor.
void qsynthMainForm::init (void)
{
    m_pSetup = NULL;
    
    m_iTimerDelay = 0;

    m_pSynth        = NULL;
    m_pAudioDriver  = NULL;
    m_pMidiRouter   = NULL;
    m_pMidiDriver   = NULL;
    m_pPlayer       = NULL;
    m_pServer       = NULL;

    m_pStdoutNotifier = NULL;

    m_iGainChanged   = 0;
    m_iReverbChanged = 0;
    m_iChorusChanged = 0;

    m_iGainUpdated   = 0;
    m_iReverbUpdated = 0;
    m_iChorusUpdated = 0;

    m_pXpmLedOn  = new QPixmap(QPixmap::fromMimeSource("ledon1.png"));
    m_pXpmLedOff = new QPixmap(QPixmap::fromMimeSource("ledoff1.png"));

    // All forms are to be created right now.
    m_pMessagesForm = new qsynthMessagesForm(this, 0, Qt::WType_TopLevel | Qt::WStyle_Tool);
    m_pChannelsForm = new qsynthChannelsForm(this, 0, Qt::WType_TopLevel | Qt::WStyle_Tool);
}


// Kind of destructor.
void qsynthMainForm::destroy (void)
{
    // Stop th press!
    stopSynth();
    m_pSetup = NULL;

    // Delete pixmaps.
    delete m_pXpmLedOn;
    delete m_pXpmLedOff;
    
    // Finally drop any popup widgets around...
    delete m_pMessagesForm;
    delete m_pChannelsForm;
}


// Make and set a proper setup step.
void qsynthMainForm::setup ( qsynthSetup *pSetup )
{
    // Finally, fix settings descriptor
    // and stabilize the form.
    m_pSetup = pSetup;

    // Try to restore old window positioning.
    m_pSetup->loadWidgetGeometry(this);
    // And for the whole widget gallore...
    m_pSetup->loadWidgetGeometry(m_pMessagesForm);
    m_pSetup->loadWidgetGeometry(m_pChannelsForm);

    // Set defaults...
    updateMessagesFont();

    // Check if we can redirect our own stdout/stderr...
    if (m_pSetup->bStdoutCapture && ::pipe(g_fdStdout) == 0) {
        ::dup2(g_fdStdout[QSYNTH_FDWRITE], STDOUT_FILENO);
        ::dup2(g_fdStdout[QSYNTH_FDWRITE], STDERR_FILENO);
        m_pStdoutNotifier = new QSocketNotifier(g_fdStdout[QSYNTH_FDREAD], QSocketNotifier::Read, this);
        QObject::connect(m_pStdoutNotifier, SIGNAL(activated(int)), this, SLOT(stdoutNotifySlot(int)));
    }

    // Final startup stabilization...
    stabilizeForm();

    // Register the initial timer slot.
    QTimer::singleShot(QSYNTH_TIMER_MSECS, this, SLOT(timerSlot()));
}


// Window close event handlers.
bool qsynthMainForm::queryClose (void)
{
    bool bQueryClose = true;

    // Now's the time?
    if (m_pSetup) {
        // Dow we quit right away?
        if (m_pSynth && m_pAudioDriver && m_pSetup->bQueryClose) {
            bQueryClose = (QMessageBox::warning(this, tr("Warning"),
                tr("qsynth is about to terminate.") + "\n\n" +
                tr("Are you sure?"),
                tr("OK"), tr("Cancel")) == 0);
        }
        // Some windows default fonts is here on demeand too.
        if (bQueryClose && m_pMessagesForm)
            m_pSetup->sMessagesFont = m_pMessagesForm->messagesFont().toString();
        // Try to save current positioning.
        if (bQueryClose) {
            m_pSetup->saveWidgetGeometry(m_pChannelsForm);
            m_pSetup->saveWidgetGeometry(m_pMessagesForm);
            m_pSetup->saveWidgetGeometry(this);
            // Close popup widgets.
            m_pMessagesForm->close();
            m_pChannelsForm->close();        
        }
    }
    return bQueryClose;
}


void qsynthMainForm::closeEvent ( QCloseEvent *pCloseEvent )
{
    if (queryClose())
        pCloseEvent->accept();
    else
        pCloseEvent->ignore();
}


// Add dropped files to playlist or soundfont stack.
void qsynthMainForm::playLoadFiles ( const QStringList& files, bool bSetup )
{
    if (m_pSetup == NULL || m_pSynth == NULL)
        return;

    // Add each list item to Soundfont stack or MIDI player playlist...
    const QString sElipsis = "...";
    int   iSoundfonts = 0;
    int   iMidiFiles  = 0;
    for (QStringList::ConstIterator iter = files.begin(); iter != files.end(); iter++) {
        char *pszFilename = (char *) (*iter).latin1();
        // Is it a soundfont file...
        if (::fluid_is_soundfont(pszFilename)) {
            if (bSetup || m_pSetup->soundfonts.find(*iter) == m_pSetup->soundfonts.end()) {
                appendMessagesColor(tr("Loading soundfont") + ": \"" + *iter + "\"" + sElipsis, "#999933");
                if (::fluid_synth_sfload(m_pSynth, pszFilename, 1) >= 0) {
                    iSoundfonts++;
                    if (!bSetup)
                        m_pSetup->soundfonts.append(*iter);
                }
                else appendMessagesError(tr("Failed to load the soundfont") + ": \"" + *iter + "\".");
            }
        }  // Or is it a bare midifile?
        else if (::fluid_is_midifile(pszFilename) && m_pPlayer) {
            appendMessagesColor(tr("Playing MIDI file") + ": \"" + *iter + "\"" + sElipsis, "#99cc66");
            if (::fluid_player_add(m_pPlayer, pszFilename) >= 0)
                iMidiFiles++;
            else
                appendMessagesError(tr("Failed to play MIDI file") + ": \"" + *iter + "\".");
        }
    }
    
    // Reset all presets, if applicable...
    if (!bSetup && iSoundfonts > 0)
        programReset();
    // Start playing, if any...
    if (m_pPlayer && iMidiFiles > 0)
        ::fluid_player_play(m_pPlayer);
}


// Drag'n'drop midi player feature handlers.
bool qsynthMainForm::decodeDragFiles ( const QMimeSource *pEvent, QStringList& files )
{
    bool bDecode = false;
    
    if (QTextDrag::canDecode(pEvent)) {
        QString sText;
        bDecode = QTextDrag::decode(pEvent, sText);
        if (bDecode) {
            files = QStringList::split("\n", sText);
            for (QStringList::Iterator iter = files.begin(); iter != files.end(); iter++)
                *iter = (*iter).stripWhiteSpace().replace(QRegExp("^file:"), "");
        }
    }
    
    return bDecode;
}

void qsynthMainForm::dragEnterEvent ( QDragEnterEvent* pDragEnterEvent )
{
    bool bAccept = false;
    
    if (m_pPlayer && QTextDrag::canDecode(pDragEnterEvent)) {
        QStringList files;
        if (decodeDragFiles(pDragEnterEvent, files)) {
            for (QStringList::Iterator iter = files.begin(); iter != files.end() && !bAccept; iter++) {
                char *pszFilename = (char *) (*iter).latin1();
                if (::fluid_is_midifile(pszFilename) || ::fluid_is_soundfont(pszFilename))
                    bAccept = true;
            }
        }
    }
    
    pDragEnterEvent->accept(bAccept);
}

void qsynthMainForm::dropEvent ( QDropEvent* pDropEvent )
{
    QStringList files;
    
    if (decodeDragFiles(pDropEvent, files))
        playLoadFiles(files, false);
}


// Own stdout/stderr socket notifier slot.
void qsynthMainForm::stdoutNotifySlot ( int fd )
{
    char achBuffer[1024];
    int  cchBuffer = ::read(fd, achBuffer, sizeof(achBuffer) - 1);
    if (cchBuffer > 0) {
        achBuffer[cchBuffer] = (char) 0;
        appendStdoutBuffer(achBuffer);
    }
}


// Stdout buffer handler -- now splitted by complete new-lines...
void qsynthMainForm::appendStdoutBuffer ( const QString& s )
{
    m_sStdoutBuffer.append(s);

    int iLength = m_sStdoutBuffer.findRev('\n') + 1;
    if (iLength > 0) {
        QString sTemp = m_sStdoutBuffer.left(iLength);
        m_sStdoutBuffer.remove(0, iLength);
        QStringList list = QStringList::split('\n', sTemp, true);
        for (QStringList::Iterator iter = list.begin(); iter != list.end(); iter++)
            appendMessagesText(*iter);
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
    appendMessagesColor(s, "#ff0000");

    QMessageBox::critical(this, tr("Error"), s, tr("Cancel"));
}


// Force update of the messages font.
void qsynthMainForm::updateMessagesFont (void)
{
    if (m_pSetup == NULL)
        return;

    if (m_pMessagesForm && !m_pSetup->sMessagesFont.isEmpty()) {
        QFont font;
        if (font.fromString(m_pSetup->sMessagesFont))
            m_pMessagesForm->setMessagesFont(font);
    }
}


// Stabilize current form toggle buttons that may be astray.
void qsynthMainForm::stabilizeForm (void)
{
    if (m_pSynth) {
        GainGroupBox->setEnabled(true);
        ReverbGroupBox->setEnabled(true);
        bool bReverbActive = ReverbActiveCheckBox->isChecked();
        ReverbRoomTextLabel->setEnabled(bReverbActive);
        ReverbDampTextLabel->setEnabled(bReverbActive);
        ReverbWidthTextLabel->setEnabled(bReverbActive);
        ReverbLevelTextLabel->setEnabled(bReverbActive);
        ReverbRoomDial->setEnabled(bReverbActive);
        ReverbDampDial->setEnabled(bReverbActive);
        ReverbWidthDial->setEnabled(bReverbActive);
        ReverbLevelDial->setEnabled(bReverbActive);
        ReverbRoomSpinBox->setEnabled(bReverbActive);
        ReverbDampSpinBox->setEnabled(bReverbActive);
        ReverbWidthSpinBox->setEnabled(bReverbActive);
        ReverbLevelSpinBox->setEnabled(bReverbActive);
        bool bChorusActive = ChorusActiveCheckBox->isChecked();
        ChorusNrTextLabel->setEnabled(bChorusActive);
        ChorusLevelTextLabel->setEnabled(bChorusActive);
        ChorusSpeedTextLabel->setEnabled(bChorusActive);
        ChorusDepthTextLabel->setEnabled(bChorusActive);
        ChorusTypeTextLabel->setEnabled(bChorusActive);
        ChorusNrDial->setEnabled(bChorusActive);
        ChorusLevelDial->setEnabled(bChorusActive);
        ChorusSpeedDial->setEnabled(bChorusActive);
        ChorusDepthDial->setEnabled(bChorusActive);
        ChorusNrSpinBox->setEnabled(bChorusActive);
        ChorusLevelSpinBox->setEnabled(bChorusActive);
        ChorusSpeedSpinBox->setEnabled(bChorusActive);
        ChorusDepthSpinBox->setEnabled(bChorusActive);
        ChorusTypeComboBox->setEnabled(bChorusActive);
        ChorusGroupBox->setEnabled(true);
        ProgramResetPushButton->setEnabled(true);
        SystemResetPushButton->setEnabled(true);
    } else {
        GainGroupBox->setEnabled(false);
        ReverbGroupBox->setEnabled(false);
        ChorusGroupBox->setEnabled(false);
        ProgramResetPushButton->setEnabled(false);
        SystemResetPushButton->setEnabled(false);
    }
    RestartPushButton->setEnabled(true);

    bool bMidiIn = (m_pSynth && m_pSetup && m_pSetup->bMidiIn);
    MidiEventPixmapLabel->setEnabled(bMidiIn);
    MidiEventTextLabel->setEnabled(bMidiIn);
    ChannelsPushButton->setEnabled(bMidiIn);

    MessagesPushButton->setOn(m_pMessagesForm && m_pMessagesForm->isVisible());
    ChannelsPushButton->setOn(m_pChannelsForm && m_pChannelsForm->isVisible());
}


// Program reset command slot (all channels).
void qsynthMainForm::programReset (void)
{
    if (m_pSynth) {
        appendMessagesColor("fluid_synth_program_reset", "#996666");
        ::fluid_synth_program_reset(m_pSynth);
        if (m_pChannelsForm)
            m_pChannelsForm->resetAllChannels();
    }
    stabilizeForm();
}


// System reset command slot.
void qsynthMainForm::systemReset (void)
{
    if (m_pSynth) {
#ifdef CONFIG_FLUID_RESET
        appendMessagesColor("fluid_synth_system_reset", "#993366");
        ::fluid_synth_system_reset(m_pSynth);
#else
        appendMessagesColor("fluid_synth_program_reset", "#996666");
        ::fluid_synth_program_reset(m_pSynth);
#endif
        if (m_pChannelsForm)
            m_pChannelsForm->resetAllChannels();
    }
    stabilizeForm();
}


// Complete engine restart.
void qsynthMainForm::promptRestart (void)
{
    bool bRestart = true;

    // If currently running, prompt user...
    if (m_pSynth) {
        bRestart = (QMessageBox::warning(this, tr("Warning"),
            tr("New settings will be effective after\n"
               "restarting the fluidsynth engine.") + "\n\n" +
            tr("Please note that this operation may cause\n"
               "temporary MIDI and Audio disruption.") + "\n\n" +
            tr("Do you want to restart the engine now?"),
            tr("Yes"), tr("No")) == 0);
    }

    // If allowed, just restart the engine...
    if (bRestart)
        restartSynth();
}


// Message log form requester slot.
void qsynthMainForm::toggleMessagesForm (void)
{
    if (m_pSetup == NULL)
        return;

    if (m_pMessagesForm) {
        m_pSetup->saveWidgetGeometry(m_pMessagesForm);
        if (m_pMessagesForm->isVisible())
            m_pMessagesForm->hide();
        else
            m_pMessagesForm->show();
    }
}


// Channels view form requester slot.
void qsynthMainForm::toggleChannelsForm (void)
{
    if (m_pSetup == NULL)
        return;

    if (m_pChannelsForm) {
        m_pSetup->saveWidgetGeometry(m_pChannelsForm);
        if (m_pChannelsForm->isVisible())
            m_pChannelsForm->hide();
        else
            m_pChannelsForm->show();
    }
}


// Setup dialog requester slot.
void qsynthMainForm::showSetupForm (void)
{
    if (m_pSetup == NULL)
        return;

    qsynthSetupForm *pSetupForm = new qsynthSetupForm(this);
    if (pSetupForm) {
        // Check out some initial nullities(tm)...
        if (m_pSetup->sMessagesFont.isEmpty() && m_pMessagesForm)
            m_pSetup->sMessagesFont = m_pMessagesForm->messagesFont().toString();
        // To track down immediate changes.
        QString sOldMessagesFont = m_pSetup->sMessagesFont;
        // Load the current setup settings.
        pSetupForm->setup(m_pSetup, m_pSynth);
        // Show the setup dialog...
        if (pSetupForm->exec()) {
            // Check wheather something immediate has changed.
            if (sOldMessagesFont != m_pSetup->sMessagesFont)
                updateMessagesFont();
            // Ask for a engine restart?
            promptRestart();
        }
        delete pSetupForm;
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


// Timer callback funtion.
void qsynthMainForm::timerSlot (void)
{
    // Is it the first shot on synth start after a one slot delay?
    if (m_iTimerDelay < QSYNTH_DELAY_MSECS) {
        m_iTimerDelay += QSYNTH_TIMER_MSECS;
        if (m_iTimerDelay >= QSYNTH_DELAY_MSECS)
            startSynth();
    }

    // Some global MIDI activity?
    if (g_iMidiEvent > 0) {
        // Global activity...
        g_iMidiEvent = 0;
        if (g_iMidiState == 0) {
            MidiEventPixmapLabel->setPixmap(*m_pXpmLedOn);
            g_iMidiState++;
        }
    }   // On the other tick...
    else if (g_iMidiEvent == 0 && g_iMidiState > 0) {
        MidiEventPixmapLabel->setPixmap(*m_pXpmLedOff);
        g_iMidiState--;
    }

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

    // Register for the next timer slot.
    QTimer::singleShot(QSYNTH_TIMER_MSECS, this, SLOT(timerSlot()));
}


// Start the fluidsynth clone, based on given settings.
bool qsynthMainForm::startSynth (void)
{
    if (m_pSetup == NULL)
        return false;

    // Start realizing settings...
    m_pSetup->realize();

    const QString sElipsis = "...";
    QStringList::Iterator iter;

    // Create the synthesizer.
    appendMessages(tr("Creating synthesizer engine") + sElipsis);
    m_pSynth = ::new_fluid_synth(m_pSetup->fluid_settings());
    if (m_pSynth == NULL) {
        appendMessagesError(tr("Failed to create the synthesizer. Cannot continue without it."));
        return false;
    }

    // Load the soundfonts...
    playLoadFiles(m_pSetup->soundfonts, true);

    // Start the synthesis thread...
    appendMessages(tr("Creating audio driver") + " (" + m_pSetup->sAudioDriver + ")" + sElipsis);
    m_pAudioDriver = ::new_fluid_audio_driver(m_pSetup->fluid_settings(), m_pSynth);
    if (m_pAudioDriver == NULL) {
        appendMessagesError(tr("Failed to create the audio driver") + " (" + m_pSetup->sAudioDriver + "). " + tr("Cannot continue without it."));
        stopSynth();
        return false;
    }

    // Start the midi router and link it to the synth...
    if (m_pSetup->bMidiIn) {
        // In dump mode, text output is generated for events going into
        // and out of the router. The example dump functions are put into
        // the chain before and after the router..
        appendMessages(tr("Creating MIDI router") + " (" + m_pSetup->sMidiDriver + ")" + sElipsis);
        m_pMidiRouter = ::new_fluid_midi_router(m_pSetup->fluid_settings(),
            m_pSetup->bMidiDump ? qsynth_dump_postrouter : qsynth_handle_midi_event,
            (void*) m_pSynth);
        if (m_pMidiRouter == NULL) {
            appendMessagesError(tr("Failed to create the MIDI input router") + " (" + m_pSetup->sMidiDriver + "); " + tr("no MIDI input will be available."));
        } else {
            ::fluid_synth_set_midi_router(m_pSynth, m_pMidiRouter);
            appendMessages(tr("Creating MIDI driver") + " (" + m_pSetup->sMidiDriver + ")" + sElipsis);
            m_pMidiDriver = ::new_fluid_midi_driver(m_pSetup->fluid_settings(),
                m_pSetup->bMidiDump ? ::fluid_midi_dump_prerouter : ::fluid_midi_router_handle_midi_event,
               (void*) m_pMidiRouter);
            if (m_pMidiDriver == NULL)
                appendMessagesError(tr("Failed to create the MIDI driver") + " (" + m_pSetup->sMidiDriver + "); " + tr("no MIDI input will be available."));
        }
    }

    // Create the MIDI player.
    appendMessages(tr("Creating MIDI player") + sElipsis);
    m_pPlayer = ::new_fluid_player(m_pSynth);
    if (m_pPlayer == NULL) {
        appendMessagesError(tr("Failed to create the MIDI player. Continuing without a player."));
    } else {
        // Play the midi files, if any.
        playLoadFiles(m_pSetup->midifiles, false);
        // We'll accept drops from now on...
        setAcceptDrops(true);
    }

    // Run the server, if requested.
    if (m_pSetup->bServer) {
#ifdef CONFIG_FLUID_SERVER
        appendMessages(tr("Creating server") + sElipsis);
        m_pServer = ::new_fluid_server(m_pSetup->fluid_settings(), qsynth_newclient, m_pSynth);
        if (m_pServer == NULL)
            appendMessagesError(tr("Failed to create the server. Continuing without it."));
#else
        appendMessagesError(tr("Server mode disabled. Continuing without it."));
#endif
    }

    // Finally initialize the front panel controls.
    appendMessages(tr("Loading panel settings") + sElipsis);
    loadPanelSettings();
    // And the channel view too.
    if (m_pChannelsForm) {
        // Setup the channels view window.
        m_pChannelsForm->setup(m_pSetup, m_pSynth);
        // Prepare the channel event state flaggers.
        if (g_pMidiChannels)
            delete [] g_pMidiChannels;
        g_pMidiChannels = NULL;
        g_iMidiChannels = ::fluid_synth_count_midi_channels(m_pSynth);
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

    // Show up our efforts :)
    programReset();
    // All is right.
    appendMessages(tr("Synthesizer engine started."));

    return true;
}


// Stop the fluidsynth clone.
void qsynthMainForm::stopSynth (void)
{
    if (m_pSetup == NULL)
        return;

    const QString sElipsis = "...";

    // Flush anything that maybe pending...
    flushStdoutBuffer();
    
    // Freeze channel view.
    if (m_pChannelsForm) {
        m_pChannelsForm->setup(m_pSetup, NULL);
        // Unprepare the channel event state flaggers.
        if (g_pMidiChannels)
            delete [] g_pMidiChannels;
        g_pMidiChannels = NULL;
        g_iMidiChannels = 0;
    }

    // Start saving the front panel controls.
    // (but only if we have started alright)
    if (m_pAudioDriver) {
        appendMessages(tr("Saving panel settings") + sElipsis);
        savePanelSettings();
    }

#ifdef CONFIG_FLUID_SERVER
    // Destroy server.
    if (m_pServer) {
        appendMessages(tr("Waiting for server to terminate") + sElipsis);
        ::fluid_server_join(m_pServer);
        appendMessages(tr("Destroying server") + sElipsis);
        ::delete_fluid_server(m_pServer);
        m_pServer = NULL;
    }
#endif

    // Destroy player.
    if (m_pPlayer) {
        appendMessages(tr("Stopping MIDI player") + sElipsis);
        ::fluid_player_stop(m_pPlayer);
        appendMessages(tr("Waiting for MIDI player to terminate") + sElipsis);
        ::fluid_player_join(m_pPlayer);
        appendMessages(tr("Destroying MIDI player") + sElipsis);
        ::delete_fluid_player(m_pPlayer);
        m_pPlayer = NULL;
        // We'll refuse drops from now on...
        setAcceptDrops(false);
    }

    // Destroy MIDI router.
    if (m_pMidiRouter) {
        if (m_pMidiDriver) {
            appendMessages(tr("Destroying MIDI driver") + sElipsis);
            ::delete_fluid_midi_driver(m_pMidiDriver);
            m_pMidiDriver = NULL;
        }
        appendMessages(tr("Destroying MIDI router") + sElipsis);
        ::delete_fluid_midi_router(m_pMidiRouter);
        m_pMidiRouter = NULL;
    }

    // Destroy audio driver.
    if (m_pAudioDriver) {
        appendMessages(tr("Destroying audio driver") + sElipsis);
        ::delete_fluid_audio_driver(m_pAudioDriver);
        m_pAudioDriver = NULL;
    }

    // And finally, destroy the synthesizer engine.
    if (m_pSynth) {
        appendMessages(tr("Destroying synthesizer engine") + sElipsis);
        ::delete_fluid_synth(m_pSynth);
        m_pSynth = NULL;
        // We're done.
        appendMessages(tr("Synthesizer engine terminated."));
    }

    // Show up our efforts :)
    stabilizeForm();
}


// Start the fluidsynth clone, based on given settings.
void qsynthMainForm::restartSynth (void)
{
    // Restarting means stopping current engine...
    stopSynth();
    // And making room for immediate restart...
    m_iTimerDelay = 0;
}


// Front panel state load routine.
void qsynthMainForm::loadPanelSettings (void)
{
    if (m_pSetup == NULL)
        return;

    // Reset change flags.
    m_iGainChanged   = 0;
    m_iReverbChanged = 0;
    m_iChorusChanged = 0;
    
    // Avoid update races: set update counters > 0)...
    m_iGainUpdated   = 1;
    m_iReverbUpdated = 1;
    m_iChorusUpdated = 1;

    GainDial->setValue(qsynth_iscale_clip(QSYNTH_GAIN_SCALE, m_pSetup->fGain));
    m_iGainChanged++;

    ReverbActiveCheckBox->setChecked(m_pSetup->bReverbActive);
    ReverbRoomDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, m_pSetup->fReverbRoom));
    ReverbDampDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, m_pSetup->fReverbDamp));
    ReverbWidthDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, m_pSetup->fReverbWidth));
    ReverbLevelDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, m_pSetup->fReverbLevel));
    m_iReverbChanged++;

    ChorusActiveCheckBox->setChecked(m_pSetup->bChorusActive);
    ChorusNrDial->setValue(m_pSetup->iChorusNr);
    ChorusLevelDial->setValue(qsynth_iscale_clip(QSYNTH_CHORUS_SCALE, m_pSetup->fChorusLevel));
    ChorusSpeedDial->setValue(qsynth_iscale_clip(QSYNTH_CHORUS_SCALE, m_pSetup->fChorusSpeed));
    ChorusDepthDial->setValue(qsynth_iscale_clip(QSYNTH_CHORUS_SCALE, m_pSetup->fChorusDepth));
    ChorusTypeComboBox->setCurrentItem(m_pSetup->iChorusType);
    m_iChorusChanged++;

    // Let them get updated, on next tick.
    m_iGainUpdated   = 0;
    m_iReverbUpdated = 0;
    m_iChorusUpdated = 0;
}


// Front panel state save routine.
void qsynthMainForm::savePanelSettings (void)
{
    if (m_pSetup == NULL)
        return;

    m_pSetup->fGain = qsynth_fscale_clip(GainDial->value(), QSYNTH_GAIN_SCALE);

    m_pSetup->bReverbActive = ReverbActiveCheckBox->isChecked();
    m_pSetup->fReverbRoom   = qsynth_fscale_clip(ReverbRoomSpinBox->value(),  QSYNTH_REVERB_SCALE);
    m_pSetup->fReverbDamp   = qsynth_fscale_clip(ReverbDampSpinBox->value(),  QSYNTH_REVERB_SCALE);
    m_pSetup->fReverbWidth  = qsynth_fscale_clip(ReverbWidthSpinBox->value(), QSYNTH_REVERB_SCALE);
    m_pSetup->fReverbLevel  = qsynth_fscale_clip(ReverbLevelSpinBox->value(), QSYNTH_REVERB_SCALE);

    m_pSetup->bChorusActive = ChorusActiveCheckBox->isChecked();
    m_pSetup->iChorusNr     = ChorusNrSpinBox->value();
    m_pSetup->fChorusLevel  = qsynth_fscale_clip(ChorusLevelSpinBox->value(), QSYNTH_CHORUS_SCALE);
    m_pSetup->fChorusSpeed  = qsynth_fscale_clip(ChorusSpeedSpinBox->value(), QSYNTH_CHORUS_SCALE);
    m_pSetup->fChorusDepth  = qsynth_fscale_clip(ChorusDepthSpinBox->value(), QSYNTH_CHORUS_SCALE);
    m_pSetup->iChorusType   = ChorusTypeComboBox->currentItem();
}


// Increment reverb change flag.
void qsynthMainForm::reverbActivate ( bool bActive )
{
    if (m_pSynth == NULL)
        return;

    appendMessagesColor("fluid_synth_set_reverb_on(" + QString::number((int) bActive) + ")", "#99cc33");

    ::fluid_synth_set_reverb_on(m_pSynth, (int) bActive);

    if (bActive)
        refreshReverb();

    stabilizeForm();
}


// Increment chorus change flag.
void qsynthMainForm::chorusActivate ( bool bActive )
{
    if (m_pSynth == NULL)
        return;

    appendMessagesColor("fluid_synth_set_chorus_on(" + QString::number((int) bActive) + ")", "#cc9933");

    ::fluid_synth_set_chorus_on(m_pSynth, (int) bActive);

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
    if (m_pSynth == NULL || m_iGainUpdated > 0)
        return;
    m_iGainUpdated++;

    float fGain = qsynth_fscale_clip(GainSpinBox->value(), QSYNTH_GAIN_SCALE);

    appendMessagesColor("fluid_synth_set_gain(" + QString::number(fGain) + ")", "#6699cc");

    ::fluid_synth_set_gain(m_pSynth, fGain);
    refreshGain();

    m_iGainUpdated--;
    m_iGainChanged = 0;
}


// Update reverb state.
void qsynthMainForm::updateReverb (void)
{
    if (m_pSynth == NULL || m_iReverbUpdated > 0)
        return;
    m_iReverbUpdated++;

    double fReverbRoom  = qsynth_fscale_clip(ReverbRoomSpinBox->value(), QSYNTH_REVERB_SCALE);
    double fReverbDamp  = qsynth_fscale_clip(ReverbDampSpinBox->value(), QSYNTH_REVERB_SCALE);
    double fReverbWidth = qsynth_fscale_clip(ReverbWidthSpinBox->value(), QSYNTH_REVERB_SCALE);
    double fReverbLevel = qsynth_fscale_clip(ReverbLevelSpinBox->value(), QSYNTH_REVERB_SCALE);

    appendMessagesColor("fluid_synth_set_reverb("
        + QString::number(fReverbRoom)  + ","
        + QString::number(fReverbDamp)  + ","
        + QString::number(fReverbWidth) + ","
        + QString::number(fReverbLevel) + ")", "#99cc66");

    ::fluid_synth_set_reverb(m_pSynth, fReverbRoom, fReverbDamp, fReverbWidth, fReverbLevel);
    refreshReverb();

    m_iReverbUpdated--;
    m_iReverbChanged = 0;
}


// Update chorus state.
void qsynthMainForm::updateChorus (void)
{
    if (m_pSynth == NULL || m_iChorusUpdated > 0)
        return;
    m_iChorusUpdated++;

    int    iChorusNr    = ChorusNrSpinBox->value();
    double fChorusLevel = qsynth_fscale_clip(ChorusLevelSpinBox->value(), QSYNTH_CHORUS_SCALE);
    double fChorusSpeed = qsynth_fscale_clip(ChorusSpeedSpinBox->value(), QSYNTH_CHORUS_SCALE);
    double fChorusDepth = qsynth_fscale_clip(ChorusDepthSpinBox->value(), QSYNTH_CHORUS_SCALE);
    int    iChorusType  = ChorusTypeComboBox->currentItem();

    appendMessagesColor("fluid_synth_set_chorus("
        + QString::number(iChorusNr)    + ","
        + QString::number(fChorusLevel) + ","
        + QString::number(fChorusSpeed) + ","
        + QString::number(fChorusDepth) + ","
        + QString::number(iChorusType)  + ")", "#cc9966");

    ::fluid_synth_set_chorus(m_pSynth, iChorusNr, fChorusLevel, fChorusSpeed, fChorusDepth, iChorusType);
    refreshChorus();

    m_iChorusUpdated--;
    m_iChorusChanged = 0;
}


// Refresh gain panel controls.
void qsynthMainForm::refreshGain (void)
{
    if (m_pSynth == NULL)
        return;

    float fGain = ::fluid_synth_get_gain(m_pSynth);

    GainDial->setValue(qsynth_iscale_clip(QSYNTH_GAIN_SCALE, fGain));
}


// Refresh reverb panel controls.
void qsynthMainForm::refreshReverb (void)
{
    if (m_pSynth == NULL)
        return;

    double fReverbRoom  = ::fluid_synth_get_reverb_roomsize(m_pSynth);
    double fReverbDamp  = ::fluid_synth_get_reverb_damp(m_pSynth);
    double fReverbWidth = ::fluid_synth_get_reverb_width(m_pSynth);
    double fReverbLevel = ::fluid_synth_get_reverb_level(m_pSynth);

    ReverbRoomDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, fReverbRoom));
    ReverbDampDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, fReverbDamp));
    ReverbWidthDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, fReverbWidth));
    ReverbLevelDial->setValue(qsynth_iscale_clip(QSYNTH_REVERB_SCALE, fReverbLevel));
}


// Refresh chorus panel controls.
void qsynthMainForm::refreshChorus (void)
{
    if (m_pSynth == NULL)
        return;

    int    iChorusNr    = ::fluid_synth_get_chorus_nr(m_pSynth);
    double fChorusLevel = ::fluid_synth_get_chorus_level(m_pSynth);
    double fChorusSpeed = ::fluid_synth_get_chorus_speed_Hz(m_pSynth);
    double fChorusDepth = ::fluid_synth_get_chorus_depth_ms(m_pSynth);
    int    iChorusType  = ::fluid_synth_get_chorus_type(m_pSynth);

    ChorusNrDial->setValue(iChorusNr);
    ChorusLevelDial->setValue(qsynth_iscale_clip(QSYNTH_CHORUS_SCALE, fChorusLevel));
    ChorusSpeedDial->setValue(qsynth_iscale_clip(QSYNTH_CHORUS_SCALE, fChorusSpeed));
    ChorusDepthDial->setValue(qsynth_iscale_clip(QSYNTH_CHORUS_SCALE, fChorusDepth));
    ChorusTypeComboBox->setCurrentItem(iChorusType);
}


// end of qsynthMainForm.ui.h

