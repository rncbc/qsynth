// qsynthKnob.h
//
/****************************************************************************
   Copyright (C) 2005-2013, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qsynthKnob_h
#define __qsynthKnob_h

#include <QDial>


//-------------------------------------------------------------------------
// qsynthKnob - A better QDial for everybody

class qsynthKnob : public QDial
{
	Q_OBJECT
	Q_PROPERTY(int defaultValue READ getDefaultValue WRITE setDefaultValue)
	Q_PROPERTY(DialMode dialMode READ getDialMode WRITE setDialMode)
	Q_ENUMS(DialMode)

public:

	// Constructor.
	qsynthKnob(QWidget *pParent = 0);
	// Destructor.
	~qsynthKnob();

	int getDefaultValue() const { return m_iDefaultValue; }

	// Knob dial mode behavior:
	// DefaultMode - default (old) QDial behavior.
	// AngularMode - angularly relative to widget center.
	// LinearMode  - proportionally to distance in one ortogonal axis.

	enum DialMode { DefaultMode, AngularMode, LinearMode };

	DialMode getDialMode() const { return m_dialMode; }

public slots:

	// Set default (mid) value.
	void setDefaultValue(int iDefaultValue);

	// Set knob dial mode behavior.
	void setDialMode(DialMode dialMode);

protected:

	// Mouse angle determination.
	float mouseAngle(const QPoint& pos);

	// Alternate mouse behavior event handlers.
	virtual void mousePressEvent(QMouseEvent *pMouseEvent);
	virtual void mouseMoveEvent(QMouseEvent *pMouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent *pMouseEvent);
	virtual void wheelEvent(QWheelEvent *pWheelEvent);

private:

	// Default (mid) value.
	int m_iDefaultValue;

	// Knob dial mode behavior.
	DialMode m_dialMode;

	// Alternate mouse behavior tracking.
	bool   m_bMousePressed;
	QPoint m_posMouse;

	// Just for more precission on the movement
	float m_fLastDragValue;
};


#endif  // __qsynthKnob_h

// end of qsynthKnob.h
