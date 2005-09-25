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

#ifndef __CS_IMESH_FIRE_H__
#define __CS_IMESH_FIRE_H__

/**\file
 * Fire particle mesh object
 */ 

#include "csutil/scf.h"

/**\addtogroup meshplugins
 * @{ */

class csBox3;
class csColor;
class csVector3;

SCF_VERSION (iFireState, 0, 0, 1);

/**
 * This interface describes the API for the fire mesh object.
 */
struct iFireState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetParticleCount (int num) = 0;
  /// Get the number of particles.
  virtual int GetParticleCount () const = 0;
  /// Set the size of the fire drops.
  virtual void SetDropSize (float dropwidth, float dropheight) = 0;
  /// Get the size of the fire drops.
  virtual void GetDropSize (float& dropwidth, float& dropheight) const = 0;
  /// Set origin of the fire.
  virtual void SetOrigin (const csBox3& origin) = 0;
  /// Get origin of the fire.
  virtual const csBox3& GetOrigin () const = 0;
  /// Set direction of the fire.
  virtual void SetDirection (const csVector3& dir) = 0;
  /// Get direction of the fire.
  virtual const csVector3& GetDirection () const = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// See if lighting is enabled.
  virtual bool GetLighting () const = 0;
  /// Set swirl.
  virtual void SetSwirl (float swirl) = 0;
  /// Get swirl.
  virtual float GetSwirl () const = 0;
  /// Set color scale.
  virtual void SetColorScale (float colscale) = 0;
  /// Get color scale.
  virtual float GetColorScale () const = 0;
  /// Set total time.
  virtual void SetTotalTime (float tottime) = 0;
  /// Get total time.
  virtual float GetTotalTime () const = 0;
};

/** @} */

#endif // __CS_IMESH_FIRE_H__

