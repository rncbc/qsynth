// qsynthMainForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2003, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include <math.h>

#include "config.h"

#include "qsynthAbout.h"

// Timer constant stuff.
#define QSYNTH_TIMER_MSECS  500

// Notification pipe descriptors
#define QSYNTH_FDNIL     -1
#define QSYNTH_FDREAD     0
#define QSYNTH_FDWRITE    1

static int g_fdStdout[2] = { QSYNTH_FDNIL, QSYNTH_FDNIL };

// Needed for server mode.
static fluid_cmd_handler_t* qsynth_newclient ( void* data, char* )
{
    return ::new_fluid_cmd_handler((fluid_synth_t*) data);
}

// Midi router stubs to have some midi activity feedback.
static int g_iMidiEvent  = 0;

static int qsynth_dump_postrouter (void *pvData, fluid_midi_event_t *pMidiEvent)
{
    g_iMidiEvent++;
    return ::fluid_midi_dump_postrouter(pvData, pMidiEvent);
}

static int qsynth_handle_midi_event (void *pvData, fluid_midi_event_t *pMidiEvent)
{
    g_iMidiEvent++;
    return ::fluid_synth_handle_midi_event(pvData, pMidiEvent);
}


// Kind of constructor.
void qsynthMainForm::init (void)
{
    m_pSetup = NULL;
    m_pTimer = new QTimer(this);
    m_iTimerSlot = 0;

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

    // Check if we can redirect our own stdout/stderr...
    if (::pipe(g_fdStdout) == 0) {
        ::dup2(g_fdStdout[QSYNTH_FDWRITE], STDOUT_FILENO);
        ::dup2(g_fdStdout[QSYNTH_FDWRITE], STDERR_FILENO);
        m_pStdoutNotifier = new QSocketNotifier(g_fdStdout[QSYNTH_FDREAD], QSocketNotifier::Read, this);
        QObject::connect(m_pStdoutNotifier, SIGNAL(activated(int)), this, SLOT(stdoutNotifySlot(int)));
    }

    // All forms are to be created right now.
    m_pMessagesForm = new qsynthMessagesForm(this);

    // Register the timer slot.
    QObject::connect(m_pTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    // Our timer is started...
    m_pTimer->start(QSYNTH_TIMER_MSECS, false);
}


// Kind of destructor.
void qsynthMainForm::destroy (void)
{
    // Stop th press!
    stopSynth();
    m_pSetup = NULL;
    
    // Stop and kill timer.
    m_pTimer->stop();
    delete m_pTimer;

    // Delete pixmaps.
    delete m_pXpmLedOn;
    delete m_pXpmLedOff;
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

    // Set defaults...
    updateMessagesFont();

    // Final startup stabilization...
    stabilizeForm();
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
            m_pSetup->saveWidgetGeometry(m_pMessagesForm);
            m_pSetup->saveWidgetGeometry(this);
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


// Own stdout/stderr socket notifier slot.
void qsynthMainForm::stdoutNotifySlot ( int fd )
{
    char achBuffer[1024];
    int  cchBuffer = ::read(fd, achBuffer, sizeof(achBuffer) - 1);
    if (cchBuffer > 0) {
        achBuffer[cchBuffer] = (char) 0;
        appendMessagesText(achBuffer);
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
    } else {
        GainGroupBox->setEnabled(false);
        ReverbGroupBox->setEnabled(false);
        ChorusGroupBox->setEnabled(false);
    }

    MessagesPushButton->setOn(m_pMessagesForm && m_pMessagesForm->isVisible());
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


// Setup dialog requester slot.
void qsynthMainForm::showSetupForm (void)
{
    if (m_pSetup == NULL)
        return;

    qsynthSetupForm *pSetupForm = new qsynthSetupForm(this);
    if (pSetupForm) {
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
            if (QMessageBox::warning(this, tr("Warning"),
                tr("The new settings will be effective only\n"
                   "after restarting the fluidsynth engine.") + "\n\n" +
                tr("Please note that this operation will cause\n"
                   "temporary MIDI and Audio disruption.") + "\n\n" +
                tr("Do you want to restart the engine now?"),
                tr("Yes"), tr("No")) == 0) {
                // Just stop the engine, it will restart one tick later...
                stopSynth();
            }
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
    if (++m_iTimerSlot == 1) {
        if (!startSynth()) {
            close();
            return;
        }
    }

    // Some MIDI activity?
    if (g_iMidiEvent > 0) {
        MidiEventPixmapLabel->setPixmap(*m_pXpmLedOn);
        g_iMidiEvent = -1;
    }
    else if (g_iMidiEvent < 0) {
        MidiEventPixmapLabel->setPixmap(*m_pXpmLedOff);
        g_iMidiEvent = 0;
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
    for (iter = m_pSetup->soundfonts.begin(); iter != m_pSetup->soundfonts.end(); iter++) {
        const QString& sSoundFont = *iter;
        appendMessages(tr("Loading soundfont") + ": \"" + sSoundFont + "\"" + sElipsis);
        if (::fluid_synth_sfload(m_pSynth, sSoundFont.latin1(), 1) < 0)
            appendMessagesError(tr("Failed to load the soundfont") + ": \"" + sSoundFont + "\".");
    }

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
        for (iter = m_pSetup->midifiles.begin(); iter != m_pSetup->midifiles.end(); iter++) {
            const QString& sMidiFile = *iter;
            ::fluid_player_add(m_pPlayer, (char *) sMidiFile.latin1());
        }
        // Start playing...
        if (m_pSetup->midifiles.count() > 0)
            ::fluid_player_play(m_pPlayer);
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

    // All is right.
    appendMessages(tr("Synthesizer engine started."));
    // Show up our efforts :)
    stabilizeForm();

    return true;
}


// Stop the fluidsynth clone.
void qsynthMainForm::stopSynth (void)
{
    if (m_pSetup == NULL)
        return;

    const QString sElipsis = "...";

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
    }

    // We're done.
    appendMessages(tr("Synthesizer engine terminated."));
    // Show up our efforts :)
    stabilizeForm();
    // Make room for immediate restart...
    m_iTimerSlot = 0;
}


// Front panel state load routine.
void qsynthMainForm::loadPanelSettings (void)
{
    if (m_pSetup == NULL)
        return;

    m_iGainChanged   = 0;
    m_iReverbChanged = 0;
    m_iChorusChanged = 0;

    m_iGainUpdated   = 0;
    m_iReverbUpdated = 0;
    m_iChorusUpdated = 0;

    GainDial->setValue((int) ::ceil(10.0 * m_pSetup->fGain));

    ReverbActiveCheckBox->setChecked(m_pSetup->bReverbActive);
    ReverbRoomDial->setValue((int) ::ceil(10.0 * m_pSetup->fReverbRoom));
    ReverbDampDial->setValue((int) ::ceil(10.0 * m_pSetup->fReverbDamp));
    ReverbWidthDial->setValue((int) ::ceil(10.0 * m_pSetup->fReverbWidth));
    ReverbLevelDial->setValue((int) ::ceil(10.0 * m_pSetup->fReverbLevel));

    ChorusActiveCheckBox->setChecked(m_pSetup->bChorusActive);
    ChorusNrDial->setValue(m_pSetup->iChorusNr);
    ChorusLevelDial->setValue((int) ::ceil(10.0 * m_pSetup->fChorusLevel));
    ChorusSpeedDial->setValue((int) ::ceil(10.0 * m_pSetup->fChorusSpeed));
    ChorusDepthDial->setValue((int) ::ceil(10.0 * m_pSetup->fChorusDepth));
    ChorusTypeComboBox->setCurrentItem(m_pSetup->iChorusType);
}


// Front panel state save routine.
void qsynthMainForm::savePanelSettings (void)
{
    if (m_pSetup == NULL)
        return;

    m_pSetup->fGain = (double) (GainDial->value() / 10.0);

    m_pSetup->bReverbActive = ReverbActiveCheckBox->isChecked();
    m_pSetup->fReverbRoom   = (double) (ReverbRoomSpinBox->value()  / 10.0);
    m_pSetup->fReverbDamp   = (double) (ReverbDampSpinBox->value()  / 10.0);
    m_pSetup->fReverbWidth  = (double) (ReverbWidthSpinBox->value() / 10.0);
    m_pSetup->fReverbLevel  = (double) (ReverbLevelSpinBox->value() / 10.0);

    m_pSetup->bChorusActive = ChorusActiveCheckBox->isChecked();
    m_pSetup->iChorusNr     = ChorusNrSpinBox->value();
    m_pSetup->fChorusLevel  = (double) (ChorusLevelSpinBox->value() / 10.0);
    m_pSetup->fChorusSpeed  = (double) (ChorusSpeedSpinBox->value() / 10.0);
    m_pSetup->fChorusDepth  = (double) (ChorusDepthSpinBox->value() / 10.0);
    m_pSetup->iChorusType   = ChorusTypeComboBox->currentItem();
}


// Increment reverb change flag.
void qsynthMainForm::reverbActivate ( bool bActive )
{
    if (m_pSynth == NULL)
        return;

    appendMessages("fluid_synth_set_reverb_on(" + QString::number((int) bActive) + ")");

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

    appendMessages("fluid_synth_set_chorus_on(" + QString::number((int) bActive) + ")");

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

    float fGain = (float) (GainSpinBox->value() / 10.0);

    appendMessages("fluid_synth_set_gain(" + QString::number(fGain) + ")");

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

    double fReverbRoom  = (double) (ReverbRoomSpinBox->value()  / 10.0);
    double fReverbDamp  = (double) (ReverbDampSpinBox->value()  / 10.0);
    double fReverbWidth = (double) (ReverbWidthSpinBox->value() / 10.0);
    double fReverbLevel = (double) (ReverbLevelSpinBox->value() / 10.0);

    appendMessages("fluid_synth_set_reverb("
        + QString::number(fReverbRoom)  + ","
        + QString::number(fReverbDamp)  + ","
        + QString::number(fReverbWidth) + ","
        + QString::number(fReverbLevel) + ")");

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
    double fChorusLevel = (double) (ChorusLevelSpinBox->value() / 10.0);
    double fChorusSpeed = (double) (ChorusSpeedSpinBox->value() / 10.0);
    double fChorusDepth = (double) (ChorusDepthSpinBox->value() / 10.0);
    int    iChorusType  = ChorusTypeComboBox->currentItem();

    appendMessages("fluid_synth_set_chorus("
        + QString::number(iChorusNr)    + ","
        + QString::number(fChorusLevel) + ","
        + QString::number(fChorusSpeed) + ","
        + QString::number(fChorusDepth) + ","
        + QString::number(iChorusType)  + ")");

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

    GainDial->setValue((int) ::ceil(10.0 * fGain));
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

    ReverbRoomDial->setValue((int) ::ceil(10.0 * fReverbRoom));
    ReverbDampDial->setValue((int) ::ceil(10.0 * fReverbDamp));
    ReverbWidthDial->setValue((int) ::ceil(10.0 * fReverbWidth));
    ReverbLevelDial->setValue((int) ::ceil(10.0 * fReverbLevel));
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
    ChorusLevelDial->setValue((int) ::ceil(10.0 * fChorusLevel));
    ChorusSpeedDial->setValue((int) ::ceil(10.0 * fChorusSpeed));
    ChorusDepthDial->setValue((int) ::ceil(10.0 * fChorusDepth));
    ChorusTypeComboBox->setCurrentItem(iChorusType);
}


// end of qsynthMainForm.ui.h
