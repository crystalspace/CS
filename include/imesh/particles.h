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

/**
 * Sorting modes for particle renderer
 */
enum csParticleSortMode
{
  /// No sorting at all
  CS_PARTICLE_SORT_NONE,
  /// Sort by distance to camera
  CS_PARTICLE_SORT_DISTANCE,
  /// Sort by dot product of normalized camera vector and particle direction
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
 * The particle emitters are responsible for adding new particles 
 */
struct iParticleEmitter : public virtual iBase
{
  SCF_INTERFACE(iParticleEmitter,1,0,0);

  /**
   * Set emitters enabled state. 
   * The emitter will only emit particles if enabled.
   */
  virtual void SetEnabled (bool enabled) = 0;

  /**
   * Get emitters enabled state
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
   * Set duration (in seconds) for this emitter.
   * By default emitters will emit particles infinitely, but by setting this
   * you can make them stop a given number of seconds after they initiated
   * emission.
   * A negative duration is the same as infinite duration.
   */
  virtual void SetDuration (float time) = 0;

  /**
   * Get duration (in seconds) for this emitter.
   */
  virtual float GetDuration () const = 0;

  /**
   * Set emission rate in particles per second.
   */
  virtual void SetEmissionRate (float particlesPerSecond) = 0;

  /**
   * Get emission rate in particles per second.
   */
  virtual float GetEmissionRate () const = 0;

  /**
   * Set initial time-to-live span.
   * The emitter will assign a time-to-live in the range specified.
   */
  virtual void SetInitialTTL (float min, float max) = 0;

  /**
   * Get initial time-to-live span.
   */
  virtual void GetInitialTTL (float& min, float& max) const= 0;

  /**
   * Set initial mass for new particles.
   * The emitter will assign a mass in the range specified.
   */
  virtual void SetInitialMass (float min, float max) = 0;

  /**
   * Get initial mass for new particles.   
   */
  virtual void GetInitialMass (float& min, float& max) const = 0;

  /**
   * Clone this emitter
   */
  virtual csPtr<iParticleEmitter> Clone () const = 0;

  /**
   * Get number of particles this emitter wants to emit
   */
  virtual size_t ParticlesToEmit (iParticleSystemBase* system,
    float dt, float totalTime) = 0;

  /**
   * Spawn new particles.
   */
  virtual void EmitParticles (iParticleSystemBase* system,
    const csParticleBuffer& particleBuffer, float dt, float totalTime) = 0;

};

/**
 * Base interface for particle effector.
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

  /**
   * Set if particle system should be in local transform mode.
   * 
   * When a particle system is in local transform mode all positions and
   * orientations, including emitters, effectors and common directory,
   * will be relative to the particle system. This is useful for particle
   * effects that should follow another entity around.
   *
   * The opposite to local mode is global mode, where all coordinates will be
   * in world coordinates.
   */
  virtual void SetLocalMode (bool local) = 0;

  /// Get local mode
  virtual bool GetLocalMode () const = 0;

  /// Set if particles should use specified or their own size
  virtual void SetUseIndividualSize (bool individual) = 0;

  /// Get if particles should use specified or their own size
  virtual bool GetUseIndividualSize () const = 0;

  /// Set common size of particles
  virtual void SetParticleSize (const csVector2& size) = 0;

  /// Get common size of particles
  virtual const csVector2& GetParticleSize () const = 0;

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

  /// Get if deep copy chould be used
  virtual bool GetDeepCreation () const = 0;
};


/**
 * Properties for particle system object.
 */
struct iParticleSystem : public iParticleSystemBase
{
  SCF_INTERFACE(iParticleSystem,1,0,0);

  /// Get number of particles currently in the system
  virtual size_t GetParticleCount () const = 0;

  /// Get a specific particle
  virtual csParticle* GetParticle (size_t index) = 0;

  /// Get aux-data for a specific particle
  virtual csParticleAux* GetParticleAux (size_t index) = 0;

  /**
   * Lock the particles and take external control over them.
   * 
   */
  virtual csParticleBuffer* LockForExternalControl (size_t maxParticles) = 0;
};


/** @} */

