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

#ifndef __CS_IENGINE_CAMERA_H__
#define __CS_IENGINE_CAMERA_H__

/** \file
 * Camera object
 */

/**
 * \addtogroup engine3d_views
 * @{ */

#include "csgeom/matrix4.h"
#include "csutil/scf.h"
#include "csgeom/matrix4.h"

#define CS_VEC_FORWARD   csVector3(0,0,1)
#define CS_VEC_BACKWARD  csVector3(0,0,-1)
#define CS_VEC_RIGHT     csVector3(1,0,0)
#define CS_VEC_LEFT      csVector3(-1,0,0)
#define CS_VEC_UP        csVector3(0,1,0)
#define CS_VEC_DOWN      csVector3(0,-1,0)

#define CS_VEC_ROT_RIGHT  csVector3(0,1,0)
#define CS_VEC_ROT_LEFT   csVector3(0,-1,0)
#define CS_VEC_TILT_RIGHT (-csVector3(0,0,1))
#define CS_VEC_TILT_LEFT  (-csVector3(0,0,-1))
#define CS_VEC_TILT_UP    (-csVector3(1,0,0))
#define CS_VEC_TILT_DOWN  (-csVector3(-1,0,0))

class csOrthoTransform;
class csPlane3;
class csVector2;
class csVector3;

struct iSector;
struct iCamera;
struct iSceneNode;

/**
 * Implement this interface if you are interested in learning when
 * the camera changes sector or moves.
 *
 * This callback is used by:
 * - iCamera
 */
struct iCameraListener : public virtual iBase
{
  SCF_INTERFACE (iCameraListener, 0, 0, 1);

  /// Fired when the sector changes.
  virtual void NewSector (iCamera* camera, iSector* sector) = 0;

  /// Fired when the camera moves.
  virtual void CameraMoved (iCamera* camera) = 0;
};

#include "csutil/deprecated_warn_off.h"

/**
 * Implement this interface if you are interested in learning when
 * the camera changes sector.
 *
 * This callback is used by:
 * - iCamera
 */
struct CS_DEPRECATED_TYPE_MSG("Use iCameraListener instead")
iCameraSectorListener : public iCameraListener
{
  SCF_INTERFACE (iCameraSectorListener, 1, 0, 0);

  /// Fired when the sector changes.
  virtual void NewSector (iCamera* camera, iSector* sector) = 0;

  // Make it compile.
  void CameraMoved (iCamera* camera) {}
};

#include "csutil/deprecated_warn_on.h"

/**
 * Camera class. This class represents camera objects which can be used to
 * render a world in the engine. A camera has the following properties:
 * - Home sector: The sector in which rendering starts.
 * - Transformation: This is an orthonormal transformation which is applied
 *   to all rendered objects to move them from world space to camera space.
 *   It is the mathematical representation of position and direction of the
 *   camera. The position should be inside the home sector.
 * - Field of View: Controls the size on screen of the rendered objects and
 *   can be used for zooming effects. The FOV can be given either in pixels
 *   or as an angle in degrees.
 * - Shift amount: The projection center in screen coordinates.
 * - Mirrored Flag: Should be set to true if the transformation is mirrored.
 * - Far Plane: A distant plane that is orthogonal to the view direction. It
 *   is used to clip away all objects that are farther away than a certain
 *   distance, usually to improve rendering speed.
 * - Camera number: An identifier for a camera transformation, used
 *   internally in the engine to detect outdated vertex buffers.
 * - Only Portals Flag: If this is true then no collisions are detected for
 *   camera movement except for portals.
 *
 * Main creators of instances implementing this interface:
 * - iEngine::CreateCamera()
 * - csView
 * 
 * Main ways to get pointers to this interface:
 * - iView::GetCamera()
 * 
 * Main users of this interface:
 * - csView
 * - iView
 */
struct iCamera : public virtual iBase
{
  SCF_INTERFACE(iCamera, 3,0,0);
  /**
   * Create a clone of this camera. Note that the array of listeners
   * is not cloned.
   */
  virtual csPtr<iCamera> Clone () const = 0;

