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
#include "isys/plugin.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iengine/camera.h"
#include "iengine/campos.h"
#include "imesh/object.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/lightmap.h"
#include "imap/parser.h"
#include "plugins/cscript/cspython/cspython.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"
#include "ivideo/halo.h"

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
  ~csVector3();
};

struct csRGBpixel
{
  unsigned char red, green, blue, alpha;
};

struct csPixelFormat
{
  unsigned long RedMask, GreenMask, BlueMask;
  int RedShift, GreenShift, BlueShift;
  int RedBits, GreenBits, BlueBits;
  int PalEntries;
  int PixelBytes;
};

enum G3D_FOGMETHOD
{
};

struct csGraphics3DCaps
{
  bool CanClip;
  int minTexHeight, minTexWidth;
  int maxTexHeight, maxTexWidth;
  G3D_FOGMETHOD fog;
  bool NeedsPO2Maps;
  int MaxAspectRatio;
};

struct csImageArea
{
  int x, y, w, h;
  char *data;

  csImageArea (int sx, int sy, int sw, int sh);
};

//***** Interfaces
struct iPlugin:public iBase
{
  bool Initialize(iObjectRegistry *object_reg);
  bool HandleEvent(iEvent&);
};

struct iTextureWrapper : public iBase
{
};

struct iTextureHandle : public iBase
{
  bool GetMipMapDimensions (int mipmap, int &mw, int &mh);
  void GetMeanColor (unsigned char &red, unsigned char &green, unsigned char &blue);
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

struct iFont : public iBase
{
  virtual void SetSize (int iSize) = 0;
  virtual int GetSize () = 0;
  virtual void GetMaxSize (int &oW, int &oH) = 0;
  virtual bool GetGlyphSize (unsigned char c, int &oW, int &oH) = 0;
  virtual void *GetGlyphBitmap (unsigned char c, int &oW, int &oH) = 0;
  virtual void GetDimensions (const char *text, int &oW, int &oH) = 0;
  virtual int GetLength (const char *text, int maxwidth) = 0;
};


struct iFontServer : public iBase
{
  virtual iFont *LoadFont (const char *filename) = 0;
  virtual int GetFontCount () = 0;
  virtual iFont *GetFont (int iIndex) = 0;
};

struct iGraphics2D : public iBase
{
  virtual bool Open () = 0;
  virtual void Close () = 0;
  virtual int GetWidth () = 0;
  virtual int GetHeight () = 0;
  virtual bool GetFullScreen () = 0;
  virtual int GetPage () = 0;
  virtual bool DoubleBuffer (bool Enable) = 0;
  virtual bool GetDoubleBufferState () = 0;
  virtual csPixelFormat *GetPixelFormat () = 0;
  virtual int GetPixelBytes () = 0;
  virtual int GetPalEntryCount () = 0;
  virtual csRGBpixel *GetPalette () = 0;
  virtual void SetRGB (int i, int r, int g, int b) = 0;
  virtual void SetClipRect (int nMinX, int nMinY, int nMaxX, int nMaxY) = 0;
  virtual void GetClipRect(int& nMinX, int& nMinY, int& nMaxX, int& nMaxY) = 0;
  virtual bool BeginDraw () = 0;
  virtual void FinishDraw () = 0;
  virtual void Print (csRect *pArea) = 0;
  virtual void Clear (int color) = 0;
  virtual void ClearAll (int color) = 0;
  virtual void DrawLine(float x1, float y1, float x2, float y2, int color) = 0;
  virtual void DrawBox (int x, int y, int w, int h, int color) = 0;
  virtual bool ClipLine (float& x1, float& y1, float& x2, float& y2,
    int xmin, int ymin, int xmax, int ymax) = 0;
  virtual void DrawPixel (int x, int y, int color) = 0;
  virtual void *GetPixelAt (int x, int y) = 0;
  virtual void GetPixel (int x, int y, unsigned char &oR, unsigned char &oG, unsigned char &oB) = 0;
  virtual csImageArea *SaveArea (int x, int y, int w, int h) = 0;
  virtual void RestoreArea (csImageArea *Area, bool Free) = 0;
  virtual void FreeArea (csImageArea *Area) = 0;
  virtual void Write (iFont *font, int x, int y, int fg, int bg,
    const char *str) = 0;
  virtual iFontServer *GetFontServer () = 0;
  virtual bool SetMousePosition (int x, int y) = 0;
  virtual bool SetMouseCursor (csMouseCursorID iShape) = 0;
//  virtual bool PerformExtension (char const* command, ...) = 0;
//  virtual bool PerformExtensionV (char const* command, va_list) = 0;
  virtual iImage *ScreenShot () = 0;
  virtual iGraphics2D *CreateOffScreenCanvas (int width, int height,
     void *buffer, bool alone_hint, csPixelFormat *ipfmt,
     csRGBpixel *palette = NULL, int pal_size = 0) = 0;
  virtual void AllowResize (bool iAllow) = 0;
};

struct iHalo : public iBase
{
  virtual int GetWidth () = 0;
  virtual int GetHeight () = 0;
  virtual void SetColor (float &iR, float &iG, float &iB) = 0;
  virtual void GetColor (float &oR, float &oG, float &oB) = 0;
  virtual void Draw (float x, float y, float w, float h, float iIntensity,
    csVector2 *iVertices, int iVertCount) = 0;
};

struct iGraphics3D:public iBase
{
  bool Open ();
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
  void *GetZBuffAt (int x, int y);
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

struct iLightMap : public iBase
{
  virtual void *GetMapData () = 0;
  virtual int GetWidth () = 0;
  virtual int GetHeight () = 0;
  virtual int GetRealWidth () = 0;
  virtual int GetRealHeight () = 0;
  virtual void *GetCacheData () = 0;
  virtual void SetCacheData (void *d) = 0;
  virtual void GetMeanLighting (int& r, int& g, int& b) = 0;
  virtual long GetSize () = 0;
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
  void *GetAlpha ();
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

struct iSectorList : public iBase
{
  virtual int GetSectorCount () = 0;
  virtual iSector *GetSector (int idx) = 0;
  virtual void AddSector (iSector *sec) = 0;
  virtual void RemoveSector (iSector *sec) = 0;
  virtual iSector *FindByName (const char *name) = 0;
};

struct iEngine : public iBase
{
  virtual int GetTextureFormat () = 0;
  virtual void DeleteAll () = 0;
  virtual iTextureWrapper* CreateTexture (const char *iName,
  	const char *iFileName, csColor *iTransp, int iFlags) = 0;
  virtual iCameraPosition* CreateCameraPosition (const char *iName,
  	const char *iSector, const csVector3 &iPos, const csVector3 &iForward,
    const csVector3 &iUpward) = 0;
  virtual bool CreatePlane (const char *iName, const csVector3 &iOrigin,
    const csMatrix3 &iMatrix) = 0;
  virtual iSector *CreateSector (const char *iName, bool link = true) = 0;
  virtual iMaterialWrapper *FindMaterial (const char *iName,
  	bool regionOnly = false) = 0;
  virtual iMeshWrapper* CreateSectorWallsMesh (iSector* sector,
      const char* name) = 0;
  virtual iSectorList *GetSectors () = 0;
};

//****** System Interface
struct iObjectRegistry:public iBase
{
public:
  %addmethods
  {
    iEngine* Query_iEngine()
    {
      iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (self,
      	iPluginManager);
      return CS_QUERY_PLUGIN(plugin_mgr, iEngine);
    }
    iGraphics3D* Query_iGraphics3D()
    {
      iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (self,
      	iPluginManager);
      return CS_QUERY_PLUGIN(plugin_mgr, iGraphics3D);
    }
    void Print(int mode, const char* format) {
      printf (format); 
    }
  }
};

