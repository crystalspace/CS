/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Alex Pfaffe.

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
#ifndef BEING_H
#define BEING_H

#include "csengine/polyset.h"
#include "csengine/cssprite.h"
#include "csengine/thing.h"
#include "csengine/polygon.h"
#include "csengine/world.h"
#include "csengine/camera.h"
#include "csengine/sector.h"
#include "csengine/collider.h"

/**
 * csCollider is a generic class.  All objects which participate in
 * collisions are csColliders (walls, stairs, bullets, monsters etc.)
 * but moving objects need to keep some state information.  To
 * serve this purpose there is a csBeing class (maybe a better name can
 * be thought of). csBeings move around and can hit things.  They maybe
 * able to climb a stair case or climb a wall.
 * They fall under the effect or gravity and may slip or bounce.
 * Under certain conditions they may be able to fly or move through
 * a wall etc.  The logic for this should be handled by the function
 * csBeing::CollisionDetect ().  The Bot for instance should subclass
 * from csBeing, or it maybe able to use csBeing as is.   To create your
 * own Monsters you should subclass csBeing and look at the
 * csBeing::CollisionDetect () code which you may need to overrule with
 * you own method.
 */
class csBeing : public csCollider
{
  // Record the location we are standing at
  csCollider *_ground;
  // Previous sector.
  csSector *_sold;
  // Transformation matrix from world space to local space.
  csMatrix3 _mold;
  // Location of the origin for the local transformation.
  csVector3 _vold;

  static csWorld    *_world;  // This should not be needed in long term.

 public:

  static csBeing   *player;       // The being followed by the camera.
  static bool     init;         // Was the world initialized.
  /// The sector in which the being currently resides.
  csSector *sector;
  /** The transformation which decsribes the objects current position
   * and orienatation.
   */
  csTransform *transform;
  /// Records whether being was falling, climbing or blocked in the last frame.
  bool falling;
  bool climbing;
  bool blocked;

  csBeing(csPolygonSet *p, csSector* s = 0, csTransform *cdt = 0);
  csBeing(csSprite3D *sp, csSector* s = 0, csTransform *cdt = 0);

  /// Create a player bounding box.
  static csBeing* PlayerSpawn (char* name);

  /// Initialize CD data for all sectors in the world.
  static int InitWorld (csWorld* world, csCamera *camera);

  /// Free any memory held by CD system.
  static void EndWorld ();

  /// Collide cd with all objects in the sector.
  virtual int CollisionDetect ();

 private:
  int _CollisionDetect (csSector* s, csTransform *cdt);

  /// Find all sector which are near enough to intersect with this being.
  int FindSectors (csVector3 v, csVector3 d, csSector *s, csSector **sa);

};


#endif
