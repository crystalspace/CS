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

#ifndef __PROCPLASMATEX_H__
#define __PROCPLASMATEX_H__

#include "csutil/cscolor.h"
#include "cstool/proctex.h"

/**
 *  Plasma.
 */
class csProcPlasma : public csProcTexture
{
private:
  /// palette
  int *palette;
  /// number of colours in palette
  int palsize;
  
  /// cos array
  uint8 *costable;

  /// table indices;
  uint8 anims[4];
  /// offsets
  uint8 offsets[2];
  /// increments
  int frameincr[4], lineincr[4], offsetincr[2];

  /// make my palette, max nr of colours
  void MakePalette (int max);
  /// get cos of angle (in 0..255) as a value 0..64
  uint8 GetCos (uint8 angle) const { return costable[angle]; }
  /// Make the cos table
  void MakeTable ();

public:
  /// Create a new texture.
  csProcPlasma ();
  ///
  virtual ~csProcPlasma ();

  virtual bool PrepareAnim ();

  /// Draw the next frame.
  virtual void Animate (csTicks current_time);
};

#endif // __PROCPLASMATEX_H__

