// qsynthOptionsForm.cpp
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
#include "qsynthOptionsForm.h"

#include "qsynthOptions.h"

#include <QValidator>
#include <QMessageBox>
#include <QFontDialog>


//----------------------------------------------------------------------------
// qsynthOptionsForm -- UI wrapper form.

// Constructor.
qsynthOptionsForm::qsynthOptionsForm (
	QWidget *pParent, Qt::WFlags wflags ) : QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// No options descriptor initially (the caller will set it).
	m_pOptions = NULL;

	// Initialize dirty control state.
	m_iDirtySetup = 0;
	m_iDirtyCount = 0;

	// Set dialog validators...
	m_ui.MessagesLimitLinesComboBox->setValidator(new QIntValidator(m_ui.MessagesLimitLinesComboBox));

	// Try to restore old window positioning.
	adjustSize();

	// UI connections...
	QObject::connect(m_ui.MessagesFontPushButton,
		SIGNAL(clicked()),
		SLOT(chooseMessagesFont()));
	QObject::connect(m_ui.QueryCloseCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.KeepOnTopCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.StdoutCaptureCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.OutputMetersCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.SystemTrayCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesLimitCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesLimitLinesComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.OkPushButton,
		SIGNAL(clicked()),
		SLOT(accept()));
	QObject::connect(m_ui.CancelPushButton,
		SIGNAL(clicked()),
		SLOT(reject()));
}


// Destructor.
qsynthOptionsForm::~qsynthOptionsForm (void)
{
}


// Populate (setup) dialog controls from options descriptors.
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
	// Messages font.
	if (m_pOptions->sMessagesFont.isEmpty()
		|| !font.fromString(m_pOptions->sMessagesFont))
		font = QFont("Monospace", 8);
	QPalette pal(m_ui.MessagesFontTextLabel->palette());
	pal.setColor(m_ui.MessagesFontTextLabel->backgroundRole(), Qt::white);
	m_ui.MessagesFontTextLabel->setPalette(pal);
	m_ui.MessagesFontTextLabel->setFont(font);
	m_ui.MessagesFontTextLabel->setText(
		font.family() + " " + QString::number(font.pointSize()));

	// Messages limit option.
	m_ui.MessagesLimitCheckBox->setChecked(m_pOptions->bMessagesLimit);
	m_ui.MessagesLimitLinesComboBox->setItemText(
		m_ui.MessagesLimitLinesComboBox->currentIndex(),
			QString::number(m_pOptions->iMessagesLimitLines));

	// Other options finally.
	m_ui.QueryCloseCheckBox->setChecked(m_pOptions->bQueryClose);
	m_ui.KeepOnTopCheckBox->setChecked(m_pOptions->bKeepOnTop);
	m_ui.StdoutCaptureCheckBox->setChecked(m_pOptions->bStdoutCapture);
	m_ui.OutputMetersCheckBox->setChecked(m_pOptions->bOutputMeters);
	m_ui.SystemTrayCheckBox->setChecked(m_pOptions->bSystemTray);

#if defined(WIN32)
	m_ui.StdoutCaptureCheckBox->setChecked(false);
	m_ui.StdoutCaptureCheckBox->setEnabled(false);
#endif

#ifndef CONFIG_SYSTEM_TRAY
	m_ui.SystemTrayCheckBox->setChecked(false);
	m_ui.SystemTrayCheckBox->setEnabled(false);
#endif

	// Done.
	m_iDirtySetup--;
	stabilizeForm();
}


// Accept options (OK button slot).
void qsynthOptionsForm::accept (void)
{
	// Save options...
	if (m_iDirtyCount > 0) {
		m_pOptions->sMessagesFont       = m_ui.MessagesFontTextLabel->font().toString();
		m_pOptions->bMessagesLimit      = m_ui.MessagesLimitCheckBox->isChecked();
		m_pOptions->iMessagesLimitLines = m_ui.MessagesLimitLinesComboBox->currentText().toInt();
		m_pOptions->bQueryClose         = m_ui.QueryCloseCheckBox->isChecked();
		m_pOptions->bKeepOnTop          = m_ui.KeepOnTopCheckBox->isChecked();
		m_pOptions->bStdoutCapture      = m_ui.StdoutCaptureCheckBox->isChecked();
		m_pOptions->bOutputMeters       = m_ui.OutputMetersCheckBox->isChecked();
		m_pOptions->bSystemTray         = m_ui.SystemTrayCheckBox->isChecked();
		// Reset dirty flag.
		m_iDirtyCount = 0;
	}

	// Just go with dialog acceptance.
	QDialog::accept();
}


// Reject options (Cancel button slot).
void qsynthOptionsForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			QSYNTH_TITLE ": " + tr("Warning"),
			tr("Some options have been changed.") + "\n\n" +
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


// Dirty up options.
void qsynthOptionsForm::optionsChanged()
{
	if (m_iDirtySetup > 0)
		return;

	m_iDirtyCount++;
	stabilizeForm();
}


// Stabilize current form state.
void qsynthOptionsForm::stabilizeForm()
{
	m_ui.MessagesLimitLinesComboBox->setEnabled(m_ui.MessagesLimitCheckBox->isChecked());

	m_ui.OkPushButton->setEnabled(m_iDirtyCount > 0);
}


// The messages font selection dialog.
void qsynthOptionsForm::chooseMessagesFont()
{
	bool  bOk  = false;
	QFont font = QFontDialog::getFont(&bOk, m_ui.MessagesFontTextLabel->font(), this);
	if (bOk) {
		m_ui.MessagesFontTextLabel->setFont(font);
		m_ui.MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
		optionsChanged();
	}
}


// end of qsynthOptionsForm.cpp
