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

#ifndef __IPOLYGON_H__
#define __IPOLYGON_H__

#include "csutil/scf.h"
#include "csgeom/math3d.h"

struct iLightMap;
struct iPolygonTexture;
struct iTextureContainer;
struct iTextureMap;
struct iTextureHandle;
class csBitSet;

// forward declarations
struct iPolygon3D;
struct iPolygonTexture;

SCF_VERSION (iPolygonSet, 0, 0, 1);

/// temporary - subject to change
struct iPolygonSet : public iBase
{
  ///
  virtual const char *GetObjectName () = 0;
};

SCF_VERSION (iPolygon3D, 0, 0, 2);

/// temporary - subject to change
struct iPolygon3D : public iBase
{
  ///
  virtual const char *GetObjectName () = 0;
  /// Get the polygonset (container) that this polygons belongs to.
  virtual iPolygonSet *GetParentObject () = 0;
  ///
  virtual csVector3 *GetCameraVector (int idx) = 0;
  ///
  virtual iPolygonTexture *GetObjectTexture (int nLevel) = 0;
  /// 
  virtual bool UsesMipMaps () = 0;
  /// Get the alpha transparency value for this polygon.
  virtual int GetAlpha () = 0;
  ///
  virtual iLightMap *GetLightMap () = 0;
};

SCF_VERSION (iPolygonTexture, 0, 0, 1);

/// temporary - subject to change
struct iPolygonTexture : public iBase
{
  /// Get the texture handle associated with this polygon
  virtual iTextureHandle *GetTextureHandle () = 0;
  ///
  virtual float GetFDU () = 0;
  ///
  virtual float GetFDV () = 0;
  /// Get width of lighted texture (power of 2)
  virtual int GetWidth () = 0;
  /// Get height of lighted texture.
  virtual int GetHeight () = 0;
  ///
  virtual int GetMipmapLevel () = 0;
  ///
  virtual int GetShiftU () = 0;
  ///
  virtual int GetSize () = 0;

  ///
  virtual int GetNumPixels () = 0;
  ///
  virtual int GetMipMapSize () = 0;
  ///
  virtual int GetMipMapShift () = 0;
  ///
  virtual int GetIMinU () = 0;
  ///
  virtual int GetIMinV () = 0;
  ///
  virtual void GetTextureBox (float& fMinU, float& fMinV, float& fMaxU, float& fMaxV) = 0;
  ///
  virtual int GetOriginalWidth () = 0;

  ///
  virtual iPolygon3D *GetPolygon () = 0;
  /**
   * Recalculate all pseudo and real dynamic lights if the
   * texture is dirty. The function returns true if there
   * was a recalculation (then the texture needs to be removed
   * from the texture cache).
   */
  virtual bool RecalculateDynamicLights () = 0;
  /**
   * Create the dirty matrix if needed. This function will also check
   * if the dirty matrix has the right size. If not it will recreate it.
   * The dirty matrix is used in combination with the sub-texture optimization.
   * If recreation of the dirty matrix was needed it will be made all dirty.
   */
  virtual void CreateDirtyMatrix () = 0;
  /**
   * Make the dirty matrix completely dirty.
   */
  virtual void MakeAllDirty () = 0;
  /**
   * Check if there are any dirty lightmap cells, and clean the dirty
   * matrix in the corresponding places if so. Returns true if there are
   * any coincident bits in both bit sets (and thus we need to compute
   * any lightmap cells in texture cache).
   */
  virtual bool CleanIfDirty (csBitSet *bs) = 0;

  /// 
  virtual iLightMap *GetLightMap () = 0;

  /// Return the number of dirty sub-textures.
  virtual int GetNumberDirtySubTex () = 0;
  ///
  virtual int GetSubtexSize () = 0;
  ///
  virtual bool GetDynlightOpt () = 0;
  /// Get data used internally by texture cache
  virtual void *GetCacheData () = 0;
  /// Set data used internally by texture cache
  virtual void SetCacheData (void *d) = 0;
};

#endif
