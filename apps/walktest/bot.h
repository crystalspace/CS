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

#ifndef BOT_H
#define BOT_H

#include "csgeom/math3d.h"
#include "csengine/objects/cssprite.h"

class csSector;

/**
 * A bot which moves randomly through the dungeon.
 */
class Bot : public csSprite3D
{
private:
  /// Current movement vector (unit vector).
  csVector3 d;
  /// Position that is followed.
  csVector3 follow;
  /// Corresponding sector.
  csSector* f_sector;

public:
  ///
  Bot* next;

public:
  ///
  Bot (csSpriteTemplate* tmpl);

  ///
  virtual ~Bot ();

  ///
  void move (long elapsed_time);

  ///
  void set_bot_move (const csVector3& v);
  ///
  void set_bot_sector (csSector* s) { f_sector = s; }

  CSOBJTYPE;
};

#endif

