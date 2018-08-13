// qsynthChannelsForm.cpp
//
/****************************************************************************
   Copyright (C) 2003-2013, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qsynthChannelsForm.h"

#include "qsynthChannels.h"
#include "qsynthEngine.h"

#include "qsynthMainForm.h"
#include "qsynthPresetForm.h"

#include <QValidator>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileInfo>

#include <QShowEvent>
#include <QHideEvent>
#include <QPixmap>
#include <QMenu>


//----------------------------------------------------------------------------
// qsynthChannelsForm -- UI wrapper form.

// Constructor.
qsynthChannelsForm::qsynthChannelsForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QWidget(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	m_iChannels  = 0;
	m_ppChannels = NULL;

	// No setup synth references initially (the caller will set them).
	m_pOptions = NULL;
	m_pEngine  = NULL;
	m_pSynth   = NULL;

	// Initialize dirty control state.
	m_iDirtySetup = 0;
	m_iDirtyCount = 0;

	// Our activity leds (same of main form :).
	m_pXpmLedOn  = new QPixmap(":/images/ledon1.png");
	m_pXpmLedOff = new QPixmap(":/images/ledoff1.png");

	// Set validators...
	m_ui.PresetComboBox->setValidator(
		new QRegExpValidator(QRegExp("[\\w-]+"), m_ui.PresetComboBox));

	// Soundfonts list view...
	QHeaderView *pHeader = m_ui.ChannelsListView->header();
	pHeader->setDefaultAlignment(Qt::AlignLeft);
//	pHeader->setDefaultSectionSize(320);
#if QT_VERSION >= 0x050000
//	pHeader->setSectionResizeMode(QHeaderView::Custom);
	pHeader->setSectionsMovable(false);
#else
//	pHeader->setResizeMode(QHeaderView::Custom);
	pHeader->setMovable(false);
#endif
	pHeader->setStretchLastSection(true);

	pHeader->resizeSection(0, 24);						// In.
	m_ui.ChannelsListView->resizeColumnToContents(1);	// Ch.
	m_ui.ChannelsListView->resizeColumnToContents(2);	// Bank.
	m_ui.ChannelsListView->resizeColumnToContents(3);	// Prog.
	pHeader->resizeSection(4, 320);						// Name.
	m_ui.ChannelsListView->resizeColumnToContents(5);	// SFID.
	m_ui.ChannelsListView->resizeColumnToContents(6);	// Soundfont.

	// Initial sort order...
	m_ui.ChannelsListView->sortItems(1, Qt::AscendingOrder);

	// UI connections...
	QObject::connect(m_ui.PresetComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(changePreset(const QString&)));
	QObject::connect(m_ui.PresetSavePushButton,
		SIGNAL(clicked()),
		SLOT(savePreset()));
	QObject::connect(m_ui.PresetDeletePushButton,
		SIGNAL(clicked()),
		SLOT(deletePreset()));
	QObject::connect(m_ui.ChannelsListView,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(contextMenuRequested(const QPoint&)));
//	QObject::connect(m_ui.ChannelsListView,
//		SIGNAL(itemActivated(QTreeWidgetItem*,int)),
//		SLOT(itemActivated(QTreeWidgetItem*,int)));
	QObject::connect(m_ui.ChannelsListView,
		SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		SLOT(itemActivated(QTreeWidgetItem*,int)));
}


// Destructor.
qsynthChannelsForm::~qsynthChannelsForm (void)
{
	// Nullify references.
	setup(NULL, NULL, false);

	// Delete pixmaps.
	delete m_pXpmLedOn;
	delete m_pXpmLedOff;
}


// Notify our parent that we're emerging.
void qsynthChannelsForm::showEvent ( QShowEvent *pShowEvent )
{
	qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
	if (pMainForm)
		pMainForm->stabilizeForm();

	QWidget::showEvent(pShowEvent);
}

// Notify our parent that we're closing.
void qsynthChannelsForm::hideEvent ( QHideEvent *pHideEvent )
{
	QWidget::hideEvent(pHideEvent);

	qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
	if (pMainForm)
		pMainForm->stabilizeForm();
}

// Just about to notify main-window that we're closing.
void qsynthChannelsForm::closeEvent ( QCloseEvent * /*pCloseEvent*/ )
{
	QWidget::hide();

	qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
	if (pMainForm)
		pMainForm->stabilizeForm();
}


