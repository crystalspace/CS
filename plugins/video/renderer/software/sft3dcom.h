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

#ifndef __SFT3DCOM_H__
#define __SFT3DCOM_H__

#include "csutil/scf.h"
#include "csgeom/transfrm.h"
#include "soft_txt.h"
#include "video/renderer/common/dtmesh.h"
#include "video/renderer/common/dpmesh.h"
#include "csutil/cfgacc.h"
#include "scan.h"
#include "ivideo/halo.h"
#include "isys/plugin.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

class csTextureCacheSoftware;
struct iConfigFile;

// Maximum number of fog tables in indexed (8-bit) modes.  This is maximum
// number of instantly visible fogs without noticeable slowdowns.
#define MAX_INDEXED_FOG_TABLES	8

/// This structure is used to hold references to all current fog objects.
struct FogBuffer
{
  FogBuffer *next, *prev;
  CS_ID id;
  float density;
  float red, green, blue;
};

/**
 * The basic software renderer class.
 * This class is the parent for both "normal" software renderer
 * as well as for procedural texture class.
 */
class csGraphics3DSoftwareCommon : public iGraphics3D
{
protected:
  friend class csSoftHalo;

  /// Driver this driver is sharing info with (if any)
  csGraphics3DSoftwareCommon *partner;

  /// if this is a procedural texture manager
  bool is_for_procedural_textures;

  /// ID/window name of this context
  const char *title;

  /// Z buffer.
  uint32* z_buffer;
  /// Size of Z buffer.
  long z_buf_size;

  /**
   * Addresses of all lines for this frame. This table is used to avoid
   * calls to GetPixelAt in the main renderer loop. It is filled every frame
   * (by BeginDraw()) because double buffering or other stuff can change
   * the addresses.
   */
  UByte** line_table;

  /// If true then really rendering with a smaller size inside a larger window.
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
  csZBufMode z_buf_mode;

  /// Values to check if we have to reinit StartPolygonFX.
  bool dpfx_valid;
  bool dpfx_use_fog;
  iMaterialHandle* dpfx_mat_handle;
  UInt dpfx_mixmode;
  csZBufMode dpfx_z_buf_mode;

  /// Alpha mask used for 16-bit mode.
  UShort alpha_mask;

  /// Fog buffers.
  FogBuffer* fog_buffers;

  /// Width of display.
  int display_width;
  /// Height of display.
  int display_height;

  /// pseudo width of display.
  int width;
  /// pseudo height of display.
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
  /// Current transformation from world to camera.
  csReversibleTransform o2c;

  /// Current 2D clipper.
  iClipper2D* clipper;
  /// Clipper type.
  int cliptype;
  /// Current near plane.
  csPlane3 near_plane;
  /// Is near plane used?
  bool do_near_plane;

  /// Current aspect ratio for perspective correction.
  float aspect;
  /// Current inverse aspect ratio for perspective correction.
  float inv_aspect;

  /// Mipmap selection coefficient (normal == 1.0)
  float mipmap_coef;

  /// Do we want mipmaps?
  int rstate_mipmap;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;
  /// Gamma correction value (16.16 format)
  int Gamma;

  /// scan_xxx routines
  csDrawScanline* ScanProc [0x28];
  /// scan_pi_xxx routines
  csDrawPIScanline* ScanProcPI [0x2c];
  /// scan_pi_xxx routines
  csDrawPIScanlineGouraud* ScanProcPIG [0x28];

  /// The routine for getting the address of needed scanline_xxx_alpha
  csDrawScanline* (*ScanProc_Alpha) (csGraphics3DSoftwareCommon*, int alpha);

  /// ScanProc_Alpha for 8 bpp modes
  static csDrawScanline* ScanProc_8_Alpha (csGraphics3DSoftwareCommon*,
    int alpha);
  /// ScanProc_Alpha for 16 bpp modes
  static csDrawScanline* ScanProc_16_Alpha (csGraphics3DSoftwareCommon*,
    int alpha);
  /// ScanProc_Alpha for 32 bpp modes
  static csDrawScanline* ScanProc_32_Alpha (csGraphics3DSoftwareCommon*,
    int alpha);

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
    unsigned long r, g, b;
    int lastuse;
  } fog_tables [MAX_INDEXED_FOG_TABLES];

  /// Build the table used for fog in paletted modes
  unsigned char *BuildIndexedFogTable ();

  /// Start a series of DrawPolygonFX
  void RealStartPolygonFX (iMaterialHandle* handle, UInt mode,
  	bool use_fog);

