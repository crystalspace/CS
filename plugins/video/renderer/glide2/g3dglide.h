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

// g3d_glide.h
// Graphics3DGlide Class Declaration
// Written by xtrochu and Nathaniel

#ifndef G3D_GLIDE_H
#define G3D_GLIDE_H

#include <glide.h>

#include "cscom/com.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "ihalo.h"
#include "ipolygon.h"
#include "icamera.h"

#include "cs3d/glide2/glcache.h"
#include "cs3d/glide2/gl_txtmgr.h"

class GlideTextureCache;
class GlideLightmapCache;

/// the Glide implementation of the Graphics3D class.
class csGraphics3DGlide2x : public IGraphics3D,
			     public IHaloRasterizer
{
private:
  /// the texture cache.
  GlideTextureCache *m_pTextureCache;
  /// the lightmap cache.
  GlideLightmapCache *m_pLightmapCache;

  csTextureManagerGlide* txtmgr; 
  /// Does require multipass rendering for lightmap (if TRUE, then there is only one TMU!)
  bool m_iMultiPass;

  TMUInfo *m_TMUs;

  /// Number of TMU for TextureMap
  int iTMUTexture;
  /// Number of TMU for LightMap
  int iTMULightMap;
  /// Wait Vertical Retrace
  bool m_bVRetrace;

  /// The current read/write settings for the Z-buffer.
  ZBufMode m_ZBufMode;

  /// The current drawing mode (2D/3D)
  int m_nDrawMode;

  /// Capabilities of the renderer.
  G3D_CAPS m_Caps;
  
  /// The camera object.
  ICamera* m_pCamera;

  /// the 2d graphics driver.
  IGraphics2D* m_piG2D;

  /// The system driver
  ISystem* m_piSystem;

  /// The world driver
  //IWorld* m_piWorld;

  /// width 'n' height
  int m_nWidth;
  int m_nHeight;
  int m_nHalfWidth;
  int m_nHalfHeight;
  int m_nFrameWidth;
  int m_nFrameHeight;

  bool rstate_dither;
  bool rstate_specular;
  bool rstate_bilinearmap;
  bool rstate_trilinearmap;
  bool rstate_gouraud;
  bool rstate_flat;
  bool rstate_alphablend;
  int rstate_mipmap;

  bool        m_gouraud;
  DPFXMixMode m_mixmode;
  float       m_alpha;

  /// use 16 bit texture else 8 bit
  bool use16BitTexture;

  /// use halo effect
  bool m_bHaloEffect;

public:
  /// The constructor. Pass all arguments to this.
  csGraphics3DGlide2x (ISystem* piSystem);
  /// the destructor.
  virtual ~csGraphics3DGlide2x ();
  
  STDMETHODIMP Initialize();
  /// opens Glide.
  STDMETHODIMP Open(char* Title);
  /// closes Glide.
  STDMETHODIMP Close();
  
  /// Change the dimensions of the display.
  STDMETHODIMP SetDimensions (int width, int height);
  
  /// Start a new frame.
  STDMETHODIMP BeginDraw (int DrawFlags);
  
  /// End the frame and do a page swap.
  STDMETHODIMP FinishDraw ();
  
  /// Set the mode for the Z buffer (functionality also exists in SetRenderState).
  STDMETHODIMP SetZBufMode (ZBufMode mode);
 
  /// Draw the projected polygon with light and texture.
  STDMETHODIMP DrawPolygon (G3DPolygonDP& poly);
  /// Draw debug poly
  STDMETHODIMP DrawPolygonDebug(G3DPolygonDP& /*poly*/)   { return E_NOTIMPL; }

  /// Draw a Line.
  STDMETHODIMP DrawLine (csVector3& v1, csVector3& v2, float fov,  int color);
 
  /// Start DrawPolygonQuick drawing.
  STDMETHODIMP StartPolygonQuick (ITextureHandle* handle, bool gouraud);

  /// Finish DrawPolygonQuick drawing.
  STDMETHODIMP FinishPolygonQuick ();

  /// Draw a projected (non-perspective correct) polygon.
  STDMETHODIMP DrawPolygonQuick (G3DPolygonDPQ& poly);

  /// Start a series of DrawPolygonFX
  STDMETHODIMP StartPolygonFX(ITextureHandle* handle, DPFXMixMode mode, float alpha, bool gouraud);

  /// Finish a series of DrawPolygonFX
  STDMETHODIMP FinishPolygonFX();

  /// Draw a polygon with special effects.
  STDMETHODIMP DrawPolygonFX    (G3DPolygonDPFX& poly);

  /// Draw a projected floating light on the screen.
  STDMETHODIMP DrawFltLight(G3DFltLight& light);

  /// Give a texture to Graphics3D to cache it.
  STDMETHODIMP CacheTexture (IPolygonTexture *piPT);
  
  /// Release a texture from the cache.
  STDMETHODIMP UncacheTexture (IPolygonTexture *piPT);

  /// Set the texture cache size.
  STDMETHODIMP SetCacheSize (long /*size*/) { return S_OK; }

  /// Dump the texture cache.
  STDMETHODIMP DumpCache(void);
  
  /// Clear the texture cache.
  STDMETHODIMP ClearCache(void);

  STDMETHODIMP GetColormapFormat( G3D_COLORMAPFORMAT& g3dFormat ) ;

    /// Print the screen.
  STDMETHODIMP Print(csRect* rect);

  /// Set a renderstate boolean.
  STDMETHODIMP SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  
  /// Get the capabilities of this driver: NOT IMPLEMENTED.
  STDMETHODIMP GetCaps(G3D_CAPS *caps);

  /// Get a render state
  STDMETHODIMP GetRenderState(G3D_RENDERSTATEOPTION, long& nValue);

  /// Get a z-buffer point
  STDMETHODIMP GetZBufPoint(int, int, unsigned long** retval);

  /// Get the width
  STDMETHODIMP GetWidth(int& nWidth) { nWidth = m_nWidth; return S_OK; }
  ///
  STDMETHODIMP GetHeight(int& nHeight) { nHeight = m_nHeight; return S_OK; }
  ///
  //  STDMETHODIMP SetWorld(IWorld* piWorld) { piWorld = m_piWorld; return S_OK; }
  ///
  STDMETHODIMP NeedsPO2Maps(void) { return S_OK; }
  ///
  STDMETHODIMP GetMaximumAspectRatio(int& ratio) { ratio = 8; return S_OK; }

  /// 
  STDMETHODIMP Get2dDriver(IGraphics2D** pG2D) 
  { 
    ASSERT(m_piG2D);
    
    m_piG2D->AddRef(); 
    *pG2D = m_piG2D; 
    return S_OK; 
  }
  
  /// Get the ITextureManager.
  STDMETHODIMP GetTextureManager (ITextureManager** pi)
  {
    *pi = (ITextureManager*)txtmgr;
    return S_OK;
  }

  /// Set the camera object.
  STDMETHODIMP SetCamera( ICamera* pCamera )
  {
      m_pCamera = pCamera;
      return S_OK;
  }

  /// Get the fog mode.
  STDMETHODIMP GetFogMode (G3D_FOGMETHOD& retval) 
  { 
    retval = G3DFOGMETHOD_PLANES; return S_OK; 
  }

  /// Get the fog mode.
  STDMETHODIMP SetFogMode (G3D_FOGMETHOD fogm) 
  { 
    if (fogm == G3DFOGMETHOD_PLANES) return S_OK; 
    else return E_FAIL; 
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
    HighColorCacheAndManage_Data *halo;
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

public:
  //bool GetWorld(IWorld** piWorld) { *piWorld = m_piWorld; return S_OK; }
private:
  // print to the system's device
  void SysPrintf(int mode, char* str, ...);

  /// board selected
	int board;
	/// Select the board
	int SelectBoard(GrHwConfiguration &);
	/// Initialize Instance from Board Config
	void InitializeBoard(GrHwConfiguration &);
	/// ptr to Rendering Function
	void (*RenderPolygon)(GrVertex*, int, bool,TextureHandler*,TextureHandler*,bool);
	/// Rendering Function with MultiPass (One TMU)
	static void RenderPolygonMultiPass(GrVertex*, int, bool,TextureHandler*,TextureHandler*,bool);
	/// Rendering Function with SinglePass (Two (or more) TMUs)
	static void RenderPolygonSinglePass(GrVertex*, int, bool,TextureHandler*,TextureHandler*,bool);

  /// used to set up polygon geometry before rasterization.
  inline void SetupPolygon( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
                                                float& K1, float& K2, float& K3,
                                                float& M,  float& N,  float& O  );

  DECLARE_INTERFACE_TABLE(csGraphics3DGlide2x)
  DECLARE_IUNKNOWN()
};

class csGraphics3DGlide2xFactory : public IGraphicsContextFactory
{
    /// Create the graphics context
    STDMETHODIMP CreateInstance( REFIID riid, ISystem* piSystem, void** ppv );

    /// Lock or unlock from memory.
    STDMETHODIMP LockServer(BOOL bLock);

    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics3DGlide2xFactory)
};


#endif // G3D_GLIDE_H
