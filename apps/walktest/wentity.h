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

#ifndef __WENTITY_H
#define __WENTITY_H

#include "csobject/dataobj.h"
#include "csgeom/vector3.h"

class csThing;

/**
 * This is a global object that holds all 'busy' entities.
 * An entity can push itself onto this list when it wants something
 * to happen for a time. Then it can remove itself.
 */
class csEntityList : public csObject
{
  CSOBJTYPE;
};


/**
 * A general WalkTest entity.
 */
class csWalkEntity : public csObject
{
public:
  /// Activate this entity.
  virtual void Activate () = 0;

  /// Handle next frame.
  virtual void NextFrame (float elapsed_time) = 0;

  CSOBJTYPE;
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
  /// Parent thing.
  csThing* parent;

public:
  /// Create this door.
  csDoor (csThing* p);

  /// Set hinge location.
  void SetHinge (const csVector3& h) { hinge = h; }

  /// Activate this entity (open/close door).
  virtual void Activate ();

  /// Handle next frame.
  virtual void NextFrame (float elapsed_time);

  CSOBJTYPE;
};

#endif // __WENTITY_H

