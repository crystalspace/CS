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

#include "csutil/scf.h"
#include "soft_txt.h"
#include "iconfig.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "iplugin.h"
#include "scan.h"

#if !defined (PROC_INTEL) || defined (NO_ASSEMBLER)
#  undef DO_MMX
#endif

// Max number of fog tables in indexed (8-bit) modes
// This is maximal number of instantly visible fogs without noticeable slowdowns
#define MAX_INDEXED_FOG_TABLES	8

class TextureCache;
class TextureCache16;

struct iGraphics2D;
class csIniFile;
class csPolygonClipper;

/// This structure is used to hold references to all current fog objects.
struct FogBuffer
{
  FogBuffer* next, * prev;
  CS_ID id;
  float density;
  float red, green, blue;
};

///
class csGraphics3DSoftware : public iGraphics3D
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
   * If true the we are really rendering with a smaller size inside a larger window.
   */
  bool do_smaller_rendering;

  /// Buffer for smaller rendering.
  unsigned char* smaller_buffer;

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
  G3DZBufMode z_buf_mode;

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

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

  /// draw_scanline_xxx routines
  csDrawScanline* ScanProc [0x14];
  /// draw_pi_scanline_xxx routines
  csDrawPIScanline* ScanProcPI [4];
  /// draw_pi_scanline_gouraud_xxx routines
  csDrawPIScanlineGouraud* ScanProcPIG [4];
  /// draw_pi_scanline_fx_xxx routines
  csDrawPIScanlineGouraud* ScanProcPIFX [4];

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
  void DrawPolygonFlat (G3DPolygonDPF& poly);

  /// The dynamically built fog tables
  struct
  {
    unsigned char *table;
    int r, g, b;
    int lastuse;
  } fog_tables [MAX_INDEXED_FOG_TABLES];

  /// Build the table used for fog in paletted modes
  unsigned char *BuildIndexedFogTable ();

  /// Set the texture cache size.
  void SetCacheSize (long size);

public:
  DECLARE_IBASE;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DSoftware is in charge of creating and managing this.
   */
  iGraphics2D* G2D;

  /// The configuration file
  csIniFile* config;

  /// The texture manager.
  csTextureManagerSoftware* txtmgr;

  /// The texture cache.
  TextureCache* tcache;

  /// The System interface.
  iSystem* System;

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

  /// Do interlacing? (-1 - no, 0/1 - yes)
  int do_interlaced;
  /**
   * For interlacing. Temporary set to true if we moved quickly. This will decrease
   * the bluriness a little.
   */
  bool ilace_fastmove;

  // An experimental filter feature.
  static int filter_bf;

  ///
  csGraphics3DSoftware (iBase *iParent);
  ///
  virtual ~csGraphics3DSoftware ();

  /// Setup scanline drawing routines according to current bpp and setup flags
  void ScanSetup ();

  ///
  virtual bool Initialize (iSystem *iSys);
  ///
  virtual bool Open (const char *Title);
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

  /// Get the fog mode.
  virtual G3D_FOGMETHOD GetFogMode ()
  { return fogMode; }

  /// Get the fog mode.
  virtual bool SetFogMode (G3D_FOGMETHOD fogm)
  { fogMode = fogm; return true; }

  /**
   * Initiate a volumetric fog object. This function will be called
   * before front-facing and back-facing fog polygons are added to
   * the object. The fog object will be convex but not necesarily closed.
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
   *    <li>CS_FOG_FRONT:       a front-facing polygon
   *    <li>CS_FOG_BACK:        a back-facing polygon
   *    <li>CS_FOG_VIEW:        the view-plane
   * </ul>
   */
  virtual void AddFogPolygon (CS_ID id, G3DPolygonAFP& poly, int fogtype);

  /**
   * Close a volumetric fog object. After the volumetric object is
   * closed it should be rendered on screen (whether you do it here
   * or in DrawFrontFog/DrawBackFog is not important).
   */
  virtual void CloseFogObject (CS_ID id);

  /// Draw a line in camera space.
  virtual void DrawLine (csVector3& v1, csVector3& v2, float fov, int color);

  /// Start a series of DrawPolygonFX
  virtual void StartPolygonFX (iTextureHandle* handle, UInt mode);

  /// Finish a series of DrawPolygonFX
  virtual void FinishPolygonFX ();

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Give a texture to csGraphics3DSoftware to cache it.
  virtual void CacheTexture (iPolygonTexture* texture);

  /**
   * Give a texture to csGraphics3DSoftware to initialize the cache for it.
   * This is used together with the sub-texture optimization and is meant
   * to allocate the space in the cache but not do any actual calculations yet.
   */
  void CacheInitTexture (iPolygonTexture* texture);

  /// Give a sub-texture to csGraphics3DSoftware to cache it.
  void CacheSubTexture (iPolygonTexture* texture, int u, int v);

  /**
   * Give a rectangle to csGraphics3DSoftware so that all sub-textures
   * in this rectangle are cached.
   */
  void CacheRectTexture (iPolygonTexture* texture, int minu, int minv, int maxu, int maxv);

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
  virtual unsigned long *GetZBufPoint(int x, int y)
  { return z_buffer + x + y*width; }

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

  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return txtmgr; }

  /// Returns true if this driver requires all maps to be PO2.
  virtual bool NeedsPO2Maps () { return false; }
  /// Returns the maximum aspect ratio of maps.
  virtual int GetMaximumAspectRatio ()
  { return 32768; }

  /// Get the colorformat you want.
  virtual G3D_COLORMAPFORMAT GetColormapFormat()
  { return G3DCOLORFORMAT_ANY; }

  /// Use to printf through system driver
  void SysPrintf (int mode, char* str, ...);
#ifdef REMOVE_ME_IF_YOU_HAVE_HALOGEN_CPP
  ///------------ iHaloRasterizer interface implementation ------------------
  class csSoftHalo : public iHaloRasterizer
  {
    csPolygonClipper *Clipper;
  public:
    csSoftHalo ();
    virtual ~csSoftHalo ();

    DECLARE_EMBEDDED_IBASE (csGraphics3DSoftware);
    virtual csHaloHandle CreateHalo (float iR, float iG, float iB,
      float iFactor, float iCross);
    virtual void DestroyHalo (csHaloHandle iHalo);
    virtual bool DrawHalo (csVector3 *iCenter, float iIntensity, csHaloHandle iHalo);
    virtual bool TestHalo (csVector3 *iCenter);
    virtual void SetHaloClipper (csVector2 *iClipper, int iCount);
  } scfiHaloRasterizer;
  friend class  csSoftHalo;
  friend struct csSoftHaloHandle;
#endif // REMOVE_ME_IF_YOU_HAVE_HALOGEN_CPP
  ///------------------- iConfig interface implementation -------------------
  struct csSoftConfig : public iConfig
  {
    DECLARE_EMBEDDED_IBASE (csGraphics3DSoftware);
    virtual int GetOptionCount ();
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csSoftConfig;
};

#endif // __SOFT_G3D_H__
