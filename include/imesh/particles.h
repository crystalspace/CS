/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#include "csutil/scf_interface.h"

#include "csgeom/obb.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/quaternion.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/ref.h"

/**\file
 * Particle System Interface
 */

struct iParticleSystemBase;

/**\addtogroup meshplugins
 * @{ */

/**\name Particle systems
 * @{ */

/**
 * Sorting modes to be used by the particle renderer
 */
enum csParticleSortMode
{
  /// No sorting at all
  CS_PARTICLE_SORT_NONE,
  /// Sort by the distance to the camera
  CS_PARTICLE_SORT_DISTANCE,
  /// Sort by the dot product of the normalized camera vector and the particle direction
  CS_PARTICLE_SORT_DOT
};

/**
 * Particle orientation.
 * The particle type defines how the billboard is setup in relation to the
 * particle position, direction and the camera.
 */
enum csParticleRenderOrientation
{
  /**
   * Billboard always facing the camera, with exact computation per particle.
   * This is what you usually expect from a billboard.
   */
  CS_PARTICLE_CAMERAFACE,
  /**
   * Billboard always facing the camera direction. This is approximately
   * the same as CS_PARTICLE_CAMERAFACE, but is a bit optimized by utilizing
   * the camera direction instead of the exact direction from particle to
   * the camera,
   */
  CS_PARTICLE_CAMERAFACE_APPROX,
  /**
   * Orient billboard around a common direction (y/up direction), facing the 
   * camera. All particles will use the same common direction.
   */
  CS_PARTICLE_ORIENT_COMMON,
  /**
   * Orient billboard around a common direction (y/up direction), facing the 
   * camera. This is approximately the same as CS_PARTICLE_ORIENT_SAME_APPROX, 
   * but is a bit optimized by utilizing the camera direction instead of the 
   * exact direction from particle to the camera,
   */
  CS_PARTICLE_ORIENT_COMMON_APPROX,
  /**
   * Orient billboard around a common direction (y/up direction), facing the 
   * camera. The particles will use their direction (velocity) vector as common
   * direction.
   */
  CS_PARTICLE_ORIENT_VELOCITY,
  /**
   * Orient the particles according to their internal rotation.
   * The billboard will be aligned so that the normal is along the z axis
   * and the particle in the xy plane of the base specified.
   */
  CS_PARTICLE_ORIENT_SELF,
  /**
   * Orient the particles according to their internal rotation.
   * The billboard will be aligned so that the normal is along the z axis
   * and the particle in the xy plane of the base specified.
   * This differs from CS_PARTICLE_ORIENT_SELF in the sense that the particles
   * will always have their "forward" side towards the camera
   */
  CS_PARTICLE_ORIENT_SELF_FORWARD
};

/**
 * Rotation mode.
 * Specifies how particle rotation is handled.
 */
enum csParticleRotationMode
{
  /// Do not take rotation into account at all
  CS_PARTICLE_ROTATE_NONE,
  /// Rotate texture coordinates
  CS_PARTICLE_ROTATE_TEXCOORD,
  /// Rotate particle vertices in the billboard plane
  CS_PARTICLE_ROTATE_VERTICES
};

/**
 * Particle integration mode.
 * Specifies how the velocity-to-position integration is done.
 * Default is CS_PARTICLE_INTEGRATE_LINEAR
 */
enum csParticleIntegrationMode
{
  /// Perform no integration
  CS_PARTICLE_INTEGRATE_NONE,
  /// Integrate linear velocity into linear position
  CS_PARTICLE_INTEGRATE_LINEAR,
  /**
   * Integrate both linear and angular velocity into pose. Notice that the
   * angular integration is rather performance heavy so use only when needed.
   */
  CS_PARTICLE_INTEGRATE_BOTH
};

/**
 * Particle transformation mode.
 * Controls how and when particles are transformed, and thereby also controls
 * the coordinate system for particles, emitters and effectors.
 */
enum csParticleTransformMode
{
  /**
   * Fully local mode. 
   * All positions and coordinates are relative to particle system.
   */
  CS_PARTICLE_LOCAL_MODE,
  /**
   * Mixed coordinate mode.
   * Particle position and effectors are specified in world space, while
   * emitters operate in local mode.
   * \warning Do note that this mode will introduce extra overhead compared
   * to the other two modes and use only when neccesary.
   */
  CS_PARTICLE_LOCAL_EMITTER,
  /**
   * Fully global mode.
   * All coordinates are in world space (absolute space).
   */
  CS_PARTICLE_WORLD_MODE
};

