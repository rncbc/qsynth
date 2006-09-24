// qsynthChannels.cpp
//
/****************************************************************************
   Copyright (C) 2003-2006, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qsynthChannels.h"


//-------------------------------------------------------------------------
// qsynthChannels - Prototype settings structure.
//

// Constructor.
qsynthChannelsViewItem::qsynthChannelsViewItem ( QListView *pParent )
    : QListViewItem(pParent)
{
}


// Default Destructor.
qsynthChannelsViewItem::~qsynthChannelsViewItem (void)
{
}

// Special column sorting virtual comparator.
int qsynthChannelsViewItem::compare ( QListViewItem* pItem, int iColumn, bool bAscending ) const
{
    switch (iColumn) {

        case QSYNTH_CHANNELS_BANK:
        case QSYNTH_CHANNELS_PROG:
        case QSYNTH_CHANNELS_SFID:
        {
            int iNum1 = text(iColumn).toInt();
            int iNum2 = pItem->text(iColumn).toInt();
            if (iNum1 > iNum2)
                return (bAscending ?  1 : -1);
            else if (iNum1 < iNum2)
                return (bAscending ? -1 :  1);
            break;
        }

        case QSYNTH_CHANNELS_NAME:
        case QSYNTH_CHANNELS_SFNAME:
        {
            const QString sName1 = text(iColumn);
            const QString sName2 = pItem->text(iColumn);
            if (sName1 > sName2)
                return (bAscending ?  1 : -1);
            else if (sName1 < sName2)
                return (bAscending ? -1 :  1);
            break;
        }
    }

    int iChan1 = text(QSYNTH_CHANNELS_CHAN).toInt();
    int iChan2 = pItem->text(QSYNTH_CHANNELS_CHAN).toInt();
    if (iChan1 > iChan2)
        return (bAscending ?  1 : -1);
    else
    if (iChan1 < iChan2)
        return (bAscending ? -1 :  1);

    return 0;
}

// end of qsynthChannels.cpp
