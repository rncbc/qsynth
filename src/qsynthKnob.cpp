// qsynthKnob.cpp
//
/****************************************************************************
   Copyright (C) 2005-2020, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qsynthAbout.h"
#include "qsynthKnob.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <cmath>


//-------------------------------------------------------------------------
// qsynthKnob - Instance knob widget class.
//

// Constructor.
qsynthKnob::qsynthKnob ( QWidget *pParent )
	: QDial(pParent), m_iDefaultValue(-1), m_dialMode(DefaultMode),
	m_bMousePressed(false), m_fLastDragValue(0.0f)
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
float qsynthKnob::mouseAngle ( const QPoint& pos )
{
	float dx = pos.x() - (width() / 2);
	float dy = (height() / 2) - pos.y();
	return 180.0f * atan2f(dx, dy) / float(M_PI);
}


// Alternate mouse behavior event handlers.
void qsynthKnob::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (pMouseEvent->button() == Qt::MidButton) {
		// Reset to default value...
		if (m_iDefaultValue < minimum() || m_iDefaultValue > maximum())
			m_iDefaultValue = (maximum() + minimum()) / 2;
		setValue(m_iDefaultValue);
	} else if (m_dialMode == DefaultMode) {
		QDial::mousePressEvent(pMouseEvent);
	} else if (pMouseEvent->button() == Qt::LeftButton) {
		m_bMousePressed = true;
		m_posMouse = pMouseEvent->pos();
		m_fLastDragValue = float(value());
		emit sliderPressed();
	}
}


void qsynthKnob::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	if (m_dialMode == DefaultMode) {
		QDial::mouseMoveEvent(pMouseEvent);
		return;
	}

	if (!m_bMousePressed)
		return;

	const QPoint& pos = pMouseEvent->pos();
	int xdelta = pos.x() - m_posMouse.x();
	int ydelta = pos.y() - m_posMouse.y();
	float fAngleDelta =  mouseAngle(pos) - mouseAngle(m_posMouse);

	int iNewValue = value();

	switch (m_dialMode)	{
	case LinearMode:
		iNewValue = int(m_fLastDragValue) + xdelta - ydelta;
		break;
	case AngularMode:
	default:
		// Forget about the drag origin to be robust on full rotations
		if (fAngleDelta > +180.0f) fAngleDelta -= 360.0f;
		else
		if (fAngleDelta < -180.0f) fAngleDelta += 360.0f;
		m_fLastDragValue += float(maximum() - minimum()) * fAngleDelta / 270.0f;
		if (m_fLastDragValue > float(maximum()))
			m_fLastDragValue = float(maximum());
		else
		if (m_fLastDragValue < float(minimum()))
			m_fLastDragValue = float(minimum());
		m_posMouse = pos;
		iNewValue = int(m_fLastDragValue + 0.5f);
		break;
	}

	setValue(iNewValue);
	update();

	emit sliderMoved(value());
}


void qsynthKnob::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	if (m_dialMode == DefaultMode && pMouseEvent->button() != Qt::MidButton) {
		QDial::mouseReleaseEvent(pMouseEvent);
	} else if (m_bMousePressed) {
		m_bMousePressed = false;
	}
}


void qsynthKnob::wheelEvent ( QWheelEvent *pWheelEvent )
{
	if (m_dialMode == DefaultMode) {
		QDial::wheelEvent(pWheelEvent);
	} else {
		int iValue = value();
		if (pWheelEvent->angleDelta().y() > 0)
			iValue += pageStep();
		else
			iValue -= pageStep();
		if (iValue > maximum())
			iValue = maximum();
		else
		if (iValue < minimum())
			iValue = minimum();
		setValue(iValue);
	}
}


// end of qsynthKnob.cpp
