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
