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

/**
 * A rain particle system. Particles start falling down.
 * Since speed if fixed due to air friction, this is not a NewtonianPartSys.
 */
class csRainMeshObject : public csNewParticleSystem
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

  /// iRainState functions
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

  SCF_DECLARE_IBASE_EXT (csNewParticleSystem);

  //------------------------- iRainState implementation ----------------
  class RainState : public iRainState
  {
    SCF_DECLARE_EMBEDDED_IBASE (csRainMeshObject);
    virtual void SetParticleCount (int num)
    { scfParent->SetParticleCount (num); }
    virtual void SetDropSize (float dropwidth, float dropheight)
    { scfParent->SetDropSize (dropwidth, dropheight); }
    virtual void SetBox (const csVector3& minbox, const csVector3& maxbox)
    { scfParent->SetBox (minbox, maxbox); }
    virtual void SetLighting (bool l)
    { scfParent->SetLighting (l); }
    virtual void SetFallSpeed (const csVector3& fspeed)
    { scfParent->SetFallSpeed (fspeed); }
    virtual int GetParticleCount () const
    { return scfParent->ParticleCount; }
    virtual void GetDropSize (float& dropwidth, float& dropheight) const
    { scfParent->GetDropSize(dropwidth, dropheight); }
    virtual void GetBox (csVector3& minbox, csVector3& maxbox) const
    { scfParent->GetBox(minbox, maxbox); }
    virtual bool GetLighting () const
    { return scfParent->GetLighting(); }
    virtual const csVector3& GetFallSpeed () const
    { return scfParent->GetFallSpeed(); }
    virtual void SetCollisionDetection(bool cd)
    { scfParent->SetCollisionDetection(cd); };
    virtual bool GetCollisionDetection() const
    { return scfParent->GetCollisionDetection(); };
    virtual void SetMaterialWrapper (iMaterialWrapper* material)
    { scfParent->SetMaterialWrapper (material); }
    virtual iMaterialWrapper* GetMaterialWrapper () const
    { return scfParent->GetMaterialWrapper (); }
    virtual void SetMixMode (uint mode)
    { scfParent->MixMode = mode; }
    virtual uint GetMixMode () const
    { return scfParent->MixMode; }
    virtual void SetColor (const csColor& color)
    { scfParent->Color = color; }
    virtual const csColor& GetColor () const
    { return scfParent->Color; }
  } scfiRainState;
  friend class RainState;
};

#endif // __CS_RAIN_H__
