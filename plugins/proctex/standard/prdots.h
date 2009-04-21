/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_PROCDOTSTEX_H__
#define __CS_PROCDOTSTEX_H__

#include "csextern.h"

#include "csutil/cscolor.h"
#include "cstool/proctex.h"

/**
 * Random dots (demonstration procedural texture).
 */
class csProcDots : public csProcTexture
{
private:
  /// palette
  int *palette;
  /// number of colours in palette
  int palsize;
  /// Accumulated elapsed time.
  csTicks elapsed;
  /// State.
  int state;

  /// make my palette, max nr of colours
  void MakePalette (int max);

public:
  /// Create a new texture.
  csProcDots (iTextureFactory* p);
  ///
  virtual ~csProcDots ();

  virtual bool PrepareAnim ();

  /// Draw the next frame.
  virtual void Animate (csTicks current_time);
};

#endif // __CS_PROCDOTSTEX_H__

