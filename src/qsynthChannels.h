// qsynthChannels.h
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

#ifndef __qsynthChannels_h
#define __qsynthChannels_h

#include <QTreeWidget>

// Column index helpers.
#define QSYNTH_CHANNELS_IN      0
#define QSYNTH_CHANNELS_CHAN    1
#define QSYNTH_CHANNELS_BANK    2
#define QSYNTH_CHANNELS_PROG    3
#define QSYNTH_CHANNELS_NAME    4
#define QSYNTH_CHANNELS_SFID    5
#define QSYNTH_CHANNELS_SFNAME  6


// The channels list view item.
class qsynthChannelsItem : public QTreeWidgetItem
{
public:

	// Constructor.
	qsynthChannelsItem(QTreeWidget *pParent);
	// Default destructor.
	~qsynthChannelsItem();

	// Special column sorting virtual comparator.
	// Sort/compare overriden method.
	bool operator< (const QTreeWidgetItem& other) const;
};

typedef qsynthChannelsItem * qsynthChannelsItemPtr;


#endif  // __qsynthChannels_h

// end of qsynthChannels.h
