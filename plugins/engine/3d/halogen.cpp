/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

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

// This doesn't look too realistic
//#define HALO_HAS_OUTER_RIM

/**
 * Generate an halo alpha map given halo iSize (iSize x iSize) and
 * halo intensity factor (0..1) (this is NOT the intensity itself!).
 * The iCross argument is an 0..1 float that defines how much the
 * hallo ressembles a cross (0) or a circle (1).
 */
unsigned char *csGenerateHalo (int iSize, float iFactor, float iCross)
{
  unsigned char *image = new unsigned char [iSize * iSize];
  memset (image, 0, iSize * iSize);

  const int s1 = iSize - 1;	// size - 1
  const int s2 = iSize / 2;	// halo diameter
#ifdef HALO_HAS_OUTER_RIM
  int s3 = (s2 / 2);		// outer rim diameter squared
  s3 *= s3;
#endif

  int x;
  for (x = 0; x <= s2; x++)
  {
	int y;
    for (y = /*s2*/x; y <= s2; y++)
    {
      int dx = (s2 - x);
      int dy = (s2 - y);
      float a = pow ((pow (float (dx) / float (s2), iCross) +
                      pow (float (dy) / float (s2), iCross)), iFactor);

#ifdef HALO_HAS_OUTER_RIM
      float rim = ABS (s3 - (dx * dx + dy * dy)) / 64.0;
      if (rim < 1)
        a = a * 0.7 + rim * 0.3;
#endif

      if (a <= 1)
      {
        unsigned char uca = 255 - int (255 * a);
        image [     x + (     y) * iSize] = uca;
        image [     y + (     x) * iSize] = uca;
        image [s1 - x + (     y) * iSize] = uca;
        image [s1 - y + (     x) * iSize] = uca;
        image [     x + (s1 - y) * iSize] = uca;
        image [     y + (s1 - x) * iSize] = uca;
        image [s1 - x + (s1 - y) * iSize] = uca;
        image [s1 - y + (s1 - x) * iSize] = uca;
      }
    }
  }

  return image;
}