/**
 * Data representation of a single particle.
 */
struct csParticle
{
  /**
   * Position.
   * In absolute (world) space or relative to system depending on properties 
   * of the particle system.
   */
  csVector3 position;

  /**
   * Particle mass
   */
  float mass;

  /**
   * Orientation of a single particle.
   * 
   * The particle is defined to have its normal along the z-axis and thus
   * lies in the xy plane defined by this quaternion.
   */
  csQuaternion orientation;
  
  /**
   * Current linear velocity.
   * In absolute (world) space or relative to system depending on properties 
   * of the particle system.
   */
  csVector3 linearVelocity;

  /**
   * Current time to live
   */
  float timeToLive;

  /**
   * Angular velocity.
   */
  csVector3 angularVelocity;

  // Pad to make this struct 64 bytes
  float pad;
};

/**
 * Auxiliary data per particle, not used as often
 */
struct csParticleAux
{
  /**
   * Current color value
   */
  csColor4 color;

  /**
   * Size of particle. Only used if particle renderer is set to use individual
   * sizes of the particles
   */
  csVector2 particleSize;

  // Pad to make this 32 bytes
  float pad[2];
};

/**
 * Buffer holder for particle buffers.
 */
struct csParticleBuffer
{
  /// Main particle data
  csParticle* particleData;
  
  /// Auxiliary data, indexed in same way as the main data
  csParticleAux* particleAuxData;

  /// Number of valid particles in the buffer
  size_t particleCount;
};

/**
 * A particle emitter.
 * The particle emitters are responsible for adding new particles and
 * setting up their initial state.
 */
struct iParticleEmitter : public virtual iBase
{
  SCF_INTERFACE(iParticleEmitter,1,0,0);

  /**
   * Set whether or not this emitter is enabled. 
   * The emitter will emit particles only if it is enabled.
   */
  virtual void SetEnabled (bool enabled) = 0;

  /**
   * Get whether or not this emitter is enabled. 
   */
  virtual bool GetEnabled () const = 0;

  /**
   * Set the start time (in seconds) for this emitter.
   * By default emitters will start emitting particles as soon as the
   * particle system is activated (comes into view), but with this setting
   * this can be delayed.
   */
  virtual void SetStartTime (float time) = 0;

  /**
   * Get the start time (in seconds)
   */
  virtual float GetStartTime () const = 0;

  /**
   * Set the duration (in seconds) for this emitter.
   * By default emitters will emit particles infinitely, but by setting this
   * you can make them stop a given number of seconds after they initiated
   * emission.
   * A negative duration is the same as infinite duration.
   */
  virtual void SetDuration (float time) = 0;

  /**
   * Get the duration (in seconds) for this emitter.
   */
  virtual float GetDuration () const = 0;

  /**
   * Set the emission rate, in particles per second.
   */
  virtual void SetEmissionRate (float particlesPerSecond) = 0;

  /**
   * Get the emission rate, in particles per second.
   */
  virtual float GetEmissionRate () const = 0;

  /**
   * Set the initial time-to-live span of the particles emitted.
   * The emitter will assign a time-to-live in the range specified.
   */
  virtual void SetInitialTTL (float min, float max) = 0;

  /**
   * Get the initial time-to-live span of the particles emitted.
   */
  virtual void GetInitialTTL (float& min, float& max) const= 0;

  /**
   * Set the initial mass of the new particles.
   * The emitter will assign a mass in the range specified.
   */
  virtual void SetInitialMass (float min, float max) = 0;

  /**
   * Get the initial mass of the new particles.   
   */
  virtual void GetInitialMass (float& min, float& max) const = 0;

  /**
   * Clone this emitter
   */
  virtual csPtr<iParticleEmitter> Clone () const = 0;

  /**
   * Get the number of particles this emitter wants to emit
   * \param system The particle system for which particles may be emitted
   * \param dt The time step during which some particles may be emitted,
   * in seconds (the number of particles emitted should be equal to this,
   * times the emission rate).
   * \param totalTime The total time since the particle system has started
   * emitting, in seconds and including \a dt.
   */
  virtual size_t ParticlesToEmit (iParticleSystemBase* system,
    float dt, float totalTime) = 0;

