/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles

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

#ifndef __CS_IVIDEO_HALO_H__
#define __CS_IVIDEO_HALO_H__

/**\file
 * Halo interface
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"

class csVector2;

SCF_VERSION (iHalo, 0, 0, 1);

/**
 * iHalo: used to render halos (aka "light globes").
 * This interface can be used as well for any scalable semi-transparent
 * 2D sprites. The "halo" is really just an alpha map; the sprite is a
 * single-colored rectangle with more or less transparent portions
 * (depends on alpha map).
 */
struct iHalo : public iBase
{
  /// Query halo width
  virtual int GetWidth () = 0;

  /// Query halo height
  virtual int GetHeight () = 0;

  /// Change halo color
  virtual void SetColor (float &iR, float &iG, float &iB) = 0;

  /// Query halo color
  virtual void GetColor (float &oR, float &oG, float &oB) = 0;

  /**
   * Draw the halo given a center point and an intensity.
   * If either w and/or h is negative, the native width and/or height
   * is used instead. If the halo should be clipped against some
   * polygon, that polygon should be given, otherwise if a 0 pointer
   * is passed, the halo is clipped just against screen bounds.
   */
  virtual void Draw (float x, float y, float w, float h, float iIntensity,
    csVector2 *iVertices, size_t iVertCount) = 0;
};

/** @} */

#endif // __CS_IVIDEO_HALO_H__
