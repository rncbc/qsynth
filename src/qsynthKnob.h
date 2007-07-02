// qsynthKnob.h
//
/****************************************************************************
   Copyright (C) 2005-2007, rncbc aka Rui Nuno Capela. All rights reserved.

   This widget is based on a design by Thorsten Wilms, 
   implemented by Chris Cannam in Rosegarden,
   adapted for QSynth by Pedro Lopez-Cabanillas,
   improved for Qt4 by David García Garzon.

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
#include <QMap>


//-------------------------------------------------------------------------
// qsynthKnob - A better QDial for QSynth.

class qsynthKnob : public QDial
{
	Q_OBJECT
	Q_PROPERTY( QColor knobColor READ getKnobColor WRITE setKnobColor )
	Q_PROPERTY( QColor meterColor READ getMeterColor WRITE setMeterColor )
	Q_PROPERTY( QColor pointerColor READ getPointerColor WRITE setPointerColor )
	Q_PROPERTY( QColor borderColor READ getBorderColor WRITE setBorderColor )
	Q_PROPERTY( int defaultValue READ getDefaultValue WRITE setDefaultValue )
	Q_PROPERTY( DialMode dialMode READ getDialMode WRITE setDialMode )
	Q_ENUMS(DialMode)

public:

	// Constructor.
	qsynthKnob(QWidget *pParent = 0);
	// Destructor.
	~qsynthKnob();

	const QColor& getKnobColor()    const { return m_knobColor;  }
	const QColor& getMeterColor()   const { return m_meterColor; }
	const QColor& getPointerColor() const { return m_pointerColor; }
	const QColor& getBorderColor()  const { return m_borderColor; }

	int getDefaultValue() const { return m_iDefaultValue; }

	// Knob dial mode behavior:
	// QDialMode   - Old QDial BEHAVIOUR.
	// AngularMode - Angularly relative to widget center.
	// LinearMode  - Proportionally to distance in one ortogonal axis.

	enum DialMode {	QDialMode, AngularMode, LinearMode };

	DialMode getDialMode() const { return m_dialMode; }

public slots:

	// Set the colour of the knob
	void setKnobColor(const QColor& color);

	// Set the colour of the meter
	void setMeterColor(const QColor& color);

	// Set the colour of the pointer
	void setPointerColor(const QColor& color);

	// Set the colour of the border
	void setBorderColor(const QColor& color);

	// Set default (mid) value.
	void setDefaultValue(int iDefaultValue);

	// Set knob dial mode behavior.
	void setDialMode(DialMode dialMode);

protected:

	// Paint (drawing) methods.
	virtual void paintEvent(QPaintEvent *pPaintEvent);

	// Mouse angle determination.
	double mouseAngle(const QPoint& pos);

	// Alternate mouse behavior event handlers.
	virtual void mousePressEvent(QMouseEvent *pMouseEvent);
	virtual void mouseMoveEvent(QMouseEvent *pMouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent *pMouseEvent);
	virtual void wheelEvent(QWheelEvent *pWheelEvent);

private:

	// Custom colors.
	QColor m_knobColor;
	QColor m_meterColor;
	QColor m_pointerColor;
	QColor m_borderColor;

	// Default (mid) value.
	int m_iDefaultValue;

	// Knob dial mode behavior.
	DialMode m_dialMode;

	// Alternate mouse behavior tracking.
	bool   m_bMousePressed;
	QPoint m_posMouse;

	// Just for more precission on the movement
	double m_lastDragValue;
};


#endif  // __qsynthKnob_h

// end of qsynthKnob.h
