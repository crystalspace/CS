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

#ifndef LIGHTMAP_H
#define LIGHTMAP_H

#include "cscom/com.h"
#include "ilghtmap.h"

class csPolyTexture;
class csPolygonSet;
class csPolygon3D;
class csLight;
class csWorld;
class Dumper;

struct HighColorCache_Data;

/**
 * This is a shadow-map for a pseudo-dynamic light.
 */
class csShadowMap
{
  ///
  friend class csLightMap;
  ///
  friend class csPolyTexture;

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
  void Alloc (csLight* light, int w, int h, int lms);
  ///
  void MipmapLightmap (int w, int h, int lms, csShadowMap* source, int w2, int h2, int lms2);
  ///
  void CopyLightmap (csShadowMap* source, int size);
};

/**
 * A small class encapsulating an RGB lightmap.
 */
class csRGBLightMap
{
public:
  unsigned char* mapR;
  unsigned char* mapG;
  unsigned char* mapB;

  ///
  void Clear ()
  {
    CHK (delete [] mapR); mapR = NULL;
    CHK (delete [] mapG); mapG = NULL;
    CHK (delete [] mapB); mapB = NULL;
  }

  ///
  csRGBLightMap () { mapR = mapG = mapB = NULL; }
  ///
  ~csRGBLightMap () { Clear (); }

  ///
  void AllocRed (int size)
  {
    CHK (mapR = new unsigned char [size]);
  }

  ///
  void AllocGreen (int size)
  {
    CHK (mapG = new unsigned char [size]);
  }

  ///
  void AllocBlue (int size)
  {
    CHK (mapB = new unsigned char [size]);
  }

  ///
  void Copy (csRGBLightMap& other, int size)
  {
    Clear ();
    if (other.mapR) { AllocRed (size); memcpy (mapR, other.mapR, size); }
    if (other.mapG) { AllocGreen (size); memcpy (mapG, other.mapG, size); }
    if (other.mapB) { AllocBlue (size); memcpy (mapB, other.mapB, size); }
  }
};

/**
 * The static and all dynamic lightmaps for one or more mipmap-levels of a polygon.
 */
class csLightMap
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
   * Linked list of shadow-maps (for the pseudo-dynamic lights).
   * These shadowmaps are applied to static_lm to get real_lm.
   */
  csShadowMap* first_smap;

  /// Size of the lighted texture.
  long size;
  /// Size of the lightmap.
  long lm_size;

  /// Lightmap dims (possibly po2 depending on used 3D driver).
  int lwidth, lheight;
  /// Original lightmap dims (non-po2 possibly).
  int rwidth, rheight;
  
  /// The hicolor cache ptr.
  HighColorCache_Data *hicolorcache;

  /**
   * Mean lighting value of this lightmap.
   * (only for static lighting currently).
   */
  int mean_r;
  int mean_g;
  int mean_b;
  
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

  /**
   * Calculate the sizes of this lightmap.
   */
  void SetSize (int w, int h, int lms);

public:
  ///
  csLightMap ();
  ///
  virtual ~csLightMap ();

  ///
  csRGBLightMap& GetStaticMap () { return static_lm; }
  ///
  csRGBLightMap& GetRealMap () { return real_lm; }

  ///
  void GetMeanLighting (int& r, int& g, int& b) { r = mean_r; g = mean_g; b = mean_b; }

  ///
  int GetWidth () { return lwidth; }
  ///
  int GetHeight () { return lheight; }
  ///
  int GetRealWidth () { return rwidth; }
  ///
  int GetRealHeight () { return rheight; }
  ///
  long GetSize () { return lm_size; }

  // DAN: High color cache specific stuff
  bool in_memory;
  ///
  HighColorCache_Data* GetHighColorCacheData () { return hicolorcache; }
  ///
  void SetHighColorCacheData (HighColorCache_Data *d) { hicolorcache = d; }

  /**
   * Allocate the lightmap. 'w' and 'h' are the size of the
   * bounding box in lightmap space.
   */
  void Alloc (int w, int h, int lms, csPolygon3D* poly);

  /**
   * Allocate this lightmap by mipmapping the given source lightmap.
   */
  void MipmapLightmap (int w, int h, int lms, csLightMap* source, int w2, int h2, int lms2);

  /// Copy a lightmap.
  void CopyLightmap (csLightMap* source);

  /**
   * Create a ShadowMap for this LightMap.
   */
  csShadowMap* NewShadowMap (csLight* light, int w, int h, int lms);

  /**
   * Allocate the static RGBLightMap.
   */
  void AllocStaticLM (int w, int h, int lms);

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
   * Index is the index of the polygon in the containing object. It is used
   * for identifying the lightmap on disk.
   */
  bool ReadFromCache (int w, int h, int lms, csPolygonSet* owner, csPolygon3D* poly, int index, csWorld* world);

  /**
   * Cache the lightmaps in the precalculation area.
   */
  void Cache (csPolygonSet* owner, csPolygon3D* poly, int index, csWorld* world);

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

  DECLARE_INTERFACE_TABLE( csLightMap )
  DECLARE_IUNKNOWN()

  DECLARE_COMPOSITE_INTERFACE( LightMap )
};

#define GetILightMapFromcsLightMap(a)  &a->m_xLightMap
#define GetcsLightMapFromILightMap(a)  ((csLightMap*)((size_t)a - offsetof(csLightMap, m_xLightMap)))

#endif /*LIGHTMAP_H*/

