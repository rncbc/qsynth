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

#include "config.h"


// Kind of constructor.
void qsynthPresetForm::init()
{
    m_pSynth  = NULL;
    m_iChan   = 0;
    m_pPreset = NULL;
}


// Kind of destructor.
void qsynthPresetForm::destroy()
{
}


// Dialog setup loader.
void qsynthPresetForm::setup ( fluid_synth_t *pSynth, int iChan )
{
    m_pSynth = pSynth;
    m_iChan  = iChan;

    // set the proper caption...
    setCaption(tr("Channel") + " " + QString::number(m_iChan + 1));
    
    // Load soundfont view from actual synth stack...
    SoundFontComboBox->clear();
    int cSoundFonts = ::fluid_synth_sfcount(m_pSynth);
    for (int i = cSoundFonts - 1; i >= 0; i--) {
        fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(pSynth, i);
        if (pSoundFont) {
            QString sSoundFontText = QString::number(pSoundFont->id);
            sSoundFontText += ":";
            sSoundFontText += pSoundFont->get_name(pSoundFont);
            SoundFontComboBox->insertItem(sSoundFontText);
        }
    }
    
    // Load and set the dialog controls.
    fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(m_pSynth, m_iChan);
    if (pPreset) {
        int iSoundFontIndex = findSoundFontIndex((pPreset->sfont)->id);
        SoundFontComboBox->setCurrentItem(iSoundFontIndex);
        soundfontChanged();
        int iBankIndex = findBankIndex(pPreset->get_banknum(pPreset));
        BankListBox->setCurrentItem(iBankIndex);
        bankChanged();
        int iProgramIndex = findProgramIndex(pPreset->get_num(pPreset));
        ProgramListBox->setCurrentItem(iProgramIndex);
    }
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

    bValid = bValid && (SoundFontComboBox->currentItem() >= 0);
    bValid = bValid && (BankListBox->currentItem() >= 0);
    bValid = bValid && (ProgramListBox->currentItem() >= 0);

    return bValid;
}

// Validate form fields and accept it valid.
void qsynthPresetForm::acceptForm()
{
    if (validateForm()) {
        // Unload from current selected dialog items.
        int iSFID = SoundFontComboBox->currentText().section(':', 0, 0).toInt();
        int iBank = BankListBox->currentText().toInt();
        int iProgram = ProgramListBox->currentText().section(':', 0, 0).toInt();
        // And set it right away...
        ::fluid_synth_program_select(m_pSynth, m_iChan, iSFID, iBank, iProgram);
        // We got it.
        accept();
    }
}


// Find the soundfont item index of given id.
int qsynthPresetForm::findSoundFontIndex ( int iSFID )
{
    int cItems = SoundFontComboBox->count();
    for (int iItem = 0; iItem < cItems; iItem++) {
        if (iSFID == SoundFontComboBox->text(iItem).section(':', 0, 0).toInt())
            return iItem;
    }
    return -1;
}


// Find the bank item index of given bank number id.
int qsynthPresetForm::findBankIndex ( int iBank )
{
    int cItems = BankListBox->count();
    for (int iItem = 0; iItem < cItems; iItem++) {
        if (iBank == BankListBox->text(iItem).toInt())
            return iItem;
    }
    return -1;
}


// Find the program item index of given program number id.
int qsynthPresetForm::findProgramIndex ( int iProgram )
{
    int cItems = ProgramListBox->count();
    for (int iItem = 0; iItem < cItems; iItem++) {
        if (iProgram == ProgramListBox->text(iItem).section(':', 0, 0).toInt())
            return iItem;
    }
    return -1;
}


// Soundfont change slot.
void qsynthPresetForm::soundfontChanged (void)
{
    // Clear up the banks listbox.
    BankListBox->clear();
    // Grab soundfont id from combobox item text.
    int iSFID = SoundFontComboBox->currentText().section(':', 0, 0).toInt();
    // Grab the soundfont...
    fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, iSFID);
    if (pSoundFont) {
        fluid_preset_t preset;
        pSoundFont->iteration_start(pSoundFont);
        while (pSoundFont->iteration_next(pSoundFont, &preset)) {
            int iBank = preset.get_banknum(&preset);
            if (findBankIndex(iBank) < 0)
                BankListBox->insertItem(QString::number(iBank));
        }
    }
    // Stabilize the bank list.
    bankChanged();
}


// Bank change slot.
void qsynthPresetForm::bankChanged (void)
{
    // Clear up the program listbox.
    ProgramListBox->clear();
    // Grab soundfont id from combobox item text.
    int iSFID = SoundFontComboBox->currentText().section(':', 0, 0).toInt();
    int iBank = BankListBox->currentText().toInt();
    // Grab the soundfont...
    fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, iSFID);
    if (pSoundFont) {
        fluid_preset_t preset;
        pSoundFont->iteration_start(pSoundFont);
        while (pSoundFont->iteration_next(pSoundFont, &preset)) {
            if (iBank == preset.get_banknum(&preset)) {
                QString sProgramText = QString::number(preset.get_num(&preset));
                sProgramText += ":";
                sProgramText += preset.get_name(&preset);
                ProgramListBox->insertItem(sProgramText);
            }
        }
    }
    // Stabilize the form.
    stabilizeForm();
}


// end of qsynthPresetForm.ui.h
