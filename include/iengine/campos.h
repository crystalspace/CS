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

#ifndef __CS_IENGINE_CAMPOS_H__
#define __CS_IENGINE_CAMPOS_H__

/**\file
 */
/**
 * \addtogroup engine3d_views
 * @{ */
 
#include "csutil/scf.h"

class csVector3;
class csPlane3;
struct iObject;
struct iEngine;
struct iCamera;

SCF_VERSION (iCameraPosition, 0, 1, 0);

/**
 * A camera position. This object can be used to initialize a camera object to
 * a certain state. It has the following properties:
 * <ul>
 * <li> Home sector name: This name is used to find the home sector of the
 *      camera in the engine when the camera position is applied.
 * <li> Position: Position of the camera
 * <li> Upward and forward vectors: These vectors define the orientation of the
 *      camera. More exactly, they are used to compute the transformation of
 *      the camera.
 * <li> A far plane used to clip geometry which is too far away.
 * </ul>
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iCameraPositionList::NewCameraPosition()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iCameraPositionList::Get()
 *   <li>iCameraPositionList::FindByName()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Engine stores them.
 *   <li>Application uses them.
 *   </ul>
 */
struct iCameraPosition : public iBase
{
  /// Get the iObject for this camera position.
  virtual iObject *QueryObject() = 0;

  /// Create a clone this camera position.
  virtual iCameraPosition* Clone () const = 0;

  /// Return the home sector.
  virtual const char *GetSector () = 0;
  /// Set the home sector.
  virtual void SetSector (const char *Name) = 0;

  /// Return the position.
  virtual const csVector3 &GetPosition () = 0;
  /// Set the position.
  virtual void SetPosition (const csVector3 &p) = 0;

  /// Return the 'up' vector.
  virtual const csVector3 &GetUpwardVector () = 0;
  /// Set the 'up' vector.
  virtual void SetUpwardVector (const csVector3 &v) = 0;

  /// Return the 'front' vector.
  virtual const csVector3 &GetForwardVector () = 0;
  /// Set the 'front' vector.
  virtual void SetForwardVector (const csVector3 &v) = 0;

  /// Set all attributes of the camera position.
  virtual void Set (const char *sector, const csVector3 &pos,
      const csVector3 &forward, const csVector3 &upward) = 0;

  /// Load the camera position into a camera object.
  virtual bool Load (iCamera*, iEngine*) = 0;

  /**
   * Set the 3D far plane used to clip all geometry.
   * If the pointer is 0 then far plane clipping will be disabled.
   * Otherwise it will be enabled and the plane will be copied (so you
   * can free or reuse the pointer you give here). Note that the far-plane
   * will cull away geometry which is on the negative side of the plane
   * (with csPlane3::Classify() function).
   */
  virtual void SetFarPlane (csPlane3* pl) = 0;

  /**
   * Clear the far plane so no clipping will occur. This is equivalent
   * to SetFarPlane(0).
   */
  virtual void ClearFarPlane () = 0;

  /**
   * Get the current far plane (or 0 if none is defined).
   */
  virtual csPlane3* GetFarPlane () const = 0;
};

/**
 * A list of camera position objects.
 * <p>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>iEngine::GetCameraPositions()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Engine stores them.
 *   <li>Application uses them.
 *   </ul>
 */
struct iCameraPositionList : public virtual iBase
{
  SCF_INTERFACE(iCameraPositionList,2,0,0);
  /// Create a new empty camera position.
  virtual iCameraPosition* NewCameraPosition (const char* name) = 0;

  /// Return the number of camera positions in this list
  virtual int GetCount () const = 0;

  /// Return a camera position by index
  virtual iCameraPosition *Get (int n) const = 0;

  /// Add a camera position
  virtual int Add (iCameraPosition *obj) = 0;

  /// Remove a camera position
  virtual bool Remove (iCameraPosition *obj) = 0;

  /// Remove the nth camera position
  virtual bool Remove (int n) = 0;

  /// Remove all camera positions
  virtual void RemoveAll () = 0;

  /// Find a camera position and return its index
  virtual int Find (iCameraPosition *obj) const = 0;

  /// Find a camera position by name
  virtual iCameraPosition *FindByName (const char *Name) const = 0;
};

/** @} */

#endif // __CS_IENGINE_CAMPOS_H__

