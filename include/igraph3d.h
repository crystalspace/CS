/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Jorrit Tyberghein, Dan Ogles, and Gary Clark.

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

#ifndef __IGRAPH3D_H__
#define __IGRAPH3D_H__

#include "csutil/scf.h"
#include "iplugin.h"

class csMatrix3;
class csVector3;
class csRect;

struct iGraphics2D;
struct iPolygonTexture;
struct iTextureManager;
struct iTextureHandle;
struct iHalo;
struct iImage;

#define CS_FOG_FRONT		0
#define CS_FOG_BACK		1
#define CS_FOG_VIEW		2

/**
 * Mix modes for DrawPolygonFX ()
 * The constants below can be ORed together if they belong to different masks.
 */ 
#define CS_FX_MASK_MIXMODE	0xF0000000	// SRC/DST mixing mode mask
#define CS_FX_COPY		0x00000000	// =SRC
#define CS_FX_MULTIPLY		0x10000000	// =SRC*DST
#define CS_FX_MULTIPLY2		0x20000000	// =2*SRC*DST
#define CS_FX_ADD		0x30000000	// =SRC+DST
#define CS_FX_ALPHA		0x40000000	// =(1-alpha)*SRC + alpha*DST
#define CS_FX_TRANSPARENT	0x50000000	// =DST
#define CS_FX_KEYCOLOR		0x08000000	// color 0 is transparent
#define CS_FX_GOURAUD		0x04000000	// Gouraud shading
#define CS_FX_MASK_ALPHA	0x000000FF	// alpha = 0..FF (opaque..transparent)

/// Macro for easier setting of alpha bits into mixmode
#define CS_FX_SETALPHA(alpha)	(CS_FX_ALPHA | UInt (alpha * CS_FX_MASK_ALPHA))

/// Vertex Structure for use with G3DPolygonDP and G3DPolygonAFP
class G3DVertex
{
public:
  /// Screen x space.
  float sx;
  /// Screen y space.
  float sy;
};

/// Vertex Structure for use with G3DPolygonDPQ
class G3DTexturedVertex : public G3DVertex
{
public:
  /// z value
  float z;
  
  //Texture coordinates
  float u, v; 

  //Lighting info (Used only with Gouroud shading (between 0 and 1))
  float r, g, b;
};

/// Extra information for vertex fogging.
class G3DFogInfo
{
public:
  /// Color.
  float r, g, b;
  /// Intensity (== density * thickness).
  float intensity;
};

///
class G3DTexturePlane
{
public:
  /// Transformation from camera space to texture space.
  csMatrix3* m_cam2tex;
  /// Transformation from camera space to texture space.
  csVector3* v_cam2tex;
};

///
class G3DPolyNormal
{
public:
  ///
  float A;
  ///
  float B;
  ///
  float C;
  ///
  float D;
};


/// Structure containing all info needed by DrawPolygonFX (DPFX)
struct G3DPolygonDPFX
{
  /// Current number of vertices.
  int num;
  /// Vertices that form the polygon.
  G3DTexturedVertex vertices[100];
  /// Extra optional fog information.
  G3DFogInfo fog_info[100];
  /// Use fog info?
  bool use_fog;

  /// Invert aspect ratio that was used to perspective project the vertices (1/fov)
  float inv_aspect;

  /// The texture handle as returned by iTextureManager.
  iTextureHandle* txt_handle;

  /// Use this color for drawing (if txt_handle == NULL) instead of a texture.
  float flat_color_r;
  float flat_color_g;
  float flat_color_b;
};

/// Structure containing all info needed by DrawPolygon (DP)
struct G3DPolygonDP
{
  /// Current number of vertices.
  int num;
  /// Vertices that form the polygon.
  G3DVertex vertices[100];
  /// Extra optional fog information.
  G3DFogInfo fog_info[100];
  /// Use fog info?
  bool use_fog;

  /// Invert aspect ratio that was used to perspective project the vertices (1/fov)
  float inv_aspect;

  /// The texture handle as returned by iTextureManager.
  iTextureHandle* txt_handle;

