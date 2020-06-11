// qsynthMessagesForm.cpp
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

#include "qsynthAbout.h"
#include "qsynthMessagesForm.h"

#include "qsynthMainForm.h"

#include <QFile>
#include <QDateTime>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextStream>

#include <QShowEvent>
#include <QHideEvent>


// The maximum number of message lines.
#define QSYNTH_MESSAGES_MAXLINES  1000


// Deprecated QTextStreamFunctions/Qt namespaces workaround.
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define endl	Qt::endl;
#endif


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

	m_pMessagesLog = nullptr;
}


// Destructor.
qsynthMessagesForm::~qsynthMessagesForm (void)
{
	setLogging(false);
}


// Notify our parent that we're emerging.
void qsynthMessagesForm::showEvent ( QShowEvent *pShowEvent )
{
	qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
	if (pMainForm)
		pMainForm->stabilizeFormEx();

	QWidget::showEvent(pShowEvent);
}


// Notify our parent that we're closing.
void qsynthMessagesForm::hideEvent ( QHideEvent *pHideEvent )
{
	QWidget::hideEvent(pHideEvent);

	qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
	if (pMainForm)
		pMainForm->stabilizeFormEx();
}


// Messages view font accessors.
QFont qsynthMessagesForm::messagesFont (void) const
{
	return m_ui.MessagesTextView->font();
}

void qsynthMessagesForm::setMessagesFont ( const QFont& font )
{
	m_ui.MessagesTextView->setFont(font);
}


// Messages line limit accessors.
int qsynthMessagesForm::messagesLimit (void) const
{
	return m_iMessagesLimit;
}

void qsynthMessagesForm::setMessagesLimit ( int iMessagesLimit )
{
	m_iMessagesLimit = iMessagesLimit;
	m_iMessagesHigh  = iMessagesLimit + (iMessagesLimit / 3);

//	m_ui.MessagesTextView->setMaxLogLines(iMessagesLimit);
}


// Messages logging stuff.
bool qsynthMessagesForm::isLogging (void) const
{
	return (m_pMessagesLog != nullptr);
}

void qsynthMessagesForm::setLogging ( bool bEnabled, const QString& sFilename )
{
	if (m_pMessagesLog) {
		appendMessages(tr("Logging stopped --- %1 ---")
			.arg(QDateTime::currentDateTime().toString()));
		m_pMessagesLog->close();
		delete m_pMessagesLog;
		m_pMessagesLog = nullptr;
	}

	if (bEnabled) {
		m_pMessagesLog = new QFile(sFilename);
		if (m_pMessagesLog->open(QIODevice::Text | QIODevice::Append)) {
			appendMessages(tr("Logging started --- %1 ---")
				.arg(QDateTime::currentDateTime().toString()));
		} else {
			delete m_pMessagesLog;
			m_pMessagesLog = nullptr;
		}
	}
}


// Messages log output method.
void qsynthMessagesForm::appendMessagesLog ( const QString& s )
{
	if (m_pMessagesLog) {
		QTextStream(m_pMessagesLog)
			<< QTime::currentTime().toString("hh:mm:ss.zzz")
			<< ' ' + s << endl;
		m_pMessagesLog->flush();
	}
}

// Messages widget output method.
void qsynthMessagesForm::appendMessagesLine ( const QString& s )
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


// Messages widget output method.
void qsynthMessagesForm::appendMessages ( const QString& s )
{
	appendMessagesColor(s, Qt::gray);
}

void qsynthMessagesForm::appendMessagesColor ( const QString& s, const QColor& rgb )
{
	appendMessagesLine("<font color=\"" + rgb.name() + "\">" + s + "</font>");
	appendMessagesLog(s);
}

void qsynthMessagesForm::appendMessagesText ( const QString& s )
{
	appendMessagesLine(s);
	appendMessagesLog(s);
}


// end of qsynthMessagesForm.cpp
