/*
    Copyright (C) 2000 by Andrew Zabolotny
  
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

#ifndef __CS_CAMPOS_H__
#define __CS_CAMPOS_H__

#include "csobject/csobject.h"
#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "csgeom/vector3.h"
#include "csengine/camera.h"
#include "iengine/campos.h"

class csEngine;

/**
 * Camera position structure. This structure is used internally by the
 * engine to store named camera positions than can be retrieved by
 * client programs to store starting map points, teleporter positions
 * and so on. In the map file you can use CAMERA (...) keyword to
 * define such points.
 */
class csCameraPosition : public csPObject
{
public:
  /// The sector this camera points to
  char *Sector;
  /// The camera position
  csVector3 Position;
  /// Camera orientation: forward vector
  csVector3 Forward;
  /// Camera orientation: upward vector
  csVector3 Upward;

  /// Initialize the camera position object
  csCameraPosition (const char *iName, const char *iSector,
    const csVector3 &iPosition,
    const csVector3 &iForward, const csVector3 &iUpward);

  /// Destroy this object and free all associated memory
  virtual ~csCameraPosition ();

  /// Change camera position object
  void Set (const char *iSector, const csVector3 &iPosition,
    const csVector3 &iForward, const csVector3 &iUpward);

  /// Load the camera position into a camera object
  bool Load (csCamera&, csEngine*);

  CSOBJTYPE;
  DECLARE_OBJECT_INTERFACE;
  DECLARE_IBASE;

  //--------------------- iCameraPosition implementation ----------------------
  struct CameraPosition : public iCameraPosition
  {
    DECLARE_EMBEDDED_IBASE(csCameraPosition);

    virtual iObject *QueryObject() {return scfParent;}
	
	virtual char *GetSector() { return scfParent->Sector; }

	virtual csVector3 GetPosition() { return scfParent->Position; }

	virtual csVector3 GetUpwardVector() { return scfParent->Upward; }
	
	virtual csVector3 GetForwardVector() { return scfParent->Forward; }

  } scfiCameraPosition;
};

#endif // __CS_CAMPOS_H__
