/******************************************************************************

   Skulpture - Classical Three-Dimensional Artwork for Qt 4

   Copyright (c) 2019 rncbc aka Rui Nuno Capela <rncbc@rncbc.org>
   Copyright (c) 2007-2009 Christoph Feck <christoph@maxiom.de>

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

#ifndef QSYNTHDIALSKULPTURESTYLE_H_
#define QSYNTHDIALSKULPTURESTYLE_H_

#include <QCommonStyle>

class qsynthDialSkulptureStyle : public QCommonStyle
{
public:
    qsynthDialSkulptureStyle() {}
    virtual ~qsynthDialSkulptureStyle() {}

    virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                    const QWidget *widget = 0) const;

};

#endif /* QSYNTHDIALSKULPTURESTYLE_H_ */
