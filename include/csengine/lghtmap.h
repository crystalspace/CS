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

#ifndef __CS_LIGHTMAP_H__
#define __CS_LIGHTMAP_H__

#include "csutil/scf.h"
#include "csgfx/rgbpixel.h"
#include "iengine/lightmap.h"

class csPolyTexture;
class csThing;
class csPolygon3D;
class csCurve;
class csLight;
class csEngine;
class Dumper;
class csDelayedLightingInfo;
class csObject;

/**
 * This is a shadow-map for a pseudo-dynamic light.
 */
class csShadowMap
{
  ///
  friend class csLightMap;
  ///
  friend class csPolyTexture;
  friend class csCurve;
  friend class Dumper;

private:
  csShadowMap* next;
  unsigned char* map;

  /**
   * The pseudo-dynamic light.
   */
  csLight* light;

  ///
  csShadowMap ();
  ///
  virtual ~csShadowMap ();

  ///
  unsigned char* GetShadowMap () { return map; }

  ///
  void Alloc (csLight* light, int w, int h);
  ///
  void CopyLightMap (csShadowMap* source, int size);
};

/**
 * A small class encapsulating an RGB lightmap.
 */
class csRGBLightMap
{
private:
  int max_sizeRGB;	// Max size for every map.
  unsigned char* map;

public:
  ///
  void Clear ()
  {
    delete [] map; map = NULL;
    max_sizeRGB = 0;
  }

  ///
  csRGBLightMap () : max_sizeRGB (0), map (NULL) { }
  ///
  ~csRGBLightMap () { Clear (); }

  ///
  int GetMaxSize () { return max_sizeRGB; }
  ///
  void SetMaxSize (int s) { max_sizeRGB = s; }

  /// Get data.
  unsigned char* GetMap () { return map; }
#if !NEW_LM_FORMAT
  /// Get red map.
  unsigned char* GetRed () { return map; }
  /// Get green map.
  unsigned char* GetGreen () { return map+max_sizeRGB; }
  /// Get blue map.
  unsigned char* GetBlue () { return map+(max_sizeRGB<<1); }
#endif

  /// Set color maps.
  void SetMap (unsigned char* m) { map = m; }

  ///
  void Alloc (int size)
  {
    max_sizeRGB = size;
    delete [] map;
#if NEW_LM_FORMAT
    map = new unsigned char [size*4];
#else
    map = new unsigned char [size*3];
#endif
  }

  ///
  void Copy (csRGBLightMap& other, int size)
  {
    Clear ();
#if NEW_LM_FORMAT
    if (other.map) { Alloc (size); memcpy (map, other.map, size*4); }
#else
    if (other.map) { Alloc (size); memcpy (map, other.map, size*3); }
#endif
  }
};

/**
 * The static and all dynamic lightmaps for one polygon.
 */
class csLightMap : public iLightMap
{
  ///
  friend class csPolyTexture;
  ///
  friend class Dumper;

private:
  /**
   * A color lightmap containing all static lighting information
   * for the static lights (no pseudo-dynamic lights are here).
   */
  csRGBLightMap static_lm;

  /**
   * The final lightmap that is going to be used for rendering.
   * In many cases this is just a copy of static_lm. But it may
   * contain extra lighting information obtained from all the
   * pseudo-dynamic shadowmaps and also the true dynamic lights.
   */
  csRGBLightMap real_lm;

  /** 
   * A flag indicating whether the lightmaps needs recalculating
   * for dynamic lights
   */
  bool dyn_dirty;

  /**
   * Linked list of shadow-maps (for the pseudo-dynamic lights).
   * These shadowmaps are applied to static_lm to get real_lm.
   */
  csShadowMap* first_smap;

  /// Size of the lightmap.
  long lm_size;

  /// LightMap dims (possibly po2 depending on used 3D driver).
  int lwidth, lheight;
  /// Original lightmap dims (non-po2 possibly).
  int rwidth, rheight;
  
  /**
   * Mean lighting value of this lightmap.
   * (only for static lighting currently).
   */
  csRGBpixel mean_color;

  /// The hicolor cache ptr.
  void *cachedata;

