/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_PARTSYS_H__
#define __CS_IMESH_PARTSYS_H__

/**\file
 * Particle system interface
 */ 

#include "csutil/scf.h"

/**\addtogroup meshplugins
 * @{ */

struct iMaterialWrapper;
class csColor;

SCF_VERSION (iParticleState, 0, 0, 2);

/**
 * This interface describes the API for the particle mesh object.
 */
struct iParticleState : public iBase
{
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
  /// Set the alpha of particles.
  virtual void SetAlpha(float alpha) = 0;
  /// Get the probable alpha of the particles
  virtual float GetAlpha() const = 0;
  /// Set change color.
  virtual void SetChangeColor (const csColor& color) = 0;
  /// Unset change color.
  virtual void UnsetChangeColor () = 0;
  /// see if change color is enabled, and get a copy if so.
  virtual bool GetChangeColor (csColor& col) const = 0;
  /// Set change size of all particles, by factor per second.
  virtual void SetChangeSize (float factor) = 0;
  /// Unset change of size.
  virtual void UnsetChangeSize () = 0;
  /// see if change size is enabled, and get the value if so.
  virtual bool GetChangeSize (float& factor) const = 0;
  /// Change rotation of all particles, by angle in radians per second.
  virtual void SetChangeRotation (float angle) = 0;
  /// Stop change of rotation.
  virtual void UnsetChangeRotation () = 0;
  /// see if change rotation is enabled, and get the angle if so.
  virtual bool GetChangeRotation (float& angle) const = 0;
  /// Change alpha of all particles, by factor per second.
  virtual void SetChangeAlpha (float factor) = 0;
  /// Stop change of alpha.
  virtual void UnsetChangeAlpha () = 0;
  /// see if change alpha is enabled, and get the value if so.
  virtual bool GetChangeAlpha (float& factor) const = 0;
  /// Set selfdestruct mode on, and msec to live.
  virtual void SetSelfDestruct (csTicks t) = 0;
  /// System will no longer self destruct.
  virtual void UnSetSelfDestruct () = 0;

};

/** @} */

#endif // __CS_IMESH_PARTSYS_H__

