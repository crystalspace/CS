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

#ifndef PARTICLESASSET_H__
#define PARTICLESASSET_H__

#include "assetbase.h"

struct iParticleBuiltinEffectorForce;
struct iParticleBuiltinEffectorLinColor;
struct iParticleBuiltinEffectorVelocityField;
struct iParticleBuiltinEffectorLinear;
struct iParticleBuiltinEmitterSphere;
struct iParticleBuiltinEmitterCone;
struct iParticleBuiltinEmitterBox;
struct iParticleBuiltinEmitterCylinder;
struct iParticleSystem;
struct iParticleSystemFactory;

class ParticlesAsset : public AssetBase
{
private:
  csRef<iParticleSystem> object;
  csRef<iParticleSystemFactory> factory;

  template<typename T>
  csPtr<iStringArray> GetCommonProps(T* emitter);
  csPtr<iStringArray> GetSphereProps(iParticleBuiltinEmitterSphere* emitter);
  csPtr<iStringArray> GetConeProps(iParticleBuiltinEmitterCone* emitter);
  csPtr<iStringArray> GetBoxProps(iParticleBuiltinEmitterBox* emitter);
  csPtr<iStringArray> GetCylinderProps(iParticleBuiltinEmitterCylinder* emitter);

  csPtr<iStringArray> GetForceProps(iParticleBuiltinEffectorForce* effector);
  csPtr<iStringArray> GetLinColorProps(iParticleBuiltinEffectorLinColor* effector);
  csPtr<iStringArray> GetVelFieldProps(iParticleBuiltinEffectorVelocityField* effector);
  csPtr<iStringArray> GetLinearProps(iParticleBuiltinEffectorLinear* effector);

  PropType GetCommonPropType(uint indx);
  PropType GetSpherePropType(uint indx);
  PropType GetConePropType(uint indx);
  PropType GetBoxPropType(uint indx);
  PropType GetCylinderPropType(uint indx);

  PropType GetForcePropType(uint indx);
  PropType GetLinColorPropType(uint indx);
  PropType GetVelFieldPropType(uint indx);
  PropType GetLinearPropType(uint indx);

  struct Property
  {
    PropType type;
    void* valPtr;

    Property(PropType type, void* valPtr)
      : type(type), valPtr(valPtr) {}
  };

  template<typename T>
  bool GetPropValue(csArray<Property>& properties, uint indx, T& val);
  template<typename T>
  bool GetEmitterPropValueT(iParticleEmitter* emitter, uint id, T& val);
  template<typename T>
  bool GetEffectorPropValueT(iParticleEffector* effector, uint id, T& val);

  template<typename T>
  bool SetEmitterPropValueT(iParticleEmitter* emitter, uint id, T& val);
  template<typename T>
  bool SetEffectorPropValueT(iParticleEffector* effector, uint id, T& val);

  template<typename T>
  bool SetPropValue(csArray<Property>& properties, uint id, T& val);

  template<typename T>
  bool UpdateCommonProp(T* emitter, uint id);
  bool UpdateSphereProp(iParticleBuiltinEmitterSphere* emitter, uint id);
  bool UpdateConeProp(iParticleBuiltinEmitterCone* emitter, uint id);
  bool UpdateBoxProp(iParticleBuiltinEmitterBox* emitter, uint id);
  bool UpdateCylinderProp(iParticleBuiltinEmitterCylinder* emitter, uint id);

  bool UpdateForceProp(iParticleBuiltinEffectorForce* force, uint id);
  bool UpdateLinColorProp(iParticleBuiltinEffectorLinColor* lincolor, uint id);
  bool UpdateVelFieldProp(iParticleBuiltinEffectorVelocityField* velfield, uint id);
  bool UpdateLinearProp(iParticleBuiltinEffectorLinear* linear, uint id);

