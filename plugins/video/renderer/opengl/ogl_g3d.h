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

#include <GL/gl.h>

#include "cscom/com.h"
#include "cs3d/opengl/ogl_txtmgr.h"
#include "igraph3d.h"

interface IPolygon3D;
interface IGraphics2D;
class OpenGLTextureCache;
class OpenGLLightmapCache;

extern const CLSID CLSID_OpenGLGraphics3D;

///
class csGraphics3DOpenGL : public IGraphics3D
{
private:
  // set proper GL flags based on ZBufMode.  This is usually
  // utilized just before a polygon is drawn; it is not
  // done when the Z-buffer mode is set in SetRasterCaps because
  // other routines may modify the GL flags between a call
  // to SetRasterCaps and the call to the polygon
  // drawing routine
  void SetGLZBufferFlags();

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
   * DAN: render-states
   * these override any other variable settings.
   */
  bool rstate_dither;
  bool rstate_specular;
  bool rstate_trilinearmap;
  bool rstate_gouraud;
  bool rstate_alphablend;
  int rstate_mipmap;

  /// Should DrawPolygonFX use Gouraud shading?
  bool  m_gouraud;
  /// Mixing mode for DrawPolygonFX
  UInt  m_mixmode;
  /// Alpha value for DrawPolygonFX
  float m_alpha;
  /// Should DrawPolygonFX use texture?
  bool  m_textured;

  /**
   * Current settings of the user configurable blend parameters for
   * lightmaps.  Certain settings work better on certain cards
   */
  GLenum m_lightmap_src_blend;
  GLenum m_lightmap_dst_blend;

  /**
   * Brighten rendered textures in an extra pass.
   * This slows down rendering (we should use multi-texturing)
   * but is simulates 2*SRC*DST on cards that only support SRC*DST.
   * At least it seems to do this on a RIVA 128.
   */
  bool do_extra_bright;

  /// For debugging: the maximum number of polygons to draw in a frame.
  long dbg_max_polygons_to_draw;
  /// For debugging: the current polygon number.
  long dbg_current_polygon;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

public:
  /**
   * Low-level 2D graphics layer.
   * csGraphics3DOpenGL is in charge of creating and managing this.
   */
  IGraphics2D* m_piG2D;

  /// The configuration file
  csIniFile* config;

  /// The texture manager
  csTextureManagerOpenGL* txtmgr;

  /// The texture cache.
  OpenGLTextureCache* texture_cache;
  /// The lightmap cache.
  OpenGLLightmapCache* lightmap_cache;

  /// The System interface. 
  ISystem* m_piSystem;

  /// Option variable: do texture lighting?
  static bool do_lighting;
  /// Option variable: render transparent textures?
  static bool do_transp;
  /// Option variable: render textures?
  static bool do_textured;
  /// Option variable: do expensive texel filtering?
  static bool do_texel_filt;
  /// Option variable: do perfect texture mapping?
  static bool do_perfect;

  /// Option variable: do multitexturing?  This value is zero if multitexturing
  /// is not available.  If multitexturing is available, this value holds
  /// the number of textures that can be mixed together in one pass.
  static int do_multitexture_level;

  /// For interlacing. 0 = draw odd lines, 1 = draw even lines, -1 = draw all lines.
  static int do_interlaced;
  /**
   * For interlacing. Temporary set to true if we moved quickly. This will decrease
   * the bluriness a little.
   */
  static bool ilace_fastmove;

  ///
  csGraphics3DOpenGL (ISystem* piSystem);
  ///
  virtual ~csGraphics3DOpenGL ();

  ///
  STDMETHODIMP Initialize ();
  ///
  STDMETHODIMP Open(const char *Title);
  ///
  STDMETHODIMP Close();

  /// Change the dimensions of the display.
  STDMETHODIMP SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  STDMETHODIMP BeginDraw (int DrawFlags);

  /// End the frame and do a page swap.
  STDMETHODIMP FinishDraw ();

  /// Print the image in backbuffer
  STDMETHODIMP Print (csRect *area);

  /// Set the mode for the Z buffer used for drawing the next polygon.
  STDMETHODIMP SetZBufMode (G3DZBufMode mode);

  /// Draw the projected polygon with light and texture.
  STDMETHODIMP DrawPolygon (G3DPolygonDP& poly);

  /**
   * Draw the projected polygon with light and texture.
   * Debugging version. This one does not actually draw anything
   * but it just prints debug information about what it would have
   * done.
   */
  STDMETHODIMP DrawPolygonDebug (G3DPolygonDP& poly);

  /// Draw a line in camera space.
  STDMETHODIMP DrawLine (csVector3& v1, csVector3& v2, float fov, int color);

