// qsynthTabBar.cpp
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

#include "qsynthTabBar.h"

#include <qiconset.h>

// Common icon set.
static int       g_iIconRefCount = 0;
static QIconSet *g_pIconLedOn    = NULL;
static QIconSet *g_pIconLedOff   = NULL;


//-------------------------------------------------------------------------
// qsynthTab - Instance tab structure class.
//

// Constructor.
qsynthTab::qsynthTab ( qsynthEngine *pEngine )
    : QTab(*g_pIconLedOff, pEngine->name())
{
    m_pEngine = pEngine;
}


// Destructor.
qsynthTab::~qsynthTab (void)
{
    if (m_pEngine)
        delete m_pEngine;
    m_pEngine = NULL;
}


// Engine property accessor.
qsynthEngine *qsynthTab::engine (void) const
{
    return m_pEngine;
}


// Icon toggler.
void qsynthTab::setOn ( bool bOn )
{
    QTab::setIconSet(bOn ? *g_pIconLedOn : *g_pIconLedOff);
}


//-------------------------------------------------------------------------
// qsynthTabBar - Instance tab widget class.
//

// Constructor.
qsynthTabBar::qsynthTabBar ( QWidget *pParent, const char *pszName )
    : QTabBar(pParent, pszName)
{
    QTabBar::setShape(QTabBar::RoundedBelow);

    if (g_iIconRefCount == 0) {
        g_pIconLedOn  = new QIconSet(QPixmap::fromMimeSource("ledon1.png"));
        g_pIconLedOff = new QIconSet(QPixmap::fromMimeSource("ledoff1.png"));
    }
    g_iIconRefCount++;
}


// Destructor.
qsynthTabBar::~qsynthTabBar (void)
{
    if (--g_iIconRefCount == 0) {
        delete g_pIconLedOn;
        delete g_pIconLedOff;
        g_pIconLedOn  = NULL;
        g_pIconLedOff = NULL;
    }
}


// Current engine accessor.
qsynthEngine *qsynthTabBar::currentEngine (void)
{
    qsynthTab *pTab = (qsynthTab *) QTabBar::tab(QTabBar::currentTab());
    if (pTab == NULL)
        return NULL;
    return pTab->engine();
}


// Context menu event.
void qsynthTabBar::contextMenuEvent ( QContextMenuEvent *pContextMenuEvent )
{
    qsynthTab *pTab = (qsynthTab *) QTabBar::selectTab(pContextMenuEvent->pos());
    if (pTab)
        QTabBar::setCurrentTab(pTab);

    // Emit context menu signal.
    emit contextMenuRequested(pTab, pContextMenuEvent->globalPos());
}


// end of qsynthTabBar.cpp
