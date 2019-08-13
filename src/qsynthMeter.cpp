// qsynthMeter.cpp
//
/****************************************************************************
   Copyright (C) 2004-2019, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qsynthMeter.h"

#include <QPainter>
#include <QToolTip>
#include <QLabel>

#include <QHBoxLayout>

#include <math.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#define horizontalAdvance  width
#endif


// Meter level limits (in dB).
#define QSYNTH_METER_MAXDB		(+3.0f)
#define QSYNTH_METER_MINDB		(-70.0f)

// The decay rates (magic goes here :).
// - value decay rate (faster)
#define QSYNTH_METER_DECAY_RATE1	(1.0f - 3E-2f)
// - peak decay rate (slower)
#define QSYNTH_METER_DECAY_RATE2	(1.0f - 3E-6f)

// Number of cycles the peak stays on hold before fall-off.
#define QSYNTH_METER_PEAK_FALLOFF	16


//----------------------------------------------------------------------------
// qsynthMeterScale -- Meter bridge scale widget.

// Constructor.
qsynthMeterScale::qsynthMeterScale( qsynthMeter *pMeter )
	: QWidget(pMeter), m_pMeter(pMeter)
{
	m_iLastY = 0;

	QWidget::setMinimumWidth(16);
//	QWidget::setBackgroundRole(QPalette::Mid);

	const QFont& font = QWidget::font();
	QWidget::setFont(QFont(font.family(), font.pointSize() - 4));
}


// Draw IEC scale line and label; assumes labels drawed from top to bottom.
void qsynthMeterScale::drawLineLabel (
	QPainter *p, int y, const QString& sLabel )
{
	const int iCurrY = QWidget::height() - y;
	const int iWidth = QWidget::width()  - 2;

	const QFontMetrics& fm = p->fontMetrics();
	const int iMidHeight = (fm.height() >> 1);

	if (iCurrY < iMidHeight || iCurrY > m_iLastY + iMidHeight) {
		if (fm.horizontalAdvance(sLabel) < iWidth - 5) {
			p->drawLine(0, iCurrY, 2, iCurrY);
			if (m_pMeter->portCount() > 1)
				p->drawLine(iWidth - 3, iCurrY, iWidth - 1, iCurrY);
		}
		p->drawText(0, iCurrY - iMidHeight, iWidth - 2, fm.height(),
			Qt::AlignHCenter | Qt::AlignVCenter, sLabel);
		m_iLastY = iCurrY + 1;
	}
}


// Paint event handler.
void qsynthMeterScale::paintEvent ( QPaintEvent * )
{
	QPainter p(this);

	m_iLastY = 0;

	const QPalette& pal = QWidget::palette();
	const bool bDark = (pal.base().color().value() < 0x7f);
	const QColor& color = pal.midlight().color();
	p.setPen(bDark ? color.lighter() : color.darker());

	drawLineLabel(&p, m_pMeter->iec_level(qsynthMeter::Color0dB),   "0");
	drawLineLabel(&p, m_pMeter->iec_level(qsynthMeter::Color3dB),   "3");
	drawLineLabel(&p, m_pMeter->iec_level(qsynthMeter::Color6dB),   "6");
	drawLineLabel(&p, m_pMeter->iec_level(qsynthMeter::Color10dB), "10");

	for (float dB = -20.0f; dB > QSYNTH_METER_MINDB; dB -= 10.0f)
		drawLineLabel(&p, m_pMeter->iec_scale(dB), QString::number(-int(dB)));
}


//----------------------------------------------------------------------------
// qsynthMeterValue -- Meter bridge value widget.

// Constructor.
qsynthMeterValue::qsynthMeterValue ( qsynthMeter *pMeter )
	: QFrame(pMeter), m_pMeter(pMeter)
{
	m_fValue = 0.0f;

	m_iValue = 0;
	m_fValueDecay = QSYNTH_METER_DECAY_RATE1;

	m_iPeak = 0;
	m_fPeakDecay = QSYNTH_METER_DECAY_RATE2;
	m_iPeakHold  = 0;
	m_iPeakColor = qsynthMeter::Color6dB;

	QWidget::setMinimumWidth(12);
	QFrame::setBackgroundRole(QPalette::NoRole);

	QFrame::setFrameShape(QFrame::StyledPanel);
	QFrame::setFrameShadow(QFrame::Sunken);
}


// Frame value one-way accessors.
void qsynthMeterValue::setValue ( float fValue )
{
	if (m_fValue < fValue)
		m_fValue = fValue;

	refresh();
}


// Reset peak holder.
void qsynthMeterValue::peakReset (void)
{
	m_iPeak = 0;
}


// Value refreshment.
void qsynthMeterValue::refresh (void)
{
	if (m_fValue < 0.001f && m_iPeak < 1)
		return;

	float dB = QSYNTH_METER_MINDB;
	if (m_fValue > 0.0f) {
		dB = 20.0f * ::log10f(m_fValue);
		m_fValue = 0.0f;
	}

	if (dB < QSYNTH_METER_MINDB)
		dB = QSYNTH_METER_MINDB;
	else if (dB > QSYNTH_METER_MAXDB)
		dB = QSYNTH_METER_MAXDB;

	int iValue = m_pMeter->iec_scale(dB);
	if (iValue < m_iValue) {
		iValue = int(m_fValueDecay * float(m_iValue));
		m_fValueDecay *= m_fValueDecay;
	} else {
		m_fValueDecay = QSYNTH_METER_DECAY_RATE1;
	}

	int iPeak = m_iPeak;
	if (iPeak < iValue) {
		iPeak = iValue;
		m_iPeakHold = 0;
		m_fPeakDecay = QSYNTH_METER_DECAY_RATE2;
		m_iPeakColor = qsynthMeter::Color10dB;
		for (; m_iPeakColor > qsynthMeter::ColorOver
			&& iPeak >= m_pMeter->iec_level(m_iPeakColor); --m_iPeakColor)
			/* empty body loop */;
	} else if (++m_iPeakHold > m_pMeter->peakFalloff()) {
		iPeak = int(m_fPeakDecay * float(iPeak));
		if (iPeak < iValue) {
			iPeak = iValue;
		} else {
			m_fPeakDecay *= m_fPeakDecay;
		}
	}

	if (iValue == m_iValue && iPeak == m_iPeak)
		return;

	m_iValue = iValue;
	m_iPeak  = iPeak;

	update();
}



