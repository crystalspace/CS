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

#ifndef GRAPH3D_OPENGL_H
#define GRAPH3D_OPENGL_H

// GRAPH3D.H
// csGraphics3DOpenGL OpenGL rasterizer class.

// Concieved by Jorrit Tyberghein and Gary Clark
// Expanded by Dan Ogles
// Further expanded by Gary Haussmann

#include <GL/gl.h>

#include "csutil/scf.h"
#include "cs3d/opengl/ogl_txtmgr.h"
#include "igraph3d.h"
#include "iplugin.h"

struct iGraphics2D;
class OpenGLTextureCache;
class OpenGLLightmapCache;

///
class csGraphics3DOpenGL : public iGraphics3D
{
private:
  // set proper GL flags based on ZBufMode.  This is usually
  // utilized just before a polygon is drawn; it is not
  // done when the Z-buffer mode is set in SetRasterCaps because
  // other routines may modify the GL flags between a call
  // to SetRasterCaps and the call to the polygon
  // drawing routine
  void SetGLZBufferFlags();

  /// handle of a local 1D alpha-blend texture; this texture holds an
  /// exponential alpha curve from 0 to 1.  Using this texture, you
  /// can convert from a linear texture coordinate of 0-1 to an
  /// exponential alpha-blend curve.  Such a mapping is useful for
  /// drawing exponential fog polygons.  A value of 0 means the handle
  /// is unallocated
  GLuint m_fogtexturehandle;

  /// Pointer to a member function that tries to draw the polygon in a quick
  /// optimized manner.  If this function succeeds in drawing the polygon,
  /// it should return true thus bypassing the normal polygon drawing code
  /// altogether.  If the function cannot draw the polygon for whatever
  /// reason (typically it is because the polygon has certain layers or
  /// attributes that cannot be drawn in an optimized manner) the function
  /// should return false, meaning that the default generalized polygon
  /// drawing code should draw the polygon instead.
  bool (csGraphics3DOpenGL::*ShortcutDrawPolygon)(G3DPolygonDP &poly);

  /// This shortcut routine gets called by
  /// StartPolygonFX before normal FX setup occurs.  If the shortcut routine
  /// determines that it can draw the upcoming series of polygons in an
  /// optimized manner, it should set up properly and return true.  If it
  /// cannot, it should return false and let the standard DrawPolygonFX
  /// perform setup.  In any case, the value returned by the StartPolygonFX
  /// must be matched in subsequent calls to DrawPolygonFX; that is, if
  /// a StartPolygonFX call returns 'true', then calls to DrawPolygonFX and
  /// FinishPolygonFX must also return 'true' until the next time StartPolygonFX
  /// is called.
  bool (csGraphics3DOpenGL::*ShortcutStartPolygonFX)(iTextureHandle *handle,UInt mode);

  /// Shortcut tried before executing standard DrawPolygonFX code.  The
  /// value returned by this must match the most recent return value of
  /// ShortcutStartPolygonFX(); see comments for that member.
  bool (csGraphics3DOpenGL::*ShortcutDrawPolygonFX)(G3DPolygonDPFX &poly);

  /// Shortcut tried before executing standard FinishPolygonFX code.
  /// The value returned by this must match the most recent return value of
  /// ShortcutStartPolygonFX(); see comments for that member.
  bool (csGraphics3DOpenGL::*ShortcutFinishPolygonFX)();

  // Some common shortcut functions that may or may not apply, depending
  // on the underlying hardware

  /// Shortcut to Override standard polygon drawing when we have multitexture
  bool MultitextureDrawPolygon(G3DPolygonDP &poly);

  /// Shortcuts to replace the standard Start/Draw/Finish set of Draw...FX functions;
  /// this set collects up polygons and then draws them in batches, instead
  /// of drawing each individual poly with gl calls
  bool BatchStartPolygonFX(iTextureHandle *handle, UInt mode);
  bool BatchAccumulatePolygonFX(G3DPolygonDPFX &poly);
  bool BatchFlushPolygonFX();

protected:
  /// Z Buffer mode to use while rendering next polygon.
  G3DZBufMode z_buf_mode;

  /// Width of display.
  int width;
  /// Height of display.
  int height;
  /// Opt: width divided by 2.
  int width2;
  /// Opt: height divided by 2.
  int height2;

  /**
   * render-states
   * these override any other variable settings.
   */
  struct
  {
    bool dither; // dither colors?
    bool specular; // draw specular highlights?
    bool trilinearmap; // texel/mipmap interpolate?
    bool gouraud; // gouraud shading on polygons?
    bool alphablend; // enable transparency?
    int  mipmap;    // enable mipmapping?
    bool lighting; // Option variable: do texture lighting? (lightmaps)
    bool textured; // Option variable: render textures?
    bool texel_filt; // Option variable: do expensive texel filtering?
    bool perfect; // Option variable: do perfect texture mapping?
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
    
