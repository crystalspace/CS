/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_THING_POLYGON_H__
#define __CS_THING_POLYGON_H__

#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"

struct iMaterialHandle;
struct csPolyLightMapMapping;
struct csPolyTextureMapping;

class csVector3;
class csMatrix3;

/**
 * This structure holds mapping information to map the texture on
 * a polygon.
 */
struct csPolyTextureMapping
{
  /// Transformation from object to texture space.
  csMatrix3 m_obj2tex;
  /// Translation from object to texture space.
  csVector3 v_obj2tex;

  // @@@ some entries really rather belong to csPolyLightMapMapping.
  float fdu, fdv;

  /**
   * Bounding box of corresponding polygon in 2D texture space.
   * Note that the u-axis of this bounding box is made a power of 2 for
   * efficiency reasons.
   */
  int Imin_u, Imin_v;

  /// fp bounding box (0..1 texture space)
  float Fmin_u, Fmin_v, Fmax_u, Fmax_v;

  ///
  uint16 shf_u;

  /**
   * Get the power of the lowest power of 2 that is not smaller than the
   * texture bounding box' width.
   * that is: 2^shift_u >= texbbox-width > 2^(shift_u-1)
   */
  int GetShiftU () const { return shf_u; }

  /// Get the rounded u-value of the textures bounding box' lower left corner.
  int GetIMinU () const { return Imin_u; }
  /// Get the rounded v-value of the textures bounding box' lower left corner.
  int GetIMinV () const { return Imin_v; }
  /// Get texture box.
  void GetTextureBox (float& fMinU, float& fMinV,
    float& fMaxU, float& fMaxV) const
  {
    fMinU = Fmin_u;
    fMaxU = Fmax_u;
    fMinV = Fmin_v;
    fMaxV = Fmax_v;
  }

  /// Get the u-value of the textures bounding box' lower left corner.
  float GetFDU () const { return fdu; }
  /// Get the v-value of the textures bounding box' lower left corner.
  float GetFDV () const { return fdv; }
};

/**
 * This structure holds mapping information to map the lightmap on
 * a polygon.
 */
struct csPolyLightMapMapping
{
  /*
   * New texture data with lighting added. This is an untiled texture
   * so it is more efficient to draw. This texture data is allocated
   * and maintained by the texture cache. If a PolyTexture is in the
   * cache it will be allocated, otherwise it won't.
   */

  /// Width of lighted texture ('w' is a power of 2).
  int w; //@@@ renderer specific

  /// Height of lighted texture.
  int h; //@@@ renderer specific 

  /// Original width (may not be a power of 2) (w_orig <= w).
  int w_orig;  //@@@ renderer specific
  
  /// Light cell size
  //int lightCellSize;
  /// Light cell shift
  //int lightCellShift;

  /// Get width of lit texture (power of 2).
  int GetWidth () const { return w; }
  /// Get height of lit texture.
  int GetHeight () const { return h; }

  /// Get original width.
  int GetOriginalWidth () const { return w_orig; }
};

#endif // __CS_THING_POLYGON_H__