  /**
   * Spawn some new particles. The number of particles to be emitted has
   * be defined through the last call to ParticlesToEmit().
   * \param system The particle system for which particles may be emitted
   * \param particleBuffer The storage place for the data of the new
   * particles to be emitted
   * \param dt The time step during which some particles may be emitted,
   * in seconds (the number of particles emitted should be equal to this,
   * times the emission rate).
   * \param totalTime The total time since the particle system has started
   * emitting, in seconds and including \a dt.
   * \param emitterToParticle A local transform to apply on the position
   * of the new particles emitted
   */
  virtual void EmitParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime,
    const csReversibleTransform* const emitterToParticle = 0) = 0;

};

/**
 * Base interface for particle effectors.
 * A particle effector is an object which affects the movement and lifetime
 * of particles, such as simple forces (gravity), 
 */
struct iParticleEffector : public virtual iBase
{
  SCF_INTERFACE(iParticleEffector,1,0,0);

  /**
   * Clone this effector
   */
  virtual csPtr<iParticleEffector> Clone () const = 0;

  /**
   * Calculate effect on particles and update their velocities
   */
  virtual void EffectParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime) = 0;
};


/** 
 * Base properties for particle system.
 * Interface shared between particle system and particle system factories,
 * where the factory will use the values as defaults for newly created particle
 * systems.
 */
struct iParticleSystemBase : public virtual iBase
{
  SCF_INTERFACE(iParticleSystemBase, 1,0,0);

  /// Set particle type generated
  virtual void SetParticleRenderOrientation (csParticleRenderOrientation o) = 0;

  /// Get particle type
  virtual csParticleRenderOrientation GetParticleRenderOrientation () const = 0;

  /// Set particle rotation mode
  virtual void SetRotationMode (csParticleRotationMode mode) = 0;

  /// Get particle rotation mode
  virtual csParticleRotationMode GetRotationMode () const = 0;

  /// Set particle sort mode
  virtual void SetSortMode (csParticleSortMode mode) = 0;

  /// Get particle sort mode
  virtual csParticleSortMode GetSortMode () const = 0;

  /// Set particle integration mode
  virtual void SetIntegrationMode (csParticleIntegrationMode mode) = 0;

  /// Get particle integration mode
  virtual csParticleIntegrationMode GetIntegrationMode () = 0;

  /// Set the common direction
  virtual void SetCommonDirection (const csVector3& direction) = 0;

  /// Get the common direction
  virtual const csVector3& GetCommonDirection () const = 0;

  /// Set transform mode
  virtual void SetTransformMode (csParticleTransformMode mode) = 0;

  /// Get transform mode
  virtual csParticleTransformMode GetTransformMode () const = 0;

  /// Set if particles should use specified or their own size
  virtual void SetUseIndividualSize (bool individual) = 0;

  /// Get if particles should use specified or their own size
  virtual bool GetUseIndividualSize () const = 0;

  /// Set common size of particles
  virtual void SetParticleSize (const csVector2& size) = 0;

  /// Get common size of particles
  virtual const csVector2& GetParticleSize () const = 0;

  /// Set the smallest bounding box particle system should use
  virtual void SetMinBoundingBox (const csBox3& box) = 0;
  
  /// Get the smallest bounding box particle system should use
  virtual const csBox3& GetMinBoundingBox () const = 0;

  /// Add an emitter to the system. The particle should increment the reference.
  virtual void AddEmitter (iParticleEmitter* emitter) = 0;

  /// Get an emitter
  virtual iParticleEmitter* GetEmitter (size_t index) const = 0;

  /// Remove an emitter by index
  virtual void RemoveEmitter (size_t index) = 0;

  /// Get total number of emitters 
  virtual size_t GetEmitterCount () const = 0;

  /// Add an effector to the system. The particle should increment the reference.
  virtual void AddEffector (iParticleEffector* effector) = 0;

  /// Get an effector
  virtual iParticleEffector* GetEffector (size_t index) const = 0;

  /// Remove an effector by index
  virtual void RemoveEffector (size_t index) = 0;

  /// Get total number of effector 
  virtual size_t GetEffectorCount () const = 0;

};