  /// Transformation matrices for the texture. @@@ BAD NAME
  G3DTexturePlane plane;
  /// The plane equation in camera space of this polygon. @@@ BAD NAME
  G3DPolyNormal normal;

  /// Use this color for drawing (if txt_handle == NULL) instead of a texture.
  float flat_color_r;
  float flat_color_g;
  float flat_color_b;

  ///Handle to lighted textures (texture+lightmap) (for all mipmap levels)
  iPolygonTexture* poly_texture[4];

  /** 
    * AlphaValue of the polygon. Ranges from 0 to 100. 0 means opaque, 100 is 
    * comletely transparent.
    */
  int alpha;

  ///true, if it is ok, to use mipmaps
  bool uses_mipmaps;

  ///z value (in camera space) of vertex[0]
  float z_value;
};

/// Structure containing all info needed by DrawPolygonFlat (DPF)
typedef G3DPolygonDP G3DPolygonDPF;

/// Structure containing all info needed by AddFogPolygon (AFP)
struct G3DPolygonAFP
{
  /// Current number of vertices.
  int num;
  /// Vertices that form the polygon.
  G3DVertex vertices[100];

  /// Invert aspect ratio that was used to perspective project the vertices (1/fov)
  float inv_aspect;

  /// The plane equation in camera space of this polygon. @@@ BAD NAME
  G3DPolyNormal normal;
};

/// Don't test/write, write, test, and write/test, respectively.
enum G3DZBufMode
{
  CS_ZBUF_NONE = 0,
  CS_ZBUF_FILL = 1,
  CS_ZBUF_TEST = 2,
  CS_ZBUF_USE  = 3
};

///
enum G3D_RENDERSTATEOPTION
{
  G3DRENDERSTATE_NOTHING,
  G3DRENDERSTATE_ZBUFFERTESTENABLE,
  G3DRENDERSTATE_ZBUFFERFILLENABLE,
  G3DRENDERSTATE_DITHERENABLE,
  G3DRENDERSTATE_SPECULARENABLE,
  G3DRENDERSTATE_BILINEARMAPPINGENABLE,
  G3DRENDERSTATE_TRILINEARMAPPINGENABLE,
  G3DRENDERSTATE_TRANSPARENCYENABLE,
  G3DRENDERSTATE_MIPMAPENABLE,
  G3DRENDERSTATE_TEXTUREMAPPINGENABLE,
  G3DRENDERSTATE_FILTERINGENABLE,
  G3DRENDERSTATE_PERFECTMAPPINGENABLE,	// OBSOLETE
  G3DRENDERSTATE_LIGHTINGENABLE,
  G3DRENDERSTATE_INTERLACINGENABLE,
  G3DRENDERSTATE_MMXENABLE,
  G3DRENDERSTATE_INTERPOLATIONSTEP,
  G3DRENDERSTATE_MAXPOLYGONSTODRAW,
  G3DRENDERSTATE_GOURAUDENABLE
};

/// Bit flags for iGraphics3D::BeginDraw ()
/// We're going to draw 2D graphics
#define CSDRAW_2DGRAPHICS   0x00000001
/// We're going to draw 3D graphics
#define CSDRAW_3DGRAPHICS   0x00000002
/// Clear Z-buffer ?
#define CSDRAW_CLEARZBUFFER 0x00000010
/// Clear frame buffer ?
#define CSDRAW_CLEARSCREEN  0x00000020

///
enum G3D_COLORMODEL
{
  G3DCOLORMODEL_MONO,
  G3DCOLORMODEL_RGB
};

///
enum G3D_RASTERCAPS
{
  G3DRASTERCAPS_DITHER = 0x01,
  G3DRASTERCAPS_SUBPIXEL = 0x02,
  G3DRASTERCAPS_ZBUFFERLESSHSR = 0x04
};

///
enum G3D_SHADECAPS
{
  G3DRASTERCAPS_FLAT = 1,
  G3DRASTERCAPS_GOURAUD = 2,
  G3DRASTERCAPS_PHONG = 4,
  G3DRASTERCAPS_LIGHTMAP = 8
};

