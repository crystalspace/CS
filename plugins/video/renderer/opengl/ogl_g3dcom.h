/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __OGL3DCOM_H__
#define __OGL3DCOM_H__

// GRAPH3D.H
// csGraphics3DOGLCommon OpenGL rasterizer class.

// Concieved by Jorrit Tyberghein and Gary Clark
// Expanded by Dan Ogles
// Further expanded by Gary Haussmann

#include <GL/gl.h>

#include "csutil/scf.h"
#include "csutil/cfgacc.h"
#include "video/renderer/common/dtmesh.h"
#include "video/renderer/common/dpmesh.h"
#include "csgeom/transfrm.h"
#include "ogl_txtmgr.h"
#include "ivideo/graph3d.h"
#include "isys/plugin.h"

struct iGraphics2D;
class OpenGLTextureCache;
class OpenGLLightmapCache;
class csClipper;

///
class csGraphics3DOGLCommon : public iGraphics3D
{
private:
  /**
   * set proper GL flags based on ZBufMode.  This is usually
   * utilized just before a polygon is drawn; it is not
   * done when the Z-buffer mode is set in SetRenderState because
   * other routines may modify the GL flags between a call
   * to SetRenderState and the call to the polygon
   * drawing routine
   */
  void SetGLZBufferFlags();

  /**
   * Pointer to a member function that tries to draw the polygon in a quick
   * optimized manner.  If this function succeeds in drawing the polygon,
   * it should return true thus bypassing the normal polygon drawing code
   * altogether.  If the function cannot draw the polygon for whatever
   * reason (typically it is because the polygon has certain layers or
   * attributes that cannot be drawn in an optimized manner) the function
   * should return false, meaning that the default generalized polygon
   * drawing code should draw the polygon instead.
   */
  bool (csGraphics3DOGLCommon::*ShortcutDrawPolygon)(G3DPolygonDP &poly);

  /**
   * This shortcut routine gets called by
   * StartPolygonFX before normal FX setup occurs.  If the shortcut routine
   * determines that it can draw the upcoming series of polygons in an
   * optimized manner, it should set up properly and return true.  If it
   * cannot, it should return false and let the standard DrawPolygonFX
   * perform setup.  In any case, the value returned by the StartPolygonFX
   * must be matched in subsequent calls to DrawPolygonFX; that is, if
   * a StartPolygonFX call returns 'true', then calls to DrawPolygonFX and
   * FinishPolygonFX must also return 'true' until the next time StartPolygonFX
   * is called.
   */
  bool (csGraphics3DOGLCommon::*ShortcutStartPolygonFX)(iMaterialHandle *handle,UInt mode);

  /**
   * Shortcut tried before executing standard DrawPolygonFX code.  The
   * value returned by this must match the most recent return value of
   * ShortcutStartPolygonFX(); see comments for that member.
   */
  bool (csGraphics3DOGLCommon::*ShortcutDrawPolygonFX)(G3DPolygonDPFX &poly);

  /**
   * Shortcut tried before executing standard FinishPolygonFX code.
   * The value returned by this must match the most recent return value of
   * ShortcutStartPolygonFX(); see comments for that member.
   */
  bool (csGraphics3DOGLCommon::*ShortcutFinishPolygonFX)();

  // Some common shortcut functions that may or may not apply, depending
  // on the underlying hardware
  // guess the proper blend mode to use
  void Guess_BlendMode(GLenum *src, GLenum *dst);

protected:
  friend class csOpenGLHalo;

  /**
   * handle of a local 1D alpha-blend texture; this texture holds an
   * exponential alpha curve from 0 to 1.  Using this texture, you
   * can convert from a linear texture coordinate of 0-1 to an
   * exponential alpha-blend curve.  Such a mapping is useful for
   * drawing exponential fog polygons.  A value of 0 means the handle
   * is unallocated
   */
  GLuint m_fogtexturehandle;

  /// Z Buffer mode to use while rendering next polygon.
  csZBufMode z_buf_mode;

  /// Render capabilities
  csGraphics3DCaps Caps;