/**
 * Properties for particle system factory.
 */
struct iParticleSystemFactory : public iParticleSystemBase
{
  SCF_INTERFACE(iParticleSystemFactory,1,0,0);

  /**
   * Set if emitters and effectors should be deep-copied (cloned) when creating
   * a particle system or if the ones in the factory should be reused.
   */
  virtual void SetDeepCreation (bool deep) = 0;

  /// Get whether or not deep copy should be used
  virtual bool GetDeepCreation () const = 0;
};


/**
 * Properties for particle system object.
 */
struct iParticleSystem : public iParticleSystemBase
{
  SCF_INTERFACE(iParticleSystem,1,0,1);

  /// Get number of particles currently in the system
  virtual size_t GetParticleCount () const = 0;

  /// Get a specific particle
  virtual csParticle* GetParticle (size_t index) = 0;

  /// Get the auxiliary data of a specific particle
  virtual csParticleAux* GetParticleAux (size_t index) = 0;

  /**
   * Lock the particles and take external control over them. The particle system
   * will no more use the emitters and effectors, so the particles have to be
   * animated manually.
   * \param maxParticles Amount of particles for which memory is allocated in
   * the returned particles buffer. (The actual number of provided particles 
   * must be set there; obviously it can't exceed \a maxParticles.)
   */
  virtual csParticleBuffer* LockForExternalControl (size_t maxParticles) = 0;
  
  /**
   * Advance the time of the particle system object by the given duration.
   * This is useful to "fill" a particle system after its initial creation.
   * \remarks Internally, the time is advanced in multiple steps of a smaller
   * duration. This means that the run time needed to advance a particle
   * system grows proportionally with the time to advance!
   */
  virtual void Advance (csTicks time) = 0;
};

/** @} */

/**\name Default particle system emitters
* @{ */

/// Set where in the emitter the builtin emitters should spawn their particles
enum csParticleBuiltinEmitterPlacement
{
  /// In the center
  CS_PARTICLE_BUILTIN_CENTER,
  /// Anywhere in the volume
  CS_PARTICLE_BUILTIN_VOLUME,
  /// On the surface of the volume
  CS_PARTICLE_BUILTIN_SURFACE
};

/**
 * Base interface for the emitters already built-in.
 */
struct iParticleBuiltinEmitterBase : public iParticleEmitter
{
  SCF_INTERFACE(iParticleBuiltinEmitterBase,1,0,0);

  /**
   * Set the position of the emitter.
   *
   * \sa iParticleSystemBase::SetLocalMode
   */
  virtual void SetPosition (const csVector3& position) = 0;

  /// Get the position of the emitter
  virtual const csVector3& GetPosition () const = 0;

  /// Set the initial particle placement
  virtual void SetParticlePlacement (csParticleBuiltinEmitterPlacement place) = 0;

  /// Get the initial particle placement
  virtual csParticleBuiltinEmitterPlacement GetParticlePlacement () const = 0;

  /**
   * Set the initial velocity assignment strategy. 
   *
   * Uniform velocity means that direction is always "outward pushing" 
   * (exactly what that is depends on the shape of the emitter, for example
   * sphere emitter give radial velocity). When using uniform velocity only
   * the magnitude is used from the set velocity vector.
   * 
   * Opposite to uniform is to use a single velocity vector for new particles.
   * 
   * Default should be uniform velocity distribution.
   */
  virtual void SetUniformVelocity (bool uniform) = 0;

  /// Get the initial velocity strategy
  virtual bool GetUniformVelocity () const = 0;

  /// Set the initial velocity/magnitude of the emitted particles
  virtual void SetInitialVelocity (const csVector3& linear, 
    const csVector3& angular) = 0;

  /// Get the initial velocity/magnitude of the emitted particles
  virtual void GetInitialVelocity (csVector3& linear, 
    csVector3& angular) const = 0;
};

/**
 * An emitter spawning the new particles around a sphere geometry
 */
struct iParticleBuiltinEmitterSphere : public iParticleBuiltinEmitterBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterSphere,1,0,0);

  /// Set sphere radius
  virtual void SetRadius (float radius) = 0;

  /// Get sphere radius
  virtual float GetRadius () const = 0;
};

/**
 * An emitter spawning the new particles around a cone geometry
 */
