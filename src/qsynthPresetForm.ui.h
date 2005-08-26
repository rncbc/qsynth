// qsynthPresetForm.ui.h
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

#include "qsynthAbout.h"

#include <qfileinfo.h>


// Kind of constructor.
void qsynthPresetForm::init()
{
    m_pSynth = NULL;
    m_iChan  = 0;
    m_iBank  = 0;
    m_iProg  = 0;

    // To avoid setup jitterness and preview side effects.
    m_iDirtySetup = 0;
    m_iDirtyCount = 0;

    // No default sorting, initially.
    BankListView->setSorting(BankListView->columns() + 1);
    ProgListView->setSorting(ProgListView->columns() + 1);
}


// Kind of destructor.
void qsynthPresetForm::destroy()
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
	setCaption(QSYNTH_TITLE ": "
		+ tr("Channel") + " " + QString::number(m_iChan + 1));

    QListViewItem *pBankItem = NULL;
    QListViewItem *pProgItem = NULL;

    // Load bank list from actual synth stack...
    BankListView->clear();
    BankListView->setUpdatesEnabled(false);
    fluid_preset_t preset;
    // For all soundfonts (in reversed stack order) fill the available banks...
    int cSoundFonts = ::fluid_synth_sfcount(m_pSynth);
    for (int i = 0; i < cSoundFonts; i++) {
        fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, i);
        if (pSoundFont) {
            pSoundFont->iteration_start(pSoundFont);
            while (pSoundFont->iteration_next(pSoundFont, &preset)) {
                int iBank = preset.get_banknum(&preset);
                if (!findBankItem(iBank)) {
                    pBankItem = new QListViewItem(BankListView, pBankItem);
                    if (pBankItem)
                        pBankItem->setText(0, QString::number(iBank));
                }
            }
        }
    }
    BankListView->setUpdatesEnabled(true);

    // Set the selected bank.
    fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(m_pSynth, m_iChan);
    if (pPreset)
        m_iBank = pPreset->get_banknum(pPreset);
    pBankItem = findBankItem(m_iBank);
    BankListView->setSelected(pBankItem, true);
    BankListView->ensureItemVisible(pBankItem);
    bankChanged();

    // Set the selected program.
    if (pPreset)
        m_iProg = pPreset->get_num(pPreset);
    pProgItem = findProgItem(m_iProg);
    ProgListView->setSelected(pProgItem, true);
    ProgListView->ensureItemVisible(pProgItem);

    // And the preview state...
    PreviewCheckBox->setChecked(m_pOptions->bPresetPreview);

    // Done with setup...
    m_iDirtySetup--;
}


// Stabilize current state form.
void qsynthPresetForm::stabilizeForm()
{
    OkPushButton->setEnabled(validateForm());
}


// Validate form fields.
bool qsynthPresetForm::validateForm()
{
    bool bValid = true;

    bValid = bValid && (BankListView->selectedItem() != NULL);
    bValid = bValid && (ProgListView->selectedItem() != NULL);

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
        int iBank = (BankListView->selectedItem())->text(0).toInt();
        int iProg = (ProgListView->selectedItem())->text(0).toInt();
        // And set it right away...
        setBankProg(iBank, iProg);
        // Do remember preview state...
        if (m_pOptions)
            m_pOptions->bPresetPreview = PreviewCheckBox->isChecked();
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
QListViewItem *qsynthPresetForm::findBankItem ( int iBank )
{
    return BankListView->findItem(QString::number(iBank), 0);
}


// Find the program item of given program number id.
QListViewItem *qsynthPresetForm::findProgItem ( int iProg )
{
    return ProgListView->findItem(QString::number(iProg), 0);
}



// Bank change slot.
void qsynthPresetForm::bankChanged (void)
{
    if (m_pSynth == NULL)
        return;

    QListViewItem *pBankItem = BankListView->selectedItem();
    if (pBankItem == NULL)
        pBankItem = BankListView->currentItem();
    if (pBankItem == NULL)
        return;
    int iBank = pBankItem->text(0).toInt();

    // Clear up the program listview.
    ProgListView->clear();
    // Start freeze...
    BankListView->setUpdatesEnabled(false);
    ProgListView->setUpdatesEnabled(false);
    QListViewItem *pProgItem = NULL;
    fluid_preset_t preset;
    // For all soundfonts (in reversed stack order) fill the available programs...
    int cSoundFonts = ::fluid_synth_sfcount(m_pSynth);
    for (int i = 0; i < cSoundFonts; i++) {
        fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, i);
        if (pSoundFont) {
            pSoundFont->iteration_start(pSoundFont);
            while (pSoundFont->iteration_next(pSoundFont, &preset)) {
                int iProg = preset.get_num(&preset);
                if (iBank == preset.get_banknum(&preset) && !findProgItem(iProg)) {
                    pProgItem = new QListViewItem(ProgListView, pProgItem);
                    if (pProgItem) {
                        pProgItem->setText(0, QString::number(iProg));
                        pProgItem->setText(1, preset.get_name(&preset));
                        pProgItem->setText(2, QString::number(pSoundFont->id));
                        pProgItem->setText(3, QFileInfo(pSoundFont->get_name(pSoundFont)).baseName());
                    }
                }
            }
        }
    }
    // Freeze's over.
    BankListView->setUpdatesEnabled(true);
    ProgListView->setUpdatesEnabled(true);

    // Stabilize the form.
    stabilizeForm();
}


// Program change slot.
void qsynthPresetForm::progChanged (void)
{
    if (m_pSynth == NULL)
        return;

    // Which preview state...
    if (PreviewCheckBox->isChecked() && validateForm()) {
        // Set current selection.
        int iBank = (BankListView->selectedItem())->text(0).toInt();
        int iProg = (ProgListView->selectedItem())->text(0).toInt();
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



// end of qsynthPresetForm.ui.h
