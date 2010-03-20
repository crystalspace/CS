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
      desc.Format ("VelFld(%s)", (type == CS_PARTICLE_BUILTIN_SPIRAL) ? "spiral" : "radpnt");
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
