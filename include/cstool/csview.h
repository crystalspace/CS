/*
    CrystalSpace 3D renderer view
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_CSVIEW_H__
#define __CS_CSVIEW_H__

/**\file
 * Crystal Space 3D renderer view
 */

#include "csextern.h"

#include "ivaria/view.h"

class csBox2;
class csPoly2D;

/**
 * The csView class encapsulates the top-level Crystal Space
 * renderer interface. It is basically a camera and a clipper.
 */
class CS_CRYSTALSPACE_EXPORT csView : public iView
{
private:
  /// the engine
  csRef<iEngine> Engine;
  /// rendering context
  csRef<iGraphics3D> G3D;
  /// context size at the time the clipper was created
  int OldWidth, OldHeight;

  /// the camera
  csRef<iCamera> Camera;

  /// Rect clipping region (0 if this is a polygon-based clipper)
  csBox2* RectView;
  /// Poly clipping region (0 if this is a rectangular clipper)
  csPoly2D* PolyView;
  /// The prepared clipper
  csRef<iClipper2D> Clipper;

  /// Rescale the clipper to deal with a context resize
  void UpdateView ();

  /// State of the automatic resizing
  bool AutoResize;

public:
  /// Constructor.
  csView (iEngine *iEngine, iGraphics3D* ig3d);
  /// Destructor.
  virtual ~csView ();

  /// Get engine handle.
  virtual iEngine* GetEngine ();
  /// Set engine handle.
  virtual void SetEngine (iEngine* e);

  /// Get current camera.
  virtual iCamera* GetCamera ();
  /// Set current camera.
  virtual void SetCamera (iCamera* c);

  /// Get Context
  virtual iGraphics3D* GetContext ();
  /// Set Context
  virtual void SetContext (iGraphics3D *ig3d);

  /// Set clipping rectangle.
  virtual void SetRectangle (int x, int y, int w, int h);
  /// Clear clipper in order to start building a polygon-based clipper.
  virtual void ClearView ();
  /// Add a vertex to clipping polygon (non-rectangular clipping).
  virtual void AddViewVertex (int x, int y);
  /// Clip the view clipper to the screen boundaries
  virtual void RestrictClipperToScreen ();

  /// Enable / Disable automatic resizing.
  virtual void SetAutoResize (bool state) { AutoResize = state; }

  /// Update the Clipper. This is usually called from Draw.
  virtual void UpdateClipper();
  /// Return the clipper.
  virtual iClipper2D* GetClipper ();
  /// Draw 3D world as seen from the camera.
  virtual void Draw ();

  SCF_DECLARE_IBASE;
};

#endif // __CS_CSVIEW_H__
