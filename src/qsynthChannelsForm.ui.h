// qsynthChannelsForm.ui.h
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
#include <qpopupmenu.h>
#include <qfileinfo.h>

#include "config.h"

#include "qsynthChannels.h"


// Kind of constructor.
void qsynthChannelsForm::init (void)
{
    m_iChannels  = 0;
    m_ppChannels = NULL;

    // No setup synth references initially (the caller will set them).
    m_pSetup = NULL;
    m_pSynth = NULL;
    // Initialize dirty control state.
    m_iDirtySetup = 0;
    m_iDirtyCount = 0;

    // Our activity leds (same of main form :).
    m_pXpmLedOn  = new QPixmap(QPixmap::fromMimeSource("ledon1.png"));
    m_pXpmLedOff = new QPixmap(QPixmap::fromMimeSource("ledoff1.png"));

    // Set validators...
    PresetComboBox->setValidator(new QRegExpValidator(QRegExp("[\\w]+"), PresetComboBox));

    // Initially sorted by channel number.
    ChannelsListView->setSorting(1);
}


// Kind of destructor.
void qsynthChannelsForm::destroy (void)
{
    // Nullify references.
    setup(NULL, NULL);

    // Delete pixmaps.
    delete m_pXpmLedOn;
    delete m_pXpmLedOff;
}


// Notify our parent that we're emerging.
void qsynthChannelsForm::showEvent ( QShowEvent *pShowEvent )
{
    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();

    QWidget::showEvent(pShowEvent);
}

// Notify our parent that we're closing.
void qsynthChannelsForm::hideEvent ( QHideEvent *pHideEvent )
{
    QWidget::hideEvent(pHideEvent);

    qsynthMainForm *pMainForm = (qsynthMainForm *) QWidget::parentWidget();
    if (pMainForm)
        pMainForm->stabilizeForm();
}


// Populate (setup) synth settings descriptors.
void qsynthChannelsForm::setup ( qsynthSetup *pSetup, fluid_synth_t *pSynth )
{
    // Set the proper descriptors.
    m_pSetup = pSetup;
    m_pSynth = pSynth;

    // Check this first.
    if (m_pSetup == NULL)
        return;
        
    // Free up current channel list view.
    if (m_pSynth == NULL && m_ppChannels) {
        ChannelsListView->clear();
        delete [] m_ppChannels;
        m_ppChannels = NULL;
        m_iChannels  = 0;
    }
    
    // Allocate a new channel list view...
    if (m_pSynth && m_ppChannels == NULL) {
        ChannelsListView->clear();
        m_iChannels = ::fluid_synth_count_midi_channels(m_pSynth);
        if (m_iChannels > 0)
            m_ppChannels = new qsynthChannelsViewItemPtr [m_iChannels];
        if (m_ppChannels) {
            for (int iChan = 0; iChan < m_iChannels; iChan++) {
                qsynthChannelsViewItem *pItem = new qsynthChannelsViewItem(ChannelsListView);
                if (pItem) {
                    pItem->setPixmap(QSYNTH_CHANNELS_IN, *m_pXpmLedOff);
                    pItem->setText(QSYNTH_CHANNELS_CHAN, QString::number(iChan + 1));
                }
                m_ppChannels[iChan] = pItem;
            }
        }
        // Load preset list...
        m_iDirtySetup++;
        resetPresets();
        PresetComboBox->setCurrentText(m_pSetup->sDefPreset);
        m_iDirtySetup--;
        // Load default preset and update/refresh the whole thing...
        changePreset(PresetComboBox->currentText());
    }
}


// Channel item update.
void qsynthChannelsForm::updateChannel ( int iChan )
{
    if (m_pSynth == NULL || m_ppChannels == NULL)
        return;
    if (iChan < 0 || iChan >= m_iChannels)
        return;

    qsynthChannelsViewItem *pItem = m_ppChannels[iChan];
    
    fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(m_pSynth, iChan);
    if (pPreset) {
        pItem->setText(QSYNTH_CHANNELS_SFID, QString::number((pPreset->sfont)->id));
        pItem->setText(QSYNTH_CHANNELS_SFNAME, QFileInfo((pPreset->sfont)->get_name(pPreset->sfont)).baseName());
        pItem->setText(QSYNTH_CHANNELS_BANK, QString::number(pPreset->get_banknum(pPreset)));
        pItem->setText(QSYNTH_CHANNELS_PROG, QString::number(pPreset->get_num(pPreset)));
        pItem->setText(QSYNTH_CHANNELS_NAME, pPreset->get_name(pPreset));
    } else {
        QString n = "-";
        pItem->setText(QSYNTH_CHANNELS_SFID, n);
        pItem->setText(QSYNTH_CHANNELS_SFNAME, n);
        pItem->setText(QSYNTH_CHANNELS_BANK, n);
        pItem->setText(QSYNTH_CHANNELS_PROG, n);
        pItem->setText(QSYNTH_CHANNELS_NAME, n);
    }
}

