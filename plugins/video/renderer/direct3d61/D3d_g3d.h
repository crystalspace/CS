/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles

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

// g3d_d3d.h
// csGraphics3DDirect3DDx6 Class Declaration
// Written by dan
// Some modifications by Nathaniel

// Ported to COM by Dan Ogles on 8.26.98
// modiefied by TristanMcLure  09/08/1999

#ifndef G3D_D3D_H
#define G3D_D3D_H

#include <windows.h>
#include "ddraw.h"
#include "d3d.h"
#include "d3dcaps.h"

#include "cs3d/direct3d61/d3d_txtcache.h"
#include "cs3d/direct3d61/d3d_txtmgr.h"
#include "cscom/com.h"
#include "cssys/win32/IDDetect.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "ihalo.h"
#include "ipolygon.h"

//DIRECT3D DRIVER HRESULTS/////////////////////////////

#define CSD3DERR_ERRORSTARTINGDIRECTDRAW    MAKE_CSHRESULT(0x0000)
#define CSD3DERR_WRONGVERSION               MAKE_CSHRESULT(0x5001)
#define CSD3DERR_NOPRIMARYSURFACE           MAKE_CSHRESULT(0x5002)
#define CSD3DERR_NOZBUFFER                  MAKE_CSHRESULT(0x5003)
#define CSD3DERR_CANNOTATTACH               MAKE_CSHRESULT(0x5004)
#define CSD3DERR_NOCOMPATIBLESURFACEDESC    MAKE_CSHRESULT(0x5005)
#define CSD3DERR_ERRORCREATINGMATERIAL      MAKE_CSHRESULT(0x5006)
#define CSD3DERR_ERRORCREATINGVIEWPORT      MAKE_CSHRESULT(0x5007)
#define CSD3DERR_ERRORCLEARINGBUFFER        MAKE_CSHRESULT(0x5008)
#define CSD3DERR_ERRORBEGINSCENE            MAKE_CSHRESULT(0x5009)
#define CSD3DERR_ERRORENDSCENE              MAKE_CSHRESULT(0x500A)
#define CSD3DERR_INTERNALERROR              MAKE_CSHRESULT(0x000B)

//DIRECT3D SPECIFIC STRUCTS///////////////////////////

class D3DTextureCache;
class D3DLightMapCache;

