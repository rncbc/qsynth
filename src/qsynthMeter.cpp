// qsynthMeter.cpp
//
/****************************************************************************
   Copyright (C) 2004, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qsynthMeter.h"

#include <qpopupmenu.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <unistd.h>

// Meter level limits (in dB).
#define QSYNTHMETER_MAXDB        (+3.0)
#define QSYNTHMETER_MINDB        (-70.0)

// The peak decay rate (magic goes here :).
#define QSYNTHMETER_DECAY_RATE   (0.998)


//----------------------------------------------------------------------------
// qsynthMeterScale -- Meter bridge scale widget.

// Constructor.
qsynthMeterScale::qsynthMeterScale( qsynthMeter *pMeter )
    : QWidget(pMeter)
{
    m_pMeter = pMeter;
    m_iLastY = 0;

    QWidget::setBackgroundMode(Qt::PaletteMid);
}

// Default destructor.
qsynthMeterScale::~qsynthMeterScale (void)
{
}


// Draw IEC scale line and label; assumes labels drawed from top to bottom.
void qsynthMeterScale::drawLineLabel ( QPainter& p, int y, const char* pszLabel )
{
    int iCurrY = QWidget::height() - y;
    p.drawLine(1, iCurrY, QWidget::width() - 2, iCurrY);
    if (iCurrY > m_iLastY + p.fontMetrics().ascent()) {
        if (m_pMeter->portCount() > 1)
            p.drawText(1, iCurrY - p.fontMetrics().ascent() - 1, QWidget::width() - 2, p.fontMetrics().height(), Qt::AlignHCenter | Qt::AlignVCenter, pszLabel);
        else
            p.drawText(2, iCurrY - 1, pszLabel);
    }
    m_iLastY = iCurrY;
}


// Paint event handler.
void qsynthMeterScale::paintEvent ( QPaintEvent * )
{
    QPainter p(this);

    p.setFont(QFont("Helvetica", 6));
    p.setPen(m_pMeter->color(QSYNTHMETER_FORE));

    m_iLastY = 0;

    drawLineLabel(p, m_pMeter->iec_level(QSYNTHMETER_0DB),   "0");
    drawLineLabel(p, m_pMeter->iec_level(QSYNTHMETER_3DB),   "3");
    drawLineLabel(p, m_pMeter->iec_level(QSYNTHMETER_6DB),   "6");
    drawLineLabel(p, m_pMeter->iec_level(QSYNTHMETER_10DB), "10");

    for (float dB = -20.0; dB > QSYNTHMETER_MINDB; dB -= 10.0)
        drawLineLabel(p, m_pMeter->iec_scale(dB), QString::number((int) -dB));
}


// Resize event handler.
void qsynthMeterScale::resizeEvent ( QResizeEvent * )
{
    QWidget::repaint(true);
}


//----------------------------------------------------------------------------
// qsynthMeterValue -- Meter bridge value widget.

// Constructor.
qsynthMeterValue::qsynthMeterValue( qsynthMeter *pMeter )
    : QWidget(pMeter)
{
    m_pMeter     = pMeter;
    m_fValue     = 0.0;
    m_iPeak      = 0;
    m_iPeakColor = QSYNTHMETER_6DB;
    m_fDecayRate = QSYNTHMETER_DECAY_RATE;

    QWidget::setBackgroundMode(Qt::NoBackground);
}

// Default destructor.
qsynthMeterValue::~qsynthMeterValue (void)
{
}


// Frame value one-way accessors.
void qsynthMeterValue::setValue ( float fValue )
{
    m_fValue = fValue;
}


// Paint event handler.
void qsynthMeterValue::paintEvent ( QPaintEvent * )
{
    int iWidth  = QWidget::width();
    int iHeight = QWidget::height();

    QPixmap pm(iWidth, iHeight);
    QPainter p(&pm);

    pm.fill(isEnabled() ? m_pMeter->color(QSYNTHMETER_BACK) : Qt::gray);

    p.setViewport(0, 0, iWidth, iHeight);
    p.setWindow(0, 0, iWidth, iHeight);

    float dB = QSYNTHMETER_MINDB;
    if (m_fValue > 0.0)
        dB = 20.0 * ::log(m_fValue) / M_LN10;

    if (dB < QSYNTHMETER_MINDB)
        dB = QSYNTHMETER_MINDB;
    else if (dB > QSYNTHMETER_MAXDB)
        dB = QSYNTHMETER_MAXDB;

    int y_over = 0;
    int y_curr = 0;
    int y  = m_pMeter->iec_scale(dB);
    int iLevel;
    for (iLevel = QSYNTHMETER_10DB; iLevel > QSYNTHMETER_OVER && y >= y_over; iLevel--) {
        y_curr = m_pMeter->iec_level(iLevel);
        if (y < y_curr)
            p.fillRect(0, iHeight - y, iWidth, y - y_over, m_pMeter->color(iLevel));
        else
            p.fillRect(0, iHeight - y_curr, iWidth, y_curr - y_over, m_pMeter->color(iLevel));
        y_over = y_curr;
    }

    if (y > y_over)
        p.fillRect(0, iHeight - y, iWidth, y - y_over, m_pMeter->color(QSYNTHMETER_OVER));

    if (y > m_iPeak) {
        m_iPeak = y;
        m_iPeakColor = iLevel;
        m_fDecayRate = QSYNTHMETER_DECAY_RATE;
    } else {
        m_iPeak = (int) ((float) m_iPeak * m_fDecayRate);
        if (m_iPeak < m_pMeter->iec_level(QSYNTHMETER_10DB))
            m_iPeakColor = QSYNTHMETER_6DB;
        m_fDecayRate *= m_fDecayRate;
    }

    p.setPen(m_pMeter->color(m_iPeakColor));
    p.drawLine(0, iHeight - m_iPeak, iWidth, iHeight - m_iPeak);

    bitBlt(this, 0, 0, &pm);
}


// Resize event handler.
void qsynthMeterValue::resizeEvent ( QResizeEvent * )
{
    m_iPeak = 0;

    QWidget::repaint(true);
}


//----------------------------------------------------------------------------
// qsynthMeter -- Meter bridge slot widget.

// Constructor.
qsynthMeter::qsynthMeter ( QWidget *pParent, const char *pszName )
    : QHBox(pParent, pszName)
{
    m_iPortCount  = 2;	// FIXME: Default port count.
    m_iScaleCount = m_iPortCount;
    m_ppValues    = NULL;
    m_ppScales    = NULL;
    m_fScale      = 0.0;

    for (int i = 0; i < QSYNTHMETER_LEVELS; i++)
        m_iLevels[i] = 0;

    m_pColors[QSYNTHMETER_OVER] = new QColor(240,  0, 20);
    m_pColors[QSYNTHMETER_0DB]  = new QColor(240,160, 20);
    m_pColors[QSYNTHMETER_3DB]  = new QColor(220,220, 20);
    m_pColors[QSYNTHMETER_6DB]  = new QColor(160,220, 20);
    m_pColors[QSYNTHMETER_10DB] = new QColor( 40,160, 40);
    m_pColors[QSYNTHMETER_BACK] = new QColor( 20, 40, 20);
    m_pColors[QSYNTHMETER_FORE] = new QColor( 80, 80, 80);

    if (m_iPortCount > 0) {
        if (m_iPortCount > 1)
            m_iScaleCount--;
        m_ppValues = new qsynthMeterValue *[m_iPortCount];
        m_ppScales = new qsynthMeterScale *[m_iScaleCount];
        for (int iPort = 0; iPort < m_iPortCount; iPort++) {
            m_ppValues[iPort] = new qsynthMeterValue(this);
            if (iPort < m_iScaleCount)
                m_ppScales[iPort] = new qsynthMeterScale(this);
        }
        QWidget::setBackgroundMode(Qt::NoBackground);
        int iStripCount = 2 * m_iPortCount;
        if (m_iPortCount > 1)
            iStripCount--;
        QWidget::setMinimumSize(QSize(13 * iStripCount, 160));
        QWidget::setMaximumWidth(16 * iStripCount);
    } else {
        QWidget::setMinimumSize(QSize(2, 160));
        QWidget::setMaximumWidth(4);
    }
    
    QWidget::setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));

    QHBox::setFrameShape(QFrame::StyledPanel);
    QHBox::setFrameShadow(QFrame::Sunken);
}


// Default destructor.
qsynthMeter::~qsynthMeter (void)
{
    for (int iPort = 0; iPort < m_iPortCount; iPort++) {
        delete m_ppValues[iPort];
        if (iPort < m_iScaleCount)
            delete m_ppScales[iPort];
    }

    delete [] m_ppScales;
    delete [] m_ppValues;

    for (int i = 0; i < QSYNTHMETER_COLORS; i++)
        delete m_pColors[i];
}


// Child widget accessors.
int qsynthMeter::iec_scale ( float dB )
{
    float fDef = 1.0;

    if (dB < -70.0)
        fDef = 0.0;
    else if (dB < -60.0)
        fDef = (dB + 70.0) * 0.0025;
    else if (dB < -50.0)
        fDef = (dB + 60.0) * 0.005 + 0.05;
    else if (dB < -40.0)
        fDef = (dB + 50.0) * 0.0075 + 0.075;
    else if (dB < -30.0)
        fDef = (dB + 40.0) * 0.015 + 0.15;
    else if (dB < -20.0)
        fDef = (dB + 30.0) * 0.02 + 0.3;
    else /* if (dB < 0.0) */
        fDef = (dB + 20.0) * 0.025 + 0.5;

    return (int) (fDef * m_fScale);
}


int qsynthMeter::iec_level ( int iIndex )
{
    return m_iLevels[iIndex];
}


int qsynthMeter::portCount (void)
{
    return m_iPortCount;
}


// Slot refreshment.
void qsynthMeter::refresh (void)
{
    for (int iPort = 0; iPort < m_iPortCount; iPort++)
        m_ppValues[iPort]->update();
}


// Resize event handler.
void qsynthMeter::resizeEvent ( QResizeEvent * )
{
    m_fScale = ((float) QWidget::height() * 40.0) / (40.0 + QSYNTHMETER_MAXDB);

    m_iLevels[QSYNTHMETER_0DB]  = iec_scale(  0.0);
    m_iLevels[QSYNTHMETER_3DB]  = iec_scale( -3.0);
    m_iLevels[QSYNTHMETER_6DB]  = iec_scale( -6.0);
    m_iLevels[QSYNTHMETER_10DB] = iec_scale(-10.0);
}


// Meter value proxy.
void qsynthMeter::setValue ( int iPort, float fValue )
{
    m_ppValues[iPort]->setValue(fValue);
}


// Common resource accessor.
QColor& qsynthMeter::color ( int iIndex )
{
    return *m_pColors[iIndex];
}



// end of qsynthMeter.cpp
