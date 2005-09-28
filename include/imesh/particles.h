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

#include "csutil/scf.h"

#include "csgeom/vector3.h"
#include "csgeom/vector4.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"

/**\file
 * Particle System Interface
 */

/**\addtogroup meshplugins
 * @{ */

struct iMaterialWrapper;

class csColor;
class csMatrix3;
class csReversibleTransform;

/// Particle force falloff type
enum csParticleFalloffType
{
  CS_PART_FALLOFF_CONSTANT,
  CS_PART_FALLOFF_LINEAR,
  CS_PART_FALLOFF_PARABOLIC
};

/// Particle heat function
enum csParticleColorMethod
{
  CS_PART_COLOR_CONSTANT,
  CS_PART_COLOR_LINEAR,
  CS_PART_COLOR_LOOPING,
  CS_PART_COLOR_HEAT,
  CS_PART_COLOR_CALLBACK
};

/// Particle emit type
enum csParticleEmitType
{
  CS_PART_EMIT_SPHERE,
  CS_PART_EMIT_PLANE,
  CS_PART_EMIT_BOX,
  CS_PART_EMIT_CYLINDER
};

/// Particle force type
enum csParticleForceType
{
  CS_PART_FORCE_RADIAL,
  CS_PART_FORCE_LINEAR,
  CS_PART_FORCE_CONE
};

/// Representational information of a particle.
struct csParticlesData
{
  csVector3 position;
  csVector4 color;
  csVector3 velocity;
  float mass;
  float time_to_live;
  float sort;
};

SCF_VERSION (iParticlesColorCallback, 0, 0, 1);

/**
 * Particles state can be set up to retrieve color via a callback.  This
 * interface must be implemented by objects which wish to provide color
 * information to the particles state.
 */
struct iParticlesColorCallback : public iBase
{
  /**
   * Return a color appropriate for a particle at a given time.
   * \param time Time left for particle
   * (1.0 for just born ranging to 0.0 for dead)
   */
  virtual csColor GetColor (float time) = 0;
};

SCF_VERSION (iParticlesStateBase, 1, 1, 0);

/**
 * Particles shared state interface
 */
struct iParticlesStateBase : public iBase
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

  /// Set the emitter type to a cylinder (which can be rotated)
  virtual void SetCylinderEmitType (float radius, float height) = 0;

  /// Get the inner radius for a sphere emitter
  virtual float GetSphereEmitInnerRadius () = 0;

  /// Get the outer radius for a sphere emitter
  virtual float GetSphereEmitOuterRadius () = 0;

  /// Get the X size for a plane or box emitter
  virtual float GetEmitXSize () = 0;

  /// Get the Y size for a plane or box emitter
  virtual float GetEmitYSize () = 0;

  /// Get the Z size for a plane or box emitter
  virtual float GetEmitZSize () = 0;

  /// Get the emitter type
  virtual csParticleEmitType GetEmitType () = 0;

  /// Set a radial force type, with range and falloff type
  virtual void SetRadialForceType (float range, csParticleFalloffType) = 0;

  /// Set a linear force type
  virtual void SetLinearForceType (const csVector3 &direction, float range,
    csParticleFalloffType falloff) = 0;

  /// Set a cone force type
  virtual void SetConeForceType (const csVector3 &direction, float range,
    csParticleFalloffType falloff, float radius,
    csParticleFalloffType radius_falloff) = 0;

  /// Get the force type
  virtual csParticleForceType GetForceType () = 0;

  /// Get force range
  virtual float GetForceRange () = 0;

  /// Get the force falloff types
  virtual void GetFalloffType (csParticleFalloffType &force,
    csParticleFalloffType &cone) = 0;

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
  virtual void SetGravity (const csVector3 &gravity) = 0;

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

  /// Set the color method to a constant color
  virtual void SetConstantColorMethod (csColor4 color) = 0;

  /**
   * Set the color method to linear color (based on time to live 
   * using the gradient (specified above using ClearColors() and
   * AddColor() )
   */
  virtual void SetLinearColorMethod () = 0;

  /**
   * Set the color method to looping color (loops forever, cycling
   * once per seconds specified)
   */
  virtual void SetLoopingColorMethod (float seconds) = 0;

  /**
   * Set the color method to use heat (calculated by the physics plugin)
   * \param base_temp The temperature in degrees C at the emitter
   */
  virtual void SetHeatColorMethod (int base_temp) = 0;

  /// Set the color method to use a callback 
  virtual void SetColorCallback (iParticlesColorCallback*) = 0;

  /// Get the color callback.  Returns null if no callback has been set.
  virtual iParticlesColorCallback* GetColorCallback () = 0;

  /// Add a color to the gradient
  virtual void AddColor (csColor4 color) = 0;

  /// Clear the color gradient
  virtual void ClearColors () = 0;

  /// Get the particle color method
  virtual csParticleColorMethod GetParticleColorMethod () = 0;

  /// Get the constant color (for constant color method)
  virtual void GetConstantColor (csColor4& color) = 0;

  /// Get the color gradient
  virtual const csArray<csColor4> &GetGradient () = 0;

  /// Get the loop time (for looping color method)
  virtual float GetColorLoopTime () = 0;

  /// Get the base heat (for heat color method)
  virtual float GetBaseHeat () = 0;

  /// Set the point radius
  virtual void SetParticleRadius (float radius) = 0;

  /// Get the particle radius
  virtual float GetParticleRadius () = 0;

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

  /// Set whether to apply the mesh's transform to the individual particles
  virtual void SetTransformMode (bool transform) = 0;

  /// Returns true if this particle object uses transform mode
  virtual bool GetTransformMode () = 0;
};


