/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_PROCMESH_H__
#define __CS_PROCMESH_H__

/**\file
 * Render a mesh on a texture.
 */

#include "csextern.h"

#include "csutil/ref.h"
#include "csutil/scf_implementation.h"
#include "cstool/csview.h"

struct iTextureHandle;

/**
 * This class manages the rendering of a mesh on a texture.
 */
class CS_CRYSTALSPACE_EXPORT csMeshOnTexture
{
private:
  csRef<iGraphics3D> g3d;
  csRef<iEngine> engine;

  // We will view the object through this view.
  csRef<csView> view;

public:
  /**
   * Construct a csMeshOnTexture object. This will also
   * create a private view that will be used to render the
   * object with.
   */
  csMeshOnTexture (iObjectRegistry* object_reg);
  
  /// Destruct.
  virtual ~csMeshOnTexture ();

  /**
   * Get the view that is represented by this mesh on
   * texture instance. This can be used to setup the camera
   * that you want to use here.
   */
  csView* GetView () const { return view; }

  /**
   * With the view camera transformation set at a certain spot
   * in space, this function will try to move the camera as
   * close as possible so that as much of the object is
   * visible at the same time in the given texture resolution.
   */
  void ScaleCamera (iMeshWrapper* mesh, int txtx, int txty);

  /**
   * Look at a given mesh at a given distance. The relative orientation
   * to the mesh remains unchanged.
   */
  void ScaleCamera (iMeshWrapper* mesh, float distance);

  /**
   * Render a mesh on a texture. Returns false on failure.
   * Error will have been reported on the reporter.
   * <p>
   * If 'persistent' is true then the current contents of the texture
   * will be copied on screen before drawing occurs (in the first
   * call to BeginDraw). Otherwise it is assumed that you fully render
   * the texture.
   */
  bool Render (iMeshWrapper* mesh, iTextureHandle* handle,
      bool persistent = false);
};

#endif // __CS_PROCMESH_H__

