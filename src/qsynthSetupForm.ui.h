// qsynthSetupForm.ui.h
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
#include <qfiledialog.h>
#include <qfontdialog.h>

#include "config.h"


// Kind of constructor.
void qsynthSetupForm::init (void)
{
    // Try to restore old window positioning.
    adjustSize();

    // Final startup stabilization...
    stabilizeForm();
}


// Kind of destructor.
void qsynthSetupForm::destroy (void)
{
}


void qsynthSetupForm::stabilizeForm (void)
{
}


// The messages font selection dialog.
void qsynthSetupForm::chooseMessagesFont()
{
    bool  bOk  = false;
    QFont font = QFontDialog::getFont(&bOk, MessagesFontTextLabel->font(), this);
    if (bOk) {
        MessagesFontTextLabel->setFont(font);
        MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
    }
}


// Populate (load) dialog controls from setup struct members.
void qsynthSetupForm::load ( qsynthSetup *pSetup )
{
    // Load Options...
    QFont font;
    if (pSetup->sMessagesFont.isEmpty() || !font.fromString(pSetup->sMessagesFont))
        font = QFont("Terminal", 8);
    MessagesFontTextLabel->setFont(font);
    MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
}


// Populate (save) setup struct members from dialog controls.
void qsynthSetupForm::save ( qsynthSetup *pSetup )
{
    // Save Options...
    pSetup->sMessagesFont = MessagesFontTextLabel->font().toString();
}


// end of qsynthSetupForm.ui.h
