// qsynthSetupForm.cpp
//
/****************************************************************************
   Copyright (C) 2003-2024, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qsynthSetupForm.h"

#include "qsynthEngine.h"

#include <QValidator>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QFileInfo>
#include <QPixmap>
#include <QMenu>


// Our local parameter data struct.
struct qsynth_settings_data
{
	qsynthSetup     *pSetup;
	QTreeWidget     *pListView;
	QTreeWidgetItem *pListItem;
	QStringList      options;
};

static void qsynth_settings_foreach_option (
#ifdef CONFIG_FLUID_SETTINGS_FOREACH_OPTION
	void *pvData, const char *, const char *pszOption )
#else
	void *pvData, char *, char *pszOption )
#endif
{
	qsynth_settings_data *pData = (qsynth_settings_data *) pvData;

	pData->options.append(pszOption);
}

static void qsynth_settings_foreach (
#ifdef CONFIG_FLUID_SETTINGS_FOREACH
	void *pvData, const char *pszName, int iType )
#else
	void *pvData, char *pszName, int iType )
#endif
{
	qsynth_settings_data *pData = (qsynth_settings_data *) pvData;
	fluid_settings_t *pFluidSettings = (pData->pSetup)->fluid_settings();

	// Add the new list item.
	int iCol = 0;
	pData->pListItem = new QTreeWidgetItem(pData->pListView, pData->pListItem);
	(pData->pListItem)->setText(iCol++, pszName);
	(pData->pListItem)->setFlags(
		Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);

	// Check for type...
	QString sType = "?";
	switch (iType) {
	case FLUID_NUM_TYPE: sType = "num"; break;
	case FLUID_INT_TYPE: sType = "int"; break;
	case FLUID_STR_TYPE: sType = "str"; break;
	case FLUID_SET_TYPE: sType = "set"; break;
	}
	(pData->pListItem)->setText(iCol++, sType);
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
	#ifdef CONFIG_FLUID_SETTINGS_GETNUM_DEFAULT
		double fDefault = 0.0f;
		::fluid_settings_getnum_default(pFluidSettings, pszName, &fDefault);
	#else
		const double fDefault  = ::fluid_settings_getnum_default(pFluidSettings, pszName);
	#endif
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
	#ifdef CONFIG_FLUID_SETTINGS_GETINT_DEFAULT
		int iDefault = 0;
		::fluid_settings_getint_default(pFluidSettings, pszName, &iDefault);
	#else
		const int iDefault  = ::fluid_settings_getint_default(pFluidSettings, pszName);
	#endif
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
	#ifdef CONFIG_FLUID_SETTINGS_GETSTR_DEFAULT
		char *pszDefault = nullptr;
		::fluid_settings_getstr_default(pFluidSettings, pszName, &pszDefault);
	#else
		const char *pszDefault = ::fluid_settings_getstr_default(pFluidSettings, pszName);
	#endif
		char *pszCurrent = nullptr;
	#ifdef CONFIG_FLUID_SETTINGS_DUPSTR
		::fluid_settings_dupstr(pFluidSettings, pszName, &pszCurrent);
	#else
		::fluid_settings_getstr(pFluidSettings, pszName, &pszCurrent);
	#endif
		(pData->pListItem)->setText(iCol++, pszCurrent);
		(pData->pListItem)->setText(iCol++, pszDefault);
		(pData->pListItem)->setText(iCol++, QString());
		(pData->pListItem)->setText(iCol++, QString());
	#ifdef CONFIG_FLUID_SETTINGS_DUPSTR
	#ifdef CONFIG_FLUID_FREE
		::fluid_free(pszCurrent);
	#else
		::free(pszCurrent);
	#endif
	#endif
		break;
	}}

	// Check for options.
	pData->options.clear();
	::fluid_settings_foreach_option(pFluidSettings, pszName, pvData, qsynth_settings_foreach_option);
	(pData->pListItem)->setText(iCol++, pData->options.join(" "));
}


//----------------------------------------------------------------------------
// qsynthSetupForm -- UI wrapper form.

// Constructor.
qsynthSetupForm::qsynthSetupForm ( QWidget *pParent )
	: QDialog(pParent)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// No settings descriptor initially (the caller will set it).
	m_pSetup = nullptr;
	m_pOptions = nullptr;

	// Initialize dirty control state.
	m_iDirtySetup = 0;
	m_iDirtyCount = 0;

	// Check for pixmaps.
	m_pXpmSoundFont = new QPixmap(":/images/sfont1.png");

	// Way to keep track of current settings custom editor.
	m_pSettingsItemEditor = nullptr;

	// Set dialog validators...
	QRegularExpression rx("[\\w-]+");
	m_ui.DisplayNameLineEdit->setValidator(new QRegularExpressionValidator(rx, m_ui.DisplayNameLineEdit));
	m_ui.SampleRateComboBox->setValidator(new QIntValidator(m_ui.SampleRateComboBox));
	m_ui.AudioBufSizeComboBox->setValidator(new QIntValidator(m_ui.AudioBufSizeComboBox));
	m_ui.AudioBufCountComboBox->setValidator(new QIntValidator(m_ui.AudioBufCountComboBox));
	m_ui.JackNameComboBox->setValidator(new QRegularExpressionValidator(rx, m_ui.JackNameComboBox));
	m_ui.MidiNameComboBox->setValidator(new QRegularExpressionValidator(rx, m_ui.MidiNameComboBox));

	// No sorting on soundfont stack list.
	//m_ui.SoundFontListView->setSorting(-1);

	// Soundfonts list view...
	QHeaderView *pHeader = m_ui.SoundFontListView->header();
	pHeader->setDefaultAlignment(Qt::AlignLeft);
//	pHeader->setDefaultSectionSize(300);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
//	pHeader->setSectionResizeMode(QHeaderView::Custom);
	pHeader->setSectionsMovable(false);
#else
//	pHeader->setResizeMode(QHeaderView::Custom);
	pHeader->setMovable(false);
#endif
	pHeader->setStretchLastSection(true);

	m_ui.SoundFontListView->resizeColumnToContents(0);	// SFID.
	pHeader->resizeSection(1, 300);						// Name.
	m_ui.SoundFontListView->resizeColumnToContents(2);	// Offset.

	// Settings list view...
	m_ui.SettingsListView->setItemDelegate(
		new qsynthSettingsItemDelegate(this));

	pHeader = m_ui.SettingsListView->header();
	pHeader->setDefaultAlignment(Qt::AlignLeft);
//	pHeader->setDefaultSectionSize(160);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
//	pHeader->setSectionResizeMode(QHeaderView::Custom);
	pHeader->setSectionsMovable(false);
#else
//	pHeader->setResizeMode(QHeaderView::Custom);
	pHeader->setMovable(false);
#endif
	pHeader->setStretchLastSection(true);

	pHeader->resizeSection(0, 160);						// Name.
	m_ui.SettingsListView->resizeColumnToContents(1);	// Type.
	m_ui.SettingsListView->resizeColumnToContents(2);	// Realtime.
//	m_ui.SettingsListView->resizeColumnToContents(3);	// Current.
//	m_ui.SettingsListView->resizeColumnToContents(4);	// Default.
	m_ui.SettingsListView->resizeColumnToContents(5);	// Min.
	m_ui.SettingsListView->resizeColumnToContents(6);	// Max.
	m_ui.SettingsListView->resizeColumnToContents(7);	// Options.

	// Try to restore old window positioning.
	adjustSize();

	// UI connections...
	QObject::connect(m_ui.DisplayNameLineEdit,
		SIGNAL(textChanged(const QString&)),
		SLOT(nameChanged(const QString&)));
	QObject::connect(m_ui.MidiInCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.MidiDriverComboBox,
		SIGNAL(activated(int)),
		SLOT(midiDriverChanged(int)));
	QObject::connect(m_ui.MidiDeviceComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.MidiAutoConnectCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.MidiChannelsSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.VerboseCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.MidiDumpCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.MidiNameComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.MidiBankSelectComboBox,
		SIGNAL(activated(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.AudioDriverComboBox,
		SIGNAL(activated(int)),
		SLOT(audioDriverChanged(int)));
	QObject::connect(m_ui.AudioDeviceComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.SampleFormatComboBox,
		SIGNAL(activated(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.SampleRateComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.AudioBufSizeComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.AudioBufCountComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.AudioChannelsSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.AudioGroupsSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.PolyphonySpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.JackAutoConnectCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.JackMultiCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(settingsChanged()));
	QObject::connect(m_ui.JackNameComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(settingsChanged()));
#if defined(Q_OS_WINDOWS)
	QObject::connect(m_ui.WasapiExclusiveCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(settingsChanged()));
#endif
	QObject::connect(m_ui.SoundFontListView,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(soundfontContextMenu(const QPoint&)));
	QObject::connect(m_ui.SoundFontListView,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		SLOT(stabilizeForm()));
	QObject::connect(m_ui.SoundFontOpenPushButton,
		SIGNAL(clicked()),
		SLOT(openSoundFont()));
	QObject::connect(m_ui.SoundFontRemovePushButton,
		SIGNAL(clicked()),
		SLOT(removeSoundFont()));
	QObject::connect(m_ui.SoundFontEditPushButton,
		SIGNAL(clicked()),
		SLOT(editSoundFont()));
	QObject::connect(m_ui.SoundFontMoveUpPushButton,
		SIGNAL(clicked()),
		SLOT(moveUpSoundFont()));
	QObject::connect(m_ui.SoundFontMoveDownPushButton,
		SIGNAL(clicked()),
		SLOT(moveDownSoundFont()));
	QObject::connect(m_ui.SoundFontListView->itemDelegate(),
		SIGNAL(commitData(QWidget*)),
		SLOT(soundfontItemChanged()));
	QObject::connect(m_ui.SettingsListView,
		SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		SLOT(settingsItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
	QObject::connect(m_ui.SettingsListView,
		SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		SLOT(settingsItemActivated(QTreeWidgetItem *, int)));
	QObject::connect(m_ui.SettingsListView->itemDelegate(),
		SIGNAL(commitData(QWidget*)),
		SLOT(settingsItemChanged()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));
}


// Destructor.
qsynthSetupForm::~qsynthSetupForm (void)
{
	// Delete pixmaps.
	delete m_pXpmSoundFont;
}


// A combo-box text item setter helper.
void qsynthSetupForm::setComboBoxCurrentText (
	QComboBox *pComboBox, const QString& sText ) const
{
	if (pComboBox->isEditable()) {
		pComboBox->setEditText(sText);
	} else {
		int iIndex = pComboBox->findText(sText);
		if (iIndex < 0) {
			iIndex = 0;
			if (!sText.isEmpty())
				pComboBox->insertItem(0, sText);
		}
		pComboBox->setCurrentIndex(iIndex);
	}
}


// Populate (setup) dialog controls from settings descriptors.
void qsynthSetupForm::setup ( qsynthOptions *pOptions, qsynthEngine *pEngine, bool bNew )
{
	// Check this first.
	if (pOptions == nullptr || pEngine == nullptr)
		return;

	// Set reference descriptors.
	m_pOptions = pOptions;
	m_pSetup = pEngine->setup();

	// Make local copy to custom settings map...
	m_settings = m_pSetup->settings;

	// Update caption.
	setWindowTitle(pEngine->name() + " - " + tr("Setup"));

	// Start clean?
	m_iDirtyCount = 0;
	if (bNew) {
		m_pSetup->realize();
		++m_iDirtyCount;
	}
	// Avoid nested changes.
	++m_iDirtySetup;

	// Display name.
	m_ui.DisplayNameLineEdit->setText(m_pSetup->sDisplayName);

	// Load Setttings view...
	qsynth_settings_data data;
	// Set data context.
	data.pSetup    = m_pSetup;
	data.pListView = m_ui.SettingsListView;
	data.pListItem = nullptr;
	// And start filling it in...
	::fluid_settings_foreach(m_pSetup->fluid_settings(), &data, qsynth_settings_foreach);

	// Midi Driver combobox options;
	// check if intended MIDI driver is available.
	data.options.clear();
	char midi_driver[] = "midi.driver";
	::fluid_settings_foreach_option(m_pSetup->fluid_settings(),
		midi_driver, &data, qsynth_settings_foreach_option);
	m_ui.MidiDriverComboBox->clear();
	if (!data.options.contains(m_pSetup->sMidiDriver))
		data.options.append(m_pSetup->sMidiDriver);
	m_ui.MidiDriverComboBox->addItems(data.options);

	// Audio Driver combobox options.
	// check if intended Audio driver is available.
	data.options.clear();
	char audio_driver[] = "audio.driver";
	::fluid_settings_foreach_option(m_pSetup->fluid_settings(),
		audio_driver, &data, qsynth_settings_foreach_option);
	m_ui.AudioDriverComboBox->clear();
	if (!data.options.contains(m_pSetup->sAudioDriver))
		data.options.append(m_pSetup->sAudioDriver);
	m_ui.AudioDriverComboBox->addItems(data.options);

	// Sample Format combobox options.
	data.options.clear();
	char audio_sample_fmt[] = "audio.sample-format";
	::fluid_settings_foreach_option(m_pSetup->fluid_settings(),
		audio_sample_fmt, &data, qsynth_settings_foreach_option);
	m_ui.SampleFormatComboBox->clear();
	m_ui.SampleFormatComboBox->addItems(data.options);

	// Midi bank select combobox options
	data.options.clear();
	char midi_bank_select[] = "synth.midi-bank-select";
	::fluid_settings_foreach_option(m_pSetup->fluid_settings(),
		midi_bank_select, &data, qsynth_settings_foreach_option);
	m_ui.MidiBankSelectComboBox->clear();
	m_ui.MidiBankSelectComboBox->addItems(data.options);

	// Midi settings...
	m_ui.MidiInCheckBox->setChecked(m_pSetup->bMidiIn);
	setComboBoxCurrentText(m_ui.MidiDriverComboBox,
		m_pSetup->sMidiDriver);
	updateMidiDevices(m_pSetup->sMidiDriver);

	setComboBoxCurrentText(m_ui.MidiDeviceComboBox,
		m_pSetup->sMidiDevice);
	setComboBoxCurrentText(m_ui.MidiBankSelectComboBox,
		m_pSetup->sMidiBankSelect);
	m_ui.MidiChannelsSpinBox->setValue(m_pSetup->iMidiChannels);
	m_ui.MidiAutoConnectCheckBox->setChecked(m_pSetup->bMidiAutoConnect);
	m_ui.MidiDumpCheckBox->setChecked(m_pSetup->bMidiDump);
	m_ui.VerboseCheckBox->setChecked(m_pSetup->bVerbose);
	// ALSA client identifier.
	m_ui.MidiNameComboBox->addItem(m_pSetup->sDisplayName);
	setComboBoxCurrentText(m_ui.MidiNameComboBox,
		bNew ? m_pSetup->sDisplayName : m_pSetup->sMidiName);

	// Audio settings...
	setComboBoxCurrentText(m_ui.AudioDriverComboBox,
		m_pSetup->sAudioDriver);
	updateAudioDevices(m_pSetup->sAudioDriver);

	setComboBoxCurrentText(m_ui.AudioDeviceComboBox,
		m_pSetup->sAudioDevice);
	setComboBoxCurrentText(m_ui.SampleFormatComboBox,
		m_pSetup->sSampleFormat);
	setComboBoxCurrentText(m_ui.SampleRateComboBox,
		QString::number(m_pSetup->fSampleRate));
	setComboBoxCurrentText(m_ui.AudioBufSizeComboBox,
		QString::number(m_pSetup->iAudioBufSize));
	setComboBoxCurrentText(m_ui.AudioBufCountComboBox,
		QString::number(m_pSetup->iAudioBufCount));
	m_ui.AudioChannelsSpinBox->setValue(m_pSetup->iAudioChannels);
	m_ui.AudioGroupsSpinBox->setValue(m_pSetup->iAudioGroups);
	m_ui.PolyphonySpinBox->setValue(m_pSetup->iPolyphony);
	m_ui.JackMultiCheckBox->setChecked(m_pSetup->bJackMulti);
	m_ui.JackAutoConnectCheckBox->setChecked(m_pSetup->bJackAutoConnect);
#if defined(Q_OS_WINDOWS)
	m_ui.WasapiExclusiveCheckBox->setChecked(m_pSetup->bWasapiExclusive);
#else
	m_ui.WasapiExclusiveCheckBox->hide();
#endif
	// JACK client name...
	QString sJackName;
	if (!m_pSetup->sDisplayName.contains(QSYNTH_TITLE))
		sJackName += QSYNTH_TITLE "_";
	sJackName += m_pSetup->sDisplayName;
	m_ui.JackNameComboBox->addItem(sJackName);
	setComboBoxCurrentText(m_ui.JackNameComboBox,
		bNew ? sJackName : m_pSetup->sJackName);

	// Load the soundfonts view.
	m_ui.SoundFontListView->clear();
	m_ui.SoundFontListView->setUpdatesEnabled(false);
	QTreeWidgetItem *pItem = nullptr;
	if (pEngine->pSynth) {
		// Load soundfont view from actual synth stack...
		int cSoundFonts = ::fluid_synth_sfcount(pEngine->pSynth);
		for (int i = cSoundFonts - 1; i >= 0; --i) {
			fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(pEngine->pSynth, i);
			if (pSoundFont) {
				pItem = new QTreeWidgetItem(m_ui.SoundFontListView, pItem);
				if (pItem) {
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
					pItem->setIcon(0, *m_pXpmSoundFont);
					pItem->setText(0, QString::number(iSFID));
					pItem->setText(1, sSFName);
				#ifdef CONFIG_FLUID_BANK_OFFSET
					pItem->setText(2, QString::number(
						::fluid_synth_get_bank_offset(pEngine->pSynth, iSFID)));
					pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
				#endif
				}
			}
		}
	} else {
		// Load soundfont view from configuration setup list...
		int i = 0;
		QStringListIterator iter(m_pSetup->soundfonts);
		while (iter.hasNext()) {
			pItem = new QTreeWidgetItem(m_ui.SoundFontListView, pItem);
			if (pItem) {
				pItem->setIcon(0, *m_pXpmSoundFont);
				pItem->setText(0, QString::number(i));
				pItem->setText(1, iter.next());
			#ifdef CONFIG_FLUID_BANK_OFFSET
				pItem->setText(2, m_pSetup->bankoffsets[i]);
				pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
			#endif
			}
			++i;
		}
	}
	m_ui.SoundFontListView->setUpdatesEnabled(true);
	m_ui.SoundFontListView->update();

	// Done.
	m_iDirtySetup--;
	stabilizeForm();
}


// Settings accessors.
QTreeWidget *qsynthSetupForm::settingsListView (void) const
{
	return m_ui.SettingsListView;
}


qsynthSetup *qsynthSetupForm::engineSetup (void) const
{
	return m_pSetup;
}


void qsynthSetupForm::setSettingsItem ( const QString& sKey, const QString& sVal )
{
	m_settings.insert(sKey, sVal);
}


QString qsynthSetupForm::settingsItem ( const QString& sKey ) const
{
	return m_settings.value(sKey);
}


bool qsynthSetupForm::isSettingsItem ( const QString& sKey ) const
{
	return m_settings.contains(sKey);
}


void qsynthSetupForm::setSettingsItemEditor ( qsynthSettingsItemEditor *pItemEditor )
{
	m_pSettingsItemEditor = pItemEditor;
}


qsynthSettingsItemEditor *qsynthSetupForm::settingsItemEditor (void) const
{
	return m_pSettingsItemEditor;
}


// Accept settings (OK button slot).
void qsynthSetupForm::accept (void)
{
	if (m_iDirtyCount > 0) {
		// Save the soundfont view.
		m_pSetup->soundfonts.clear();
		m_pSetup->bankoffsets.clear();
		const int iItemCount = m_ui.SoundFontListView->topLevelItemCount();
		for (int i = 0; i < iItemCount; ++i) {
			QTreeWidgetItem *pItem = m_ui.SoundFontListView->topLevelItem(i);
			m_pSetup->soundfonts.append(pItem->text(1));
			m_pSetup->bankoffsets.append(pItem->text(2));
		}
		// Will we have a setup renaming?
		m_pSetup->sDisplayName     = m_ui.DisplayNameLineEdit->text();
		// Midi settings...
		m_pSetup->bMidiIn          = m_ui.MidiInCheckBox->isChecked();
		m_pSetup->sMidiDriver      = m_ui.MidiDriverComboBox->currentText();
		m_pSetup->sMidiDevice      = m_ui.MidiDeviceComboBox->currentText();
		m_pSetup->iMidiChannels    = m_ui.MidiChannelsSpinBox->value();
		m_pSetup->sMidiBankSelect  = m_ui.MidiBankSelectComboBox->currentText();
		m_pSetup->bMidiDump        = m_ui.MidiDumpCheckBox->isChecked();
		m_pSetup->bVerbose         = m_ui.VerboseCheckBox->isChecked();
		m_pSetup->sMidiName        = m_ui.MidiNameComboBox->currentText();
		m_pSetup->bMidiAutoConnect = m_ui.MidiAutoConnectCheckBox->isChecked();
		// Audio settings...
		m_pSetup->sAudioDriver     = m_ui.AudioDriverComboBox->currentText();
		m_pSetup->sAudioDevice     = m_ui.AudioDeviceComboBox->currentText();
		m_pSetup->sSampleFormat    = m_ui.SampleFormatComboBox->currentText();
		m_pSetup->fSampleRate      = m_ui.SampleRateComboBox->currentText().toDouble();
		m_pSetup->iAudioBufSize    = m_ui.AudioBufSizeComboBox->currentText().toInt();
		m_pSetup->iAudioBufCount   = m_ui.AudioBufCountComboBox->currentText().toInt();
		m_pSetup->iAudioChannels   = m_ui.AudioChannelsSpinBox->value();
		m_pSetup->iAudioGroups     = m_ui.AudioGroupsSpinBox->value();
		m_pSetup->iPolyphony       = m_ui.PolyphonySpinBox->value();
		m_pSetup->bJackMulti       = m_ui.JackMultiCheckBox->isChecked();
		m_pSetup->sJackName        = m_ui.JackNameComboBox->currentText();
		m_pSetup->bJackAutoConnect = m_ui.JackAutoConnectCheckBox->isChecked();
	#if defined(Q_OS_WINDOWS)
		m_pSetup->bWasapiExclusive = m_ui.WasapiExclusiveCheckBox->isChecked();
	#endif
		// Copy custom settings map...
		m_pSetup->settings = m_settings;
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
		switch (QMessageBox::warning(this,
			tr("Warning"),
			tr("Some settings have been changed.") + "\n\n" +
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


// Dirty up engine display name.
void qsynthSetupForm::nameChanged ( const QString& )
{
	if (m_iDirtySetup > 0)
		return;

	int iItem;
	const QString& sDisplayName = m_ui.DisplayNameLineEdit->text();

	iItem = m_ui.MidiNameComboBox->findText(sDisplayName);
	if (iItem >= 0) {
		m_ui.MidiNameComboBox->removeItem(iItem);
		m_ui.MidiNameComboBox->insertItem(0, sDisplayName);
	}

	iItem = m_ui.JackNameComboBox->findText(sDisplayName);
	if (iItem >= 0) {
		m_ui.JackNameComboBox->removeItem(iItem);
		m_ui.JackNameComboBox->insertItem(0, sDisplayName);
	}

	settingsChanged();
}


// Fill MIDI device options.
void qsynthSetupForm::updateMidiDevices ( const QString& sMidiDriver )
{
	qsynth_settings_data data;
	data.pSetup    = m_pSetup;
	data.pListView = nullptr;
	data.pListItem = nullptr;

	// MIDI Device combobox options;
	QString sOldText = m_ui.MidiDeviceComboBox->currentText();
	m_ui.MidiDeviceComboBox->clear();
	QString sName = "midi." + sMidiDriver + ".device";
	data.options.clear();
	::fluid_settings_foreach_option(m_pSetup->fluid_settings(),
		sName.toLocal8Bit().data(), &data, qsynth_settings_foreach_option);

	m_ui.MidiDeviceComboBox->addItems(data.options);
}


void qsynthSetupForm::midiDriverChanged ( int )
{
	if (m_iDirtySetup > 0)
		return;

	QString sMidiDriver = m_ui.MidiDriverComboBox->currentText();
	QString sMidiDevice = m_ui.MidiDeviceComboBox->currentText();
	updateMidiDevices(sMidiDriver);
	setComboBoxCurrentText(m_ui.MidiDeviceComboBox, sMidiDevice);

	settingsChanged();
}


// Fill audio device options.
void qsynthSetupForm::updateAudioDevices ( const QString& sAudioDriver )
{
	qsynth_settings_data data;
	data.pSetup    = m_pSetup;
	data.pListView = nullptr;
	data.pListItem = nullptr;

	// Audio Device combobox options;
	QString sOldText = m_ui.AudioDeviceComboBox->currentText();
	m_ui.AudioDeviceComboBox->clear();
	QString sName = "audio." + sAudioDriver + ".device";
	data.options.clear();
	::fluid_settings_foreach_option(m_pSetup->fluid_settings(),
		sName.toLocal8Bit().data(), &data, qsynth_settings_foreach_option);

	m_ui.AudioDeviceComboBox->addItems(data.options);
}

void qsynthSetupForm::audioDriverChanged ( int )
{
	if (m_iDirtySetup > 0)
		return;

	const QString& sAudioDriver = m_ui.AudioDriverComboBox->currentText();
	const QString& sAudioDevice = m_ui.AudioDeviceComboBox->currentText();
	updateAudioDevices(sAudioDriver);
	setComboBoxCurrentText(m_ui.AudioDeviceComboBox, sAudioDevice);

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
	bool bEnabled = m_ui.MidiInCheckBox->isChecked();

	const bool bAlsaEnabled = (m_ui.MidiDriverComboBox->currentText() == "alsa_seq");
	const bool bCoreMidiEnabled = (m_ui.MidiDriverComboBox->currentText() == "coremidi");
	m_ui.MidiDriverTextLabel->setEnabled(bEnabled);
	m_ui.MidiDriverComboBox->setEnabled(bEnabled);
	m_ui.MidiDeviceTextLabel->setEnabled(bEnabled && !bAlsaEnabled);
	m_ui.MidiDeviceComboBox->setEnabled(bEnabled && !bAlsaEnabled);
	m_ui.MidiChannelsTextLabel->setEnabled(bEnabled);
	m_ui.MidiChannelsSpinBox->setEnabled(bEnabled);
	m_ui.MidiDumpCheckBox->setEnabled(bEnabled);
	m_ui.VerboseCheckBox->setEnabled(bEnabled);
	m_ui.MidiBankSelectTextLabel->setEnabled(bEnabled);
	m_ui.MidiBankSelectComboBox->setEnabled(bEnabled);
	m_ui.MidiNameTextLabel->setEnabled(bEnabled && (bAlsaEnabled | bCoreMidiEnabled));
	m_ui.MidiNameComboBox->setEnabled(bEnabled && (bAlsaEnabled | bCoreMidiEnabled));
#if FLUIDSYNTH_VERSION_MAJOR >= 2
	m_ui.MidiAutoConnectCheckBox->setEnabled(bEnabled);
#else
	m_ui.MidiAutoConnectCheckBox->setEnabled(false);
#endif
	const bool bJackEnabled = (m_ui.AudioDriverComboBox->currentText() == "jack");
	const bool bJackMultiEnabled = m_ui.JackMultiCheckBox->isChecked();
#if defined(Q_OS_WINDOWS)
	const bool bWasapiEnabled = (m_ui.AudioDriverComboBox->currentText() == "wasapi");
#endif
	m_ui.AudioDeviceTextLabel->setEnabled(!bJackEnabled);
	m_ui.AudioDeviceComboBox->setEnabled(!bJackEnabled);
	m_ui.JackMultiCheckBox->setEnabled(bJackEnabled);
	m_ui.JackAutoConnectCheckBox->setEnabled(bJackEnabled);
	m_ui.JackNameTextLabel->setEnabled(bJackEnabled);
	m_ui.JackNameComboBox->setEnabled(bJackEnabled);
#if defined(Q_OS_WINDOWS)
	m_ui.WasapiExclusiveCheckBox->setEnabled(bWasapiEnabled);
#endif
	if (bJackEnabled) {
		m_ui.AudioChannelsTextLabel->setEnabled(bJackMultiEnabled);
		m_ui.AudioChannelsSpinBox->setEnabled(bJackMultiEnabled);
		m_ui.AudioChannelsSpinBox->setSingleStep(2);
		m_ui.AudioChannelsSpinBox->setMinimum(2);
		m_ui.AudioGroupsTextLabel->setEnabled(bJackMultiEnabled);
		m_ui.AudioGroupsSpinBox->setEnabled(bJackMultiEnabled);
	//	m_ui.AudioGroupsSpinBox->setSingleStep(2);
	//	m_ui.AudioGroupsSpinBox->setMinimum(2);
	}

	m_ui.SoundFontOpenPushButton->setEnabled(true);
	QTreeWidgetItem *pSelectedItem = m_ui.SoundFontListView->currentItem();
	if (pSelectedItem) {
		const int iItem = m_ui.SoundFontListView->indexOfTopLevelItem(pSelectedItem);
		const int iItemCount = m_ui.SoundFontListView->topLevelItemCount();
		m_ui.SoundFontEditPushButton->setEnabled(
				pSelectedItem->flags() & Qt::ItemIsEditable);
		m_ui.SoundFontRemovePushButton->setEnabled(true);
		m_ui.SoundFontMoveUpPushButton->setEnabled(iItem > 0);
		m_ui.SoundFontMoveDownPushButton->setEnabled(iItem < iItemCount - 1);
	} else {
		m_ui.SoundFontRemovePushButton->setEnabled(false);
		m_ui.SoundFontEditPushButton->setEnabled(false);
		m_ui.SoundFontMoveUpPushButton->setEnabled(false);
		m_ui.SoundFontMoveDownPushButton->setEnabled(false);
	}

	bEnabled = (m_iDirtyCount > 0);
	if (bEnabled && m_pSetup) {
		const QString& sDisplayName = m_ui.DisplayNameLineEdit->text();
		if (sDisplayName != m_pSetup->sDisplayName) {
			bEnabled = (!m_pOptions->engines.contains(sDisplayName));
			if (bEnabled)
				bEnabled = (sDisplayName != (m_pOptions->defaultSetup())->sDisplayName);
		}
	}
	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bEnabled);
}


// Soundfont view context menu handler.
void qsynthSetupForm::soundfontContextMenu ( const QPoint& pos )
{
	int iItem = 0;
	int iItemCount = 0;
	QTreeWidgetItem *pItem = m_ui.SoundFontListView->itemAt(pos);
	if (pItem == nullptr)
		pItem = m_ui.SoundFontListView->currentItem();
	if (pItem) {
		iItem = m_ui.SoundFontListView->indexOfTopLevelItem(pItem);
		iItemCount = m_ui.SoundFontListView->topLevelItemCount();
	}

	// Build the soundfont context menu...
	QMenu menu(this);
	QAction *pAction;

	pAction = menu.addAction(
		QIcon(":/images/open1.png"),
		tr("Open..."), this, SLOT(openSoundFont()));
	menu.addSeparator();
	bool bEnabled = (pItem != nullptr);
	pAction = menu.addAction(
		QIcon(":/images/edit1.png"),
		tr("Edit"), this, SLOT(editSoundFont()));
	pAction->setEnabled(
			(bEnabled && (pItem->flags() & Qt::ItemIsEditable)));
	pAction = menu.addAction(
		QIcon(":/images/remove1.png"),
		tr("Remove"), this, SLOT(removeSoundFont()));
	pAction->setEnabled(bEnabled);
	menu.addSeparator();
	pAction = menu.addAction(
		QIcon(":/images/up1.png"),
		tr("Move Up"), this, SLOT(moveUpSoundFont()));
	pAction->setEnabled(bEnabled && iItem > 0);
	pAction = menu.addAction(
		QIcon(":/images/down1.png"),
		tr("Move Down"), this, SLOT(moveDownSoundFont()));
	pAction->setEnabled(bEnabled && iItem < iItemCount - 1);

	menu.exec((m_ui.SoundFontListView->viewport())->mapToGlobal(pos));
}


// Refresh the soundfont view ids.
void qsynthSetupForm::refreshSoundFonts (void)
{
	m_ui.SoundFontListView->setUpdatesEnabled(false);
	const int iItemCount = m_ui.SoundFontListView->topLevelItemCount();
	for (int i = 0; i < iItemCount; ++i) {
		QTreeWidgetItem *pItem = m_ui.SoundFontListView->topLevelItem(i);
		pItem->setText(0, QString::number(i + 1));
	}
	m_ui.SoundFontListView->setUpdatesEnabled(true);
	m_ui.SoundFontListView->update();
}


// Browse for a soundfont file on the filesystem.
void qsynthSetupForm::openSoundFont (void)
{
	QStringList soundfonts = QFileDialog::getOpenFileNames(this,
		tr("Soundfont files"), m_pOptions->sSoundFontDir,
		tr("Soundfont files") + " (*.sf2 *.SF2 *.sf3 *.SF3 *.dls *.DLS);;" +
		tr("All files") + " (*.*)");

/* Note: Qt's own File Dialog file filters are case insensitive, but when
   using platform native dialogs this may not be true, like for instance:
   QGtk3FileDialogHelper; see: https://bugreports.qt.io/browse/QTBUG-51712 */

	QTreeWidgetItem *pItem = nullptr;

	// For avery selected soundfont to load...
	QStringListIterator iter(soundfonts);
	while (iter.hasNext()) {
		const QString& sSoundFont = iter.next();
		// Is it a soundfont file...
		if (::fluid_is_soundfont(sSoundFont.toLocal8Bit().data())) {
			// Check if not already there...
			if (!m_ui.SoundFontListView->findItems(
					sSoundFont, Qt::MatchExactly, 1).isEmpty() &&
				QMessageBox::warning(this,
					tr("Warning"),
					tr("Soundfont file already on list") + ":\n\n" +
					"\"" + sSoundFont + "\"\n\n" +
					tr("Add anyway?"),
					QMessageBox::Ok | QMessageBox::Cancel)
				== QMessageBox::Cancel) {
				continue;
			}
			// Start inserting in the current selected or last item...
			if (pItem == nullptr) {
				pItem = m_ui.SoundFontListView->currentItem();
				if (pItem)
					pItem->setSelected(false);
			}
			// New item on the block :-)
			pItem = new QTreeWidgetItem(m_ui.SoundFontListView, pItem);
			if (pItem) {
				pItem->setIcon(0, *m_pXpmSoundFont);
				pItem->setText(1, sSoundFont);
			#ifdef CONFIG_FLUID_BANK_OFFSET
				pItem->setText(2, "0");
				pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
			#endif
				pItem->setSelected(true);
				m_ui.SoundFontListView->setCurrentItem(pItem);
				m_pOptions->sSoundFontDir = QFileInfo(sSoundFont).dir().absolutePath();
				++m_iDirtyCount;
			}
		} else {
			QMessageBox::critical(this,
				tr("Error"),
				tr("Failed to add soundfont file") + ":\n\n" +
				"\"" + sSoundFont + "\"\n\n" +
				tr("Please, check for a valid soundfont file."),
				QMessageBox::Cancel);
		}
	}

	refreshSoundFonts();
	stabilizeForm();
}


