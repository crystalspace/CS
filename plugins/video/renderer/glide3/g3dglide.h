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

#include "csutil/scf.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "iplugin.h"
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
class csGraphics3DGlide3x : public iGraphics3D, public iHaloRasterizer
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
  G3DZBufMode m_ZBufMode;

  /// The current drawing mode (2D/3D)
  int m_nDrawMode;

  /// Capabilities of the renderer.
  G3D_CAPS m_Caps;
  
  /// The camera object.
  iCamera* m_pCamera;

  /// the 2d graphics driver.
  iGraphics2D* m_piG2D;

  /// The system driver
  iSystem* m_piSystem;

  /// width 'n' height
  int m_nWidth;
  int m_nHeight;
  int m_nHalfWidth;
  int m_nHalfHeight;
  int m_nFrameWidth;
  int m_nFrameHeight;

  /// use 16 bit texture else 8 bit
  bool use16BitTexture;

  /// Our private config file
  csIniFile *config;

public:
  DECLARE_IBASE;

  /// The constructor. Pass all arguments to this.
  csGraphics3DGlide3x (iBase*);
  /// the destructor.
  virtual ~csGraphics3DGlide3x ();

  ///
  virtual bool Initialize (iSystem *iSys);

  /// opens Glide.
  virtual bool Open(const char* Title);
  /// closes Glide.
  virtual void Close();
  
  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);
  
  /// Start a new frame.
  virtual bool BeginDraw (int DrawFlags);
  
  /// End the frame and do a page swap.
  virtual void FinishDraw ();
  
  /// Set the mode for the Z buffer (functionality also exists in SetRenderState).
  virtual void SetZBufMode (G3DZBufMode mode);
 
  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly);
  /// Draw debug poly
  virtual void DrawPolygonDebug(G3DPolygonDP& poly) { }

  /// Draw a Line.
  virtual void DrawLine (csVector3& v1, csVector3& v2, int color);
 
  /// Draw a projected (non-perspective correct) polygon.
  virtual void DrawPolygonQuick (G3DPolygonDPQ& poly);

  /// Give a texture to Graphics3D to cache it.
  virtual CacheTexture (iPolygonTexture *texture);
  
  /// Release a texture from the cache.
  virtual void UncacheTexture (iPolygonTexture *piPT);
      
  /// Dump the texture cache.
  virtual void DumpCache ();
  
  /// Clear the texture cache.
  virtual void ClearCache ();

  ///
  virtual G3D_COLORMAPFORMAT GetColormapFormat ();

  /// Print the screen.
  virtual void Print (csRect* rect);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  
  /// Get the capabilities of this driver: NOT IMPLEMENTED.
  virtual void GetCaps (G3D_CAPS *caps);

  /// Get a render state
  virtual long GetRenderState (G3D_RENDERSTATEOPTION)
  { return 0; }

  /// Get a z-buffer point
  virtual long *GetZBufPoint(int, int)
  { return NULL; }

  /// Get the width
  virtual int GetWidth () { return m_nWidth; }
  ///
  virtual int GetHeight () { return m_nHeight; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);

  ///
  virtual bool NeedsPO2Maps () { return true; }
  ///
  virtual int GetMaximumAspectRatio ()
  { return 8; }

  /// 
  virtual iGraphics2D *GetDriver2D () 
  { return m_piG2D; }
  
  /// Set the camera object.
  virtual void SetCamera (iCamera* pCamera)
  { m_pCamera = pCamera; }

  virtual void OpenFogObject (CS_ID id, csFog* fog) { }
  virtual void AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype) { }
  virtual void CloseFogObject (CS_ID id) { }

  virtual csHaloHandle CreateHalo (float iR, float iG, float iB,
    float iFactor, float iCross)
  { return NULL; }
  virtual void DestroyHalo (csHaloHandle iHalo)
  { }
  virtual bool DrawHalo (csVector3 *iCenter, float iIntensity, csHaloHandle iHalo)
  { }
  virtual bool TestHalo (csVector3 *iCenter)
  { }
  virtual void SetHaloClipper (csVector2 *iClipper, int iCount)
  { }

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
};

#endif // G3D_GLIDE_H
