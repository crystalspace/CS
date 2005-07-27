/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_IMESH_RAIN_H__
#define __CS_IMESH_RAIN_H__

#include "csutil/scf.h"

struct iMaterialWrapper;

class csColor;
class csVector3;

SCF_VERSION (iRainState, 0, 0, 1);

/**
 * This interface describes the API for the rain mesh object.
 */
struct iRainState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetParticleCount (int num) = 0;
  /// Get the number of particles used.
  virtual int GetParticleCount () const = 0;
  /// Set the size of the drops.
  virtual void SetDropSize (float dropwidth, float dropheight) = 0;
  /// Get the size of the rain drops.
  virtual void GetDropSize (float& dropwidth, float& dropheight) const = 0;
  /// Set box.
  virtual void SetBox (const csVector3& minbox, const csVector3& maxbox) = 0;
  /// Get box.
  virtual void GetBox (csVector3& minbox, csVector3& maxbox) const = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// See if lighting is enabled.
  virtual bool GetLighting () const = 0;
  /// Set fall speed.
  virtual void SetFallSpeed (const csVector3& fspeed) = 0;
  /// Get fall speed.
  virtual const csVector3& GetFallSpeed () const = 0;
  /// Enable/Disable Collision Detection for drops.
  virtual void SetCollisionDetection (bool cd) = 0;
  /// Get CD flag.
  virtual bool GetCollisionDetection () const = 0;
  /// Set material of particle system.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of particle system.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;
  /// Set particle color.
  virtual void SetColor (const csColor& color) = 0;
  /// Get particle color.
  virtual const csColor& GetColor () const = 0;
};

#endif // __CS_IMESH_RAIN_H__