///
enum G3D_FILTERCAPS
{
  G3DFILTERCAPS_LINEAR = 0x01,
  G3DFILTERCAPS_LINEARMIPLINEAR = 0x02,
  G3DFILTERCAPS_LINEARMIPNEAREST = 0x04,
  G3DFILTERCAPS_MIPLINEAR = 0x08,
  G3DFILTERCAPS_MIPNEAREST = 0x10,
  G3DFILTERCAPS_NEAREST = 0x20
};

///
enum G3D_FOGMETHOD
{
  G3DFOGMETHOD_NONE = 0x00,
  G3DFOGMETHOD_ZBUFFER = 0x01,
  G3DFOGMETHOD_VERTEX = 0x02
};

///
struct G3D_PRIMCAPS
{
  G3D_RASTERCAPS RasterCaps;
  bool canBlend;
  G3D_SHADECAPS ShadeCaps;
  bool PerspectiveCorrects;
  G3D_FILTERCAPS FilterCaps;
};

///
struct G3D_CAPS
{
  G3D_COLORMODEL ColorModel;
  bool CanClip;
  bool SupportsArbitraryMipMapping;
  int BitDepth;
  int ZBufBitDepth;
  int minTexHeight, minTexWidth;
  int maxTexHeight, maxTexWidth;
  G3D_PRIMCAPS PrimaryCaps;
  G3D_FOGMETHOD fog;
};

enum G3D_COLORMAPFORMAT
{
  G3DCOLORFORMAT_ANY,             // can use either private color maps or global color maps
  G3DCOLORFORMAT_PRIVATE,         // only uses private color maps.
  G3DCOLORFORMAT_GLOBAL,          // only uses global color maps.
  G3DCOLORFORMAT_24BIT            // only uses 24 bit textures
};

/**
 * Fog structure.
 */
struct csFog
{
  /// If true then fog is enabled.
  bool enabled;
  /// Density (0 is off).
  float density;
  /// Color (red).
  float red;
  /// Color (green).
  float green;
  /// Color (blue).
  float blue;
};

SCF_VERSION (iGraphics3D, 0, 0, 3);

/**
 * This is the standard 3D graphics interface.
 * All 3D graphics rasterizer servers for Crystal Space
 * should implement this interface, as well as the iGraphics2D interface.
 * The standard implementation is csGraphics3DSoftware.
 */
struct iGraphics3D : public iPlugIn
{
  /// Initialize the 3D graphics system.
  virtual bool Initialize (iSystem *pSystem) = 0;

