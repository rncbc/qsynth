// qsynthAboutForm.h
//
/****************************************************************************
   Copyright (C) 2003-2025, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthAboutForm_h
#define __qsynthAboutForm_h

#include "ui_qsynthAboutForm.h"


//----------------------------------------------------------------------------
// qsynthAboutForm -- UI wrapper form.

class qsynthAboutForm : public QDialog
{
	Q_OBJECT

public:

	// Constructor.
	qsynthAboutForm(QWidget *pParent = nullptr);
	// Destructor.
	~qsynthAboutForm();

public slots:

	void aboutQt();

private:

	// The Qt-designer UI struct...
	Ui::qsynthAboutForm m_ui;
};


#endif	// __qsynthAboutForm_h


// end of qsynthAboutForm.h
