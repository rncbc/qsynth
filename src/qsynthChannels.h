// qsynthChannels.h
//
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

#ifndef __qsynthChannels_h
#define __qsynthChannels_h

#include <qlistview.h>

// Column index helpers.
#define QSYNTH_CHANNELS_IN      0
#define QSYNTH_CHANNELS_CHAN    1
#define QSYNTH_CHANNELS_SFID    2
#define QSYNTH_CHANNELS_SFNAME  3
#define QSYNTH_CHANNELS_BANK    4
#define QSYNTH_CHANNELS_PROG    5
#define QSYNTH_CHANNELS_NAME    6


// The channels list view item.
class qsynthChannelsViewItem : public QListViewItem
{
public:

    // Constructor.
    qsynthChannelsViewItem(QListView *pParent);
    // Default destructor.
    ~qsynthChannelsViewItem();

    // Special column sorting virtual comparator.
    virtual int compare (QListViewItem* pItem, int iColumn, bool bAscending) const;
};

typedef qsynthChannelsViewItem * qsynthChannelsViewItemPtr;

#endif  // __qsynthChannels_h

// end of qsynthChannels.h

