/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __CS_POLYTEXT_H__
#define __CS_POLYTEXT_H__

#include "csgeom/math3d.h"
#include "csengine/rview.h"
#include "ipolygon.h"

class csPolygon3D;
class Textures;
class csPolyPlane;
class csLightMap;
class csLightPatch;
class csBitSet;
class Dumper;
struct iMaterialHandle;
struct iPolygon3D;
struct LightInfo;


// a structure used to build the light coverage data
struct csCoverageMatrix
{
  // The coverage array. Each float corresponds to a lightmap grid cell
  // and contains the area of light cell that is covered by light.
  float *coverage;
  // The width and height of the coverage array
  int width, height;

  csCoverageMatrix (int w, int h)
  { 
    coverage = new float [ (width = w) * (height = h)]; 
    memset (coverage, 0, w*h*sizeof(float)); 
  }

  ~csCoverageMatrix ()
  { delete[]coverage; }
};

/**
 * This class represents a lighted texture for a polygon.
 */
class csPolyTexture : public iPolygonTexture
{
  friend class csPolygon3D;
  friend class csPolygon2D;
  friend class Dumper;

private:
  /// The corresponding polygon.
  csPolygon3D* polygon;
  /// SCF pointer.
  iPolygon3D* ipolygon;

  /// The corresponding unlighted material.
  iMaterialHandle* mat_handle;

  /**
   * Bounding box of corresponding polygon in 2D texture space.
   * Note that the u-axis of this bounding box is made a power of 2 for
   * efficiency reasons.
   */
  int Imin_u, Imin_v, Imax_u, Imax_v;

  //@@@Fmin... Fmax... is the same for all four csPolyTexture.
  //SHARE!
  /// fp bounding box (0..1 texture space)
  float Fmin_u, Fmin_v, Fmax_u, Fmax_v;

  ///
  UShort shf_u;
  ///
  UShort and_u;

  /*
   * New texture data with lighting added. This is an untiled texture
   * so it is more efficient to draw. This texture data is allocated
   * and maintained by the texture cache. If a PolyTexture is in the
   * cache it will be allocated, otherwise it won't.
   */

  /// Width of lighted texture ('w' is a power of 2).
  int w;

  /// Height of lighted texture.
  int h;

  /// Original width (not a power of 2) (w_orig <= w).
  int w_orig;

  ///
  float fdu;
  ///
  float fdv;

  /// LightMap.
  csLightMap* lm;

  /// Internally used by (software) texture cache
  void *cache_data [4];

public:
  /**
   * Option variable: control how much the angle of the light with the polygon
   * it hits affects the final light value. Values ranges from -1 to 1.
   * With -1 the polygons will get no light at all. With 0 it will be perfect
   * cosine rule. With 1 the cosine is ignored and it will be like Crystal Space
   * was in the past. Note that changing this value at runtime only has an
   * effect on dynamic lights.
   */
  static float cfg_cosinus_factor;

  ///
  csPolyTexture ();
  ///
  virtual ~csPolyTexture ();

  /**
   * Set the corresponding polygon for this polytexture.
   */
  void SetPolygon (csPolygon3D* p);

  /**
   * Return the polygon corresponding to this texture
   */
  csPolygon3D *GetCSPolygon () { return polygon; }

  /**
   * Set the lightmap for this polytexture (and call IncRef()
   * on the lightmap). Can also be used to clear the reference
   * to the lightmap if 'lightmap' is NULL.
   */
  void SetLightMap (csLightMap* lightmap);

  /// Get the cslightmap, for engine internal use (users see GetLightMap below)
  csLightMap *GetCSLightMap() { return lm; }

  /// Set the material to be used for this polytexture.
  void SetMaterialHandle (iMaterialHandle* th) { mat_handle = th; }

  /**
   * Calculate the bounding box in (u,v) space for the lighted texture.
   */
  void CreateBoundingTextureBox ();

  /**
   * Initialize the lightmaps.
   */
  void InitLightMaps ();

  /**
   * Update the lightmap for the given light.
   */
  void FillLightMap (csFrustumView& lview);

  /// set the dirty flag for our lightmap
  void MakeDirtyDynamicLights ();

  /**
   * Update the real lightmap for a given csLightPatch
   * (used for a dynamic light).
   */
  void ShineDynLightMap (csLightPatch* lp);

  /// Get the bounding rectangle of the whole lightmap in world space
  bool GetLightmapBounds (const csVector3& lightpos, bool mirror,
  	csVector3 *bounds);

  /// Get the coverage matrix for the associated lightmap
  void GetCoverageMatrix (csFrustumView& lview, csCoverageMatrix &cm);

  /// Process lighting for all delayed polygon lightmaps
  static void ProcessDelayedLightmaps (csFrustumView *lview,
    csFrustumViewCleanup *lighting_info);

  /// Collect all relevant shadows from this frustum that covers this lightmap
  bool CollectShadows (csFrustumView *lview, csPolygon3D *poly);

  //--------------------- iPolygonTexture implementation ---------------------
  DECLARE_IBASE;
  ///
  virtual iMaterialHandle *GetMaterialHandle ();
  ///
  virtual float GetFDU ();
  ///
  virtual float GetFDV ();
  /// Get width of lighted texture (power of 2)
  virtual int GetWidth ();
  /// Get height of lighted texture.
  virtual int GetHeight ();
  ///
  virtual int GetShiftU ();
  ///
  virtual int GetIMinU ();
  ///
  virtual int GetIMinV ();
  ///
  virtual void GetTextureBox (float& fMinU, float& fMinV, float& fMaxU, float& fMaxV);
  ///
  virtual int GetOriginalWidth ();

  ///
  virtual iPolygon3D *GetPolygon ()
  {
    return ipolygon;
  }
  /// Check if dynamic lighting information should be recalculated
  virtual bool DynamicLightsDirty ();
  /**
   * Recalculate all pseudo and real dynamic lights if the
   * texture is dirty. The function returns true if there
   * was a recalculation (then the texture needs to be removed
   * from the texture cache).
   */
  virtual bool RecalculateDynamicLights ();

  /// 
  virtual iLightMap *GetLightMap ();
  /// Query the size of one light cell
  virtual int GetLightCellSize ();
  /// Query log2 (cell size)
  virtual int GetLightCellShift ();

  /// Get data used internally by texture cache
  virtual void *GetCacheData (int idx);
  /// Set data used internally by texture cache
  virtual void SetCacheData (int idx, void *d);
};

#endif // __CS_POLYTEXT_H__
