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

#ifndef __BOT_H__
#define __BOT_H__

#include "csgeom/math3d.h"
#include "iengine/mesh.h"

struct iMeshObject;
struct iSector;
struct iLight;
struct iEngine;
class csEngine;

/**
 * A bot which moves randomly through the dungeon.
 */
class Bot 
{
private:
  /// Engine handle.
  iEngine *engine;
  /// Current movement vector (unit vector).
  csVector3 d;
  /// Position that is followed.
  csVector3 follow;
  /// Corresponding sector.
  iSector* f_sector;
  csRef<iMeshWrapper> mesh;

public:
  /// Optional dynamic light.
  csRef<iLight> light;

public:
  /// Constructor.
  Bot (iEngine *Engine, iMeshWrapper* botmesh);
  /// Destructor.
  virtual ~Bot ();

  /// Time-base move.
  void move (csTicks elapsed_time);

  /// Set movement vector.
  void set_bot_move (const csVector3& v);
  /// Set bot's sector.
  void set_bot_sector (iSector* s) { f_sector = s; }
};

#endif // __BOT_H__
