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

#include "csutil/csbase.h"
#include "csgeom/math2d.h"
#include "csgeom/box.h"
#include "iview.h"

class csPolygon2D;
class csCamera;
class csEngine;
class csSector;
class csClipper;
struct iGraphics3D;
struct iCamera;

/**
 * The csView class encapsulates the top-level Crystal Space
 * renderer interface. It is basicly a camera and a clipper.
 */
class csView : public csBase, public iBase
{
private:
  // csClipper.
  csClipper* clipper;
  //
  iGraphics3D* G3D;

  int orig_width, orig_height;

  // Clipping rectangle.
  csBox2 *bview;
  // Clipping region.
  csPolygon2D *pview;

  // csCamera.
  csCamera *camera;
  // iCamera.
  iCamera* icamera;
  // Engine handle.
  csEngine *engine;

  /// Update view on context rescale or change (automatic)
  void UpdateView ();

public:
  /// Constructor.
  csView (csEngine *iEngine, iGraphics3D* ig3d);
  /// Destructor.
  ~csView ();

  /// Get engine handle.
  csEngine* GetEngine () { return engine; }
  /// Set engine handle.
  void SetEngine (csEngine* e) { engine = e; }
  /// Get current camera.
  csCamera* GetCamera () { return camera; }
  /// Set current camera.
  void SetCamera (csCamera* c);

  /// Clear clipping polygon.
  virtual void ClearView ();
  /// Set clipping rectangle.
  virtual void SetRectangle (int x, int y, int w, int h);
  /// Set Context
  void SetContext (iGraphics3D *ig3d);
  /// Add a vertex to clipping polygon (non-rectangular clipping).
  void AddViewVertex (int x, int y);
  /// Update the Clipper. This is usually called from Draw.
  void UpdateClipper();
  /// Draw 3D world as seen from the camera.
  void Draw ();
  /// Set sector for the current camera.
  void SetSector (csSector *sector);

  /// Return the clipper.
  csClipper* GetClipper () { return clipper; }

  /**
   * Change the shift for perspective correction.
   */
  void SetPerspectiveCenter (float x, float y);

  DECLARE_IBASE;

  //------------------------- iView implementation -----------------------//
  class View : public iView
  {
  public:
    DECLARE_EMBEDDED_IBASE (csView);
    virtual void SetSector (iSector* sector);
    virtual iCamera* GetCamera ()
    {
      return scfParent->icamera;
    }
    virtual void SetCamera (iCamera* c);
    virtual void ClearView ()
    {
      scfParent->ClearView ();
    }
    virtual void SetRectangle (int x, int y, int w, int h)
    {
      scfParent->SetRectangle (x, y, w, h);
    }
    virtual void Draw ()
    {
      scfParent->Draw ();
    }
  } scfiView;
  friend class View;
};

#endif // __CS_CSVIEW_H__
