// qsynthSetupForm.ui.h
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
#include <qpopupmenu.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfontdialog.h>
#include <qlistbox.h>

#include "config.h"

#include "qsynthAbout.h"


// Our local parameter data struct.
struct qsynth_settings_data
{
    qsynthSetup   *pSetup;
    QListView     *pListView;
    QListViewItem *pListItem;
    QStringList    options;
};

static void qsynth_settings_foreach_option ( void *pvData, char *, char *pszOption )
{
    qsynth_settings_data *pData = (qsynth_settings_data *) pvData;
    
    pData->options.append(pszOption);
}

static void qsynth_settings_foreach ( void *pvData, char *pszName, int iType )
{
    qsynth_settings_data *pData = (qsynth_settings_data *) pvData;
    fluid_settings_t *pFluidSettings = (pData->pSetup)->fluid_settings();

    // Add the new list item.
    int iCol = 0;
    pData->pListItem = new QListViewItem(pData->pListView, pData->pListItem);
    (pData->pListItem)->setText(iCol++, pszName);

    // Check for type...
    char *pszType = "?";
    switch (iType) {
      case FLUID_NUM_TYPE: pszType = "num"; break;
      case FLUID_INT_TYPE: pszType = "int"; break;
      case FLUID_STR_TYPE: pszType = "str"; break;
      case FLUID_SET_TYPE: pszType = "set"; break;
    }
    (pData->pListItem)->setText(iCol++, pszType);
/*
    // Check for hints...
    int iHints = ::fluid_settings_get_hints(pFluidSettings, pszName);
    QString sHints = "";
    if (iHints & FLUID_HINT_BOUNDED_BELOW)
        sHints += " BOUNDED_BELOW ";
    if (iHints & FLUID_HINT_BOUNDED_ABOVE)
        sHints += " BOUNDED_ABOVE ";
    if (iHints & FLUID_HINT_TOGGLED)
        sHints += " TOGGLED ";
    if (iHints & FLUID_HINT_SAMPLE_RATE)
        sHints += " SAMPLE_RATE ";
    if (iHints & FLUID_HINT_LOGARITHMIC)
        sHints += " LOGARITHMIC ";
    if (iHints & FLUID_HINT_LOGARITHMIC)
        sHints += " INTEGER ";
    if (iHints & FLUID_HINT_FILENAME)
        sHints += " FILENAME ";
    if (iHints & FLUID_HINT_OPTIONLIST)
        sHints += " OPTIONLIST ";
*/
    bool bRealtime = (bool) ::fluid_settings_is_realtime(pFluidSettings, pszName);
    (pData->pListItem)->setText(iCol++, (bRealtime ? "yes" : "no"));

    switch (iType) {

      case FLUID_NUM_TYPE:
      {
        double fDefault  = ::fluid_settings_getnum_default(pFluidSettings, pszName);
        double fCurrent  = 0.0;
        double fRangeMin = 0.0;
        double fRangeMax = 0.0;
        ::fluid_settings_getnum(pFluidSettings, pszName, &fCurrent);
        ::fluid_settings_getnum_range(pFluidSettings, pszName, &fRangeMin, &fRangeMax);
        (pData->pListItem)->setText(iCol++, QString::number(fCurrent));
        (pData->pListItem)->setText(iCol++, QString::number(fDefault));
        (pData->pListItem)->setText(iCol++, QString::number(fRangeMin));
        (pData->pListItem)->setText(iCol++, QString::number(fRangeMax));
        break;
      }

      case FLUID_INT_TYPE:
      {
        int iDefault  = ::fluid_settings_getint_default(pFluidSettings, pszName);
        int iCurrent  = 0;
        int iRangeMin = 0;
        int iRangeMax = 0;
        ::fluid_settings_getint(pFluidSettings, pszName, &iCurrent);
        ::fluid_settings_getint_range(pFluidSettings, pszName, &iRangeMin, &iRangeMax);
        if (iRangeMin + iRangeMax < 2) {
            iRangeMin = 0;
            iRangeMax = 1;
        }
        (pData->pListItem)->setText(iCol++, QString::number(iCurrent));
        (pData->pListItem)->setText(iCol++, QString::number(iDefault));
        (pData->pListItem)->setText(iCol++, QString::number(iRangeMin));
        (pData->pListItem)->setText(iCol++, QString::number(iRangeMax));
        break;
      }

      case FLUID_STR_TYPE:
      {
        char *pszDefault = ::fluid_settings_getstr_default(pFluidSettings, pszName);
        char *pszCurrent = NULL;
        ::fluid_settings_getstr(pFluidSettings, pszName, &pszCurrent);
        (pData->pListItem)->setText(iCol++, pszCurrent);
        (pData->pListItem)->setText(iCol++, pszDefault);
        (pData->pListItem)->setText(iCol++, QString::null);
        (pData->pListItem)->setText(iCol++, QString::null);
        break;
      }
    }

    // Check for options.
    pData->options.clear();
    ::fluid_settings_foreach_option(pFluidSettings, pszName, pvData, qsynth_settings_foreach_option);
    QStringList::Iterator iter = pData->options.begin();
    QString sOptions = "";
    while (iter != pData->options.end()) {
        sOptions += *iter++;
        sOptions += " ";
    }
    (pData->pListItem)->setText(iCol++, sOptions.stripWhiteSpace());
}


