/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
  
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
#include "icamera.h"
#include "csengine/planeclp.h"

class csSector;
class csPolygon3D;
class Vertex;
class csEngine;
class Dumper;

/**
 * A camera positioned in the 3D world.
 */
class csCamera : public csOrthoTransform, public iBase
{
  friend class Dumper;

private:
  /// The sector the camera is in.
  csSector* sector;
  /// If true we are in a mirrored world.
  bool mirror;

  /// a farplane to cut everything thats behind it
  csPlaneClip *fp;
  bool use_farplane;

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

public:
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
  int GetFOV () const { return aspect; }
  /// Get the inverse FOV for this camera.
  float GetInvFOV () const { return inv_aspect; }

  /// Set the FOV in angles (degrees).
  void SetFOVAngle (float a, int width);
  /// Get the FOV in angles (degrees).
  float GetFOVAngle ()
  {
    return fov_angle;
  }

  /// Get the X shift value.
  float GetShiftX () const { return shift_x; }
  /// Get the Y shift value.
  float GetShiftY () const { return shift_y; }

  /// Set farplane, everything behind this will be cut
  void SetFarPlane (csPlaneClip* farplane) { fp = farplane; }
  /// Get the Farplane
  csPlaneClip* GetFarPlane () { return fp; }
  
  /// do we actually use the farplane ?
  bool UseFarPlane () { return use_farplane; }
  /// Set whether we use farplane or not. Farplane must been set before
  void UseFarPlane (bool useit) { use_farplane = fp && useit; }
  
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
  virtual void SetPosition (const csVector3& v) { SetOrigin (v); }

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
   * If 'cd' is true then collision detection with objects and things
   * inside the sector is active. Otherwise you can walk through objects
   * (but portals will still be correctly checked).
   */
  virtual void MoveWorld (const csVector3& v, bool cd = true);

  /**
   * Moves the camera a relative amount in camera coordinates.
   */
  virtual void Move (const csVector3& v, bool cd = true) { MoveWorld (m_t2o * v, cd); }

  /**
   * Moves the camera a relative amount in world coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveWorldUnrestricted (const csVector3& v) { v_o2t += v; }

  /**
   * Moves the camera a relative amount in camera coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveUnrestricted (const csVector3& v) { v_o2t += m_t2o * v; }

  /**
   * Rotate the camera by the angle (radians) around the given vector,
   * in world coordinates.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void RotateWorld (const csVector3& v, float angle);

  /**
   * Rotate the camera by the angle (radians) around the given vector,
   * in camera coordinates.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void Rotate (const csVector3& v, float angle);

  /**
   * Use the given transformation matrix, in worldspace,
   * to reorient the camera.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void RotateWorld (const csMatrix3& m) { SetT2O (m * m_t2o); }

  /**
   * Use the given transformation matrix, in camera space,
   * to reorient the camera.
   * Note: this function rotates the camera, not the coordinate system.
   */
  virtual void Rotate (const csMatrix3& m) { SetT2O (m_t2o * m); }

  /**
   * Have the camera look at the given (x,y,z) point, using up as
   * the up-vector. 'v' should be given relative to the position
   * of the camera.
   */
  virtual void LookAt (const csVector3& v, const csVector3& up);

  /**
   * Eliminate roundoff error by snapping the camera orientation to a
   * grid of density n
   */
  virtual void Correct (int n);

  /// Change the shift for perspective correction.
  void SetPerspectiveCenter (float x, float y) { shift_x = x; shift_y = y; }

  /// Calculate perspective corrected point for this camera.
  void Perspective (const csVector3& v, csVector2& p) const
  {
    float iz = aspect / v.z;
    p.x = v.x * iz + shift_x;
    p.y = v.y * iz + shift_y;
  }

  /// Calculate inverse perspective corrected point for this camera.
  void InvPerspective (const csVector2& p, float z, csVector3& v) const
  {
    v.z = z;
    v.x = (p.x - shift_x) * z * inv_aspect;
    v.y = (p.y - shift_y) * z * inv_aspect;
  }

  DECLARE_IBASE;

  //------------------------ iCamera implementation ------------------------
  struct Camera : public iCamera
  {
    DECLARE_EMBEDDED_IBASE (csCamera);
    virtual csCamera* GetPrivateObject ()
    {
      return scfParent;
    }
    virtual float GetFOV () { return scfParent->GetFOV (); }
    virtual float GetInvFOV () { return scfParent->GetInvFOV (); }
    virtual float GetFOVAngle () { return scfParent->GetFOVAngle (); }
    virtual float GetShiftX () { return scfParent->GetShiftX (); }
    virtual float GetShiftY () { return scfParent->GetShiftY (); }
    virtual csOrthoTransform& GetTransform () { return *(csOrthoTransform*)scfParent; }
    virtual void SetPosition (const csVector3& v)
    {
      scfParent->SetPosition (v);
    }
    virtual void MoveWorld (const csVector3& v, bool cd = true)
    {
      scfParent->MoveWorld (v, cd);
    }
    virtual void Move (const csVector3& v, bool cd = true)
    {
      scfParent->Move (v, cd);
    }
    virtual void MoveWorldUnrestricted (const csVector3& v)
    {
      scfParent->MoveWorldUnrestricted (v);
    }
    virtual void MoveUnrestricted (const csVector3& v)
    {
      scfParent->MoveUnrestricted (v);
    }
    virtual void RotateWorld (const csVector3& v, float angle)
    {
      scfParent->RotateWorld (v, angle);
    }
    virtual void Rotate (const csVector3& v, float angle)
    {
      scfParent->Rotate (v, angle);
    }
    virtual void RotateWorld (const csMatrix3& m)
    {
      scfParent->RotateWorld (m);
    }
    virtual void Rotate (const csMatrix3& m)
    {
      scfParent->Rotate (m);
    }
    virtual void LookAt (const csVector3& v, const csVector3& up)
    {
      scfParent->LookAt (v, up);
    }
    virtual void Correct (int n)
    {
      scfParent->Correct (n);
    }
    virtual bool IsMirrored ()
    {
      return scfParent->IsMirrored ();
    }
    virtual void SetMirrored (bool m)
    {
      scfParent->SetMirrored (m);
    }
    virtual bool GetFarPlane (csPlane3& pl)
    {
      if (scfParent->fp) { pl = *scfParent->fp; return true; }
      else return false;
    }
  } scfiCamera;

private:
  ///
  void Correct (int n, float* vals[]);
};

#endif // __CS_CAMERA_H__
