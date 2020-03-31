// qsynthOptionsForm.cpp
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
#include "qsynthOptionsForm.h"

#include "qsynthPaletteForm.h"

#include "qsynthOptions.h"

#include <QValidator>
#include <QMessageBox>
#include <QFontDialog>
#include <QFileDialog>

#include <QStyleFactory>

#ifdef CONFIG_SYSTEM_TRAY
#include <QSystemTrayIcon>
#endif


// Default (empty/blank) name.
static const char *g_pszDefName = QT_TRANSLATE_NOOP("qsynthOptionsForm", "(default)");


//----------------------------------------------------------------------------
// qsynthOptionsForm -- UI wrapper form.

// Constructor.
qsynthOptionsForm::qsynthOptionsForm ( QWidget *pParent )
	: QDialog(pParent)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// No options descriptor initially (the caller will set it).
	m_pOptions = nullptr;

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
	QObject::connect(m_ui.MessagesLogCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesLogPathComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesLogPathToolButton,
		SIGNAL(clicked()),
		SLOT(browseMessagesLogPath()));
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
#ifdef CONFIG_SYSTEM_TRAY
	QObject::connect(m_ui.SystemTrayCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.SystemTrayQueryCloseCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.StartMinimizedCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
#endif
	QObject::connect(m_ui.CustomColorThemeComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.CustomColorThemeToolButton,
		SIGNAL(clicked()),
		SLOT(editCustomColorThemes()));
	QObject::connect(m_ui.CustomStyleThemeComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.BaseFontSizeComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesLimitCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.MessagesLimitLinesComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.KnobStyleComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.KnobMouseMotionComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
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

	// Load combo box history...
	m_pOptions->loadComboBoxHistory(m_ui.MessagesLogPathComboBox);

	// Load Display options...
	QFont font;
	// Messages font.
	if (m_pOptions->sMessagesFont.isEmpty()
		|| !font.fromString(m_pOptions->sMessagesFont))
		font = QFont("Monospace", 8);
	QPalette pal(m_ui.MessagesFontTextLabel->palette());
	pal.setColor(QPalette::Background, pal.base().color());
	m_ui.MessagesFontTextLabel->setPalette(pal);
	m_ui.MessagesFontTextLabel->setFont(font);
	m_ui.MessagesFontTextLabel->setText(
		font.family() + ' ' + QString::number(font.pointSize()));

	// Messages limit option.
	m_ui.MessagesLimitCheckBox->setChecked(m_pOptions->bMessagesLimit);
	m_ui.MessagesLimitLinesComboBox->setEditText(
		QString::number(m_pOptions->iMessagesLimitLines));

	// Logging options
	m_ui.MessagesLogCheckBox->setChecked(m_pOptions->bMessagesLog);
	m_ui.MessagesLogPathComboBox->setEditText(m_pOptions->sMessagesLogPath);

	// Other options finally.
	m_ui.QueryCloseCheckBox->setChecked(m_pOptions->bQueryClose);
	m_ui.KeepOnTopCheckBox->setChecked(m_pOptions->bKeepOnTop);
	m_ui.StdoutCaptureCheckBox->setChecked(m_pOptions->bStdoutCapture);
	m_ui.OutputMetersCheckBox->setChecked(m_pOptions->bOutputMeters);
#ifdef CONFIG_SYSTEM_TRAY
	m_ui.SystemTrayCheckBox->setChecked(m_pOptions->bSystemTray);
	m_ui.SystemTrayQueryCloseCheckBox->setChecked(m_pOptions->bSystemTrayQueryClose);
	m_ui.StartMinimizedCheckBox->setChecked(m_pOptions->bStartMinimized);
#endif
	if (m_pOptions->iBaseFontSize > 0) {
		m_ui.BaseFontSizeComboBox->setEditText(
			QString::number(m_pOptions->iBaseFontSize));
	} else {
		m_ui.BaseFontSizeComboBox->setCurrentIndex(0);
	}

	// Knobs
	m_ui.KnobStyleComboBox->setCurrentIndex(m_pOptions->iKnobStyle);
	m_ui.KnobMouseMotionComboBox->setCurrentIndex(m_pOptions->iKnobMotion);

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
	m_ui.StdoutCaptureCheckBox->setChecked(false);
	m_ui.StdoutCaptureCheckBox->setEnabled(false);
#endif

#ifdef CONFIG_SYSTEM_TRAY
	const bool bSystemTray = QSystemTrayIcon::isSystemTrayAvailable();
#else
	const bool bSystemTray = false;
#endif
	if (!bSystemTray) {
		m_ui.SystemTrayCheckBox->setChecked(false);
		m_ui.SystemTrayCheckBox->setEnabled(false);
		m_ui.SystemTrayQueryCloseCheckBox->setChecked(false);
		m_ui.SystemTrayQueryCloseCheckBox->setEnabled(false);
		m_ui.StartMinimizedCheckBox->setChecked(false);
		m_ui.StartMinimizedCheckBox->setEnabled(false);
	}

	// Custom display options...
	resetCustomColorThemes(m_pOptions->sCustomColorTheme);
	resetCustomStyleThemes(m_pOptions->sCustomStyleTheme);

	// Done.
	m_iDirtySetup--;
	stabilizeForm();
}


// Accept options (OK button slot).
void qsynthOptionsForm::accept (void)
{
	// Save options...
	if (m_iDirtyCount > 0) {
		const bool bOldStdoutCapture = m_pOptions->bStdoutCapture;
		const bool bOldKeepOnTop     = m_pOptions->bKeepOnTop;
		const int  iOldBaseFontSize  = m_pOptions->iBaseFontSize;
		m_pOptions->sMessagesFont   = m_ui.MessagesFontTextLabel->font().toString();
		m_pOptions->bMessagesLimit  = m_ui.MessagesLimitCheckBox->isChecked();
		m_pOptions->iMessagesLimitLines = m_ui.MessagesLimitLinesComboBox->currentText().toInt();
		m_pOptions->bMessagesLog    = m_ui.MessagesLogCheckBox->isChecked();
		m_pOptions->sMessagesLogPath = m_ui.MessagesLogPathComboBox->currentText();
		m_pOptions->bQueryClose     = m_ui.QueryCloseCheckBox->isChecked();
		m_pOptions->bKeepOnTop      = m_ui.KeepOnTopCheckBox->isChecked();
		m_pOptions->bStdoutCapture  = m_ui.StdoutCaptureCheckBox->isChecked();
		m_pOptions->bOutputMeters   = m_ui.OutputMetersCheckBox->isChecked();
	#ifdef CONFIG_SYSTEM_TRAY
		m_pOptions->bSystemTray     = m_ui.SystemTrayCheckBox->isChecked();
		m_pOptions->bSystemTrayQueryClose = m_ui.SystemTrayQueryCloseCheckBox->isChecked();
		m_pOptions->bStartMinimized = m_ui.StartMinimizedCheckBox->isChecked();
	#endif
		m_pOptions->iBaseFontSize   = m_ui.BaseFontSizeComboBox->currentText().toInt();
		// Knobs
		m_pOptions->iKnobStyle      = m_ui.KnobStyleComboBox->currentIndex();
		m_pOptions->iKnobMotion     = m_ui.KnobMouseMotionComboBox->currentIndex();
		// Custom color/style theme options...
		const QString sOldCustomStyleTheme = m_pOptions->sCustomStyleTheme;
		if (m_ui.CustomStyleThemeComboBox->currentIndex() > 0)
			m_pOptions->sCustomStyleTheme = m_ui.CustomStyleThemeComboBox->currentText();
		else
			m_pOptions->sCustomStyleTheme.clear();
		const QString sOldCustomColorTheme = m_pOptions->sCustomColorTheme;
		if (m_ui.CustomColorThemeComboBox->currentIndex() > 0)
			m_pOptions->sCustomColorTheme = m_ui.CustomColorThemeComboBox->currentText();
		else
			m_pOptions->sCustomColorTheme.clear();
		// Check whether restart is needed or whether
		// custom options maybe set up immediately...
		int iNeedRestart = 0;
		if (m_pOptions->sCustomStyleTheme != sOldCustomStyleTheme) {
			if (m_pOptions->sCustomStyleTheme.isEmpty()) {
				++iNeedRestart;
			} else {
				QApplication::setStyle(
					QStyleFactory::create(m_pOptions->sCustomStyleTheme));
			}
		}
		if (m_pOptions->sCustomColorTheme != sOldCustomColorTheme) {
			if (m_pOptions->sCustomColorTheme.isEmpty()) {
				++iNeedRestart;
			} else {
				QPalette pal;
				if (qsynthPaletteForm::namedPalette(
						&m_pOptions->settings(), m_pOptions->sCustomColorTheme, pal))
					QApplication::setPalette(pal);
			}
		}
		// Warn if something will be only effective on next run...
		if (( bOldStdoutCapture && !m_pOptions->bStdoutCapture) ||
			(!bOldStdoutCapture &&  m_pOptions->bStdoutCapture) ||
			( bOldKeepOnTop     && !m_pOptions->bKeepOnTop)     ||
			(!bOldKeepOnTop     &&  m_pOptions->bKeepOnTop)     ||
			(iOldBaseFontSize != m_pOptions->iBaseFontSize)) {
			++iNeedRestart;
		}
		// Show restart message if needed...
		if (iNeedRestart > 0) {
			QMessageBox::information(this,
				tr("Information"),
				tr("Some settings may be only effective\n"
				"next time you start this application."));
		}
		// Reset dirty flag.
		m_iDirtyCount = 0;
	}

	// Save combobox history...
	m_pOptions->saveComboBoxHistory(m_ui.MessagesLogPathComboBox);

	// Save/commit to disk.
	m_pOptions->saveOptions();

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
			tr("Warning"),
			tr("Some options have been changed.") + "\n\n" +
			tr("Do you want to apply the changes?"),
			QMessageBox::Apply | QMessageBox::Discard | QMessageBox::Cancel)) {
		case QMessageBox::Apply:
			accept();
			return;
		case QMessageBox::Discard:
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


// Custom color palette theme manager.
void qsynthOptionsForm::editCustomColorThemes (void)
{
	qsynthPaletteForm form(this);
	form.setSettings(&m_pOptions->settings());

	QString sCustomColorTheme;
	int iDirtyCustomColorTheme = 0;

	const int iCustomColorTheme
		= m_ui.CustomColorThemeComboBox->currentIndex();
	if (iCustomColorTheme > 0) {
		sCustomColorTheme = m_ui.CustomColorThemeComboBox->itemText(
			iCustomColorTheme);
		form.setPaletteName(sCustomColorTheme);
	}

	if (form.exec() == QDialog::Accepted) {
		sCustomColorTheme = form.paletteName();
		++iDirtyCustomColorTheme;
	}

	if (iDirtyCustomColorTheme > 0 || form.isDirty()) {
		resetCustomColorThemes(sCustomColorTheme);
		optionsChanged();
	}
}


// Custom color palette themes settler.
void qsynthOptionsForm::resetCustomColorThemes (
	const QString& sCustomColorTheme )
{
	m_ui.CustomColorThemeComboBox->clear();
	m_ui.CustomColorThemeComboBox->addItem(
		tr(g_pszDefName));
	m_ui.CustomColorThemeComboBox->addItems(
		qsynthPaletteForm::namedPaletteList(&m_pOptions->settings()));

	int iCustomColorTheme = 0;
	if (!sCustomColorTheme.isEmpty()) {
		iCustomColorTheme = m_ui.CustomColorThemeComboBox->findText(
			sCustomColorTheme);
		if (iCustomColorTheme < 0)
			iCustomColorTheme = 0;
	}
	m_ui.CustomColorThemeComboBox->setCurrentIndex(iCustomColorTheme);
}


// Custom widget style themes settler.
void qsynthOptionsForm::resetCustomStyleThemes (
	const QString& sCustomStyleTheme )
{
	m_ui.CustomStyleThemeComboBox->clear();
	m_ui.CustomStyleThemeComboBox->addItem(
		tr(g_pszDefName));
	m_ui.CustomStyleThemeComboBox->addItems(QStyleFactory::keys());

	int iCustomStyleTheme = 0;
	if (!sCustomStyleTheme.isEmpty()) {
		iCustomStyleTheme = m_ui.CustomStyleThemeComboBox->findText(
			sCustomStyleTheme);
		if (iCustomStyleTheme < 0)
			iCustomStyleTheme = 0;
	}
	m_ui.CustomStyleThemeComboBox->setCurrentIndex(iCustomStyleTheme);
}


// Stabilize current form state.
void qsynthOptionsForm::stabilizeForm()
{
	bool bValid = (m_iDirtyCount > 0);

	m_ui.MessagesLimitLinesComboBox->setEnabled(
		m_ui.MessagesLimitCheckBox->isChecked());

	bool bEnabled = m_ui.MessagesLogCheckBox->isChecked();
	m_ui.MessagesLogPathComboBox->setEnabled(bEnabled);
	m_ui.MessagesLogPathToolButton->setEnabled(bEnabled);
	if (bEnabled && bValid) {
		const QString& sPath = m_ui.MessagesLogPathComboBox->currentText();
		bValid = !sPath.isEmpty();
	}

#ifdef CONFIG_SYSTEM_TRAY
	bEnabled = m_ui.SystemTrayCheckBox->isChecked();
	m_ui.SystemTrayQueryCloseCheckBox->setEnabled(bEnabled);
	m_ui.StartMinimizedCheckBox->setEnabled(bEnabled);
#endif

	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bValid);
}


// Messages log path browse slot.
void qsynthOptionsForm::browseMessagesLogPath()
{
	QString sFileName = QFileDialog::getSaveFileName(
		this,											// Parent.
		tr("Messages Log"),				                // Caption.
		m_ui.MessagesLogPathComboBox->currentText(),	// Start here.
		tr("Log files") + " (*.log)"	                // Filter (log files)
	);

	if (!sFileName.isEmpty()) {
		m_ui.MessagesLogPathComboBox->setEditText(sFileName);
		m_ui.MessagesLogPathComboBox->setFocus();
		optionsChanged();
	}
}


// The messages font selection dialog.
void qsynthOptionsForm::chooseMessagesFont()
{
	bool  bOk  = false;
	QFont font = QFontDialog::getFont(&bOk, m_ui.MessagesFontTextLabel->font(), this);
	if (bOk) {
		m_ui.MessagesFontTextLabel->setFont(font);
		m_ui.MessagesFontTextLabel->setText(
			font.family() + ' ' + QString::number(font.pointSize()));
		optionsChanged();
	}
}


// end of qsynthOptionsForm.cpp
