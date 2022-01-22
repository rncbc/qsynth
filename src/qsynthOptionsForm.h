// qsynthOptionsForm.h
//
/****************************************************************************
   Copyright (C) 2003-2022, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthOptionsForm_h
#define __qsynthOptionsForm_h

#include "ui_qsynthOptionsForm.h"

// Forward declarations.
class qsynthOptions;


//----------------------------------------------------------------------------
// qsynthOptionsForm -- UI wrapper form.

class qsynthOptionsForm : public QDialog
{
	Q_OBJECT

public:

	// Constructor.
	qsynthOptionsForm(QWidget *pParent = nullptr);
	// Destructor.
	~qsynthOptionsForm();

	void setup(qsynthOptions *pOptions);

protected slots:

	void optionsChanged();

	void chooseMessagesFont();
	void browseMessagesLogPath();

	void editCustomColorThemes();

	void accept();
	void reject();

protected:

	// Custom color/style themes settlers.
	void resetCustomColorThemes(const QString& sCustomColorTheme);
	void resetCustomStyleThemes(const QString& sCustomStyleTheme);

	void resetDefaultLanguage(const QString& sLanguage);

	void stabilizeForm();

private:

	// The Qt-designer UI struct...
	Ui::qsynthOptionsForm m_ui;

	// Instance variables.
	qsynthOptions *m_pOptions;

	int m_iDirtySetup;
	int m_iDirtyCount;
};


#endif	// __qsynthOptionsForm_h


// end of qsynthOptionsForm.h
