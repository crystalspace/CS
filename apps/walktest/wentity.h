/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#ifndef __WENTITY_H
#define __WENTITY_H

#include "csengine/movable.h"
#include "csobject/pobject.h"
#include "csobject/dataobj.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/csvector.h"

struct iLight;
struct iMeshWrapper;


SCF_VERSION (csWalkEntity, 0, 0, 1);

/**
 * A general WalkTest entity.
 */
class csWalkEntity : public csPObject
{
public:
  /// Activate this entity.
  virtual void Activate () = 0;

  /// Handle next frame.
  virtual void NextFrame (float elapsed_time) = 0;

  DECLARE_IBASE_EXT (csPObject);

  virtual ~csWalkEntity ()
  {}
};

/**
 * A Door.
 */
class csDoor : public csWalkEntity
{
private:
  /// Open or closed.
  bool is_open;
  /// Hinge location.
  csVector3 hinge;
  /**
   * If 0 then door is in is_open state. Otherwise it is between
   * the old state (1) and is_open state (representing 0 again).
   */
  float transition;
  /// Parent mesh.
  iMeshWrapper* tparent;

public:
  /// Create this door.
  csDoor (iMeshWrapper* p);

  /// Set hinge location.
  void SetHinge (const csVector3& h) { hinge = h; }

  /// Activate this entity (open/close door).
  virtual void Activate ();

  /// Handle next frame.
  virtual void NextFrame (float elapsed_time);
};

/**
 * A Rotating object.
 */
class csRotatingObject : public csWalkEntity
{
private:
  /// If 'always' is true it rotates continuously.
  bool always;
  /// The rotation angles.
  csVector3 angles;
  /// Parent.
  iObject* tparent;
  /// Movable of the parent.
  iMovable* movable;
  /// Time remaining before we stop rotating (if always == false).
  float remaining;

public:
  /// Create this object.
  csRotatingObject (iObject* p);

  /// Set rotation angles.
  void SetAngles (const csVector3& a) { angles = a; }

  /// Set the 'always' flag.
  void SetAlways (bool a) { always = a; }

  /// Activate this entity (start rotation).
  virtual void Activate ();

  /// Handle next frame.
  virtual void NextFrame (float elapsed_time);
};

/**
 * An object controlling a pseudo-dynamic light.
 */
class csLightObject : public csWalkEntity
{
private:
  /// The light that is controlled by this entity.
  iLight* light;
  /// The color to set this light to on activation.
  csColor start_color;
  /// The color we will end up with when activation ends.
  csColor end_color;
  /// On activation, the time to take to go from start to end.
  float act_time;
  /// Current elapsed time.
  float cur_time;

public:
  /// Create this object.
  csLightObject (iLight* p);

  /// Set colors.
  void SetColors (const csColor& start_color, const csColor& end_color)
  {
    csLightObject::start_color = start_color;
    csLightObject::end_color = end_color;
  }

  /// Set total time to take for animation.
  void SetTotalTime (float t) { act_time = t; }

  /// Activate this entity.
  virtual void Activate ();

  /// Handle next frame.
  virtual void NextFrame (float elapsed_time);
};

#endif // __WENTITY_H

