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

#ifndef __WALKTEST_ANIMSKY_H__
#define __WALKTEST_ANIMSKY_H__

class WalkTest;
struct iMeshWrapper;
struct iObject;

/**
 * Animating sky.
 */
class WalkTestAnimateSky
{
private:
  WalkTest* walktest;

  /// A pointer to a skybox to animate (if any).
  iMeshWrapper* anim_sky;
  /// Speed of this animation (with 1 meaning 1 full rotation in a second).
  float anim_sky_speed;
  /// Rotation direction (0=x, 1=y, 2=z)
  int anim_sky_rot;
  /// A pointer to the terrain for which we animate the dirlight.
  iMeshWrapper* anim_dirlight;

public:
  WalkTestAnimateSky (WalkTest* walktest);

  void MoveSky (csTicks elapsed_time, csTicks current_time);
  void AnimateSky (const char* value, iObject* src);
  void AnimateDirLight (iObject* src);
};

#endif // __WALKTEST_ANIMSKY_H__

