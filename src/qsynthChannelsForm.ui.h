// qsynthChannelsForm.ui.h
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

#include <qvalidator.h>

#include "config.h"

#include "qsynthChannels.h"


// Kind of constructor.
void qsynthChannelsForm::init (void)
{
    m_iChannels  = 0;
    m_ppChannels = NULL;

    // No setup synth references initially (the caller will set them).
    m_pSetup = NULL;
    m_pSynth = NULL;

    // Set validators...
    PresetComboBox->setValidator(new QRegExpValidator(QRegExp("[\\w]+"), PresetComboBox));
}


// Kind of destructor.
void qsynthChannelsForm::destroy (void)
{
    if (m_ppChannels)
        delete [] m_ppChannels;

    m_ppChannels = NULL;
    m_iChannels  = 0;
}


// Notify our parent that we're emerging.
void qsynthChannelsForm::showEvent ( QShowEvent *pShowEvent )
{
    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();

    QWidget::showEvent(pShowEvent);
}

// Notify our parent that we're closing.
void qsynthChannelsForm::hideEvent ( QHideEvent *pHideEvent )
{
    QWidget::hideEvent(pHideEvent);

    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();
}


// Populate (setup) synth settings descriptors.
void qsynthChannelsForm::setup ( qsynthSetup *pSetup, fluid_synth_t *pSynth )
{
    m_pSetup = pSetup;
    m_pSynth = pSynth;
}


// Channels preset naming slots.
void qsynthChannelsForm::changePreset( const QString& sPreset )
{
}


void qsynthChannelsForm::savePreset (void)
{
}


void qsynthChannelsForm::deletePreset (void)
{
}


void qsynthChannelsForm::resetPresets (void)
{
}


// end of qsynthChannelsForm.ui.h
