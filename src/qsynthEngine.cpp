// qsynthEngine.cpp
//
/****************************************************************************
   Copyright (C) 2003-2006, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qsynthEngine.h"


//-------------------------------------------------------------------------
// qsynthEngine - Meta-fluidsynth engine structure class.
//

// Constructor.
qsynthEngine::qsynthEngine ( qsynthOptions *pOptions, const QString& sName )
{
	// We're the default (first) engine whether we've given a name...
	m_bDefault = sName.isEmpty();
	if (m_bDefault) {
		m_pSetup = pOptions->defaultSetup();
	} else {
		m_pSetup = new qsynthSetup();
		pOptions->loadSetup(m_pSetup, sName);
	}

	// From now on, we must have a name.
	m_sName = m_pSetup->sDisplayName;

	pSynth       = NULL;
	pAudioDriver = NULL;
	pMidiRouter  = NULL;
	pMidiDriver  = NULL;
	pPlayer      = NULL;
	pServer      = NULL;

	iMidiEvent = 0;
	iMidiState = 0;

	bMeterEnabled  = false;
	fMeterValue[0] = 0.0f;
	fMeterValue[1] = 0.0f;
}


// Default destructor.
qsynthEngine::~qsynthEngine (void)
{
	if (!m_bDefault && m_pSetup) {
		delete m_pSetup;
		m_pSetup = NULL;
	}
}

// Engine predicate.
bool qsynthEngine::isDefault (void) const
{
	return m_bDefault;
}


// Engine setup accessor.
qsynthSetup *qsynthEngine::setup (void)
{
	return m_pSetup;
}

// Engine name accessors.
const QString& qsynthEngine::name (void) const
{
	return m_sName;
}

void qsynthEngine::setName ( const QString& sName )
{
	m_sName = sName;
}


// end of qsynthEngine.cpp
