/*
    Copyright (C) 2010 by Mike Gist

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

#include "cssysdef.h"

#include "csutil/scf.h"
#include "csutil/scfstringarray.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "imesh/particles.h"
#include "imesh/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "particlesasset.h"

bool ParticlesAsset::Support(iMeshWrapper* mesh)
{
  csRef<iParticleSystem> x = scfQueryInterface<iParticleSystem> (mesh->GetMeshObject());
  return x.IsValid();
}

ParticlesAsset::ParticlesAsset(iObjectRegistry* obj_reg, iMeshWrapper* mesh)
  : AssetBase(obj_reg, mesh)
{
  object = scfQueryInterface<iParticleSystem> (mesh->GetMeshObject());
  factory = scfQueryInterface<iParticleSystemFactory> (mesh->GetFactory()->GetMeshObjectFactory());
}

ParticlesAsset::~ParticlesAsset()
{
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  engine->RemoveObject(spritewrapper);
  engine->RemoveObject(spritewrapper->GetFactory());
  spritewrapper.Invalidate();
  object.Invalidate();
  factory.Invalidate();
}

csPtr<iStringArray> ParticlesAsset::GetEmitters()
{
  scfStringArray* arr = new scfStringArray;
  size_t numEmitters = object->GetEmitterCount();

  for (size_t i=0; i<numEmitters; ++i)
  {
    // For each emitter, construct a description string.
    iParticleEmitter* emitter = object->GetEmitter(i);

    csRef<iParticleBuiltinEmitterSphere> sphere = scfQueryInterface<iParticleBuiltinEmitterSphere> (emitter);
    csRef<iParticleBuiltinEmitterCone> cone = scfQueryInterface<iParticleBuiltinEmitterCone> (emitter);
    csRef<iParticleBuiltinEmitterBox> box = scfQueryInterface<iParticleBuiltinEmitterBox> (emitter);
    csRef<iParticleBuiltinEmitterCylinder> cylinder = scfQueryInterface<iParticleBuiltinEmitterCylinder> (emitter);

    csString desc;
    if (sphere)
    {
      desc.Format ("Sphere(%g)", sphere->GetRadius ());
    }
    else if (cone)
    {
      const csVector3& ext = cone->GetExtent ();
      desc.Format ("Cone(%g,%g,%g)", ext.x, ext.y, ext.z);
    }
    else if (box)
    {
      desc.Format ("Box()");
    }
    else if (cylinder)
    {
      desc.Format ("Cylinder(%g)", cylinder->GetRadius ());
    }
    else
    {
      desc.Format ("Unknown(?)");
    }

    arr->Push(desc);
  }

  return csPtr<iStringArray>(arr);
}

csPtr<iStringArray> ParticlesAsset::GetEffectors()
{
  scfStringArray* arr = new scfStringArray;
  size_t numEffectors = object->GetEffectorCount();

  for (size_t i=0; i<numEffectors; ++i)
  {
    // For each effector, construct a description string.
    iParticleEffector* effector = object->GetEffector(i);

    csRef<iParticleBuiltinEffectorForce> force = scfQueryInterface<iParticleBuiltinEffectorForce> (effector);
    csRef<iParticleBuiltinEffectorLinColor> lc = scfQueryInterface<iParticleBuiltinEffectorLinColor> (effector);
    csRef<iParticleBuiltinEffectorVelocityField> vf = scfQueryInterface<iParticleBuiltinEffectorVelocityField> (effector);
    csRef<iParticleBuiltinEffectorLinear> lin = scfQueryInterface<iParticleBuiltinEffectorLinear> (effector);

    csString desc;
    if (force)
    {
      const csVector3& f = force->GetForce ();
      desc.Format ("Force(%g,%g,%g)", f.x, f.y, f.z);
    }
    else if (lc)
    {
      desc.Format ("LinCol(%d)", lc->GetColorCount());
    }
    else if (vf)
    {
      csParticleBuiltinEffectorVFType type = vf->GetType ();
      desc.Format ("VelFld(%s)", (type == CS_PARTICLE_BUILTIN_SPIRAL) ? "spiral" : "radial point");
    }
    else if (lin)
    {
      desc.Format ("Linear()");
    }
    else
    {
      desc.Format ("Unknown(?)");
    }

    arr->Push(desc);
  }

  return csPtr<iStringArray>(arr);
}

iParticleEmitter* ParticlesAsset::AddEmitter(uint type)
{
  csRef<iParticleEmitter> emitter;

  csRef<iParticleBuiltinEmitterFactory> emitterFact = 
    csLoadPluginCheck<iParticleBuiltinEmitterFactory> (
    object_reg, "crystalspace.mesh.object.particles.emitter", false);

  switch(type)
  {
  case 0:
    {
      csRef<iParticleBuiltinEmitterSphere> sphere = emitterFact->CreateSphere();
      emitter = scfQueryInterface<iParticleEmitter>(sphere);
      break;
    }
  case 1:
    {
      csRef<iParticleBuiltinEmitterCone> cone = emitterFact->CreateCone();
      emitter = scfQueryInterface<iParticleEmitter>(cone);
      break;
    }
  case 2:
    {
      csRef<iParticleBuiltinEmitterBox> box = emitterFact->CreateBox();
      emitter = scfQueryInterface<iParticleEmitter>(box);
      break;
    }
  case 3:
    {
      csRef<iParticleBuiltinEmitterCylinder> cylinder = emitterFact->CreateCylinder();
      emitter = scfQueryInterface<iParticleEmitter>(cylinder);
      break;
    }
  default:
    break;
  }

  if(emitter)
  {
    factory->AddEmitter(emitter);
    object->AddEmitter(emitter);
  }

  return emitter;
}

csPtr<iStringArray> ParticlesAsset::GetEmitterProps(uint indx)
{
  if(indx < object->GetEmitterCount())
  {
    return GetEmitterProps(object->GetEmitter(indx));
  }

  return 0;
}

csPtr<iStringArray> ParticlesAsset::GetEmitterProps(iParticleEmitter* emitter)
{
  // Test for the type.
  csRef<iParticleBuiltinEmitterSphere> sphere = scfQueryInterface<iParticleBuiltinEmitterSphere>(emitter);
  if(sphere) return GetSphereProps(sphere);

  csRef<iParticleBuiltinEmitterCone> cone = scfQueryInterface<iParticleBuiltinEmitterCone>(emitter);
  if(cone) return GetConeProps(cone);

  csRef<iParticleBuiltinEmitterBox> box = scfQueryInterface<iParticleBuiltinEmitterBox>(emitter);
  if(box) return GetBoxProps(box);

  csRef<iParticleBuiltinEmitterCylinder> cylinder = scfQueryInterface<iParticleBuiltinEmitterCylinder>(emitter);
  if(cylinder) return GetCylinderProps(cylinder);

  scfStringArray* arr = new scfStringArray;
  return csPtr<iStringArray> (arr);
}

template<typename T>
csPtr<iStringArray> ParticlesAsset::GetCommonProps(T* emitter)
{
  csString desc;
  scfStringArray* arr = new scfStringArray;

  float duration = emitter->GetDuration();
  desc.Format("Duration(%g)", duration);
  arr->Push(desc);

  float emissionRate = emitter->GetEmissionRate();
  desc.Format("Emission Rate(%g)", emissionRate);
  arr->Push(desc);

  bool enabled = emitter->GetEnabled();
  if(enabled) arr->Push("Enabled(true)");
  else arr->Push("Enabled(false)");

  float minMass, maxMass;
  emitter->GetInitialMass(minMass, maxMass);
  desc.Format("Init Mass(%g, %g)", minMass, maxMass);
  arr->Push(desc);

  float minTTL, maxTTL;
  emitter->GetInitialTTL(minTTL, maxTTL);
  desc.Format("Init TTL(%g, %g)", minTTL, maxTTL);
  arr->Push(desc);

  csVector3 linVel, angVel;
  emitter->GetInitialVelocity(linVel, angVel);
  desc.Format("Init Lin Vel(%g, %g, %g)", linVel.x, linVel.y, linVel.z);
  arr->Push(desc);
  desc.Format("Init Ang Vel(%g, %g, %g)", angVel.x, angVel.y, angVel.z);
  arr->Push(desc);

  csParticleBuiltinEmitterPlacement placement = emitter->GetParticlePlacement();
  if(placement == CS_PARTICLE_BUILTIN_CENTER) arr->Push("Placement(center)");
  else if(placement == CS_PARTICLE_BUILTIN_SURFACE) arr->Push("Placement(surface)");
  else if(placement == CS_PARTICLE_BUILTIN_VOLUME) arr->Push("Placement(volume)");

  csVector3 position = emitter->GetPosition();
  desc.Format("Position(%g, %g, %g)", position.x, position.y, position.z);
  arr->Push(desc);

  float startTime = emitter->GetStartTime();
  desc.Format("Start Time(%g)", startTime);
  arr->Push(desc);

  bool uniformVel = emitter->GetUniformVelocity();
  if(uniformVel) arr->Push("Uniform Vel(true)");
  else arr->Push("Uniform Vel(false)");

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetSphereProps(iParticleBuiltinEmitterSphere* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterSphere>(emitter);

  csString desc;
  float radius = emitter->GetRadius();
  desc.Format("Radius(%g)", radius);
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetConeProps(iParticleBuiltinEmitterCone* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterCone>(emitter);

  csString desc;

  float angle = emitter->GetConeAngle();
  desc.Format("Cone Angle(%g)", angle);
  arr->Push(desc);

  const csVector3& extent = emitter->GetExtent();
  desc.Format("Extent(%g, %g, %g)", extent.x, extent.y, extent.z);
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetBoxProps(iParticleBuiltinEmitterBox* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterBox>(emitter);

  csString desc;

  desc.Format("Box(%s)", emitter->GetBox().Description().GetData());
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetCylinderProps(iParticleBuiltinEmitterCylinder* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterCylinder>(emitter);

  csString desc;

  float angle = emitter->GetRadius();
  desc.Format("Radius(%g)", angle);
  arr->Push(desc);

  const csVector3& extent = emitter->GetExtent();
  desc.Format("Extent(%g, %g, %g)", extent.x, extent.y, extent.z);
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

bool ParticlesAsset::DeleteEmitter(uint idx)
{
  if(idx < object->GetEmitterCount())
  {
    factory->RemoveEmitter(idx);
    object->RemoveEmitter(idx);
    return true;
  }

  return false;
}

iParticleEffector* ParticlesAsset::AddEffector(uint type)
{
  csRef<iParticleEffector> effector;

  csRef<iParticleBuiltinEffectorFactory> effectorFact = 
    csLoadPluginCheck<iParticleBuiltinEffectorFactory> (
    object_reg, "crystalspace.mesh.object.particles.effector", false);

  switch(type)
  {
  case 0:
    {
      csRef<iParticleBuiltinEffectorForce> force = effectorFact->CreateForce();
      effector = scfQueryInterface<iParticleEffector>(force);
      break;
    }
  case 1:
    {
      csRef<iParticleBuiltinEffectorLinColor> lincolor = effectorFact->CreateLinColor();
      effector = scfQueryInterface<iParticleEffector>(lincolor);
      break;
    }
  case 2:
    {
      csRef<iParticleBuiltinEffectorVelocityField> velfield = effectorFact->CreateVelocityField();
      effector = scfQueryInterface<iParticleEffector>(velfield);
      break;
    }
  case 3:
    {
      csRef<iParticleBuiltinEffectorLinear> linear = effectorFact->CreateLinear();
      effector = scfQueryInterface<iParticleEffector>(linear);
      break;
    }
  default:
    break;
  }

  if(effector)
  {
    factory->AddEffector(effector);
    object->AddEffector(effector);
  }

  return effector;
}

bool ParticlesAsset::DeleteEffector(uint idx)
{
  if(idx < object->GetEffectorCount())
  {
    factory->RemoveEffector(idx);
    object->RemoveEffector(idx);
    return true;
  }

  return false;
}

csPtr<iStringArray> ParticlesAsset::GetEffectorProps(uint indx)
{
  if(indx < object->GetEffectorCount())
  {
    return GetEffectorProps(object->GetEffector(indx));
  }

  return 0;
}

csPtr<iStringArray> ParticlesAsset::GetEffectorProps(iParticleEffector* effector)
{
  // Test for the type.
  csRef<iParticleBuiltinEffectorForce> force = scfQueryInterface<iParticleBuiltinEffectorForce>(effector);
  if(force) return GetForceProps(force);

  csRef<iParticleBuiltinEffectorLinColor> lincolor = scfQueryInterface<iParticleBuiltinEffectorLinColor>(effector);
  if(lincolor) return GetLinColorProps(lincolor);

  csRef<iParticleBuiltinEffectorVelocityField> velfield = scfQueryInterface<iParticleBuiltinEffectorVelocityField>(effector);
  if(velfield) return GetVelFieldProps(velfield);

  csRef<iParticleBuiltinEffectorLinear> linear = scfQueryInterface<iParticleBuiltinEffectorLinear>(effector);
  if(linear) return GetLinearProps(linear);

  scfStringArray* arr = new scfStringArray;
  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetForceProps(iParticleBuiltinEffectorForce* effector)
{
  scfStringArray* arr = new scfStringArray;

  {
    csString desc;
    const csVector3& accel = effector->GetAcceleration();
    desc.Format("Accel(%g, %g, %g)", accel.x, accel.y, accel.z);
    arr->Push(desc);
  }

  {
    csString desc;
    const csVector3& force = effector->GetForce();
    desc.Format("Force(%g, %g, %g)", force.x, force.y, force.z);
    arr->Push(desc);
  }

  {
    csString desc;
    const csVector3& randAccel = effector->GetRandomAcceleration();
    desc.Format("Rand Accel(%g, %g, %g)", randAccel.x, randAccel.y, randAccel.z);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetLinColorProps(iParticleBuiltinEffectorLinColor* effector)
{
  scfStringArray* arr = new scfStringArray;

  for(size_t i=0; i<effector->GetColorCount(); ++i)
  {
    csString desc;
    const csColor4& color = effector->GetColor(i);
    desc.Format("Color(%g, %g, %g, %g)", color.red, color.green, color.blue, color.alpha);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetVelFieldProps(iParticleBuiltinEffectorVelocityField* effector)
{
  scfStringArray* arr = new scfStringArray;

  if(effector->GetType() == CS_PARTICLE_BUILTIN_RADIALPOINT)
  {
    arr->Push("Type(radial point)");
  }
  else if(effector->GetType() == CS_PARTICLE_BUILTIN_SPIRAL)
  {
    arr->Push("Type(spiral)");
  }

  for(size_t i=0; i<effector->GetFParameterCount(); ++i)
  {
    csString desc;
    desc.Format("FParam(%g)", effector->GetFParameter(i));
    arr->Push(desc);
  }

  for(size_t i=0; i<effector->GetVParameterCount(); ++i)
  {
    csString desc;
    csVector3 vParam = effector->GetVParameter(i);
    desc.Format("VParam(%g, %g, %g)", vParam.x, vParam.y, vParam.z);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetLinearProps(iParticleBuiltinEffectorLinear* effector)
{
  scfStringArray* arr = new scfStringArray;

  for(size_t i=0; i<effector->GetParameterSetCount(); ++i)
  {
    csParticleParameterSet paramSet = effector->GetParameterSet(i);
    csVector3 angularVel = paramSet.angularVelocity;
    csColor4 color = paramSet.color;
    csVector3 linVel = paramSet.linearVelocity;
    float mass = paramSet.mass;
    csVector2 size = paramSet.particleSize;


    csString desc;
    desc.Format("ParamSet %d, angVel(%s)", i, angularVel.Description().GetData());
    arr->Push(desc);
    desc.Format("ParamSet %d, color(%g, %g, %g, %g)", i, color.red, color.green, color.blue, color.alpha);
    arr->Push(desc);
    desc.Format("ParamSet %d, linVel(%g, %g, %g)", i, linVel.x, linVel.y, linVel.z);
    arr->Push(desc);
    desc.Format("ParamSet %d, mass(%g)", i, mass);
    arr->Push(desc);
    desc.Format("ParamSet %d, size(%g, %g)", i, size.x, size.y);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}
