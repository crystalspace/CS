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

#ifndef __IENGINE_POLYGON_H__
#define __IENGINE_POLYGON_H__

#include "csutil/scf.h"
#include "csgeom/plane3.h"

struct iMaterialHandle;
struct iMaterialWrapper;
struct iPolygon3D;
struct iPolygonTexture;
struct iLightMap;
struct iPortal;
struct iSector;
struct iThing;

class csPolygon3D;
class csVector3;
class csMatrix3;
class csColor;

/**
 * If CS_POLY_LIGHTING is set for a polygon then the polygon will be lit.
 * It is set by default.
 */
#define CS_POLY_LIGHTING	0x00000001

SCF_VERSION (iPolygon3D, 0, 1, 6);

/**
 * This is the interface to 3D polygons.
 */
struct iPolygon3D : public iBase
{
  /// Used by engine to retrieve internal object structure
  virtual csPolygon3D *GetPrivateObject () = 0;

  /// Get polygon name
  virtual const char *GetName () const = 0;
  /// Set polygon name
  virtual void SetName (const char *iName) = 0;

  /**
   * Get the thing (container) that this polygon belongs to.
   * The reference counter on iThing is NOT incremented.
   */
  virtual iThing *GetParent () = 0;
  /// Get the lightmap associated with this polygon
  virtual iLightMap *GetLightMap () = 0;
  /// Get the handle to the polygon texture object
  virtual iPolygonTexture *GetTexture () = 0;
  /// Get the material handle for the texture manager.
  virtual iMaterialHandle *GetMaterialHandle () = 0;
  /// Set the material for this polygon.
  virtual void SetMaterial (iMaterialWrapper* mat) = 0;

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
  virtual void CreatePlane (const csVector3 &iOrigin,
    const csMatrix3 &iMatrix) = 0;
  /// Set polygon texture mapping plane
  virtual bool SetPlane (const char *iName) = 0;

  /// Get the flags for this polygon
  virtual unsigned GetFlags () = 0;
  /// Set any number of flags for this polygon
  virtual void SetFlags (unsigned iMask, unsigned iValue) = 0;

  /// Set Gouraud vs lightmap polygon lighting
  virtual void SetLightingMode (bool iGouraud) = 0;

  /// Create a portal object pointing to given sector
  virtual iPortal *CreatePortal (iSector *iTarget) = 0;
  /**
   * Return the pointer to the portal if there is one.
   */
  virtual iPortal* GetPortal () = 0;

  /// Set texture space mapping (if using lightmapping) for this polygon.
  virtual void SetTextureSpace (csVector3& v_orig, csVector3& v1, float len1) = 0;

  /// Get world space plane.
  virtual const csPlane3& GetWorldPlane () = 0;
  /// Get object space plane.
  virtual const csPlane3& GetObjectPlane () = 0;
  /// Get camera space plane.
  virtual const csPlane3& GetCameraPlane () = 0;

  /**
   * Return true if this polygon or the texture it uses is transparent.
   */
  virtual bool IsTransparent () = 0;
};

SCF_VERSION (iPolygonTexture, 1, 0, 0);

/**
 * This is a interface to an object responsible for containing
 * the data required for a lighted texture on a polygon.
 */
struct iPolygonTexture : public iBase
{
  /// Get the material handle associated with this polygon
  virtual iMaterialHandle *GetMaterialHandle () = 0;
  /// Get FDU :-)
  virtual float GetFDU () = 0;
  /// Get FDV :-)
  virtual float GetFDV () = 0;
  /// Get width of lighted texture (power of 2)
  virtual int GetWidth () = 0;
  /// Get height of lighted texture.
  virtual int GetHeight () = 0;
  ///Get ShiftU :-)
  virtual int GetShiftU () = 0;

  /// Get IMinU :-)
  virtual int GetIMinU () = 0;
  /// Get IMinV :-)
  virtual int GetIMinV () = 0;
  /// Get texture box.
  virtual void GetTextureBox (float& fMinU, float& fMinV,
    float& fMaxU, float& fMaxV) = 0;
  /// Get original width.
  virtual int GetOriginalWidth () = 0;

  /// Get polygon.
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

  /// Get light map.
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

#endif // __IENGINE_POLYGON_H__
