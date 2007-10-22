// qsynthSetup.cpp
//
/****************************************************************************
   Copyright (C) 2003-2007, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qsynthSetup.h"

#include <QObject>


//-------------------------------------------------------------------------
// qsynthSetup - Prototype settings structure.
//

// Constructor.
qsynthSetup::qsynthSetup (void)
{
	m_pFluidSettings = NULL;

	sDefPresetName = QObject::tr("(default)");
}


// Default Destructor.
qsynthSetup::~qsynthSetup (void)
{
	if (m_pFluidSettings)
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
	if (m_pFluidSettings)
		::delete_fluid_settings(m_pFluidSettings);
	m_pFluidSettings = ::new_fluid_settings();

	// The 'groups' setting is only relevant for LADSPA operation
	// If not given, set number groups to number of audio channels, because
	// they are the same (there is nothing between synth output and 'sound card')
	if ((iAudioGroups == 0) && (iAudioChannels != 0))
		iAudioGroups = iAudioChannels;

	if (!sMidiDriver.isEmpty())
		::fluid_settings_setstr(m_pFluidSettings, "midi.driver",
			const_cast<char *> (sMidiDriver.toUtf8().constData()));
	if (sMidiDriver == "alsa_seq") {
		if (!sAlsaName.isEmpty())
			::fluid_settings_setstr(m_pFluidSettings, "midi.alsa_seq.id",
				const_cast<char *> (sAlsaName.toUtf8().constData()));
	}
	else
	if (!sMidiDevice.isEmpty()) {
		QString sMidiKey = "midi.";
		if (sMidiDriver == "alsa_raw")
			sMidiKey += "alsa";
		else
			sMidiKey += sMidiDriver;
		sMidiKey += + ".device";
		::fluid_settings_setstr(m_pFluidSettings,
			const_cast<char *> (sMidiKey.toUtf8().constData()),
			const_cast<char *> (sMidiDevice.toUtf8().constData()));
	}

	if (!sAudioDriver.isEmpty())
		::fluid_settings_setstr(m_pFluidSettings, "audio.driver",
			const_cast<char *> (sAudioDriver.toUtf8().constData()));
	if (!sAudioDevice.isEmpty()) {
		QString sAudioKey = "audio." + sAudioDriver + '.';
		if (sAudioDriver == "file")
			sAudioKey += "name";
		else
			sAudioKey += "device";
		::fluid_settings_setstr(m_pFluidSettings,
			const_cast<char *> (sAudioKey.toUtf8().constData()),
			const_cast<char *> (sAudioDevice.toUtf8().constData()));
	}
	if (!sJackName.isEmpty())
		::fluid_settings_setstr(m_pFluidSettings, "audio.jack.id",
			const_cast<char *> (sJackName.toUtf8().constData()));

	::fluid_settings_setint(m_pFluidSettings, "audio.jack.autoconnect",
		int(bJackAutoConnect));
	::fluid_settings_setstr(m_pFluidSettings, "audio.jack.multi",
		const_cast<char *> (bJackMulti ? "yes" : "no"));
	::fluid_settings_setstr(m_pFluidSettings, "audio.sample-format",
		const_cast<char *> (sSampleFormat.toUtf8().constData()));
	if (iAudioBufSize > 0)
		::fluid_settings_setint(m_pFluidSettings, "audio.period-size",
			iAudioBufSize);
	if (iAudioBufCount > 0)
		::fluid_settings_setint(m_pFluidSettings, "audio.periods",
			iAudioBufCount);

	if (iMidiChannels > 0)
		::fluid_settings_setint(m_pFluidSettings, "synth.midi-channels",
			iMidiChannels);
	if (iAudioChannels > 0)
		::fluid_settings_setint(m_pFluidSettings, "synth.audio-channels",
			iAudioChannels);
	if (iAudioGroups > 0)
		::fluid_settings_setint(m_pFluidSettings, "synth.audio-groups",
			iAudioGroups);
	if (fSampleRate > 0.0)
		::fluid_settings_setnum(m_pFluidSettings, "synth.sample-rate",
			fSampleRate);
	if (iPolyphony > 0)
		::fluid_settings_setint(m_pFluidSettings, "synth.polyphony",
			iPolyphony);

//  Gain is set on realtime (don't need to set it here)
//  if (fGain > 0.0)
//      ::fluid_settings_setnum(m_pFluidSettings, "synth.gain", fGain);
	::fluid_settings_setstr(m_pFluidSettings, "synth.reverb.active",
		const_cast<char *> (bReverbActive ? "yes" : "no"));
	::fluid_settings_setstr(m_pFluidSettings, "synth.chorus.active",
		const_cast<char *> (bChorusActive ? "yes" : "no"));
	::fluid_settings_setstr(m_pFluidSettings, "synth.ladspa.active",
		const_cast<char *> (bLadspaActive ? "yes" : "no"));
	::fluid_settings_setstr(m_pFluidSettings, "synth.dump",
		const_cast<char *> (bMidiDump     ? "yes" : "no"));
	::fluid_settings_setstr(m_pFluidSettings, "synth.verbose",
		const_cast<char *> (bVerbose      ? "yes" : "no"));
}


// end of qsynthSetup.cpp
