// qsynthTabBar.h
//
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

#ifndef __qsynthTabBar_h
#define __qsynthTabBar_h

#include "qsynthEngine.h"

#include <qtabbar.h>

//-------------------------------------------------------------------------
// qsynthTab - Instance tab structure class.
//

class qsynthTab : public QTab
{
public:

    // Constructor.
    qsynthTab(qsynthEngine *pEngine);
    // Destructor.
    ~qsynthTab();
    
    // Engine property accessor.
    qsynthEngine *engine();
    
    // Icon toggler.
    void setOn(bool bOn);

private:

    // Tab instance engine reference.
    qsynthEngine *m_pEngine;
};


//-------------------------------------------------------------------------
// qsynthTabBar - Instance tab widget class.
//


class qsynthTabBar : public QTabBar
{
    Q_OBJECT
    
public:

    // Constructor.
    qsynthTabBar(QWidget *pParent, const char *pszName = 0);
    // Destructor.
    ~qsynthTabBar();

    // Current engine accessor.
    qsynthEngine *currentEngine();
    
signals:

    // Context menu signal.
    void contextMenuRequested(qsynthTab *pTab, const QPoint& pos);

protected:

    // Context menu event.
    void contextMenuEvent(QContextMenuEvent *pContextMenuEvent);
};


#endif  // __qsynthTabBar_h


// end of qsynthTabBar.h