// Populate (setup) synth settings descriptors.
void qsynthChannelsForm::setup ( qsynthOptions *pOptions,
	qsynthEngine *pEngine, bool bPreset )
{
	// Set the proper descriptors.
	m_pOptions = pOptions;
	m_pEngine  = pEngine;
	m_pSynth   = pEngine ? pEngine->pSynth : NULL;

	// Update caption.
	QString sTitle = QSYNTH_TITLE ": " + tr("Channels");
	if (pEngine)
		sTitle += " [" + pEngine->name() + "]";
	setWindowTitle(sTitle);

	// Free up current channel list view.
	if (m_ppChannels) {
		delete [] m_ppChannels;
		m_ppChannels = NULL;
		m_iChannels  = 0;
	}

	// Allocate a new channel list view...
	m_ui.ChannelsListView->clear();
	if (m_pSynth && m_ppChannels == NULL) {
		m_iChannels = ::fluid_synth_count_midi_channels(m_pSynth);
		if (m_iChannels > 0)
			m_ppChannels = new qsynthChannelsItemPtr [m_iChannels];
		if (m_ppChannels) {
			for (int iChan = 0; iChan < m_iChannels; iChan++) {
				qsynthChannelsItem *pItem = new qsynthChannelsItem(
					m_ui.ChannelsListView);
				if (pItem) {
					pItem->setIcon(QSYNTH_CHANNELS_IN,
						*m_pXpmLedOff);
					pItem->setText(QSYNTH_CHANNELS_CHAN,
						QString::number(iChan + 1));
				}
				m_ppChannels[iChan] = pItem;
			}
		}
		// Load preset list...
		m_iDirtySetup++;
		resetPresets();
		m_ui.PresetComboBox->setEditText((m_pEngine->setup())->sDefPreset);
		m_iDirtySetup--;
		// Load default preset and update/refresh the whole thing...
		resetAllChannels(bPreset);
	}
}


// Channel item update.
void qsynthChannelsForm::updateChannel ( int iChan )
{
	if (m_pSynth == NULL || m_ppChannels == NULL)
		return;
	if (iChan < 0 || iChan >= m_iChannels)
		return;

	qsynthChannelsItem *pItem = m_ppChannels[iChan];

	const QString n = "-";
#ifdef CONFIG_FLUID_CHANNEL_INFO
	fluid_synth_channel_info_t info;
	::memset(&info, 0, sizeof(info));
	::fluid_synth_get_channel_info(m_pSynth, iChan, &info);
	if (info.assigned) {
	#ifdef CONFIG_FLUID_BANK_OFFSET
		info.bank += ::fluid_synth_get_bank_offset(m_pSynth, info.sfont_id);
	#endif
		pItem->setText(QSYNTH_CHANNELS_BANK,
			QString::number(info.bank));
		pItem->setText(QSYNTH_CHANNELS_PROG,
			QString::number(info.program));
		pItem->setText(QSYNTH_CHANNELS_NAME,
			info.name);
		pItem->setText(QSYNTH_CHANNELS_SFID,
			QString::number(info.sfont_id));
		fluid_sfont_t *sfont
			= ::fluid_synth_get_sfont_by_id(m_pSynth, info.sfont_id);
		pItem->setText(QSYNTH_CHANNELS_SFNAME,
			sfont ? QFileInfo(sfont->get_name(sfont)).baseName() : n);
		// Make this a dirty-operation.
		m_iDirtyCount++;
	} 
#else
	fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(m_pSynth, iChan);
	if (pPreset) {
		int iBank = ::fluid_preset_get_banknum(pPreset);
		fluid_sfont_t* sfont = ::fluid_preset_get_sfont(pPreset);
		int sfid = ::fluid_sfont_get_id(sfont);
	#ifdef CONFIG_FLUID_BANK_OFFSET
		iBank += ::fluid_synth_get_bank_offset(m_pSynth, sfid);
	#endif
		pItem->setText(QSYNTH_CHANNELS_BANK,
			QString::number(iBank));
		pItem->setText(QSYNTH_CHANNELS_PROG,
			QString::number(::fluid_preset_get_num(pPreset)));
		pItem->setText(QSYNTH_CHANNELS_NAME,
			::fluid_preset_get_name(pPreset));
		pItem->setText(QSYNTH_CHANNELS_SFID,
			QString::number(sfid));
		pItem->setText(QSYNTH_CHANNELS_SFNAME,
			QFileInfo(::fluid_sfont_get_name(sfont)).baseName());
		// Make this a dirty-operation.
		m_iDirtyCount++;
	}
#endif
	else {
		pItem->setText(QSYNTH_CHANNELS_BANK, n);
		pItem->setText(QSYNTH_CHANNELS_PROG, n);
		pItem->setText(QSYNTH_CHANNELS_NAME, n);
		pItem->setText(QSYNTH_CHANNELS_SFID, n);
		pItem->setText(QSYNTH_CHANNELS_SFNAME, n);
	}
}


