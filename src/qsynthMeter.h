// qsynthMeter.h
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

#ifndef __qsynthMeter_h
#define __qsynthMeter_h

#include <QFrame>

// Forward declarations.
class qsynthMeter;

class QHBoxLayout;


//----------------------------------------------------------------------------
// qsynthMeterScale -- Meter bridge scale widget.

class qsynthMeterScale : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qsynthMeterScale(qsynthMeter *pMeter);

protected:

	// Specific event handlers.
	void paintEvent(QPaintEvent *);

	// Draw IEC scale line and label.
	void drawLineLabel(QPainter *p, int y, const QString& sLabel);

private:

	// Local instance variables.
	qsynthMeter *m_pMeter;

	// Running variables.
	int m_iLastY;
};


//----------------------------------------------------------------------------
// qsynthMeterValue -- Meter bridge value widget.

class qsynthMeterValue : public QFrame
{
	Q_OBJECT

public:

	// Constructor.
	qsynthMeterValue(qsynthMeter *pMeter);

	// Frame value accessors.
	void setValue(float fValue);

	// Value refreshment.
	void refresh();

	// Reset peak holder.
	void peakReset();

protected:

	// Specific event handlers.
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);

private:

	// Local instance variables.
	qsynthMeter *m_pMeter;

	// Running variables.
	float m_fValue;

	int   m_iValue;
	float m_fValueDecay;

	int   m_iPeak;
	float m_fPeakDecay;
	int   m_iPeakHold;
	int   m_iPeakColor;
};


//----------------------------------------------------------------------------
// qsynthMeter -- Meter bridge slot widget.

class qsynthMeter : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qsynthMeter(QWidget *pParent = 0);
	// Default destructor.
	~qsynthMeter();

	// Port count accessor.
	int portCount() const;

	// Value proxy.    
	void setValue(int iPort, float fValue);

	// IEC scale accessors.
	int iec_scale(float dB) const;
	int iec_level(int iIndex) const;

#ifdef CONFIG_GRADIENT
	const QPixmap& pixmap() const;
	void updatePixmap();
#endif

	// Slot refreshment.
	void refresh();

	// Color/level indexes.
	enum {
		ColorOver	= 0,
		Color0dB	= 1,
		Color3dB	= 2,
		Color6dB	= 3,
		Color10dB	= 4,
		LevelCount	= 5,
		ColorBack	= 5,
		ColorFore	= 6,
		ColorCount	= 7
	};

	// Common resource accessors.
	const QColor& color(int iIndex) const;

	// Peak falloff mode setting.
	void setPeakFalloff(int bPeakFalloff);
	int peakFalloff() const;

	// Reset peak holder.
	void peakReset();

protected:

	// Specific event handlers.
	virtual void resizeEvent(QResizeEvent *);

private:

	// Local instance variables.
	QHBoxLayout       *m_pHBoxLayout;
	int                m_iPortCount;
	int                m_iScaleCount;
	qsynthMeterValue **m_ppValues;
	qsynthMeterScale **m_ppScales;

	float   m_fScale;

	int     m_levels[LevelCount];
	QColor  m_colors[ColorCount];

#ifdef CONFIG_GRADIENT
	QPixmap *m_pPixmap;
#endif

	// Peak falloff mode setting (0=no peak falloff).
	int m_iPeakFalloff;
};


#endif  // __qsynthMeter_h

// end of qsynthMeter.h