  /// Used when computing lightmaps shared by several polygons
  csDelayedLightingInfo *delayed_light_info;

#if 0
  /**
   * Convert three lightmap tables to the right mixing mode.
   * This function assumes that mixing mode is one of FAST_Wxx
   * and will not function correctly if called with NOCOLOR, TRUE_RGB
   * or FAST_RGB.<br>
   * This function correctly recognizes a dynamic lightmap which only
   * contains shadow information and does not do the conversion in that
   * case.
   */
  void ConvertToMixingMode (unsigned char* mr, unsigned char* mg,
			    unsigned char* mb, int sz);
#endif

  /**
   * Calculate the sizes of this lightmap.
   */
  void SetSize (int w, int h);

public:
  /// Option variable: shadow cell size
  static int lightcell_size;
  /// Log base 2 of lightcell_size
  static int lightcell_shift;

  /// Return the width of a lightmap given a texture size.
  static int CalcLightMapWidth (int w)
  {
    return 1 + ((w + lightcell_size - 1) >> lightcell_shift);
  }
  /// Return the height of a lightmap given a texture size.
  static int CalcLightMapHeight (int h)
  {
    return 1 + ((h + lightcell_size - 1) >> lightcell_shift);
  }

  ///
  csLightMap ();
  ///
  virtual ~csLightMap ();

  /// set the dirty flag for this lightmap
  void MakeDirtyDynamicLights () { dyn_dirty = true; }

  bool UpdateRealLightMap ();
  
  ///
  csRGBLightMap& GetStaticMap () { return static_lm; }
  ///
  csRGBLightMap& GetRealMap () { return real_lm; }

  /**
   * Allocate the lightmap. 'w' and 'h' are the size of the
   * bounding box in lightmap space.
   * r,g,b is the ambient light color used to initialize the lightmap.
   */
  void Alloc (int w, int h, int r, int g, int b);

  /// Copy a lightmap.
  void CopyLightMap (csLightMap* source);

  /**
   * Create a ShadowMap for this LightMap.
   */
  csShadowMap* NewShadowMap (csLight* light, int w, int h);

  /**
   * Allocate the static RGBLightMap.
   */
  void AllocStaticLM (int w, int h);

  /**
   * Find a ShadowMap for a specific pseudo-dynamic light.
   */
  csShadowMap* FindShadowMap (csLight* light);

  /**
   * Delete a ShadowMap. NOTE!!! This function only works
   * if the ShadowMap was the LAST CREATED for this LightMap!!!
   * It is ment for dynamic lights that do not reach the polygon
   * but this can only be seen after trying.
   */
  void DelShadowMap (csShadowMap* plm);

  /**
   * Read from the cache. Return true if succesful.
   */
  bool ReadFromCache (int w, int h, csObject* obj, bool isPolygon, csEngine*);

  /**
   * Cache the lightmaps in the precalculation area.
   */
  void Cache (csPolygon3D* poly, csCurve* curve, csEngine*);

  /**
   * Convert the lightmaps to the correct mixing mode.
   * This function does nothing unless the mixing mode is
   * nocolor.<p>
   *
   * This function also calculates the mean color of the lightmap.
   */
  void ConvertToMixingMode ();

  /**
   * Convert the lightmaps to a 3D driver dependent size.
   */
  void ConvertFor3dDriver (bool requirePO2, int maxAspect = 32767);

  /**
   * Set the size of one lightmap cell (default = 16).
   * Do not directly assign to the lightcell_size variable, as
   * lightmap_shift also has to be updated.
   */
  static void SetLightCellSize (int size);

  //------------------------ iLightMap implementation ------------------------
  DECLARE_IBASE;
  ///
#if NEW_LM_FORMAT
  virtual unsigned char *GetMapData ();
#else
  virtual unsigned char *GetMap (int nMap);
#endif
  ///
  virtual int GetWidth ()
  { return lwidth; }
  ///
  virtual int GetHeight ()
  { return lheight; }
  ///
  virtual int GetRealWidth ()
  { return rwidth; }
  ///
  virtual int GetRealHeight ()
  { return rheight; }
  ///
  virtual void *GetCacheData ()
  { return cachedata; }
  ///
  virtual void SetCacheData (void *d)
  { cachedata = d; }
  ///
  virtual void GetMeanLighting (int &r, int &g, int &b)
  { r = mean_color.red; g = mean_color.green; b = mean_color.blue; }
  /// Get size of one lightmap
  virtual long GetSize ()
  { return lm_size; }
};

#endif // __CS_LIGHTMAP_H__
