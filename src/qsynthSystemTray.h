// qsynthSystemTray.h
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

#ifndef __qsynthSystemTray_h
#define __qsynthSystemTray_h

#include <qlabel.h>
#include <qpixmap.h>


//----------------------------------------------------------------------------
// qsynthSystemTray -- Custom system tray widget.

class qsynthSystemTray : public QLabel
{
    Q_OBJECT

public:

    // Constructor.
    qsynthSystemTray(QWidget *pParent = 0, const char *pszName = 0);
    // Default destructor.
    ~qsynthSystemTray();

    // Set system tray icon overlay.
    void setPixmapOverlay(const QPixmap& pmOverlay);

signals:

    // Clicked signal.
    void clicked();
    // Context menu signal.
    void contextMenuRequested(const QPoint& pos);

protected:

    // Overriden mouse event method.
    void mousePressEvent(QMouseEvent *);
};


#endif  // __qsynthSystemTray_h

// end of qsynthSystemTray.h