  /// Open the 3D graphics display.
  virtual bool Open (const char *Title) = 0;
  /// Close the 3D graphics display.
  virtual void Close () = 0;

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height) = 0;

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags) = 0;

  /// End the frame and do a page swap.
  virtual void FinishDraw () = 0;

  /// Print the image in backbuffer
  virtual void Print (csRect *area) = 0;

  /// Set the mode for the Z buffer used for drawing the next polygon.
  virtual void SetZBufMode (G3DZBufMode mode) = 0;

  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly) = 0;

  /**
   * Draw the projected polygon with light and texture.
   * Debugging version. This one does not actually draw anything
   * but it just prints debug information about what it would have
   * done.
   */
  virtual void DrawPolygonDebug (G3DPolygonDP& poly) = 0;

  /// Draw a line in camera space.
  virtual void DrawLine (csVector3& v1, csVector3& v2, float fov, int color) = 0;

  /**
   * Prepare for drawing a series of Polygon FX which all use
   * the same settings. You must call this function before calling a
   * series of DrawPolygonFX(). After calling the series you should
   * call FinishPolygonFX().<p>
   *
   * Warning! After calling this function you are not allowed to do
   * any calls to the 3D rasterizer other than DrawPolygonFX() and
   * FinishPolygonFX().
   *
   * Warning! Do not rely on this method to handle Color keying under
   * all circumstances. Color Keying will only work reliable in Mixmodes
   * FX_Copy, FX_Add and FX_Transparent. When using FX_Multiply
   * and FX_Multiply2, it depends very much on the driver if it works or
   * not. For example the RivaTNT Detonator 0.48 driver can display 
   * Multiply with color keying, while newer versions can't. They will 
   * then not display anything at all. It is always safer to use a texture
   * where transparent sections are white or 50% gray if you want to achieve
   * transparent sections in Multiply, Multiply2.
   * There are also some drivers (which I would consider buggy...), that won't
   * display FX_Alpha correctly with Color Keying. I can't provide a valid 
   * workaround for that except using FX_Multiplay and FX_Add, to manually
   * create the image, but that would be very expensive.
   * 
   * parameters:
   * handle:  The texture handle as returned by iTextureManager.
   * mode:    How shall the new polygon be combined with the current 
   *          screen content. This is any legal combination of CS_FX_XXX
   *          flags including alpha value (if CS_FX_ALPHA flag is set)
   */
  virtual void StartPolygonFX (iTextureHandle* handle, UInt mode) = 0;

  /**
   * Finish drawing a series of Polygon FX.
   */
  virtual void FinishPolygonFX () = 0;

  /**
   * Draw a polygon with special effects. This is the most rich and slowest
   * variant of DrawPolygonXxx. (If you use these features) 
   */
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly) = 0;

  /// Get the current fog mode (G3D_FOGMETHOD).
  virtual G3D_FOGMETHOD GetFogMode () = 0;

  /// Set the current fog mode as supported by this 3D rasterizer.
  virtual bool SetFogMode (G3D_FOGMETHOD fogMethod) = 0;

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily closed.
   * The given CS_ID can be used to identify multiple fog objects when
   * multiple objects are started.
   */
  virtual void OpenFogObject (CS_ID id, csFog* fog) = 0;

  /**
   * Add a front or back-facing fog polygon in the current fog object.
   * Note that it is guaranteed that all back-facing fog polygons
   * will have been added before the first front-facing polygon.
   * fogtype can be:<br>
   * <ul>
   *	<li>CS_FOG_FRONT:	a front-facing polygon
   *	<li>CS_FOG_BACK:	a back-facing polygon
   *	<li>CS_FOG_VIEW:	the view-plane
   * </ul>
   */
  virtual void AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype) = 0;

  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in AddFogPolygon is not important).
   */
  virtual void CloseFogObject (CS_ID id) = 0;

  /// Give a texture to csGraphics3D to cache it.
  virtual void CacheTexture (iPolygonTexture* texture) = 0;

  /// Release a texture from the cache.
  virtual void UncacheTexture (iPolygonTexture* texture) = 0;

  /// Set a renderstate value.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val) = 0;

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op) = 0;

  /**
   * Get the current driver's capabilities. Each driver implements their
   * own function.
   */
  virtual void GetCaps (G3D_CAPS *caps) = 0;

  /// Debugging only: get a pointer to Z-buffer at some location
  virtual unsigned long *GetZBufPoint (int x, int y) = 0;

  /// Dump the texture cache.
  virtual void DumpCache () = 0;

  /// Clear the texture cache.
  virtual void ClearCache () = 0;

  /// Get drawing buffer width
  virtual int GetWidth () = 0;
  /// Get drawing buffer height
  virtual int GetHeight () = 0;

  /**
   * Set center of projection for perspective projection.
   * Center is set in screen space coordinates.
   */
  virtual void SetPerspectiveCenter (int x, int y) = 0;

  /// Get the texture representation scheme.
  virtual G3D_COLORMAPFORMAT GetColormapFormat () = 0;

  /// Returns true if the driver needs PO2 lightmaps and texture maps
  virtual bool NeedsPO2Maps () = 0;

  /// Get the maximum aspect ratio of texture maps.
  virtual int GetMaximumAspectRatio () = 0;

  /// Get the 2D driver: This does NOT increment the refcount of 2D driver!
  virtual iGraphics2D *GetDriver2D () = 0;

  /// Get the texture manager: do NOT increment the refcount of texture manager
  virtual iTextureManager *GetTextureManager () = 0;

  /// Get Z-buffer value at given X,Y position
  virtual float GetZbuffValue (int x, int y) = 0;

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char *iAlpha, int iWidth, int iHeight) = 0;

  /// Do a screenshot: return a new iImage object
  virtual iImage *ScreenShot () { return NULL; }
};

#endif // __IGRAPH3D_H__
