/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef CAMERA_H
#define CAMERA_H

#include "csutil/scf.h"
#include "csgeom/transfrm.h"
#include "icamera.h"

#define VEC_FORWARD   csVector3(0,0,1)
#define VEC_BACKWARD  csVector3(0,0,-1)
#define VEC_RIGHT     csVector3(1,0,0)
#define VEC_LEFT      csVector3(-1,0,0)
#define VEC_UP        (-csVector3(0,-1,0))
#define VEC_DOWN      (-csVector3(0,1,0))

#define VEC_ROT_RIGHT  csVector3(0,1,0)
#define VEC_ROT_LEFT   csVector3(0,-1,0)
#define VEC_TILT_RIGHT (-csVector3(0,0,1))
#define VEC_TILT_LEFT  (-csVector3(0,0,-1))
#define VEC_TILT_UP    (-csVector3(1,0,0))
#define VEC_TILT_DOWN  (-csVector3(-1,0,0))

class csSector;
class csPolygon3D;
class Vertex;
class csWorld;
class Dumper;

/**
 * A camera positioned in the 3D world.
 */
class csCamera : public csOrthoTransform, public iCamera
{
  friend class Dumper;

  /// The sector the camera is in.
  csSector* sector;
  /// If true we are in a mirrored world.
  bool mirror;

public:
  ///
  int aspect;
  static int default_aspect;
  ///
  float inv_aspect;
  static float default_inv_aspect;
  ///
  float shift_x;
  float shift_y;

  ///
  csCamera ();
  /// copy constructor
  csCamera (csCamera* c);
  ///
  virtual ~csCamera ();

  /**
   * Check if there is a polygon in front of us in the direction
   * defined by 'v' (world space coordinates). Return the nearest polygon.
   */
  csPolygon3D* GetHit (csVector3& v);

  /// Set the default FOV for new cameras.
  static void SetDefaultFOV (int fov)
  {
    default_aspect = fov;
    default_inv_aspect = 1.0f / default_aspect;
  }

  /// Get the default FOV for new cameras.
  static int GetDefaultFOV () { return default_aspect; }

  /// Set the FOV for this camera.
  void SetFOV (int a) { aspect = a; inv_aspect = 1.0f / a; }
  /// Get the FOV for this camera
  int GetFOV () const { return aspect; }

  /**
   * Set the sector that the camera resides in.
   * Note that this function does not check if the current
   * camera location is really in that sector. In fact
   * it is legal to have a camera which is viewing the
   * current sector from outside.
   */
  void SetSector (csSector *s) { sector = s; }

  /**
   * Get the current sector of the camera.
   */
  csSector* GetSector () { return sector; }

  /**
   * Returns true if we are in a mirrored world.
   * Basicly this means that back-face culling will
   * be reversed.
   */
  bool IsMirrored () { return mirror; }

  /**
   * Set the mirrored state of this camera.
   * The effect of this is mainly that back-face culling will
   * be reversed. This is useful if you are stepping into a
   * mirrored sector.
   */
  void SetMirrored (bool m) { mirror = m; }

  /**
   * Sets the absolute position of the camera inside the sector.
   * Vector 'v' is in world space coordinates. This function does
   * not check if the vector is really in the current sector. In
   * fact it is legal to set the position outside the sector
   * boundaries.
   */
  inline void SetPosition (const csVector3& v) { SetOrigin (v); }

  /**
   * Set the world to camera transformation matrix.
   * This basicly defines the direction that the camera looks.
   */
  inline void SetW2C (const csMatrix3& m) { SetO2T (m); }

  /**
   * Set the camera to world transformation matrix.
   * This basicly defines the direction that the camera looks.
   */
  inline void SetC2W (const csMatrix3& m) { SetT2O (m); }

  /**
   * Return the world to camera transformation matrix.
   */
  inline csMatrix3 GetW2C () const { return GetO2T (); }

  /**
   * Return the camera to world transformation matrix.
   */
  inline csMatrix3 GetC2W () const { return GetT2O (); }

  /**
   * Return the world to camera translation.
   */
  inline csVector3 GetW2CTranslation () const { return GetO2TTranslation (); }

  /**
   * Transform a worldspace point to camera space.
   */
  inline csVector3 World2Camera (const csVector3& v) const { return Other2This (v); }

  /**
   * Transform a camera space point to world space.
   */
  inline csVector3 Camera2World (const csVector3& v) const { return This2Other (v); }

  /**
   * Transform a camera space point to worldspace, relative to camera position.
   */
  inline csVector3 Camera2WorldRelative (const csVector3& v) const { return This2OtherRelative (v); }

  /**
   * Moves the camera a relative amount in world coordinates.
   */
  void MoveWorld (const csVector3& v);

  /**
   * Moves the camera a relative amount in camera coordinates.
   */
  void Move (const csVector3& v) { MoveWorld (m_t2o * v); }

  /**
   * Moves the camera a relative amount in world coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  void MoveWorldUnrestricted (const csVector3& v) { v_o2t += v; }

  /**
   * Moves the camera a relative amount in camera coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  void MoveUnrestricted (const csVector3& v) { v_o2t += m_t2o * v; }

  /**
   * Rotate the camera by the angle (radians) around the given vector,
   * in world coordinates.
   * Note: this function rotates the camera, not the coordinate system.
   */
  void RotateWorld (const csVector3& v, float angle);

  /**
   * Rotate the camera by the angle (radians) around the given vector,
   * in camera coordinates.
   * Note: this function rotates the camera, not the coordinate system.
   */
  void Rotate (const csVector3& v, float angle);

  /**
   * Use the given transformation matrix, in worldspace,
   * to reorient the camera.
   * Note: this function rotates the camera, not the coordinate system.
   */
  void RotateWorld (const csMatrix3& m) { SetT2O (m * m_t2o); }

  /**
   * Use the given transformation matrix, in camera space,
   * to reorient the camera.
   * Note: this function rotates the camera, not the coordinate system.
   */
  void Rotate (const csMatrix3& m) { SetT2O (m_t2o * m); }

  /**
   * Have the camera look at the given (x,y,z) point, using up as
   * the up-vector.
   */
  void LookAt (const csVector3& v, const csVector3& up);

  /**
   * Eliminate roundoff error by snapping the camera orientation to a
   * grid of density n
   */
  void Correct (int n);

  /// Change the shift for perspective correction.
  void SetPerspectiveCenter (float x, float y) { shift_x = x; shift_y = y; }

  ///------------------------ iCamera implementation ------------------------
  DECLARE_IBASE;
  ///
  virtual float GetAspect ();
  ///
  virtual float GetInvAspect ();

private:
  ///
  void Correct (int n, float* vals[]);
};

#define GetICameraFromCamera(a)  &a->m_xCamera
#define GetCameraFromICamera(a)  ((csCamera*)((size_t)a - offsetof(csCamera, m_xCamera)))

#endif /*CAMERA_H*/