struct iParticleBuiltinEmitterCone : public iParticleBuiltinEmitterBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterCone,1,0,0);

  /// Set cone extent vector (from center to end-point)
  virtual void SetExtent (const csVector3& extent) = 0;

  /// Get cone extent vector (from center to end-point)
  virtual const csVector3& GetExtent () const = 0;

  /**
   * Set cone angle, angle between center line and cone surface (in radians)
   * Range: [0, Pi/2)
   */
  virtual void SetConeAngle (float angle) = 0;

  /// Get cone angle, angle between center line and cone surface (in radians)
  virtual float GetConeAngle () const = 0;
};

/**
 * An emitter spawning the new particles around a box geometry
 */
struct iParticleBuiltinEmitterBox : public iParticleBuiltinEmitterBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterBox,1,0,0);

  /// Set emitter box
  virtual void SetBox (const csOBB& box) = 0;

  /// Get emitter box
  virtual const csOBB& GetBox () const = 0;
};

/**
 * An emitter spawning the new particles around a cylinder geometry
 */
struct iParticleBuiltinEmitterCylinder : public iParticleBuiltinEmitterBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterCylinder,1,0,0);

  /// Set cylinder radius
  virtual void SetRadius (float radius) = 0;

  /// Get cylinder radius
  virtual float GetRadius () const = 0;

  /// Set cylinder extent vector (from center to one end-point)
  virtual void SetExtent (const csVector3& extent) = 0;

  /// Get cylinder extent vector (from center to one end-point)
  virtual const csVector3& GetExtent () const = 0;
};

/**
 * Factory for built-in emitters
 */
struct iParticleBuiltinEmitterFactory : public virtual iBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterFactory,1,0,0);

  /// Create a 'sphere' particle emitter
  virtual csPtr<iParticleBuiltinEmitterSphere> CreateSphere () const = 0;

  /// Create a 'cone' particle emitter
  virtual csPtr<iParticleBuiltinEmitterCone> CreateCone () const = 0;

  /// Create a 'box' particle emitter
  virtual csPtr<iParticleBuiltinEmitterBox> CreateBox () const = 0;

  /// Create a 'cylinder' particle emitter
  virtual csPtr<iParticleBuiltinEmitterCylinder> CreateCylinder () const = 0;
};

/** @} */

/**\name Default particle system effectors
* @{ */

/**
 * Simple force/acceleration applied to particles.
 * 
 * The new velocity of particles is computed by a simple formula
 *
 * v' = v + (a+f/m)*dt
 *
 * v  - old velocity (vector)
 * v' - new velocity (vector)
 * a  - constant acceleration (vector)
 * f  - force (vector)
 * m  - particle mass (scalar)
 */
struct iParticleBuiltinEffectorForce : public iParticleEffector
{
  SCF_INTERFACE(iParticleBuiltinEffectorForce,2,0,0);
  
  /// Set constant acceleration vector
  virtual void SetAcceleration (const csVector3& acceleration) = 0;

  /// Get constant acceleration vector
  virtual const csVector3& GetAcceleration () const = 0;

  /// Set the force vector
  virtual void SetForce (const csVector3& force) = 0;

  /// Get the force vector
  virtual const csVector3& GetForce () const = 0;

  /// Set random acceleration magnitude.
  virtual void SetRandomAcceleration (const csVector3& magnitude) = 0;

  /// Get random acceleration magnitude
  virtual const csVector3& GetRandomAcceleration () const = 0;
};

/**
 * Simple linear interpolation of particle color based on particle lifetime
 *
 * The age of particle P is defined as max(0, maxAge - P.TTL)
 *
 * The first color value is regarded as having time 0, independently of what
 * it is set to have
 */
struct iParticleBuiltinEffectorLinColor : public iParticleEffector
{
  SCF_INTERFACE(iParticleBuiltinEffectorLinColor,1,1,0);

  /** 
   * Add color to list of colors to interpolate between.
   * \return Index of new color
   */
  virtual size_t AddColor (const csColor4& color, float endTTL) = 0;

  /**
   * Remove a specific entry.
   */
  virtual void RemoveColor (size_t index) = 0;

  /**
   * Remove all entries.
   */
  virtual void Clear () = 0;

  /**
   * Set the color of an already existing entry
   */
  virtual void SetColor (size_t index, const csColor4& color) = 0;

