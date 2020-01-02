// qsynthTabBar.cpp
//
/****************************************************************************
   Copyright (C) 2003-2020, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qsynthTabBar.h"

#include "qsynthEngine.h"

#include <QIcon>

#include <QContextMenuEvent>


// Common icon set.
static int    g_iIconRefCount = 0;
static QIcon *g_pIconLedOn    = nullptr;
static QIcon *g_pIconLedOff   = nullptr;


//-------------------------------------------------------------------------
// qsynthTabBar - Instance tab widget class.
//

// Constructor.
qsynthTabBar::qsynthTabBar ( QWidget *pParent ) : QTabBar(pParent)
{
	QTabBar::setShape(QTabBar::RoundedSouth);

	if (++g_iIconRefCount == 1) {
		g_pIconLedOn  = new QIcon(":/images/ledon1.png");
		g_pIconLedOff = new QIcon(":/images/ledoff1.png");
	}
}


// Destructor.
qsynthTabBar::~qsynthTabBar (void)
{
	if (--g_iIconRefCount == 0) {
		delete g_pIconLedOn;
		delete g_pIconLedOff;
		g_pIconLedOn  = nullptr;
		g_pIconLedOff = nullptr;
	}
}


// Engine accessor.
qsynthEngine *qsynthTabBar::engine ( int iTab ) const
{
	return static_cast<qsynthEngine *> (QTabBar::tabData(iTab).value<void *>());
}


// Current engine accessor.
qsynthEngine *qsynthTabBar::currentEngine (void) const
{
	return engine(QTabBar::currentIndex());
}


// Engine adder.
int qsynthTabBar::addEngine ( qsynthEngine *pEngine )
{
	int iTab = QTabBar::addTab(*g_pIconLedOff, pEngine->name());
	if (iTab >= 0) {
		QTabBar::setTabData(iTab,
			QVariant::fromValue(static_cast<void *> (pEngine)));
	}
	return iTab;
}


// Engine removal.
void qsynthTabBar::removeEngine ( int iTab )
{
	qsynthEngine *pEngine = engine(iTab);
	if (pEngine)
		delete pEngine;

	QTabBar::removeTab(iTab);
}


// Engine tab icon accessor.
void qsynthTabBar::setOn ( int iTab, bool bOn )
{
	QTabBar::setTabIcon(iTab, bOn ? *g_pIconLedOn : *g_pIconLedOff);
}


// Context menu event.
void qsynthTabBar::contextMenuEvent ( QContextMenuEvent *pContextMenuEvent )
{
	int iTab = QTabBar::tabAt(pContextMenuEvent->pos());
	if (iTab >= 0)
		QTabBar::setCurrentIndex(iTab);

	// Emit context menu signal.
	emit contextMenuRequested(iTab, pContextMenuEvent->globalPos());
}


// end of qsynthTabBar.cpp
