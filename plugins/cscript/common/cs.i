/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

//-----------------------------------------------------------------------------
// A Swig interface definition which provides access to many classes within
// the Crystal Space engine.
//-----------------------------------------------------------------------------

%module cspace
%{
  #include "css.h"
//***** SCF Wrappers
  int MakeVersion(int version0, int version1, int version2)
  {
    return SCF_CONSTRUCT_VERSION(version0, version1, version2);
  }

#include "isys/system.h"
#include "iengine/camera.h"
#include "iengine/campos.h"
#include "imesh/object.h"
#include "imesh/thing/thing.h"
#include "imap/parser.h"
#include "plugins/cscript/cspython/cspython.h"
iSystem* GetSystem()
{
  return thisclass->Sys;
}

void* GetMyPtr() { return NULL; }
%}

%include pointer.i

//***** SCF
struct iBase
{
  void DecRef ();
};

struct iSCF:public iBase
{
  void* CreateInstance(const char *iClassID, const char *iInterfaceID,
    int iVersion);
};
int MakeVersion(int version0, int version1, int version2);

//***** Classes
class csVector3
{
public:
  float x,y,z;
  csVector3(float x, float y, float z);
};

struct csRGBpixel
{
  unsigned char red, green, blue, alpha;
};

//***** Interfaces
struct iPlugIn:public iBase
{
  bool Initialize(iSystem *iSys);
  bool HandleEvent(iEvent&);
};

struct iTextureWrapper : public iBase
{
};

struct iTextureHandle : public iBase
{
  bool GetMipMapDimensions (int mipmap, int &mw, int &mh);
  void GetMeanColor (UByte &red, UByte &green, UByte &blue);
  void *GetCacheData ();
  void SetCacheData (void *d);
  void *GetPrivateObject ();
};

struct iMaterialHandle : public iBase
{
  virtual iTextureHandle *GetTexture () = 0;
  virtual void GetFlatColor (csRGBpixel &oColor) = 0;
  virtual void GetReflection (float &oDiffuse, float &oAmbient, float &oReflection) = 0;
  virtual void Prepare () = 0;
};

struct iMaterialWrapper : public iBase
{
public:
  virtual iMaterialHandle* GetMaterialHandle () = 0;
};

struct iGraphics3D:public iPlugIn
{
  bool Initialize (iSystem *pSystem);
  bool Open (const char *Title);
  void Close ();
  void SetDimensions (int width, int height);
  bool BeginDraw (int DrawFlags);
  void FinishDraw ();
  void Print (csRect *area);
  void DrawPolygon (G3DPolygonDP& poly);
  void DrawPolygonDebug (G3DPolygonDP& poly);
  void DrawLine (const csVector3& v1, const csVector3& v2,
    float fov, int color);
//void DrawPolygonFX (G3DPolygonDPFX& poly);
  void DrawTriangleMesh (G3DTriangleMesh& mesh);
  void DrawPolygonMesh (G3DPolygonMesh& mesh);
  void OpenFogObject (CS_ID id, csFog* fog);
  void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype);
  void CloseFogObject (CS_ID id);
//bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
//long GetRenderState (G3D_RENDERSTATEOPTION op);
  csGraphics3DCaps *GetCaps ();
  unsigned long *GetZBuffAt (int x, int y);
  float GetZBuffValue (int x, int y);
  void DumpCache ();
  void ClearCache ();
  void RemoveFromCache (iPolygonTexture* poly_texture);
  int GetWidth ();
  int GetHeight ();
  void SetPerspectiveCenter (int x, int y);
  void SetPerspectiveAspect (float aspect);
  void SetObjectToCamera (csReversibleTransform* o2c);
  iGraphics2D *GetDriver2D ();
  iTextureManager *GetTextureManager ();
  iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char *iAlpha, int iWidth, int iHeight);
  void DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th);
};

struct iCamera:public iBase
{
  float GetFOV ();
  float GetInvFOV ();
};

struct iSector : public iBase
{
};

struct iThingState : public iBase
{
  iPolygon3D* CreatePolygon (const char* name);
};

struct iMeshObject : public iBase
{
  %addmethods
  {
    iThingState* Query_iThingState()
    {
      return SCF_QUERY_INTERFACE(self, iThingState);
    }
  }
};

struct iMeshWrapper : public iBase
{
  virtual iMeshObject* GetMeshObject ();
};

