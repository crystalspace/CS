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

// g3d_glide.h
// Graphics3DGlide Class Declaration
// Written by Nathaniel

#ifndef G3D_GLIDE_H
#define G3D_GLIDE_H

#include <glide.h>

#include "cscom/com.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "igraph2d.h"
#include "ipolygon.h"

#include "cs3d/glide3/glcache.h"

extern const CLSID CLSID_Glide3xGraphics3D;

class GlideTextureCache;
class GlideLightmapCache;

typedef struct {
  float  sow;                   /* s texture ordinate (s over w) */
  float  tow;                   /* t texture ordinate (t over w) */  
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */
}  MyGrTmuVertex;

typedef struct
{
  float x, y;         /* X and Y in screen space */
  float ooz;          /* 65535/Z (used for Z-buffering) */
  float oow;          /* 1/W (used for W-buffering, texturing) */
  float r, g, b, a;   /* R, G, B, A [0..255.0] */
  MyGrTmuVertex  tmuvtx[3]; // tmu unit (max three)
} MyGrVertex;

/// the Glide implementation of the Graphics3D class.
class csGraphics3DGlide3x : public IGraphics3D
{
private:
  /// the texture cache.
  GlideTextureCache *m_pTextureCache;
  /// the lightmap cache.
  GlideLightmapCache *m_pLightmapCache;

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

  /// width 'n' height
  int m_nWidth;
  int m_nHeight;
  int m_nHalfWidth;
  int m_nHalfHeight;
  int m_nFrameWidth;
  int m_nFrameHeight;

  /// use 16 bit texture else 8 bit
  bool use16BitTexture;

public:
  /// The constructor. Pass all arguments to this.
  csGraphics3DGlide3x (ISystem* piSystem);
  /// the destructor.
  ~csGraphics3DGlide3x ();
  
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
  STDMETHODIMP DrawPolygonDebug(G3DPolygonDP& poly)   { return E_NOTIMPL; }

  /// Draw a Line.
  STDMETHODIMP DrawLine (csVector3& v1, csVector3& v2, int color);
 
  /// Draw a projected (non-perspective correct) polygon.
  STDMETHODIMP DrawPolygonQuick (G3DPolygonDPQ& poly);

  /// Draw a projected floating light on the screen.
  STDMETHODIMP DrawFltLight(G3DFltLight& light);

  /// Give a texture to Graphics3D to cache it.
  STDMETHODIMP CacheTexture (IPolygonTexture *piPT);
  
  /// Release a texture from the cache.
  STDMETHODIMP UncacheTexture (IPolygonTexture *piPT);
      
  /// Set the texture cache size.
  STDMETHODIMP SetCacheSize (long size) { return S_OK; }
  
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
  STDMETHODIMP GetRenderState(G3D_RENDERSTATEOPTION, long& nValue) { nValue = 0; return E_NOTIMPL; }

  /// Get a z-buffer point
  STDMETHODIMP GetZBufPoint(int, int, unsigned long** retval) { *retval = NULL; return E_NOTIMPL; }

  /// Get the width
  STDMETHODIMP GetWidth(int& nWidth) { nWidth = m_nWidth; return S_OK; }
  ///
  STDMETHODIMP GetHeight(int& nHeight) { nHeight = m_nHeight; return S_OK; }
  ///
  STDMETHODIMP SetWorld(IWorld* piWorld) { return S_OK; }
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
  
  /// Set the camera object.
  STDMETHODIMP SetCamera( ICamera* pCamera )
  {
      m_pCamera = pCamera;
      return S_OK;
  }

  STDMETHODIMP OpenFogObject (CS_ID id, csFog* fog) { return E_NOTIMPL; }
  STDMETHODIMP AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype) { return E_NOTIMPL; }
  STDMETHODIMP CloseFogObject (CS_ID id) { return E_NOTIMPL; }

  STDMETHODIMP CreateHalo(float r, float g, float b, HALOINFO* pRetVal) { return E_NOTIMPL;};  
  STDMETHODIMP DestroyHalo(HALOINFO haloInfo) {return E_NOTIMPL;};
  STDMETHODIMP DrawHalo(csVector3* pCenter, float fIntensity, HALOINFO haloInfo) {return E_NOTIMPL;};
  STDMETHODIMP TestHalo(csVector3* pCenter) {return E_NOTIMPL;};

private:
  // print to the system's device
  void SysPrintf(int mode, char* str, ...);

  /// board selected
	int board;

  /// Graphic context
  GrContext_t grcontext;

  // w-buffer hardware limits : wLimits[0] is min, wLimits[1] is max
  FxU8 wLimits[2];

  /// Select the board
	int SelectBoard();

	/// Initialize Instance from Board Config
	void InitializeBoard();

	/// ptr to Rendering Function
	void (*RenderPolygon) (MyGrVertex*, int, bool,TextureHandler*,TextureHandler*);
	
  /// Rendering Function with MultiPass (One TMU)
	static void RenderPolygonMultiPass(MyGrVertex*, int, bool,TextureHandler*,TextureHandler*);
	/// Rendering Function with SinglePass (Two (or more) TMUs)
	static void RenderPolygonSinglePass(MyGrVertex*, int, bool,TextureHandler*,TextureHandler*);

  /// used to set up polygon geometry before rasterization.
  inline void SetupPolygon( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
                                                float& K1, float& K2, float& K3,
                                                float& M,  float& N,  float& O  );

  DECLARE_INTERFACE_TABLE(csGraphics3DGlide3x)
  DECLARE_IUNKNOWN()
};

class csGraphics3DGlide3xFactory : public IGraphicsContextFactory
{
    /// Create the graphics context
    STDMETHODIMP CreateInstance( REFIID riid, ISystem* piSystem, void** ppv );

    /// Lock or unlock from memory.
    STDMETHODIMP LockServer(COMBOOL bLock);

    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics3DGlide3xFactory)
};


#endif // G3D_GLIDE_H