SCF_VERSION (iParticlesObjectState, 1, 0, 1);

/**
 * Particles state object.
 */
struct iParticlesObjectState : public iParticlesStateBase
{
  /// Get emitter position
  virtual void GetEmitPosition (csVector3 &position) = 0;

  /// Get the object rotation matrix
  virtual const csMatrix3 &GetRotation () = 0;

  /// Get the camera transform
  virtual csReversibleTransform GetObjectToCamera () = 0;

  /**
   * Change the particle physics plugin
   * (Defaults to loading 'crystalspace.particles.physics.simple')
   */
  virtual void ChangePhysicsPlugin (const char *plugin) = 0;

  /**
   * (Re)Start the particle emitter. This is automatically called when
   * the particle mesh object is created if autostart is enabled
   * (default:yes)
   */
  virtual void Start () = 0;

  /// Stop this particle object from emitting any more particles
  virtual void Stop () = 0;

  /// Returns true if this particle simulation is running
  virtual bool IsRunning () = 0;

  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;
};

SCF_VERSION (iParticlesFactoryState, 1, 0, 1);

/**
 * Particles factory state.
 */
struct iParticlesFactoryState : public iParticlesStateBase
{
  /// Set the material to use for this particle factory
  virtual void SetMaterial (iMaterialWrapper *material) = 0;

  /// Set whether the emitter automatically starts (default: true)
  virtual void SetAutoStart (bool autostart) = 0;

  /**
   * Set the particle physics plugin
   * (Defaults to 'crystalspace.particles.physics.simple')
   */
  virtual void SetPhysicsPlugin (const char *plugin) = 0;

  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;
};


SCF_VERSION (iParticlesPhysics, 0, 1, 0);

/**
 * Particles physics interface.
 */
struct iParticlesPhysics : public iBase
{
  /**
   * Register a particles object with the physics plugin
   */
  virtual const csArray<csParticlesData> *RegisterParticles (
  	iParticlesObjectState *particles) = 0;

  /**
   * Remove a particles object from the physics plugin
   */
  virtual void RemoveParticles (iParticlesObjectState *particles) = 0;

  /// (Re)Start a particle simulation
  virtual void Start (iParticlesObjectState *particles) = 0;

  /// Stop a particle simulation
  virtual void Stop (iParticlesObjectState *particles) = 0;
};

/** @} */

#endif // __CS_IMESH_PARTICLES_H__
