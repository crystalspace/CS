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

#ifndef __CS_RAIN_H__
#define __CS_RAIN_H__

#include "csplugincommon/particlesys/particle.h"
#include "imesh/rain.h"
#include "csutil/scf_implementation.h"

/**
 * A rain particle system. Particles start falling down.
 * Since speed if fixed due to air friction, this is not a NewtonianPartSys.
 */
class csRainMeshObject :
  public scfImplementationExt1<csRainMeshObject, csNewParticleSystem,
    iRainState>
{
protected:
  /// falling speed
  csVector3 Speed;

public:
  /// constructor
  csRainMeshObject (iEngine *engine, iMeshObjectFactory *fact);

  /// Destructor.
  virtual ~csRainMeshObject ();

  /// Update the particle system.
  virtual void Update (csTicks elapsed_time);

  /// spread rain particles throughout the rain box
  void Spread (int first, int limit);

  //------------------------- iRainState implementation ----------------
  void SetParticleCount (int num);
  void SetDropSize (float dropwidth, float dropheight);
  void SetBox (const csVector3& minbox, const csVector3& maxbox);
  void SetFallSpeed (const csVector3& fspeed);
  int GetParticleCount () const;
  void GetDropSize (float& dropwidth, float& dropheight) const;
  void GetBox (csVector3& minbox, csVector3& maxbox) const;
  const csVector3& GetFallSpeed () const;
  void SetCollisionDetection (bool cd);
  bool GetCollisionDetection () const;

  // Redirect these functions to csNewParticleSystem
  virtual void SetLighting (bool l)
  { csNewParticleSystem::SetLighting (l); }
  virtual bool GetLighting () const
  { return csNewParticleSystem::GetLighting(); }
};

#endif // __CS_RAIN_H__