  /**
   * Get the scene node that this object represents.
   * \todo iCamera doesn't yet support iMovable so scene nodes are not
   * properly working yet.
   */
  virtual iSceneNode* QuerySceneNode () = 0;

  /**
   * Return the FOV (field of view) in pixels
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */  
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual int GetFOV () const = 0;
  /**
   * Return the inverse flield of view (1/FOV) in pixels
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetInvFOV () const = 0;
  /**
   * Return the FOV (field of view) in degrees.
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetFOVAngle () const = 0;

  /**
   * Set the FOV in pixels. 'fov' is the desired FOV in pixels. 'width' is
   * the display width, also in pixels.
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual void SetFOV (int fov, int width) = 0;
  /**
   * Set the FOV in degrees. 'fov' is the desired FOV in degrees. 'width' is
   * the display width in pixels.
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual void SetFOVAngle (float fov, int width) = 0;

  /**
   * Get the X shift amount. The parameter specified the desired X coordinate
   * on screen of the projection center of the camera.
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetShiftX () const = 0;
  /**
   * Get the Y shift amount. The parameter specified the desired Y coordinate
   * on screen of the projection center of the camera.
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetShiftY () const = 0;
  /**
   * Set the shift amount. The parameter specified the desired projection
   * center of the camera on screen.
   * \deprecated Deprecated in 1.3. Use iPerspectiveCamera instead
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
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

  /// 'const' version of GetTransform ()
  virtual const csOrthoTransform& GetTransform () const = 0;

  /**
   * Set the transform corresponding to this camera. In this transform,
   * 'other' is world space and 'this' is camera space.
   */
  virtual void SetTransform (const csOrthoTransform& tr) = 0;

  /**
   * Moves the camera a relative amount in world coordinates.
   * \warning The \a cd parameter is not used
   */
  virtual void MoveWorld (const csVector3& v, bool cd = true) = 0;
  /**
   * Moves the camera a relative amount in camera coordinates.
   * \warning The \a cd parameter is not used
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
  CS_DEPRECATED_METHOD_MSG("Don't use it anymore")
  virtual void Correct (int n) = 0;

  /// Return true if space is mirrored.
  virtual bool IsMirrored () const = 0;
  /// Set mirrored state.
  virtual void SetMirrored (bool m) = 0;

  /**
   * Get the 3D far plane that should be used to clip all geometry.
   * If this function returns 0 no far clipping is required.
   * Otherwise it must be used to clip the object before
   * drawing.
   */
  virtual csPlane3* GetFarPlane () const = 0;

  /**
   * Set the 3D far plane used to clip all geometry.
   * If the pointer is 0 then far plane clipping will be disabled.
   * Otherwise it will be enabled and the plane will be copied (so you
   * can free or reuse the pointer you give here). Note that the far-plane
   * will cull away geometry which is on the negative side of the plane
   * (with csPlane3::Classify() function).
   */
  virtual void SetFarPlane (csPlane3* fp) = 0;

  /**
   * Get the camera number. This number is changed for every new camera
   * instance and it is also updated whenever the camera transformation
   * changes. This number can be used to cache camera vertex arrays, for
   * example.
   */
  virtual long GetCameraNumber () const = 0;

  /// Calculate perspective corrected point for this camera.
  virtual csVector2 Perspective (const csVector3& v) const = 0;
  /// Calculate inverse perspective corrected point for this camera.
  virtual csVector3 InvPerspective (const csVector2& p, float z) const = 0;

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

#include "csutil/deprecated_warn_off.h"

  /// Add a listener to this camera.
  CS_DEPRECATED_METHOD_MSG("Use iCameraListener instead")
  virtual void AddCameraSectorListener (iCameraSectorListener* listener) = 0;
  /// Remove a listener from this camera.
  CS_DEPRECATED_METHOD_MSG("Use iCameraListener instead")
  virtual void RemoveCameraSectorListener (iCameraSectorListener* listener) = 0;

#include "csutil/deprecated_warn_on.h"

