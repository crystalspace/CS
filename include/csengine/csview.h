/*
    CrystalSpace 3D renderer view
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

#ifndef CSVIEW_H
#define CSVIEW_H

#include "csutil/csbase.h"
#include "csgeom/math2d.h"
#include "csgeom/box.h"

class csPolygon2D;
class csCamera;
class csWorld;
class csSector;
class csClipper;
struct iGraphics3D;

/**
 * The csView class encapsulates the top-level Crystal Space
 * renderer interface. It is basicly a camera and a clipper.
 */
class csView : public csBase
{
private:
  // csClipper.
  csClipper* clipper;
  //
  iGraphics3D* G3D;

  // Clipping rectangle.
  csBox bview;
  // Clipping region.
  csPolygon2D *view;

  // csCamera.
  csCamera *camera;
  // World.
  csWorld *world;

public:
  /// Constructor.
  csView (csWorld *iWorld, iGraphics3D* ig3d);
  /// Destructor.
  ~csView ();

  /// Get world.
  csWorld* GetWorld () { return world; }
  /// Get current camera.
  csCamera* GetCamera () { return camera; }
  /// Set current camera.
  void SetCamera (csCamera* c) { camera = c; }

  /// Clear clipping polygon.
  void ClearView ();
  /// Set clipping rectangle.
  void SetRectangle (int x, int y, int w, int h);
  /// Add a vertex to clipping polygon (non-rectangular clipping).
  void AddViewVertex (int x, int y);
  /// Draw world as seen from the camera.
  void Draw ();
  /// Set sector for the current camera.
  void SetSector (csSector *sector);

  /// Return the clipper.
  csClipper* GetClipper () { return clipper; }

  /**
   * Change the shift for perspective correction.
   */
  void SetPerspectiveCenter (float x, float y);
};

#endif //  CSVIEW_H

