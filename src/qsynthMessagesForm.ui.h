// qsynthMessagesForm.ui.h
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

#include "config.h"


// Kind of constructor.
void qsynthMessagesForm::init (void)
{
}


// Kind of destructor.
void qsynthMessagesForm::destroy (void)
{
}


// Notify our parent that we're emerging.
void qsynthMessagesForm::showEvent ( QShowEvent *pShowEvent )
{
    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parent();
    if (pMainForm)
        pMainForm->stabilizeForm();

    QWidget::showEvent(pShowEvent);
}

// Notify our parent that we're closing.
void qsynthMessagesForm::hideEvent ( QHideEvent *pHideEvent )
{
    QWidget::hideEvent(pHideEvent);

    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parent();
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
    while (MessagesTextView->paragraphs() > 1000) {
        MessagesTextView->setUpdatesEnabled(false);
        MessagesTextView->removeParagraph(0);
        MessagesTextView->scrollToBottom();
        MessagesTextView->setUpdatesEnabled(true);
    }
    MessagesTextView->append(s);
}


// end of qsynthMessagesForm.ui.h

