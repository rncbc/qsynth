// qsynthChannelsForm.h
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

#ifndef __qsynthChannelsForm_h
#define __qsynthChannelsForm_h

#include "ui_qsynthChannelsForm.h"

#include <fluidsynth.h>

// Forward declarations.
class qsynthOptions;
class qsynthEngine;

class qsynthChannelsItem;

class QPixmap;


//----------------------------------------------------------------------------
// qsynthChannelsForm -- UI wrapper form.

class qsynthChannelsForm : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qsynthChannelsForm(QWidget *pParent = 0, Qt::WFlags wflags = 0);
	// Destructor.
	~qsynthChannelsForm();

	void setup(qsynthOptions *pOptions,
		qsynthEngine *pEngine, bool bPreset);

	void setChannelOn(int iChan, bool bOn);
	void resetAllChannels(bool bPreset);
	void updateChannel(int iChan);

public slots:

	void itemActivated(QTreeWidgetItem*,int);

	void changePreset(const QString& sPreset);

	void savePreset();
	void deletePreset();

	void editSelectedChannel();

	void updateAllChannels();

	void contextMenuRequested(const QPoint&);

protected:

	void showEvent(QShowEvent * pShowEvent);
	void hideEvent(QHideEvent * pHideEvent);
	void stabilizeForm();
	void resetPresets();

private:

	// The Qt-designer UI struct...
	Ui::qsynthChannelsForm m_ui;

	// Instance variables.
	int m_iChannels;

	qsynthChannelsItem **m_ppChannels;

	qsynthOptions *m_pOptions;
	qsynthEngine  *m_pEngine;

	fluid_synth_t *m_pSynth;

	int m_iDirtySetup;
	int m_iDirtyCount;

	QPixmap *m_pXpmLedOn;
	QPixmap *m_pXpmLedOff;
};


#endif	// __qsynthChannelsForm_h


// end of qsynthChannelsForm.h

