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

#ifndef __CS_PARTRAIN_H__
#define __CS_PARTRAIN_H__

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
 * A rain particle system. Particles start falling down.
 * Since speed if fixed due to air friction, this is not a NewtonianPartSys.
 */
class csRainParticleSystem : public csParticleSystem
{
protected:
  csBox3 rainbox;
  csVector3 rain_dir;
  csVector3 *part_pos;

public:
  /**
   * creates a rain particle system given parameters.
   * number : number of raindrops visible at one time
   * mat: material of raindrops. mixmode = mixmode used.
   * lighted: the particles will be lighted if true.
   * drop_width, drop_height: size of rectangular raindrops.
   * rainbox_min and max: give the box in the world where it will rain.
   *   raindrops will start ...
   *   and when they exit this box they will disappear.
   * fall_speed: the direction and speed of the falling raindrops.
   *   You can make slanted rain this way. Although you would also want to
   *   slant the particles in that case...
   */
  csRainParticleSystem(csObject* theParent, int number,
    csMaterialWrapper* mat, UInt mixmode,
    bool lighted_particles, float drop_width, float drop_height, 
    const csVector3& rainbox_min, const csVector3& rainbox_max,
    const csVector3& fall_speed
    );
  /// Destructor.
  virtual ~csRainParticleSystem();

  /// Update and light is flickered as well. particles will be scaled.
  virtual void Update (cs_time elapsed_time);

  CSOBJTYPE;
};

#endif // __CS_PARTRAIN_H__
