// qsynthSetupForm.ui.h
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
#include <qfiledialog.h>
#include <qfontdialog.h>

#include "config.h"

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
    // Try to restore old window positioning.
    adjustSize();

    // Final startup stabilization...
    stabilizeForm();
}


// Kind of destructor.
void qsynthSetupForm::destroy (void)
{
}


void qsynthSetupForm::stabilizeForm (void)
{
}


// The messages font selection dialog.
void qsynthSetupForm::chooseMessagesFont()
{
    bool  bOk  = false;
    QFont font = QFontDialog::getFont(&bOk, MessagesFontTextLabel->font(), this);
    if (bOk) {
        MessagesFontTextLabel->setFont(font);
        MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
    }
}


// Populate (load) dialog controls from setup struct members.
void qsynthSetupForm::load ( qsynthSetup *pSetup )
{
    // Load Setttings view...
    qsynth_settings_data data;
    // Set data context.
    data.pSetup    = pSetup;
    data.pListView = SettingsListView;
    data.pListItem = NULL;
    // And start filling it in...
    ::fluid_settings_foreach(pSetup->fluid_settings(), &data, qsynth_settings_foreach);

    // Load Options...
    QFont font;
    if (pSetup->sMessagesFont.isEmpty() || !font.fromString(pSetup->sMessagesFont))
        font = QFont("Terminal", 8);
    MessagesFontTextLabel->setFont(font);
    MessagesFontTextLabel->setText(font.family() + " " + QString::number(font.pointSize()));
}


// Populate (save) setup struct members from dialog controls.
void qsynthSetupForm::save ( qsynthSetup *pSetup )
{
    // Save Options...
    pSetup->sMessagesFont = MessagesFontTextLabel->font().toString();
}


// end of qsynthSetupForm.ui.h
