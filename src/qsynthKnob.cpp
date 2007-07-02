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

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qsynthKnob.h"

#include <QTimer>
#include <QToolTip>
#include <QPainter>
#include <QImage>

#include <QMouseEvent>
#include <QWheelEvent>

#include <QRadialGradient>
#include <QColormap>

#include <cmath>


//-------------------------------------------------------------------------
// qsynthKnob - Instance knob widget class.
//

#define QSYNTH_KNOB_MIN      (0.25 * M_PI)
#define QSYNTH_KNOB_MAX      (1.75 * M_PI)
#define QSYNTH_KNOB_RANGE    (QSYNTH_KNOB_MAX - QSYNTH_KNOB_MIN)


// Constructor.
qsynthKnob::qsynthKnob ( QWidget *pParent )
	: QDial(pParent), m_iDefaultValue(-1), m_dialMode(AngularMode),
	m_bMousePressed(false), m_lastDragValue(0.0)
{
}


// Destructor.
qsynthKnob::~qsynthKnob (void)
{
}


void qsynthKnob::paintEvent ( QPaintEvent * )
{
	double angle = QSYNTH_KNOB_MIN // offset
		+ (QSYNTH_KNOB_RANGE *
			(double(value() - minimum()) /
			(double(maximum() - minimum()))));
	int degrees = int(angle * 180.0 / M_PI);

	int side = width() < height() ? width() : height();
	int xcenter = width()  / 2;
	int ycenter = height() / 2;

	int notchWidth   = 1 + side / 400;
	int pointerWidth = 2 + side / 30;

	int scaleShadowWidth = 1 + side / 100;
	int knobBorderWidth  = 4 + side / 50;;

	int ns = notchSize();
	int numTicks = 1 + (maximum() + ns - minimum()) / ns;
	int indent = int(0.15 * side) + 2;
	int knobWidth = side - 2 * indent;
	int shineFocus = knobWidth / 4;
	int shineCenter = knobWidth / 5;
	int shineExtension = shineCenter * 4;
	int shadowShift = shineCenter * 2;
	int meterWidth = side - 2 * scaleShadowWidth;
	
	QColor knobColor(m_knobColor);
	if (!knobColor.isValid())
		knobColor = palette().mid().color();

	QColor borderColor(m_borderColor);
	if (!borderColor.isValid())
		knobBorderWidth = 0;
	//	borderColor = knobColor.light();

	QColor meterColor(m_meterColor);
	if (!isEnabled())
		meterColor = palette().mid().color();
	else if (!m_meterColor.isValid())
		meterColor = palette().highlight().color();
	QColor background = palette().window().color();

	QPainter paint(this);
	paint.setRenderHint(QPainter::Antialiasing, true);

	// The bright metering bit...

	QConicalGradient meterShadow(xcenter, ycenter, -90);
	meterShadow.setColorAt(0, meterColor.dark());
	meterShadow.setColorAt(0.5, meterColor);
	meterShadow.setColorAt(1.0, meterColor.light().light());
	paint.setBrush(meterShadow);
	paint.setPen(Qt::transparent);
	paint.drawPie(xcenter - meterWidth / 2, ycenter - meterWidth / 2,
		meterWidth, meterWidth, (180 + 45) * 16, -(degrees - 45) * 16);

	// Knob projected shadow
	QRadialGradient projectionGradient(
		xcenter + shineCenter, ycenter + shineCenter,
		shineExtension,	xcenter + shadowShift, ycenter + shadowShift);
	projectionGradient.setColorAt(0, QColor(  0, 0, 0, 100));
	projectionGradient.setColorAt(1, QColor(200, 0, 0,  10));
	QBrush shadowBrush(projectionGradient);
	paint.setBrush(shadowBrush);
	paint.drawEllipse(xcenter - shadowShift, ycenter - shadowShift,
		knobWidth, knobWidth);

	// Knob body and face...

	QPen pen;
	pen.setColor(knobColor);
	pen.setWidth(knobBorderWidth);
	paint.setPen(pen);

	QRadialGradient gradient(
		xcenter - shineCenter, ycenter - shineCenter,
		shineExtension,	xcenter - shineFocus, ycenter - shineFocus);
	gradient.setColorAt(0.2, knobColor.light().light());
	gradient.setColorAt(0.5, knobColor);
	gradient.setColorAt(1.0, knobColor.dark(150));
	QBrush knobBrush(gradient);
	paint.setBrush(knobBrush);
	paint.drawEllipse(xcenter - knobWidth / 2, ycenter - knobWidth / 2,
		knobWidth, knobWidth);

	// Tick notches...

	paint.setBrush(Qt::NoBrush);

	if (notchesVisible()) {
		pen.setColor(palette().dark().color());
		pen.setWidth(notchWidth);
		paint.setPen(pen);
		double hyp = double(side - scaleShadowWidth) / 2.0;
		double len = hyp / 4;
		for (int i = 0; i < numTicks; ++i) {
			int div = numTicks;
			if (div > 1) --div;
			bool internal = (i != 0 && i != numTicks - 1);
			double angle = QSYNTH_KNOB_MIN
				+ (QSYNTH_KNOB_MAX - QSYNTH_KNOB_MIN) * i / div;
			double dir = (internal ? -1 : len);
			double sinAngle = sin(angle);
			double cosAngle = cos(angle);
			double x0 = xcenter - (hyp - len) * sinAngle;
			double y0 = ycenter + (hyp - len) * cosAngle;
			double x1 = xcenter - (hyp + dir) * sinAngle;
			double y1 = ycenter + (hyp + dir) * cosAngle;
			paint.drawLine(QLineF(x0, y0, x1, y1));
		}
	}

	// Shadowing...

	// Knob shadow...
	if (knobBorderWidth > 0) {
		QLinearGradient inShadow(xcenter - side / 4, ycenter - side / 4,
			xcenter + side / 4, ycenter + side / 4);
		inShadow.setColorAt(0.0, borderColor.light());
		inShadow.setColorAt(1.0, borderColor.dark());
		paint.setPen(QPen(QBrush(inShadow), knobBorderWidth * 7 / 8));
		paint.drawEllipse(xcenter - side / 2 + indent,
			ycenter - side / 2 + indent,
			side - 2 * indent, side - 2 * indent);
	}

	// Scale shadow...
	QLinearGradient outShadow(xcenter - side / 3, ycenter - side / 3,
		xcenter + side / 3, ycenter + side / 3);
	outShadow.setColorAt(0.0, background.dark().dark());
	outShadow.setColorAt(1.0, background.light().light());
	paint.setPen(QPen(QBrush(outShadow), scaleShadowWidth));
	paint.drawArc(xcenter - side / 2 + scaleShadowWidth / 2,
		ycenter - side / 2 + scaleShadowWidth / 2,
		side - scaleShadowWidth, side - scaleShadowWidth, -45 * 16, 270 * 16);

	// Pointer notch...

	double hyp = double(side) / 2.0;
	double len = hyp - indent - 1;

	double x = xcenter - len * sin(angle);
	double y = ycenter + len * cos(angle);

	QColor pointerColor = m_pointerColor;
	if (!pointerColor.isValid()) 
		pointerColor = palette().dark().color();
	pen.setColor(isEnabled() ? pointerColor.dark(140) : pointerColor);
	pen.setWidth(pointerWidth + 2);
	paint.setPen(pen);
	paint.drawLine(QLineF(xcenter, ycenter, x, y));
	pen.setColor(isEnabled() ? pointerColor.light() : pointerColor.light(140));
	pen.setWidth(pointerWidth);
	paint.setPen(pen);
	paint.drawLine(QLineF(xcenter - 1, ycenter - 1, x - 1, y - 1));
}


