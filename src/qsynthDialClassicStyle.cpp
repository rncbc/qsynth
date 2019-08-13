/******************************************************************************

   Based on an original design by Thorsten Wilms.
   Implemented as a widget for the Rosegarden MIDI and audio sequencer
   and notation editor by Chris Cannam.
   Extracted into a standalone Qt3 widget by Pedro Lopez-Cabanillas
   and adapted for use in QSynth.
   Ported to Qt4 by Chris Cannam.
   Adapted as a QStyle by Pedro Lopez-Cabanillas.   

   This file,
   copyright 2019 rncbc aka Rui Nuno Capela,
   copyright 2003-2006 Chris Cannam,
   copyright 2005,2008 Pedro Lopez-Cabanillas, 
   copyright 2006 Queen Mary, University of London.

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

#include "qsynthDialClassicStyle.h"

#include <QStyleOptionSlider>
#include <QPainter>

#include <iostream>
#include <cmath>

#define DIAL_MIN (0.25 * M_PI)
#define DIAL_MAX (1.75 * M_PI)
#define DIAL_RANGE (DIAL_MAX - DIAL_MIN)

static void drawTick(QPainter *p, float angle, int size, bool internal)
{
	float hyp = float(size) / 2.0;
	float x0 = hyp - (hyp - 1) * sin(angle);
	float y0 = hyp + (hyp - 1) * cos(angle);
	if (internal) {
		float len = hyp / 4;
		float x1 = hyp - (hyp - len) * sin(angle);
		float y1 = hyp + (hyp - len) * cos(angle);
		p->drawLine(int(x0), int(y0), int(x1), int(y1));
	} else {
		float len = hyp / 4;
		float x1 = hyp - (hyp + len) * sin(angle);
		float y1 = hyp + (hyp + len) * cos(angle);
		p->drawLine(int(x0), int(y0), int(x1), int(y1));
	}
}

void 
qsynthDialClassicStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget) const
{
	if (cc != QStyle::CC_Dial) {
		QCommonStyle::drawComplexControl(cc, opt, p, widget);
		return;
	}
	
	const QStyleOptionSlider *dial = qstyleoption_cast<const QStyleOptionSlider *>(opt); 
	if (dial == nullptr)
		return;
	
    float angle = DIAL_MIN + (DIAL_RANGE * (float(dial->sliderValue - dial->minimum) /
                   (float(dial->maximum - dial->minimum))));
	int degrees = int(angle * 180.0 / M_PI);
    int ns = dial->tickInterval;
    int numTicks = 1 + (dial->maximum + ns - dial->minimum) / ns;
    int size = dial->rect.width() < dial->rect.height() ? dial->rect.width() : dial->rect.height();
    int scale = 1;
    int width = size * scale;
    int indent = (int)(width * 0.15 + 1);

	QPalette pal = opt->palette;    
    QColor knobColor = pal.mid().color(); //pal.background().color();
    QColor meterColor = (dial->state & State_Enabled) ? pal.highlight().color() : pal.mid().color(); 
    QPen pen;
    QColor c;

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->translate(1+(dial->rect.width()-size)/2, 1+(dial->rect.height()-size)/2);

    // Knob body and face...

    pen.setColor(knobColor);
    pen.setWidth(scale * 2);
    pen.setCapStyle(Qt::FlatCap);
    p->setPen(pen);
    
    QRadialGradient gradient(size/2, size/2, size-indent, size/2-indent, size/2-indent);
	gradient.setColorAt(0, knobColor.lighter());
	gradient.setColorAt(1, knobColor.darker());
	p->setBrush(gradient);    
    p->drawEllipse(indent, indent, width-2*indent, width-2*indent);

    // The bright metering bit...

    c = meterColor;
    pen.setColor(c);
    pen.setWidth(indent);
    p->setPen(pen);

    int arcLen = -(degrees - 45) * 16;
    if (arcLen == 0) arcLen = -16;
    p->drawArc(indent/2, indent/2, width-indent, width-indent, (180 + 45) * 16, arcLen);
    p->setBrush(Qt::NoBrush);

    // Tick notches...

    if (dial->subControls & QStyle::SC_DialTickmarks) {
    	//	std::cerr << "Notches visible" << std::endl;
    	pen.setColor(pal.dark().color());
    	pen.setWidth(scale);
    	p->setPen(pen);
    	for (int i = 0; i < numTicks; ++i) {
    		int div = numTicks;
    		if (div > 1) --div;
    		drawTick(p, DIAL_MIN + (DIAL_MAX - DIAL_MIN) * i / div, width, true);
    	}
    }

    // Shadowing...

    pen.setWidth(scale);
    p->setPen(pen);

    // Knob shadow...

    int shadowAngle = -720;
    c = knobColor.darker();
    for (int arc = 120; arc < 2880; arc += 240)
	{
		pen.setColor(c);
		p->setPen(pen);
		p->drawArc(indent, indent, width-2*indent, width-2*indent, shadowAngle
				+ arc, 240);
		p->drawArc(indent, indent, width-2*indent, width-2*indent, shadowAngle
				- arc, 240);
		c = c.lighter(110);
	}

    // Scale shadow...

    shadowAngle = 2160;
    c = pal.dark().color();
    for (int arc = 120; arc < 2880; arc += 240) {
    	pen.setColor(c);
    	p->setPen(pen);
    	p->drawArc(scale/2, scale/2,
    			width-scale, width-scale, shadowAngle + arc, 240);
    	p->drawArc(scale/2, scale/2,
    			width-scale, width-scale, shadowAngle - arc, 240);
    	c = c.lighter(108);
    }

    // Undraw the bottom part...

    pen.setColor(pal.window().color());
    pen.setWidth(scale * 4);
    p->setPen(pen);
    p->drawArc(scale/2, scale/2,
		  width-scale, width-scale, -45 * 16, -90 * 16);

    // Pointer notch...

    float hyp = float(width) / 2.0;
    float len = hyp - indent;
    --len;

    float x0 = hyp;
    float y0 = hyp;

    float x = hyp - len * sin(angle);
    float y = hyp + len * cos(angle);

    c = pal.dark().color();
    pen.setColor((dial->state & State_Enabled) ? c.darker(130) : c);
    pen.setWidth(scale * 2);
    p->setPen(pen);
    p->drawLine(int(x0), int(y0), int(x), int(y));

	// done
	p->restore();	
}