  /**
   * Set the TTL for an already existing entry.
   */
  virtual void SetEndTTL (size_t index, float ttl) = 0;

  /**
   * Get color and time
   */
  virtual void GetColor (size_t index, csColor4& color, float& endTTL) const = 0;

  /**
   * Get color.
   */
  virtual const csColor4& GetColor (size_t index) const = 0;
  /**
   * Get TTL.
   */
  virtual float GetEndTTL (size_t index) const = 0;

  /**
   * Get number of color entries
   */
  virtual size_t GetColorCount () const = 0;
};

/**
 * Velocity field effector types
 * Determine the ODE the velocity field effector will solve to get new particle
 * positions from current ones.
 */
enum csParticleBuiltinEffectorVFType
{
  /**
   * Spiral around a given line.
   *
   * ODE:
   * pl = closest point on line defined by vparam[0] + t*vparam[1]
   * p' = vparam[2] * p-pl x vparam[1] + (p-pl) * fparam[0] + vparam[3]
   */
  CS_PARTICLE_BUILTIN_SPIRAL,

  /**
   * Exhort a radial movement relative to a given point.
   *
   * ODE:
   * p' = p-vparam[0] / |p-vparam[0]| * (fparam[0] + fparam[1] * sin(t))
   */
  CS_PARTICLE_BUILTIN_RADIALPOINT
};

/**
 * Velocity field effector.
 *
 * The velocity field effector works by taking a function that defines the velocity
 * as a function of point in space and time, and then integrate the position 
 * according to this function.
 *
 * The functions can have a number of (optional) scalar and vector parameters.
 *
 * \sa csParticleBuiltinEffectorFFType 
 */
struct iParticleBuiltinEffectorVelocityField : public iParticleEffector
{
  SCF_INTERFACE(iParticleBuiltinEffectorVelocityField,2,0,0);

  /**
   * Set force field type
   */
  virtual void SetType (csParticleBuiltinEffectorVFType type) = 0;

  /**
   * Get force field type
   */
  virtual csParticleBuiltinEffectorVFType GetType () const = 0;

  /**
   * Set scalar parameter
   */
  virtual void SetFParameter (size_t parameterNumber, float value) = 0;
  
  /**
   * Get value of scalar parameter
   */
  virtual float GetFParameter (size_t parameterNumber) const = 0;

  /**
   * Get the number of set scalar parameters
   */
  virtual size_t GetFParameterCount () const = 0;

  /**
   * Add an F parameter.
   */
  virtual void AddFParameter(float value) = 0;

  /**
   * Remove an F parameter.
   */
  virtual void RemoveFParameter(size_t index) = 0;

  /**
   * Set vector parameter
   */
  virtual void SetVParameter (size_t parameterNumber, const csVector3& value) = 0;
  
  /**
   * Get value of vector parameter
   */
  virtual csVector3 GetVParameter (size_t parameterNumber) const = 0;

  /**
   * Get the number of set vector parameters
   */
  virtual size_t GetVParameterCount () const = 0;

  /**
   * Add a V parameter.
   */
  virtual void AddVParameter(const csVector3& value) = 0;

  /**
   * Remove a V parameter.
   */
  virtual void RemoveVParameter(size_t index) = 0;

};

/**
 * Mask to influence which parameters we will interpolate in the
 * linear effector (iParticleBuiltinEffectorLinear).
 */
enum csParticleParameterMask
{
  /// Mass
  CS_PARTICLE_MASK_MASS = 1,
  /// Linear velocity
  CS_PARTICLE_MASK_LINEARVELOCITY = 2,
  /// Angular velocity
  CS_PARTICLE_MASK_ANGULARVELOCITY = 4,
  /// Color
  CS_PARTICLE_MASK_COLOR = 8,
  /// Particle size
  CS_PARTICLE_MASK_PARTICLESIZE = 16,

  /// All parameters
  CS_PARTICLE_MASK_ALL = CS_PARTICLE_MASK_MASS | CS_PARTICLE_MASK_LINEARVELOCITY |
    CS_PARTICLE_MASK_ANGULARVELOCITY | CS_PARTICLE_MASK_COLOR | CS_PARTICLE_MASK_PARTICLESIZE
};

/**
 * Parameters that can be modified based on age for the linear
 * effector (iParticleBuiltinEffectorLinear).
 */
