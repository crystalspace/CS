#ifndef __IPOLYGON_H__
#define __IPOLYGON_H__

#include "cscom/com.h"
#include "csgeom/math3d.h"

interface ILightMap;
interface IPolygonTexture;
interface ITextureContainer;
interface ITextureMap;
interface ITextureHandle;

extern const IID IID_IPolygon3D;
extern const IID IID_IPolygonSet;
extern const IID IID_IPolygonTexture;

// forward declarations
interface IPolygonSet;
interface IPolygon3D;
interface IPolygonTexture;

/// temporary - subject to change
interface IPolygonSet : public IUnknown
{
  ///
  STDMETHOD (GetName) (const char** szName);

  DECLARE_IUNKNOWN ()
};


/// COM Vector type - for passing between DLL boundaries only!
struct ComcsVector3
{
  float x, y, z;
};

/// temporary - subject to change
interface IPolygon3D : public IUnknown
{
  ///
  STDMETHOD (GetName) (const char** szName);
  ///
  STDMETHOD (GetParent) (IPolygonSet** retval);
  ///
  STDMETHOD (GetCameraVector) (int idx, ComcsVector3* retval);
  ///
  STDMETHOD (GetTexture) (int nLevel, IPolygonTexture** retval);
  /// 
  STDMETHOD (UsesMipMaps) (void);
  ///
  STDMETHOD (GetAlpha) (int& retval);
  ///
  STDMETHOD (GetLightMap) (ILightMap** retval);

  DECLARE_IUNKNOWN ()
};

/// temporary - subject to change
interface IPolygonTexture : public IUnknown
{
  ///
  STDMETHOD (GetTextureHandle) (ITextureHandle** retval);
  ///
  STDMETHOD (GetFDU) (float& retval);
  ///
  STDMETHOD (GetFDV) (float& retval);
  ///
  STDMETHOD (GetWidth) (int& retval);
  ///
  STDMETHOD (GetHeight) (int& retval);
  ///
  STDMETHOD (GetMipmapLevel) (int& retval);
  ///
  STDMETHOD (GetShiftU) (int& retval);
  ///
  STDMETHOD (GetSize) (long& retval);

  ///
  STDMETHOD (GetTCacheData) (void** retval);
  ///
  STDMETHOD (SetTCacheData) (void* newval);

  ///
  STDMETHOD (GetNumPixels) (int& retval);
  ///
  STDMETHOD (GetMipMapSize) (int& retval);
  ///
  STDMETHOD (GetMipMapShift) (int& retval);
  ///
  STDMETHOD (GetIMinU) (int& retval);
  ///
  STDMETHOD (GetIMinV) (int& retval);
  ///
  STDMETHOD (GetTextureBox) (float& fMinU, float& fMinV, float& fMaxU, float& fMaxV);
  ///
  STDMETHOD (GetOriginalWidth) (int& retval);

  ///
  STDMETHOD (GetPolygon) (IPolygon3D** retval);
  ///
  STDMETHOD (RecalculateDynamicLights) (bool& recalc);
  ///
  STDMETHOD (CreateDirtyMatrix) ();
  ///
  STDMETHOD (MakeAllDirty) ();
  ///
  STDMETHOD (CleanIfDirty) (int lu, int lv, bool& retval);

  /// 
  STDMETHOD (GetLightMap) (ILightMap** retval);

  /// 
  STDMETHOD (GetNumberDirtySubTex) (int& nDirty);
  ///
  STDMETHOD (GetSubtexSize) (int& retval);
  ///
  STDMETHOD (GetDynlightOpt) (bool& retval);

  DECLARE_IUNKNOWN ()
};

#endif