// Kind of constructor.
void qsynthSetupForm::init (void)
{
    // No settings descriptor initially (the caller will set it).
    m_pSetup = NULL;
    m_pOptions = NULL;

    // Initialize dirty control state.
    m_iDirtySetup = 0;
    m_iDirtyCount = 0;

    // Check for pixmaps.
    m_pXpmSoundFont  = new QPixmap(QPixmap::fromMimeSource("sfont1.png"));

    // Set dialog validators...
    QRegExp rx("[\\w]+");
    DisplayNameLineEdit->setValidator(new QRegExpValidator(rx, DisplayNameLineEdit));
    SampleRateComboBox->setValidator(new QIntValidator(SampleRateComboBox));
    AudioBufSizeComboBox->setValidator(new QIntValidator(AudioBufSizeComboBox));
    AudioBufCountComboBox->setValidator(new QIntValidator(AudioBufCountComboBox));
    JackNameComboBox->setValidator(new QRegExpValidator(rx, JackNameComboBox));
    AlsaNameComboBox->setValidator(new QRegExpValidator(rx, AlsaNameComboBox));

    // No sorting on soundfont stack list.
    SoundFontListView->setSorting(-1);

    // Try to restore old window positioning.
    adjustSize();
}


// Kind of destructor.
void qsynthSetupForm::destroy (void)
{
    // Delete pixmaps.
    delete m_pXpmSoundFont;
}


