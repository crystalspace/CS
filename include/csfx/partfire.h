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

#ifndef __CS_PARTFIRE_H__
#define __CS_PARTFIRE_H__

#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "csengine/cssprite.h"
#include "csengine/particle.h"
#include "iparticl.h"

class csSprite2D;
class csMaterialWrapper;
class csEngine;
class csSector;
class csDynLight;
class csLight;
class csRenderView;

/**
 * A Fire particle system. Each x msec n particles shoot out of the fire, 
 */
class csFireParticleSystem : public csParticleSystem
{
protected:
  int amt;
  csVector3 direction;
  csVector3 origin;
  float swirl;
  float color_scale;
  csVector3* part_pos;
  csVector3* part_speed;
  float *part_age;
  float total_time;
  float time_left; // from previous update
  int next_oldest;

  csLight *light;
  int light_time;
  bool delete_light;
  csEngine *light_engine;

  int FindOldest();
  void RestartParticle(int index, float pre_move);
  void MoveAndAge(int index, float delta_t);

public:
  /**
   * creates a fire particle system given parameters.
   * number : number of raindrops visible at one time
   * mat: material of raindrops. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular particles.
   * total_time is the seconds a particle gets to burn.
   * dir is direction of fire.
   * origin is the starting point of the flame
   * swirl is the amount of swirling of particles.
   * color_scale scales the colour the particles are set to.
   */
  csFireParticleSystem(csObject* theParent, int number, 
    csMaterialWrapper* mat, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    float total_time, const csVector3& dir, const csVector3& origin,
    float swirl, float color_scale
    );
  /// Destructor.
  virtual ~csFireParticleSystem();

  /// Update the particle system.
  virtual void Update (cs_time elapsed_time);

  /// You can set a pseudo-static light here
  void SetControlledLight(csLight *l) {light = l;}
  /**
   * Add a new dynamic light (no need to call SetControlledLight). 
   * NB Will not move upon SetSector.
   */
  void AddLight(csEngine*, csSector*);

  CSOBJTYPE;
};

#endif // __CS_PARTFIRE_H__