// All channels update.
void qsynthChannelsForm::updateAllChannels (void)
{
	for (int iChan = 0; iChan < m_iChannels; iChan++)
		updateChannel(iChan);

	m_ui.ChannelsListView->update();

	stabilizeForm();
}


// All channels reset update.
void qsynthChannelsForm::resetAllChannels ( bool bPreset )
{
	if (m_pEngine == NULL)
		return;

	qsynthSetup *pSetup = m_pEngine->setup();
	if (pSetup == NULL)
		return;

	if (bPreset)
		changePreset(pSetup->sDefPreset);
	else
		updateAllChannels();
}


// Update channel activity status LED.
void qsynthChannelsForm::setChannelOn ( int iChan, bool bOn )
{
	if (m_ppChannels == NULL)
		return;
	if (iChan < 0 || iChan >= m_iChannels)
		return;

	m_ppChannels[iChan]->setIcon(QSYNTH_CHANNELS_IN,
		(bOn ? *m_pXpmLedOn : *m_pXpmLedOff));
}


// Channel view context menu handler.
void qsynthChannelsForm::contextMenuRequested ( const QPoint& pos )
{
	QTreeWidgetItem *pItem = m_ui.ChannelsListView->itemAt(pos);

	// Build the channel context menu...
	QMenu menu(this);
	QAction *pAction;

	bool bEnabled = (m_pSynth && pItem);
	pAction = menu.addAction(
		QIcon(":/images/edit1.png"),
		tr("Edit") + "...", this, SLOT(editSelectedChannel()));
	pAction->setEnabled(bEnabled);
#ifdef CONFIG_FLUID_UNSET_PROGRAM
	pAction = menu.addAction(
		QIcon(":/images/remove1.png"),
		tr("Unset"), this, SLOT(unsetSelectedChannel()));
#endif
	menu.addSeparator();
	pAction = menu.addAction(
		tr("Refresh"), this, SLOT(updateAllChannels()));
	pAction->setEnabled(bEnabled);

	menu.exec((m_ui.ChannelsListView->viewport())->mapToGlobal(pos));
}


// Edit detail dialog.
void qsynthChannelsForm::editSelectedChannel (void)
{
	itemActivated(m_ui.ChannelsListView->currentItem(), 0);
}


// Unset program slot.
void qsynthChannelsForm::unsetSelectedChannel (void)
{
	QTreeWidgetItem *pItem = m_ui.ChannelsListView->currentItem();
	if (pItem == NULL)
		return;
	if (m_ppChannels == NULL)
		return;
	if (m_pOptions == NULL || m_pEngine == NULL || m_pSynth == NULL)
		return;

	int iChan = (pItem->text(QSYNTH_CHANNELS_CHAN).toInt() - 1);
	if (iChan < 0 || iChan >= m_iChannels)
		return;

#ifdef CONFIG_FLUID_UNSET_PROGRAM
	::fluid_synth_unset_program(m_pSynth, iChan);
	// Make this a dirty-operation.
	m_iDirtyCount++;
#endif

	updateChannel(iChan);

	stabilizeForm();
}


