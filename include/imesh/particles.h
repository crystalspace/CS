/*
    Copyright (C) 2003 by Jorrit Tyberghein, John Harger, Daniel Duhprey

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

#ifndef __CS_IMESH_PARTICLES_H__
#define __CS_IMESH_PARTICLES_H__

/**\file
 * Particle System Interface
 */

/**
 * \addtogroup gfx3d
 * @{ */

#include "csgeom/vector3.h"
#include "csgeom/vector4.h"

#include "csutil/array.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"

#include "iengine/material.h"

class csColor;

/// Particle force falloff type
enum csParticleFalloffType
{
  CS_PART_FALLOFF_CONSTANT,
  CS_PART_FALLOFF_LINEAR,
  CS_PART_FALLOFF_PARABOLIC
};

/// Particle heat function
enum csParticleHeatFunction
{
  CS_PART_HEAT_CONSTANT,
  CS_PART_HEAT_TIME_LINEAR,
  CS_PART_HEAT_SPEED,
  CS_PART_HEAT_CALLBACK
};

/// Particle emit type
enum csParticleEmitType
{
  CS_PART_EMIT_SPHERE,
  CS_PART_EMIT_PLANE,
  CS_PART_EMIT_BOX
};

/// Particle force type
enum csParticleForceType
{
  CS_PART_FORCE_RADIAL,
  CS_PART_FORCE_LINEAR,
  CS_PART_FORCE_CONE
};

struct csParticlesData
{
  csVector3 position;
  csVector4 color;
  csVector3 velocity;
  float mass;
  float time_to_live;
  float sort;
};

SCF_VERSION (iParticlesObjectState, 0, 0, 1);

struct iParticlesObjectState : public iBase
{
  /// Sets the particles to be emitted per second
  virtual void SetParticlesPerSecond (int count) = 0;

  /// Get the particles emitted per second count
  virtual int GetParticlesPerSecond () = 0;

  /// Set the initial particle burst count
  virtual void SetInitialParticleCount (int count) = 0;

  /// Get the initial particle burst count
  virtual int GetInitialParticleCount () = 0;

  /// Set the emitter type to a point
  virtual void SetPointEmitType () = 0;

  /// Set the emitter type to a sphere (which can have an inner radius)
  virtual void SetSphereEmitType (float outer_radius, float inner_radius) = 0;

  /// Set the emitter type to a plane (which can be rotated)
  virtual void SetPlaneEmitType (float x_size, float y_size) = 0;

  /// Set the emitter type to a box (which can be rotated)
  virtual void SetBoxEmitType (float x_size, float y_size, float z_size) = 0;

  /// Get the emitter type
  virtual csParticleEmitType GetEmitType () = 0;

  /// Get emitter position
  virtual void GetEmitPosition (csVector3 &position) = 0;

  /// Set a radial force type, with range and falloff type
  virtual void SetRadialForceType (float range, csParticleFalloffType falloff) = 0;

  /// Set a linear force type
  virtual void SetLinearForceType (csVector3 &direction, float range, csParticleFalloffType falloff) = 0;

  /// Set a cone force type
  virtual void SetConeForceType (csVector3 &direction, float range, csParticleFalloffType falloff, float radius, csParticleFalloffType radius_falloff) = 0;

  /// Get the force type
  virtual csParticleForceType GetForceType () = 0;

  /// Get force range
  virtual float GetForceRange () = 0;

  /// Get the force falloff types
  virtual void GetFalloffType (csParticleFalloffType &force, csParticleFalloffType &cone) = 0;

  /// Get the force direction
  virtual void GetForceDirection (csVector3 &dir) = 0;

  /// Get the force cone radius
  virtual float GetForceConeRadius () = 0;

  /// Set the force amount
  virtual void SetForce (float force) = 0;

  /// Get the force amount
  virtual float GetForce () = 0;

  /// Set the diffusion amount (random walk) in CS units per second
  virtual void SetDiffusion (float size) = 0;

  /// Get the diffusion amount
  virtual float GetDiffusion () = 0;

  /// Set the gravity vector to effect this particle set
  virtual void SetGravity (csVector3 &gravity) = 0;

  /// Get the gravity vector
  virtual void GetGravity (csVector3 &gravity) = 0;

  /// How many seconds the emitter will be emitting
  virtual void SetEmitTime (float time) = 0;

  /// Get emit time
  virtual float GetEmitTime () = 0;

  /// The time that each particle exists, in seconds
  virtual void SetTimeToLive (float time) = 0;

  /// Get time to live
  virtual float GetTimeToLive () = 0;

  /// Set the random variation in particle time to live, in seconds
  virtual void SetTimeVariation (float variation) = 0;

  /// Get the time variation
  virtual float GetTimeVariation () = 0;

  /// Set the particle heat function to a built-in function or a callback
  virtual void SetParticleHeatFunction(csParticleHeatFunction type, float (*callback)(float time, float speed, float dist) = NULL) = 0;

  /// Add a color to the gradient
  virtual void AddColor (csColor color) = 0;

  /// Clear the color gradient
  virtual void ClearColors () = 0;

  /// Set a color callback (heat is 0.0 for no heat, 1.0 for maximum heat)
  virtual void SetColorCallback (csColor (*callback)(float heat) = NULL) = 0;

