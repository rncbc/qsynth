// qsynthEngine.h
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

#ifndef __qsynthEngine_h
#define __qsynthEngine_h

#include "qsynthOptions.h"


//-------------------------------------------------------------------------
// qsynthEngine - Meta-fluidsynth engine structure class.
//

class qsynthEngine
{
public:

	// Constructor.
	qsynthEngine(qsynthOptions *pOptions, const QString& sName = QString());
	// Default destructor.
	~qsynthEngine();

	// Engine predicate.
	bool isDefault() const;

	// Engine setup accessor.
	qsynthSetup *setup();
	
	// Engine name accessors.
	const QString& name() const;
	void setName(const QString& sName);

	// Engine member public variables.
	fluid_synth_t        *pSynth;
	fluid_audio_driver_t *pAudioDriver;
	fluid_midi_router_t  *pMidiRouter;
	fluid_midi_driver_t  *pMidiDriver;
	fluid_player_t       *pPlayer;
	fluid_server_t       *pServer;

	// Dirty MIDI event trackers.
	int iMidiEvent;
	int iMidiState;

	// Current peak level meters (pseudo-stereo).
	bool  bMeterEnabled;
	float fMeterValue[2];

private:

	// Engine member variables.
	bool           m_bDefault;
	qsynthSetup   *m_pSetup;
	QString        m_sName;
};


#endif  // __qsynthEngine_h


// end of qsynthEngine.h
