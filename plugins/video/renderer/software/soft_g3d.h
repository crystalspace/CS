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

#ifndef __SOFT_G3D_H__
#define __SOFT_G3D_H__

// GRAPH3D.H
// csGraphics3DSoftware software rasterizer class.

// Concieved by Jorrit Tyberghein and Gary Clark
// Expanded by Dan Ogles

#include "cscom/com.h"
#include "iconfig.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "scan.h"

#if !defined (PROC_INTEL) || defined (NO_ASSEMBLER)
#  undef DO_MMX
#endif

class TextureCache;

interface IPolygon3D;
interface IGraphics2D;
class csTextureManagerSoftware;
class TextureCache;
class TextureCache16;
extern const CLSID CLSID_SoftwareGraphics3D;
struct csFog;

/// This structure is used to hold references to all current fog objects.
struct FogBuffer
{
  FogBuffer* next, * prev;
  CS_ID id;
  float density;
  float red, green, blue;
};

/// Composite version of IConfig.
interface IXConfig3DSoft : public IConfig
{
  DECLARE_IUNKNOWN ()
  STDMETHODIMP SetOption (int id, csVariant* value);
  STDMETHODIMP GetOption (int id, csVariant* value);
  STDMETHODIMP GetNumberOptions (int& num);
  STDMETHODIMP GetOptionDescription (int idx, csOptionDescription* option);

private:
  static csOptionDescription config_options[];
};

