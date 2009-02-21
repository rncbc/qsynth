// qsynthSetup.cpp
//
/****************************************************************************
   Copyright (C) 2003-2009, rncbc aka Rui Nuno Capela. All rights reserved.

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

	if (!sMidiDriver.isEmpty()) {
	  char szTmp[] = "midi.driver";
		::fluid_settings_setstr(m_pFluidSettings, szTmp,
		  sMidiDriver.toLocal8Bit().data());
	}
	if (sMidiDriver == "alsa_seq") {
	  char szTmp[] = "midi.alsa_seq.id";
		if (!sAlsaName.isEmpty())
			::fluid_settings_setstr(m_pFluidSettings, szTmp,
				sAlsaName.toLocal8Bit().data());
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
			sMidiKey.toLocal8Bit().data(),
			sMidiDevice.toLocal8Bit().data());
	}

	if (!sAudioDriver.isEmpty()) {
	  char szTmp[] = "audio.driver";
		::fluid_settings_setstr(m_pFluidSettings, szTmp,
			sAudioDriver.toLocal8Bit().data());
	}
	if (!sAudioDevice.isEmpty()) {
		QString sAudioKey = "audio." + sAudioDriver + '.';
		if (sAudioDriver == "file")
			sAudioKey += "name";
		else
			sAudioKey += "device";
		  ::fluid_settings_setstr(m_pFluidSettings,
			  sAudioKey.toLocal8Bit().data(),
			  sAudioDevice.toLocal8Bit().data());
	}
	if (!sJackName.isEmpty()) {
	  char szTmp[] = "audio.jack.id";
		::fluid_settings_setstr(m_pFluidSettings, szTmp,
			sJackName.toLocal8Bit().data());
	}
	{
	  char szTmp[] = "audio.jack.autoconnect";
	  ::fluid_settings_setint(m_pFluidSettings, szTmp,
	    int(bJackAutoConnect));
	}
	{
	  char szTmp[] = "audio.jack.multi";
	  QString qsTmp = (bJackMulti ? "yes" : "no");
	  ::fluid_settings_setstr(m_pFluidSettings, szTmp,
	    qsTmp.toLocal8Bit().data());
	}
	if (!sSampleFormat.isEmpty()) {
	  char szTmp[] = "audio.sample-format";
	  ::fluid_settings_setstr(m_pFluidSettings, szTmp,
		  sSampleFormat.toLocal8Bit().data());
	}
	if (iAudioBufSize > 0) {
	  char szTmp[] = "audio.period-size";
		::fluid_settings_setint(m_pFluidSettings, szTmp,
			iAudioBufSize);
	}
	if (iAudioBufCount > 0) {
    char szTmp[] = "audio.periods";
		::fluid_settings_setint(m_pFluidSettings, szTmp,
			iAudioBufCount);
  }
	if (iMidiChannels > 0) {
	  char szTmp[] = "synth.midi-channels";
		::fluid_settings_setint(m_pFluidSettings, szTmp,
			iMidiChannels);
  }
	if (iAudioChannels > 0) {
    char szTmp[] = "synth.audio-channels";
		::fluid_settings_setint(m_pFluidSettings, szTmp,
			iAudioChannels);
	}
	if (iAudioGroups > 0) {
    char szTmp[] = "synth.audio-groups";
		::fluid_settings_setint(m_pFluidSettings, szTmp,
			iAudioGroups);
	}
	if (fSampleRate > 0.0) {
    char szTmp[] = "synth.sample-rate";
		::fluid_settings_setnum(m_pFluidSettings, szTmp,
			fSampleRate);
	}
	if (iPolyphony > 0) {
    char szTmp[] = "synth.polyphony";
		::fluid_settings_setint(m_pFluidSettings, szTmp,
			iPolyphony);
	}
//  Gain is set on realtime (don't need to set it here)
//  if (fGain > 0.0)
//      ::fluid_settings_setnum(m_pFluidSettings, "synth.gain", fGain);
	{
	  char szTmp[] = "synth.reverb.active";
	  QString qsTmp = (bReverbActive ? "yes" : "no");
	  ::fluid_settings_setstr(m_pFluidSettings, szTmp,
      qsTmp.toLocal8Bit().data());
	}
	{
	  char szTmp[] = "synth.chorus.active";
	  QString qsTmp = (bChorusActive ? "yes" : "no");
	  ::fluid_settings_setstr(m_pFluidSettings, szTmp,
	    qsTmp.toLocal8Bit().data());
  }
  {
    char szTmp[] = "synth.ladspa.active";
    QString qsTmp = (bLadspaActive ? "yes" : "no");
	  ::fluid_settings_setstr(m_pFluidSettings, szTmp,
	    qsTmp.toLocal8Bit().data());
  }
  {
    char szTmp[] = "synth.dump";
    QString qsTmp = (bMidiDump ? "yes" : "no");
	  ::fluid_settings_setstr(m_pFluidSettings, szTmp,
	    qsTmp.toLocal8Bit().data());
  }
  {
    char szTmp[] = "synth.verbose";
    QString qsTmp = (bVerbose ? "yes" : "no");
	  ::fluid_settings_setstr(m_pFluidSettings, szTmp,
	    qsTmp.toLocal8Bit().data());
  }
}


// end of qsynthSetup.cpp
