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

scfInterface iLightMap;
scfInterface iPolygonTexture;
scfInterface iTextureContainer;
scfInterface iTextureMap;
scfInterface iTextureHandle;

// forward declarations
scfInterface iPolygon3D;
scfInterface iPolygonTexture;

/// temporary - subject to change
SCF_INTERFACE (iPolygonSet, 0, 0, 1) : public iBase
{
  ///
  virtual const char *GetObjectName () = 0;
};

/// temporary - subject to change
SCF_INTERFACE (iPolygon3D, 0, 0, 1) : public iBase
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

/// temporary - subject to change
SCF_INTERFACE (iPolygonTexture, 0, 0, 1) : public iBase
{
  ///
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
  virtual void *GetTCacheData () = 0;
  ///
  virtual void SetTCacheData (void *iCache) = 0;

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
  ///
  virtual bool RecalculateDynamicLights () = 0;
  ///
  virtual void CreateDirtyMatrix () = 0;
  ///
  virtual void MakeAllDirty () = 0;
  ///
  virtual bool CleanIfDirty (int lu, int lv) = 0;

  /// 
  virtual iLightMap *GetLightMap () = 0;

  /// Return the number of dirty sub-textures.
  virtual int GetNumberDirtySubTex () = 0;
  /// Return the number of clean sub-textures.
  virtual int GetNumberCleanSubTex () = 0;
  ///
  virtual int GetSubtexSize () = 0;
  ///
  virtual bool GetDynlightOpt () = 0;
};

#endif
