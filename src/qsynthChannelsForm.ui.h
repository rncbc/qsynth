// qsynthChannelsForm.ui.h
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

#include "qsynthAbout.h"
#include "qsynthChannels.h"

#include <qvalidator.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>


// Kind of constructor.
void qsynthChannelsForm::init (void)
{
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
    m_pXpmLedOn  = new QPixmap(QPixmap::fromMimeSource("ledon1.png"));
    m_pXpmLedOff = new QPixmap(QPixmap::fromMimeSource("ledoff1.png"));

    // Set validators...
    PresetComboBox->setValidator(new QRegExpValidator(QRegExp("[\\w-]+"), PresetComboBox));

    // Initially sorted by channel number.
    ChannelsListView->setSorting(1);
}


// Kind of destructor.
void qsynthChannelsForm::destroy (void)
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
void qsynthChannelsForm::setup ( qsynthOptions *pOptions, qsynthEngine *pEngine, bool bPreset )
{
    // Set the proper descriptors.
    m_pOptions = pOptions;
    m_pEngine  = pEngine;
    m_pSynth   = pEngine ? pEngine->pSynth : NULL;
        
    // Update caption.
    QString sCaption = QSYNTH_TITLE ": " + tr("Channels");
    if (pEngine)
        sCaption += " [" + pEngine->name() + "]";
    setCaption(sCaption);

    // Free up current channel list view.
    if (m_ppChannels) {
        delete [] m_ppChannels;
        m_ppChannels = NULL;
        m_iChannels  = 0;
    }
    
    // Allocate a new channel list view...
    ChannelsListView->clear();
    if (m_pSynth && m_ppChannels == NULL) {
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
        PresetComboBox->setCurrentText((m_pEngine->setup())->sDefPreset);
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

    qsynthChannelsViewItem *pItem = m_ppChannels[iChan];

    fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(m_pSynth, iChan);
    if (pPreset) {
        pItem->setText(QSYNTH_CHANNELS_BANK, QString::number(pPreset->get_banknum(pPreset)));
        pItem->setText(QSYNTH_CHANNELS_PROG, QString::number(pPreset->get_num(pPreset)));
        pItem->setText(QSYNTH_CHANNELS_NAME, pPreset->get_name(pPreset));
        pItem->setText(QSYNTH_CHANNELS_SFID, QString::number((pPreset->sfont)->id));
        pItem->setText(QSYNTH_CHANNELS_SFNAME, QFileInfo((pPreset->sfont)->get_name(pPreset->sfont)).baseName());
        // Make this a dirty-operation.
        m_iDirtyCount++;
    } else {
        QString n = "-";
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
        
    m_ppChannels[iChan]->setPixmap(QSYNTH_CHANNELS_IN, (bOn ? *m_pXpmLedOn : *m_pXpmLedOff));
}


// Channel view context menu handler.
void qsynthChannelsForm::contextMenu( QListViewItem *pItem, const QPoint& pos, int )
{
	int iItemID;

	// Build the channel context menu...
	QPopupMenu* pContextMenu = new QPopupMenu(this);

	bool bEnabled = (m_pSynth && pItem);
	iItemID = pContextMenu->insertItem(
		QIconSet(QPixmap::fromMimeSource("edit1.png")),
		tr("Edit") + "...", this, SLOT(editSelectedChannel()));
	pContextMenu->setItemEnabled(iItemID, bEnabled);
	pContextMenu->insertSeparator();
	iItemID = pContextMenu->insertItem(
		tr("Refresh"), this, SLOT(updateAllChannels()));
	pContextMenu->setItemEnabled(iItemID, bEnabled);

	pContextMenu->exec(pos);

	delete pContextMenu;
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
void qsynthChannelsForm::changePreset( const QString& sPreset )
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

    QString sPreset = PresetComboBox->currentText();
    if (sPreset.isEmpty())
        return;

    // Unload presets (from synth).
    if (m_pOptions->savePreset(m_pEngine, sPreset)) {
        m_iDirtySetup++;
        resetPresets();
        PresetComboBox->setCurrentText(sPreset);
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

    QString sPreset = PresetComboBox->currentText();
    if (sPreset.isEmpty())
        return;

    // Try to prompt user if he/she really wants this...
    if (QMessageBox::warning(this, tr("Warning"),
        tr("Delete preset:") + "\n\n" +
        sPreset + "\n\n" +
        tr("Are you sure?"),
        tr("OK"), tr("Cancel")) > 0)
        return;

    // Remove current preset item...
    m_iDirtySetup++;
    int iItem = PresetComboBox->currentItem();
    m_pOptions->deletePreset(m_pEngine, sPreset);
    resetPresets();
    PresetComboBox->setCurrentItem(iItem);
    m_iDirtySetup--;

    // Load a new preset...
    changePreset(PresetComboBox->currentText());
}


void qsynthChannelsForm::resetPresets (void)
{
    if (m_pEngine == NULL)
        return;
        
    qsynthSetup *pSetup = m_pEngine->setup();
    if (pSetup == NULL)
        return;

    PresetComboBox->clear();
    PresetComboBox->insertStringList(pSetup->presets);
    PresetComboBox->insertItem(pSetup->sDefPresetName);
}


// Stabilize current form state.
void qsynthChannelsForm::stabilizeForm (void)
{
    if (m_pEngine == NULL)
        return;
        
    qsynthSetup *pSetup = m_pEngine->setup();
    if (pSetup == NULL)
        return;

    QString sPreset = PresetComboBox->currentText();
    if (m_pSynth && !sPreset.isEmpty()) {
        bool bPreset = (pSetup->presets.find(sPreset) != pSetup->presets.end());
        PresetSavePushButton->setEnabled(m_iDirtyCount > 0 || (!bPreset && sPreset != pSetup->sDefPresetName));
        PresetDeletePushButton->setEnabled(bPreset);
    } else {
        PresetSavePushButton->setEnabled(false);
        PresetDeletePushButton->setEnabled(false);
    }
}


// end of qsynthChannelsForm.ui.h
