/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
  
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

#ifndef __ICAMERA_H__
#define __ICAMERA_H__

#include "csutil/scf.h"
#include "csgeom/transfrm.h"

#define VEC_FORWARD   csVector3(0,0,1)
#define VEC_BACKWARD  csVector3(0,0,-1)
#define VEC_RIGHT     csVector3(1,0,0)
#define VEC_LEFT      csVector3(-1,0,0)
#define VEC_UP        csVector3(0,1,0)
#define VEC_DOWN      csVector3(0,-1,0)

#define VEC_ROT_RIGHT  csVector3(0,1,0)
#define VEC_ROT_LEFT   csVector3(0,-1,0)
#define VEC_TILT_RIGHT (-csVector3(0,0,1))
#define VEC_TILT_LEFT  (-csVector3(0,0,-1))
#define VEC_TILT_UP    (-csVector3(1,0,0))
#define VEC_TILT_DOWN  (-csVector3(-1,0,0))

class csCamera;
struct iSector;

SCF_VERSION (iCamera, 0, 0, 7);

/// Camera class.
struct iCamera : public iBase
{
  /// Ugly@@@.
  virtual csCamera* GetPrivateObject () = 0;
  ///
  virtual float GetFOV () = 0;
  ///
  virtual float GetInvFOV () = 0;
  ///
  virtual float GetFOVAngle () = 0;
  ///
  virtual float GetShiftX () = 0;
  ///
  virtual float GetShiftY () = 0;
  /**
   * Sets the absolute position of the camera inside the sector.
   * Vector 'v' is in world space coordinates. This function does
   * not check if the vector is really in the current sector. In
   * fact it is legal to set the position outside the sector
   * boundaries.
   */
  virtual void SetPosition (const csVector3& v) = 0;
  /**
   * Moves the camera a relative amount in world coordinates.
   * If 'cd' is true then collision detection with objects and things
   * inside the sector is active. Otherwise you can walk through objects
   * (but portals will still be correctly checked).
   */
  virtual void MoveWorld (const csVector3& v, bool cd = true) = 0;

  /**
   * Moves the camera a relative amount in camera coordinates.
   */
  virtual void Move (const csVector3& v, bool cd = true) = 0;

  /**
   * Moves the camera a relative amount in world coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveWorldUnrestricted (const csVector3& v) = 0;

  /**
   * Moves the camera a relative amount in camera coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveUnrestricted (const csVector3& v) = 0;

  /**
   * Rotate the camera by the angle (radians) around the given vector,
   * in world coordinates.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void RotateWorld (const csVector3& v, float angle) = 0;

  /**
   * Rotate the camera by the angle (radians) around the given vector,
   * in camera coordinates.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void Rotate (const csVector3& v, float angle) = 0;

  /**
   * Use the given transformation matrix, in worldspace,
   * to reorient the camera.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void RotateWorld (const csMatrix3& m)  = 0;

  /**
   * Use the given transformation matrix, in camera space,
   * to reorient the camera.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void Rotate (const csMatrix3& m)  = 0;

  /**
   * Have the camera look at the given (x,y,z) point, using up as
   * the up-vector. 'v' should be given relative to the position
   * of the camera.
   */
  virtual void LookAt (const csVector3& v, const csVector3& up) = 0;

  /**
   * Eliminate roundoff error by snapping the camera orientation to a
   * grid of density n
   */
  virtual void Correct (int n) = 0;

  /**
   * Get the transform corresponding to this camera.
   */
  virtual csOrthoTransform& GetTransform () = 0;

  /// Return true if space is mirrored.
  virtual bool IsMirrored () = 0;
  /// Set mirrored state.
  virtual void SetMirrored (bool m) = 0;

  /**
   * Get the 3D far plane that should be used to clip all geometry.
   * If this function returns false then this plane is invalid and should
   * not be used. Otherwise it must be used to clip the object before
   * drawing.
   */
  virtual bool GetFarPlane (csPlane3& pl) = 0;

  /**
   * Get the current sector.
   */
  virtual iSector* GetSector () = 0;
};

#endif
