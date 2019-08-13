// qsynthSetup.h
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

#ifndef __qsynthSetup_h
#define __qsynthSetup_h

#include <QStringList>
#include <QSettings>

#include <fluidsynth.h>


//-------------------------------------------------------------------------
// qsynthSetup - Prototype settings class.
//

class qsynthSetup
{
public:

	// Constructor.
	qsynthSetup();
	// Default destructor.
	~qsynthSetup();

	// Settings cache realization.
	void realize();

	// Fluidsynth settings accessor.
	fluid_settings_t *fluid_settings();

	// Setup display name.
	QString sDisplayName;

	// Settings variables.
	bool    bMidiIn;
	QString sMidiDriver;
	QString sMidiDevice;
	QString sMidiBankSelect;
	int     iMidiChannels;
	QString sMidiName;
	QString sAudioDriver;
	QString sAudioDevice;
	QString sJackName;
	bool    bJackAutoConnect;
	bool    bJackMulti;
	int     iAudioChannels;
	int     iAudioGroups;
	int     iAudioBufSize;
	int     iAudioBufCount;
	QString sSampleFormat;
	float   fSampleRate;
	int     iPolyphony;
	bool    bReverbActive;
	double  fReverbRoom;
	double  fReverbDamp;
	double  fReverbWidth;
	double  fReverbLevel;
	bool    bChorusActive;
	int     iChorusNr;
	double  fChorusLevel;
	double  fChorusSpeed;
	double  fChorusDepth;
	int     iChorusType;
	bool    bLadspaActive;
	float   fGain;
	bool    bServer;
	bool    bMidiDump;
	bool    bVerbose;

	// Optional options.
	QStringList options;

	// Optional file lists.
	QStringList soundfonts;
	QStringList bankoffsets;
	QStringList midifiles;

	// Current (translated) preset name.
	QString sDefPresetName;

	// Current (default) preset name.
	QString sDefPreset;
	// Available presets list.
	QStringList presets;

private:

	// Fluidsynth settings member variable.
	fluid_settings_t *m_pFluidSettings;
};


#endif  // __qsynthSetup_h


// end of qsynthSetup.h
