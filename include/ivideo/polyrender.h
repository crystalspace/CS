/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter

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

#ifndef __CS_IVIDEO_POLYRENDER_H__
#define __CS_IVIDEO_POLYRENDER_H__

/**\file
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"

class csVector3;
class csMatrix3;
struct csRenderMesh;

/**
 * This structure holds mapping information to map the texture and lightmap on
 * a polygon.
 */
struct csPolyTextureMapping
{
  csPolyTextureMapping() : fdu(0.0f), fdv(0.0f), Imin_u(0), Imin_v(0),
    Fmin_u(0.0f), Fmin_v(0.0f), Fmax_u(0.0f), Fmax_v(0.0f), shf_u(0),
    w(0), h(0), w_orig(0), 
    lmu1(0.0f), lmv1(0.0f), lmu2(0.0f), lmv2(0.0f)
  {
  }

  /// Transformation from object to texture space.
  csMatrix3 m_obj2tex;
  /// Translation from object to texture space.
  csVector3 v_obj2tex;

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

  /// Width of lit texture ('w' is a power of 2).
  int w; //@@@ renderer specific

  /// Height of lit texture.
  int h; //@@@ renderer specific 

  /// Original width (may not be a power of 2) (w_orig <= w).
  int w_orig;  //@@@ renderer specific

  /// Get width of lit texture (power of 2).
  int GetWidth () const { return w; }
  /// Get height of lit texture.
  int GetHeight () const { return h; }

  /// Get original width.
  int GetOriginalWidth () const { return w_orig; }

  /**
   * Coordinates of the lightmap on the super lightmap, in renderer coords.
   */
  float lmu1, lmv1, lmu2, lmv2;
};

/**
 * This structure is used for communicating polygon information to the
 * polygon renderer.
 */
struct csPolygonRenderData
{
  /// Object space plane of the polygon.
  csPlane3 plane_obj;
  /// Texture mapping information.
  csPolyTextureMapping* tmapping;
  /// Number of vertices in this polygon.
  int num_vertices;
  /// Pointer to vertex indices.
  int* vertices;
  /**
   * Double pointer to the array of vertices in object space.
   */
  csVector3** p_obj_verts;
  /// Poly uses lightmap
  bool useLightmap;
};



SCF_VERSION (iPolygonRenderer, 0, 1, 0);

// @@@ Document me.
struct iPolygonRenderer : public iBase
{
  virtual void PrepareRenderMesh (csRenderMesh& mesh) = 0;
  
  virtual void Clear () = 0;
  virtual void AddPolygon (csPolygonRenderData* poly) = 0;
};

/** @} */

#endif // __CS_IVIDEO_POLYRENDER_H__

