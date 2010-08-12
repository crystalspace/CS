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

#ifndef __WALKTEST_LIGHTS_H__
#define __WALKTEST_LIGHTS_H__

class WalkTest;
#include "iengine/mesh.h"
#include "isndsys.h"

#define DYN_TYPE_MISSILE 1
#define DYN_TYPE_RANDOM 2
#define DYN_TYPE_EXPLOSION 3

struct LightStruct
{
  int type;
};

struct RandomLight
{
  int type;		// type == DYN_TYPE_RANDOM
  float dyn_move_dir;
  float dyn_move;
  float dyn_r1, dyn_g1, dyn_b1;
};

/**
 * This class manages the dynamic lights that go with a missile.
 */
class WalkTestLights
{
private:
  WalkTest* walktest;
  csRefArray<iLight> dynamic_lights;

  csRef<iLight> flashlight;
  csReversibleTransform FlashlightShift (const csReversibleTransform& tf);
public:
  WalkTestLights (WalkTest* walktest);
  
  void EnableFlashlight (bool enable);
  bool IsFlashlightEnabled () const { return flashlight != 0; }

  bool HandleDynLight (iLight* dyn);
  void HandleDynLights ();
  csRef<iLight> CreateRandomLight (const csVector3& pos, iSector* where, float dyn_radius);
  void AddLight (const char* arg);
  void DelLight ();
  void DelLights ();
};

#endif // __WALKTEST_LIGHTS_H__