// Populate (setup) dialog controls from settings descriptors.
void qsynthSetupForm::setup ( qsynthOptions *pOptions, qsynthEngine *pEngine, bool bNew )
{
    // Check this first.
    if (pOptions == NULL || pEngine == NULL)
        return;

    // Set reference descriptors.
    m_pOptions = pOptions;
    m_pSetup = pEngine->setup();

    // Update caption.
    setCaption(QSYNTH_TITLE ": " + tr("Setup") + " [" + pEngine->name() + "]");

    // Start clean?
    m_iDirtyCount = 0;
    if (bNew)
        m_iDirtyCount++;
    // Avoid nested changes.
    m_iDirtySetup++;

    // Display name.
    DisplayNameLineEdit->setText(m_pSetup->sDisplayName);
    
    // Load Setttings view...
    qsynth_settings_data data;
    // Set data context.
    data.pSetup    = m_pSetup;
    data.pListView = SettingsListView;
    data.pListItem = NULL;
    // And start filling it in...
    ::fluid_settings_foreach(m_pSetup->fluid_settings(), &data, qsynth_settings_foreach);

    // Midi Driver combobox options;
    // check if intended MIDI driver is available.
    data.options.clear();
    ::fluid_settings_foreach_option(m_pSetup->fluid_settings(), "midi.driver", &data, qsynth_settings_foreach_option);
    MidiDriverComboBox->clear();
    if (data.options.find(m_pSetup->sMidiDriver) == data.options.end())
        data.options.append(m_pSetup->sMidiDriver);
    MidiDriverComboBox->insertStringList(data.options);

    // Audio Driver combobox options.
    // check if intended Audio driver is available.
    data.options.clear();
    ::fluid_settings_foreach_option(m_pSetup->fluid_settings(), "audio.driver", &data, qsynth_settings_foreach_option);
    AudioDriverComboBox->clear();
    if (data.options.find(m_pSetup->sAudioDriver) == data.options.end())
        data.options.append(m_pSetup->sAudioDriver);
    AudioDriverComboBox->insertStringList(data.options);

    // Sample Format combobox options.
    data.options.clear();
    ::fluid_settings_foreach_option(m_pSetup->fluid_settings(), "audio.sample-format", &data, qsynth_settings_foreach_option);
    SampleFormatComboBox->clear();
    SampleFormatComboBox->insertStringList(data.options);

    // Midi settings...
    MidiInCheckBox->setChecked(m_pSetup->bMidiIn);
    MidiDriverComboBox->setCurrentText(m_pSetup->sMidiDriver);
    MidiChannelsSpinBox->setValue(m_pSetup->iMidiChannels);
    MidiDumpCheckBox->setChecked(m_pSetup->bMidiDump);
    VerboseCheckBox->setChecked(m_pSetup->bVerbose);
    // ALSA client identifier.
    AlsaNameComboBox->insertItem(m_pSetup->sDisplayName);
    AlsaNameComboBox->setCurrentText(bNew ? m_pSetup->sDisplayName : m_pSetup->sAlsaName);

    // Audio settings...
    AudioDriverComboBox->setCurrentText(m_pSetup->sAudioDriver);
    SampleFormatComboBox->setCurrentText(m_pSetup->sSampleFormat);
    SampleRateComboBox->setCurrentText(QString::number(m_pSetup->fSampleRate));
    AudioBufSizeComboBox->setCurrentText(QString::number(m_pSetup->iAudioBufSize));
    AudioBufCountComboBox->setCurrentText(QString::number(m_pSetup->iAudioBufCount));
    AudioChannelsSpinBox->setValue(m_pSetup->iAudioChannels);
    AudioGroupsSpinBox->setValue(m_pSetup->iAudioGroups);
    PolyphonySpinBox->setValue(m_pSetup->iPolyphony);
    JackMultiCheckBox->setChecked(m_pSetup->bJackMulti);
    JackAutoConnectCheckBox->setChecked(m_pSetup->bJackAutoConnect);
    // JACK client name...
    QString sJackName;
    if (m_pSetup->sDisplayName.find(QSYNTH_TITLE) < 0)
        sJackName += QSYNTH_TITLE "_";
    sJackName += m_pSetup->sDisplayName;
    JackNameComboBox->insertItem(sJackName);
    JackNameComboBox->setCurrentText(bNew ? sJackName : m_pSetup->sJackName);

    // Load the soundfonts view.
    SoundFontListView->clear();
    SoundFontListView->setUpdatesEnabled(false);
    QListViewItem *pItem = NULL;
    if (pEngine->pSynth) {
        // Load soundfont view from actual synth stack...
        int cSoundFonts = ::fluid_synth_sfcount(pEngine->pSynth);
        for (int i = cSoundFonts - 1; i >= 0; i--) {
            fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(pEngine->pSynth, i);
            if (pSoundFont) {
                pItem = new QListViewItem(SoundFontListView, pItem);
                if (pItem) {
                    pItem->setPixmap(0, *m_pXpmSoundFont);
                    pItem->setText(0, QString::number(pSoundFont->id));
                    pItem->setText(1, pSoundFont->get_name(pSoundFont));
                }
            }
        }
    } else {
        // Load soundfont view from configuration setup list...
        int i = 0;
        for (QStringList::Iterator iter = m_pSetup->soundfonts.begin(); iter != m_pSetup->soundfonts.end(); iter++) {
            pItem = new QListViewItem(SoundFontListView, pItem);
            if (pItem) {
                pItem->setPixmap(0, *m_pXpmSoundFont);
                pItem->setText(0, QString::number(i++));
                pItem->setText(1, *iter);
            }
        }
    }
    SoundFontListView->setUpdatesEnabled(true);
    SoundFontListView->update();

    // Done.
    m_iDirtySetup--;
    stabilizeForm();
}


