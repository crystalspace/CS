/*
    Crystal Space 3D engine
    Copyright (C) 2001 by Jorrit Tyberghein
  
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

#ifndef __IVISCULL_H__
#define __IVISCULL_H__

#include "csutil/scf.h"

struct iRenderView;
struct iVisibilityObject;
struct iMovable;
class csBox3;

SCF_VERSION (iVisibilityCuller, 0, 0, 1);

/**
 * This interface represents a visibility culling system.
 * To use it you first register visibility objects (which are all the
 * objects for which you want to test visibility) to this culler.
 */
struct iVisibilityCuller : public iBase
{
  /// Register a visibility object with this culler.
  virtual void RegisterVisObject (iVisibilityObject* visobj) = 0;
  /// Unregister a visibility object with this culler.
  virtual void UnregisterVisObject (iVisibilityObject* visobj) = 0;
  /**
   * Do the visibility test from a given viewpoint. This will first
   * clear the visible flag on all registered objects and then it will
   * mark all visible objects. If this function returns false then
   * the visibility test could not happen for some reason (disabled
   * or circumstances are not right). In this case all objects should
   * be considered visible.
   */
  virtual bool VisTest (iRenderView* irview) = 0;
};

SCF_VERSION (iVisibilityObject, 0, 0, 1);

/**
 * An object that wants to know if it is visible or not
 * for some visibility culler needs to implement this interface.
 */
struct iVisibilityObject : public iBase
{
  /// Get the reference to the movable from this object.
  virtual iMovable* GetMovable () = 0;
  /// Get the bounding box of the object in object space.
  virtual void GetBoundingBox (csBox3& bbox) = 0;
  /**
   * Mark the object as visible. This will be called by the visibility
   * culler whenever it thinks the object is visible.
   */
  virtual void MarkVisible () = 0;
  /**
   * Mark the object as invisible. This will be called by the visibility
   * culler at initialization time.
   */
  virtual void MarkInvisible () = 0;
  /**
   * After running iVisibilityCuller::VisTest() this function can be used
   * to test if the object is visible or not.
   */
  virtual bool IsVisible () = 0;
};

#endif // __IVISCULL_H__