struct iPolygon3D : public iBase
{
  iLightMap *GetLightMap ();
  iMaterialHandle *GetMaterialHandle ();
  void SetMaterial (iMaterialWrapper* material);
//iPolygonTexture *GetTexture ();
//iTextureHandle *GetTextureHandle ();
  int GetVertexCount ();
  csVector3 &GetVertex (int idx);
  csVector3 &GetVertexW (int idx);
  csVector3 &GetVertexC (int idx);
  %name(CreateVertexByIndex) int CreateVertex (int idx);
  int CreateVertex (const csVector3 &iVertex);
  int GetAlpha ();
  void SetAlpha (int iAlpha);
  void CreatePlane (const csVector3 &iOrigin,
    const csMatrix3 &iMatrix);
  bool SetPlane (const char *iName);
  void SetTextureSpace (csVector3& v_orig, csVector3& v1, float len1);
};

struct iImage : public iBase
{
  void *GetImageData ();
  int GetWidth ();
  int GetHeight ();
  int GetSize ();
  void Rescale (int NewWidth, int NewHeight);
  iImage *MipMap (int step, csRGBpixel *transp);
  void SetName (const char *iName);
  const char *GetName ();
  int GetFormat ();
  csRGBpixel *GetPalette ();
  UByte *GetAlpha ();
  void SetFormat (int iFormat);
  iImage *Clone ();
  iImage *Crop (int x, int y, int width, int height);
};

struct iTextureManager : public iBase
{
  iTextureHandle *RegisterTexture (iImage *image, int flags);
  void PrepareTextures ();
  void FreeImages ();
  void ResetPalette ();
  void ReserveColor (int r, int g, int b);
  int FindRGB (int r, int g, int b);
  void SetPalette ();
  void SetVerbose (bool vb);
  int GetTextureFormat ();
};

struct iPolygonTexture : public iBase
{
  iMaterialHandle *GetMaterialHandle ();
//iTextureHandle *GetTextureHandle ();
  float GetFDU ();
  float GetFDV ();
  int GetWidth ();
  int GetHeight ();
  int GetShiftU ();
  int GetIMinU ();
  int GetIMinV ();
  void GetTextureBox (float& fMinU, float& fMinV,
    float& fMaxU, float& fMaxV);
  int GetOriginalWidth ();
  iPolygon3D *GetPolygon ();
  bool DynamicLightsDirty ();
  bool RecalculateDynamicLights ();
  iLightMap *GetLightMap ();
  int GetLightCellSize ();
  int GetLightCellShift ();
  void *GetCacheData (int idx);
  void SetCacheData (int idx, void *d);
};

struct iCameraPosition : public iBase
{
};

struct iEngine : public iPlugIn
{
  virtual int GetTextureFormat () = 0;
  virtual void SelectLibrary (const char *iName) = 0;
  virtual bool DeleteLibrary (const char *iName) = 0;
  virtual void DeleteAll () = 0;
  virtual iTextureWrapper* CreateTexture (const char *iName, const char *iFileName, 
    csColor *iTransp, int iFlags) = 0;
  virtual iCameraPosition* CreateCameraPosition (const char *iName, const char *iSector,
    const csVector3 &iPos, const csVector3 &iForward,
    const csVector3 &iUpward) = 0;
  virtual bool CreatePlane (const char *iName, const csVector3 &iOrigin,
    const csMatrix3 &iMatrix) = 0;
  virtual iSector *CreateSector (const char *iName, bool link = true) = 0;
  virtual iSector *FindSector (const char *iName) = 0;
  virtual iSector *GetSector (int idx) = 0;
  virtual iMaterialWrapper *FindMaterial (const char *iName, bool regionOnly = false) = 0;
  virtual iMeshWrapper* CreateSectorWallsMesh (iSector* sector,
      const char* name) = 0;
};

//****** System Interface
struct iSystem:public iBase
{
public:
  %addmethods
  {
    iEngine* Query_iEngine()
    {
      return CS_QUERY_PLUGIN(self, iEngine);
    }
    iGraphics3D* Query_iGraphics3D()
    {
      return CS_QUERY_PLUGIN(self, iGraphics3D);
    }
    void Print(int mode, const char* format) {
      self->Printf(mode, format); 
    }	
  }
};

iSystem* GetSystem();
void* GetMyPtr();
