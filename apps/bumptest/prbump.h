/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2000-2001 by W.C.A. Wijngaards

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

#ifndef __PROCBUMPTEX_H__
#define __PROCBUMPTEX_H__

#include "csutil/cscolor.h"
#include "csfx/proctex.h"

struct iImage;
struct iLight;

/**
 *  An bumpmapping overlay.
 *  The bumpmap specifies a perturbation of the Normal vector of the polygon.
 *  Since the polygon is already lighted nicely.
 *  Only the shading differences due to that perturbation have to be applied.
 *  Use with mixmode CS_FX_MULTIPLY2
 */
class csProcBump : public csProcTexture
{
private:
  /// palette
  int *palette;
  /// number of colours in palette
  int palsize;
  /// the bumpmap
  iImage *bumpmap;
  
  /// make my palette, max nr of colours
  void MakePalette (int max);
  /// HSI to RGB csColor 
  void SetHSI (csColor& col, float H, float S, float I);

  /// get a pixel's 'height' from the bumpmap
  int GetHeight(int x, int y);
  /// get the normal vector at some position
  csVector3 GetNormal(int x, int y, const csVector3& mainnormal,
    const csVector3& xdir, const csVector3& ydir);
public:
  /// Create a new texture, sized 256x256.
  csProcBump ();
  /// Create a new texture, right size for given bumpmap.
  csProcBump (iImage *map);
  ///
  virtual ~csProcBump ();

  virtual bool PrepareAnim ();

  /// set the bumpmap
  void SetBumpMap(iImage *m) {bumpmap = m;}
  /// calc an array of lights
  void Recalc(const csVector3& center, const csVector3& normal, 
    const csVector3& xdir, const csVector3& ydir,
    int numlight, iLight **lights);

  /// Draw the next frame.
  virtual void Animate (cs_time current_time);

  CSOBJTYPE;
};

#endif // __PROCBUMPTEX_H__