// Paint event handler.
void qsynthMeterValue::paintEvent ( QPaintEvent * )
{
	QPainter painter(this);

	const int w = QWidget::width();
	const int h = QWidget::height();

	int y;

	if (isEnabled()) {
		painter.fillRect(0, 0, w, h,
			m_pMeter->color(qsynthMeter::ColorBack));
		y = m_pMeter->iec_level(qsynthMeter::Color0dB);
		painter.setPen(m_pMeter->color(qsynthMeter::ColorFore));
		painter.drawLine(0, h - y, w, h - y);
	} else {
		painter.fillRect(0, 0, w, h, QWidget::palette().dark().color());
	}

#ifdef CONFIG_GRADIENT
	painter.drawPixmap(0, h - m_iValue,
		m_pMeter->pixmap(), 0, h - m_iValue, w, m_iValue);
#else
	y = m_iValue;

	int y_over = 0;
	int y_curr = 0;

	for (int i = qsynthMeter::Color10dB;
			i > qsynthMeter::ColorOver && y >= y_over; --i) {
		y_curr = m_pMeter->iec_level(i);
		if (y < y_curr) {
			painter.fillRect(0, h - y, w, y - y_over,
				m_pMeter->color(i));
		} else {
			painter.fillRect(0, h - y_curr, w, y_curr - y_over,
				m_pMeter->color(i));
		}
		y_over = y_curr;
	}

	if (y > y_over) {
		painter.fillRect(0, h - y, w, y - y_over,
			m_pMeter->color(qsynthMeter::ColorOver));
	}
#endif

	painter.setPen(m_pMeter->color(m_iPeakColor));
	painter.drawLine(0, h - m_iPeak, w, h - m_iPeak);
}


// Resize event handler.
void qsynthMeterValue::resizeEvent ( QResizeEvent *pResizeEvent )
{
	m_iPeak = 0;

	QWidget::resizeEvent(pResizeEvent);
//	QWidget::repaint(true);
}


//----------------------------------------------------------------------------
// qsynthMeter -- Meter bridge slot widget.