/// the Direct3D implementation of the Graphics3D class.
class csGraphics3DDirect3DDx6 : public IGraphics3D,
			     public IHaloRasterizer
{
  /// Pointer to DirectDraw class
  IDirectDraw4* m_lpDD4;
  /// Primary Surface
  IDirectDrawSurface4* m_lpddPrimary;
  /// Offscreen Surface
  IDirectDrawSurface4* m_lpddDevice;
  
  /// Direct3D class
  IDirect3D3* m_lpD3D;
  /// ZBuffer surface
  IDirectDrawSurface4* m_lpddZBuffer;
  /// D3D Device
  IDirect3DDevice2* m_lpd3dDevice;
  IDirect3DDevice3* m_lpd3dDevice2;
  /// D3d Viewport
  IDirect3DViewport3* m_lpd3dViewport;
  /// D3d Background material.
  IDirect3DMaterial3* m_lpd3dBackMat;
  /// D3D handle to the background material.
  D3DMATERIALHANDLE m_hd3dBackMat;
  
  /// whether this is a hardware device.
  bool m_bIsHardware;
  /// the bit-depth of this device.
  DWORD m_dwDeviceBitDepth;
  /// the globally unique identifier for this device.
  GUID m_Guid;
  
  /// The texture manager
  csTextureManagerDirect3D* txtmgr;

  /// the texture cache.
  D3DTextureCache* m_pTextureCache;
  /// the lightmap cache.
  D3DLightMapCache* m_pLightmapCache;
  
  /// supported lightmap type
  int m_iTypeLightmap;
  /// support halo.
  bool m_bHaloEffect;
  
  /// Dimensions of viewport.
  int m_nWidth,  m_nHeight;
  
  /// Half-dimensions of viewport.
  int m_nHalfWidth,  m_nHalfHeight;
  
  /// The current read/write settings for the Z-buffer.
  G3DZBufMode m_ZBufMode;
  
  /// The current drawing mode (2D/3D)
  int m_nDrawMode;

  /// Option variables: mipmap distances
  float zdist_mipmap1, zdist_mipmap2, zdist_mipmap3;

  /**
   * DAN: render-states
   * these override any other variable settings.
   */
  bool rstate_dither;
  bool rstate_specular;
  bool rstate_bilinearmap;
  bool rstate_trilinearmap;
  bool rstate_gouraud;
  bool rstate_flat;
  bool rstate_alphablend;
  int  rstate_mipmap;

  bool  m_gouraud;
  UInt  m_mixmode;
  float m_alpha;

  /// Capabilities of the renderer.
  G3D_CAPS m_Caps;

  //Maximum possible aspect ratio for the renderer.
  int m_MaxAspectRatio;
  
  /// the 2d graphics driver.
  IGraphics2D* m_piG2D;
  
  /// The directdraw device description
  IDirectDetectionInternal* m_pDirectDevice;
  
  /// The system driver
  ISystem* m_piSystem;

  /// Verbose mode
  bool m_bVerbose;
public:
  
  static DDSURFACEDESC2 m_ddsdLightmapSurfDesc;
  static DDSURFACEDESC2 m_ddsdTextureSurfDesc;
  static DDSURFACEDESC2 m_ddsdHaloSurfDesc;
  
  /// The constructor. It is passed an interface to the system using it.
  csGraphics3DDirect3DDx6 (ISystem*);
  /// the destructor.
  ~csGraphics3DDirect3DDx6 ();
  
  ///
  STDMETHODIMP Initialize ();

  /// opens Direct3D.
  STDMETHODIMP Open(char* Title);
  /// closes Direct3D.
  STDMETHODIMP Close();
  
  /// Change the dimensions of the display.
  STDMETHODIMP SetDimensions (int width, int height);
  
  /// Start a new frame.
  STDMETHODIMP BeginDraw (int DrawFlags);
  
  /// End the frame and do a page swap.
  STDMETHODIMP FinishDraw ();
  
  /// Set the mode for the Z buffer (functionality also exists in SetRenderState).
  STDMETHODIMP SetZBufMode (G3DZBufMode mode);
  
  /// Draw the projected polygon with light and texture.
  STDMETHODIMP DrawPolygon (G3DPolygonDP& poly);
  /// Draw debug poly
  STDMETHODIMP DrawPolygonDebug(G3DPolygonDP& /*poly*/)   { return E_NOTIMPL; }
  
  /// Draw a Line.
  STDMETHODIMP DrawLine (csVector3& v1, csVector3& v2, float fov, int color);
  
  /// Start a series of DrawPolygonFX
  STDMETHODIMP StartPolygonFX (ITextureHandle* handle, UInt mode);

  /// Finish a series of DrawPolygonFX
  STDMETHODIMP FinishPolygonFX ();

  /// Draw a polygon wit special effects.
  STDMETHODIMP DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Draw a projected floating light on the screen.
  STDMETHODIMP DrawFltLight(G3DFltLight& light);
  
  /// Give a texture to Graphics3D to cache it.
  STDMETHODIMP CacheTexture (IPolygonTexture* texture);
  
  /// Release a texture from the cache.
  STDMETHODIMP UncacheTexture (IPolygonTexture* texture);
  
  /// Get the capabilities of this driver: NOT IMPLEMENTED.
  STDMETHODIMP GetCaps(G3D_CAPS *caps);
  
  /// Dump the texture cache.
  STDMETHODIMP DumpCache(void);
  
  /// Clear the texture cache.
  STDMETHODIMP ClearCache(void);
  ///
  STDMETHODIMP GetColormapFormat( G3D_COLORMAPFORMAT& g3dFormat );

  ///
  STDMETHODIMP GetStringError( HRESULT hRes, char* szErrorString );
  
  /// Print the screen.
  STDMETHODIMP Print(csRect* rect) {  return m_piG2D->Print(rect);  }
  
  /// Set a render state
  STDMETHODIMP SetRenderState(G3D_RENDERSTATEOPTION, long);
  /// Get a render state
  STDMETHODIMP GetRenderState(G3D_RENDERSTATEOPTION, long& nValue);
  
  /// Get a z-buffer point
  STDMETHODIMP GetZBufPoint(int, int, unsigned long** retval) { *retval = NULL; return E_NOTIMPL; }
  
  /// Get the width
  STDMETHODIMP GetWidth(int& nWidth) { nWidth = m_nWidth; return S_OK; }
  ///
  STDMETHODIMP GetHeight(int& nHeight) { nHeight = m_nHeight; return S_OK; }
  /// Set center of projection.
  STDMETHODIMP SetPerspectiveCenter (int x, int y);

  ///
  STDMETHODIMP NeedsPO2Maps(void) { return S_OK; }
  ///
  STDMETHODIMP GetMaximumAspectRatio(int& ratio) { ratio = m_MaxAspectRatio; return S_OK; }
  
  /// Get the fog mode.
  STDMETHODIMP GetFogMode (G3D_FOGMETHOD& retval) { retval = G3DFOGMETHOD_VERTEX; return S_OK; }

  /// Get the fog mode.
  STDMETHODIMP SetFogMode (G3D_FOGMETHOD fogm) { if (fogm == G3DFOGMETHOD_VERTEX) return S_OK; else return E_FAIL; }

  /// Get the ITextureManager.
  STDMETHODIMP GetTextureManager (ITextureManager** pi) { *pi = (ITextureManager*)txtmgr; return S_OK; }

  /// 
  STDMETHODIMP Get2dDriver(IGraphics2D** pG2D) 
  { 
    ASSERT(m_piG2D);
    
    m_piG2D->AddRef(); 
    *pG2D = m_piG2D; 
    return S_OK; 
  }
  
  STDMETHODIMP OpenFogObject (CS_ID id, csFog* fog);
  STDMETHODIMP AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype);
  STDMETHODIMP CloseFogObject (CS_ID id);

  STDMETHODIMP CreateHalo(float r, float g, float b, HALOINFO* pRetVal);  
  STDMETHODIMP DestroyHalo(HALOINFO haloInfo);
  STDMETHODIMP DrawHalo(csVector3* pCenter, float fIntensity, HALOINFO haloInfo);
  STDMETHODIMP TestHalo(csVector3* pCenter);

  /// Our internal representation of halos.
  struct csG3DHardwareHaloInfo
  {
    LPDIRECTDRAWSURFACE4 lpsurf;		// texture data surface
    LPDIRECT3DTEXTURE2 lptex;		// texture interface
    D3DTEXTUREHANDLE htex;			// texture handle
  };

  /// Actually draws a halo the the screen.
  class csHaloDrawer
  {
  public:
    ///
    csHaloDrawer(IGraphics2D* piG2D, float r, float g, float b);
    ///
    ~csHaloDrawer();

    unsigned long* GetBuffer() { return mpBuffer; }
    
  private:

    /// the width and height of the graphics context
    int mWidth, mHeight;
    /// the 2D graphics context.
    IGraphics2D* mpiG2D;
    /// the size to be drawn (the diameter of the halo)
    int mDim;
    /// the color of the halo
    float mRed, mGreen, mBlue;
    /// the ratio of the color intensity vs the radius
    float mRatioRed, mRatioGreen, mRatioBlue;
    /// the center coords.
    int mx, my;
    /// the buffer.
    unsigned long* mpBuffer;
    /// the width of the buffer.
    int mBufferWidth;

    void drawline_vertical(int x, int y1, int y2);
    void drawline_outerrim(int x1, int x2, int y);
    void drawline_innerrim(int x1, int x2, int y);
  };