// Edit current selected soundfont.
void qsynthSetupForm::editSoundFont (void)
{
	QTreeWidgetItem *pItem = m_ui.SoundFontListView->currentItem();
	if (pItem)
		m_ui.SoundFontListView->editItem(pItem, 2);

	stabilizeForm();
}


// Remove current selected soundfont.
void qsynthSetupForm::removeSoundFont (void)
{
	QTreeWidgetItem *pItem = m_ui.SoundFontListView->currentItem();
	if (pItem) {
		++m_iDirtyCount;
		delete pItem;
	}

	refreshSoundFonts();
	stabilizeForm();
}


// Move current selected soundfont one position up.
void qsynthSetupForm::moveUpSoundFont (void)
{
	QTreeWidgetItem *pItem = m_ui.SoundFontListView->currentItem();
	if (pItem) {
		int iItem = m_ui.SoundFontListView->indexOfTopLevelItem(pItem);
		if (iItem > 0) {
			pItem->setSelected(false);
			pItem = m_ui.SoundFontListView->takeTopLevelItem(iItem);
			m_ui.SoundFontListView->insertTopLevelItem(iItem - 1, pItem);
			pItem->setSelected(true);
			m_ui.SoundFontListView->setCurrentItem(pItem);
			++m_iDirtyCount;
		}
	}

	refreshSoundFonts();
	stabilizeForm();
}


