// qsynthMeter.h
//
/****************************************************************************
   Copyright (C) 2004-2005, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthMeter_h
#define __qsynthMeter_h

#include <qhbox.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qpixmap.h>
#include <qpainter.h>

#include <math.h>

// Color/level indexes.
#define QSYNTHMETER_OVER    0
#define QSYNTHMETER_0DB     1
#define QSYNTHMETER_3DB     2
#define QSYNTHMETER_6DB     3
#define QSYNTHMETER_10DB    4
#define QSYNTHMETER_BACK    5
#define QSYNTHMETER_FORE    6

#define QSYNTHMETER_LEVELS  5
#define QSYNTHMETER_COLORS  7

// Forward declarations.
class qsynthMeter;


//----------------------------------------------------------------------------
// qsynthMeterScale -- Meter bridge scale widget.

class qsynthMeterScale : public QWidget
{
    Q_OBJECT

public:

    // Constructor.
    qsynthMeterScale(qsynthMeter *pMeter);
    // Default destructor.
    ~qsynthMeterScale();

    // IEC scaling helpers.
    int iec_scale (float dB);
    int iec_level (int iIndex);

protected:

    // Specific event handlers.
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

private:

    // Draw IEC scale line and label.
    void drawLineLabel(QPainter& p, int y, const char* pszLabel);

    // Local instance variables.
    qsynthMeter *m_pMeter;
    // Running variables.
    float m_fScale;
    int   m_iLastY;
};


//----------------------------------------------------------------------------
// qsynthMeterValue -- Meter bridge value widget.

class qsynthMeterValue : public QWidget
{
    Q_OBJECT

public:

    // Constructor.
    qsynthMeterValue(qsynthMeter *pMeter);
    // Default destructor.
    ~qsynthMeterValue();

    // Frame value accessors.
    void setValue(float fValue);

protected:

    // Specific event handlers.
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

private:

    // Local instance variables.
    qsynthMeter *m_pMeter;
    // Running variables.
    float m_fValue;
    int   m_iValue;
    float m_fValueDecay;
    int   m_iPeak;
    float m_fPeakDecay;
    int   m_iPeakColor;
};


//----------------------------------------------------------------------------
// qsynthMeter -- Meter bridge slot widget.

class qsynthMeter : public QHBox
{
    Q_OBJECT

public:

    // Constructor.
    qsynthMeter(QWidget *pParent = 0, const char *pszName = 0);
    // Default destructor.
    ~qsynthMeter();

    // Port count accessor.
    int portCount();

    // Value proxy.    
    void setValue(int iPort, float fValue);

    // IEC scale accessors.
    int iec_scale(float dB);
    int iec_level(int iIndex);

    // Slot refreshment.
    void refresh();

    // Common resource accessors.
    QColor& color(int iIndex);

protected:

    // Specific event handlers.
    virtual void resizeEvent(QResizeEvent *);

private:

    // Local instance variables.
    int                m_iPortCount;
    int                m_iScaleCount;
    qsynthMeterValue **m_ppValues;
    qsynthMeterScale **m_ppScales;
    int                m_iLevels[QSYNTHMETER_LEVELS];
    QColor            *m_pColors[QSYNTHMETER_COLORS];
    float              m_fScale;
};

    
#endif  // __qsynthMeter_h

// end of qsynthMeter.h
