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

#ifndef __CS_POLYTEXT_H__
#define __CS_POLYTEXT_H__

#include "csgeom/math3d.h"
#include "csutil/csvector.h"
#include "imesh/thing/polygon.h"
#include "iengine/fview.h"

struct iFrustumView;
struct csFrustumViewCleanup;
struct iMaterialHandle;
struct iPolygon3D;
struct csRGBpixel;
class csPolygon3D;
class csPolyTexture;
class csLightMap;
class csLightPatch;
class csFrustumContext;
class csFrustumView;
class csLight;
class csMatrix3;
class csVector3;
class csVector2;
class csColor;

/**
 * This is user-data for iFrustumView for the lighting process.
 * It represents a queue holding references to csPolyTexture
 * for all polygons that were hit by a light during the lighting
 * process.
 */
struct csLightingPolyTexQueue : public iFrustumViewUserdata
{
private:
  // Vector containing csPolygonTexture pointers.
  csVector polytxts;

public:
  csLightingPolyTexQueue ();
  virtual ~csLightingPolyTexQueue ();

  /**
   * Add a csPolyTexture to the queue. Only call this when the
   * polytexture is not already there! A csPolyTexture should be
   * added to the queue when it gets a shadow_bitmap.
   */
  void AddPolyTexture (csPolyTexture* pt);

  /**
   * Update all lightmaps or shadowmaps mentioned in the queue.
   */
  void UpdateMaps (csLight* light, const csVector3& lightpos,
  	const csColor& lightcolor);

  SCF_DECLARE_IBASE;
};

/**
 * This class represents a shadow-bitmap. It is used while calculating
 * lighting for one light on a polygon. First shadows are collected on
 * this bitmap. Later the bitmap will be used to update the lightmap.
 */
class csShadowBitmap
{
private:
  char* light;		// Light bitmap.
  char* shadow;		// Shadow bitmap.
  int lm_w, lm_h;	// Original lightmap size.
  int sb_w, sb_h;	// Shadow bitmap size.
  int quality;		// Quality factor.
  bool full_shadow;	// Optimization: we are fully shadowed.

private:
  /**
   * Get the lighting level of a point in the lightmap (using lightmap
   * coordinates). This will be a number between 1 and 0 with 1 meaning
   * fully lit and 0 meaning fully shadowed.
   */
  float GetLighting (int lm_u, int lm_v);

  /*
   * For csAntialiasedPolyFill().
   */
  static void LightPutPixel (int x, int y, float area, void *arg);
  static void LightDrawBox (int x, int y, int w, int h, void *arg);
  static void ShadowPutPixel (int x, int y, float area, void *arg);
  static void ShadowDrawBox (int x, int y, int w, int h, void *arg);

public:
  /**
   * Make a new shadow bitmap of the given lightmap size and quality.
   * Quality will be a number indicating how much we want to enhance
   * or reduce size of this bitmap compared to the lightmap size.
   * A quality of 0 means no change (i.e. the bitmap will hold as many
   * shadow-points as lumels). A quality of -1 means that for every 2x2
   * lumels there will be one shadow-point. A quality of 1 means that
   * one lumel corresponds with 2x2 shadow-points.
   */
  csShadowBitmap (int lm_w, int lm_h, int quality);

  /**
   * Destroy the shadow bitmap.
   */
  ~csShadowBitmap () { delete[] shadow; delete[] light; }

  /**
   * Render a polygon on this bitmap. The coordinates of this
   * polygon are given in lightmap coordinates. WARNING the given polygon
   * will be modified by this function!
   * 'val' can be 0 or 1 and will be used to fill the polygon. To render
   * a shadow you would use 1 and for light you would use 0.
   */
  void RenderPolygon (csVector2* poly, int num_vertices, int val);