// Move current selected soundfont one position down.
void qsynthSetupForm::moveDownSoundFont (void)
{
	QTreeWidgetItem *pItem = m_ui.SoundFontListView->currentItem();
	if (pItem) {
		const int iItemCount = m_ui.SoundFontListView->topLevelItemCount();
		int iItem = m_ui.SoundFontListView->indexOfTopLevelItem(pItem);
		if (iItem < iItemCount - 1) {
			pItem->setSelected(false);
			pItem = m_ui.SoundFontListView->takeTopLevelItem(iItem);
			m_ui.SoundFontListView->insertTopLevelItem(iItem + 1, pItem);
			pItem->setSelected(true);
			m_ui.SoundFontListView->setCurrentItem(pItem);
			m_iDirtyCount++;
		}
	}

	refreshSoundFonts();
	stabilizeForm();
}


// Check soundfont bank offset edit.
void qsynthSetupForm::soundfontItemChanged (void)
{
	QTreeWidgetItem *pItem = m_ui.SoundFontListView->currentItem();
	if (pItem) {
		const int iBankOffset = pItem->text(2).toInt();
		if (iBankOffset < 0 || iBankOffset > 16384)
			pItem->setText(2, QString::number(0));
		m_iDirtyCount++;
	}

	stabilizeForm();
}


