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

#ifndef __CS_G3D_GLIDE_H__
#define __CS_G3D_GLIDE_H__

#include <glide.h>

#include "csutil/scf.h"
#include "csutil/cfgacc.h"
#include "ivideo/igraph3d.h"
#include "ivideo/ihalo.h"
#include "isys/iplugin.h"
#include "iengine/ipolygon.h"
#include "iengine/icamera.h"
#include "glcache.h"
#include "gltex.h"
#include "video/renderer/common/dtmesh.h"
#include "video/renderer/common/dpmesh.h"
#include "glhalo.h"
#include "csgeom/transfrm.h"
#include "csgeom/polyclip.h"
#include "video/canvas/glide2common/iglide2d.h"

class csGraphics3DGlide;
class csGlideTextureCache;
class csTextureManagerGlide; 

/// the Glide implementation of the Graphics3D class.
class csGraphics3DGlide  : public iGraphics3D
{
friend class csGlideHalo;
friend class csGlideProcedural;
private:
  /// the texture cache.
  csGlideTextureCache *m_pTextureCache, *m_pLightmapCache, *m_pAlphamapCache;
  /// texturehandler for FX polygon drawing
  TextureHandler *m_thTex;

#ifdef GLIDE3
  /// graphics context
  GrContext_t context;
  /// State of GlideEngine
  UByte *state;
#else
  int context;
  GrState *state;
#endif

  /// vertex array for FX polygon drawing
  MyGrVertex *m_verts, *m_dpverts;
  static long m_vertstrulen; // length of our VertexArray structur
  
  int m_vertsize, m_dpvertsize;
  int  poly_alpha;
  bool poly_fog;

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
  static long int m_ZBufMode;

  /// The current drawing mode (2D/3D)
  int m_nDrawMode;

  /// Capabilities of the renderer.
  csGraphics3DCaps m_Caps;
  
  /// The camera object.
  iCamera* m_pCamera;

  /// the 2d graphics driver.
  iGraphics2D* m_piG2D;
  /// Same seen as glide driver
  iGraphics2DGlide *m_piGlide2D;
  
  /// The system driver
  iSystem* m_piSystem;

  static csGraphics3DGlide *G3D;
  
  /// width 'n' height
  int m_nWidth;
  int m_nHeight;
  int m_nHalfWidth;
  int m_nHalfHeight;
  int m_nFrameWidth;
  int m_nFrameHeight;

  long m_wminmax[2];
  
  /**
   * current render state
   */
  struct
  {
    bool dither;
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
  csConfigAccess config;

  /// fogtable
  GrFog_t *fogtable;

  /// aspect
  float aspect, inv_aspect;
  /// object -> camera transformation
  csReversibleTransform o2c;
  /// current clipper
  csClipper* clipper;

  void ClearBufferUnderTop();  
public:
  /// The Glide texture manager
  csTextureManagerGlide* txtmgr; 

  DECLARE_IBASE;

  /// The constructor. Pass all arguments to this.
  csGraphics3DGlide (iBase* iParent);
  /// the destructor.
  virtual ~csGraphics3DGlide ();
  
  virtual bool Initialize (iSystem *iSys);

  /// clears the render buffer
  void ClearBuffer ();

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
  
  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly);
  /// Draw debug poly
  virtual void DrawPolygonDebug(G3DPolygonDP& /*poly*/) { }

  /// Draw a Line.
  virtual void DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color);
 
  /// Start a series of DrawPolygonFX
  virtual void StartPolygonFX (iMaterialHandle* handle, UInt mode);

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

  /// Remove some polygon from the cache.
  virtual void RemoveFromCache (iPolygonTexture* /*poly_texture*/) { }

  /// Print the screen.
  virtual void Print (csRect* rect);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);
  
  /// Get the capabilities of this driver
  virtual csGraphics3DCaps *GetCaps ()
  { return &m_Caps; }

  /// Get a render state
  virtual long GetRenderState (G3D_RENDERSTATEOPTION);

  /// Get a z-buffer point
  virtual unsigned long *GetZBuffAt (int, int)
  { return NULL; }

  /// Get the width
  virtual int GetWidth () { return m_nWidth; }
  ///
  virtual int GetHeight () { return m_nHeight; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);
  /// Get center of projection.
  virtual void GetPerspectiveCenter (int& x, int& y)
  {
    x = m_nHalfWidth;
    y = m_nHalfHeight;
  }
  /// Set perspective aspect
  virtual void SetPerspectiveAspect (float aspect) 
  {
    this->aspect = aspect; inv_aspect = 1.0/aspect;	 
  }
  /// Get perspective aspect
  virtual float GetPerspectiveAspect () 
  {
    return aspect;
  }
  /// Set world to camera transformation.
  virtual void SetObjectToCamera (csReversibleTransform* o2c) 
  { 
    this->o2c = *o2c;
  }
  /// Get world to camera transformation.
  virtual const csReversibleTransform& GetObjectToCamera () 
  {
    return o2c;
  }
  /// Set optional clipper
  virtual void SetClipper (csVector2* vertices, int num_vertices);
  /// Get optional clipper
  virtual void GetClipper (csVector2* vertices, int& num_vertices);
  /// Draw a triangle mesh.
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh) 
  { 
    DefaultDrawTriangleMesh( mesh, this, o2c, clipper, aspect, m_nHalfWidth, m_nHalfHeight );
  }

  /// Draw a polygon mesh.
  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh)
  {
    DefaultDrawPolygonMesh (mesh, this, o2c, clipper, aspect, inv_aspect, m_nHalfWidth, m_nHalfHeight );
  }

  /// 
  virtual iGraphics2D *GetDriver2D () 
  { return m_piG2D; }
  
  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return txtmgr; }

  /// Set the camera object.
  virtual void SetCamera (iCamera *pCamera)
  { m_pCamera =  pCamera; }

  virtual void OpenFogObject (CS_ID id, csFog* fog);
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype);
  virtual void CloseFogObject (CS_ID id);

  virtual iHalo *CreateHalo(float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, int iHeight );

  virtual float GetZBuffValue( int x, int y );

  /// Draw a 2D sprite
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw, int sh,
    int tx, int ty, int tw, int th, uint8 Alpha);

private:
  /// board selected
  int board;
  /// Select the board
  // int SelectBoard();
  /// Initialize Instance from Board Config
  void InitializeBoard(const char*);
  /// ptr to Rendering Function
  void (*RenderPolygon)(MyGrVertex*, int, TextureHandler*,TextureHandler*,bool);
  /// Rendering Function with MultiPass (One TMU)
  static void RenderPolygonMultiPass(MyGrVertex*, int, TextureHandler*,TextureHandler*,bool);
  /// Rendering Function with SinglePass (Two (or more) TMUs)
  static void RenderPolygonSinglePass(MyGrVertex*, int, TextureHandler*,TextureHandler*,bool);

  /// used to set up polygon geometry before rasterization.
  inline void SetupPolygon( G3DPolygonDP& poly, float& J1, float& J2, float& J3, 
                                                float& K1, float& K2, float& K3,
                                                float& M,  float& N,  float& O  );
};

#endif // __CS_G3D_GLIDE_H__
