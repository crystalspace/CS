/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#include "cssysdef.h"
#include "csutil/scf.h"

#include "csgfx/imagetools.h"

static inline unsigned sqr (int x)
{
  return (x * x);
}

int csImageTools::ClosestPaletteIndex (const csRGBpixel* Palette, 
				       const csRGBpixel& iColor, 
				       int palEntries)
{
  if (!Palette)
    return -1;

  int closest_idx = -1;
  unsigned closest_dst = (unsigned)-1;

  int idx;
  for (idx = 0; idx < palEntries; idx++)
  {
    unsigned dst = sqr (iColor.red   - Palette [idx].red)   * R_COEF_SQ +
                   sqr (iColor.green - Palette [idx].green) * G_COEF_SQ +
                   sqr (iColor.blue  - Palette [idx].blue)  * B_COEF_SQ;
    if (dst == 0)
      return idx;
    if (dst < closest_dst)
    {
      closest_dst = dst;
      closest_idx = idx;
    } /* endif */
  }
  return closest_idx;
}