// Settings list-view slots.
void qsynthSetupForm::settingsItemActivated (
	QTreeWidgetItem *pItem, int iColumn )
{
	m_ui.SettingsListView->editItem(pItem, 3);
}



void qsynthSetupForm::settingsItemChanged ( QTreeWidgetItem *pItem, QTreeWidgetItem * )
{
	qsynthSettingsItemEditor *pItemEditor = settingsItemEditor();
	if (pItemEditor) {
		QTreeWidget *pListView = settingsListView();
		QAbstractItemDelegate *pItemDelegate = pListView->itemDelegate();
		if (pItemDelegate)
			pItemDelegate->destroyEditor(pItemEditor, pItemEditor->index());
	}
}


void qsynthSetupForm::settingsItemChanged (void)
{
	++m_iDirtyCount;
	stabilizeForm();
}


//-------------------------------------------------------------------------
// qsynthSettingsItemEditor - list-view item editor widget impl.

#include <QToolButton>

// Constructor.
qsynthSettingsItemEditor::qsynthSettingsItemEditor (
	qsynthSetupForm *pSetupForm, const QModelIndex& index, QWidget *pParent )
	: QWidget(pParent), m_pSetupForm(pSetupForm), m_index(index)
{
	QWidget *pEditor = nullptr;

	QTreeWidget *pListView = m_pSetupForm->settingsListView();
	QTreeWidgetItem *pListItem = pListView->topLevelItem(m_index.row());
	m_sCurrentKey = pListItem->text(0);
	m_sCurrentValue = pListItem->text(3);
	m_sDefaultValue = pListItem->text(4);

	qsynthSetup *pSetup = m_pSetupForm->engineSetup();
	fluid_settings_t *pFluidSettings = pSetup->fluid_settings();
	const QByteArray aKey = m_sCurrentKey.toLocal8Bit();
	const char *pszKey = aKey.constData();
	switch (::fluid_settings_get_type(pFluidSettings, pszKey)) {
	case FLUID_INT_TYPE:
	{
		m_type = SpinBox;
		m_u.pSpinBox = new QSpinBox(/*this*/);
		int iRangeMin = 0;
		int iRangeMax = 0;
		::fluid_settings_getint_range(
			pFluidSettings, pszKey,
			&iRangeMin, &iRangeMax);
		m_u.pSpinBox->setMinimum(iRangeMin);
		m_u.pSpinBox->setMaximum(iRangeMax);
		m_u.pSpinBox->setAccelerated(true);
		m_u.pSpinBox->setStepType(
			QAbstractSpinBox::AdaptiveDecimalStepType);
		QObject::connect(m_u.pSpinBox,
			SIGNAL(valueChanged(int)),
			SLOT(committed())
		);
		pEditor = m_u.pSpinBox;
		break;
	}
	case FLUID_NUM_TYPE:
	{
		m_type = DoubleSpinBox;
		m_u.pDoubleSpinBox = new QDoubleSpinBox(/*this*/);
		double fRangeMin = 0.0;
		double fRangeMax = 0.0;
		::fluid_settings_getnum_range(
			pFluidSettings, pszKey,
			&fRangeMin, &fRangeMax);
		m_u.pDoubleSpinBox->setMinimum(fRangeMin);
		m_u.pDoubleSpinBox->setMaximum(fRangeMax);
		m_u.pDoubleSpinBox->setAccelerated(true);
		m_u.pDoubleSpinBox->setStepType(
			QAbstractSpinBox::AdaptiveDecimalStepType);
		QObject::connect(m_u.pDoubleSpinBox,
			SIGNAL(valueChanged(double)),
			SLOT(committed())
		);
		pEditor = m_u.pDoubleSpinBox;
		break;
	}
	case FLUID_STR_TYPE:
	case FLUID_SET_TYPE:
	default:
	{
		qsynth_settings_data data;
		data.pSetup    = pSetup;
		data.pListView = pListView;
		data.pListItem = pListItem;
		::fluid_settings_foreach_option(
			pFluidSettings, pszKey, &data,
			qsynth_settings_foreach_option);
		if (data.options.isEmpty()) {
			m_type = LineEdit;
			m_u.pLineEdit = new QLineEdit(/*this*/);
			QObject::connect(m_u.pLineEdit,
				SIGNAL(textChanged(const QString&)),
				SLOT(changed())
			);
			QObject::connect(m_u.pLineEdit,
				SIGNAL(editingFinished()),
				SLOT(committed())
			);
			pEditor = m_u.pLineEdit;
		} else {
			m_type = ComboBox;
			m_u.pComboBox = new QComboBox(/*this*/);
			m_u.pComboBox->setEditable(false);
			m_u.pComboBox->addItems(data.options);
			QObject::connect(m_u.pComboBox,
				SIGNAL(activated(int)),
				SLOT(committed())
			);
			pEditor = m_u.pComboBox;
		}
		break;
	}}

	m_pToolButton = new QToolButton(/*this*/);
	m_pToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
//	m_pToolButton->setIconSize(QSize(18, 18));
	m_pToolButton->setIcon(QPixmap(":/images/itemReset.png"));

	QHBoxLayout *pLayout = new QHBoxLayout();
	pLayout->setSpacing(0);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->addWidget(pEditor);
	pLayout->addWidget(m_pToolButton);
	QWidget::setLayout(pLayout);

	QWidget::setFocusPolicy(Qt::StrongFocus);
	QWidget::setFocusProxy(pEditor);

	QObject::connect(m_pToolButton,
		SIGNAL(clicked()),
		SLOT(reset()));
}


