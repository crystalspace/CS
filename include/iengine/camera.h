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

#ifndef __IENGINE_CAMERA_H__
#define __IENGINE_CAMERA_H__

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
class csVector3;
class csVector2;
struct iSector;

SCF_VERSION (iCamera, 0, 0, 12);

/// Camera class.
struct iCamera : public iBase
{
  /// Ugly@@@.
  virtual csCamera* GetPrivateObject () = 0;

  ///
  virtual int GetFOV () const = 0;
  ///
  virtual float GetInvFOV () const = 0;
  ///
  virtual float GetFOVAngle () const = 0;
  ///
  virtual void SetFOV (int a, int width) = 0;
  ///
  virtual void SetFOVAngle (float a, int width) = 0;

  ///
  virtual float GetShiftX () const = 0;
  ///
  virtual float GetShiftY () const = 0;
  ///
  virtual void SetPerspectiveCenter (float x, float y) = 0;

  /**
   * Get the transform corresponding to this camera. In this transform,
   * 'other' is world space and 'this' is camera space.
   * WARNING! It is illegal to directly assign to the given transform
   * in order to modify it. To change the entire transform you have
   * to use SetTransform(). Note that it is legal to modify the
   * returned transform otherwise. Just do not assign to it.
   */
  virtual csOrthoTransform& GetTransform () = 0;
  /**
   * Set the transform corresponding to this camera. In this transform,
   * 'other' is world space and 'this' is camera space.
   */
  virtual void SetTransform (const csOrthoTransform& tr) = 0;

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

  /// Get the current sector.
  virtual iSector* GetSector () const = 0;
  /// Move to another sector.
  virtual void SetSector (iSector*) = 0;

  /**
   * Eliminate roundoff error by snapping the camera orientation to a
   * grid of density n
   */
  virtual void Correct (int n) = 0;

  /// Return true if space is mirrored.
  virtual bool IsMirrored () const = 0;
  /// Set mirrored state.
  virtual void SetMirrored (bool m) = 0;

  /**
   * Get the 3D far plane that should be used to clip all geometry.
   * If this function returns false then this plane is invalid and should
   * not be used. Otherwise it must be used to clip the object before
   * drawing.
   */
  virtual bool GetFarPlane (csPlane3& pl) const = 0;

  /**
   * Get the camera number. This number is changed for every new camera
   * instance and it is also updated whenever the camera transformation
   * changes. This number can be used to cache camera vertex arrays, for
   * example.
   */
  virtual long GetCameraNumber () const = 0;

  /// Calculate perspective corrected point for this camera.
  virtual void Perspective (const csVector3& v, csVector2& p) const = 0;
  /// Calculate inverse perspective corrected point for this camera.
  virtual void InvPerspective (const csVector2& p, float z,
  	csVector3& v) const = 0;

  /**
   * If the hit-only-portals flag is true then only portals will be
   * checked with the 'MoveWorld()' function. This is a lot faster
   * but it does mean that you will have to do collision detection with
   * non-portal polygons using another technique. The default for this
   * flag is true.
   */
  virtual void OnlyPortals (bool hop) = 0;

  /// Get the hit-only-portals flag.
  virtual bool GetOnlyPortals () = 0;
};

#endif