  /// Set the particle radius
  virtual void SetParticleRadius (float radius) = 0;

  /// Get the particle radius
  virtual float GetParticleRadius () = 0;

  /// Set the dampener (air viscosity)
  virtual void SetDampener (float damp) = 0;

  /// Set the dampener (air viscosity)
  virtual float GetDampener () = 0;

  /// Set the individual particle mass
  virtual void SetMass (float mass) = 0;

  /// Set the random variation in particle mass
  virtual void SetMassVariation (float variation) = 0;

  /// Get the particle mass
  virtual float GetMass () = 0;

  /// Get the random variation in particle mass
  virtual float GetMassVariation () = 0;

  /// Set whether the emitter automatically starts (default: true)
  virtual void SetAutoStart (bool autostart) = 0;

  /**
   * Change the particle physics plugin
   * (Defaults to loading 'crystalspace.particles.physics.simple')
   */
  virtual void ChangePhysicsPlugin (const char *plugin) = 0;

  /**
   * (Re)Start the particle emitter. This is automatically called when
   * the particle mesh object is created
   */
  virtual void Start () = 0;

  /// Stop this particle system from emitting any more particles
  virtual void Stop () = 0;

  /**
   * Update the particle system (should only be called by an 
   * iParticlesPhysics plugin
   */
  virtual void Update (float elapsed_time) = 0;

};

SCF_VERSION (iParticlesFactoryState, 0, 0, 1);

struct iParticlesFactoryState : public iBase
{
  /// Set the material to use for this particle factory
  virtual void SetMaterial (iMaterialWrapper *material) = 0;

  /// Sets the particles to be emitted per second
  virtual void SetParticlesPerSecond (int count) = 0;

  /// Set the initial particle burst count
  virtual void SetInitialParticleCount (int count) = 0;

  /// Set the emitter type to a point
  virtual void SetPointEmitType () = 0;

  // Set the emitter type to a sphere (which can have an inner radius)
  virtual void SetSphereEmitType (float outer_radius, float inner_radius) = 0;

  /// Set the emitter type to a plane (which can be rotated)
  virtual void SetPlaneEmitType (float x_size, float y_size) = 0;

  /// Set the emitter type to a box (which can be rotated)
  virtual void SetBoxEmitType (float x_size, float y_size, float z_size) = 0;

  /// Set a radial force type, with range and falloff type
  virtual void SetRadialForceType (float range, csParticleFalloffType falloff) = 0;

  /// Set a linear force type
  virtual void SetLinearForceType (csVector3 &direction, float range, csParticleFalloffType falloff) = 0;

  /// Set a cone force type
  virtual void SetConeForceType (csVector3 &direction, float range, csParticleFalloffType falloff, float radius, csParticleFalloffType radius_falloff) = 0;

  /// Set the force amount
  virtual void SetForce (float force) = 0;

  /// Set the diffusion amount (random walk) in CS units per second
  virtual void SetDiffusion (float size) = 0;

  /// Set the gravity vector to effect this particle set
  virtual void SetGravity (csVector3 &gravity) = 0;

  /// How many seconds the emitter will be emitting
  virtual void SetEmitTime (float time) = 0;

  /// The time that each particle exists, in seconds
  virtual void SetTimeToLive (float time) = 0;

  /// Set the random variation in particle time to live, in seconds
  virtual void SetTimeVariation (float variation) = 0;

  /// Set the particle heat function to a built-in function or a callback
  virtual void SetParticleHeatFunction(csParticleHeatFunction type, float (*callback)(float time, float speed, float dist) = NULL) = 0;

  /// Add a color to the gradient
  virtual void AddColor (csColor color) = 0;

  /// Clear the color gradient
  virtual void ClearColors () = 0;

  /// Set a color callback (heat is 0.0 for no heat, 1.0 for maximum heat)
  virtual void SetColorCallback (csColor (*callback)(float heat) = NULL) = 0;

  /// Set the point radius
  virtual void SetParticleRadius (float radius) = 0;

  /// Set the dampener (air viscosity)
  virtual void SetDampener (float damp) = 0;

  /// Set the dampener (air viscosity)
  virtual float GetDampener () = 0;

  /// Set the individual particle mass
  virtual void SetMass(float mass) = 0;

  /// Set the random variation in particle mass
  virtual void SetMassVariation (float variation) = 0;

  /// Get the particle mass
  virtual float GetMass () = 0;

  /// Get the random variation in particle mass
  virtual float GetMassVariation () = 0;

  /// Set whether the emitter automatically starts (default: true)
  virtual void SetAutoStart (bool autostart) = 0;

  /**
   * Set the particle physics plugin
   * (Defaults to 'crystalspace.particles.physics.simple')
   */
  virtual void SetPhysicsPlugin (const char *plugin) = 0;
};


SCF_VERSION (iParticlesPhysics, 0, 0, 1);

struct iParticlesPhysics : public iBase
{
  /**
   * Register a particles object with the physics plugin
   */
  virtual void RegisterParticles (iParticlesObjectState *particles,
    csArray<csParticlesData> *data) = 0;

  /**
   * Remove a particles object from the physics plugin
   */
  virtual void RemoveParticles (iParticlesObjectState *particles) = 0;
};

/** @} */

#endif // __CS_IMESH_PARTICLES_H__
