/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __INF_G3D_H__
#define __INF_G3D_H__

// csGraphics3DInfinite infinite rasterizer class.

#include "csutil/scf.h"
#include "csutil/cfgacc.h"
#include "csgeom/transfrm.h"
#include "video/renderer/common/dtmesh.h"
#include "video/renderer/common/dpmesh.h"
#include "video/canvas/common/graph2d.h"
#include "inf_txt.h"
#include "iconfig.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "iplugin.h"

class csClipper;
struct iGraphics2D;
struct iConfigFileNew;

///
class csGraphics3DInfinite : public iGraphics3D
{
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

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

  /// Gather statistics.
  int num_frames;
  int num_drawpolygon;
  int num_drawpolygon_mesh;
  int num_drawpolygonfx;
  int num_drawpolygonfx_mesh;
  int num_drawtrianglemesh;
  int num_drawpolymesh;
  bool in_mesh;
  cs_time total_3d_time;
  cs_time total_2d_time;
  cs_time total_none_time;
  cs_time total_time;

  /// Test for overdraw.
  bool do_overdraw;
  float pixels_drawn;
  float pixels_drawn_fx;
  float screen_pixels;

  /// Emulate fast mesh drawing.
  bool do_fastmesh;

public:
  DECLARE_IBASE;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DInfinite is in charge of creating and managing this.
   */
  iGraphics2D* G2D;

  /// The configuration file
  csConfigAccess config;

  /// The texture manager.
  csTextureManagerInfinite* texman;

  /// The System interface.
  iSystem* System;

  ///
  csGraphics3DInfinite (iBase *iParent);
  ///
  virtual ~csGraphics3DInfinite ();

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

  /// Accurate timer.
  long GetAccurateTime ();

  /// Draw the projected polygon with light and texture.
  virtual void DrawPolygon (G3DPolygonDP& poly);

  ///
  virtual void DrawPolygonDebug (G3DPolygonDP&) { }

  ///
  virtual void OpenFogObject (CS_ID, csFog* ) { }

  ///
  virtual void DrawFogPolygon (CS_ID, G3DPolygonDFP&, int ) { }

  ///
  virtual void CloseFogObject (CS_ID) { }

  /// Draw a line in camera space.
  virtual void DrawLine (const csVector3& v1, const csVector3& v2, float fov, int color);

  /// Start a series of DrawPolygonFX
  virtual void StartPolygonFX (iMaterialHandle*, UInt) { }

  /// Finish a series of DrawPolygonFX
  virtual void FinishPolygonFX () { }

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  ///
  void CacheTexture (iPolygonTexture*) { }

  ///
  void CacheInitTexture (iPolygonTexture*) { }

  ///
  void CacheSubTexture (iPolygonTexture*, int, int) { }

  ///
  void CacheRectTexture (iPolygonTexture*, int, int, int, int) { }

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

  /// Get address of Z-buffer at specific point.
  virtual unsigned long *GetZBuffAt (int, int) { return NULL; }

  /// Dump the texture cache.
  virtual void DumpCache () { }

  /// Clear the texture cache.
  virtual void ClearCache () { }

  /// Remove some polygon from the cache.
  virtual void RemoveFromCache (iPolygonTexture* /*poly_texture*/) { }

  /// Get drawing buffer width.
  virtual int GetWidth () { return width; }
  /// Get drawing buffer height
  virtual int GetHeight () { return height; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);
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
  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh);

  /// Get the iGraphics2D driver.
  virtual iGraphics2D *GetDriver2D ()
  { return G2D; }

  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return texman; }

  /// Get Z-buffer value at given X,Y position
  virtual float GetZBuffValue (int, int) { return 0; }

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float, float, float,
    unsigned char *, int, int) { return NULL; }

  /**
   * Draw a sprite (possibly rescaled to given width (sw) and height (sh))
   * using given rectangle from given texture clipped with G2D's clipper.
   */
  virtual void DrawPixmap (iTextureHandle*, int, int, int, int, int, int,
    int, int, uint8)
  { }

  ///------------------- iConfig interface implementation -------------------
  struct csInfiniteConfig : public iConfig
  {
    DECLARE_EMBEDDED_IBASE (csGraphics3DInfinite);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csInfiniteConfig;
};

/// Infinite version.
class csGraphics2DInfinite : public csGraphics2D
{
public:
  DECLARE_IBASE;

  csGraphics2DInfinite (iBase *iParent);
  virtual ~csGraphics2DInfinite ();

  virtual bool Initialize (iSystem *pSystem);
  virtual bool Open (const char *Title) { return csGraphics2D::Open (Title); }
  virtual void Close () { csGraphics2D::Close (); }

  virtual bool BeginDraw () { return csGraphics2D::BeginDraw (); }

  virtual void Print (csRect *) { }
  virtual void SetRGB (int, int, int, int) { }

  virtual void DrawLine (float, float, float, float, int) { }
  virtual void Clear (int) { }
  virtual void Write (int, int, int, int, const char *) { }

  virtual bool SetMousePosition (int, int) { return true; }
  virtual bool SetMouseCursor (csMouseCursorID) { return true; }
};


#endif // __INF_G3D_H__
