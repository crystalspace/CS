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

#include "csutil/scf.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "iplugin.h"
#include "ipolygon.h"
#include "icamera.h"
#include "glcache.h"
#include "gl_txtmgr.h"
#include "cs3d/common/dtmesh.h"
#include "gl_halo.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"

class csGlideTextureCache;

/// the Glide implementation of the Graphics3D class.
class csGraphics3DGlide2x  : public iGraphics3D
{
friend class csGlideHalo;
private:
  /// the texture cache.
  csGlideTextureCache *m_pTextureCache, *m_pLightmapCache, *m_pAlphamapCache;
  /// texturehandler for FX polygon drawing
  TextureHandler *m_thTex;
  /// vertex array for FX polygon drawing
  GrVertex *m_verts, *m_dpverts;
  int m_vertsize, m_dpvertsize;
  int  poly_alpha;
  bool poly_fog;

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

  /// The world driver
  //IWorld* m_piWorld;

  /// width 'n' height
  int m_nWidth;
  int m_nHeight;
  int m_nHalfWidth;
  int m_nHalfHeight;
  int m_nFrameWidth;
  int m_nFrameHeight;

  /**
   * current render state
   */
  struct
  {
    bool dither;
    bool specular;
    bool bilinearmap;
    bool trilinearmap;
    bool gouraud;
    bool textured;
    bool alphablend;
    bool lighting;
    int  mipmap;
  } m_renderstate;

  /**
   * the effects currently in effect by DrawPolygonFX
   */
  struct
  {
    bool  gouraud;
    bool  textured;
    UInt  mixmode;
    UInt  alpha;
  } m_dpfx;
  
  /// use 16 bit texture else 8 bit
  bool use16BitTexture;

  /// use halo effect
  bool m_bHaloEffect;

  /// Our private config file
  csIniFile *config;

  /// fogtable
  GrFog_t fogtable[ GR_FOG_TABLE_SIZE];

  /// State of GlideEngine
  GrState state;
    
  /// aspect
  float aspect, inv_aspect;
  /// object -> camera transformation
  csTransform o2c;
  /// current clipper
  csClipper* clipper;

  void ClearBufferUnderTop();  
public:
  DECLARE_IBASE;

  /// The constructor. Pass all arguments to this.
  csGraphics3DGlide2x (iBase* iParent);
  /// the destructor.
  virtual ~csGraphics3DGlide2x ();
  
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
  virtual void DrawPolygonDebug(G3DPolygonDP& /*poly*/) { }

  /// Draw a Line.
  virtual void DrawLine (csVector3& v1, csVector3& v2, float fov, int color);
 
  /// Start a series of DrawPolygonFX
  virtual void StartPolygonFX (iTextureHandle* handle, UInt mode);

  /// Finish a series of DrawPolygonFX
  virtual void FinishPolygonFX ();

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Give a texture to Graphics3D to cache it.
  void CacheTexture (iPolygonTexture *piPT);
  
  /// Dump the texture cache.
  virtual void DumpCache ();
  
  /// Clear the texture cache.
  virtual void ClearCache ();

  /// Print the screen.
  virtual void Print (csRect* rect);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  
  /// Get the capabilities of this driver: NOT IMPLEMENTED.
  virtual void GetCaps (G3D_CAPS *caps);

  /// Get a render state
  virtual long GetRenderState (G3D_RENDERSTATEOPTION);

  /// Get a z-buffer point
  virtual unsigned long *GetZBufPoint (int, int);

  /// Get the width
  virtual int GetWidth () { return m_nWidth; }
  ///
  virtual int GetHeight () { return m_nHeight; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);
  /// Set perspective aspect
  virtual void SetPerspectiveAspect (float aspect) 
  {
    this->aspect = aspect; inv_aspect = 1.0/aspect;	 
  }
  /// Set world to camera transformation.
  virtual void SetObjectToCamera (csTransform* o2c) 
  { 
    this->o2c = *o2c;
  }
  /// Set optional clipper
  virtual void SetClipper (csVector2* vertices, int num_vertices);
  /// Draw a triangle mesh.
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh) 
  { 
    DefaultDrawTriangleMesh( mesh, this, o2c, clipper, aspect, m_nHalfWidth, m_nHalfHeight );
  }

  ///
  virtual void AdjustToOptimalTextureSize (int& w, int& h) { (void)w; (void)h; }
  ///
  virtual bool NeedsPO2Maps () { return true; }
  ///
  virtual int GetMaximumAspectRatio () { return 8; }

  /// 
  virtual iGraphics2D *GetDriver2D () 
  { return m_piG2D; }
  
  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return txtmgr; }

  /// Set the camera object.
  virtual void SetCamera (iCamera *pCamera)
  { m_pCamera =  pCamera; }

  /// Get the fog mode.
  virtual G3D_FOGMETHOD GetFogMode ()
  { return G3DFOGMETHOD_VERTEX; }

  /// Get the fog mode.
  virtual bool SetFogMode (G3D_FOGMETHOD fogm)
  { return (fogm == G3DFOGMETHOD_VERTEX); }

  virtual void OpenFogObject (CS_ID id, csFog* fog);
  virtual void AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype);
  virtual void CloseFogObject (CS_ID id);

  virtual iHalo *CreateHalo(float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, int iHeight );

  virtual float GetZbuffValue( int x, int y ) { return (float)(2<<16); }

private:
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
};

#endif // G3D_GLIDE_H
