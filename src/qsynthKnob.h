// qsynthKnob.h
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

#ifndef __qsynthKnob_h
#define __qsynthKnob_h

#include <qdial.h>
#include <qmap.h>


//-------------------------------------------------------------------------
// qsynthKnob - A better QDial for QSynth.

class qsynthKnob : public QDial
{
	Q_OBJECT
	Q_PROPERTY( QColor knobColor READ getKnobColor WRITE setKnobColor )
	Q_PROPERTY( QColor meterColor READ getMeterColor WRITE setMeterColor )
	Q_PROPERTY( bool mouseDial READ getMouseDial WRITE setMouseDial )
	Q_PROPERTY( int defaultValue READ getDefaultValue WRITE setDefaultValue )

public:

	// Constructor.
	qsynthKnob(QWidget *pParent = 0, const char *pszName = 0);
	// Destructor.
	~qsynthKnob();

	const QColor& getKnobColor()  const { return m_knobColor;  }
	const QColor& getMeterColor() const { return m_meterColor; }

	bool getMouseDial() const { return m_bMouseDial; }

	int getDefaultValue() const { return m_iDefaultValue; }

public slots:

	// Set the colour of the knob
	void setKnobColor(const QColor& color);

	// Set the colour of the meter
	void setMeterColor(const QColor& color);

	// (old) QDial mouse behavior.
	void setMouseDial(bool bMouseDial);

	// Set default (mid) value.
	void setDefaultValue(int iDefaultValue);

protected:

	void drawTick(QPainter& paint, double angle, int size, bool internal);
	virtual void repaintScreen(const QRect *pRect = 0);

	// Mouse angle determination.
	double mouseAngle(const QPoint& pos);

	// Alternate mouse behavior event handlers.
	virtual void mousePressEvent(QMouseEvent *pMouseEvent);
	virtual void mouseMoveEvent(QMouseEvent *pMouseEvent);
	virtual void mouseReleaseEvent(QMouseEvent *pMouseEvent);
	virtual void wheelEvent(QWheelEvent *pWheelEvent);
	void valueChange();

private:

	QColor m_knobColor;
	QColor m_meterColor;

	// Alternate mouse behavior tracking.
	bool   m_bMouseDial;
	bool   m_bMousePressed;
	QPoint m_posMouse;

	// Default (mid) value.
	int m_iDefaultValue;
};


#endif  // __qsynthKnob_h

// end of qsynthKnob.h
