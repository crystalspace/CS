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

#ifndef __IMESHOBJ_H__
#define __IMESHOBJ_H__

#include "csutil/scf.h"
#include "csgeom/box.h"
#include "iplugin.h"

struct iRenderView;

SCF_VERSION (iMeshObject, 0, 0, 1);

/**
 * This is a general mesh object that the engine can interact with.
 */
struct iMeshObject : public iBase
{
  /**
   * Draw this mesh object. Returns false if not visible.
   * If this function returns true it does not mean that the object
   * is invisible. It just means that this MeshObject thinks that the
   * object was probably visible.
   */
  virtual bool Draw (iRenderView* rview) = 0;
};

SCF_VERSION (iMeshObjectFactory, 0, 0, 1);

struct iMeshObjectFactory : public iPlugIn
{
  /// Create an instance of iMeshObject.
  virtual iMeshObject* NewInstance () = 0;
};

#endif