  /// Width of display.
  int width;
  /// Height of display.
  int height;
  /// Opt: width divided by 2.
  int width2;
  /// Opt: height divided by 2.
  int height2;

  /// Current transformation from world to camera.
  csReversibleTransform o2c;
  /// Current 2D clipper.
  csClipper* clipper;
  /// Current aspect ratio for perspective correction.
  float aspect;
  /// Current inverse aspect ratio for perspective correction.
  float inv_aspect;
  /// Use an inverted orthographic projection matrix?
  bool inverted;
  /**
   * render-states
   * these override any other variable settings.
   */
  struct
  {
    bool dither; // dither colors?
    bool trilinearmap; // texel/mipmap interpolate?
    bool gouraud; // gouraud shading on polygons?
    bool alphablend; // enable transparency?
    int  mipmap;    // enable mipmapping?
    bool lighting; // Option variable: do texture lighting? (lightmaps)
    bool textured; // Option variable: render textures?
  } m_renderstate;

  /// Should DrawPolygonFX use Gouraud shading?
  bool  m_gouraud;
  /// Mixing mode for DrawPolygonFX
  UInt  m_mixmode;
  /// Alpha value for DrawPolygonFX
  float m_alpha;
  /// Should DrawPolygonFX use texture?
  bool  m_textured;

  // load-time configuration options
  struct
  {
    /**
     * Current settings of the user configurable blend parameters for
     * lightmaps.  Certain settings work better on certain cards
     */
    GLenum m_lightmap_src_blend;
    GLenum m_lightmap_dst_blend;
    
    /**
     * Option variable: do multitexturing?  This value is zero if multitexturing
     * is not available.  If multitexturing is available, this value holds
     * the number of textures that can be mixed together in one pass.
     */
    int do_multitexture_level;
    
    /**
     * Brighten rendered textures in an extra pass.
     * This slows down rendering (we should use multi-texturing)
     * but is simulates 2*SRC*DST on cards that only support SRC*DST.
     * At least it seems to do this on a RIVA 128.
     */
    bool do_extra_bright;
  } m_config_options;

  /// For debugging: the maximum number of polygons to draw in a frame.
  long dbg_max_polygons_to_draw;
  /// For debugging: the current polygon number.
  long dbg_current_polygon;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

public:
  /// The maximum texture size
  GLint max_texture_size;
  /// The texture cache.
  OpenGLTextureCache* texture_cache;
  /// The texture manager
  csTextureManagerOpenGL* txtmgr;

  /// The lightmap cache.
  OpenGLLightmapCache* lightmap_cache;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DOGLCommon is in charge of creating and managing this.
   */
  iGraphics2D* G2D;

  /// The configuration file
  csConfigAccess config;

  /// The System interface. 
  iSystem* System;

  ///
  csGraphics3DOGLCommon ();
  ///
  virtual ~csGraphics3DOGLCommon ();