// Show detail dialog.
void qsynthChannelsForm::itemActivated ( QTreeWidgetItem *pItem, int )
{
	if (pItem == NULL)
		return;
	if (m_ppChannels == NULL)
		return;
	if (m_pOptions == NULL || m_pEngine == NULL || m_pSynth == NULL)
		return;

	int iChan = (pItem->text(QSYNTH_CHANNELS_CHAN).toInt() - 1);
	if (iChan < 0 || iChan >= m_iChannels)
		return;

	qsynthPresetForm *pPresetForm = new qsynthPresetForm(this);
	if (pPresetForm) {
		// The the proper context.
		pPresetForm->setup(m_pOptions, m_pSynth, iChan);
		// Show the channel preset dialog...
		if (pPresetForm->exec())
			updateChannel(iChan);
		// Done.
		delete pPresetForm;
	}

	stabilizeForm();
}

// Channels preset naming slots.
void qsynthChannelsForm::changePreset ( const QString& sPreset )
{
	if (m_pOptions == NULL || m_pEngine == NULL || m_pSynth == NULL)
		return;
	if (m_iDirtySetup > 0)
		return;

	// Force this is pseudo-dirty procedure...
	m_iDirtyCount++;
	// Load presets and update/refresh the whole thing.
	if (m_pOptions->loadPreset(m_pEngine, sPreset)) {
		updateAllChannels();
		// Very special, make this the new default preset.
		(m_pEngine->setup())->sDefPreset = sPreset;
		// This is clean now, for sure.
		m_iDirtyCount = 0;
	}

	stabilizeForm();
}


void qsynthChannelsForm::savePreset (void)
{
	if (m_pOptions == NULL || m_pEngine == NULL || m_pSynth == NULL)
		return;

	QString sPreset = m_ui.PresetComboBox->currentText();
	if (sPreset.isEmpty())
		return;

	// Unload presets (from synth).
	if (m_pOptions->savePreset(m_pEngine, sPreset)) {
		m_iDirtySetup++;
		resetPresets();
		m_ui.PresetComboBox->setEditText(sPreset);
		m_iDirtySetup--;
		// Again special, force this the default preset.
		(m_pEngine->setup())->sDefPreset = sPreset;
		// Not dirty anymore, by definition.
		m_iDirtyCount = 0;
	}

	stabilizeForm();
}


void qsynthChannelsForm::deletePreset (void)
{
	if (m_pOptions == NULL || m_pEngine == NULL || m_pSynth == NULL)
		return;

	QString sPreset = m_ui.PresetComboBox->currentText();
	if (sPreset.isEmpty())
		return;

	// Try to prompt user if he/she really wants this...
	if (QMessageBox::warning(this,
		QSYNTH_TITLE ": " + tr("Warning"),
		tr("Delete preset:") + "\n\n" +
		sPreset + "\n\n" +
		tr("Are you sure?"),
		QMessageBox::Ok | QMessageBox::Cancel)
		== QMessageBox::Cancel)
		return;

	// Remove current preset item...
	m_iDirtySetup++;
	int iItem = m_ui.PresetComboBox->currentIndex();
	m_pOptions->deletePreset(m_pEngine, sPreset);
	resetPresets();
	m_ui.PresetComboBox->setCurrentIndex(iItem);
	m_iDirtySetup--;

	// Load a new preset...
	changePreset(m_ui.PresetComboBox->currentText());
}


void qsynthChannelsForm::resetPresets (void)
{
	if (m_pEngine == NULL)
		return;

	qsynthSetup *pSetup = m_pEngine->setup();
	if (pSetup == NULL)
		return;

	m_ui.PresetComboBox->clear();
	m_ui.PresetComboBox->addItems(pSetup->presets);
	m_ui.PresetComboBox->addItem(pSetup->sDefPresetName);
}


// Stabilize current form state.
void qsynthChannelsForm::stabilizeForm (void)
{
	if (m_pEngine == NULL)
		return;

	qsynthSetup *pSetup = m_pEngine->setup();
	if (pSetup == NULL)
		return;

	QString sPreset = m_ui.PresetComboBox->currentText();
	if (m_pSynth && !sPreset.isEmpty()) {
		bool bPreset = pSetup->presets.contains(sPreset);
		m_ui.PresetSavePushButton->setEnabled(m_iDirtyCount > 0
			|| (!bPreset && sPreset != pSetup->sDefPresetName));
		m_ui.PresetDeletePushButton->setEnabled(bPreset);
	} else {
		m_ui.PresetSavePushButton->setEnabled(false);
		m_ui.PresetDeletePushButton->setEnabled(false);
	}
}


// end of qsynthChannelsForm.cpp
