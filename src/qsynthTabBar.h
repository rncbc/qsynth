// qsynthTabBar.h
//
/****************************************************************************
   Copyright (C) 2003-2013, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthTabBar_h
#define __qsynthTabBar_h

#include <QTabBar>

// Forward declarations.
class qsynthEngine;


//-------------------------------------------------------------------------
// qsynthTabBar - Instance tab widget class.
//


class qsynthTabBar : public QTabBar
{
	Q_OBJECT
	
public:

	// Constructor.
	qsynthTabBar(QWidget *pParent);
	// Destructor.
	~qsynthTabBar();

	// Engine accessor.
	qsynthEngine *engine(int iTab) const;
	// Current engine accessor.
	qsynthEngine *currentEngine() const;

	// Engine adder.
	int addEngine(qsynthEngine *pEngine);
	// Engine removal.
	void removeEngine(int iTab);

	// Engine tab icon accessor.
	void setOn(int iTab, bool bOn);

signals:

	// Context menu signal.
	void contextMenuRequested(int iTab, const QPoint& pos);

protected:

	// Context menu event.
	void contextMenuEvent(QContextMenuEvent *pContextMenuEvent);
};


#endif  // __qsynthTabBar_h


// end of qsynthTabBar.h
