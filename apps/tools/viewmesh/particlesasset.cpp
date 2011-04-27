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

void ParticlesAsset::DeleteProperty (Property& property)
{
  switch (property.type)
  {
  case Bool:
    delete static_cast<bool*> (property.valPtr);
    break;

  case Float:
    delete static_cast<float*> (property.valPtr);
    break;

  case Vector2:
    delete static_cast<csVector2*> (property.valPtr);
    break;

  case Vector3:
    delete static_cast<csVector3*> (property.valPtr);
    break;

  case Color4:
    delete static_cast<csColor4*> (property.valPtr);
    break;

  case Enum:
    delete static_cast<int*> (property.valPtr);
    break;

  default:
    break;
  }
}

ParticlesAsset::~ParticlesAsset()
{
  for(size_t i=0; i<commonProps.GetSize(); ++i)
    DeleteProperty (commonProps[i]);

  for(size_t i=0; i<sphereProps.GetSize(); ++i)
    DeleteProperty (sphereProps[i]);

  for(size_t i=0; i<coneProps.GetSize(); ++i)
    DeleteProperty (coneProps[i]);

  for(size_t i=0; i<boxProps.GetSize(); ++i)
    DeleteProperty (boxProps[i]);

  for(size_t i=0; i<cylinderProps.GetSize(); ++i)
    DeleteProperty (cylinderProps[i]);

  for(size_t i=0; i<forceProps.GetSize(); ++i)
    DeleteProperty (forceProps[i]);

  for(size_t i=0; i<lincolorProps.GetSize(); ++i)
    DeleteProperty (lincolorProps[i]);

  for(size_t i=0; i<velfieldProps.GetSize(); ++i)
    DeleteProperty (velfieldProps[i]);

  for(size_t i=0; i<linearProps.GetSize(); ++i)
    DeleteProperty (linearProps[i]);


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
      desc.Format ("LinCol(%d)", (int) lc->GetColorCount());
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

iParticleEmitter* ParticlesAsset::GetEmitter(uint indx)
{
  if(indx < object->GetEmitterCount())
  {
    return object->GetEmitter(indx);
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

  // Init the property array.
  if(commonProps.IsEmpty())
  {
    float* duration = new float;
    commonProps.Push(Property(Float, static_cast<void*>(duration)));

    float* emissionRate = new float;
    commonProps.Push(Property(Float, static_cast<void*>(emissionRate)));

    bool* enabled = new bool;
    commonProps.Push(Property(Bool, static_cast<void*>(enabled)));

    csVector2* initMass = new csVector2;
    commonProps.Push(Property(Vector2, static_cast<void*>(initMass)));

    csVector2* initTTL = new csVector2;
    commonProps.Push(Property(Vector2, static_cast<void*>(initTTL)));

    csVector3* linVel = new csVector3;
    commonProps.Push(Property(Vector3, static_cast<void*>(linVel)));

    csVector3* angVel = new csVector3;
    commonProps.Push(Property(Vector3, static_cast<void*>(angVel)));

    csParticleBuiltinEmitterPlacement* placement = new csParticleBuiltinEmitterPlacement;
    commonProps.Push(Property(Enum, static_cast<void*>(placement)));

    csVector3* position = new csVector3;
    commonProps.Push(Property(Vector3, static_cast<void*>(position)));

    float* startTime = new float;
    commonProps.Push(Property(Float, static_cast<void*>(startTime)));

    bool* uniformVel = new bool;
    commonProps.Push(Property(Bool, static_cast<void*>(uniformVel)));
  }

  float* duration = (float*)commonProps[0].valPtr;
  *duration = emitter->GetDuration();
  desc.Format("Duration(%g)", *duration);
  arr->Push(desc);

  float* emissionRate = (float*)commonProps[1].valPtr;
  *emissionRate = emitter->GetEmissionRate();
  desc.Format("Emission Rate(%g)", *emissionRate);
  arr->Push(desc);

  bool* enabled = (bool*)commonProps[2].valPtr;
  *enabled = emitter->GetEnabled();
  if(*enabled) arr->Push("Enabled(true)");
  else arr->Push("Enabled(false)");

  csVector2* initMass = (csVector2*)commonProps[3].valPtr;
  emitter->GetInitialMass(initMass->x, initMass->y);
  desc.Format("Init Mass(%g, %g)", initMass->x, initMass->y);
  arr->Push(desc);

  csVector2* initTTL = (csVector2*)commonProps[4].valPtr;
  emitter->GetInitialTTL(initTTL->x, initTTL->y);
  desc.Format("Init TTL(%g, %g)", initTTL->x, initTTL->y);
  arr->Push(desc);

  csVector3* linVel = (csVector3*)commonProps[5].valPtr;
  csVector3* angVel = (csVector3*)commonProps[6].valPtr;
  emitter->GetInitialVelocity(*linVel, *angVel);

  desc.Format("Init Lin Vel(%g, %g, %g)", linVel->x, linVel->y, linVel->z);
  arr->Push(desc);
  desc.Format("Init Ang Vel(%g, %g, %g)", angVel->x, angVel->y, angVel->z);
  arr->Push(desc);

  csParticleBuiltinEmitterPlacement* placement = (csParticleBuiltinEmitterPlacement*)commonProps[7].valPtr;
  *placement = emitter->GetParticlePlacement();
  if(*placement == CS_PARTICLE_BUILTIN_CENTER) arr->Push("Placement(center)");
  else if(*placement == CS_PARTICLE_BUILTIN_SURFACE) arr->Push("Placement(surface)");
  else if(*placement == CS_PARTICLE_BUILTIN_VOLUME) arr->Push("Placement(volume)");

  csVector3* position = (csVector3*)commonProps[8].valPtr;
  *position = emitter->GetPosition();
  desc.Format("Position(%g, %g, %g)", position->x, position->y, position->z);
  arr->Push(desc);

  float* startTime = (float*)commonProps[9].valPtr;
  *startTime = emitter->GetStartTime();
  desc.Format("Start Time(%g)", *startTime);
  arr->Push(desc);

  bool* uniformVel = (bool*)commonProps[10].valPtr;
  *uniformVel = emitter->GetUniformVelocity();
  if(*uniformVel) arr->Push("Uniform Vel(true)");
  else arr->Push("Uniform Vel(false)");

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetSphereProps(iParticleBuiltinEmitterSphere* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterSphere>(emitter);

  // Init the property array.
  if(sphereProps.IsEmpty())
  {
    float* radius = new float;
    sphereProps.Push(Property(Float, static_cast<void*>(radius)));
  }

  csString desc;
  float* radius = (float*)sphereProps[0].valPtr;
  *radius = emitter->GetRadius();
  desc.Format("Radius(%g)", *radius);
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetConeProps(iParticleBuiltinEmitterCone* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterCone>(emitter);

  // Init the property array.
  if(coneProps.IsEmpty())
  {
    float* angle = new float;
    coneProps.Push(Property(Float, static_cast<void*>(angle)));

    csVector3* extent = new csVector3;
    coneProps.Push(Property(Vector3, static_cast<void*>(extent)));
  }

  csString desc;

  float* angle = (float*)coneProps[0].valPtr;
  *angle = emitter->GetConeAngle();
  desc.Format("Cone Angle(%g)", *angle);
  arr->Push(desc);

  csVector3* extent = (csVector3*)coneProps[1].valPtr;
  *extent = emitter->GetExtent();
  desc.Format("Extent(%g, %g, %g)", extent->x, extent->y, extent->z);
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetBoxProps(iParticleBuiltinEmitterBox* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterBox>(emitter);

  // Init the property array.
  if(boxProps.IsEmpty())
  {
    csOBB* box = new csOBB;
    boxProps.Push(Property(Unknown, static_cast<void*>(box)));
  }

  csString desc;

  csOBB* box = (csOBB*)boxProps[0].valPtr;
  *box = emitter->GetBox();
  desc.Format("Box(%s)", box->Description().GetData());
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetCylinderProps(iParticleBuiltinEmitterCylinder* emitter)
{
  csRef<iStringArray> arr = GetCommonProps<iParticleBuiltinEmitterCylinder>(emitter);

  // Init the property array.
  if(cylinderProps.IsEmpty())
  {
    float* radius = new float;
    cylinderProps.Push(Property(Float, static_cast<void*>(radius)));

    csVector3* extent = new csVector3;
    cylinderProps.Push(Property(Vector3, static_cast<void*>(extent)));
  }

  csString desc;

  float* radius = (float*)cylinderProps[0].valPtr;
  *radius = emitter->GetRadius();
  desc.Format("Radius(%g)", *radius);
  arr->Push(desc);

  csVector3* extent = (csVector3*)cylinderProps[1].valPtr;
  *extent = emitter->GetExtent();
  desc.Format("Extent(%g, %g, %g)", extent->x, extent->y, extent->z);
  arr->Push(desc);

  return csPtr<iStringArray> (arr);
}

PropType ParticlesAsset::GetCommonPropType(uint indx)
{
  if (indx < commonProps.GetSize())
    return commonProps[indx].type;

  return Unknown;
}

PropType ParticlesAsset::GetSpherePropType(uint indx)
{
  PropType propType = GetCommonPropType(indx);

  if(propType == Unknown)
  {
    if(indx - commonProps.GetSize() < sphereProps.GetSize())
      return sphereProps[indx - commonProps.GetSize()].type;
  }

  return propType;
}

PropType ParticlesAsset::GetConePropType(uint indx)
{
  PropType propType = GetCommonPropType(indx);

  if(propType == Unknown)
  {
    if(indx - commonProps.GetSize() < coneProps.GetSize())
      return coneProps[indx - commonProps.GetSize()].type;
  }

  return propType;
}

PropType ParticlesAsset::GetBoxPropType(uint indx)
{
  PropType propType = GetCommonPropType(indx);

  if(propType == Unknown)
  {
    if(indx - commonProps.GetSize() < boxProps.GetSize())
      return boxProps[indx - commonProps.GetSize()].type;
  }

  return propType;
}

PropType ParticlesAsset::GetCylinderPropType(uint indx)
{
  PropType propType = GetCommonPropType(indx);

  if(propType == Unknown)
  {
    if(indx - commonProps.GetSize() < cylinderProps.GetSize())
      return cylinderProps[indx - commonProps.GetSize()].type;
  }

  return propType;
}

PropType ParticlesAsset::GetEmitterPropType(iParticleEmitter* emitter, uint indx)
{
  // Test for the type.
  csRef<iParticleBuiltinEmitterSphere> sphere = scfQueryInterface<iParticleBuiltinEmitterSphere>(emitter);
  if(sphere) return GetSpherePropType(indx);

  csRef<iParticleBuiltinEmitterCone> cone = scfQueryInterface<iParticleBuiltinEmitterCone>(emitter);
  if(cone) return GetConePropType(indx);

  csRef<iParticleBuiltinEmitterBox> box = scfQueryInterface<iParticleBuiltinEmitterBox>(emitter);
  if(box) return GetBoxPropType(indx);

  csRef<iParticleBuiltinEmitterCylinder> cylinder = scfQueryInterface<iParticleBuiltinEmitterCylinder>(emitter);
  if(cylinder) return GetCylinderPropType(indx);

  return Unknown;
}

template<typename T>
bool ParticlesAsset::GetPropValue(csArray<Property>& properties, uint indx, T& val)
{
  if (indx < properties.GetSize())
  {
    switch(properties[indx].type)
    {
    case Bool:
      {
        val = *static_cast<T*>(properties[indx].valPtr);
        return true;
      }
    case Float:
      {
        val = *static_cast<T*>(properties[indx].valPtr);
        return true;
      }
    case Vector2:
      {
        val = *static_cast<T*>(properties[indx].valPtr);
        return true;
      }
    case Vector3:
      {
        val = *static_cast<T*>(properties[indx].valPtr);
        return true;
      }
    case Color4:
      {
        val = *static_cast<T*>(properties[indx].valPtr);
        return true;
      }
    case Enum:
    case Unknown:
      {
        return false;
      }
    default:
      {
        CS_ASSERT_MSG("Unhandled property type!", false);
        return false;
      }
    }
  }

  return false;
}

template<typename T>
bool ParticlesAsset::GetEmitterPropValueT(iParticleEmitter* emitter, uint id, T& val)
{
  // Test for the type.
  csRef<iParticleBuiltinEmitterSphere> sphere = scfQueryInterface<iParticleBuiltinEmitterSphere>(emitter);
  if(sphere)
  {
    return GetPropValue<T>(commonProps, id, val) ||
      GetPropValue<T>(sphereProps, id - (uint)commonProps.GetSize(), val);
  }

  csRef<iParticleBuiltinEmitterCone> cone = scfQueryInterface<iParticleBuiltinEmitterCone>(emitter);
  if(cone)
  {
    return GetPropValue<T>(commonProps, id, val) ||
      GetPropValue<T>(coneProps, id - (uint)commonProps.GetSize(), val);
  }

  csRef<iParticleBuiltinEmitterBox> box = scfQueryInterface<iParticleBuiltinEmitterBox>(emitter);
  if(box)
  {
    return GetPropValue<T>(commonProps, id, val) ||
      GetPropValue<T>(boxProps, id - (uint)commonProps.GetSize(), val);
  }

  csRef<iParticleBuiltinEmitterCylinder> cylinder = scfQueryInterface<iParticleBuiltinEmitterCylinder>(emitter);
  if(cylinder)
  {
    return GetPropValue<T>(commonProps, id, val) ||
      GetPropValue<T>(cylinderProps, id - (uint)commonProps.GetSize(), val);
  }

  return false;
}

template<typename T>
bool ParticlesAsset::SetPropValue(csArray<Property>& properties, uint id, T& val)
{
  if(id < properties.GetSize())
  {
    *((T*)properties[id].valPtr) = val;
    return true;
  }

  return false;
}

template<typename T>
bool ParticlesAsset::SetEmitterPropValueT(iParticleEmitter* emitter, uint id, T& val)
{
  // Test for the type.
  csRef<iParticleBuiltinEmitterSphere> sphere = scfQueryInterface<iParticleBuiltinEmitterSphere>(emitter);
  if(sphere)
  {
    if(SetPropValue<T>(commonProps, id, val))
    {
      return UpdateCommonProp<iParticleBuiltinEmitterSphere>(sphere, id);
    }
    
    if(SetPropValue<T>(sphereProps, id - (uint)commonProps.GetSize(), val))
    {
      return UpdateSphereProp(sphere, id - (uint)commonProps.GetSize());
    }

    return false;
  }

  csRef<iParticleBuiltinEmitterCone> cone = scfQueryInterface<iParticleBuiltinEmitterCone>(emitter);
  if(cone)
  {
    if(SetPropValue<T>(commonProps, id, val))
    {
      return UpdateCommonProp<iParticleBuiltinEmitterCone>(cone, id);
    }

    if(SetPropValue<T>(coneProps, id - (uint)commonProps.GetSize(), val))
    {
      return UpdateConeProp(cone, id - (uint)commonProps.GetSize());
    }

    return false;
  }

  csRef<iParticleBuiltinEmitterBox> box = scfQueryInterface<iParticleBuiltinEmitterBox>(emitter);
  if(box)
  {
    if(SetPropValue<T>(commonProps, id, val))
    {
      return UpdateCommonProp<iParticleBuiltinEmitterBox>(box, id);
    }

    if(SetPropValue<T>(boxProps, id - (uint)commonProps.GetSize(), val))
    {
      return UpdateBoxProp(box, id - (uint)commonProps.GetSize());
    }

    return false;
  }

  csRef<iParticleBuiltinEmitterCylinder> cylinder = scfQueryInterface<iParticleBuiltinEmitterCylinder>(emitter);
  if(cylinder)
  {
    if(SetPropValue<T>(commonProps, id, val))
    {
      return UpdateCommonProp<iParticleBuiltinEmitterCylinder>(cylinder, id);
    }

    if(SetPropValue<T>(cylinderProps, id - (uint)commonProps.GetSize(), val))
    {
      return UpdateCylinderProp(cylinder, id - (uint)commonProps.GetSize());
    }

    return false;
  }

  return false;
}

template<typename T>
bool ParticlesAsset::UpdateCommonProp(T* emitter, uint id)
{
  switch(id)
  {
  case 0:
    {
      float* duration = (float*)commonProps[0].valPtr;
      emitter->SetDuration(*duration);
      return true;
    }
  case 1:
    {
      float* emissionRate = (float*)commonProps[1].valPtr;
      emitter->SetEmissionRate(*emissionRate);
      return true;
    }
  case 2:
    {
      bool* enabled = (bool*)commonProps[2].valPtr;
      emitter->SetEnabled(*enabled);
      return true;
    }
  case 3:
    {
      csVector2* initMass = (csVector2*)commonProps[3].valPtr;
      emitter->SetInitialMass(initMass->x, initMass->y);
      return true;
    }
  case 4:
    {
      csVector2* initTTL = (csVector2*)commonProps[4].valPtr;
      emitter->SetInitialTTL(initTTL->x, initTTL->y);
      return true;
    }
  case 5:
  case 6:
    {
      csVector3* linVel = (csVector3*)commonProps[5].valPtr;
      csVector3* angVel = (csVector3*)commonProps[6].valPtr;
      emitter->SetInitialVelocity(*linVel, *angVel);
      return true;
    }
  case 7:
    {
      csParticleBuiltinEmitterPlacement* placement = (csParticleBuiltinEmitterPlacement*)commonProps[7].valPtr;
      emitter->SetParticlePlacement(*placement);
      return true;
    }
  case 8:
    {
      csVector3* position = (csVector3*)commonProps[8].valPtr;
      emitter->SetPosition(*position);
      return true;
    }
  case 9:
    {
      float* startTime = (float*)commonProps[9].valPtr;
      emitter->SetStartTime(*startTime);
      return true;
    }
  case 10:
    {
      bool* uniformVel = (bool*)commonProps[10].valPtr;
      emitter->SetUniformVelocity(*uniformVel);
      return true;
    }
  default:
    {
      return false;
    }
  }
}

bool ParticlesAsset::UpdateSphereProp(iParticleBuiltinEmitterSphere* emitter, uint id)
{
  switch(id)
  {
  case 0:
    {
      float* radius = (float*)sphereProps[0].valPtr;
      emitter->SetRadius(*radius);
      return true;
    }
  default:
    {
      return false;
    }
  }
}

bool ParticlesAsset::UpdateConeProp(iParticleBuiltinEmitterCone* emitter, uint id)
{
  switch(id)
  {
  case 0:
    {
      float* angle = (float*)coneProps[0].valPtr;
      emitter->SetConeAngle(*angle);
      return true;
    }
  case 1:
    {
      csVector3* extent = (csVector3*)coneProps[1].valPtr;
      emitter->SetExtent(*extent);
      return true;
    }
  default:
    {
      return false;
    }
  }
}

bool ParticlesAsset::UpdateBoxProp(iParticleBuiltinEmitterBox* emitter, uint id)
{
  switch(id)
  {
  case 0:
    {
      csOBB* box = (csOBB*)boxProps[0].valPtr;
      emitter->SetBox(*box);
      return true;
    }
  default:
    {
      return false;
    }
  }
}

bool ParticlesAsset::UpdateCylinderProp(iParticleBuiltinEmitterCylinder* emitter, uint id)
{
  switch(id)
  {
  case 0:
    {
      float* radius = (float*)cylinderProps[0].valPtr;
      emitter->SetRadius(*radius);
      return true;
    }
  case 1:
    {
      csVector3* extent = (csVector3*)cylinderProps[1].valPtr;
      emitter->SetExtent(*extent);
      return true;
    }
  default:
    {
      return false;
    }
  }  
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

iParticleEffector* ParticlesAsset::GetEffector(uint indx)
{
  if(indx < object->GetEffectorCount())
  {
    return object->GetEffector(indx);
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

  if(forceProps.IsEmpty())
  {
    csVector3* accel = new csVector3;
    forceProps.Push(Property(Vector3, static_cast<void*>(accel)));

    csVector3* force = new csVector3;
    forceProps.Push(Property(Vector3, static_cast<void*>(force)));

    csVector3* randAccel = new csVector3;
    forceProps.Push(Property(Vector3, static_cast<void*>(randAccel)));
  }

  {
    csString desc;
    csVector3* accel = (csVector3*)forceProps[0].valPtr;
    *accel = effector->GetAcceleration();
    desc.Format("Accel(%g, %g, %g)", accel->x, accel->y, accel->z);
    arr->Push(desc);
  }

  {
    csString desc;
    csVector3* force = (csVector3*)forceProps[1].valPtr;
    *force = effector->GetForce();
    desc.Format("Force(%g, %g, %g)", force->x, force->y, force->z);
    arr->Push(desc);
  }

  {
    csString desc;
    csVector3* randAccel = (csVector3*)forceProps[2].valPtr;
    *randAccel = effector->GetRandomAcceleration();
    desc.Format("Rand Accel(%g, %g, %g)", randAccel->x, randAccel->y, randAccel->z);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetLinColorProps(iParticleBuiltinEffectorLinColor* effector)
{
  scfStringArray* arr = new scfStringArray;

  while(lincolorProps.GetSize() < 2 * effector->GetColorCount())
  {
    float* ttl = new float;
    csColor4* color = new csColor4;
    lincolorProps.Push(Property(Color4, static_cast<void*>(color)));
    lincolorProps.Push(Property(Float, static_cast<void*>(ttl)));
  }

  for(size_t i=0; i<effector->GetColorCount(); ++i)
  {
    csString desc;
    csColor4* color = (csColor4*)lincolorProps[2*i].valPtr;
    float* ttl = (float*)lincolorProps[2*i+1].valPtr;
    *color = effector->GetColor(i);
    *ttl = effector->GetEndTTL(i);
    desc.Format("Color(%g, %g, %g, %g)", color->red, color->green, color->blue, color->alpha);
    arr->Push(desc);
    desc.Format("TTL(%g)", *ttl);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetVelFieldProps(iParticleBuiltinEffectorVelocityField* effector)
{
  scfStringArray* arr = new scfStringArray;

  if(velfieldProps.IsEmpty())
  {
    csParticleBuiltinEffectorVFType* vfType = new csParticleBuiltinEffectorVFType;
    velfieldProps.Push(Property(Enum, static_cast<void*>(vfType)));

    for(size_t i=0; i<effector->GetFParameterCount(); ++i)
    {
      float* fParam = new float;
      velfieldProps.Push(Property(Float, static_cast<void*>(fParam)));
    }

    for(size_t i=0; i<effector->GetVParameterCount(); ++i)
    {
      csVector3* vParam = new csVector3;
      velfieldProps.Push(Property(Vector3, static_cast<void*>(vParam)));
    }
  }

  if(effector->GetType() == CS_PARTICLE_BUILTIN_RADIALPOINT)
  {
    csParticleBuiltinEffectorVFType* vfType = (csParticleBuiltinEffectorVFType*)velfieldProps[0].valPtr;
    *vfType = effector->GetType();
    arr->Push("Type(radial point)");
  }
  else if(effector->GetType() == CS_PARTICLE_BUILTIN_SPIRAL)
  {
    csParticleBuiltinEffectorVFType* vfType = (csParticleBuiltinEffectorVFType*)velfieldProps[0].valPtr;
    *vfType = effector->GetType();
    arr->Push("Type(spiral)");
  }

  for(size_t i=0; i<effector->GetFParameterCount(); ++i)
  {
    csString desc;
    float* fParam = (float*)velfieldProps[1+i].valPtr;
    *fParam = effector->GetFParameter(i);
    desc.Format("FParam(%g)", effector->GetFParameter(i));
    arr->Push(desc);
  }

  for(size_t i=0; i<effector->GetVParameterCount(); ++i)
  {
    csString desc;
    csVector3* vParam = (csVector3*)velfieldProps[1+effector->GetFParameterCount()+i].valPtr;
    *vParam = effector->GetVParameter(i);
    desc.Format("VParam(%g, %g, %g)", vParam->x, vParam->y, vParam->z);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}

csPtr<iStringArray> ParticlesAsset::GetLinearProps(iParticleBuiltinEffectorLinear* effector)
{
  scfStringArray* arr = new scfStringArray;

  if(linearProps.IsEmpty())
  {
    for(size_t i=0; i<effector->GetParameterSetCount(); ++i)
    {
      csVector3* angVel = new csVector3;
      linearProps.Push(Property(Vector3, static_cast<void*>(angVel)));

      csColor4* color = new csColor4;
      linearProps.Push(Property(Color4, static_cast<void*>(color)));

      csVector3* linVel = new csVector3;
      linearProps.Push(Property(Vector3, static_cast<void*>(linVel)));

      float* mass = new float;
      linearProps.Push(Property(Float, static_cast<void*>(mass)));

      csVector2* particleSize = new csVector2;
      linearProps.Push(Property(Vector2, static_cast<void*>(particleSize)));
    }
  }

  for(size_t i=0; i<effector->GetParameterSetCount(); ++i)
  {
    csVector3* angularVel = (csVector3*)linearProps[i*5+0].valPtr;
    csColor4* color = (csColor4*)linearProps[i*5+1].valPtr;
    csVector3* linVel = (csVector3*)linearProps[i*5+2].valPtr;
    float* mass = (float*)linearProps[i*5+3].valPtr;
    csVector2* size = (csVector2*)linearProps[i*5+4].valPtr;

    const csParticleParameterSet& paramSet = effector->GetParameterSet(i);
    *angularVel = paramSet.angularVelocity;
    *color = paramSet.color;
    *linVel = paramSet.linearVelocity;
    *mass = paramSet.mass;
    *size = paramSet.particleSize;

    csString desc;
    desc.Format("ParamSet %d, angVel(%s)", (int) i, angularVel->Description().GetData());
    arr->Push(desc);
    desc.Format("ParamSet %d, color(%g, %g, %g, %g)", (int) i, color->red, color->green, color->blue, color->alpha);
    arr->Push(desc);
    desc.Format("ParamSet %d, linVel(%g, %g, %g)", (int) i, linVel->x, linVel->y, linVel->z);
    arr->Push(desc);
    desc.Format("ParamSet %d, mass(%g)", (int) i, *mass);
    arr->Push(desc);
    desc.Format("ParamSet %d, size(%g, %g)", (int) i, size->x, size->y);
    arr->Push(desc);
  }

  return csPtr<iStringArray> (arr);
}

PropType ParticlesAsset::GetForcePropType(uint indx)
{
  if(indx < forceProps.GetSize())
    return forceProps[indx].type;

  return Unknown;
}

PropType ParticlesAsset::GetLinColorPropType(uint indx)
{
  if(indx < lincolorProps.GetSize())
    return lincolorProps[indx].type;

  return Unknown;
}

PropType ParticlesAsset::GetVelFieldPropType(uint indx)
{
  if(indx < velfieldProps.GetSize())
    return velfieldProps[indx].type;

  return Unknown;
}

PropType ParticlesAsset::GetLinearPropType(uint indx)
{
  if(indx < linearProps.GetSize())
    return linearProps[indx].type;

  return Unknown;
}

PropType ParticlesAsset::GetEffectorPropType(iParticleEffector* effector, uint indx)
{
  // Test for the type.
  csRef<iParticleBuiltinEffectorForce> force = scfQueryInterface<iParticleBuiltinEffectorForce>(effector);
  if(force) return GetForcePropType(indx);

  csRef<iParticleBuiltinEffectorLinColor> lincolor = scfQueryInterface<iParticleBuiltinEffectorLinColor>(effector);
  if(lincolor) return GetLinColorPropType(indx);

  csRef<iParticleBuiltinEffectorVelocityField> velfield = scfQueryInterface<iParticleBuiltinEffectorVelocityField>(effector);
  if(velfield) return GetVelFieldPropType(indx);

  csRef<iParticleBuiltinEffectorLinear> linear = scfQueryInterface<iParticleBuiltinEffectorLinear>(effector);
  if(linear) return GetLinearPropType(indx);

  return Unknown;
}

template<typename T>
bool ParticlesAsset::GetEffectorPropValueT(iParticleEffector* effector, uint id, T& val)
{
  // Test for the type.
  csRef<iParticleBuiltinEffectorForce> force = scfQueryInterface<iParticleBuiltinEffectorForce>(effector);
  if(force)
  {
    return GetPropValue<T>(forceProps, id, val);
  }

  csRef<iParticleBuiltinEffectorLinColor> lincolor = scfQueryInterface<iParticleBuiltinEffectorLinColor>(effector);
  if(lincolor)
  {
    return GetPropValue<T>(lincolorProps, id, val);
  }

  csRef<iParticleBuiltinEffectorVelocityField> velfield = scfQueryInterface<iParticleBuiltinEffectorVelocityField>(effector);
  if(velfield)
  {
    return GetPropValue<T>(velfieldProps, id, val);
  }

  csRef<iParticleBuiltinEffectorLinear> linear = scfQueryInterface<iParticleBuiltinEffectorLinear>(effector);
  if(linear)
  {
    return GetPropValue<T>(linearProps, id, val);
  }

  return false;
}

template<typename T>
bool ParticlesAsset::SetEffectorPropValueT(iParticleEffector* effector, uint id, T& val)
{
  // Test for the type.
  csRef<iParticleBuiltinEffectorForce> force = scfQueryInterface<iParticleBuiltinEffectorForce>(effector);
  if(force)
  {
    if(SetPropValue<T>(forceProps, id, val))
    {
      return UpdateForceProp(force, id);
    }

    return false;
  }

  csRef<iParticleBuiltinEffectorLinColor> lincolor = scfQueryInterface<iParticleBuiltinEffectorLinColor>(effector);
  if(lincolor)
  {
    if(SetPropValue<T>(lincolorProps, id, val))
    {
      return UpdateLinColorProp(lincolor, id);
    }

    return false;
  }

  csRef<iParticleBuiltinEffectorVelocityField> velfield = scfQueryInterface<iParticleBuiltinEffectorVelocityField>(effector);
  if(velfield)
  {
    if(SetPropValue<T>(velfieldProps, id, val))
    {
      return UpdateVelFieldProp(velfield, id);
    }

    return false;
  }

  csRef<iParticleBuiltinEffectorLinear> linear = scfQueryInterface<iParticleBuiltinEffectorLinear>(effector);
  if(linear)
  {
    if(SetPropValue<T>(linearProps, id, val))
    {
      return UpdateLinearProp(linear, id);
    }

    return false;
  }

  return false;
}

bool ParticlesAsset::UpdateForceProp(iParticleBuiltinEffectorForce* force, uint id)
{
  switch(id)
  {
  case 0:
    {
      csVector3* accel = (csVector3*)forceProps[0].valPtr;
      force->SetAcceleration(*accel);
      return true;
    }
  case 1:
    {
      csVector3* vForce = (csVector3*)forceProps[1].valPtr;
      force->SetForce(*vForce);
      return true;
    }
  case 2:
    {
      csVector3* randAccel = (csVector3*)forceProps[2].valPtr;
      force->SetRandomAcceleration(*randAccel);
      return true;
    }
  }

  return false;
}

bool ParticlesAsset::UpdateLinColorProp(iParticleBuiltinEffectorLinColor* lincolor, uint id)
{
  if(id < lincolorProps.GetSize())
  {
    if(lincolorProps[id].type == Color4)
    {
      csColor4* color = (csColor4*)lincolorProps[id].valPtr;
      lincolor->SetColor(id/2, *color);
    }
    else if(lincolorProps[id].type == Float)
    {
      float* ttl = (float*)lincolorProps[id].valPtr;
      lincolor->SetEndTTL(id/2, *ttl);
    }

    return true;
  }

  return false;
}

bool ParticlesAsset::UpdateVelFieldProp(iParticleBuiltinEffectorVelocityField* velfield, uint id)
{
  if(0 == id--)
  {
    csParticleBuiltinEffectorVFType* vfType = (csParticleBuiltinEffectorVFType*)velfieldProps[0].valPtr;
    velfield->SetType(*vfType);
    return true;
  }

  if(id < velfield->GetFParameterCount())
  {
    float* fParam = (float*)velfieldProps[id].valPtr;
    velfield->SetFParameter(id, *fParam);
    return true;
  }

  id -= (uint)velfield->GetFParameterCount();
  if(id < velfield->GetVParameterCount())
  {
    csVector3* vParam = (csVector3*)velfieldProps[id].valPtr;
    velfield->SetVParameter(id, *vParam);
    return true;
  }

  return false;
}

bool ParticlesAsset::UpdateLinearProp(iParticleBuiltinEffectorLinear* linear, uint id)
{
  uint paramSet = id / 5;
  csParticleParameterSet newSet(linear->GetParameterSet(paramSet));

  switch(id % 5)
  {
  case 0:
    {
      newSet.angularVelocity = *(csVector3*)linearProps[id].valPtr;
      break;
    }
  case 1:
    {
      newSet.color = *(csColor4*)linearProps[id].valPtr;
      break;
    }
  case 2:
    {
      newSet.linearVelocity = *(csVector3*)linearProps[id].valPtr;
      break;
    }
  case 3:
    {
      newSet.mass = *(float*)linearProps[id].valPtr;
      break;
    }
  case 4:
    {
      newSet.particleSize = *(csVector2*)linearProps[id].valPtr;
      break;
    }
  default:
    {
      break;
    }
  }

  linear->SetParameterSet(paramSet, newSet);

  return true;
}

bool ParticlesAsset::AddProp(iParticleEffector* effector)
{
  // Test for the type.
  csRef<iParticleBuiltinEffectorLinColor> lincolor = scfQueryInterface<iParticleBuiltinEffectorLinColor>(effector);
  if(lincolor)
  {
    lincolor->AddColor(csColor4(0.0f, 0.0f, 0.0f), 0.0f);
    return true;
  }

  csRef<iParticleBuiltinEffectorVelocityField> velfield = scfQueryInterface<iParticleBuiltinEffectorVelocityField>(effector);
  if(velfield)
  {
  }

  return false;
}

bool ParticlesAsset::DeleteProp(iParticleEffector* effector, uint id)
{
  // Test for the type.
  csRef<iParticleBuiltinEffectorLinColor> lincolor = scfQueryInterface<iParticleBuiltinEffectorLinColor>(effector);
  if(lincolor)
  {
    lincolor->RemoveColor(id);
    return true;
  }

  csRef<iParticleBuiltinEffectorVelocityField> velfield = scfQueryInterface<iParticleBuiltinEffectorVelocityField>(effector);
  if(velfield)
  {
    if(id-- == 0)
    {
      return false;
    }

    if(id < velfield->GetFParameterCount())
    {
      velfield->RemoveFParameter(id);
      return true;
    }

    id -= (uint)velfield->GetFParameterCount();
    if(id < velfield->GetVParameterCount())
    {
      velfield->RemoveVParameter(id);
      return true;
    }
  }

  return false;
}
