// qsynthSetup.cpp
//
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

#include "qsynthSetup.h"

#include <qobject.h>

#include "config.h"


//-------------------------------------------------------------------------
// qsynthSetup - Prototype settings structure.
//

// Constructor.
qsynthSetup::qsynthSetup (void)
{
    m_pFluidSettings = ::new_fluid_settings();
    
    sDefPresetName = QObject::tr("(default)");
}


// Default Destructor.
qsynthSetup::~qsynthSetup (void)
{
    ::delete_fluid_settings(m_pFluidSettings);
    m_pFluidSettings = NULL;
}


// Fluidsynth settings accessor.
fluid_settings_t *qsynthSetup::fluid_settings (void)
{
    return m_pFluidSettings;
}


//-------------------------------------------------------------------------
// Settings cache realization.
//

void qsynthSetup::realize (void)
{
    // The 'groups' setting is only relevant for LADSPA operation
    // If not given, set number groups to number of audio channels, because
    // they are the same (there is nothing between synth output and 'sound card')
    if ((iAudioGroups == 0) && (iAudioChannels != 0))
        iAudioGroups = iAudioChannels;

    if (!sMidiDriver.isEmpty())
        ::fluid_settings_setstr(m_pFluidSettings, "midi.driver", (char *) sMidiDriver.latin1());
    if (iMidiChannels > 0)
        ::fluid_settings_setint(m_pFluidSettings, "synth.midi-channels", iMidiChannels);
    if (!sAudioDriver.isEmpty())
        ::fluid_settings_setstr(m_pFluidSettings, "audio.driver", (char *) sAudioDriver.latin1());
    if (!sJackName.isEmpty())
        ::fluid_settings_setstr(m_pFluidSettings, "audio.jack.id", (char *) sJackName.latin1());
    if (!sAlsaName.isEmpty())
        ::fluid_settings_setstr(m_pFluidSettings, "midi.alsa_seq.id", (char *) sAlsaName.latin1());

    ::fluid_settings_setint(m_pFluidSettings, "audio.jack.autoconnect", (int) bJackAutoConnect);
    ::fluid_settings_setstr(m_pFluidSettings, "audio.jack.multi", (char *) (bJackMulti ? "yes" : "no"));
    ::fluid_settings_setstr(m_pFluidSettings, "audio.sample-format", (char *) sSampleFormat.latin1());

    if (iAudioChannels > 0)
        ::fluid_settings_setint(m_pFluidSettings, "synth.audio-channels", iAudioChannels);
    if (iAudioGroups > 0)
        ::fluid_settings_setint(m_pFluidSettings, "synth.audio-groups", iAudioGroups);
    if (iAudioBufSize > 0)
        ::fluid_settings_setint(m_pFluidSettings, "audio.period-size", iAudioBufSize);
    if (iAudioBufCount > 0)
        ::fluid_settings_setint(m_pFluidSettings, "audio.periods", iAudioBufCount);
    if (fSampleRate > 0.0)
        ::fluid_settings_setnum(m_pFluidSettings, "synth.sample-rate", fSampleRate);
    if (iPolyphony > 0)
        ::fluid_settings_setint(m_pFluidSettings, "synth.polyphony", iPolyphony);
//  Gain is set on realtime (don't need to set it here)
//  if (fGain > 0.0)
//      ::fluid_settings_setnum(m_pFluidSettings, "synth.gain", fGain);

    ::fluid_settings_setstr(m_pFluidSettings, "synth.reverb.active", (char *) (bReverbActive ? "yes" : "no"));
    ::fluid_settings_setstr(m_pFluidSettings, "synth.chorus.active", (char *) (bChorusActive ? "yes" : "no"));
    ::fluid_settings_setstr(m_pFluidSettings, "synth.ladspa.active", (char *) (bLadspaActive ? "yes" : "no"));
    ::fluid_settings_setstr(m_pFluidSettings, "synth.dump",          (char *) (bMidiDump     ? "yes" : "no"));
    ::fluid_settings_setstr(m_pFluidSettings, "synth.verbose",       (char *) (bVerbose      ? "yes" : "no"));
}


// end of qsynthSetup.cpp
