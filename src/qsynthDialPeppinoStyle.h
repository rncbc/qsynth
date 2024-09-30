/***************************************************************************

   This style is based on a widget designed by Giuseppe Cigala,  
   adapted as a Qt style for QSynth by Pedro Lopez-Cabanillas

   This file,
   Copyright (C) 2019 rncbc aka Rui Nuno Capela <rncbc@rncbc.org>,
   Copyright (C) 2008 Giuseppe Cigala <g_cigala@virgilio.it>,
   Copyright (C) 2008 Pedro Lopez-Cabanillas <plcl@users.sf.net>,
   Copyright (C) 2024 Rui Nuno Capela <rncbc@rncbc.org>.

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

#ifndef PEPPINOSTYLE_H_
#define PEPPINOSTYLE_H_

#include <QCommonStyle>

class qsynthDialPeppinoStyle : public QCommonStyle
{
public:
	qsynthDialPeppinoStyle() {};
	virtual ~qsynthDialPeppinoStyle() {};

	virtual void drawComplexControl(
		ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
		const QWidget *widget = 0) const;
};

#endif /*PEPPINOSTYLE_H_*/