///
class csGraphics3DSoftware : public IGraphics3D,
                             public IHaloRasterizer
{
private:
  /// Z buffer for software renderer only. Hardware rendering uses own Z buffer.
  unsigned long* z_buffer;
  /// Size of Z buffer for software renderer.
  long z_buf_size;

  /**
   * Addresses of all lines for this frame. This table is used to avoid
   * calls to GetPixelAt in the main renderer loop. It is filled every frame
   * (by BeginDraw()) because double buffering or other stuff can change
   * the addresses.
   */
  UByte** line_table;

  /**
   * Number of bytes for every pixel (expressed in shifts). Also used in
   * combination with line_table to calculate the in-screen offset.
   */
  int pixel_shift;

  /// For debugging: the maximum number of polygons to draw in a frame.
  long dbg_max_polygons_to_draw;
  /// For debugging: the current polygon number.
  long dbg_current_polygon;

  /// Z Buffer mode to use while rendering next polygon.
  ZBufMode z_buf_mode;

  /// Fog buffers.
  FogBuffer* fog_buffers;

  /// Width of display.
  int width;
  /// Height of display.
  int height;
  /// Opt: width divided by 2.
  int width2;
  /// Opt: height divided by 2.
  int height2;
  /// The pixel format of display
  csPixelFormat pfmt;
#if defined (DO_MMX)
  /// True if CPU has MMX instructions.
  bool cpu_mmx;
  /// True if 3D rendering should use MMX if available.
  bool do_mmx;
#endif

  /// Do we want dithering? (dummy for now)
  bool rstate_dither;
  /// Do we want specular lighting? (dummy for now)
  bool rstate_specular;
  /// Do we want mipmaps?
  int rstate_mipmap;
  /// Do we want visible edges (mostly debug)?
  bool rstate_edges;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

  /// draw_scanline_xxx routines
  csDrawScanline* ScanProc [0x14];
  /// draw_pi_scanline_xxx routines
  csDrawPIScanline* ScanProcPI [4];
  /// draw_pi_scanline_gouraud_xxx routines
  csDrawPIScanlineGouraud* ScanProcPIG [4];
  /// draw_pi_scanline_fx_xxx routines
  csDrawPIScanlineFX* ScanProcPIFX[4];

  UByte* m_BlendingTable[8];

  /// The routine for getting the address of needed scanline_xxx_alpha
  csDrawScanline* (*ScanProc_Alpha) (csGraphics3DSoftware *This, int alpha);

  /// ScanProc_Alpha for 8 bpp modes
  static csDrawScanline* ScanProc_8_Alpha (csGraphics3DSoftware *This, int alpha);
  /// ScanProc_Alpha for 16 bpp modes
  static csDrawScanline* ScanProc_16_Alpha (csGraphics3DSoftware *This, int alpha);
  /// ScanProc_Alpha for 32 bpp modes
  static csDrawScanline* ScanProc_32_Alpha (csGraphics3DSoftware *This, int alpha);

  G3D_FOGMETHOD fogMode;

  /// Look for a given fog buffer
  FogBuffer* find_fog_buffer (CS_ID id);

  /**
   * Same as DrawPolygon but no texture mapping.
   * (Flat drawing).
   */
  HRESULT DrawPolygonFlat (G3DPolygonDPF& poly);

public:
  /**
   * Low-level 2D graphics layer.
   * csGraphics3DSoftware is in charge of creating and managing this.
   */
  IGraphics2D* m_piG2D;

  /// The texture manager.
  csTextureManagerSoftware* txtmgr;

  /// The texture cache.
  TextureCache* tcache;

  /// The System interface.
  ISystem* m_piSystem;

  /// Option variable: do texture lighting?
  bool do_lighting;
  /// Option variable: render transparent textures?
  bool do_transp;
  /// Option variable: render textures?
  bool do_textured;
  /// Option variable: do expensive texel filtering?
  bool do_texel_filt;
  /// Option variable: do very expensive bilinear filtering?
  bool do_bilin_filt;
  /// Option variable: do perfect texture mapping?
  bool do_perfect;
  /// Option variables: mipmap distances
  float zdist_mipmap1, zdist_mipmap2, zdist_mipmap3;
  /// Do we want Gouraud Shaded polygons?
  bool rstate_gouraud;

  /**
   * Option variable: draw white borders around the area of all polygons hit
   * by some light. The light is selected with selected_light.
   */
  bool do_light_frust;

  /// @@todo: remove this
  bool do_debug;
  /// Do interlacing? (-1 - no, 0/1 - yes)
  int do_interlaced;
  /**
   * For interlacing. Temporary set to true if we moved quickly. This will decrease
   * the bluriness a little.
   */
  bool ilace_fastmove;

  ///
  csGraphics3DSoftware (ISystem* piSystem);
  ///
  virtual ~csGraphics3DSoftware ();

  /// Setup scanline drawing routines according to current bpp and setup flags
  void ScanSetup ();

  ///
  STDMETHODIMP Initialize ();
  ///
  STDMETHODIMP Open (char *Title);
  ///
  STDMETHODIMP Close ();

  /// Change the dimensions of the display.
  STDMETHODIMP SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  STDMETHODIMP BeginDraw (int DrawFlags);

  /// End the frame and do a page swap.
  STDMETHODIMP FinishDraw ();

  /// Print the image in backbuffer
  STDMETHODIMP Print (csRect *area);

  /// Set the mode for the Z buffer used for drawing the next polygon.
  STDMETHODIMP SetZBufMode (ZBufMode mode);

  /// Draw the projected polygon with light and texture.
  STDMETHODIMP DrawPolygon (G3DPolygonDP& poly);

  /**
   * Draw the projected polygon with light and texture.
   * Debugging version. This one does not actually draw anything
   * but it just prints debug information about what it would have
   * done.
   */
  STDMETHODIMP DrawPolygonDebug (G3DPolygonDP& poly);

  /// Get the fog mode.
  STDMETHODIMP GetFogMode (G3D_FOGMETHOD& retval) { retval = fogMode; return S_OK; }

  /// Get the fog mode.
  STDMETHODIMP SetFogMode (G3D_FOGMETHOD fogm) { fogMode = fogm; return S_OK; }

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily closed.
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
   *    <li>CS_FOG_FRONT:       a front-facing polygon
   *    <li>CS_FOG_BACK:        a back-facing polygon
   *    <li>CS_FOG_VIEW:        the view-plane
   *    <li>CS_FOG_PLANE:       used in planed fog mode
   * </ul>
   */
  STDMETHODIMP AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype);

  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in DrawFrontFog/DrawBackFog is not important).
   */
  STDMETHODIMP CloseFogObject (CS_ID id);

  /// Draw a line in camera space.
  STDMETHODIMP DrawLine (csVector3& v1, csVector3& v2, float fov, int color);

  /// Start drawing.
  STDMETHODIMP StartPolygonQuick (ITextureHandle* handle, bool gouraud);

  /// Finish drawing.
  STDMETHODIMP FinishPolygonQuick ();

  /// Draw a projected polygon.
  STDMETHODIMP DrawPolygonQuick (G3DPolygonDPQ& poly);

  /// Start a series of DrawPolygonFX
  STDMETHODIMP StartPolygonFX(ITextureHandle* handle, DPFXMixMode mode, float alpha, bool gouraud);

  /// Finish a series of DrawPolygonFX
  STDMETHODIMP FinishPolygonFX();

  /// Draw a polygon with special effects.
  STDMETHODIMP DrawPolygonFX    (G3DPolygonDPFX& poly);

  /// Give a texture to csGraphics3DSoftware to cache it.
  STDMETHODIMP CacheTexture (IPolygonTexture* texture);

  /**
   * Give a texture to csGraphics3DSoftware to initialize the cache for it.
   * This is used together with the sub-texture optimization and is meant
   * to allocate the space in the cache but not do any actual calculations yet.
   */
  void CacheInitTexture (IPolygonTexture* texture);

  /// Give a sub-texture to csGraphics3DSoftware to cache it.
  void CacheSubTexture (IPolygonTexture* texture, int u, int v);

  /**
   * Give a rectangle to csGraphics3DSoftware so that all sub-textures
   * in this rectangle are cached.
   */
  void CacheRectTexture (IPolygonTexture* texture, int minu, int minv, int maxu, int maxv);

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
  STDMETHODIMP GetZBufPoint(int x, int y, unsigned long** retval) { *retval = z_buffer + x + y*width; return S_OK; }

  /// Set the texture cache size.
  STDMETHODIMP SetCacheSize (long size);

  /// Dump the texture cache.
  STDMETHODIMP DumpCache (void);

  /// Clear the texture cache.
  STDMETHODIMP ClearCache (void);

  /// Get drawing buffer width
  STDMETHODIMP GetWidth(int& retval) { retval = width; return S_OK; }
  /// Get drawing buffer height
  STDMETHODIMP GetHeight(int& retval) { retval = height; return S_OK; }

  /// Get the IGraphics2D driver.
  STDMETHODIMP Get2dDriver(IGraphics2D** pi) { *pi = m_piG2D; return S_OK; }

  /// Get the ITextureManager.
  STDMETHODIMP GetTextureManager (ITextureManager** pi)
  {
    *pi = (ITextureManager*)txtmgr;
    return S_OK;
  }

  /// Returns S_OK if this driver requires all maps to be PO2.
  STDMETHODIMP NeedsPO2Maps() { return S_FALSE; }
  /// Returns the maximum aspect ratio of maps.
  STDMETHODIMP GetMaximumAspectRatio(int& retval) { retval = 32768; return S_OK; }

  /// Get the colorformat you want.
  STDMETHODIMP GetColormapFormat(G3D_COLORMAPFORMAT& retval) { retval = G3DCOLORFORMAT_ANY; return S_OK; }

  ////// IHaloRasterizer Methods ////

  /// Create a halo of the specified color. This must be destroyed using DestroyHalo.
  STDMETHODIMP CreateHalo(float r, float g, float b, HALOINFO* pRetVal);
  /// Destroy the halo.
  STDMETHODIMP DestroyHalo(HALOINFO haloInfo);

  /// Draw the halo given a center point and an intensity.
  STDMETHODIMP DrawHalo(csVector3* pCenter, float fIntensity, HALOINFO haloInfo);

  /// Test to see if a halo would be visible (but don't attempt to draw it)
  STDMETHODIMP TestHalo(csVector3* pCenter);

  /// Our internal representation of halos.
  struct csG3DSoftwareHaloInfo
  {
    unsigned short* pbuf;
    unsigned char* palpha;
  };

  /// Actually draws a halo the the screen.
  class csHaloDrawer
  {
  public:
    ///
    csHaloDrawer(IGraphics2D* piG2D, float r, float g, float b);
    ///
    ~csHaloDrawer();

    unsigned short* GetBuffer() { return mpBuffer; }
    unsigned char* GetAlphaBuffer() { return mpAlphaBuffer; }

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
    unsigned short* mpBuffer;
    /// the alpha buffer.
    unsigned char* mpAlphaBuffer;
    /// the width of the buffer.
    int mBufferWidth;
    /// The number of bits to shift to get to red (depends on 555 or 565 mode).
    int red_shift;
    /// The mask for green (depends on 555 or 565 mode).
    int green_mask;
    /// The bits in one byte that are NOT used for green.
    int not_green_bits;

    void drawline_vertical(int x, int y1, int y2);
    void drawline_outerrim(int x1, int x2, int y);
    void drawline_innerrim(int x1, int x2, int y);
  };

  void SysPrintf (int mode, char* str, ...);

  DECLARE_IUNKNOWN ()
  DECLARE_INTERFACE_TABLE (csGraphics3DSoftware)

  /// the COM composite interface for IConfig.
  DECLARE_COMPOSITE_INTERFACE (XConfig3DSoft)
};

class csGraphics3DSoftwareFactory : public IGraphicsContextFactory
{
    /// Create the graphics context
    STDMETHODIMP CreateInstance( REFIID riid, ISystem* piSystem, void** ppv );

    /// Lock or unlock from memory.
    STDMETHODIMP LockServer(BOOL bLock);

    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE( csGraphics3DSoftwareFactory )
};

#endif // __SOFT_G3D_H__
