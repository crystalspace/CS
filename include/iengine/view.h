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

#ifndef __IENGINE_VIEW_H__
#define __IENGINE_VIEW_H__

#include "csutil/scf.h"

SCF_VERSION (iView, 0, 0, 2);

struct iCamera;
struct iSector;

/**
 * The iView class encapsulates the top-level Crystal Space
 * renderer interface. It is basically a camera and a clipper.
 */
struct iView : public iBase
{
  /// Set sector for this view.
  virtual void SetSector (iSector* sector) = 0;
  /// Get current camera.
  virtual iCamera* GetCamera () = 0;
  /// Set current camera.
  virtual void SetCamera (iCamera* c) = 0;
  /// Clear clipping polygon.
  virtual void ClearView () = 0;
  /// Set clipping rectangle.
  virtual void SetRectangle (int x, int y, int w, int h) = 0;
  /// Draw 3D world as seen from the camera.
  virtual void Draw () = 0;
};

#endif

