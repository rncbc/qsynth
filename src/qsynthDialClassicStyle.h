/******************************************************************************

   Based on an original design by Thorsten Wilms.
   Implemented as a widget for the Rosegarden MIDI and audio sequencer
   and notation editor by Chris Cannam.
   Extracted into a standalone Qt3 widget by Pedro Lopez-Cabanillas
   and adapted for use in QSynth.
   Ported to Qt4 by Chris Cannam.
   Adapted as a QStyle by Pedro Lopez-Cabanillas.   
  
   This file copyright 2003-2006 Chris Cannam, 
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

#ifndef CLASSICTYLE_H_
#define CLASSICTYLE_H_

#include <QCommonStyle>

class qsynthDialClassicStyle : public QCommonStyle
{
public:
	qsynthDialClassicStyle() {};
	virtual ~qsynthDialClassicStyle() {};

	virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            		const QWidget *widget = 0) const;

};

#endif /*CLASSICTYLE_H_*/
