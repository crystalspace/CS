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

#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "csgeom/vector3.h"
#include "csengine/camera.h"
#include "iengine/campos.h"

struct iEngine;
struct iCamera;

/**
 * A camera position. This object can be used to initialize a camera object to
 * a certain state. It has the following properties: <ul>
 * <li> Home sector name: This name is used to find the home sector of the
 *      camera in the engine when the camera position is applied.
 * <li> Position: Position of the camera
 * <li> Upward and forward vectors: These vectors define the orientation of the
 *      camera. More exactly, they are used to compute the transformation of
 *      the camera.
 * </ul>
 */
class csCameraPosition : public csObject
{
private:
  /// Destroy this object and free all associated memory
  virtual ~csCameraPosition ();

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

  /// Change camera position object
  void Set (const char *iSector, const csVector3 &iPosition,
    const csVector3 &iForward, const csVector3 &iUpward);

  /// Load the camera position into a camera object
  bool Load (iCamera*, iEngine*);

  SCF_DECLARE_IBASE_EXT (csObject);

  //--------------------- iCameraPosition implementation ----------------------
  struct CameraPosition : public iCameraPosition
  {
    SCF_DECLARE_EMBEDDED_IBASE(csCameraPosition);

    virtual iObject *QueryObject();
    virtual iCameraPosition *Clone () const;
    virtual const char *GetSector();
    virtual void SetSector(const char *Name);
    virtual const csVector3 &GetPosition();
    virtual void SetPosition (const csVector3 &v);
    virtual const csVector3 &GetUpwardVector();
    virtual void SetUpwardVector (const csVector3 &v);
    virtual const csVector3 &GetForwardVector();
    virtual void SetForwardVector (const csVector3 &v);
    virtual void Set (const char *sector, const csVector3 &pos,
      const csVector3 &forward, const csVector3 &upward);
    virtual bool Load (iCamera *c, iEngine *e);
  } scfiCameraPosition;
  friend struct CameraPosition;
};

#endif // __CS_CAMPOS_H__