// All channels update.
void qsynthChannelsForm::updateAllChannels (void)
{
    if (m_pSynth == NULL || m_ppChannels == NULL)
        return;
        
    for (int iChan = 0; iChan < m_iChannels; iChan++)
        updateChannel(iChan);
}


// Update channel activity status LED.
void qsynthChannelsForm::setChannelOn ( int iChan, bool bOn )
{
    if (m_pSynth == NULL || m_ppChannels == NULL)
        return;

    if (iChan < 0 || iChan >= m_iChannels)
        return;
        
    m_ppChannels[iChan]->setPixmap(QSYNTH_CHANNELS_IN, (bOn ? *m_pXpmLedOn : *m_pXpmLedOff));
}


// Channel view context menu handler.
void qsynthChannelsForm::contextMenu( QListViewItem *pItem, const QPoint& pos, int )
{
    int iItemID;

    // Build the channel context menu...
    QPopupMenu* pContextMenu = new QPopupMenu(this);

    bool bEnabled = (m_pSynth && pItem);
    iItemID = pContextMenu->insertItem(tr("Edit"), this, SLOT(editSelectedChannel()));
    pContextMenu->setItemEnabled(iItemID, bEnabled);
    pContextMenu->insertSeparator();
    iItemID = pContextMenu->insertItem(tr("Refresh"), this, SLOT(updateAllChannels()));
    pContextMenu->setItemEnabled(iItemID, bEnabled);

    pContextMenu->exec(pos);
}


// Edit detail dialog.
void qsynthChannelsForm::editSelectedChannel (void)
{
    doubleClick(ChannelsListView->selectedItem());
}


// Show detail dialog.
void qsynthChannelsForm::doubleClick( QListViewItem *pItem )
{
    if (pItem == NULL)
        return;
    if (m_pSynth == NULL || m_ppChannels == NULL)
        return;

    int iChan = (pItem->text(QSYNTH_CHANNELS_CHAN).toInt() - 1);
    if (iChan < 0 || iChan >= m_iChannels)
        return;
        
    qsynthPresetForm *pPresetForm = new qsynthPresetForm(this);
    if (pPresetForm) {
        // The the proper context.
        pPresetForm->setup(m_pSynth, iChan);
        // Show the channel preset dialog...
        if (pPresetForm->exec()) {
            updateChannel(iChan);
            m_iDirtyCount++;
        }
        // Done.
        delete pPresetForm;
    }
}

// Channels preset naming slots.
void qsynthChannelsForm::changePreset( const QString& sPreset )
{
    if (m_pSetup == NULL || m_pSynth == NULL || m_iDirtySetup > 0)
        return;

    // This is a pseudo-dirty procedure...
    m_iDirtyCount++;
    // Load presets and update/refresh the whole thing.
    if (m_pSetup->loadPreset(m_pSynth, sPreset)) {
        updateAllChannels();
        m_iDirtyCount = 0;
    }
    stabilizeForm();
}


void qsynthChannelsForm::savePreset (void)
{
    if (m_pSetup == NULL || m_pSynth == NULL)
        return;

    QString sPreset = PresetComboBox->currentText();
    if (sPreset.isEmpty())
        return;

    // Unload presets (from synth).
    if (m_pSetup->savePreset(m_pSynth, sPreset)) {
        m_iDirtySetup++;
        resetPresets();
        PresetComboBox->setCurrentText(sPreset);
        m_iDirtySetup--;
        // Special, set this as default.
        m_pSetup->sDefPreset = sPreset;
        m_iDirtyCount = 0;
    }

    stabilizeForm();
}


void qsynthChannelsForm::deletePreset (void)
{
    if (m_pSetup == NULL || m_pSynth == NULL)
        return;

    QString sPreset = PresetComboBox->currentText();
    if (sPreset.isEmpty())
        return;

    // Remove current preset item...
    m_iDirtySetup++;
    int iItem = PresetComboBox->currentItem();
    m_pSetup->deletePreset(sPreset);
    resetPresets();
    PresetComboBox->setCurrentItem(iItem);
    m_iDirtySetup--;

    // Load a new preset...
    changePreset(PresetComboBox->currentText());
}


void qsynthChannelsForm::resetPresets (void)
{
    if (m_pSetup == NULL)
        return;

    PresetComboBox->clear();
    PresetComboBox->insertStringList(m_pSetup->presets);
    PresetComboBox->insertItem("(default)");
}


// Stabilize current form state.
void qsynthChannelsForm::stabilizeForm (void)
{
    QString sPreset = PresetComboBox->currentText();
    if (m_pSetup && !sPreset.isEmpty()) {
        PresetSavePushButton->setEnabled(m_iDirtyCount > 0);
        PresetDeletePushButton->setEnabled(m_pSetup->presets.find(sPreset) != m_pSetup->presets.end());
    } else {
        PresetSavePushButton->setEnabled(false);
        PresetDeletePushButton->setEnabled(false);
    }
}


// end of qsynthChannelsForm.ui.h

