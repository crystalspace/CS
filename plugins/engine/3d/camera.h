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

#ifndef __CS_CAMERA_H__
#define __CS_CAMERA_H__

#include "csutil/scf.h"
#include "csgeom/transfrm.h"
#include "csutil/refarr.h"
#include "iengine/camera.h"
#include "plugins/engine/3d/sector.h"

class csEngine;

/**
 * A camera positioned in the 3D world.
 */
class csCamera : public csOrthoTransform, public iCamera
{
private:
  /// The sector the camera is in.
  iSector* sector;
  /// If true we are in a mirrored world.
  bool mirror;

  /// Callbacks.
  csRefArray<iCameraSectorListener> listeners;

  /**
   * If true then we only check collision with portals and not
   * with other polygons.
   */
  bool only_portals;

  /// a farplane to cut everything thats behind it
  csPlane3 *fp;

  ///
  int aspect;
  static int default_aspect;
  ///
  float inv_aspect;
  static float default_inv_aspect;
  ///
  float shift_x;
  float shift_y;

  /// FOV in angles (degrees).
  float fov_angle;
  static float default_fov_angle;

  /// Compute above angle.
  void ComputeAngle (int width);
  static void ComputeDefaultAngle (int width);

  /**
   * Camera number. This number is changed for every new camera
   * instance and it is also updated whenever the camera transformation
   * changes. This number can be used to cache camera vertex arrays, for
   * example.
   */
  long cameranr;
  /**
   * The last used camera number.
   */
  static long cur_cameranr;

  ///
  void Correct (int n, float* vals[]);

public:
  ///
  csCamera ();
  /// Copy constructor.
  csCamera (csCamera* c);
  /// Copy constructor.
  csCamera (const csCamera& c);
  ///
  virtual ~csCamera ();

  /**
   * Get the camera number. This number is changed for every new camera
   * instance and it is also updated whenever the camera transformation
   * changes. This number can be used to cache camera vertex arrays, for
   * example.
   */
  virtual long GetCameraNumber () const
  {
    return cameranr;
  }

  /// Set the default FOV for new cameras.
  static void SetDefaultFOV (int fov, int width)
  {
    default_aspect = fov;
    default_inv_aspect = 1.0f / default_aspect;
    ComputeDefaultAngle (width);
  }

  /// Get the default FOV for new cameras.
  static int GetDefaultFOV () { return default_aspect; }
  /// Get the default inverse FOV for new cameras.
  static float GetDefaultInvFOV () { return default_inv_aspect; }
  /// Get the default FOV in angles (degrees).
  static float GetDefaultFOVAngle () { return default_fov_angle; }

  /// Set the FOV for this camera.
  void SetFOV (int a, int width)
  {
    aspect = a;
    inv_aspect = 1.0f / a;
    ComputeAngle (width);
  }
  /// Get the FOV for this camera
  virtual int GetFOV () const { return aspect; }
  /// Get the inverse FOV for this camera.
  virtual float GetInvFOV () const { return inv_aspect; }

  /// Set the FOV in angles (degrees).
  virtual void SetFOVAngle (float a, int width);
  /// Get the FOV in angles (degrees).
  virtual float GetFOVAngle () const
  {
    return fov_angle;
  }

  /// Get the X shift value.
  virtual float GetShiftX () const { return shift_x; }
  /// Get the Y shift value.
  virtual float GetShiftY () const { return shift_y; }

  /// Set farplane, everything behind this will be cut
  virtual void SetFarPlane (csPlane3* farplane);
  /// Get the Farplane
  virtual csPlane3* GetFarPlane () const { return fp; }

  /**
   * Set the sector that the camera resides in.
   * Note that this function does not check if the current
   * camera location is really in that sector. In fact
   * it is legal to have a camera which is viewing the
   * current sector from outside.
   */
  void SetSector (iSector *s)
  {
    if (sector == s) return;
    sector = s;
    cameranr = cur_cameranr++;
    FireCameraSectorListeners (sector);
  }

  /**
   * Get the current sector of the camera.
   */
  iSector* GetSector () const { return sector; }

  /**
   * Returns true if we are in a mirrored world.
   * Basically this means that back-face culling will
   * be reversed.
   */
  bool IsMirrored () const { return mirror; }

  /**
   * Set the mirrored state of this camera.
   * The effect of this is mainly that back-face culling will
   * be reversed. This is useful if you are stepping into a
   * mirrored sector.
   */
  void SetMirrored (bool m)
  {
    if (mirror != m) cameranr = cur_cameranr++;
    mirror = m;
  }

