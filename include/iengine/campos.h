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

#ifndef __IENGINE_CAMPOS_H__
#define __IENGINE_CAMPOS_H__

#include "csutil/scf.h"

class csVector3;
struct iObject;
struct iEngine;
struct iCamera;

SCF_VERSION (iCameraPosition, 0, 0, 2);

/**
 * A camera position.
 */
struct iCameraPosition : public iBase
{
  /// Get the iObject for this camera position.
  virtual iObject *QueryObject() = 0;
  
  virtual const char *GetSector() = 0;
  
  virtual csVector3 GetPosition() = 0;
  
  virtual csVector3 GetUpwardVector() = 0;
  
  virtual csVector3 GetForwardVector() = 0;

  /// Load the camera position into a camera object
  virtual bool Load (iCamera*, iEngine*) = 0;

  virtual void Set (const char *sector, const csVector3 &pos,
      const csVector3 &forward, const csVector3 &upward) = 0;
};

SCF_VERSION (iCameraPositionList, 0, 0, 1);

struct iCameraPositionList : public iBase
{
  /// Return the number of camera positions in this list.
  virtual int GetCameraPositionCount () const = 0;
  /// Return a single camera position.
  virtual iCameraPosition *GetCameraPosition (int idx) const = 0;
  /// Create a new empty camera position.
  virtual iCameraPosition* NewCameraPosition (const char* name) = 0;
  /// Remove a camera position
  virtual void RemoveCameraPosition (iCameraPosition *campos) = 0;
  /// Find a camera position by name
  virtual iCameraPosition *FindByName (const char *name) const = 0;
  /// Find a camera position and return its index
  virtual int Find (iCameraPosition *campos) const = 0;
};

#endif