// Destructor.
qsynthSettingsItemEditor::~qsynthSettingsItemEditor (void)
{
	m_pSetupForm->setSettingsItemEditor(nullptr);
}


// Target index accessor.
const QModelIndex& qsynthSettingsItemEditor::index (void) const
{
	return m_index;
}


// Value accessors.
void qsynthSettingsItemEditor::setValue ( const QString& sValue )
{
	switch (m_type) {
	case SpinBox:
		m_u.pSpinBox->setValue(sValue.toInt());
		break;
	case DoubleSpinBox:
		m_u.pDoubleSpinBox->setValue(sValue.toDouble());
		break;
	case LineEdit:
		m_u.pLineEdit->setText(sValue);
		break;
	case ComboBox:
		m_u.pComboBox->setCurrentText(sValue);
		break;
	}

	changed();
}


QString qsynthSettingsItemEditor::value (void) const
{
	QString sValue;

	switch (m_type) {
	case SpinBox:
		sValue = QString::number(m_u.pSpinBox->value());
		break;
	case DoubleSpinBox:
		sValue = QString::number(m_u.pDoubleSpinBox->value());
		break;
	case LineEdit:
		sValue = m_u.pLineEdit->text();
		break;
	case ComboBox:
		sValue = m_u.pComboBox->currentText();
		break;
	}

	return sValue;
}


