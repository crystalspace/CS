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

#ifndef __ISKELETON_H__
#define __ISKELETON_H__

#include "csutil/scf.h"

struct iSkeletonConnection;
class csTransform;

SCF_VERSION (iSkeletonLimb, 0, 0, 1);

/**
 * This interface describes the API for a limb in the skeleton system.
 */
struct iSkeletonLimb : public iBase
{
  /// Add a vertex index.
  virtual void AddVertex (int v) = 0;
  /// Get the array of vertex indices.
  virtual int* GetVertices () = 0;
  /// Get the number of vertices in the array returned by GetVertices.
  virtual int GetNumVertices () = 0;
  /// Create a new connection and add it as a child.
  virtual iSkeletonConnection* CreateConnection () = 0;
  /// Set the name for this limb.
  virtual void SetName (const char* name) = 0;
  /// Get the name for this limb.
  virtual const char* GetName () const = 0;
  /// Get next sibling in list.
  virtual iSkeletonLimb* GetNextSibling () = 0;
};

SCF_VERSION (iSkeletonConnection, 0, 0, 1);

/**
 * This interface describes the API for a connection in the skeleton system.
 * A connection is also a limb but is implemented in a seperate interface.
 */
struct iSkeletonConnection : public iBase
{
  /// Set the transformation used for this connection.
  virtual void SetTransformation (const csTransform& tr) = 0;
  /// Get the transformation.
  virtual csTransform& GetTransformation () = 0;
};

SCF_VERSION (iSkeleton, 0, 0, 1);

/**
 * This interface describes the API for the main skeleton itself.
 * A skeleton is also a limb but is implemented in a seperate interface.
 */
struct iSkeleton : public iBase
{
};

SCF_VERSION (iSkeletonLimbState, 0, 0, 1);

/**
 * This interface describes the API for a limb state in the skeleton system.
 * The state versions of the skeleton interfaces control the dynamic
 * behaviour of the skeleton. The non-limb versions are the static
 * representation and initial settings of the skeleton.
 */
struct iSkeletonLimbState : public iBase
{
  /// Get the first child of this limb.
  virtual iSkeletonLimbState* GetChildren () = 0;
  /// Get next sibling in list.
  virtual iSkeletonLimbState* GetNextSibling () = 0;
  /// Set the name for this limb.
  virtual void SetName (const char* name) = 0;
  /// Get the name for this limb.
  virtual const char* GetName () const = 0;
  /// Set userdata object.
  virtual void SetUserData (void* data) = 0;
  /// Get userdata object.
  virtual void* GetUserData () = 0;
};

SCF_VERSION (iSkeletonConnectionState, 0, 0, 1);

/**
 * This interface describes the API for a connection state in the skeleton
 * system. A connection is also a limb. This interface is closely
 * related to iSkeletonBone but it is specific to the 3D sprite system
 * in CS.
 */
struct iSkeletonConnectionState : public iBase
{
};

SCF_VERSION (iSkeletonState, 0, 0, 1);

/**
 * This interface describes the API for the skeleton state itself.
 */
struct iSkeletonState : public iBase
{
};

#endif

