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

#ifndef __CS_IMESH_EMIT_H__
#define __CS_IMESH_EMIT_H__

#include "csutil/scf.h"

class csColor;

SCF_VERSION (iEmitGen3D, 0, 0, 1);
SCF_VERSION (iEmitFixed, 0, 0, 1);
SCF_VERSION (iEmitSphere, 0, 0, 1);
SCF_VERSION (iEmitBox, 0, 0, 1);
SCF_VERSION (iEmitCone, 0, 0, 1);
SCF_VERSION (iEmitCylinder, 0, 0, 1);
SCF_VERSION (iEmitMix, 0, 0, 1);
SCF_VERSION (iEmitLine, 0, 0, 1);
SCF_VERSION (iEmitSphereTangent, 0, 0, 1);
SCF_VERSION (iEmitCylinderTangent, 0, 0, 1);

/**
 * This interface is for objects that can generate 3d vectors, which
 * are used for emitting.
 * <p>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitGen3D : public iBase
{
  /// get the 3d value, posibly using a given value.
  virtual void GetValue(csVector3& value, csVector3 &given) = 0;
};

/**
 * Fixed value emitter - returns a particular point value.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateFixed()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitFixed : public iEmitGen3D
{
  /// set the fixed value
  virtual void SetValue(const csVector3& value) = 0;
};

/**
 * Sphere value emitter - returns points in a sphere.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateSphere()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitSphere : public iEmitGen3D
{
  /// set content, center and min, max radius
  virtual void SetContent(const csVector3& center, float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& center, float& min, float& max) = 0;
};

/**
 * Box value emitter - returns points in an (axis aligned) box.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateBox()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitBox : public iEmitGen3D
{
  /// set content, min and max vector values
  virtual void SetContent(const csVector3& min, const csVector3& max) = 0;
  /// get content
  virtual void GetContent(csVector3& min, csVector3& max) = 0;
};

/**
 * Cone value emitter - returns points in a cone.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateCone()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitCone : public iEmitGen3D
{
  /**
   * Set content, origin, elevation, azimuth, aperture(opening),
   * and distance min, distance max from the origin of the cone.
   */
  virtual void SetContent(const csVector3& origin, float elevation,
    float azimuth, float aperture, float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& origin, float& elevation,
    float& azimuth, float& aperture, float& min, float& max) = 0;
};

/**
 * Mix value emitter - returns a weighted random mix of other emitters.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateMix()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitMix : public iEmitGen3D
{
  /// add a weighted emitter to the mix
  virtual void AddEmitter(float weight, iEmitGen3D* emit) = 0;
  /** removes an emitter from the mix given a zero based emitter number.
   *  Use GetEmitterCount() and GetContent() to enumerate through the mix and find the index of an emitter.
   */
  virtual void RemoveEmitter(int num) = 0;
  /// get the total weight in this mix
  virtual float GetTotalWeight() = 0;
  /// get the number of emitters in this mix
  virtual int GetEmitterCount() = 0;
  /** adjust the weight of an emitter given a zero based emitter number
   *  Use GetEmitterCount() and GetContent() to enumerate through the mix and find the index of an emitter.
   */
  virtual void AdjustEmitterWeight(int num,float weight) = 0;
  /// get content, returns emitters and their weight by a number (0..number-1)
  virtual void GetContent(int num, float& weight, iEmitGen3D*& emit) = 0;
};

/**
 * Line value emitter - returns values on the line between start and end.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateLine()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitLine : public iEmitGen3D
{
  /// set content, start and end vector values
  virtual void SetContent(const csVector3& start, const csVector3& end) = 0;
  /// get content
  virtual void GetContent(csVector3& start, csVector3& end) = 0;
};

/**
 * Cylinder value emitter - returns values in a cylinder.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateCylinder()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitCylinder : public iEmitGen3D
{
  /// set content, start and end position of cylinder, min/max distance
  virtual void SetContent(const csVector3& start, const csVector3& end,
    float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& start, csVector3& end,
    float& min, float& max) = 0;
};

/**
 * Sphere tangential value emitter - gives direction tangential to sphere
 * Uses the given point, gives a tangential direction for that.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateSphereTangent()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitSphereTangent : public iEmitGen3D
{
  /// set content, center of sphere, min/max size
  virtual void SetContent(const csVector3& center, float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& center, float& min, float& max) = 0;
};

/**
 * Cylinder tangential value emitter - gives direction tangential to cylinder
 * Uses the given point, gives a tangential direction for that
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iEmitFactoryState::CreateCylinderTangent()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iEmitState
 *   </ul>
 */
struct iEmitCylinderTangent : public iEmitGen3D
{
  /// set content, start,end of cylinder, min/max size
  virtual void SetContent(const csVector3& start, const csVector3& end,
    float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& start, csVector3& end,
    float& min, float& max) = 0;
};

SCF_VERSION (iEmitFactoryState, 0, 0, 2);

/**
 * This interface describes the API for the emitter mesh factory object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Emit mesh object plugin (crystalspace.mesh.object.emit)
 *   <li>iMeshObjectType::NewFactory()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Emit Factory Loader plugin (crystalspace.mesh.loader.factory.emit)
 *   </ul>
 */
