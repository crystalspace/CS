/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef __CS_PARTFOUNTAIN_H__
#define __CS_PARTFOUNTAIN_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "csengine/cssprite.h"
#include "csengine/particle.h"
#include "iparticl.h"

class csMaterialWrapper;
class csEngine;
class csSector;
class csDynLight;
class csLight;
class csRenderView;

/**
 * A Fountain particle system. Each x msec n particles shoot out of a spout, 
 * falling down after that. Thus some particles may not reach the floor if
 * x is too small. If n is too small you will see not many particles.
 * Note that the 'spout' means the spot where the fountain originates.
 * Also you know that after fall_time, a particle has reached
 * sin(elevation)*speed*fall_time + accel.y*fall_time*fall_time + spot.y 
 * i.e. the world y height of the pool of the fountain.
 */
class csFountainParticleSystem : public csParticleSystem
{
protected:
  int amt;
  csVector3 origin;
  csVector3 accel;
  csVector3* part_pos;
  csVector3* part_speed;
  float *part_age;
  float speed, opening, azimuth, elevation, fall_time;
  float time_left; // from previous update
  int next_oldest;

  int FindOldest();
  void RestartParticle(int index, float pre_move);

public:
  /**
   * creates a fountain particle system given parameters.
   * number : number of raindrops visible at one time
   * mat: material of raindrops. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular waterdrops.
   * spot is the origin of the fountain
   * accel is the particle acceleration, in m/s^2, the gravity.
   * fall_time is the seconds a particle gets to fall.
   * speed in m/s of the drops on exiting the spout.
   * opening is the angle controlling the width of the stream.
   * azimuth is the angle of the direction (horizontally) of the stream.
   * elevation is the angle of the direction (up/down) of the stream.
   *   angles in radians (2*PI is 360 degrees)
   */
  csFountainParticleSystem(csObject* theParent, int number, 
    csMaterialWrapper* mat, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    const csVector3& spot, const csVector3& accel, float fall_time,
    float speed, float opening, float azimuth, float elevation
    );
  /// Destructor.
  virtual ~csFountainParticleSystem();

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  CSOBJTYPE;
};

#endif // __CS_PARTFOUNTAIN_H__
