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

#ifndef __IENGINE_SKELBONE_H__
#define __IENGINE_SKELBONE_H__

#include "csutil/scf.h"

class csTransform;

SCF_VERSION (iSkeletonBone, 0, 0, 2);

/**
 * A bone in the skeleton system. This is a seperate interface
 * because it is used by the motion manager seperatelly and it is
 * possible that other skeletal systems also implement iSkeletonBone
 * (without having to implement iSkeletonConnectionState).
 */
struct iSkeletonBone : public iBase
{
  /// Get the next sibling of this bone.
  virtual iSkeletonBone* GetNext () = 0;
  /// Get the children of this bone.
  virtual iSkeletonBone* GetChildren () = 0;
  /// Get the name of this bone.
  virtual const char* GetName () = 0;
  /// Set the transformation used for this bone.
  virtual void SetTransformation (const csTransform& tr) = 0;
  /// Get the transformation.
  virtual csTransform& GetTransformation () = 0;
};

#endif

