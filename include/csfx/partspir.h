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

#ifndef __CS_PARTSPIRAL_H__
#define __CS_PARTSPIRAL_H__

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
 * This class has a set of particles that act like a spiraling
 * particle fountain.
 */
class csSpiralParticleSystem : public csNewtonianParticleSystem
{
protected:
  int max;
  int time_before_new_particle; // needs to be signed.
  csVector3 source;
  int last_reuse;
  csMaterialWrapper* mat;
  csSector* this_sector;

  /// Move all particles to a sector, virtual so subclass can move more.
  virtual void MoveToSector (csSector *sector);

public:
  /// Specify max number of particles.
  csSpiralParticleSystem (csObject* theParent, int max,
    const csVector3& source, csMaterialWrapper* mat);
  /// Destructor.
  virtual ~csSpiralParticleSystem ();

  /// Moves the particles depending on their acceleration and speed.
  virtual void Update (cs_time elapsed_time);

  CSOBJTYPE;
};

#endif // __CS_PARTSPIRAL_H__