// Accept settings (OK button slot).
void qsynthSetupForm::accept (void)
{
    if (m_iDirtyCount > 0) {
        // Save the soundfont view.
        m_pSetup->soundfonts.clear();
        for (QListViewItem *pItem = SoundFontListView->firstChild(); pItem; pItem = pItem->nextSibling())
            m_pSetup->soundfonts.append(pItem->text(1));
        // Will we have a setup renaming?
        m_pSetup->sDisplayName     = DisplayNameLineEdit->text();
        // Audio settings...
        m_pSetup->sAudioDriver     = AudioDriverComboBox->currentText();
        m_pSetup->sSampleFormat    = SampleFormatComboBox->currentText();
        m_pSetup->fSampleRate      = SampleRateComboBox->currentText().toDouble();
        m_pSetup->iAudioBufSize    = AudioBufSizeComboBox->currentText().toInt();
        m_pSetup->iAudioBufCount   = AudioBufCountComboBox->currentText().toInt();
        m_pSetup->iAudioChannels   = AudioChannelsSpinBox->value();
        m_pSetup->iAudioGroups     = AudioGroupsSpinBox->value();
        m_pSetup->iPolyphony       = PolyphonySpinBox->value();
        m_pSetup->bJackMulti       = JackMultiCheckBox->isChecked();
        m_pSetup->bJackAutoConnect = JackAutoConnectCheckBox->isChecked();
        m_pSetup->sJackName        = JackNameComboBox->currentText();
        // Midi settings...
        m_pSetup->bMidiIn          = MidiInCheckBox->isChecked();
        m_pSetup->sMidiDriver      = MidiDriverComboBox->currentText();
        m_pSetup->iMidiChannels    = MidiChannelsSpinBox->value();
        m_pSetup->bMidiDump        = MidiDumpCheckBox->isChecked();
        m_pSetup->bVerbose         = VerboseCheckBox->isChecked();
        m_pSetup->sAlsaName        = AlsaNameComboBox->currentText();
        // Reset dirty flag.
        m_iDirtyCount = 0;
    }

    // Just go with dialog acceptance.
    QDialog::accept();
}


// Reject settings (Cancel button slot).
void qsynthSetupForm::reject (void)
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


// Dirty up engine display name.
void qsynthSetupForm::nameChanged ( const QString& )
{
    if (m_iDirtySetup > 0)
        return;

    QListBox *pListBox;
    const QString& sDisplayName = DisplayNameLineEdit->text();

    pListBox = AlsaNameComboBox->listBox();
    if (pListBox->findItem(sDisplayName, Qt::ExactMatch) == NULL) {
        pListBox->removeItem(pListBox->count() - 1);
        pListBox->insertItem(DisplayNameLineEdit->text());
    }

    pListBox = JackNameComboBox->listBox();
    if (pListBox->findItem(sDisplayName, Qt::ExactMatch) == NULL) {
        pListBox->removeItem(pListBox->count() - 1);
        pListBox->insertItem(DisplayNameLineEdit->text());
    }

    settingsChanged();
}


// Dirty up settings.
void qsynthSetupForm::settingsChanged (void)
{
    if (m_iDirtySetup > 0)
        return;

    m_iDirtyCount++;
    stabilizeForm();
}


