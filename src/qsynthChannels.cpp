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
// qsynthChannelsItem - Channel view item class.
//

// Constructor.
qsynthChannelsItem::qsynthChannelsItem ( QTreeWidget *pParent )
	: QTreeWidgetItem(pParent)
{
}


// Default Destructor.
qsynthChannelsItem::~qsynthChannelsItem (void)
{
}


// Special column sorting virtual comparator.
// Sort/compare overriden method.
bool qsynthChannelsItem::operator< (const QTreeWidgetItem& other) const
{
	int iColumn = QTreeWidgetItem::treeWidget()->sortColumn();
	const QString& s1 = text(iColumn);
	const QString& s2 = other.text(iColumn);
	if (iColumn == QSYNTH_CHANNELS_CHAN ||
		iColumn == QSYNTH_CHANNELS_BANK ||
		iColumn == QSYNTH_CHANNELS_PROG ||
		iColumn == QSYNTH_CHANNELS_SFID) {
		return (s1.toInt() < s2.toInt());
	} else {
		return (s1 < s2);
	}
}


// end of qsynthChannels.cpp
