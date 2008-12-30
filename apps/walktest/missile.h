/*
    Copyright (C) 2008 by Jorrit Tyberghein

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

#ifndef __WALKTEST_MISSILE_H__
#define __WALKTEST_MISSILE_H__

class WalkTest;
#include "iengine/mesh.h"
#include "isndsys.h"

struct MissileStruct
{
  int type;		// type == DYN_TYPE_MISSILE
  csOrthoTransform dir;
  csRef<iMeshWrapper> sprite;
  csRef<iSndSysSource> snd;
  csRef<iSndSysStream> snd_stream;
};

struct ExplosionStruct
{
  int type;		// type == DYN_TYPE_EXPLOSION
  float radius;
  int dir;
};

/**
 * Missile launcher.
 */
class WalkTestMissileLauncher
{
private:
  WalkTest* walktest;

public:
  WalkTestMissileLauncher (WalkTest* walktest);

  void FireMissile ();
};

#endif // __WALKTEST_MISSILE_H__

