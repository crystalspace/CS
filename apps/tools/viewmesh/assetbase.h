/*
    Copyright (C) 2009 by Jelle Hellemans

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

#ifndef IASSET_H__
#define IASSET_H__

#include "cssysdef.h"
#include "csutil/ref.h"
#include "csutil/refcount.h"
#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"

#include <map>
#include <utility>
#include <string>

class csColor4;
class csTransform;
class csVector2;
class csVector3;
struct iMeshWrapper;
struct iMaterialWrapper;
struct iParticleEffector;
struct iParticleEmitter;
struct iStringArray;

struct iObjectRegistry;

typedef std::pair<std::string, std::string> TypeValue;
typedef std::map<std::string, TypeValue> SocketDescriptor;

enum PropType
{
  Bool,
  Float,
  Vector2,
  Vector3,
  Color4,
  Enum,
  Unknown
};

struct AssetBase : public csRefCount
{
protected:
  iObjectRegistry* object_reg;
  csRef<iMeshWrapper> spritewrapper;

  csString selectedAnimation;
  csString selectedSocket;
  csString selectedSubMesh;
  csString selectedMorphTarget;

  csVector3 Decompose(const csMatrix3& m) 
  {
    csVector3 vec;
    // Assuming the angles are in radians.
    if (m.m21 > 0.998f) // singularity at north pole
    { 
      vec.y = atan2(m.m13,m.m33);
      vec.x = PI/2;
      vec.z = 0;
      return vec;
    }
    if (m.m21 < -0.998f) // singularity at south pole
    { 
      vec.y = atan2(m.m13,m.m33);
      vec.x = -PI/2;
      vec.z = 0;
      return vec;
    }
    vec.y = atan2(-m.m31,m.m11); // heading
    vec.x = atan2(-m.m23,m.m22); // bank
    vec.z = asin(m.m21); // attitude

    return vec;
  }

public:
  AssetBase(iObjectRegistry* obj_reg, iMeshWrapper* mesh)
  {
    object_reg = obj_reg;
    spritewrapper = mesh;

    selectedAnimation = "";
    selectedSocket = "";
    selectedSubMesh = "";
    selectedMorphTarget = "";
  }

  virtual ~AssetBase() {}

  virtual iMeshWrapper* GetMesh() { return spritewrapper; }

  // Animations
  virtual bool SupportsAnimations() = 0;

  virtual csPtr<iStringArray> GetAnimations() { return 0; }

  virtual const char* GetSelectedAnimation() { return selectedAnimation.GetData(); }

  virtual void SetSelectedAnimation(const char* animationName) { selectedAnimation = animationName; }

  virtual bool PlayAnimation(const char* animationName, bool cycle) { return false; }

  virtual bool StopAnimation(const char* animationName) { return false; }

  virtual bool RemoveAnimation(const char* animationName) { return false; }

  virtual bool GetReverseAction() { return false; }

  virtual void SetReverseAction(bool value) {}

  virtual void SetAnimationSpeed(float speed) {}

  // Sockets
  virtual bool SupportsSockets() = 0;

  virtual csPtr<iStringArray> GetSockets()  { return 0; }

  virtual const char* GetSelectedSocket()  {return selectedSocket.GetData(); }

  virtual void SetSelectedSocket(const char* socketName) { selectedSocket = socketName; }

  virtual bool AttachMesh(const char* socketName, iMeshWrapper* mesh) { return false; }

  virtual bool AddSocket(const char* socketName) { return false; }

  virtual bool DeleteSocket(const char* socketName) { return false; }

  virtual SocketDescriptor GetSocketTransform(const char* socketName) { return SocketDescriptor(); }

  virtual bool SetSocketTransform(const char* socketName, const SocketDescriptor& desc) { return false; }

  virtual bool DetachAll() { return false; }


  // SubMeshes
  virtual bool SupportsSubMeshes() = 0;

  virtual csPtr<iStringArray> GetSubMeshes() { return 0; }

  virtual const char* GetSelectedSubMesh() { return selectedSubMesh.GetData(); }

  virtual void SetSelectedSubMesh(const char* subMeshName) { selectedSubMesh = subMeshName; }

  virtual bool SetSubMeshRendering(const char* subMeshName, bool value) { return false; }

  virtual bool SetSubMeshMaterial(const char* subMeshName, iMaterialWrapper* mat) { return false; }


  // MorphTargets
  virtual bool SupportsMorphTargets() = 0;

  virtual csPtr<iStringArray> GetMorphTargets() { return 0; }

  virtual const char* GetSelectedMorphTarget() { return selectedMorphTarget.GetData(); }

  virtual void SetSelectedMorphTarget(const char* subMeshName) { selectedMorphTarget = subMeshName; }

  virtual float GetMorphTargetWeight(const char* name) { return 0.0f; }

  virtual bool SetMorphTargetWeight(const char* name, float value) { return false; }

  // Particles
  virtual bool SupportsParticles() = 0;

  virtual csPtr<iStringArray> GetEmitters() { return 0; }

  virtual csPtr<iStringArray> GetEffectors() { return 0; }

  virtual iParticleEmitter* AddEmitter(uint type) { return false; }

  virtual bool DeleteEmitter(uint idx) { return false; }

  virtual iParticleEmitter* GetEmitter(uint indx) { return 0; }

  virtual csPtr<iStringArray> GetEmitterProps(iParticleEmitter* emitter) { return 0; }

  virtual PropType GetEmitterPropType(iParticleEmitter* emitter, uint indx) { return Unknown; }

  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, bool& val) { return false; }
  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, float& val) { return false; }
  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector2& val) { return false; }
  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector3& val) { return false; }
  virtual bool GetEmitterPropValue(iParticleEmitter* emitter, uint id, csColor4& val) { return false; }

  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, bool& val) { return false; }
  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, float& val) { return false; }
  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector2& val) { return false; }
  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, csVector3& val) { return false; }
  virtual bool SetEmitterPropValue(iParticleEmitter* emitter, uint id, csColor4& val) { return false; }

  virtual iParticleEffector* AddEffector(uint type) { return false; }

  virtual bool DeleteEffector(uint idx) { return false; }

  virtual iParticleEffector* GetEffector(uint indx) { return 0; }

  virtual csPtr<iStringArray> GetEffectorProps(iParticleEffector* effector) { return 0; }

  virtual PropType GetEffectorPropType(iParticleEffector* effector, uint indx) { return Unknown; }

  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, bool& val) { return false; }
  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, float& val) { return false; }
  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, csVector2& val) { return false; }
  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, csVector3& val) { return false; }
  virtual bool GetEffectorPropValue(iParticleEffector* effector, uint id, csColor4& val) { return false; }

  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, bool& val) { return false; }
  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, float& val) { return false; }
  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, csVector2& val) { return false; }
  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, csVector3& val) { return false; }
  virtual bool SetEffectorPropValue(iParticleEffector* effector, uint id, csColor4& val) { return false; }

  virtual bool AddProp(iParticleEffector* effector) { return false; }
  virtual bool DeleteProp(iParticleEffector* effector, uint id) { return false; }
};

#endif // IASSET_H__
