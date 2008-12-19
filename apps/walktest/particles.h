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

#ifndef __WALKTEST_PARTICLES_H__
#define __WALKTEST_PARTICLES_H__

struct iSector;
class csVector3;

class WalkTest;

/**
 * Demo particle systems.
 */
class WalkTestParticleDemos
{
public:
  static void Rain (WalkTest* Sys, const char* arg);
  static void FollowRain (WalkTest* Sys, const char* arg);
  static void Snow (WalkTest* Sys, const char* arg);
  static void Flame (WalkTest* Sys, const char* arg);
  static void Fountain (WalkTest* Sys, const char* arg);
  static void Explosion (WalkTest* Sys, const char* arg);

  static void Explosion (WalkTest* Sys, iSector* sector, const csVector3& center,
      const char* materialname);
};

#endif // __WALKTEST_PARTICLES_H__

