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
struct iUserRenderBufferIterator;

/**
 * This structure holds mapping information to map the texture and lightmap on
 * a polygon.
 */
struct csPolyTextureMapping
{
private:
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

  /// Width of lit texture ('w' is a power of 2).
  int w; //@@@ renderer specific

  /// Height of lit texture.
  int h; //@@@ renderer specific 

  /// Original width (may not be a power of 2) (w_orig <= w).
  int w_orig;  //@@@ renderer specific

  /**
   * Coordinates of the lightmap on the super lightmap, in renderer coords.
   */
  float lmu1, lmv1, lmu2, lmv2;
public:
  csPolyTextureMapping() :
    fdu(0.0f), fdv(0.0f), Imin_u(0), Imin_v(0),
    Fmin_u(0.0f), Fmin_v(0.0f), Fmax_u(0.0f), Fmax_v(0.0f), shf_u(0),
    w(0), h(0), w_orig(0), 
    lmu1(0.0f), lmv1(0.0f), lmu2(0.0f), lmv2(0.0f)
  {
  }

  const csMatrix3& GetO2T () const { return m_obj2tex; }
  csMatrix3& GetO2T () { return m_obj2tex; }
  const csVector3& GetO2TTranslation () const { return v_obj2tex; }
  csVector3& GetO2TTranslation () { return v_obj2tex; }
  void SetO2T (const csMatrix3& m) { m_obj2tex = m; }
  void SetO2TTranslation (const csVector3& v) { v_obj2tex = v; }

  csPolyTextureMapping& operator= (const csPolyTextureMapping& other)
  {
    m_obj2tex = other.m_obj2tex;
    v_obj2tex = other.v_obj2tex;
    fdu = other.fdu;
    fdv = other.fdv;
    Imin_u = other.Imin_u;
    Imin_v = other.Imin_v;
    Fmin_u = other.Fmin_u;
    Fmin_v = other.Fmin_v;
    Fmax_u = other.Fmax_u;
    Fmax_v = other.Fmax_v;
    shf_u = other.shf_u;
    w = other.w;
    h = other.h;
    w_orig = other.w_orig;
    lmu1 = other.lmu1;
    lmv1 = other.lmv1;
    lmu2 = other.lmu2;
    lmv2 = other.lmv2;
    return *this;
  }

  /**
   * Get the power of the lowest power of 2 that is not smaller than the
   * texture bounding box' width.
   * that is: 2^shift_u >= texbbox-width > 2^(shift_u-1)
   */
  int GetShiftU () const { return shf_u; }
  /// Set the shift.
  void SetShiftU (int su) { shf_u = su; }

  /// Get the rounded u-value of the textures bounding box' lower left corner.
  int GetIMinU () const { return Imin_u; }
  /// Get the rounded v-value of the textures bounding box' lower left corner.
  int GetIMinV () const { return Imin_v; }
  /**
   * Set the rounded u and v values of the textures bounding box' lower
   * left corner.
   */
  void SetIMinUV (int u, int v) { Imin_u = u; Imin_v = v; }
  /// Get texture box.
  void GetTextureBox (float& fMinU, float& fMinV,
    float& fMaxU, float& fMaxV) const
  {
    fMinU = Fmin_u;
    fMaxU = Fmax_u;
    fMinV = Fmin_v;
    fMaxV = Fmax_v;
  }
  /// Set texture box.
  void SetTextureBox (float fMinU, float fMinV,
    float fMaxU, float fMaxV)
  {
    Fmin_u = fMinU;
    Fmax_u = fMaxU;
    Fmin_v = fMinV;
    Fmax_v = fMaxV;
  }

  /// Get the u-value of the textures bounding box' lower left corner.
  float GetFDU () const { return fdu; }
  /// Get the v-value of the textures bounding box' lower left corner.
  float GetFDV () const { return fdv; }
  /**
   * Set the u and v values of the textures bounding box' lower
   * left corner.
   */
  void SetFDUV (float u, float v) { fdu = u; fdv = v; }

  /// Set width of lit texture (power of 2).
  void SetLitWidth (int w)
  {
    csPolyTextureMapping::w = w;
  }
  /// Set height of lit texture.
  void SetLitHeight (int h)
  {
    csPolyTextureMapping::h = h;
  }
  /// Set original width of lit texture.
  void SetLitOriginalWidth (int w_orig)
  {
    csPolyTextureMapping::w_orig = w_orig;
  }
  /// Get width of lit texture (power of 2).
  int GetLitWidth () const { return w; }
  /// Get height of lit texture.
  int GetLitHeight () const { return h; }

  /// Get original width.
  int GetLitOriginalWidth () const { return w_orig; }
  /// Get lightmap coordinates (on super lightmap).
  void GetCoordsOnSuperLM (float& lmu1, float& lmv1,
  	float& lmu2, float& lmv2) const
  {
    lmu1 = csPolyTextureMapping::lmu1;
    lmv1 = csPolyTextureMapping::lmv1;
    lmu2 = csPolyTextureMapping::lmu2;
    lmv2 = csPolyTextureMapping::lmv2;
  }
  /// Set lightmap coordinates (on super lightmap).
  void SetCoordsOnSuperLM (float lmu1, float lmv1,
  	float lmu2, float lmv2)
  {
    csPolyTextureMapping::lmu1 = lmu1;
    csPolyTextureMapping::lmv1 = lmv1;
    csPolyTextureMapping::lmu2 = lmu2;
    csPolyTextureMapping::lmv2 = lmv2;
  }
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
  /**
   * Array of normals. 0 if the plane normal should be used.
   */
  csVector3** objNormals;
};



SCF_VERSION (iPolygonRenderer, 0, 1, 0);

// @@@ Document me.
struct iPolygonRenderer : public iBase
{
  virtual void PrepareRenderMesh (csRenderMesh& mesh) = 0;
  
  virtual void Clear () = 0;
  virtual void AddPolygon (csPolygonRenderData* poly, 
    iUserRenderBufferIterator* extraBuffers) = 0;
};

/** @} */

#endif // __CS_IVIDEO_POLYRENDER_H__