void qsynthKnob::setKnobColor ( const QColor& color )
{
	m_knobColor = color;
	update();
}


void qsynthKnob::setMeterColor ( const QColor& color )
{
	m_meterColor = color;
	update();
}


void qsynthKnob::setPointerColor ( const QColor& color )
{
	m_pointerColor = color;
	update();
}


void qsynthKnob::setBorderColor ( const QColor& color )
{
	m_borderColor = color;
	update();
}


void qsynthKnob::setDefaultValue ( int iDefaultValue )
{
	m_iDefaultValue = iDefaultValue;
}


void qsynthKnob::setDialMode ( qsynthKnob::DialMode dialMode )
{
	m_dialMode = dialMode;
}


// Mouse angle determination.
double qsynthKnob::mouseAngle ( const QPoint& pos )
{
	double dx = pos.x() - (width() / 2);
	double dy = (height() / 2) - pos.y();
	return 180.0 * atan2(dx, dy) / M_PI;
}


// Alternate mouse behavior event handlers.
void qsynthKnob::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (m_dialMode == QDialMode) {
		QDial::mousePressEvent(pMouseEvent);
	} else if (pMouseEvent->button() == Qt::LeftButton) {
		m_bMousePressed = true;
		m_posMouse = pMouseEvent->pos();
		m_lastDragValue = double(value());
		emit sliderPressed();
	} else if (pMouseEvent->button() == Qt::MidButton) {
		// Reset to default value...
		if (m_iDefaultValue < minimum() || m_iDefaultValue > maximum())
			m_iDefaultValue = (maximum() + minimum()) / 2;
		setValue(m_iDefaultValue);
	}
}


void qsynthKnob::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	if (m_dialMode == QDialMode) {
		QDial::mouseMoveEvent(pMouseEvent);
		return;
	}

	if (!m_bMousePressed)
		return;

	const QPoint& posMouse = pMouseEvent->pos();
	int xdelta = posMouse.x() - m_posMouse.x();
	int ydelta = posMouse.y() - m_posMouse.y();
	double angleDelta =  mouseAngle(posMouse) - mouseAngle(m_posMouse);

	int iNewValue = value();

	switch (m_dialMode)	{
	case LinearMode:
		iNewValue = int(m_lastDragValue + xdelta - ydelta);
		break;
	case AngularMode:
	default:
		// Forget about the drag origin to be robust on full rotations
		if (angleDelta > +180.0) angleDelta = angleDelta - 360.0;
		if (angleDelta < -180.0) angleDelta = angleDelta + 360.0;
		m_lastDragValue += double(maximum() - minimum()) * angleDelta / 270.0;
		if (m_lastDragValue > double(maximum()))
			m_lastDragValue = double(maximum());
		if (m_lastDragValue < double(minimum()))
			m_lastDragValue = double(minimum());
		m_posMouse = posMouse;
		iNewValue = int(m_lastDragValue + 0.5);
		break;
	}

	setValue(iNewValue);
	update();

	emit sliderMoved(value());
}


void qsynthKnob::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	if (m_dialMode == QDialMode) {
		QDial::mouseReleaseEvent(pMouseEvent);
	} else if (m_bMousePressed) {
		m_bMousePressed = false;
	}
}


void qsynthKnob::wheelEvent ( QWheelEvent *pWheelEvent )
{
	if (m_dialMode == QDialMode) {
		QDial::wheelEvent(pWheelEvent);
	} else {
		int iValue = value();
		if (pWheelEvent->delta() > 0)
			iValue -= pageStep();
		else
			iValue += pageStep();
		if (iValue > maximum())
			iValue = maximum();
		else
		if (iValue < minimum())
			iValue = minimum();
		setValue(iValue);
	}
}


// end of qsynthKnob.cpp
