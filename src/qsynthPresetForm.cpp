// qsynthPresetForm.cpp
//
/****************************************************************************
   Copyright (C) 2003-2018, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qsynthPresetForm.h"

#include "qsynthOptions.h"

#include <QHeaderView>
#include <QPushButton>
#include <QFileInfo>


// Custom list-view item (as for numerical sort purposes...)
class qsynthPresetItem : public QTreeWidgetItem
{
public:

	// Constructor.
	qsynthPresetItem(QTreeWidget *pListView,
		QTreeWidgetItem *pItemAfter)
		: QTreeWidgetItem(pListView, pItemAfter) {}

	// Sort/compare overriden method.
	bool operator< (const QTreeWidgetItem& other) const
	{
		int iColumn = QTreeWidgetItem::treeWidget()->sortColumn();
		const QString& s1 = text(iColumn);
		const QString& s2 = other.text(iColumn);
		if (iColumn == 0 || iColumn == 2) {
			return (s1.toInt() < s2.toInt());
		} else {
			return (s1 < s2);
		}
	}
};


//----------------------------------------------------------------------------
// qsynthPresetForm -- UI wrapper form.

// Constructor.
qsynthPresetForm::qsynthPresetForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	m_pSynth = NULL;
	m_iChan  = 0;
	m_iBank  = 0;
	m_iProg  = 0;

	// To avoid setup jitterness and preview side effects.
	m_iDirtySetup = 0;
	m_iDirtyCount = 0;

	// Some default sorting, initially.
	//m_ui.BankListView->setSorting(0);
	//m_ui.ProgListView->setSorting(0);

	// Soundfonts list view...
	QHeaderView *pHeader = m_ui.ProgListView->header();
	pHeader->setDefaultAlignment(Qt::AlignLeft);
//	pHeader->setDefaultSectionSize(200);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
//	pHeader->setSectionResizeMode(QHeaderView::Custom);
	pHeader->setSectionsMovable(false);
#else
//	pHeader->setResizeMode(QHeaderView::Custom);
	pHeader->setMovable(false);
#endif
	pHeader->setStretchLastSection(true);

	m_ui.ProgListView->resizeColumnToContents(0);	// Prog.
	pHeader->resizeSection(1, 200);					// Name.
	m_ui.ProgListView->resizeColumnToContents(2);	// SFID.
	m_ui.ProgListView->resizeColumnToContents(3);	// Soundfont.

	// Initial sort order...
	m_ui.BankListView->sortItems(0, Qt::AscendingOrder);
	m_ui.ProgListView->sortItems(0, Qt::AscendingOrder);

	// UI connections...
	QObject::connect(m_ui.BankListView,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		SLOT(bankChanged()));
	QObject::connect(m_ui.ProgListView,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		SLOT(progChanged()));
	QObject::connect(m_ui.PreviewCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(previewChanged()));
//	QObject::connect(m_ui.ProgListView,
//		SIGNAL(itemActivated(QTreeWidgetItem*,int)),
//		SLOT(accept()));
	QObject::connect(m_ui.ProgListView,
		SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));
}


// Destructor.
qsynthPresetForm::~qsynthPresetForm (void)
{
}


// Dialog setup loader.
void qsynthPresetForm::setup ( qsynthOptions *pOptions, fluid_synth_t *pSynth, int iChan )
{
	// Set our internal stuff...
	m_pOptions = pOptions;
	m_pSynth = pSynth;
	m_iChan  = iChan;

	// We'll goinfg to changes the whole thing...
	m_iDirtySetup++;

	// Set the proper caption...
	setWindowTitle(QSYNTH_TITLE ": "
		+ tr("Channel %1").arg(m_iChan + 1));

	// Load bank list from actual synth stack...
	m_ui.BankListView->setUpdatesEnabled(false);
	m_ui.BankListView->setSortingEnabled(false);
	m_ui.BankListView->clear();

	QTreeWidgetItem *pBankItem = NULL;
	// For all soundfonts (in reversed stack order) fill the available banks...
	int cSoundFonts = ::fluid_synth_sfcount(m_pSynth);
	for (int i = 0; i < cSoundFonts; ++i) {
		fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, i);
		if (pSoundFont) {
		#ifdef CONFIG_FLUID_BANK_OFFSET
		#ifdef CONFIG_FLUID_SFONT_GET_ID
			const int iSFID = ::fluid_sfont_get_id(pSoundFont);
		#else
			const int iSFID = pSoundFont->id;
		#endif
			const int iBankOffset = ::fluid_synth_get_bank_offset(m_pSynth, iSFID);
		#endif
		#ifdef CONFIG_FLUID_SFONT_ITERATION_START
			::fluid_sfont_iteration_start(pSoundFont);
		#else
			pSoundFont->iteration_start(pSoundFont);
		#endif
			fluid_preset_t *pPreset;
		#ifdef CONFIG_FLUID_SFONT_ITERATION_NEXT
			while ((pPreset = ::fluid_sfont_iteration_next(pSoundFont)) != NULL) {
		#else
			fluid_preset_t preset;
			pPreset = &preset;
			while (pSoundFont->iteration_next(pSoundFont, pPreset)) {
		#endif
			#ifdef CONFIG_FLUID_PRESET_GET_BANKNUM
				int iBank = ::fluid_preset_get_banknum(pPreset);
			#else
				int iBank = pPreset->get_banknum(pPreset);
			#endif
			#ifdef CONFIG_FLUID_BANK_OFFSET
				iBank += iBankOffset;
			#endif
				if (!findBankItem(iBank)) {
					pBankItem = new qsynthPresetItem(m_ui.BankListView, pBankItem);
					if (pBankItem)
						pBankItem->setText(0, QString::number(iBank));
				}
			}
		}
	}
	m_ui.BankListView->setSortingEnabled(true);
	m_ui.BankListView->setUpdatesEnabled(true);

	// Set the selected bank.
	m_iBank = 0;
#ifdef CONFIG_FLUID_CHANNEL_INFO
	fluid_synth_channel_info_t info;
	::memset(&info, 0, sizeof(info));
	::fluid_synth_get_channel_info(m_pSynth, iChan, &info);
	if (info.assigned) {
		m_iBank = info.bank;
	#ifdef CONFIG_FLUID_BANK_OFFSET
		m_iBank += ::fluid_synth_get_bank_offset(m_pSynth, info.sfont_id);
	#endif
	}
#else
	fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(m_pSynth, m_iChan);
	if (pPreset) {
	#ifdef CONFIG_FLUID_PRESET_GET_BANKNUM
		m_iBank = ::fluid_preset_get_banknum(pPreset);
	#else
		m_iBank = pPreset->get_banknum(pPreset);
	#endif
	#ifdef CONFIG_FLUID_BANK_OFFSET
		int iSFID = 0;
	#ifdef CONFIG_FLUID_PRESET_GET_SFONT
		fluid_sfont_t *pSoundFont = ::fluid_preset_get_sfont(pPreset);
	#else
		fluid_sfont_t *pSoundFont = pPreset->sfont;
	#endif
		if (pSoundFont) {
		#ifdef CONFIG_FLUID_SFONT_GET_ID
			iSFID = ::fluid_sfont_get_id(pSoundFont);
		#else
			iSFID = pSoundFont->id;
		#endif
		}
		m_iBank += ::fluid_synth_get_bank_offset(m_pSynth, iSFID);
	#endif
	}
#endif

	pBankItem = findBankItem(m_iBank);
	m_ui.BankListView->setCurrentItem(pBankItem);
//  m_ui.BankListView->ensureItemVisible(pBankItem);
	bankChanged();

	// Set the selected program.
#ifdef CONFIG_FLUID_CHANNEL_INFO
	if (info.assigned)
		m_iProg = info.program;
#else
	if (pPreset)
	#if CONFIG_FLUID_PRESET_GET_NUM
		m_iProg = ::fluid_preset_get_num(pPreset);
	#else
		m_iProg = pPreset->get_num(pPreset);
	#endif
#endif

	QTreeWidgetItem *pProgItem = findProgItem(m_iProg);
	m_ui.ProgListView->setCurrentItem(pProgItem);
//  m_ui.ProgListView->ensureItemVisible(pProgItem);

	// And the preview state...
	m_ui.PreviewCheckBox->setChecked(m_pOptions->bPresetPreview);

	// Done with setup...
	m_iDirtySetup--;
}


// Stabilize current state form.
void qsynthPresetForm::stabilizeForm()
{
	m_ui.DialogButtonBox->button(
		QDialogButtonBox::Ok)->setEnabled(validateForm());
}


// Validate form fields.
bool qsynthPresetForm::validateForm()
{
	bool bValid = true;

	bValid = bValid && (m_ui.BankListView->currentItem() != NULL);
	bValid = bValid && (m_ui.ProgListView->currentItem() != NULL);

	return bValid;
}


// Realize a bank-program selection preset.
void qsynthPresetForm::setBankProg ( int iBank, int iProg )
{
	if (m_pSynth == NULL)
		return;

	// just select the synth's program preset...
	::fluid_synth_bank_select(m_pSynth, m_iChan, iBank);
	::fluid_synth_program_change(m_pSynth, m_iChan, iProg);

	// Maybe this is needed to stabilize things around.
	::fluid_synth_program_reset(m_pSynth);
}


// Validate form fields and accept it valid.
void qsynthPresetForm::accept()
{
	if (validateForm()) {
		// Unload from current selected dialog items.
		int iBank = (m_ui.BankListView->currentItem())->text(0).toInt();
		int iProg = (m_ui.ProgListView->currentItem())->text(0).toInt();
		// And set it right away...
		setBankProg(iBank, iProg);
		// Do remember preview state...
		if (m_pOptions)
			m_pOptions->bPresetPreview = m_ui.PreviewCheckBox->isChecked();
		// We got it.
		QDialog::accept();
	}
}


// Reject settings (Cancel button slot).
void qsynthPresetForm::reject (void)
{
	// Reset selection to initial selection, if applicable...
	if (m_iDirtyCount > 0)
		setBankProg(m_iBank, m_iProg);
	// Done (hopefully nothing).
	QDialog::reject();
}


// Find the bank item of given bank number id.
QTreeWidgetItem *qsynthPresetForm::findBankItem ( int iBank )
{
	QList<QTreeWidgetItem *> banks
		= m_ui.BankListView->findItems(
			QString::number(iBank), Qt::MatchExactly, 0);

	QListIterator<QTreeWidgetItem *> iter(banks);
	if (iter.hasNext())
		return iter.next();
	else
		return NULL;
}


// Find the program item of given program number id.
QTreeWidgetItem *qsynthPresetForm::findProgItem ( int iProg )
{
	QList<QTreeWidgetItem *> progs
		= m_ui.ProgListView->findItems(
			QString::number(iProg), Qt::MatchExactly, 0);

	QListIterator<QTreeWidgetItem *> iter(progs);
	if (iter.hasNext())
		return iter.next();
	else
		return NULL;
}



// Bank change slot.
void qsynthPresetForm::bankChanged (void)
{
	if (m_pSynth == NULL)
		return;

	QTreeWidgetItem *pBankItem = m_ui.BankListView->currentItem();
	if (pBankItem == NULL)
		return;
	int iBankSelected = pBankItem->text(0).toInt();

	// Clear up the program listview.
	m_ui.ProgListView->setUpdatesEnabled(false);
	m_ui.ProgListView->setSortingEnabled(false);
	m_ui.ProgListView->clear();
//	fluid_preset_t preset;
	QTreeWidgetItem *pProgItem = NULL;
	// For all soundfonts (in reversed stack order) fill the available programs...
	int cSoundFonts = ::fluid_synth_sfcount(m_pSynth);
	for (int i = 0; i < cSoundFonts; ++i) {
		fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, i);
		if (pSoundFont) {
		#ifdef CONFIG_FLUID_SFONT_GET_ID
			const int iSFID = ::fluid_sfont_get_id(pSoundFont);
		#else
			const int iSFID = pSoundFont->id;
		#endif
		#ifdef CONFIG_FLUID_SFONT_GET_NAME
			const QString sSFName = ::fluid_sfont_get_name(pSoundFont);
		#else
			const QString sSFName = pSoundFont->get_name(pSoundFont);
		#endif
		#ifdef CONFIG_FLUID_BANK_OFFSET
			const int iBankOffset = ::fluid_synth_get_bank_offset(m_pSynth, iSFID);
		#endif
		#ifdef CONFIG_FLUID_SFONT_ITERATION_START
			::fluid_sfont_iteration_start(pSoundFont);
		#else
			pSoundFont->iteration_start(pSoundFont);
		#endif
			fluid_preset_t *pPreset;
		#ifdef CONFIG_FLUID_SFONT_ITERATION_NEXT
			while ((pPreset = ::fluid_sfont_iteration_next(pSoundFont)) != NULL) {
		#else
			fluid_preset_t preset;
			pPreset = &preset;
			while (pSoundFont->iteration_next(pSoundFont, pPreset)) {
		#endif
			#ifdef CONFIG_FLUID_PRESET_GET_BANKNUM
				int iBank = ::fluid_preset_get_banknum(pPreset);
			#else
				int iBank = pPreset->get_banknum(pPreset);
			#endif
			#ifdef CONFIG_FLUID_BANK_OFFSET
				iBank += iBankOffset;
			#endif
			#ifdef CONFIG_FLUID_PRESET_GET_NUM
				const int iProg = ::fluid_preset_get_num(pPreset);
			#else
				const int iProg = pPreset->get_num(pPreset);
			#endif
				if (iBank == iBankSelected && !findProgItem(iProg)) {
					pProgItem = new qsynthPresetItem(m_ui.ProgListView, pProgItem);
					if (pProgItem) {
					#ifdef CONFIG_FLUID_PRESET_GET_NAME
						const QString sName = ::fluid_preset_get_name(pPreset);
					#else
						const QString sName = pPreset->get_name(pPreset);
					#endif
						pProgItem->setText(0, QString::number(iProg));
						pProgItem->setText(1, sName);
						pProgItem->setText(2, QString::number(iSFID));
						pProgItem->setText(3, QFileInfo(sSFName).baseName());
					}
				}
			}
		}
	}
	m_ui.ProgListView->setSortingEnabled(true);
	m_ui.ProgListView->setUpdatesEnabled(true);

	// Stabilize the form.
	stabilizeForm();
}


// Program change slot.
void qsynthPresetForm::progChanged (void)
{
	if (m_pSynth == NULL)
		return;

	// Which preview state...
	if (m_ui.PreviewCheckBox->isChecked() && validateForm()) {
		// Set current selection.
		int iBank = (m_ui.BankListView->currentItem())->text(0).toInt();
		int iProg = (m_ui.ProgListView->currentItem())->text(0).toInt();
		// And set it right away...
		setBankProg(iBank, iProg);
		// Now we're dirty nuff.
		m_iDirtyCount++;
	}   // Have we done anything dirty before?
	else if (m_iDirtyCount > 0) {
		// Restore initial preset...
		setBankProg(m_iBank, m_iProg);
		// And we're clean again.
		m_iDirtyCount = 0;
	}

	// Stabilize the form.
	stabilizeForm();
}


// Preview change slot.
void qsynthPresetForm::previewChanged (void)
{
	// Just like a program change, if not on setup...
	if (m_iDirtySetup == 0)
		progChanged();
}


// end of qsynthPresetForm.cpp