  /// Add a listener to this camera.
  virtual void AddCameraListener (iCameraListener* listener) = 0;
  /// Remove a listener from this camera.
  virtual void RemoveCameraListener (iCameraListener* listener) = 0;
  
  /// Get the projection matrix for this camera
  virtual const CS::Math::Matrix4& GetProjectionMatrix () = 0;
  
  /**
   * Return the planes limiting the visible volume (as specified by the
   * projection). The returned planes are in camera space.
   */
  virtual const csPlane3* GetVisibleVolume (uint32& mask) = 0;
  
  /// Set the size of the viewport this camera is associated with.
  virtual void SetViewportSize (int width, int height) = 0;
  
  /// Get the inverse projection matrix for this camera
  virtual const CS::Math::Matrix4& GetInvProjectionMatrix () = 0;
};

/**
 * An implementation of iCamera that renders a world with a classical 
 * perspective.
 * 
 * Main creators of instances implementing this interface:
 * - iEngine::CreatePerspectiveCamera()
 * 
 * Main ways to get pointers to this interface:
 * - iView::GetPerspectiveCamera()
 */
struct iPerspectiveCamera : public virtual iBase
{
  SCF_INTERFACE(iPerspectiveCamera, 1, 0, 1);
  
  /// Get the iCamera interface for this camera.
  virtual iCamera* GetCamera() = 0;
  
  /// Return the normalized FOV (field of view)
  virtual float GetFOV () const = 0;
  /// Return the inverse of normalized field of view (1/FOV)
  virtual float GetInvFOV () const = 0;
  /// Return the FOV (field of view) in degrees.
  virtual float GetFOVAngle () const = 0;
  /**
   * Set the FOV. \a fov is the desired FOV in normalized screen coordinates.
   * \a width is the display width, also normalized.
   */
  virtual void SetFOV (float fov, float width) = 0;
  
  /**
   * Set the FOV in degrees. \a fov is the desired FOV in degrees. \a width is
   * the display width in normalized screen coordinates.
   */
  virtual void SetFOVAngle (float fov, float width) = 0;

  /**
   * Get the X shift amount. The parameter specified the desired X coordinate
   * on the normalized screen of the projection center of the camera.
   */
  virtual float GetShiftX () const = 0;
  /**
   * Get the Y shift amount. The parameter specified the desired Y coordinate
   * on the normalized screen of the projection center of the camera.
   */
  virtual float GetShiftY () const = 0;
  /**
   * Set the shift amount. The parameter specified the desired projection
   * center of the camera on the normalized screen.
   */
  virtual void SetPerspectiveCenter (float x, float y) = 0;
  
  /**
   * Get the near clip distance of this camera.
   * Higher clip distances increases the possibility that the camera may look
   * "inside" some geometry, however, it also provides higher depth value
   * precision - especially for large scenes this can prevent depth buffer
   * issues (Z-fighting, wrong occlusion) at a distance.
   * 
   * If you have a collider (player etc.) coupled to camera movement a good
   * initial value for the near clip distance is a value slightly above the
   * collider radius.
   */
  virtual float GetNearClipDistance() const = 0;
  /**
   * Set near clip distance of this camera.
   * 
   * The default near clipping distance is controlled by the engine.
   */
  virtual void SetNearClipDistance (float dist) = 0;
};

/**
 * An implementation of iCamera that renders a world with a custom
 * projection matrix.
 * 
 * Main creators of instances implementing this interface:
 * - iEngine::CreateCustomMatrixCamera()
 * 
 * Main ways to get pointers to this interface:
 * - iView::GetCustomMatrixCamera()
 */
struct iCustomMatrixCamera : public virtual iBase
{
  SCF_INTERFACE(iCustomMatrixCamera, 1, 0, 0);
  
  /// Get the iCamera interface for this camera.
  virtual iCamera* GetCamera() = 0;
  
  /// Set the projection matrix.
  virtual void SetProjectionMatrix (const CS::Math::Matrix4& mat) = 0;
};

/** @} */

#endif // __CS_IENGINE_CAMERA_H__

