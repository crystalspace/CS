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

#ifndef __IMESHBALL_H__
#define __IMESHBALL_H__

#include "csutil/scf.h"

struct iMaterialWrapper;

SCF_VERSION (iBallState, 0, 0, 1);

/**
 * This interface describes the API for the ball mesh object.
 * Using this you can set up the ball to whatever (ball) shape you
 * want it to have and the appearance. The ball plugin implements
 * this interface in addition to iMeshObject.
 */
struct iBallState : public iBase
{
  /// Set x, y, and z radius of ball.
  virtual void SetRadius (float radx, float rady, float radz) = 0;
  /// Set shift of ball.
  virtual void SetShift (float shiftx, float shifty, float shiftz) = 0;
  /// Set number of vertices on outer circle of ball.
  virtual void SetRimVertices (int num) = 0;
  /// Set material of ball.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of ball.
  virtual iMaterialWrapper* GetMaterialWrapper () = 0;
  /// Set mix mode.
  virtual void SetMixMode (UInt mode) = 0;
  /// Get mix mode.
  virtual UInt GetMixMode () = 0;
};

#endif

