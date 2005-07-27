/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_CANVAS_DRAW_BOX_H__
#define __CS_CSPLUGINCOMMON_CANVAS_DRAW_BOX_H__

/**\file
 * Draw a box to a (software) canvas.
 */

#include "csplugincommon/canvas/draw_common.h"

/**
 * \addtogroup plugincommon
 * @{ */

/**
 * Draw a box to a (software) canvas.
 * \c Tpixel defines the type of a pixel (e.g. uint32...),
 * \c Tpixmixer a matching mixer class.
 */
template<class Tpixel, class Tpixmixer>
class csG2DDrawBox
{
public:
  /// Draw a box.
  static void DrawBox (csGraphics2D* G2D, int x, int y, int w, int h,
    Tpixel color, uint8 alpha)
  {
    Tpixmixer mixer (G2D, color, alpha);
    while (h)
    {
      register Tpixel* dest = (Tpixel*)G2D->GetPixelAt (x, y);
      register int count = w;
      while (count--) mixer.Mix (*dest++);
      y++; h--;
    } /* endwhile */
  }
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_CANVAS_DRAW_BOX_H__