struct iEmitFactoryState : public iBase
{
  /// create an emitter, you have to set the content
  virtual csRef<iEmitFixed> CreateFixed() = 0;
  /// create an emitter
  virtual csRef<iEmitBox> CreateBox() = 0;
  /// create an emitter
  virtual csRef<iEmitSphere> CreateSphere() = 0;
  /// create an emitter
  virtual csRef<iEmitCone> CreateCone() = 0;
  /// create an emitter
  virtual csRef<iEmitMix> CreateMix() = 0;
  /// create an emitter
  virtual csRef<iEmitLine> CreateLine() = 0;
  /// create an emitter
  virtual csRef<iEmitCylinder> CreateCylinder() = 0;
  /// create an emitter
  virtual csRef<iEmitSphereTangent> CreateSphereTangent() = 0;
  /// create an emitter
  virtual csRef<iEmitCylinderTangent> CreateCylinderTangent() = 0;
};

SCF_VERSION (iEmitState, 0, 0, 1);

/**
 * This interface describes the API for the emitter mesh object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Emit mesh object plugin (crystalspace.mesh.object.emit)
 *   <li>iMeshObjectFactory::NewInstance()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshWrapper::GetMeshObject()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Emit Loader plugin (crystalspace.mesh.loader.emit)
 *   </ul>
 */
struct iEmitState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetParticleCount (int num) = 0;
  /// Get the number of particles used.
  virtual int GetParticleCount () const = 0;
  /// Enable or disable lighting.
  virtual void SetLighting (bool l) = 0;
  /// See if lighting is enabled.
  virtual bool GetLighting () const = 0;
  /// Set the time to live for all particles, in msec.
  virtual void SetParticleTime(int ttl) = 0;
  /// Get the time to live for all particles, in msec.
  virtual int GetParticleTime() const = 0;
  /// set the particle start position emitter, increfs
  virtual void SetStartPosEmit(iEmitGen3D *emit) = 0;
  /// get the particle start position emitter
  virtual iEmitGen3D* GetStartPosEmit() const = 0;
  /// set the particle start speed emitter, increfs. The position is given.
  virtual void SetStartSpeedEmit(iEmitGen3D *emit) = 0;
  /// get the particle start speed emitter
  virtual iEmitGen3D* GetStartSpeedEmit() const = 0;
  /// set the particle start acceleration emitter, increfs. Position is given.
  virtual void SetStartAccelEmit(iEmitGen3D *emit) = 0;
  /// get the particle start acceleration emitter
  virtual iEmitGen3D* GetStartAccelEmit() const = 0;
  /// set the particle attrator emitter, increfs. Position is given.
  virtual void SetAttractorEmit(iEmitGen3D *emit) = 0;
  /// get the particle attrator emitter. Null means no attractor.
  virtual iEmitGen3D* GetAttractorEmit() const = 0;

  /// Set the force of the attractor (negative gives repulsion)
  virtual void SetAttractorForce(float f) = 0;
  /// Get the force of the attractor
  virtual float GetAttractorForce() const = 0;

  /// Set the field speed emitter, increfs (given position determines speed)
  virtual void SetFieldSpeedEmit(iEmitGen3D *emit) = 0;
  /// get field speed emitter, can be 0
  virtual iEmitGen3D* GetFieldSpeedEmit() const = 0;
  /// Set the field accel emitter, increfs (given position determines accel)
  virtual void SetFieldAccelEmit(iEmitGen3D *emit) = 0;
  /// get field accel emitter, can be 0
  virtual iEmitGen3D* GetFieldAccelEmit() const = 0;

  /**
   * Add an aging moment, they are interpolated.
   * time is the time since creation of the particle in msec.
   * color is a gouraud color to set the particle to. (0..1)
   * alpha can be used to make the particles transparent.
   * the value 0 is a solid particle, the value 1 is an invisible particle
   * the swirl value gives a swirlyness of the movement of the particle.
   * rotspeed is the rotationspeed of the particle (per second).
   * scale is the size of the particle at the time
   */
  virtual void AddAge(int time, const csColor& color, float alpha,
    float swirl, float rotspeed, float scale) = 0;
  /// Get the number of aging moments
  virtual int GetAgingCount() const = 0;
  /// get the settings of aging moment i (0..number-1)
  virtual void GetAgingMoment(int i, int& time, csColor& color, float &alpha,
    float& swirl, float& rotspeed, float& scale) = 0;
  /// remove an aging moment
  virtual void RemoveAge(int time, const csColor& color, float alpha,
        float swirl, float rotspeed, float scale) = 0;
  /// replace the settings for the age at the timepoint given.
  virtual void ReplaceAge(int time, const csColor& color, float alpha,
    float swirl, float rotspeed, float scale) = 0;

  /// Set the particle system to use rectangular particles, given w, h
  virtual void SetRectParticles(float w, float h) = 0;
  /// Set the particle system to use regular shaped particles
  virtual void SetRegularParticles(int n, float radius) = 0;
  /// true if using rect particles. false if using regular particles.
  virtual bool UsingRectParticles() const = 0;
  /// get the size of rect particles;
  virtual void GetRectParticles(float &w, float &h) const = 0;
  /// Get the regular shaped particles sides and radius
  virtual void GetRegularParticles(int& n, float& radius) const = 0;

  /// Set container box, particles are only allowed inside this box.
  virtual void SetContainerBox(bool enabled, const csVector3& min, 
    const csVector3& max) = 0;
  /**
   * Get container box, particles are only allowed inside this box.
   * returns true if the container box is enabled.
   * Objects outside this box are not drawn. But they are also
   * not restarted, since that would cause many short-aged particles. 
   */
  virtual bool GetContainerBox(csVector3& min, csVector3& max) const = 0;
};

#endif // __CS_IMESH_EMIT_H__

