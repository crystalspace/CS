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

#ifndef __IMESH_EMIT_H__
#define __IMESH_EMIT_H__

#include "csutil/scf.h"

class csColor;

SCF_VERSION (iEmitGen3D, 0, 0, 1);
SCF_VERSION (iEmitFixed, 0, 0, 1);
SCF_VERSION (iEmitSphere, 0, 0, 1);
SCF_VERSION (iEmitBox, 0, 0, 1);
SCF_VERSION (iEmitCone, 0, 0, 1);
SCF_VERSION (iEmitMix, 0, 0, 1);
SCF_VERSION (iEmitLine, 0, 0, 1);
SCF_VERSION (iEmitSphereTangent, 0, 0, 1);
SCF_VERSION (iEmitCylinderTangent, 0, 0, 1);

/**
 * This interface is for objects that can generate 3d vectors, which
 * are used for emitting.
 */
struct iEmitGen3D : public iBase
{
  /// get the 3d value, posibly using a given value
  virtual void GetValue(csVector3& value, csVector3 &given) = 0;
};

/** fixed value emitter - returns a particular point value */
struct iEmitFixed : public iEmitGen3D
{
  /// set the fixed value
  virtual void SetValue(const csVector3& value) = 0;
};

/** sphere value emitter - returns points in a sphere */
struct iEmitSphere : public iEmitGen3D
{
  /// set content, center and min, max radius 
  virtual void SetContent(const csVector3& center, float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& center, float& min, float& max) = 0;
};

/** box value emitter - returns points in an (axis aligned) box */
struct iEmitBox : public iEmitGen3D
{
  /// set content, min and max vector values
  virtual void SetContent(const csVector3& min, const csVector3& max) = 0;
  /// get content
  virtual void GetContent(csVector3& min, csVector3& max) = 0;
};

/** cone value emitter - returns points in a cone */
struct iEmitCone : public iEmitGen3D
{
  /** set content, origin, elevation, azimuth, aperture(opening),
   *  and distance min, distance max from the origin of the cone.
   */
  virtual void SetContent(const csVector3& origin, float elevation,
    float azimuth, float aperture, float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& origin, float& elevation,
    float& azimuth, float& aperture, float& min, float& max) = 0;
};

/** mix value emitter - returns a weighted random mix of other emitters */
struct iEmitMix : public iEmitGen3D
{
  /// add a weighted emitter to the mix
  virtual void AddEmitter(float weight, iEmitGen3D* emit) = 0;
  /// get the total weight in this mix
  virtual float GetTotalWeight() = 0;
  /// get the number of emitters in this mix
  virtual int GetNumberEmitters() = 0;
  /// get content, returns emitters and their weight by a number (0..number-1)
  virtual void GetContent(int num, float& weight, iEmitGen3D*& emit) = 0;
};

/** line value emitter - returns values on the line between start and end */
struct iEmitLine : public iEmitGen3D
{
  /// set content, start and end vector values
  virtual void SetContent(const csVector3& start, const csVector3& end) = 0;
  /// get content
  virtual void GetContent(csVector3& start, csVector3& end) = 0;
};

/** cylinder value emitter - returns values in a cylinder */
struct iEmitCylinder : public iEmitGen3D
{
  /// set content, start and end position of cylinder, min/max distance
  virtual void SetContent(const csVector3& start, const csVector3& end,
    float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& start, csVector3& end,
    float& min, float& max) = 0;
};

/** sphere tangential value emitter - gives direction tangential to sphere
  Uses the given point, gives a tangential direction for that */
struct iEmitSphereTangent : public iEmitGen3D
{
  /// set content, center of sphere, min/max size
  virtual void SetContent(const csVector3& center, float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& center, float& min, float& max) = 0;
};

/** cylinder tangential value emitter - gives direction tangential to cylinder
  Uses the given point, gives a tangential direction for that */
struct iEmitCylinderTangent : public iEmitGen3D
{
  /// set content, start,end of cylinder, min/max size
  virtual void SetContent(const csVector3& start, const csVector3& end, 
    float min, float max) = 0;
  /// get content
  virtual void GetContent(csVector3& start, csVector3& end, 
    float& min, float& max) = 0;
};

SCF_VERSION (iEmitFactoryState, 0, 0, 1);

/**
 * This interface describes the API for the rain mesh factory object.
 */
struct iEmitFactoryState : public iBase
{
  /// create an emitter, you have to set the content
  virtual iEmitFixed* CreateFixed() = 0;
  /// create an emitter
  virtual iEmitBox* CreateBox() = 0;
  /// create an emitter
  virtual iEmitSphere* CreateSphere() = 0;
  /// create an emitter
  virtual iEmitCone* CreateCone() = 0;
  /// create an emitter
  virtual iEmitMix* CreateMix() = 0;
  /// create an emitter
  virtual iEmitLine* CreateLine() = 0;
  /// create an emitter
  virtual iEmitSphereTangent* CreateSphereTangent() = 0;
  /// create an emitter
  virtual iEmitCylinderTangent* CreateCylinderTangent() = 0;

};

SCF_VERSION (iEmitState, 0, 0, 1);

/**
 * This interface describes the API for the rain mesh object.
 */
struct iEmitState : public iBase
{
  /// Set the number of particles to use.
  virtual void SetNumberParticles (int num) = 0;
  /// Get the number of particles used.
  virtual int GetNumberParticles () const = 0;
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

  /** Add an aging moment, they are interpolated.
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
  virtual int GetNumberAging() const = 0;
  /// get the settings of aging moment i (0..number-1)
  virtual void GetAgingMoment(int i, int& time, csColor& color, float &alpha,
    float& swirl, float& rotspeed, float& scale) = 0;
};

#endif

