/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef __CS_NULL_G3D_H__
#define __CS_NULL_G3D_H__

// csGraphics3DNull line rasterizer class.

#include "csutil/scf.h"
#include "csutil/cfgacc.h"
#include "csgeom/transfrm.h"
#include "null_txt.h"
#include "iutil/config.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/halo.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/polyrender.h"
#include "plugins/video/renderer/common/polybuf.h"

class csReversibleTransform;
struct iGraphics2D;
struct iConfigFile;

/// The Null renderer.
class csGraphics3DNull : public iGraphics3D
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
  /// Dummy near plane.
  csPlane3 near_plane;

  /// Current aspect ratio for perspective correction.
  float aspect;
  /// Current inverse aspect ratio for perspective correction.
  float inv_aspect;

  /// DrawFlags on last BeginDraw ()
  int DrawMode;

  /// The pixel format
  csPixelFormat pfmt;
  /// Dummy transform.
  csReversibleTransform o2c;

public:
  SCF_DECLARE_IBASE;

  /**
   * Low-level 2D graphics layer.
   * csGraphics3DNull is in charge of creating and managing this.
   */
  csRef<iGraphics2D> G2D;

  /// The configuration file
  csConfigAccess config;

  /// The texture manager.
  csTextureManagerNull* texmgrnull;
  /// The vertex buffer manager.
  csPolArrayVertexBufferManager* vbufmgr;

  /// The System interface.
  iObjectRegistry* object_reg;

  ///
  csGraphics3DNull (iBase *iParent);
  ///
  virtual ~csGraphics3DNull ();

  ///
  virtual bool Initialize (iObjectRegistry *object_reg);
  bool HandleEvent (iEvent&);
  ///
  virtual bool Open ();
  ///
  virtual void Close ();

  /// Change the dimensions of the display.
  virtual void SetDimensions (int width, int height);

  /// Start a new frame (see CSDRAW_XXX bit flags)
  virtual bool BeginDraw (int DrawFlags);

  /// End the frame and do a page swap.
  virtual void FinishDraw ();

  /// Print the image in backbuffer
  virtual void Print (csRect const* area);

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

  /// Open a clipped portal.
  virtual void OpenPortal (size_t, const csVector2*, const csPlane3&, bool) { }

  /// Close a portal previously opened with OpenPortal().
  virtual void ClosePortal (bool) { }

  /// Draw a line in camera space.
  virtual void DrawLine (const csVector3& v1, const csVector3& v2,
  	float fov, int color);

  /// Draw a polygon with special effects.
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly);

  /// Set a renderstate boolean.
  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val);

  /// Get a renderstate value.
  virtual long GetRenderState (G3D_RENDERSTATEOPTION op) const;

  virtual bool SetOption (const char*, const char*);

  /**
   * Get the current driver's capabilities. Each driver implements their
   * own function.
   */
  virtual const csGraphics3DCaps *GetCaps () const
  { return &Caps; }

  /// Get address of Z-buffer at specific point.
  virtual uint32 *GetZBuffAt (int, int) { return 0; }

  /// Dump the texture cache.
  virtual void DumpCache () { }

  /// Clear the texture cache.
  virtual void ClearCache () { }

  /// Remove some polygon from the cache.
  virtual void RemoveFromCache (iRendererLightmap* /*rlm*/) { }

  /// Get drawing buffer width.
  virtual int GetWidth () const { return width; }
  /// Get drawing buffer height
  virtual int GetHeight () const { return height; }
  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y);
  /// Get center of projection.
  virtual void GetPerspectiveCenter (int& x, int& y) const
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
  virtual float GetPerspectiveAspect () const
  {
    return aspect;
  }
  /// Set world to camera transformation.
  virtual void SetObjectToCamera (csReversibleTransform* /*o2c*/)
  { }
  /// Get world to camera transformation.
  virtual const csReversibleTransform& GetObjectToCamera ()
  { return o2c; }

  /// Set optional clipper.
  virtual void SetClipper (iClipper2D*, int) { }
  /// Get clipper.
  virtual iClipper2D* GetClipper () { return 0; }
  /// Get cliptype.
  virtual int GetClipType () const { return CS_CLIPPER_NONE; }

  /// Set near clip plane.
  virtual void SetNearPlane (const csPlane3&) { }

  /// Reset near clip plane (i.e. disable it).
  virtual void ResetNearPlane () { }

  /// Get near clip plane.
  virtual const csPlane3& GetNearPlane () const { return near_plane; }

  /// Return true if we have near plane.
  virtual bool HasNearPlane () const { return false; }


  /// Draw a triangle mesh.
  virtual void DrawTriangleMesh (G3DTriangleMesh& /*mesh*/)
  { }

  /// Draw a polygon mesh.
  virtual void DrawPolygonMesh (G3DPolygonMesh& /*mesh*/)
  { }

  /// Get the iGraphics2D driver.
  virtual iGraphics2D *GetDriver2D ()
  { return G2D; }

  /// Get the ITextureManager.
  virtual iTextureManager *GetTextureManager ()
  { return texmgrnull; }

  /// Get the vertex buffer manager.
  virtual iVertexBufferManager* GetVertexBufferManager ()
  { return vbufmgr; }

  /// Check if lightmap is not too large
  virtual bool IsLightmapOK (int, int, int)
  { return true; }

  virtual void SetRenderTarget (iTextureHandle*, bool) { }
  virtual iTextureHandle* GetRenderTarget () const { return 0; }

  /// Get Z-buffer value at given X,Y position
  virtual float GetZBuffValue (int, int) { return 0; }

  /// Create a halo of the specified color and return a handle.
  virtual iHalo *CreateHalo (float, float, float,
    unsigned char *, int, int) { return 0; }

  /**
   * Draw a sprite (possibly rescaled to given width (sw) and height (sh))
   * using given rectangle from given texture clipped with G2D's clipper.
   */
  virtual void DrawPixmap (iTextureHandle*, int, int, int, int, int, int,
    int, int, uint8);

  //========================================================================
  // All stuff below is only to be compatible with the NR api.
  //========================================================================
  virtual csPtr<iRenderBuffer> CreateRenderBuffer (size_t, 
    csRenderBufferType, csRenderBufferComponentType, 
    int, bool copy = true) { return 0; }
  virtual csPtr<iRenderBuffer> CreateIndexRenderBuffer (size_t, 
    csRenderBufferType, csRenderBufferComponentType, 
    size_t, size_t, bool copy = true) { return 0; }
  virtual void CreateInterleavedRenderBuffers (size_t size, 
    csRenderBufferType type, int count, csRefArray<iRenderBuffer>& buffers) { }
  virtual void SetBufferState (csVertexAttrib*,
  	iRenderBuffer**, int count) { }
  virtual void SetTextureState (int*, iTextureHandle**,
  	int) { }
  virtual void DrawMesh (const csCoreRenderMesh*, const csRenderMeshModes&,
        const csArray< csArray<csShaderVariable*> >&) {}
  virtual void SetWriteMask (bool, bool, bool, bool) { }
  virtual void GetWriteMask (bool &, bool &, bool &,
	bool &) const { }
  virtual void SetZMode (csZBufMode) { }
  virtual void EnableZOffset () { }
  virtual void DisableZOffset () { }
  virtual void SetShadowState (int) { }
  virtual csPtr<iPolygonRenderer> CreatePolygonRenderer () { return 0; }
  virtual void SetWorldToCamera (csReversibleTransform* wvmatrix) {}
  virtual void DrawSimpleMesh (const csSimpleRenderMesh& mesh, uint flags = 0) 
  {}
  virtual csZBufMode GetZMode () { return CS_ZBUF_NONE; }
  //========================================================================

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGraphics3DNull);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
  struct EventHandler : public iEventHandler
  {
  private:
    csGraphics3DNull* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csGraphics3DNull* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& ev) { return parent->HandleEvent (ev); }
  } * scfiEventHandler;
};

#endif // __CS_NULL_G3D_H__