public:
  SCF_DECLARE_IBASE;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DSoftwareCommon is in charge of creating and managing this.
   */
  iGraphics2D* G2D;

  /// The configuration file
  csConfigAccess config;

  /// The texture manager.
  csTextureManagerSoftware* texman;

  /// The texture cache.
  csTextureCacheSoftware *tcache;

  /// The System interface.
  iSystem* System;

  /// Option variable: do texture lighting?
  bool do_lighting;
  /// Option variable: render alpha-transparent textures?
  bool do_alpha;
  /// Option variable: render textures?
  bool do_textured;
  /// Option variable: do very expensive bilinear filtering? (0/1/2)
  unsigned char bilinear_filter;
  /// Do we want Gouraud Shaded polygons?
  bool do_gouraud;

  /// Do interlacing? (-1 - no, 0/1 - yes)
  int do_interlaced;
  /**
   * For interlacing.  Temporary set to true if we moved quickly.  This will
   * decrease the bluriness a little.
   */
  bool ilace_fastmove;

  /// Render capabilities
  csGraphics3DCaps Caps;

  // An experimental filter feature.
  static int filter_bf;

  /// Setup scanline drawing routines according to current bpp and setup flags
  csGraphics3DSoftwareCommon (iBase* parent);
  /// Destructor.
  virtual ~csGraphics3DSoftwareCommon ();

  /**
   * Initialization method required by iPlugin interface.  Sets System pointer.
   */
  virtual bool Initialize (iSystem*);

  /// Initialize new state from config file
  void NewInitialize ();

  /**
   * Initialize state from other driver (used when there are multiple contexts
   * in the system).
   */
  void SharedInitialize (csGraphics3DSoftwareCommon *p);

  /// Open a canvas with given title.
  virtual bool Open (const char *Title);

  /// Gathers all that has to be done when opening from scratch.
  bool NewOpen ();

  /**
   * Used when multiple contexts are in system, opens sharing information from
   * other driver.
   */
  bool SharedOpen ();

  /// Scan setup.
  void ScanSetup ();
  /// Close.
  virtual void Close ();

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags);

  virtual void Print (csRect *area);
  /// End the frame and do a page swap.
  virtual void FinishDraw ();
  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly);

  /**
   * Draw the projected polygon with light and texture.  Debugging version.
   * This one does not actually draw anything but it just prints debug
   * information about what it would have done.
   */
  virtual void DrawPolygonDebug (G3DPolygonDP& poly);

  /**
   * Initiate a volumetric fog object.  This function will be called before
   * front-facing and back-facing fog polygons are added to the object.  The
   * fog object will be convex but not necesarily closed.  The given CS_ID can
   * be used to identify multiple fog objects when multiple objects are
   * started.
   */
  virtual void OpenFogObject (CS_ID id, csFog* fog);

  /**
   * Add a front or back-facing fog polygon in the current fog object.  Note
   * that it is guaranteed that all back-facing fog polygons will have been
   * added before the first front-facing polygon.  fogtype can be:
   * <ul>
   *    <li>CS_FOG_FRONT:       a front-facing polygon
   *    <li>CS_FOG_BACK:        a back-facing polygon
   *    <li>CS_FOG_VIEW:        the view-plane
   * </ul>
   */
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype);

  /**
   * Close a volumetric fog object.  After the volumetric object is closed it
   * should be rendered on screen (whether you do it here or in
   * DrawFrontFog/DrawBackFog() is not important).
   */
  virtual void CloseFogObject (CS_ID id);

  /// Draw a line in camera space.
  virtual void DrawLine (const csVector3& v1, const csVector3& v2,
    float fov, int color);

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op);

  /**
   * Get the current driver's capabilities.  Each driver implements their own
   * function.
   */
  virtual csGraphics3DCaps *GetCaps ()
  { return &Caps; }

  /// Get address of Z-buffer at specific point
  virtual uint32 *GetZBuffAt (int x, int y)
  { return z_buffer + x + y*width; }

  /// Dump the texture cache.
  virtual void DumpCache ();

  /// Clear the texture cache.
  virtual void ClearCache ();

  /// Remove texture from cache.
  virtual void RemoveFromCache (iPolygonTexture* poly_texture);

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
  virtual void SetClipper (iClipper2D* clipper, int cliptype);
  /// Get clipper.
  virtual iClipper2D* GetClipper ()
  {
    return clipper;
  }
  /// Get cliptype.
  virtual int GetClipType () { return cliptype; }

  /// Set near clip plane.
  virtual void SetNearPlane (const csPlane3& pl)
  {
    do_near_plane = true;
    near_plane = pl;
  }

  /// Reset near clip plane (i.e. disable it).
  virtual void ResetNearPlane () { do_near_plane = false; }

  /// Get near clip plane.
  virtual const csPlane3& GetNearPlane () { return near_plane; }

  /// Return true if we have near plane.
  virtual bool HasNearPlane () { return do_near_plane; }


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

  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return texman; }

  /// Get Z-buffer value at given X,Y position
  virtual float GetZBuffValue (int x, int y);

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char *iAlpha, int iWidth, int iHeight);

  /**
   * Draw a sprite (possibly rescaled to given width (sw) and height (sh))
   * using given rectangle from given texture clipped with G2D's clipper.
   */
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy, int sw,
    int sh, int tx, int ty, int tw, int th, uint8 Alpha);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGraphics3DSoftwareCommon);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

#endif // __SFT3DCOM_H__
