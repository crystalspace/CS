/*
    Copyright (C) 1999 by Denis Dmitriev
    Antialiased polygon splitting

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_POLYAA_H__
#define __CS_POLYAA_H__


#include "csextern.h"

#include "csgeom/csrect.h"

/**\file 
 * Antialiased polygon filling.
 */
/**
 * \addtogroup geom_utils
 * @{ */

class csVector2;

/**
 * Callback functions used by csAntialiasedPolyFill. We have two types of
 * callback: one for "drawing a pixel", the area of the pixel is passed
 * to the callback routine. The second routine is for "drawing a box",
 * the area of every "pixel" is always 1.0.
 */
/// "Draw one pixel" callback
typedef void (*csAAPFCBPixel) (int x, int y, float area, void *arg);
/// "Draw a box" callback
typedef void (*csAAPFCBBox) (int x, int y, int w, int h, void *arg);

/**
 * This function takes a 2D polygon and splits it against a integer grid
 * into many sub-polygons. Then the area of each subpolygon is computed
 * and a callback function is called, with the area of sub-polygon
 * passed as argument.
 */
extern CS_CRYSTALSPACE_EXPORT  void csAntialiasedPolyFill (csVector2 *iVertices, 
  int iVertexCount, void *iArg, csAAPFCBPixel iPutPixel, 
  csAAPFCBBox iDrawBox = 0);

/** @} */

#endif // __CS_POLYAA_H__
