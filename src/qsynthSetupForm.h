// qsynthSetupForm.h
//
/****************************************************************************
   Copyright (C) 2003-2020, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthSetupForm_h
#define __qsynthSetupForm_h

#include "ui_qsynthSetupForm.h"

// Forward declarations.
class qsynthOptions;
class qsynthEngine;
class qsynthSetup;

class QPixmap;


//----------------------------------------------------------------------------
// qsynthSetupForm -- UI wrapper form.

class qsynthSetupForm : public QDialog
{
	Q_OBJECT

public:

	// Constructor.
	qsynthSetupForm(QWidget *pParent = nullptr);
	// Destructor.
	~qsynthSetupForm();

	void setup(qsynthOptions *pOptions, qsynthEngine *pEngine, bool bNew);

public slots:

	void nameChanged(const QString&);
	void midiDriverChanged(const QString&);
	void audioDriverChanged(const QString&);
	void settingsChanged();

	void contextMenuRequested(const QPoint&);

	void openSoundFont();
	void editSoundFont();
	void removeSoundFont();
	void moveUpSoundFont();
	void moveDownSoundFont();

	void stabilizeForm();

protected slots:

	void itemRenamed();

	void accept();
	void reject();

protected:

	// A combo-box text item setter helper.
	void setComboBoxCurrentText(
		QComboBox *pComboBox, const QString& sText) const;

	void updateMidiDevices(const QString& sMidiDriver);
	void updateAudioDevices(const QString& sAudioDriver);

	void refreshSoundFonts();

private:

	// The Qt-designer UI struct...
	Ui::qsynthSetupForm m_ui;

	// Instance variables.
	qsynthOptions *m_pOptions;
	qsynthSetup   *m_pSetup;

	int m_iDirtySetup;
	int m_iDirtyCount;

	QString  m_sSoundFontDir;
	QPixmap *m_pXpmSoundFont;
};


#endif	// __qsynthSetupForm_h


// end of qsynthSetupForm.h