  /**
   * Take a light and update the lightmap using the information in
   * this shadow-bitmap.
   * <ul>
   * <li>lightcell_shift is the shift to scale lightmap space to
   *     texture space (with texture space meaning 0 to real texture size).
   * <li>The shf_u, shf_v, mul_u, and mul_v fields define how to translate
   *     the previous texture space to uv space (where uv goes between
   *     0 and 1 for a single texture).
   * <li>m_t2w and v_t2w transform uv space to world space coordinates.
   * <li>light is the light and lightpos is the position of that light (which
   *     can be different from the position of the light given by 'light'
   *     itself because we can have space warping).
   * <li>lightcolor can also be different from the color of the light because
   *     it could in principle be modified by portals.
   * </ul>
   */
  void UpdateLightMap (csRGBpixel* lightmap,
	int lightcell_shift,
	float shf_u, float shf_v,
	float mul_u, float mul_v,
	const csMatrix3& m_t2w, const csVector3& v_t2w,
	csLight* light, const csVector3& lightpos,
	const csColor& lightcolor,
	const csVector3& poly_normal,
	float cosfact);

  /**
   * Take a light and update the shadowmap using the information in
   * this shadow-bitmap.
   * <ul>
   * <li>lightcell_shift is the shift to scale lightmap space to
   *     texture space (with texture space meaning 0 to real texture size).
   * <li>The shf_u, shf_v, mul_u, and mul_v fields define how to translate
   *     the previous texture space to uv space (where uv goes between
   *     0 and 1 for a single texture).
   * <li>m_t2w and v_t2w transform uv space to world space coordinates.
   * <li>light is the light and lightpos is the position of that light (which
   *     can be different from the position of the light given by 'light'
   *     itself because we can have space warping).
   * </ul>
   */
  void UpdateShadowMap (unsigned char* shadowmap,
	int lightcell_shift,
	float shf_u, float shf_v,
	float mul_u, float mul_v,
	const csMatrix3& m_t2w, const csVector3& v_t2w,
	csLight* light, const csVector3& lightpos,
	const csVector3& poly_normal,
	float cosfact);

  /**
   * Return true if this bitmap is fully shadowed.
   * @@@ Will always return false at the moment (not implemented yet).
   */
  bool IsFullyShadowed () const { return full_shadow; }

  /**
   * Return true if this bitmap is fully lit.
   * @@@ Will always return false at the moment (not implemented yet).
   */
  bool IsFullyLit () const { return false; }
};


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
  /**
   * Shadow-bitmap used while doing lighting.
   */
  csShadowBitmap* shadow_bitmap;

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

  /**
   * Update the lightmap for the given light.
   * 'vis' will be false if the polygon is totally shadowed. In this
   * case we should use 'subpoly' to see where the shadow must go and
   * not the base polygon which this csPolyTexture points too.
   */
  void FillLightMapNew (csFrustumView* lview, bool vis, csPolygon3D* subpoly);

  /**
   * Update the lightmap of this polygon using the current shadow-bitmap
   * and the light information below.
   */
  void UpdateFromShadowBitmap (csLight* light, const csVector3& lightpos,
  	const csColor& lightcolor);

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
  static void ProcessDelayedLightmaps (iFrustumView *lview,
  	csFrustumContext* ctxt, csFrustumViewCleanup *lighting_info);

  /// Collect all relevant shadows from this frustum that covers this lightmap
  bool CollectShadows (csFrustumView *lview,
  	csFrustumContext* ctxt, csPolygon3D *poly);

  //--------------------- iPolygonTexture implementation ---------------------
  SCF_DECLARE_IBASE;
  ///
  virtual iMaterialHandle *GetMaterialHandle () { return mat_handle; }
  ///
  virtual float GetFDU () { return fdu; }
  ///
  virtual float GetFDV () { return fdv; }
  /// Get width of lighted texture (power of 2)
  virtual int GetWidth () { return w; }
  /// Get height of lighted texture.
  virtual int GetHeight () { return h; }
  ///
  virtual int GetShiftU () { return shf_u; }
  ///
  virtual int GetIMinU () { return Imin_u; }
  ///
  virtual int GetIMinV () { return Imin_v; }
  ///
  virtual void GetTextureBox (float& fMinU, float& fMinV,
  	float& fMaxU, float& fMaxV);
  ///
  virtual int GetOriginalWidth () { return w_orig; }

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
  virtual void *GetCacheData (int idx) { return cache_data[idx]; }
  /// Set data used internally by texture cache
  virtual void SetCacheData (int idx, void *d) { cache_data[idx] = d; }
};

#endif // __CS_POLYTEXT_H__
