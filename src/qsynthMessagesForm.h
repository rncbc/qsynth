// qsynthMessagesForm.h
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

#ifndef __qsynthMessagesForm_h
#define __qsynthMessagesForm_h

#include "ui_qsynthMessagesForm.h"


//----------------------------------------------------------------------------
// qsynthMessagesForm -- UI wrapper form.

class qsynthMessagesForm : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qsynthMessagesForm(QWidget *pParent = 0, Qt::WindowFlags wflags = 0);
	// Destructor.
	~qsynthMessagesForm();


	QFont messagesFont() const;
	void setMessagesFont(const QFont& font);

	int messagesLimit() const;
	void setMessagesLimit(int iLimit);

	void appendMessages(const QString& s);
	void appendMessagesColor(const QString& s, const QString& c);
	void appendMessagesText(const QString& s);

protected:

	void showEvent(QShowEvent * pShowEvent);
	void hideEvent(QHideEvent * pHideEvent);

private:

	// The Qt-designer UI struct...
	Ui::qsynthMessagesForm m_ui;

	// Instance variables.
	int m_iMessagesLines;
	int m_iMessagesLimit;
	int m_iMessagesHigh;
};


#endif	// __qsynthMessagesForm_h


// end of qsynthMessagesForm.h