  /// Start a series of DrawPolygonFX
  STDMETHODIMP StartPolygonFX (ITextureHandle* handle, UInt mode);

  /// Finish a series of DrawPolygonFX
  STDMETHODIMP FinishPolygonFX ();

  /// Draw a polygon with special effects.
  STDMETHODIMP DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Give a texture to csGraphics3DOpenGL to cache it.
  STDMETHODIMP CacheTexture (IPolygonTexture* texture);

  /**
   * Give a texture to csGraphics3DOpenGL to initialize the cache for it.
   * This is used together with the sub-texture optimization and is meant
   * to allocate the space in the cache but not do any actual calculations yet.
   */
  void CacheInitTexture (IPolygonTexture* texture);

  /// Give a sub-texture to csGraphics3DOpenGL to cache it.
  void CacheSubTexture (IPolygonTexture* texture, int u, int v);

  /**
   * Give a rectangle to csGraphics3DOpenGL so that all sub-textures
   * in this rectangle are cached.
   */
  void CacheRectTexture (IPolygonTexture* texture, int minu, int minv, int maxu, int maxv);

  /**
   * Allocate a 'lighted texture' in which the base texture and lightmap
   * are pre-mixed in software and then presented to OpenGL to render.
   * This emulates multi-texturing which is needed for transparent
   * lighted portals
   */
  void CacheLightedTexture(IPolygonTexture *texture);

  /// Release a texture from the cache.
  STDMETHODIMP UncacheTexture (IPolygonTexture* texture);

  /// Set a renderstate boolean.
  STDMETHODIMP SetRenderState (G3D_RENDERSTATEOPTION op, long val);

  /// Get a renderstate value.
  STDMETHODIMP GetRenderState (G3D_RENDERSTATEOPTION op, long& val);

  /**
   * Get the current driver's capabilities. Each driver implements their
   * own function.
   */
  STDMETHODIMP GetCaps (G3D_CAPS *caps);

  /// Get address of Z-buffer at specific point
  STDMETHODIMP GetZBufPoint(int /*x*/, int /*y*/, unsigned long** retval) { *retval = NULL; return E_NOTIMPL; }

  /// Dump the texture cache.
  STDMETHODIMP DumpCache (void);

  /// Clear the texture cache.
  STDMETHODIMP ClearCache (void);

  /// Get drawing buffer width
  STDMETHODIMP GetWidth(int& retval) { retval = width; return S_OK; }
  /// Get drawing buffer height
  STDMETHODIMP GetHeight(int& retval) { retval = height; return S_OK; }
  /// Set center of projection.
  STDMETHODIMP SetPerspectiveCenter (int x, int y);

  /// Get the IGraphics2D driver.
  STDMETHODIMP Get2dDriver(IGraphics2D** pi) { *pi = m_piG2D; return S_OK; }

  /// Get the ITextureManager.
  STDMETHODIMP GetTextureManager (ITextureManager** pi) { *pi = (ITextureManager*)txtmgr; return S_OK; }

  /// Returns S_OK if this driver requires all maps to be PO2.
  STDMETHODIMP NeedsPO2Maps() { return S_OK; }
  /// Returns the maximum aspect ratio of maps.
  STDMETHODIMP GetMaximumAspectRatio(int& retval) { retval = 32768; return S_OK; }

  /// Get the fog mode.
  STDMETHODIMP GetFogMode (G3D_FOGMETHOD& retval) { retval = G3DFOGMETHOD_VERTEX; return S_OK; }

  /// Get the fog mode.
  STDMETHODIMP SetFogMode (G3D_FOGMETHOD fogm) { if (fogm == G3DFOGMETHOD_VERTEX) return S_OK; else return E_FAIL; }

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily
   * closed.
   * The given CS_ID can be used to identify multiple fog objects when
   * multiple objects are started.
   */
  STDMETHODIMP OpenFogObject (CS_ID id, csFog* fog);
    
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
  STDMETHODIMP AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype);
        
  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in DrawFrontFog/DrawBackFog is not important).
   */
  STDMETHODIMP CloseFogObject (CS_ID id);
	  
  /// Get the colorformat you want.
  STDMETHODIMP GetColormapFormat(G3D_COLORMAPFORMAT& retval) { retval = G3DCOLORFORMAT_24BIT; return S_OK; }

  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE( csGraphics3DOpenGL )

  void SysPrintf (int mode, char* str, ...);
};

class csGraphics3DOpenGLFactory : public IGraphicsContextFactory
{
    /// Create the graphics context
    STDMETHODIMP CreateInstance( REFIID riid, ISystem* piSystem, void** ppv );

    /// Lock or unlock from memory.
    STDMETHODIMP LockServer(COMBOOL bLock);

    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE( csGraphics3DOpenGLFactory )
};

#endif

