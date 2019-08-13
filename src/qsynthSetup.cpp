// qsynthSetup.cpp
//
/****************************************************************************
   Copyright (C) 2003-2019, rncbc aka Rui Nuno Capela. All rights reserved.

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
	m_pFluidSettings = nullptr;

	sDefPresetName = QObject::tr("(default)");
}


// Default Destructor.
qsynthSetup::~qsynthSetup (void)
{
	if (m_pFluidSettings)
		::delete_fluid_settings(m_pFluidSettings);

	m_pFluidSettings = nullptr;
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

	// We'll need these to avoid pedandic compiler warnings...
	char *pszKey;
#if FLUIDSYNTH_VERSION_MAJOR < 2
	char *pszVal;
#endif

	// First we'll force all other conmmand line options...
	if (!sMidiDriver.isEmpty()) {
		pszKey = (char *) "midi.driver";
		::fluid_settings_setstr(m_pFluidSettings, pszKey,
			sMidiDriver.toLocal8Bit().data());
	}
	if (sMidiDriver == "alsa_seq" || sMidiDriver == "coremidi") {
		QString sKey = "midi." + sMidiDriver + ".id";
		if (!sMidiName.isEmpty()) {
			::fluid_settings_setstr(m_pFluidSettings,
				sKey.toLocal8Bit().data(),
				sMidiName.toLocal8Bit().data());
		}
	}
	else
	if (!sMidiDevice.isEmpty()) {
		QString sMidiKey = "midi.";
		if (sMidiDriver == "alsa_raw")
			sMidiKey += "alsa";
		else
			sMidiKey += sMidiDriver;
		sMidiKey += ".device";
		::fluid_settings_setstr(m_pFluidSettings,
			sMidiKey.toLocal8Bit().data(),
			sMidiDevice.toLocal8Bit().data());
	}

	if (!sAudioDriver.isEmpty()) {
		pszKey = (char *) "audio.driver";
		::fluid_settings_setstr(m_pFluidSettings, pszKey,
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
		pszKey = (char *) "audio.jack.id";
		::fluid_settings_setstr(m_pFluidSettings, pszKey,
			sJackName.toLocal8Bit().data());
	}

	pszKey = (char *) "audio.jack.autoconnect";
	::fluid_settings_setint(m_pFluidSettings, pszKey,
		int(bJackAutoConnect));

	pszKey = (char *) "audio.jack.multi";
#if FLUIDSYNTH_VERSION_MAJOR < 2
	pszVal = (char *) (bJackMulti ? "yes" : "no");
	::fluid_settings_setstr(m_pFluidSettings, pszKey, pszVal);
#else
	::fluid_settings_setint(m_pFluidSettings, pszKey, int(bJackMulti));
#endif

	if (!sSampleFormat.isEmpty()) {
		pszKey = (char *) "audio.sample-format";
		::fluid_settings_setstr(m_pFluidSettings, pszKey,
			sSampleFormat.toLocal8Bit().data());
	}
	if (iAudioBufSize > 0) {
		pszKey = (char *) "audio.period-size";
		::fluid_settings_setint(m_pFluidSettings, pszKey,
			iAudioBufSize);
	}
	if (iAudioBufCount > 0) {
		pszKey = (char *) "audio.periods";
		::fluid_settings_setint(m_pFluidSettings, pszKey,
			iAudioBufCount);
	}
	if (iMidiChannels > 0) {
		pszKey = (char *) "synth.midi-channels";
		::fluid_settings_setint(m_pFluidSettings, pszKey,
			iMidiChannels);
	}

	pszKey = (char *) "synth.midi-bank-select";
	::fluid_settings_setstr(m_pFluidSettings, pszKey, sMidiBankSelect.toLocal8Bit().data());

	if (iAudioChannels > 0) {
		pszKey = (char *) "synth.audio-channels";
		::fluid_settings_setint(m_pFluidSettings, pszKey,
			iAudioChannels);
	}
	if (iAudioGroups > 0) {
		pszKey = (char *) "synth.audio-groups";
		::fluid_settings_setint(m_pFluidSettings, pszKey,
			iAudioGroups);
	}
	if (fSampleRate > 0.0f) {
		pszKey = (char *) "synth.sample-rate";
		::fluid_settings_setnum(m_pFluidSettings, pszKey,
			double(fSampleRate));
	}
	if (iPolyphony > 0) {
		pszKey = (char *) "synth.polyphony";
		::fluid_settings_setint(m_pFluidSettings, pszKey,
			iPolyphony);
	}
//  Gain is set on realtime (don't need to set it here)
//  if (fGain > 0.0f) {
//		pszKey = (char *) "synth.gain";
//		::fluid_settings_setnum(m_pFluidSettings, pszKey,
//			double(fGain));
//	}

	pszKey = (char *) "synth.reverb.active";
#if FLUIDSYNTH_VERSION_MAJOR < 2
	pszVal = (char *) (bReverbActive ? "yes" : "no");
	::fluid_settings_setstr(m_pFluidSettings, pszKey, pszVal);
#else
	::fluid_settings_setint(m_pFluidSettings, pszKey, int(bReverbActive));
#endif

	pszKey = (char *) "synth.chorus.active";
#if FLUIDSYNTH_VERSION_MAJOR < 2
	pszVal = (char *) (bChorusActive ? "yes" : "no");
	::fluid_settings_setstr(m_pFluidSettings, pszKey, pszVal);
#else
	::fluid_settings_setint(m_pFluidSettings, pszKey, int(bChorusActive));
#endif


	pszKey = (char *) "synth.ladspa.active";
#if FLUIDSYNTH_VERSION_MAJOR < 2
	pszVal = (char *) (bLadspaActive ? "yes" : "no");
	::fluid_settings_setstr(m_pFluidSettings, pszKey, pszVal);
#else
	::fluid_settings_setint(m_pFluidSettings, pszKey, int(bLadspaActive));
#endif


	pszKey = (char *) "synth.dump";
#if FLUIDSYNTH_VERSION_MAJOR < 2
	pszVal = (char *) (bMidiDump ? "yes" : "no");
		::fluid_settings_setstr(m_pFluidSettings, pszKey, pszVal);
#else
	::fluid_settings_setint(m_pFluidSettings, pszKey, int(bMidiDump));
#endif


	pszKey = (char *) "synth.verbose";
#if FLUIDSYNTH_VERSION_MAJOR < 2
	pszVal = (char *) (bVerbose ? "yes" : "no");
	::fluid_settings_setstr(m_pFluidSettings, pszKey, pszVal);
#else
	::fluid_settings_setint(m_pFluidSettings, pszKey, int(bVerbose));
#endif

	// Last we set user supplied options...
	QStringListIterator iter(options);
	while (iter.hasNext()) {
		const QString sOpt = iter.next();
		const QString sKey = sOpt.section('=', 0, 0);
		const QString sVal = sOpt.section('=', 1, 1);
		QByteArray tmp = sKey.toLocal8Bit();
		pszKey = tmp.data();
		switch (::fluid_settings_get_type(m_pFluidSettings, pszKey)) {
		case FLUID_NUM_TYPE:
			::fluid_settings_setnum(m_pFluidSettings, pszKey,
				sVal.toFloat());
			break;
		case FLUID_INT_TYPE:
			::fluid_settings_setint(m_pFluidSettings, pszKey,
				sVal.toInt());
			break;
		case FLUID_STR_TYPE:
		default:
			::fluid_settings_setstr(m_pFluidSettings, pszKey,
				sVal.toLocal8Bit().data());
			break;
		}
	}
}


// end of qsynthSetup.cpp
