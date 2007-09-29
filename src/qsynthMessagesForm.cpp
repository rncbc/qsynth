// qsynthMessagesForm.cpp
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

#include "qsynthAbout.h"
#include "qsynthMessagesForm.h"

#include "qsynthMainForm.h"

#include <QDateTime>
#include <QTextBlock>
#include <QTextCursor>

#include <QShowEvent>
#include <QHideEvent>


// The maximum number of message lines.
#define QSYNTH_MESSAGES_MAXLINES  1000


//----------------------------------------------------------------------------
// qsynthMessagesForm -- UI wrapper form.

// Constructor.
qsynthMessagesForm::qsynthMessagesForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QWidget(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

//  m_ui.MessagesTextView->setTextFormat(Qt::LogText);

	// Initialize default message limit.
	m_iMessagesLines = 0;
	setMessagesLimit(QSYNTH_MESSAGES_MAXLINES);
}


// Destructor.
qsynthMessagesForm::~qsynthMessagesForm (void)
{
}


// Notify our parent that we're emerging.
void qsynthMessagesForm::showEvent ( QShowEvent *pShowEvent )
{
	qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
	if (pMainForm)
		pMainForm->stabilizeForm();

	QWidget::showEvent(pShowEvent);
}

// Notify our parent that we're closing.
void qsynthMessagesForm::hideEvent ( QHideEvent *pHideEvent )
{
	QWidget::hideEvent(pHideEvent);

	qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
	if (pMainForm)
		pMainForm->stabilizeForm();
}


// Messages view font accessors.
QFont qsynthMessagesForm::messagesFont (void) const
{
	return m_ui.MessagesTextView->font();
}

void qsynthMessagesForm::setMessagesFont ( const QFont & font )
{
	m_ui.MessagesTextView->setFont(font);
}


// Messages line limit accessors.
int qsynthMessagesForm::messagesLimit (void) const
{
	return m_iMessagesLimit;
}

void qsynthMessagesForm::setMessagesLimit( int iMessagesLimit )
{
	m_iMessagesLimit = iMessagesLimit;
	m_iMessagesHigh  = iMessagesLimit + (iMessagesLimit / 3);

//	m_ui.MessagesTextView->setMaxLogLines(iMessagesLimit);
}


// Messages widget output method.
void qsynthMessagesForm::appendMessages( const QString& s )
{
	appendMessagesColor(s, "#999999");
}

void qsynthMessagesForm::appendMessagesColor( const QString& s, const QString& c )
{
	appendMessagesText("<font color=\"" + c + "\">" + QTime::currentTime().toString("hh:mm:ss.zzz") + " " + s + "</font>");
}

void qsynthMessagesForm::appendMessagesText( const QString& s )
{
	// Check for message line limit...
	if (m_iMessagesLines > m_iMessagesHigh) {
		m_ui.MessagesTextView->setUpdatesEnabled(false);
		QTextCursor textCursor(m_ui.MessagesTextView->document()->begin());
		while (m_iMessagesLines > m_iMessagesLimit) {
			// Move cursor extending selection
			// from start to next line-block...
			textCursor.movePosition(
				QTextCursor::NextBlock, QTextCursor::KeepAnchor);
			m_iMessagesLines--;
		}
		// Remove the excessive line-blocks...
		textCursor.removeSelectedText();
		m_ui.MessagesTextView->setUpdatesEnabled(true);
	}

	m_ui.MessagesTextView->append(s);
	m_iMessagesLines++;
}


// end of qsynthMessagesForm.cpp