struct csParticleParameterSet
{
  /// Mass
  float mass;
  /// Linear velocity
  csVector3 linearVelocity;
  /// Angular velocity
  csVector3 angularVelocity;
  /// Color
  csColor4 color;
  /// Particle size
  csVector2 particleSize;

  csParticleParameterSet ()
  {
    Clear ();
  }

  /// Set all parameters to 0.
  void Clear ()
  {
    mass = 0.0;
    linearVelocity.Set (0, 0, 0);
    angularVelocity.Set (0, 0, 0);
    color.Set (0, 0, 0, 0);
    particleSize.Set (0, 0);
  }
};

/**
 * Linear interpolation of various parameters based on particle lifetime
 *
 * The age of particle P is defined as max(0, maxAge - P.TTL)
 *
 * The first values are regarded as having time 0, independently of what
 * they are set to have.
 */
struct iParticleBuiltinEffectorLinear : public iParticleEffector
{
  SCF_INTERFACE(iParticleBuiltinEffectorLinear,1,1,0);

  /**
   * Set the mask to influence which parameters we will interpolate. By default
   * this will be set to #CS_PARTICLE_MASK_ALL.
   */
  virtual void SetMask (int mask) = 0;

  /**
   * Get the current mask used to interpolate the parameters.
   */
  virtual int GetMask () const = 0;

  /** 
   * Add a parameter set to the list of parameters to interpolate between.
   * \return Index of new parameter
   */
  virtual size_t AddParameterSet (const csParticleParameterSet& param, float endTTL) = 0;

  /**
   * Remove a specific entry.
   */
  virtual void RemoveParameterSet (size_t index) = 0;

  /**
   * Remove all entries.
   */
  virtual void Clear () = 0;

  /**
   * Overwrite the parameter set of an already existing entry
   */
  virtual void SetParameterSet (size_t index, const csParticleParameterSet& param) = 0;

  /**
   * Set the TTL for an index.
   */
  virtual void SetEndTTL (size_t index, float ttl) = 0;

  /**
   * Get parameter set and time
   */
  virtual void GetParameterSet (size_t index, csParticleParameterSet& param, float& endTTL) const = 0;

  /**
   * Get parameter set and time
   */
  virtual const csParticleParameterSet& GetParameterSet (size_t index) const = 0;

  /**
   * Get TTL.
   */
  virtual float GetEndTTL (size_t index) const = 0;

  /**
   * Get number of parameter set entries
   */
  virtual size_t GetParameterSetCount () const = 0;
};

/**
 * This effector will create and attach a iLight to each particle of the
 * system.
 *
 * The position, orientation, base and specular colors of each light
 * will be copied from the particle it is attached to, while its cutoff
 * distance will be modified by the alpha value of the particle's color.
 */
struct iParticleBuiltinEffectorLight : public iParticleEffector
{
  SCF_INTERFACE(iParticleBuiltinEffectorLight,1,0,0);

  /**
   * Set the initial cutoff distance of the lights. The actual value for
   * each light will be the value given here, times the alpha component of
   * the particle's color. The initial value is 5.0f.
   */
  virtual void SetInitialCutoffDistance (float distance) = 0;

  /// Get the initial cutoff distance of the lights.
  virtual float GetInitialCutoffDistance () const = 0;
};

/**
 * Factory for builtin effectors
 */
struct iParticleBuiltinEffectorFactory : public virtual iBase
{
  SCF_INTERFACE(iParticleBuiltinEffectorFactory,1,0,2);

  /// Create a 'force' particle effector
  virtual csPtr<iParticleBuiltinEffectorForce> CreateForce () const = 0;

  /// Create a 'linear color' particle effector
  virtual csPtr<iParticleBuiltinEffectorLinColor> CreateLinColor () const = 0;

  /// Create a 'velocity field' particle effector
  virtual csPtr<iParticleBuiltinEffectorVelocityField> CreateVelocityField () const = 0;

  /// Create a 'linear' particle effector
  virtual csPtr<iParticleBuiltinEffectorLinear> CreateLinear () const = 0;

  /// Create a 'light' particle effector
  virtual csPtr<iParticleBuiltinEffectorLight> CreateLight () const = 0;
};

/** @} */

/** @} */


#endif // __CS_IMESH_PARTICLES_H__