    /// Option variable: do multitexturing?  This value is zero if multitexturing
    /// is not available.  If multitexturing is available, this value holds
    /// the number of textures that can be mixed together in one pass.
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

  /// The texture manager
  csTextureManagerOpenGL* txtmgr;

  /// The texture cache.
  OpenGLTextureCache* texture_cache;

  /// The lightmap cache.
  OpenGLLightmapCache* lightmap_cache;
public:
  DECLARE_IBASE;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DOpenGL is in charge of creating and managing this.
   */
  iGraphics2D* G2D;

  /// The configuration file
  csIniFile* config;

  /// The System interface. 
  iSystem* System;

  ///
  csGraphics3DOpenGL (iBase *iParent);
  ///
  virtual ~csGraphics3DOpenGL ();

  ///
  virtual bool Initialize (iSystem *iSys);
  ///
  virtual bool Open(const char *Title);
  ///
  virtual void Close();

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags);

  /// End the frame and do a page swap.
  virtual void FinishDraw ();

  /// Print the image in backbuffer
  virtual void Print (csRect *area);

  /// Set the mode for the Z buffer used for drawing the next polygon.
  virtual void SetZBufMode (G3DZBufMode mode);

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
  virtual void DrawLine (csVector3& v1, csVector3& v2, float fov, int color);

  /// Start a series of DrawPolygonFX
  virtual void StartPolygonFX (iTextureHandle* handle, UInt mode);

  /// Finish a series of DrawPolygonFX
  virtual void FinishPolygonFX ();

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Give a texture to csGraphics3DOpenGL to cache it.
  virtual void CacheTexture (iPolygonTexture *texture);

  /**
   * Give a texture to csGraphics3DOpenGL to initialize the cache for it.
   * This is used together with the sub-texture optimization and is meant
   * to allocate the space in the cache but not do any actual calculations yet.
   */
  void CacheInitTexture (iPolygonTexture* texture);

  /// Give a sub-texture to csGraphics3DOpenGL to cache it.
  void CacheSubTexture (iPolygonTexture* texture, int u, int v);

  /**
   * Give a rectangle to csGraphics3DOpenGL so that all sub-textures
   * in this rectangle are cached.
   */
  void CacheRectTexture (iPolygonTexture* texture, int minu, int minv, int maxu, int maxv);

  /**
   * Allocate a 'lighted texture' in which the base texture and lightmap
   * are pre-mixed in software and then presented to OpenGL to render.
   * This emulates multi-texturing which is needed for transparent
   * lighted portals
   */
  void CacheLightedTexture(iPolygonTexture *texture);

  /// Release a texture from the cache.
  virtual void UncacheTexture (iPolygonTexture* texture);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op);

  /**
   * Get the current driver's capabilities. Each driver implements their
   * own function.
   */
  virtual void GetCaps (G3D_CAPS *caps);

  /// Get address of Z-buffer at specific point
  virtual unsigned long *GetZBufPoint(int /*x*/, int /*y*/)
  { return NULL; }

  /// Dump the texture cache.
  virtual void DumpCache ();

  /// Clear the texture cache.
  virtual void ClearCache ();

  /// Get drawing buffer width
  virtual int GetWidth ()
  { return width; }
  /// Get drawing buffer height
  virtual int GetHeight ()
  { return height; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);

  /// Get the iGraphics2D driver.
  virtual iGraphics2D *GetDriver2D ()
  { return G2D; }

  /// Get the iTextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return txtmgr; }

  /// Returns true if this driver requires all maps to be PO2.
  virtual bool NeedsPO2Maps ()
  { return true; }
  /// Returns the maximum aspect ratio of maps.
  virtual int GetMaximumAspectRatio ()
  { return 32768; }

  /// Get the fog mode.
  virtual G3D_FOGMETHOD GetFogMode ()
  { return G3DFOGMETHOD_VERTEX; }

  /// Get the fog mode.
  virtual bool SetFogMode (G3D_FOGMETHOD fogm)
  { if (fogm == G3DFOGMETHOD_VERTEX) return true; else return false; }

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily
   * closed.
   * The given CS_ID can be used to identify multiple fog objects when
   * multiple objects are started.
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
   */
  virtual void AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype);
        
  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in DrawFrontFog/DrawBackFog is not important).
   */
  virtual void CloseFogObject (CS_ID id);
	  
  /// Get the colorformat you want.
  virtual G3D_COLORMAPFORMAT GetColormapFormat ()
  { return G3DCOLORFORMAT_24BIT; }

  void SysPrintf (int mode, char* str, ...);

  /// Get Z-buffer value at given X,Y position
  virtual float GetZbuffValue (int x, int y)
  { return 0; }

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char *iAlpha, int iWidth, int iHeight)
  { return NULL; }
};

#endif