// Stabilize current form state.
void qsynthSetupForm::stabilizeForm (void)
{
    bool bEnabled = MidiInCheckBox->isChecked();
    MidiDriverTextLabel->setEnabled(bEnabled);
    MidiDriverComboBox->setEnabled(bEnabled);
    MidiChannelsTextLabel->setEnabled(bEnabled);
    MidiChannelsSpinBox->setEnabled(bEnabled);
    MidiDumpCheckBox->setEnabled(bEnabled);
    VerboseCheckBox->setEnabled(bEnabled);

    bool bAlsaEnabled = (bEnabled && MidiDriverComboBox->currentText() == "alsa_seq");
    AlsaNameTextLabel->setEnabled(bAlsaEnabled);
    AlsaNameComboBox->setEnabled(bAlsaEnabled);
    
    bool bJackEnabled = (AudioDriverComboBox->currentText() == "jack");
    JackMultiCheckBox->setEnabled(bJackEnabled);
    JackAutoConnectCheckBox->setEnabled(bJackEnabled);
    JackNameTextLabel->setEnabled(bJackEnabled);
    JackNameComboBox->setEnabled(bJackEnabled);

    SoundFontOpenPushButton->setEnabled(true);
    QListViewItem *pSelectedItem = SoundFontListView->selectedItem();
    if (pSelectedItem) {
        SoundFontRemovePushButton->setEnabled(true);
        SoundFontMoveUpPushButton->setEnabled(pSelectedItem->itemAbove() != NULL);
        SoundFontMoveDownPushButton->setEnabled(pSelectedItem->nextSibling() != NULL);
    } else {
        SoundFontRemovePushButton->setEnabled(false);
        SoundFontMoveUpPushButton->setEnabled(false);
        SoundFontMoveDownPushButton->setEnabled(false);
    }

    bEnabled = (m_iDirtyCount > 0);
    if (bEnabled && m_pSetup) {
        const QString& sDisplayName = DisplayNameLineEdit->text();
        if (sDisplayName != m_pSetup->sDisplayName) {
            bEnabled = (m_pOptions->engines.find(sDisplayName) == m_pOptions->engines.end());
            if (bEnabled)
                bEnabled = (sDisplayName != (m_pOptions->defaultSetup())->sDisplayName);
        }
    }
    OkPushButton->setEnabled(bEnabled);
}


// Soundfont view context menu handler.
void qsynthSetupForm::contextMenu( QListViewItem *pItem, const QPoint& pos, int )
{
    int iItemID;

    // Build the soundfont context menu...
    QPopupMenu* pContextMenu = new QPopupMenu(this);

    iItemID = pContextMenu->insertItem(tr("Open..."), this, SLOT(openSoundFont()));
    pContextMenu->insertSeparator();
    bool bEnabled = (pItem != NULL);
    iItemID = pContextMenu->insertItem(tr("Remove"), this, SLOT(removeSoundFont()));
    pContextMenu->setItemEnabled(iItemID, bEnabled);
    pContextMenu->insertSeparator();
    iItemID = pContextMenu->insertItem(tr("Move Up"), this, SLOT(moveUpSoundFont()));
    pContextMenu->setItemEnabled(iItemID, (bEnabled && pItem->itemAbove() != NULL));
    iItemID = pContextMenu->insertItem(tr("Move Down"), this, SLOT(moveDownSoundFont()));
    pContextMenu->setItemEnabled(iItemID, (bEnabled && pItem->nextSibling() != NULL));

    pContextMenu->exec(pos);
    
    delete pContextMenu;
}


// Refresh the soundfont view ids.
void qsynthSetupForm::refreshSoundFonts (void)
{
    SoundFontListView->setUpdatesEnabled(false);
    int i = 0;
    for (QListViewItem *pItem = SoundFontListView->firstChild(); pItem; pItem = pItem->nextSibling())
         pItem->setText(0, QString::number(i++));
    SoundFontListView->setUpdatesEnabled(true);
    SoundFontListView->update();
}


