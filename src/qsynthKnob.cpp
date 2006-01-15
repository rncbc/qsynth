// qsynthKnob.cpp
//
/****************************************************************************
   Copyright (C) 2005-2006, rncbc aka Rui Nuno Capela. All rights reserved.

   This widget is based on a design by Thorsten Wilms, 
   implemented by Chris Cannam in Rosegarden,
   adapted for QSynth by Pedro Lopez-Cabanillas

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

#include <cmath>
#include <qtimer.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>

#include "qsynthKnob.h"


struct qsynthKnobCacheIndex
{
	// Constructor.
	qsynthKnobCacheIndex(int s = 0, int kc = 0, int mc = 0,
		int a = 0, int n = 0, int c = 0) :
			size(s), knobColor(kc), meterColor(mc),
			angle(a), numTicks(n), centered(c) {}

	bool operator<(const qsynthKnobCacheIndex &i) const {
		// woo!
		if (size < i.size) return true;
		else if (size > i.size) return false;
		else if (knobColor < i.knobColor) return true;
		else if (knobColor > i.knobColor) return false;
		else if (meterColor < i.meterColor) return true;
		else if (meterColor > i.meterColor) return false;
		else if (angle < i.angle) return true;
		else if (angle > i.angle) return false;
		else if (numTicks < i.numTicks) return true;
		else if (numTicks > i.numTicks) return false;
		else if (centered == i.centered) return false;
		else if (!centered) return true;
		return false;
	}

	int          size;
	unsigned int knobColor;
	unsigned int meterColor;
	int          angle;
	int          numTicks;
	bool         centered;
};

typedef QMap<qsynthKnobCacheIndex, QPixmap> qsynthKnobPixmapCache;


//-------------------------------------------------------------------------
// qsynthKnob - Instance knob widget class.
//

#define QSYNTHKNOB_MIN      (0.25 * M_PI)
#define QSYNTHKNOB_MAX      (1.75 * M_PI)
#define QSYNTHKNOB_RANGE    (QSYNTHKNOB_MAX - QSYNTHKNOB_MIN)


// Constructor.
qsynthKnob::qsynthKnob ( QWidget *pParent, const char *pszName )
	: QDial(pParent, pszName),
	m_knobColor(Qt::black), m_meterColor(Qt::white),
	m_bMouseDial(false), m_bMousePressed(false),
	m_iDefaultValue(-1)
{
}


// Destructor.
qsynthKnob::~qsynthKnob (void)
{
}


void qsynthKnob::repaintScreen ( const QRect */*pRect*/ )
{
	static qsynthKnobPixmapCache s_pixmaps;

	QPainter paint;

	float angle = QSYNTHKNOB_MIN // offset
		+ (QSYNTHKNOB_RANGE *
			(float(value() - minValue()) /
			(float(maxValue() - minValue()))));
	int degrees = int(angle * 180.0 / M_PI);

	int ns = notchSize();
	int numTicks = 1 + (maxValue() + ns - minValue()) / ns;
	
	QColor knobColor(m_knobColor);
	if (knobColor == Qt::black)
		knobColor = colorGroup().mid();

	QColor meterColor(m_meterColor);
	if (!isEnabled())
		meterColor = colorGroup().mid();
	else if (m_meterColor == Qt::white)
		meterColor = colorGroup().highlight();

	int m_size = width() < height() ? width() : height();
	qsynthKnobCacheIndex index(m_size, knobColor.pixel(), meterColor.pixel(),
		degrees, numTicks, false);

	if (s_pixmaps.find(index) != s_pixmaps.end()) {
		paint.begin(this);
		paint.drawPixmap(0, 0, s_pixmaps[index]);
		paint.end();
		return;
	}

	int scale = 4;
	int width = m_size * scale;
	QPixmap map(width, width);
	map.fill(paletteBackgroundColor());
	paint.begin(&map);

	QPen pen;
	QColor c;

	// Knob body and face...

	c = knobColor;
	pen.setColor(knobColor);
	pen.setWidth(scale);
	
	paint.setPen(pen);
	paint.setBrush(c);

	int indent = (int)(width * 0.15 + 1);

	paint.drawEllipse(indent, indent, width-2*indent, width-2*indent);

	pen.setWidth(2 * scale);
	int pos = indent + (width-2*indent) / 20;
	int darkWidth = (width-2*indent) * 3 / 4;
	while (darkWidth) {
		c = c.light(101);
		pen.setColor(c);
		paint.setPen(pen);
		paint.drawEllipse(pos, pos, darkWidth, darkWidth);
		if (!--darkWidth) break;
		paint.drawEllipse(pos, pos, darkWidth, darkWidth);
		if (!--darkWidth) break;
		paint.drawEllipse(pos, pos, darkWidth, darkWidth);
		++pos; --darkWidth;
	}

	// The bright metering bit...

	c = meterColor;
	pen.setColor(c);
	pen.setWidth(indent);
	paint.setPen(pen);

	paint.drawArc(indent/2, indent/2,
		width-indent, width-indent, (180 + 45) * 16, -(degrees - 45) * 16);

	// Tick notches...

	paint.setBrush(QBrush::NoBrush);

	if ( notchesVisible() ) {
		pen.setColor(colorGroup().dark());
		pen.setWidth(scale);
		paint.setPen(pen);
		for (int i = 0; i < numTicks; ++i) {
			int div = numTicks;
			if (div > 1) --div;
			drawTick(paint, QSYNTHKNOB_MIN +
				(QSYNTHKNOB_MAX - QSYNTHKNOB_MIN) * i / div,
				width, i != 0 && i != numTicks-1 );
		}
	}

	// Shadowing...

	pen.setWidth(scale);
	paint.setPen(pen);

	// Knob shadow...

	int shadowAngle = -720;
	c = knobColor.dark();
	for (int arc = 120; arc < 2880; arc += 240) {
		pen.setColor(c);
		paint.setPen(pen);
		paint.drawArc(indent, indent,
			width-2*indent, width-2*indent, shadowAngle + arc, 240);
		paint.drawArc(indent, indent,
			width-2*indent, width-2*indent, shadowAngle - arc, 240);
		c = c.light(110);
	}

	// Scale shadow...

	shadowAngle = 2160;
	c = colorGroup().dark();
	for (int arc = 120; arc < 2880; arc += 240) {
		pen.setColor(c);
		paint.setPen(pen);
		paint.drawArc(scale/2, scale/2,
			width-scale, width-scale, shadowAngle + arc, 240);
		paint.drawArc(scale/2, scale/2,
			width-scale, width-scale, shadowAngle - arc, 240);
		c = c.light(108);
	}

	// Undraw the bottom part...

	pen.setColor(paletteBackgroundColor());
	paint.setPen(pen);
	paint.drawArc(scale/2, scale/2,
		width-scale, width-scale, -45 * 16, -90 * 16);

	// Pointer notch...

	float hyp = float(width) / 2.0;
	float len = hyp - indent;
	--len;

	float x0 = hyp;
	float y0 = hyp;

	float x = hyp - len * sin(angle);
	float y = hyp + len * cos(angle);

	c = colorGroup().dark();
	pen.setColor(isEnabled() ? c.dark(130) : c);
	pen.setWidth(scale * 2);
	paint.setPen(pen);
	paint.drawLine(int(x0), int(y0), int(x), int(y));
	paint.end();

	// Image rendering...

	QImage img = map.convertToImage().smoothScale(m_size, m_size);
	s_pixmaps[index] = QPixmap(img);
	paint.begin(this);
	paint.drawPixmap(0, 0, s_pixmaps[index]);
	paint.end();
}


