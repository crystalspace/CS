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

struct iTextureHandle;
struct iPolygonSet;
struct iPolygon3D;
struct iPolygonTexture;
struct iLightMap;

class csVector3;
class csMatrix3;

SCF_VERSION (iPolygon3D, 0, 1, 0);

/**
 * This is the interface to 3D polygons.
 * The iPolygonSet interface defines the interface for
 * the containers of iPolygon3D objects.
 */
struct iPolygon3D : public iBase
{
  /// Get polygon name
  virtual const char *GetName () = 0;
  /// Set polygon name
  virtual void SetName (const char *iName) = 0;

  /// Get the polygonset (container) that this polygons belongs to.
  virtual iPolygonSet *GetContainer () = 0;
  /// Get the lightmap associated with this polygon
  virtual iLightMap *GetLightMap () = 0;
  /// Get the handle to the polygon texture object
  virtual iPolygonTexture *GetTexture () = 0;
  /// Get the texture handle for the texture manager.
  virtual iTextureHandle *GetTextureHandle () = 0;

  /// Query number of vertices in this polygon
  virtual int GetVertexCount () = 0;
  /// Get the given polygon vertex coordinates in object space
  virtual csVector3 &GetVertex (int idx) = 0;
  /// Get the given polygon vertex coordinates in world space
  virtual csVector3 &GetVertexW (int idx) = 0;
  /// Get the given polygon vertex coordinates in camera space
  virtual csVector3 &GetVertexC (int idx) = 0;
  /// Create a polygon vertex given his index in parent polygon set
  virtual int CreateVertex (int idx) = 0;
  /// Create a polygon vertex and add it to parent object
  virtual int CreateVertex (const csVector3 &iVertex) = 0;

  /// Get the alpha transparency value for this polygon.
  virtual int GetAlpha () = 0;
  /**
   * Set the alpha transparency value for this polygon (only if
   * it is a portal).
   * Not all renderers support all possible values. 0, 25, 50,
   * 75, and 100 will always work but other values may give
   * only the closest possible to one of the above.
   */
  virtual void SetAlpha (int iAlpha) = 0;

  /// Create a private polygon texture mapping plane
  virtual void CreatePlane (const csVector3 &iOrigin, const csMatrix3 &iMatrix) = 0;
  /// Set polygon texture mapping plane
  virtual bool SetPlane (const char *iName) = 0;
};

SCF_VERSION (iPolygonTexture, 1, 0, 0);

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
  virtual int GetShiftU () = 0;

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
  /// Check if dynamic lighting information should be recalculated
  virtual bool DynamicLightsDirty () = 0;
  /**
   * Recalculate all pseudo and real dynamic lights if the
   * texture is dirty. The function returns true if there
   * was a recalculation (then the texture needs to be removed
   * from the texture cache).
   */
  virtual bool RecalculateDynamicLights () = 0;

  /// 
  virtual iLightMap *GetLightMap () = 0;
  /// Query the size of one light cell (always a power of two)
  virtual int GetLightCellSize () = 0;
  /// Query log2 (cell size)
  virtual int GetLightCellShift () = 0;

  /// Get data used internally by texture cache
  virtual void *GetCacheData (int idx) = 0;
  /// Set data used internally by texture cache
  virtual void SetCacheData (int idx, void *d) = 0;
};

#endif // __IPOLYGON_H__
