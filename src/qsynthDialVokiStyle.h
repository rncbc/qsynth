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

#ifndef VOKISTYLE_H_
#define VOKISTYLE_H_

#include <QCommonStyle>

class qsynthDialVokiStyle : public QCommonStyle
{
public:
	qsynthDialVokiStyle() {}
	virtual ~qsynthDialVokiStyle() {}
	
    virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            		const QWidget *widget = 0) const;
	
};

#endif /*VOKISTYLE_H_*/
