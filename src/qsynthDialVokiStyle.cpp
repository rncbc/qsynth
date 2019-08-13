/******************************************************************************

   This style is based on a design by Thorsten Wilms, 
   implemented as a widget by Chris Cannam in Rosegarden,
   adapted for QSynth by Pedro Lopez-Cabanillas,
   improved for Qt4 by David Garcia Garzon,
   adapted as a QStyle by Pedro Lopez-Cabanillas,
   updated for Qt5 by rncbc aka Rui Nuno Capela.

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

#include "qsynthDialVokiStyle.h"

#include <QStyleOptionSlider>
#include <QPainter>

#include <iostream>
#include <cmath>

#define DIAL_MIN      (0.25 * M_PI)
#define DIAL_MAX      (1.75 * M_PI)
#define DIAL_RANGE    (DIAL_MAX - DIAL_MIN)

void 
qsynthDialVokiStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget) const
{
	if (cc != QStyle::CC_Dial)
	{
		QCommonStyle::drawComplexControl(cc, opt, p, widget);
		return;
	}
	
	const QStyleOptionSlider *dial = qstyleoption_cast<const QStyleOptionSlider *>(opt); 
	if (dial == nullptr)
		return;
	
	double angle = DIAL_MIN // offset
		+ (DIAL_RANGE *
			(double(dial->sliderValue - dial->minimum) /
			(double(dial->maximum - dial->minimum))));
	int degrees = int(angle * 180.0 / M_PI);
	int side = dial->rect.width() < dial->rect.height() ? dial->rect.width() : dial->rect.height();
	int xcenter = dial->rect.width() / 2;
	int ycenter = dial->rect.height() / 2;
	int notchWidth   = 1 + side / 400;
	int pointerWidth = 2 + side / 30;
	int scaleShadowWidth = 1 + side / 100;
	int knobBorderWidth = 0;
	int ns = dial->tickInterval;
	int numTicks = 1 + (dial->maximum + ns - dial->minimum) / ns;
	int indent = int(0.15 * side) + 2;
	int knobWidth = side - 2 * indent;
	int shineFocus = knobWidth / 4;
	int shineCenter = knobWidth / 5;
	int shineExtension = shineCenter * 4;
	int shadowShift = shineCenter * 2;
	int meterWidth = side - 2 * scaleShadowWidth;
	
	QPalette pal = opt->palette;
	QColor knobColor = pal.mid().color();
	QColor borderColor = knobColor.lighter();
	QColor meterColor = (dial->state & State_Enabled) ? 
						pal.highlight().color() : pal.mid().color();   
	QColor background = pal.window().color();

	p->save();
	p->setRenderHint(QPainter::Antialiasing, true);

	// The bright metering bit...

	QConicalGradient meterShadow(xcenter, ycenter, -90);
	meterShadow.setColorAt(0, meterColor.darker());
	meterShadow.setColorAt(0.5, meterColor);
	meterShadow.setColorAt(1.0, meterColor.lighter().lighter());
	p->setBrush(meterShadow);
	p->setPen(Qt::transparent);
	p->drawPie(xcenter - meterWidth / 2, ycenter - meterWidth / 2,
		meterWidth, meterWidth, (180 + 45) * 16, -(degrees - 45) * 16);

	// Knob projected shadow
	QRadialGradient projectionGradient(
		xcenter + shineCenter, ycenter + shineCenter,
		shineExtension,	xcenter + shadowShift, ycenter + shadowShift);
	projectionGradient.setColorAt(0, QColor(  0, 0, 0, 100));
	projectionGradient.setColorAt(1, QColor(200, 0, 0,  10));
	QBrush shadowBrush(projectionGradient);
	p->setBrush(shadowBrush);
	p->drawEllipse(xcenter - shadowShift, ycenter - shadowShift,
		knobWidth, knobWidth);

	// Knob body and face...

	QPen pen;
	pen.setColor(knobColor);
	pen.setWidth(knobBorderWidth);
	p->setPen(pen);

	QRadialGradient gradient(
		xcenter - shineCenter, ycenter - shineCenter,
		shineExtension,	xcenter - shineFocus, ycenter - shineFocus);
	gradient.setColorAt(0.2, knobColor.lighter().lighter());
	gradient.setColorAt(0.5, knobColor);
	gradient.setColorAt(1.0, knobColor.darker(150));
	QBrush knobBrush(gradient);
	p->setBrush(knobBrush);
	p->drawEllipse(xcenter - knobWidth / 2, ycenter - knobWidth / 2,
		knobWidth, knobWidth);

	// Tick notches...

	p->setBrush(Qt::NoBrush);

	if (dial->subControls & QStyle::SC_DialTickmarks)
	{
		pen.setColor(pal.dark().color());
		pen.setWidth(notchWidth);
		p->setPen(pen);
		double hyp = double(side - scaleShadowWidth) / 2.0;
		double len = hyp / 4;
		for (int i = 0; i < numTicks; ++i) {
			int div = numTicks;
			if (div > 1) --div;
			bool internal = (i != 0 && i != numTicks - 1);
			double angle = DIAL_MIN
				+ (DIAL_MAX - DIAL_MIN) * i / div;
			double dir = (internal ? -1 : len);
			double sinAngle = sin(angle);
			double cosAngle = cos(angle);
			double x0 = xcenter - (hyp - len) * sinAngle;
			double y0 = ycenter + (hyp - len) * cosAngle;
			double x1 = xcenter - (hyp + dir) * sinAngle;
			double y1 = ycenter + (hyp + dir) * cosAngle;
			p->drawLine(QLineF(x0, y0, x1, y1));
		}
	}

	// Shadowing...

	// Knob shadow...
	if (knobBorderWidth > 0) {
		QLinearGradient inShadow(xcenter - side / 4, ycenter - side / 4,
			xcenter + side / 4, ycenter + side / 4);
		inShadow.setColorAt(0.0, borderColor.lighter());
		inShadow.setColorAt(1.0, borderColor.darker());
		p->setPen(QPen(QBrush(inShadow), knobBorderWidth * 7 / 8));
		p->drawEllipse(xcenter - side / 2 + indent,
			ycenter - side / 2 + indent,
			side - 2 * indent, side - 2 * indent);
	}

	// Scale shadow...
	QLinearGradient outShadow(xcenter - side / 3, ycenter - side / 3,
		xcenter + side / 3, ycenter + side / 3);
	outShadow.setColorAt(0.0, background.darker().darker());
	outShadow.setColorAt(1.0, background.lighter().lighter());
	p->setPen(QPen(QBrush(outShadow), scaleShadowWidth));
	p->drawArc(xcenter - side / 2 + scaleShadowWidth / 2,
		ycenter - side / 2 + scaleShadowWidth / 2,
		side - scaleShadowWidth, side - scaleShadowWidth, -45 * 16, 270 * 16);

	// Pointer notch...

	double hyp = double(side) / 2.0;
	double len = hyp - indent - 1;

	double x = xcenter - len * sin(angle);
	double y = ycenter + len * cos(angle);

	QColor pointerColor = pal.dark().color();
	pen.setColor((dial->state & State_Enabled) ? pointerColor.darker(140) : pointerColor);
	pen.setWidth(pointerWidth + 2);
	p->setPen(pen);
	p->drawLine(QLineF(xcenter, ycenter, x, y));
	pen.setColor((dial->state & State_Enabled) ? pointerColor.lighter() : pointerColor.lighter(140));
	pen.setWidth(pointerWidth);
	p->setPen(pen);
	p->drawLine(QLineF(xcenter - 1, ycenter - 1, x - 1, y - 1));
	
	// done
	p->restore();
}
