/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef __CS_PROCWATERTEX_H__
#define __CS_PROCWATERTEX_H__

#include "csextern.h"

#include "csutil/cscolor.h"
#include "cstool/proctex.h"

/**
 * Water.
 */
class CS_CSTOOL_EXPORT csProcWater : public csProcTexture
{
private:
  /// palette
  int *palette;
  /// number of colours in palette
  int palsize;

  /// whole texture arrays..
  signed char *image;
  /// nr of arrays
  int nr_images;
  /// current images
  int cur_image;

  /// make my palette, max nr of colours
  void MakePalette (int max);
  /// HSI to RGB csColor
  void SetHSI (csColor& col, float H, float S, float I);
  /// get image val of x,y (wraps)
  signed char& GetImage (int im, int x, int y);

  /// dampening try 4
  int dampening;

public:
  /// Create a new texture.
  csProcWater (iTextureFactory* p);
  ///
  virtual ~csProcWater ();

  virtual bool PrepareAnim ();

  /// Draw the next frame.
  virtual void Animate (csTicks current_time);

  /// Make a puddle in the water (as if a raindrop) center, radius, strength.
  void MakePuddle (int sx, int sy, int rad, int val);
  /// Press down at x,y,radius, strength.
  void PressAt (int sx, int sy, int rad, int val);
};

#endif // __CS_PROCWATERTEX_H__