void qsynthKnob::drawTick ( QPainter& paint,
	float angle, int size, bool internal )
{
	float hyp = float(size) / 2.0;
	float x0 = hyp - (hyp - 1) * sin(angle);
	float y0 = hyp + (hyp - 1) * cos(angle);

	if (internal) {

		float len = hyp / 4;
		float x1 = hyp - (hyp - len) * sin(angle);
		float y1 = hyp + (hyp - len) * cos(angle);
		
		paint.drawLine(int(x0), int(y0), int(x1), int(y1));

	} else {

		float len = hyp / 4;
		float x1 = hyp - (hyp + len) * sin(angle);
		float y1 = hyp + (hyp + len) * cos(angle);

		paint.drawLine(int(x0), int(y0), int(x1), int(y1));
	}
}


void qsynthKnob::setKnobColor ( const QColor& color )
{
	m_knobColor = color;
	repaint();
}


void qsynthKnob::setMeterColor ( const QColor& color )
{
	m_meterColor = color;
	repaint();
}


void qsynthKnob::setMouseDial ( bool bMouseDial )
{
	m_bMouseDial = bMouseDial;
}


void qsynthKnob::setDefaultValue ( int iDefaultValue )
{
	m_iDefaultValue = iDefaultValue;
}


// Alternate mouse behavior event handlers.
void qsynthKnob::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (m_bMouseDial) {
		QDial::mousePressEvent(pMouseEvent);
	} else if (pMouseEvent->button() == Qt::LeftButton) {
		m_bMousePressed = true;
		m_posMouse = pMouseEvent->pos();
		emit dialPressed();
	} else if (pMouseEvent->button() == Qt::MidButton) {
		// Reset to default value...
		if (m_iDefaultValue < minValue() || m_iDefaultValue > maxValue())
			m_iDefaultValue = (maxValue() + minValue()) / 2;
		setValue(m_iDefaultValue);
	}
}


void qsynthKnob::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	if (m_bMouseDial) {
		QDial::mouseMoveEvent(pMouseEvent);
	} else if (m_bMousePressed) {
		// Dragging by x or y axis when clicked modifies value.
		const QPoint& posMouse = pMouseEvent->pos();
		int iValue = value()
			+ ((posMouse.x() - m_posMouse.x())
			+ (m_posMouse.y() - posMouse.y())) * lineStep();
		if (iValue > maxValue())
			iValue = maxValue();
		else
		if (iValue < minValue())
			iValue = minValue();
		m_posMouse = posMouse;
		setValue(iValue);
		emit dialMoved(value());
	}
}

void qsynthKnob::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	if (m_bMouseDial) {
		QDial::mouseReleaseEvent(pMouseEvent);
	} else if (m_bMousePressed) {
		m_bMousePressed = false;
	}
}


void qsynthKnob::wheelEvent ( QWheelEvent *pWheelEvent )
{
	if (m_bMouseDial) {
		QDial::wheelEvent(pWheelEvent);
	} else {
		int iValue = value();
		if (pWheelEvent->delta() > 0)
			iValue -= pageStep();
		else
			iValue += pageStep();
		if (iValue > maxValue())
			iValue = maxValue();
		else
		if (iValue < minValue())
			iValue = minValue();
		setValue(iValue);
	}
}


void qsynthKnob::valueChange (void)
{
	repaintScreen();
	emit valueChanged(value());
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
}


// end of qsynthKnob.cpp
