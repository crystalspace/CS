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

#ifndef __IMESH_CUBE_H__
#define __IMESH_CUBE_H__

#include "csutil/scf.h"

struct iMaterialWrapper;

SCF_VERSION (iCubeFactoryState, 0, 0, 4);

/**
 * This interface describes the API for the cube mesh object.
 * Using this you can set up the cube to whatever (cube) shape you
 * want it to have and the appearance. The cube plugin implements
 * this interface in addition to iMeshObjectFactory.
 */
struct iCubeFactoryState : public iBase
{
  /// Set size of cube.
  virtual void SetSize (float sizex, float sizey, float sizez) = 0;
  /// Get size of cube.
  virtual float GetSizeX () const = 0;
  /// Get size of cube.
  virtual float GetSizeY () const = 0;
  /// Get size of cube.
  virtual float GetSizeZ () const = 0;
  /// Set shift of cube.
  virtual void SetShift (float shiftx, float shifty, float shiftz) = 0;
  /// Get shift of cube.
  virtual float GetShiftX () const = 0;
  /// Get shift of cube.
  virtual float GetShiftY () const = 0;
  /// Get shift of cube.
  virtual float GetShiftZ () const = 0;
  /// Set material of cube.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of cube.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;
};

#endif