  /**
   * Set 'other' to 'this' transformation matrix.
   * csCamera overrides this in order to be able to update the
   * camera number.
   */
  virtual void SetO2T (const csMatrix3& m)
  {
    csOrthoTransform::SetO2T (m);
    cameranr = cur_cameranr++;
  }

  /**
   * Set 'this' to 'other' transformation matrix.
   * csCamera overrides this in order to be able to update the
   * camera number.
   */
  virtual void SetT2O (const csMatrix3& m)
  {
    csOrthoTransform::SetT2O (m);
    cameranr = cur_cameranr++;
  }

  /**
   * Set 'world' to 'this' translation.
   * csCamera overrides this in order to be able to update the
   * camera number.
   */
  virtual void SetO2TTranslation (const csVector3& v)
  {
    csOrthoTransform::SetO2TTranslation (v);
    cameranr = cur_cameranr++;
  }

  /**
   * Sets the absolute position of the camera inside the sector.
   * Vector 'v' is in world space coordinates. This function does
   * not check if the vector is really in the current sector. In
   * fact it is legal to set the position outside the sector
   * boundaries.
   */
  virtual void SetPosition (const csVector3& v) { SetOrigin (v); }

  /**
   * Set the world to camera transformation matrix.
   * This basically defines the direction that the camera looks.
   */
  inline void SetW2C (const csMatrix3& m) { SetO2T (m); }

  /**
   * Set the camera to world transformation matrix.
   * This basically defines the direction that the camera looks.
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
  inline csVector3 World2Camera (const csVector3& v) const
  { return Other2This (v); }

  /**
   * Transform a camera space point to world space.
   */
  inline csVector3 Camera2World (const csVector3& v) const
  { return This2Other (v); }

  /**
   * Transform a camera space point to worldspace, relative to camera position.
   */
  inline csVector3 Camera2WorldRelative (const csVector3& v) const
  { return This2OtherRelative (v); }

  /**
   * Moves the camera a relative amount in world coordinates.
   * If 'cd' is true then collision detection with objects and things
   * inside the sector is active. Otherwise you can walk through objects
   * (but portals will still be correctly checked).
   */
  virtual void MoveWorld (const csVector3& v, bool cd = true);

  /**
   * Moves the camera a relative amount in camera coordinates.
   */
  virtual void Move (const csVector3& v, bool cd = true)
  { MoveWorld (m_t2o * v, cd); }

  /**
   * Moves the camera a relative amount in world coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveWorldUnrestricted (const csVector3& v) { Translate (v); }

  /**
   * Moves the camera a relative amount in camera coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveUnrestricted (const csVector3& v) { Translate (m_t2o * v); }

  /**
   * Eliminate roundoff error by snapping the camera orientation to a
   * grid of density n
   */
  void Correct (int n);

  /// Change the shift for perspective correction.
  void SetPerspectiveCenter (float x, float y) { shift_x = x; shift_y = y; }

  /// Calculate perspective corrected point for this camera.
  virtual void Perspective (const csVector3& v, csVector2& p) const
  {
    float iz = aspect / v.z;
    p.x = v.x * iz + shift_x;
    p.y = v.y * iz + shift_y;
  }

  /// Calculate inverse perspective corrected point for this camera.
  virtual void InvPerspective (const csVector2& p, float z, csVector3& v) const
  {
    v.z = z;
    v.x = (p.x - shift_x) * z * inv_aspect;
    v.y = (p.y - shift_y) * z * inv_aspect;
  }

  virtual void AddCameraSectorListener (iCameraSectorListener* listener)
  {
    listeners.Push (listener);
  }

  virtual void RemoveCameraSectorListener (iCameraSectorListener* listener)
  {
    listeners.Delete (listener);
  }

  void FireCameraSectorListeners (iSector* sector);

  SCF_DECLARE_IBASE;

  virtual iCamera* Clone () const
  {
    return (iCamera*)new csCamera (*this);
  }
  virtual void OnlyPortals (bool hop)
  {
    only_portals = hop;
  }
  virtual bool GetOnlyPortals ()
  {
    return only_portals;
  }
  virtual csOrthoTransform& GetTransform ()
  { return *(csOrthoTransform*)this; }
  virtual const csOrthoTransform& GetTransform () const
  { return *(csOrthoTransform*)this; }
  virtual void SetTransform (const csOrthoTransform& tr)
  {
    *(csOrthoTransform*)this = tr;
    cameranr = cur_cameranr++;
  }
};

#endif // __CS_CAMERA_H__
