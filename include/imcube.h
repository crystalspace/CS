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

#ifndef __IMESHCUBE_H__
#define __IMESHCUBE_H__

#include "csutil/scf.h"
#include "iplugin.h"

struct iMaterialWrapper;

SCF_VERSION (iCubeMeshObject, 0, 0, 2);

/**
 * This interface describes the API for the cube mesh object.
 * Using this you can set up the cube to whatever (cube) shape you
 * want it to have and the appearance. The cube plugin implements
 * this interface in addition to iMeshObjectFactory.
 */
struct iCubeMeshObject : public iBase
{
  /// Set size of cube.
  virtual void SetSize (float size) = 0;
  /// Get size of cube.
  virtual float GetSize () = 0;
  /// Set material of cube.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of cube.
  virtual iMaterialWrapper* GetMaterialWrapper () = 0;
  /// Set mix mode.
  virtual void SetMixMode (UInt mode) = 0;
  /// Get mix mode.
  virtual UInt GetMixMode () = 0;
};

#endif

