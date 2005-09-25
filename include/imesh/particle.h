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

#ifndef __CS_IMESH_PARTICLE_H__
#define __CS_IMESH_PARTICLE_H__

/**\file
 * Particle interface
 */ 

#include "csutil/scf.h"
#include "csutil/array.h"

/**\addtogroup meshplugins
 * @{ */

struct iLight;
struct iMovable;
struct iRenderView;

class csColor;
class csReversibleTransform;
class csVector3;

struct csRenderMesh;

SCF_VERSION (iParticle, 0, 1, 0);

/**
 * A iParticle can be used in particle Systems.
 * Each particle may perform these operations in it's own manner,
 * Or even do nothing at some of the functions.
 * If some are not implemented, functionality depending on that
 * feature may not work.
 */
struct iParticle : public iBase
{
  /**
   * Set the position of this particle in coordinates relative
   * to the parent particle system.
   */
  virtual void SetPosition (const csVector3& pos) = 0;
  virtual const csVector3& GetPosition () const = 0;
  /**
   * Move the particle relative to position.
   */
  virtual void MovePosition (const csVector3& move) = 0;

  /// Set the color of this particle.
  virtual void SetColor (const csColor& col) = 0;
  /// Add color to the color of the sprite.
  virtual void AddColor (const csColor& col) = 0;
  /// Scale particle by this factor.
  virtual void ScaleBy (float factor) = 0;
  /// Set the MixMode for the particle.
  virtual void SetMixMode (uint mode) = 0;
  /// Rotate the particle is some particle dependent manner, in radians.
  virtual void Rotate (float angle) = 0;

  /**
   * Light this particle.
   * The given transform is the transform of the parent particle system.
   * The position of this particle should be relative to that transform.
   */
  virtual void UpdateLighting (const csArray<iLight*>& lights,
      const csReversibleTransform& transform) = 0;

  /// Get the rendermesh(es) of this particle.
  virtual csRenderMesh** GetRenderMeshes (int& n, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask) = 0;
};

/** @} */

#endif // __CS_IMESH_PARTICLE_H__
