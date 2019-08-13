/***************************************************************************

   This style is based on a widget designed by Giuseppe Cigala,
   adapted as a Qt style for QSynth by Pedro Lopez-Cabanillas

   This file,
   Copyright (C) 2019 rncbc aka Rui Nuno Capela <rncbc@rncbc.org>,
   Copyright (C) 2008 Giuseppe Cigala <g_cigala@virgilio.it>,
   Copyright (C) 2008 Pedro Lopez-Cabanillas <plcl@users.sf.net>.

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

#include "qsynthDialPeppinoStyle.h"

#include <QStyleOptionSlider>
#include <QPainter>

#include <iostream>
#include <cmath>

inline void paintBorder(QPainter *p)
{
    p->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QLinearGradient linGrad1(0, 100, 100, 80);
    linGrad1.setColorAt(0, Qt::gray);
    linGrad1.setColorAt(1, Qt::white);
    linGrad1.setSpread(QGradient::ReflectSpread);
    p->setBrush(linGrad1);

    QRectF border(5, 5, 190, 190);
    p->drawRect(border);

    // draw screws
    p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p->drawEllipse(10, 10, 10, 10);
    p->drawLine(13, 13, 17, 17);
    p->drawLine(17, 13, 13, 17);

    p->drawEllipse(180, 10, 10, 10);
    p->drawLine(183, 13, 187, 17);
    p->drawLine(187, 13, 183, 17);

    p->drawEllipse(10, 180, 10, 10);
    p->drawLine(13, 183, 17, 187);
    p->drawLine(17, 183, 13, 187);

    p->drawEllipse(180, 180, 10, 10);
    p->drawLine(183, 183, 187, 187);
    p->drawLine(187, 183, 183, 187);

    QLinearGradient linGrad(20, 150, 210, 160);
    linGrad.setColorAt(0, Qt::white);
    linGrad.setColorAt(1, Qt::darkGray);
    linGrad.setSpread(QGradient::PadSpread);
    p->setBrush(linGrad);

    p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QRectF rectangle(15, 15, 170, 170);
    p->drawPie(rectangle, 225*16, -270*16);
}

inline int valueAngle(const QStyleOptionSlider *dial)
{
    return -((dial->sliderValue - dial->minimum) * 4320) / (dial->maximum - dial->minimum);
}

inline void paintArc(QPainter *p, const QStyleOptionSlider *dial)
{
    QPalette pal = dial->palette;
    QColor arcColor = (dial->state & QStyle::State_Enabled) ? pal.highlight().color() : pal.mid().color();
    QLinearGradient linGrad(80, 100, 140, 140);
    linGrad.setColorAt(0, arcColor.darker(140));
    linGrad.setColorAt(1, arcColor.lighter().lighter());
    linGrad.setSpread(QGradient::PadSpread);
    p->setBrush(linGrad);

    p->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QRectF rectangle(15, 15, 170, 170);

    int spanAngle = valueAngle(dial);
    int startAngle = 225 * 16;
    p->drawPie(rectangle, startAngle, spanAngle);
}

inline void paintDial(QPainter *p)
{
    p->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QLinearGradient linGrad1(20, 140, 90, 90);
    linGrad1.setColorAt(0, Qt::gray);
    linGrad1.setColorAt(1, Qt::white);
    linGrad1.setSpread(QGradient::ReflectSpread);
    p->setBrush(linGrad1);

    QRectF border1(35, 35, 130, 130);
    p->drawEllipse(border1);

    p->setPen(QPen(Qt::black, 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    QRectF border2(40, 40, 120, 120);
    p->drawEllipse(border2);
}

inline void paintDot(QPainter *p, const QStyleOptionSlider *dial)
{
    int startPoint = (225 * 16) + valueAngle(dial);
    QPalette pal = dial->palette;
    QPen dotPen((dial->state & QStyle::State_Enabled) ? Qt::red : pal.mid().color(), 7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QRect rectangle1(45, 45, 110, 110);
    p->setPen(dotPen);
    p->drawArc(rectangle1, startPoint, 5);

    if (dial->subControls & QStyle::SC_DialTickmarks)
    {
    	QRect rectangle2(10, 10, 180, 180);
    	int ns = dial->tickInterval;
    	int dot = 1 + (dial->maximum + ns - dial->minimum) / ns; //int dot = 25;
    	double delta = 4320.0 / dot;
    	p->setPen(QPen(Qt::black, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    	for (int i = 0; i <= dot; i++)
    	{
    		p->drawArc(rectangle2, int(225*16 - delta*i), 5);
    	}
    }
}

void
qsynthDialPeppinoStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget) const
{
	if (cc != QStyle::CC_Dial) {
		QCommonStyle::drawComplexControl(cc, opt, p, widget);
		return;
	}

	const QStyleOptionSlider *dial = qstyleoption_cast<const QStyleOptionSlider *>(opt);
	if (dial == nullptr)
		return;

	p->save();
	int size = dial->rect.width() < dial->rect.height() ? dial->rect.width() : dial->rect.height();
	p->setViewport((dial->rect.width()-size)/2, (dial->rect.height()-size)/2, size, size);
    p->setWindow(0, 0, 200, 200);
    p->setRenderHint(QPainter::Antialiasing);

    paintBorder(p);
    paintArc(p, dial);
    paintDial(p);
    paintDot(p, dial);

	// done
	p->restore();
}
