/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Authored by Brandon Ehle

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

#ifndef __CS_IVARIA_VIEW_H__
#define __CS_IVARIA_VIEW_H__

/**\file
 * Renderer view interface
 */

#include "csutil/scf_interface.h"

struct iCamera;
struct iEngine;
struct iGraphics3D;
struct iClipper2D;

/**
 * The iView class encapsulates the top-level Crystal Space
 * renderer interface. It is basically a camera and a clipper.
 * 
 * Main creators of instances implementing this interface:
 * - Applications using csView.
 *   
 * Main ways to get pointers to this interface:
 * - Application stores it.
 *   
 * Main users of this interface:
 * - Application uses it.
 *   
 */
struct iView : public virtual iBase
{
  SCF_INTERFACE(iView, 2,0,0);
  /// Get engine handle.
  virtual iEngine* GetEngine () = 0;
  /// Set engine handle.
  virtual void SetEngine (iEngine* e) = 0;

  /// Get current camera.
  virtual iCamera* GetCamera () = 0;
  /// Set current camera.
  virtual void SetCamera (iCamera* c) = 0;

  /// Get Context
  virtual iGraphics3D* GetContext () = 0;
  /// Set Context
  virtual void SetContext (iGraphics3D *ig3d) = 0;

  /// Set clipping rectangle.
  virtual void SetRectangle (int x, int y, int w, int h) = 0;
  /// Clear clipper in order to start building a polygon-based clipper.
  virtual void ClearView () = 0;
  /// Add a vertex to clipping polygon (non-rectangular clipping).
  virtual void AddViewVertex (int x, int y) = 0;
  /// Clip the view clipper to the screen boundaries
  virtual void RestrictClipperToScreen () = 0;

  /// Update the Clipper. This is usually called from Draw.
  virtual void UpdateClipper () = 0;
  /// Return the current clipper. This function may call UpdateClipper ().
  virtual iClipper2D* GetClipper () = 0;
  /// Draw 3D world as seen from the camera.
  virtual void Draw () = 0;
  
  /**
   * Enable / Disable automatic resizing. When this
   * is true (default) then the view will detect automatically
   * when the window size changes and adapt the view
   * and camera automatically (i.e. it will change the view
   * rectangle and perspective center). If you don't want that
   * then you can disable this.
   */
  virtual void SetAutoResize (bool state) = 0;
};

#endif // __CS_IVARIA_VIEW_H__
