// qsynthOptionsForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2003-2004, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include <qmessagebox.h>
#include <qfontdialog.h>

#include "config.h"


// Kind of constructor.
void qsynthOptionsForm::init (void)
{
    // No settings descriptor initially (the caller will set it).
    m_pOptions = NULL;

    // Initialize dirty control state.
    m_iDirtySetup = 0;
    m_iDirtyCount = 0;

    // Set dialog validators...
    MessagesLimitLinesComboBox->setValidator(new QIntValidator(MessagesLimitLinesComboBox));

    // Try to restore old window positioning.
    adjustSize();
}


// Kind of destructor.
void qsynthOptionsForm::destroy (void)
{
}


// Populate (setup) dialog controls from settings descriptors.
void qsynthOptionsForm::setup ( qsynthOptions *pOptions )
{
    // Set reference descriptor.
    m_pOptions = pOptions;

    // Start clean.
    m_iDirtyCount = 0;
    // Avoid nested changes.
    m_iDirtySetup++;

    // Load Display options...
    QFont font;
    if (m_pOptions->sMessagesFont.isEmpty() || !font.fromString(m_pOptions->sMessagesFont))
        font = QFont("Fixed", 8);
    MessagesFontTextLabel->setFont(font);
    MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));

    // Messages limit option.
    MessagesLimitCheckBox->setChecked(m_pOptions->bMessagesLimit);
    MessagesLimitLinesComboBox->setCurrentText(QString::number(m_pOptions->iMessagesLimitLines));

    // Other options finally.
    QueryCloseCheckBox->setChecked(m_pOptions->bQueryClose);
    KeepOnTopCheckBox->setChecked(m_pOptions->bKeepOnTop);
    StdoutCaptureCheckBox->setChecked(m_pOptions->bStdoutCapture);

    // Done.
    m_iDirtySetup--;
    stabilizeForm();
}


// Accept settings (OK button slot).
void qsynthOptionsForm::accept (void)
{
    // Save options...
    if (m_iDirtyCount > 0) {
        m_pOptions->sMessagesFont        = MessagesFontTextLabel->font().toString();
        m_pOptions->bMessagesLimit       = MessagesLimitCheckBox->isChecked();
        m_pOptions->iMessagesLimitLines  = MessagesLimitLinesComboBox->currentText().toInt();
        m_pOptions->bQueryClose          = QueryCloseCheckBox->isChecked();
        m_pOptions->bKeepOnTop           = KeepOnTopCheckBox->isChecked();
        m_pOptions->bStdoutCapture       = StdoutCaptureCheckBox->isChecked();
        // Reset dirty flag.
        m_iDirtyCount = 0;
    }

    // Just go with dialog acceptance.
    QDialog::accept();
}


// Reject settings (Cancel button slot).
void qsynthOptionsForm::reject (void)
{
    bool bReject = true;

    // Check if there's any pending changes...
    if (m_iDirtyCount > 0) {
        switch (QMessageBox::warning(this, tr("Warning"),
            tr("Some settings have been changed.") + "\n\n" +
            tr("Do you want to apply the changes?"),
            tr("Apply"), tr("Discard"), tr("Cancel"))) {
        case 0:     // Apply...
            accept();
            return;
        case 1:     // Discard
            break;
        default:    // Cancel.
            bReject = false;
        }
    }

    if (bReject)
        QDialog::reject();
}


// Dirty up settings.
void qsynthOptionsForm::settingsChanged()
{
    if (m_iDirtySetup > 0)
        return;

    m_iDirtyCount++;
    stabilizeForm();
}


// Stabilize current form state.
void qsynthOptionsForm::stabilizeForm()
{
    MessagesLimitLinesComboBox->setEnabled(MessagesLimitCheckBox->isChecked());

    OkPushButton->setEnabled(m_iDirtyCount > 0);
}


// The messages font selection dialog.
void qsynthOptionsForm::chooseMessagesFont()
{
    bool  bOk  = false;
    QFont font = QFontDialog::getFont(&bOk, MessagesFontTextLabel->font(), this);
    if (bOk) {
        MessagesFontTextLabel->setFont(font);
        MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
        settingsChanged();
    }
}


// end of qsynthOptionsForm.ui.h

