// qsynthMessagesForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
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

#include "qsynthAbout.h"


// The maximum number of message lines.
#define QSYNTH_MESSAGES_MAXLINES  1000


// Kind of constructor.
void qsynthMessagesForm::init (void)
{
#if QT_VERSION >= 0x030200
    MessagesTextView->setTextFormat(Qt::LogText);
#endif
    // Initialize default message limit.
    setMessagesLimit(QSYNTH_MESSAGES_MAXLINES);
}


// Kind of destructor.
void qsynthMessagesForm::destroy (void)
{
}


// Notify our parent that we're emerging.
void qsynthMessagesForm::showEvent ( QShowEvent *pShowEvent )
{
    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();

    QWidget::showEvent(pShowEvent);
}

// Notify our parent that we're closing.
void qsynthMessagesForm::hideEvent ( QHideEvent *pHideEvent )
{
    QWidget::hideEvent(pHideEvent);

    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();
}


// Messages view font accessors.
QFont qsynthMessagesForm::messagesFont (void)
{
    return MessagesTextView->font();
}

void qsynthMessagesForm::setMessagesFont ( const QFont & font )
{
    MessagesTextView->setFont(font);
}


// Messages line limit accessors.
int qsynthMessagesForm::messagesLimit (void)
{
    return m_iMessagesLimit;
}

void qsynthMessagesForm::setMessagesLimit( int iMessagesLimit )
{
    m_iMessagesLimit = iMessagesLimit;
    m_iMessagesHigh  = iMessagesLimit + (iMessagesLimit / 3);
#if QT_VERSION >= 0x030200
	MessagesTextView->setMaxLogLines(iMessagesLimit);
#endif
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
#if QT_VERSION < 0x030200
    // Check for message line limit...
    if (m_iMessagesLimit > 0) {
        int iParagraphs = MessagesTextView->paragraphs();
        if (iParagraphs > m_iMessagesHigh) {
            MessagesTextView->setUpdatesEnabled(false);
            while (iParagraphs > m_iMessagesLimit) {
                MessagesTextView->removeParagraph(0);
                iParagraphs--;
            }
            MessagesTextView->scrollToBottom();
            MessagesTextView->setUpdatesEnabled(true);
        }
    }
#endif
    MessagesTextView->append(s);
}


// end of qsynthMessagesForm.ui.h