/**\addtogroup defaultemitters
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

struct iParticleBuiltinEmitterBase : public iParticleEmitter
{
  SCF_INTERFACE(iParticleBuiltinEmitterBase,1,0,0);

  /**
   * Set position of emitter.
   *
   * \sa iParticleSystemBase::SetLocalMode
   */
  virtual void SetPosition (const csVector3& position) = 0;

  /// Get position
  virtual const csVector3& GetPosition () const = 0;

  /// Set particle placement
  virtual void SetParticlePlacement (csParticleBuiltinEmitterPlacement place) = 0;

  /// Get particle placement
  virtual csParticleBuiltinEmitterPlacement GetParticlePlacement () const = 0;

  /**
   * Set initial velocity assignment strategy. 
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

  /// Get initial velocity strategy
  virtual bool GetUniformVelocity () const = 0;

  /// Set velocity/magnitude for emitted particles
  virtual void SetInitialVelocity (const csVector3& linear, 
    const csVector3& angular) = 0;

  /// Get velocity for emitted particles
  virtual void GetInitialVelocity (csVector3& linear, 
    csVector3& angular) const = 0;
};

struct iParticleBuiltinEmitterSphere : public iParticleBuiltinEmitterBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterSphere,1,0,0);

  /// Set sphere radius
  virtual void SetRadius (float radius) = 0;

  /// Get sphere radius
  virtual float GetRadius () const = 0;
};

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

struct iParticleBuiltinEmitterBox : public iParticleBuiltinEmitterBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterBox,1,0,0);

  /// Set emitter box
  virtual void SetBox (const csOBB& box) = 0;

  /// Get emitter box
  virtual const csOBB& GetBox () const = 0;
};

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
 * Factory for builtin emitter-types
 */
struct iParticleBuiltinEmitterFactory : public virtual iBase
{
  SCF_INTERFACE(iParticleBuiltinEmitterFactory,1,0,0);

  virtual csPtr<iParticleBuiltinEmitterSphere> CreateSphere () const = 0;
  virtual csPtr<iParticleBuiltinEmitterCone> CreateCone () const = 0;
  virtual csPtr<iParticleBuiltinEmitterBox> CreateBox () const = 0;
  virtual csPtr<iParticleBuiltinEmitterCylinder> CreateCylinder () const = 0;
};

/** @} */

/**\addtogroup defaulteffectors
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
  SCF_INTERFACE(iParticleBuiltinEffectorForce,1,0,0);
  
  /// Set constant acceleration vector
  virtual void SetAcceleration (const csVector3& acceleration) = 0;

  /// Get constant acceleration vector
  virtual const csVector3& GetAcceleration () const = 0;

  /// Set the force vector
  virtual void SetForce (const csVector3& force) = 0;

  /// Get the force vector
  virtual const csVector3& GetForce () const = 0;

  /// Set random acceleration magnitude
  virtual void SetRandomAcceleration (float magnitude) = 0;

  /// Get random acceleration magnitude
  virtual float GetRandomAcceleration () const = 0;
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
  SCF_INTERFACE(iParticleBuiltinEffectorLinColor,1,0,0);

  /** 
   * Add color to list of colors to interpolate between.
   * \return Index of new color
   */
  virtual size_t AddColor (const csColor4& color, float endTTL) = 0;

  /**
   * Set the color of an already existing entry
   */
  virtual void SetColor (size_t index, const csColor4& color) = 0;

  /**
   * Get color and time
   */
  virtual void GetColor (size_t index, csColor4& color, float& endTTL) const = 0;

  /**
   * Get number of color entries
   */
  virtual size_t GetColorCount () const = 0;
};

/**
 * Factory for builtin effectors
 */
struct iParticleBuiltinEffectorFactory : public virtual iBase
{
  SCF_INTERFACE(iParticleBuiltinEffectorFactory,1,0,0);

  virtual csPtr<iParticleBuiltinEffectorForce> CreateForce () const = 0;
  virtual csPtr<iParticleBuiltinEffectorLinColor> CreateLinColor () const = 0;
};

/** @} */


#endif // __CS_IMESH_PARTICLES_H__