// Current/Default value accessors.
const QString& qsynthSettingsItemEditor::currentKey (void) const
{
	return m_sCurrentKey;
}


const QString& qsynthSettingsItemEditor::currentValue (void) const
{
	return m_sCurrentValue;
}


const QString& qsynthSettingsItemEditor::defaultValue (void) const
{
	return m_sDefaultValue;
}


// Local interaction slots.
void qsynthSettingsItemEditor::changed (void)
{
	const QString& sValue = value();
	m_pToolButton->setEnabled(
		sValue != currentValue() ||
		sValue != defaultValue());
}


void qsynthSettingsItemEditor::committed (void)
{
	changed();

	emit commitEditor(this);
}


void qsynthSettingsItemEditor::reset (void)
{
	const QString& sValue = value();
	if (sValue == currentValue())
		setValue(defaultValue());
	else
		setValue(currentValue());
}


//-------------------------------------------------------------------------
// qsynthSettingsItemDelegate - list-view item delegate impl.

qsynthSettingsItemDelegate::qsynthSettingsItemDelegate (
	qsynthSetupForm *pSetupForm )
	: QItemDelegate(pSetupForm->settingsListView()), m_pSetupForm(pSetupForm)
{
}


// Overridden paint method.
void qsynthSettingsItemDelegate::paint ( QPainter *pPainter,
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	QStyleOptionViewItem option2 = option;
	if (index.column() == 0 || index.column() == 3) {
		QTreeWidget *pListView = m_pSetupForm->settingsListView();
		QTreeWidgetItem *pListItem = pListView->topLevelItem(index.row());
		const QString& sKey = pListItem->text(0);
		if (m_pSetupForm->isSettingsItem(sKey)) {
			option2.font.setBold(true);
		}
	}

	QItemDelegate::paint(pPainter, option2, index);
}