// Browse for a soundfont file on the filesystem.
void qsynthSetupForm::openSoundFont()
{
    QStringList soundfonts = QFileDialog::getOpenFileNames(
            tr("Soundfont files") + " (*.sf2 *.SF2)",   // Filter (SF2 files)
            m_pOptions->sSoundFontDir,                  // Start here.
            this, 0,                                    // Parent and name (none)
            tr("Soundfont files")                       // Caption.
    );
    
    QListViewItem *pItem = NULL;
    // For avery selected soundfont to load...
    for (QStringList::Iterator iter = soundfonts.begin(); iter != soundfonts.end(); iter++) {
        const QString& sSoundFont = *iter;
        char *pszFilename = (char *) sSoundFont.latin1();
        // Is it a soundfont file...
        if (::fluid_is_soundfont(pszFilename)) {
            // Check if not already there...
            if (SoundFontListView->findItem(sSoundFont, 1) &&
                QMessageBox::warning(this, tr("Warning"),
                    tr("Soundfont file already on list") + ":\n\n" +
                    "\"" + sSoundFont + "\"\n\n" +
                    tr("Add anyway?"),
                    tr("OK"), tr("Cancel")) > 0) {
                continue;
            }
            // Start inserting in the current selected or last item...
            if (pItem == NULL) {
                pItem = SoundFontListView->selectedItem();
                if (pItem)
                    pItem->setSelected(false);
                else
                    pItem = SoundFontListView->lastItem();
            }
            // New item on the block :-)
            pItem = new QListViewItem(SoundFontListView, pItem);
            if (pItem) {
                pItem->setPixmap(0, *m_pXpmSoundFont);
                pItem->setText(1, sSoundFont);
                pItem->setSelected(true);
                SoundFontListView->setCurrentItem(pItem);
                m_pOptions->sSoundFontDir = QFileInfo(sSoundFont).dirPath(true);
                m_iDirtyCount++;
            }
        }
        else {
            QMessageBox::critical(this, tr("Error"),
                tr("Failed to add soundfont file") + ":\n\n" +
                "\"" + sSoundFont + "\"\n\n" +
                tr("Please, check for a valid soundfont file."),
                tr("Cancel"));
        }
    }

    refreshSoundFonts();
    stabilizeForm();
}


// Remove current selected soundfont.
void qsynthSetupForm::removeSoundFont (void)
{
    QListViewItem *pItem = SoundFontListView->selectedItem();
    if (pItem) {
        m_iDirtyCount++;
        delete pItem;
    }
    
    refreshSoundFonts();
    stabilizeForm();
}


// Move current selected soundfont one position up.
void qsynthSetupForm::moveUpSoundFont (void)
{
    QListViewItem *pItem = SoundFontListView->selectedItem();
    if (pItem) {
        QListViewItem *pItemAbove = pItem->itemAbove();
        if (pItemAbove) {
            pItem->setSelected(false);
            pItemAbove = pItemAbove->itemAbove();
            if (pItemAbove) {
                pItem->moveItem(pItemAbove);
            } else {
                SoundFontListView->takeItem(pItem);
                SoundFontListView->insertItem(pItem);
            }
            pItem->setSelected(true);
            SoundFontListView->setCurrentItem(pItem);
            m_iDirtyCount++;
        }
    }
    
    refreshSoundFonts();
    stabilizeForm();
}


// Move current selected soundfont one position down.
void qsynthSetupForm::moveDownSoundFont (void)
{
    QListViewItem *pItem = SoundFontListView->selectedItem();
    if (pItem) {
        QListViewItem *pItemBelow = pItem->nextSibling();
        if (pItemBelow) {
            pItem->setSelected(false);
            pItem->moveItem(pItemBelow);
            pItem->setSelected(true);
            SoundFontListView->setCurrentItem(pItem);
            m_iDirtyCount++;
        }
    }

    refreshSoundFonts();
    stabilizeForm();
}


// end of qsynthSetupForm.ui.h

