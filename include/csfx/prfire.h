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

#ifndef __PROCFIRETEX_H__
#define __PROCFIRETEX_H__

#include "csutil/cscolor.h"
#include "csfx/proctex.h"

/**
 *  Plasma.
 */
class csProcFire : public csProcTexture
{
private:
  /// palette
  int *palette;
  /// number of colours in palette
  int palsize;

  /// bottom line array
  uint8 *fireline;
  /// whole texture array...
  uint8 *image;

  /// make my palette, max nr of colours
  void MakePalette (int max);
  /// HSI to RGB csColor 
  void SetHSI (csColor& col, float H, float S, float I);
  /// get image val of x,y (wraps)
  uint8& GetImage (int x, int y);
  /// get fireline (wraps)
  uint8& GetFireLine (int x);

  /// animation parameter: possible burn (0..), additional burn (0..)
  /// try possburn 3=wood,90=oil,255=max. addburn=1..5 or so
  int possburn, addburn;
  /// animation parameter: continued burn (0..)  try 80
  int contburn;
  /// animation parameter: smoothing factor (0..) try 2.
  int smoothing;
  /// animation parameter: burning down param, try 3*256/height
  int extinguish;

  /// single flame mode
  bool single_flame_mode;
  /// 1/2 size of flame base, from middle bottom sideways.
  int halfbase;

public:
  /// Create a new texture.
  csProcFire ();
  ///
  virtual ~csProcFire ();

  virtual bool PrepareAnim ();

  /// Draw the next frame.
  virtual void Animate (cs_time current_time);

  /// set single flame mode with halfflamewidth or disable single flame.
  void SetSingleFlame(bool enable, int halfflame)
  { single_flame_mode = enable; halfbase = halfflame; }
  /// is the flame in singleflame(true) or in leftright tiling mode?
  bool GetSingleFlame() const {return single_flame_mode;}
};

#endif // __PROCFIRETEX_H__

