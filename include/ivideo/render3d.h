/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

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

#ifndef __IVIDEO_RENDER3D_H__
#define __IVIDEO_RENDER3D_H__


#include "csutil/scf.h"

class csRect;
class csReversibleTransform;
class csStringSet;


class csRenderMesh;

struct iClipper2D;
struct iGraphics2D;
struct iTextureManager;
struct iTextureHandle;
struct iRenderBufferManager;
struct iLightingManager;



/**\name iRender3D::BeginDraw() flags
 * @{ */
/// We're going to draw 2D graphics
#define CSDRAW_2DGRAPHICS   0x00000001
/// We're going to draw 3D graphics
#define CSDRAW_3DGRAPHICS   0x00000002
/// Clear Z-buffer ?
#define CSDRAW_CLEARZBUFFER 0x00000010
/// Clear frame buffer ?
#define CSDRAW_CLEARSCREEN  0x00000020
/** @} */

/**\name Type of clipper (for iGraphics3D::SetClipper())
 * @{ */
/**
 * There is no clipper.
 */
#define CS_CLIPPER_NONE -1
/**
 * Clipper is optional.
 */
#define CS_CLIPPER_OPTIONAL 0
/**
 * Clipper is top-level.
 */
#define CS_CLIPPER_TOPLEVEL 1
/**
 * Clipper is required.
 */
#define CS_CLIPPER_REQUIRED 2
/** @} */

/**\name Clipping requirement for DrawTriangleMesh
 * @{ */
/**
 * No clipping required.
 * (setting for clip_portal, clip_plane, or clip_z_plane).
 */
#define CS_CLIP_NOT 0
/**
 * Clipping may be needed. Depending on the type of the clipper
 * (one of the CS_CLIPPER_??? flags) the renderer has to clip or
 * not. (setting for clip_portal, clip_plane, or clip_z_plane).
 */
#define CS_CLIP_NEEDED 1
/**
 * Clipping is not needed for the current clipper but it might
 * be needed for the toplevel clipper. (setting for clip_portal,
 * will never be used for clip_plane or clip_z_plane).
 */
#define CS_CLIP_TOPLEVEL 2
/** @} */

/// Z-buffer modes
enum csZBufMode
{
  // values below are sometimes used as bit masks, so don't change them!
  /// Don't test/write
  CS_ZBUF_NONE     = 0x00000000,
  /// write
  CS_ZBUF_FILL     = 0x00000001,
  /// test
  CS_ZBUF_TEST     = 0x00000002,
  /// write/test
  CS_ZBUF_USE      = 0x00000003,
  /// only write
  CS_ZBUF_FILLONLY = 0x00000004,
  /// test if equal
  CS_ZBUF_EQUAL    = 0x00000005,
};

class csRender3dCaps
{
};

SCF_VERSION (iRender3D, 0, 0, 1);


/**
 * New 3D Interface. Work in progress!
 */

struct iRender3D : public iBase
{
  /// Open 3d renderer.
  virtual bool Open () = 0;

  /// Close renderer and release all resources used
  virtual void Close () = 0;

  /**
   * Get a pointer to our 2d canvas driver. NOTE: It's not increfed,
   * and therefore it shouldn't be decref-ed by caller.
   */
  virtual iGraphics2D* GetDriver2D () = 0;

  /// Get a pointer to our texture manager
  virtual iTextureManager* GetTextureManager () = 0;

  /**
   * Get a pointer to the VB-manager
   * Always use the manager referenced here to get VBs
   */
  virtual iRenderBufferManager* GetBufferManager () = 0;

  /// Get a pointer to lighting manager
  virtual iLightingManager* GetLightingManager () = 0;

  /// Set dimensions of window
  virtual void SetDimensions (int width, int height) = 0;
  /// Get width of window
  virtual int GetWidth () = 0;
  /// Get height of window
  virtual int GetHeight () = 0;

  /// Capabilities of the driver
  virtual csRender3dCaps* GetCaps () = 0;

  /// Set center of projection.
  virtual void SetPerspectiveCenter (int x, int y) = 0;
  
  /// Get center of projection.
  virtual void GetPerspectiveCenter (int& x, int& y) = 0;
  
  /// Set perspective aspect.
  virtual void SetPerspectiveAspect (float aspect) = 0;

  /// Get perspective aspect.
  virtual float GetPerspectiveAspect () = 0;

  /// Set world to view transform
  virtual void SetObjectToCamera (csReversibleTransform* wvmatrix) = 0;
  virtual csReversibleTransform* GetWVMatrix () = 0;

  /**
   * Set the target of rendering. If this is NULL then the target will
   * be the main screen. Otherwise it is a texture. After calling
   * g3d->FinishDraw() the target will automatically be reset to NULL (main
   * screen). Note that on some implementions rendering on a texture
   * will overwrite the screen. So you should only do this BEFORE you
   * start rendering your frame.
   * <p>
   * If 'persistent' is true then the current contents of the texture
   * will be copied on screen before drawing occurs (in the first
   * call to BeginDraw). Otherwise it is assumed that you fully render
   * the texture.
   */
  virtual void SetRenderTarget (iTextureHandle* handle, 
    bool persistent = false) = 0;

  /// Get the current render target (NULL for screen).
  virtual iTextureHandle* GetRenderTarget () = 0;

  /// Begin drawing in the renderer
  virtual bool BeginDraw (int drawflags) = 0;

  /// Indicate that drawing is finished
  virtual void FinishDraw () = 0;

  /// Do backbuffer printing
  virtual void Print (csRect* area) = 0;

  /// Drawroutine. Only way to draw stuff
  virtual void DrawMesh (csRenderMesh* mymesh,
    csZBufMode z_buf_mode,
    int clip_portal,
    int clip_plane,
    int clip_z_plane) = 0;

  /**
   * Set optional clipper to use. If clipper == null
   * then there is no clipper.
   * Currently only used by DrawTriangleMesh.
   */
  virtual void SetClipper (iClipper2D* clipper, int cliptype) = 0;

  /**
   * Get clipper that was used.
   */
  virtual iClipper2D* GetClipper () = 0;

  /**
   * Return type of clipper.
   */
  virtual int GetClipType () = 0;

  /// Get a stringhash to be used by our streamsources etc.
  virtual csStringSet* GetStringContainer () = 0;
};

#endif // __IVIDEO_RENDER3D_H__