  ///
  bool NewInitialize (iSystem *iSys);
  /// Initialize from the state data of another driver of this type
  void SharedInitialize (csGraphics3DOGLCommon *d);
  ///
  bool NewOpen (const char *Title);
  /// Open from the state data of another driver of this type
  void SharedOpen (csGraphics3DOGLCommon *d);
  /// Helper function
  void CommonOpen ();
  ///
  virtual void Close ();

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags);

  /// End the frame and do a page swap.
  virtual void FinishDraw ();

  /// Print the image in backbuffer
  virtual void Print (csRect *area);

  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly);

  /**
   * Draw the projected polygon with light and texture.
   * Debugging version. This one does not actually draw anything
   * but it just prints debug information about what it would have
   * done.
   */
  virtual void DrawPolygonDebug (G3DPolygonDP& poly);

  /// Draw a line in camera space.
  virtual void DrawLine (const csVector3& v1, const csVector3& v2,
  	float fov, int color);

  /// Start a series of DrawPolygonFX
  virtual void StartPolygonFX (iMaterialHandle* handle, UInt mode);

  /// Finish a series of DrawPolygonFX
  virtual void FinishPolygonFX ();

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Give a texture to csGraphics3DOGLCommon to cache it.
  void CacheTexture (iPolygonTexture *texture);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op);

  /**
   * Get the current driver's capabilities. Each driver implements their
   * own function.
   */
  virtual csGraphics3DCaps *GetCaps ()
  { return &Caps; }

  /// Get address of Z-buffer at specific point
  virtual unsigned long *GetZBuffAt (int, int)
  { return NULL; }

  /// Dump the texture cache.
  virtual void DumpCache ();

  /// Clear the texture cache.
  virtual void ClearCache ();

  /// Remove some polygon from the cache.
  virtual void RemoveFromCache (iPolygonTexture* /*poly_texture*/) { }

  /// Get drawing buffer width
  virtual int GetWidth ()
  { return width; }
  /// Get drawing buffer height
  virtual int GetHeight ()
  { return height; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y)
  {
    width2 = x;
    height2 = y;
  }
  /// Get center of projection.
  virtual void GetPerspectiveCenter (int& x, int& y)
  {
    x = width2;
    y = height2;
  }
  /// Set perspective aspect.
  virtual void SetPerspectiveAspect (float aspect)
  {
    this->aspect = aspect;
    inv_aspect = 1./aspect;
  }
  /// Get perspective aspect.
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
  /// Set optional clipper.
  virtual void SetClipper (csVector2* vertices, int num_vertices);
  /// Get clipper.
  virtual void GetClipper (csVector2* vertices, int& num_vertices);
  /// Draw a triangle mesh.
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh);

  /// Draw a polygon mesh.
  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh)
  {
    DefaultDrawPolygonMesh (mesh, this, o2c, clipper, aspect, inv_aspect,
    	width2, height2);
  }

  /// Get the iGraphics2D driver.
  virtual iGraphics2D *GetDriver2D ()
  { return G2D; }

  /// Get the iTextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return txtmgr; }

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily
   * closed.
   * The given CS_ID can be used to identify multiple fog objects when
   * multiple objects are started.
   * On the OpenGL driver this function is not used.  Instead the
   * fog is drawn as an additional texture on each polygon that
   * is effected by the fog.
   */
  virtual void OpenFogObject (CS_ID id, csFog* fog);
    
  /**
   * Add a front or back-facing fog polygon in the current fog object.
   * Note that it is guaranteed that all back-facing fog polygons
   * will have been added before the first front-facing polygon.
   * fogtype can be:
   * <ul>
   *	<li>CS_FOG_FRONT:	a front-facing polygon
   *	<li>CS_FOG_BACK:	a back-facing polygon
   *	<li>CS_FOG_VIEW:	the view-plane
   * </ul>
   * On the OpenGL driver this function is not used.  Instead the
   * fog is drawn as an additional texture on each polygon that
   * is effected by the fog.
   */
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype);
        
  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in DrawFrontFog/DrawBackFog is not important).
   * On the OpenGL driver this function is not used.  Instead the
   * fog is drawn as an additional texture on each polygon that
   * is effected by the fog.
   */
  virtual void CloseFogObject (CS_ID id);
	  
  /// Get Z-buffer value at given X,Y position
  virtual float GetZBuffValue (int x, int y);

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char* iAlpha, int iWidth, int iHeight);

  /// Draw a 2D sprite
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha);

  /**
   * If supported, this function will attempt to query the OpenGL driver
   * to see what extensions it supports so that other parts of the renderer
   * can use appropriate extensions where possible.
   */
  void DetectExtensions ();

  /**
   * Draw a fully-featured polygon assuming one has an OpenGL renderer
   * that supports ARB_multitexture
   */
  bool DrawPolygonMultiTexture (G3DPolygonDP &poly);

  /**
   * Draw a fully-featured polygon assuming one has an OpenGL renderer
   * that only has a single texture unit.
   */
  void DrawPolygonSingleTexture (G3DPolygonDP &poly);

  /**
   * Draw a polygon but only do z-fill. Do not actually render
   * anything.
   */
  void DrawPolygonZFill (G3DPolygonDP &poly);

  // Extension flags
  bool ARB_multitexture;

 protected:

  void start_draw_poly ();
  void end_draw_poly ();
  bool in_draw_poly;
};

#endif // __OGL3DCOM_H__