QWidget *qsynthSettingsItemDelegate::createEditor ( QWidget *pParent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index ) const
{
	qsynthSettingsItemEditor *pItemEditor
		= new qsynthSettingsItemEditor(m_pSetupForm, index, pParent);
	QObject::connect(pItemEditor,
		SIGNAL(commitEditor(QWidget *)),
		SLOT(commitEditor(QWidget *)));
	m_pSetupForm->setSettingsItemEditor(pItemEditor);
	return pItemEditor;
}


void qsynthSettingsItemDelegate::setEditorData ( QWidget *pEditor,
	const QModelIndex& index ) const
{
	qsynthSettingsItemEditor *pItemEditor
		= qobject_cast<qsynthSettingsItemEditor *> (pEditor);
	if (pItemEditor)
		pItemEditor->setValue(index.model()->data(index, Qt::DisplayRole).toString());
}


void qsynthSettingsItemDelegate::setModelData ( QWidget *pEditor,
	QAbstractItemModel *pModel, const QModelIndex& index ) const
{
	qsynthSettingsItemEditor *pItemEditor
		= qobject_cast<qsynthSettingsItemEditor *> (pEditor);
	if (pItemEditor)
		pModel->setData(index, pItemEditor->value());
}


QSize qsynthSettingsItemDelegate::sizeHint (
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	return QItemDelegate::sizeHint(option, index) + QSize(4, 4);
}


void qsynthSettingsItemDelegate::commitEditor ( QWidget *pEditor )
{
	qsynthSettingsItemEditor *pItemEditor
		= qobject_cast<qsynthSettingsItemEditor *> (pEditor);
	if (pItemEditor) {
		const QString& sVal = pItemEditor->value();
		if (sVal != pItemEditor->currentValue()) {
			m_pSetupForm->setSettingsItem(pItemEditor->currentKey(), sVal);
			emit commitData(pEditor);
		}
	}
}


// end of qsynthSetupForm.cpp