  csArray<Property> commonProps;
  csArray<Property> sphereProps;
  csArray<Property> coneProps;
  csArray<Property> boxProps;
  csArray<Property> cylinderProps;
  csArray<Property> forceProps;
  csArray<Property> lincolorProps;
  csArray<Property> velfieldProps;
  csArray<Property> linearProps;

public:
  static bool Support(iMeshWrapper* mesh);

  ParticlesAsset(iObjectRegistry* obj_reg, iMeshWrapper* mesh);
  virtual ~ParticlesAsset();
  void DeleteProperty (Property& property);

  // Animations
  virtual bool SupportsAnimations() { return false; }

  // Sockets
  virtual bool SupportsSockets() { return false; }

  // SubMeshes
  virtual bool SupportsSubMeshes() { return false; }

  // MorphTargets
  virtual bool SupportsMorphTargets() { return false; }

  // Particles
  virtual bool SupportsParticles() { return true; }

  virtual csPtr<iStringArray> GetEmitters();

  virtual csPtr<iStringArray> GetEffectors();

  virtual iParticleEmitter* AddEmitter(uint type);

  virtual bool DeleteEmitter(uint idx);

  virtual iParticleEmitter* GetEmitter(uint indx);

  virtual csPtr<iStringArray> GetEmitterProps(iParticleEmitter* emitter);

  virtual PropType GetEmitterPropType(iParticleEmitter* emitter, uint indx);

  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, bool& val)
  {
    return GetEmitterPropValueT<bool>(emitter, id, val);
  }

  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, float& val)
  {
    return GetEmitterPropValueT<float>(emitter, id, val);
  }

  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector2& val)
  {
    return GetEmitterPropValueT<csVector2>(emitter, id, val);
  }

  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector3& val)
  {
    return GetEmitterPropValueT<csVector3>(emitter, id, val);
  }

  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, csColor4& val)
  {
    return GetEmitterPropValueT<csColor4>(emitter, id, val);
  }

  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, bool& val)
  {
    return SetEmitterPropValueT<bool>(emitter, id, val);
  }

  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, float& val)
  {
    return SetEmitterPropValueT<float>(emitter, id, val);
  }

  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector2& val)
  {
    return SetEmitterPropValueT<csVector2>(emitter, id, val);
  }

  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector3& val)
  {
    return SetEmitterPropValueT<csVector3>(emitter, id, val);
  }

  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, csColor4& val)
  {
    return SetEmitterPropValueT<csColor4>(emitter, id, val);
  }

  virtual iParticleEffector* AddEffector(uint type);

  virtual bool DeleteEffector(uint idx);

  virtual iParticleEffector* GetEffector(uint indx);

  virtual csPtr<iStringArray> GetEffectorProps(iParticleEffector* effector);

  virtual PropType GetEffectorPropType(iParticleEffector* effector, uint indx);

  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, bool& val)
  {
    return GetEffectorPropValueT<bool>(effector, id, val);
  }

  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, float& val)
  {
    return GetEffectorPropValueT<float>(effector, id, val);
  }

  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, csVector2& val)
  {
    return GetEffectorPropValueT<csVector2>(effector, id, val);
  }

  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, csVector3& val)
  {
    return GetEffectorPropValueT<csVector3>(effector, id, val);
  }

  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, csColor4& val)
  {
    return GetEffectorPropValueT<csColor4>(effector, id, val);
  }

  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, bool& val)
  {
    return SetEffectorPropValueT<bool>(effector, id, val);
  }

  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, float& val)
  {
    return SetEffectorPropValueT<float>(effector, id, val);
  }

  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, csVector2& val)
  {
    return SetEffectorPropValueT<csVector2>(effector, id, val);
  }

  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, csVector3& val)
  {
    return SetEffectorPropValueT<csVector3>(effector, id, val);
  }

  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, csColor4& val)
  {
    return SetEffectorPropValueT<csColor4>(effector, id, val);
  }

  virtual bool AddProp(iParticleEffector* effector);
  virtual bool DeleteProp(iParticleEffector* effector, uint id);
};

#endif // PARTICLESASSET_H__
