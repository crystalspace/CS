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

#ifndef __LINE_G3D_H__
#define __LINE_G3D_H__

// csGraphics3DLine line rasterizer class.

#include "csutil/scf.h"
#include "csgeom/transfrm.h"
#include "cs3d/common/dtmesh.h"
#include "line_txt.h"
#include "iconfig.h"
#include "igraph2d.h"
#include "igraph3d.h"
#include "ihalo.h"
#include "iplugin.h"

class csClipper;
struct iGraphics2D;
class csIniFile;

///
class csGraphics3DLine : public iGraphics3D
{
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

  /// Current transformation from world to camera.
  csTransform o2c;
  /// Current 2D clipper.
  csClipper* clipper;
  /// Current aspect ratio for perspective correction.
  float aspect;
  /// Current inverse aspect ratio for perspective correction.
  float inv_aspect;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

public:
  DECLARE_IBASE;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DLine is in charge of creating and managing this.
   */
  iGraphics2D* G2D;

  /// The configuration file
  csIniFile* config;

  /// The texture manager.
  csTextureManagerLine* texman;

  /// The System interface.
  iSystem* System;

  ///
  csGraphics3DLine (iBase *iParent);
  ///
  virtual ~csGraphics3DLine ();

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

  ///
  virtual void DrawPolygonDebug (G3DPolygonDP&) { }

  /// Get the fog mode.
  virtual G3D_FOGMETHOD GetFogMode () { return G3DFOGMETHOD_NONE; }

  /// Get the fog mode.
  virtual bool SetFogMode (G3D_FOGMETHOD) { return true; }

  ///
  virtual void OpenFogObject (CS_ID, csFog* ) { }

  ///
  virtual void AddFogPolygon (CS_ID, G3DPolygonAFP&, int ) { }

  ///
  virtual void CloseFogObject (CS_ID) { }

  /// Draw a line in camera space.
  virtual void DrawLine (csVector3& v1, csVector3& v2, float fov, int color);

  /// Start a series of DrawPolygonFX
  virtual void StartPolygonFX (iTextureHandle*, UInt) { }

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
  virtual void GetCaps (G3D_CAPS *caps);

  /// Get address of Z-buffer at specific point.
  virtual unsigned long *GetZBufPoint (int, int) { return NULL; }

  /// Dump the texture cache.
  virtual void DumpCache () { }

  /// Clear the texture cache.
  virtual void ClearCache () { }

  /// Get drawing buffer width.
  virtual int GetWidth () { return width; }
  /// Get drawing buffer height
  virtual int GetHeight () { return height; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);
  /// Set perspective aspect.
  virtual void SetPerspectiveAspect (float aspect)
  {
    this->aspect = aspect;
    inv_aspect = 1./aspect;
  }
  /// Set world to camera transformation.
  virtual void SetObjectToCamera (csTransform* o2c)
  {
    this->o2c = *o2c;
  }
  /// Set optional clipper.
  virtual void SetClipper (csVector2* vertices, int num_vertices);
  /// Draw a triangle mesh.
  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh)
  {
    DefaultDrawTriangleMesh (mesh, this, o2c, clipper, aspect, width2, height2);
  }

  /// Get the iGraphics2D driver.
  virtual iGraphics2D *GetDriver2D ()
  { return G2D; }

  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return texman; }

  /// Returns true if this driver requires all maps to be PO2.
  virtual bool NeedsPO2Maps () { return false; }
  /// Returns the maximum aspect ratio of maps.
  virtual int GetMaximumAspectRatio ()
  { return 32768; }
  /// Adjust to optimal texture size.
  virtual void AdjustToOptimalTextureSize(int& w, int& h) 
  {(void)w;(void)h;}

  /// Get Z-buffer value at given X,Y position
  virtual float GetZbuffValue (int, int) { return 0; }

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float, float, float,
    unsigned char *, int, int) { return NULL; }

  ///------------------- iConfig interface implementation -------------------
  struct csLineConfig : public iConfig
  {
    DECLARE_EMBEDDED_IBASE (csGraphics3DLine);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
  friend struct csLineConfig;
};

#endif // __LINE_G3D_H__
