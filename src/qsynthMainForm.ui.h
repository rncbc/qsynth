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

#include "config.h"

#include "qsynthAbout.h"

// Timer constant stuff.
#define QSYNTH_DELAY_MSECS  200

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


// Kind of constructor.
void qsynthMainForm::init (void)
{
    m_pSetup = NULL;
    m_pTimer = new QTimer(this);
    
    m_pSynth        = NULL;
    m_pAudioDriver  = NULL;
    m_pMidiRouter   = NULL;
    m_pMidiDriver   = NULL;
    m_pPlayer       = NULL;
    m_pServer       = NULL;

    m_pStdoutNotifier = NULL;

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
    // Our timer is started as one shot.
    m_pTimer->start(QSYNTH_DELAY_MSECS, true);
}


// Kind of destructor.
void qsynthMainForm::destroy (void)
{
    // Stop th press!
    stopSynth();

    // Kill timer.
    delete m_pTimer;
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
    // Dow we quit right away?
    // ...
    if (m_pSetup) {
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

void qsynthMainForm::reject (void)
{
    if (queryClose())
        QDialog::reject();
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
        pSetupForm->load(m_pSetup);
        // Show the setup dialog...
        if (pSetupForm->exec()) {
            // Save the new setup settings.
            pSetupForm->save(m_pSetup);
            // Check wheather something immediate has changed.
            if (sOldMessagesFont != m_pSetup->sMessagesFont)
                updateMessagesFont();
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
    // Is it the first shot on synth start after a few delay?
    if (!startSynth())
        close();
}


// Start the fluidsynth clone, based on given settings.
bool qsynthMainForm::startSynth (void)
{
    if (m_pSetup == NULL)
        return false;

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
        appendMessages(tr("Loading soundfont:") + " \"" + sSoundFont + "\"" + sElipsis);
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
            m_pSetup->bMidiDump ? ::fluid_midi_dump_postrouter : ::fluid_synth_handle_midi_event,
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
    m_pSetup = NULL;
}


// end of qsynthMainForm.ui.h