private:
  
  // print to the system's device
  void SysPrintf(int mode, char* str, ...);
  
  // texture format enumeration callback function :: static member function.
  static HRESULT CALLBACK EnumTextFormatsCallback(LPDDSURFACEDESC2 lpddsd, LPVOID lpUserArg);
  // Microsoft changed their API for DX6, now it is not DDSURFACEDESC2 but DDPIXELFORMAT ... stupid
  static HRESULT CALLBACK EnumPixelFormatsCallback(LPDDPIXELFORMAT lpddpf, LPVOID lpUserArg);
  /// used to get the appropriate device.
  bool EnumDevices(void);
  /// whether the device is locked or not.
  bool m_bIsLocked;
  
  /// used to set up polygon geometry before rasterization.
  inline void SetupPolygon( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
    float& K1, float& K2, float& K3,
    float& M,  float& N,  float& O  );
  
  DECLARE_INTERFACE_TABLE(csGraphics3DDirect3DDx6)
    DECLARE_IUNKNOWN()
};


class csGraphics3DDirect3DDx6Factory : public IGraphicsContextFactory
{
  /// Create the graphics context
  STDMETHODIMP CreateInstance( REFIID riid, ISystem* piSystem, void** ppv );
  
  /// Lock or unlock from memory.
  STDMETHODIMP LockServer(BOOL bLock);
  
  DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics3DDirect3DDx6Factory)
};

#endif // G3D_D3D_H