// Constructor.
qsynthMeter::qsynthMeter ( QWidget *pParent )
	: QWidget(pParent)
{
	m_iPortCount   = 2;	// FIXME: Default port count.
	m_iScaleCount  = m_iPortCount;
	m_ppValues     = nullptr;
	m_ppScales     = nullptr;

	m_fScale = 0.0f;

#ifdef CONFIG_GRADIENT
	m_pPixmap = new QPixmap();
#endif

	m_iPeakFalloff = QSYNTH_METER_PEAK_FALLOFF;

	for (int i = 0; i < LevelCount; i++)
		m_levels[i] = 0;

	m_colors[ColorOver] = QColor(240,  0, 20);
	m_colors[Color0dB]  = QColor(240,160, 20);
	m_colors[Color3dB]  = QColor(220,220, 20);
	m_colors[Color6dB]  = QColor(160,220, 20);
	m_colors[Color10dB] = QColor( 40,160, 40);
	m_colors[ColorBack] = QColor( 20, 40, 20);
	m_colors[ColorFore] = QColor( 80, 80, 80);

	m_pHBoxLayout = new QHBoxLayout();
	m_pHBoxLayout->setMargin(0);
	m_pHBoxLayout->setSpacing(0);
	QWidget::setLayout(m_pHBoxLayout);

	QWidget::setBackgroundRole(QPalette::NoRole);

	if (m_iPortCount > 0) {
		if (m_iPortCount > 1)
			m_iScaleCount--;
		m_ppValues = new qsynthMeterValue *[m_iPortCount];
		m_ppScales = new qsynthMeterScale *[m_iScaleCount];
		for (int iPort = 0; iPort < m_iPortCount; iPort++) {
			m_ppValues[iPort] = new qsynthMeterValue(this);
			m_pHBoxLayout->addWidget(m_ppValues[iPort]);
			if (iPort < m_iScaleCount) {
				m_ppScales[iPort] = new qsynthMeterScale(this);
				m_pHBoxLayout->addWidget(m_ppScales[iPort]);
			}
		}
		int iStripCount = 2 * m_iPortCount;
		if (m_iPortCount > 1)
			--iStripCount;
		QWidget::setMinimumSize(12 * iStripCount, 120);
		QWidget::setMaximumWidth(16 * iStripCount);
	} else {
		QWidget::setMinimumSize(2, 120);
		QWidget::setMaximumWidth(4);
	}

	QWidget::setSizePolicy(
		QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
}


// Default destructor.
qsynthMeter::~qsynthMeter (void)
{
#ifdef CONFIG_GRADIENT
	delete m_pPixmap;
#endif
	for (int iPort = 0; iPort < m_iPortCount; iPort++) {
		delete m_ppValues[iPort];
		if (iPort < m_iScaleCount)
			delete m_ppScales[iPort];
	}

	delete [] m_ppScales;
	delete [] m_ppValues;

	delete m_pHBoxLayout;
}


// Child widget accessors.
int qsynthMeter::iec_scale ( float dB ) const
{
	float fDef = 1.0;

	if (dB < -70.0)
		fDef = 0.0;
	else if (dB < -60.0)
		fDef = (dB + 70.0) * 0.0025;
	else if (dB < -50.0)
		fDef = (dB + 60.0) * 0.005 + 0.025;
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


int qsynthMeter::iec_level ( int iIndex ) const
{
	return m_levels[iIndex];
}


int qsynthMeter::portCount (void) const
{
	return m_iPortCount;
}



// Peak falloff mode setting.
void qsynthMeter::setPeakFalloff ( int iPeakFalloff )
{
	m_iPeakFalloff = iPeakFalloff;
}

int qsynthMeter::peakFalloff (void) const
{
	return m_iPeakFalloff;
}


// Reset peak holder.
void qsynthMeter::peakReset (void)
{
	for (int iPort = 0; iPort < m_iPortCount; iPort++)
		m_ppValues[iPort]->peakReset();
}


#ifdef CONFIG_GRADIENT
// Gradient pixmap accessor.
const QPixmap& qsynthMeter::pixmap (void) const
{
	return *m_pPixmap;
}

void qsynthMeter::updatePixmap (void)
{
	const int w = QWidget::width();
	const int h = QWidget::height();

	QLinearGradient grad(0, 0, 0, h);
	grad.setColorAt(0.2f, color(ColorOver));
	grad.setColorAt(0.3f, color(Color0dB));
	grad.setColorAt(0.4f, color(Color3dB));
	grad.setColorAt(0.6f, color(Color6dB));
	grad.setColorAt(0.8f, color(Color10dB));

	*m_pPixmap = QPixmap(w, h);

	QPainter(m_pPixmap).fillRect(0, 0, w, h, grad);
}
#endif


// Slot refreshment.
void qsynthMeter::refresh (void)
{
	for (int iPort = 0; iPort < m_iPortCount; iPort++)
		m_ppValues[iPort]->refresh();
}


// Resize event handler.
void qsynthMeter::resizeEvent ( QResizeEvent * )
{
	m_fScale = 0.85f * float(QWidget::height());

	m_levels[Color0dB]  = iec_scale(  0.0f);
	m_levels[Color3dB]  = iec_scale( -3.0f);
	m_levels[Color6dB]  = iec_scale( -6.0f);
	m_levels[Color10dB] = iec_scale(-10.0f);

#ifdef CONFIG_GRADIENT
	updatePixmap();
#endif
}


// Meter value proxy.
void qsynthMeter::setValue ( int iPort, float fValue )
{
	m_ppValues[iPort]->setValue(fValue);
}


// Common resource accessor.
const QColor& qsynthMeter::color ( int iIndex ) const
{
	return m_colors[iIndex];
}


// end of qsynthMeter.cpp
