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

#ifndef POLYTEXT_H
#define POLYTEXT_H

#include "csgeom/math3d.h"
#include "csengine/rview.h"
#include "ipolygon.h"

class csPolygon3D;
class Textures;
class csPolyPlane;
class csLightMap;
class csLightPatch;
class Dumper;
struct iTextureHandle;
struct LightInfo;

/// The default sub-texture size.
#define DEFAULT_SUBTEX_SIZE 32

/**
 * Define a small (3 pixels) margin at the bottom and top of
 * the texture. This is the easiest way I could find to 'fix' the
 * overflow problems with the texture mapper.
 */
#define H_MARGIN 3

/**
 * This class represents a lighted texture for a polygon.
 * A polygon generally has four of these (one for every mipmap
 * level).
 */
class csPolyTexture : public iPolygonTexture
{
  friend class csPolygon3D;
  friend class csPolygon2D;
  friend class Dumper;

private:
  /// Private texture cache data.
  void* tcache_data;

  /**
   * Dirty matrix used in combination with the sub-texture optimization.
   */
  UByte* dirty_matrix;

  /// Width of dirty matrix.
  UShort dirty_w;

  /// Height of dirty matrix.
  UShort dirty_h;

  /**
   * Number of dirty sub-textures (if 0 whole texture is clean
   * and in the cache.
   */
  UShort dirty_cnt;

  /// The corresponding polygon.
  csPolygon3D* polygon;

  /// The corresponding unlighted texture.
  iTextureHandle* txt_handle;

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

  /**
   * Size including vertical margins (note: size is in pixels,
   * multiply this value by the real number of bytes for every
   * pixel to get the real size).
   */
  int size;

  ///
  float fdu;
  ///
  float fdv;

  /// LightMap.
  csLightMap* lm;

  /// The mipmap level (0..3) that this PolyTexture is used for.
  UByte mipmap_level;

  /// Mipmap size to use for lightmap boxes: 16, 8, 4, or 2.
  UByte mipmap_size;

  /// Mipmap shift corresponding to the mipmap_size above.
  UByte mipmap_shift;

  /// If true, dynamic lighting needs to be recalculated.
  UByte dyn_dirty;

public:
  /// Option variable: do accurate lighting of things. This is much slower however.
  static bool do_accurate_things;

  /**
   * Option variable: control how much the angle of the light with the polygon
   * it hits affects the final light value. Values ranges from -1 to 1.
   * With -1 the polygons will get no light at all. With 0 it will be perfect
   * cosine rule. With 1 the cosine is ignored and it will be like Crystal Space
   * was in the past. Note that changing this value at runtime only has an
   * effect on dynamic lights.
   */
  static float cfg_cosinus_factor;

  /**
   * The size of the sub-textures.
   * Calculations of the textures in the texture cache are done on
   * parts of the textures (sub-textures). This number defines the
   * horizontal and vertical dimensions of such sub-textures.
   * If this value is set to 0 then this sub-texture optimization
   * is not used.<br>
   * Must be a power of 2 and larger or equal than the largest lightmap
   * box-size that is used.
   */
  static int subtex_size;

  /**
   * If the sub-texture optimization is enabled (subtex_size != 0) then
   * this option gives an additional optimization. With this option enabled
   * dynamic lights will only cause updating of the really touched sub-textures.
   * This results in even more efficient behaviour.
   */
  static bool subtex_dynlight;

  ///
  csPolyTexture ();
  ///
  virtual ~csPolyTexture ();

  /// Get the mipmap size used for this texture.
  int GetMipmapSize () { return mipmap_size; }

  /// Set the mipmap size used for this texture.
  void SetMipmapSize (int mm);

  /**
   * Set the corresponding polygon for this polytexture.
   */
  void SetPolygon (csPolygon3D* p) { polygon = p; }

  ///
  void SetMipmapLevel (int mm) { mipmap_level = mm; }

  /// Set the texture to be used for this polytexture.
  void SetTextureHandle (iTextureHandle* th) { txt_handle = th; }

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
  void FillLightMap (csLightView& lview);

  /**
   * Update the real lightmap for a given csLightPatch
   * (used for a dynamic light).
   */
  void ShineDynLightMap (csLightPatch* lp);

  ///
  void MakeDirtyDynamicLights ();

  /**
   * Destroy the dirty matrix. This needs to be done after
   * changing subtex_size.
   */
  void DestroyDirtyMatrix ();

  //--------------------- iPolygonTexture implementation ---------------------
  DECLARE_IBASE;
  ///
  virtual iTextureHandle *GetTextureHandle ();
  ///
  virtual float GetFDU ();
  ///
  virtual float GetFDV ();
  /// Get width of lighted texture (power of 2)
  virtual int GetWidth ();
  /// Get height of lighted texture.
  virtual int GetHeight ();
  ///
  virtual int GetMipmapLevel ();
  ///
  virtual int GetShiftU ();
  ///
  virtual int GetSize ();

  ///
  virtual void *GetTCacheData ();
  ///
  virtual void SetTCacheData (void *iCache);

  ///
  virtual int GetNumPixels ();
  ///
  virtual int GetMipMapSize ();
  ///
  virtual int GetMipMapShift ();
  ///
  virtual int GetIMinU ();
  ///
  virtual int GetIMinV ();
  ///
  virtual void GetTextureBox (float& fMinU, float& fMinV, float& fMaxU, float& fMaxV);
  ///
  virtual int GetOriginalWidth ();

  ///
  virtual iPolygon3D *GetPolygon ();
  /**
   * Recalculate all pseudo and real dynamic lights if the
   * texture is dirty. The function returns true if there
   * was a recalculation (then the texture needs to be removed
   * from the texture cache).
   */
  virtual bool RecalculateDynamicLights ();
  /**
   * Create the dirty matrix if needed. This function will also check if the dirty
   * matrix has the right size. If not it will recreate it.
   * The dirty matrix is used in combination with the sub-texture optimization.
   * If recreation of the dirty matrix was needed it will be made all dirty.
   */
  virtual void CreateDirtyMatrix ();
  /**
   * Make the dirty matrix completely dirty.
   */
  virtual void MakeAllDirty ();
  ///
  virtual bool CleanIfDirty (int lu, int lv);

  /// 
  virtual iLightMap *GetLightMap ();

  /// Return the number of dirty sub-textures.
  virtual int GetNumberDirtySubTex ();
  ///
  virtual int GetSubtexSize ();
  ///
  virtual bool GetDynlightOpt ();
};

#endif /*POLYTEXT_H*/
