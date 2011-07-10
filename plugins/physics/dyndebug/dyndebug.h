/*
  Copyright (C) 2010 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_DEBUGDYN_H__
#define __CS_DEBUGDYN_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/array.h"
#include "ivaria/bullet.h"
#include "ivaria/dynamicsdebug.h"

struct iDynamicSystem;
struct iSector;
struct iMeshWrapper;
struct iMaterialWrapper;
class csOrthoTransform;
class csBox3;
class csSphere;

CS_PLUGIN_NAMESPACE_BEGIN(DebugDynamics)
{
  class DynamicsDebugger;
  class BoneKinematicCallback;

  class DebuggerManager : public scfImplementation2<DebuggerManager,
    CS::Debug::iDynamicsDebuggerManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(DebuggerManager);

    DebuggerManager (iBase* parent);

    //-- CS::Debug::iDynamicsDebuggerManager
    virtual CS::Debug::iDynamicSystemDebugger* CreateDebugger ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    //-- Error reporting
    void Report (int severity, const char* msg, ...);

  private:
    iObjectRegistry* object_reg;
    csRefArray<CS::Debug::iDynamicSystemDebugger> debuggers;

    friend class DynamicsDebugger;
  };

  class DynamicsDebugger : public scfImplementation1<DynamicsDebugger,
    CS::Debug::iDynamicSystemDebugger>
  {
  public:
    CS_LEAKGUARD_DECLARE(DynamicsDebugger);

    DynamicsDebugger (DebuggerManager* manager);

    //-- CS::Debug::iDynamicSystemDebugger
    virtual void SetDynamicSystem (iDynamicSystem* system);
    virtual void SetDebugSector (iSector* sector);

    virtual void SetDebugDisplayMode (bool debugMode);
    virtual void UpdateDisplay ();

    virtual void SetStaticBodyMaterial (iMaterialWrapper* material);
    virtual void SetDynamicBodyMaterial (iMaterialWrapper* material);
    virtual void SetBodyStateMaterial (CS::Physics::Bullet::BodyState state,
				       iMaterialWrapper* material);

  private:
    csRef<iMeshWrapper> CreateColliderMesh (iDynamicsSystemCollider* collider,
					    iMaterialWrapper* material);

    csRef<iMeshWrapper> CreateBoxMesh (csBox3 box,
				       iMaterialWrapper* material,
				       csOrthoTransform transform,
				       iSector* sector);

    csRef<iMeshWrapper> CreateSphereMesh (csSphere sphere,
					  iMaterialWrapper* material,
					  iSector* sector);

    csRef<iMeshWrapper> CreateCylinderMesh (float length, float radius,
					    iMaterialWrapper* material,
					    csOrthoTransform transform,
					    iSector* sector);

    csRef<iMeshWrapper> CreateCapsuleMesh (float length, float radius,
					   iMaterialWrapper* material,
					   csOrthoTransform transform,
					   iSector* sector);

    csRef<iMeshWrapper> CreateCustomMesh (csVector3*& vertices, size_t& vertexCount,
					  int*& indices, size_t& triangleCount,
					  iMaterialWrapper* material,
					  csOrthoTransform transform,
					  iSector* sector);

  private:

    struct MeshData
    {
      csRef<iRigidBody> rigidBody;
      csRef<iMeshWrapper> originalMesh;
      csRef<iMeshWrapper> debugMesh;
      csRef<BoneKinematicCallback> callback;
    };

    DebuggerManager* manager;
    csRef<iDynamicSystem> system;
    csRef<iSector> sector;
    csRef<iMaterialWrapper> materials[3];
    bool debugMode;
    csArray<MeshData> storedMeshes;
  };


  class BoneKinematicCallback : public scfImplementation1
    <BoneKinematicCallback, CS::Physics::Bullet::iKinematicCallback>
  {
  public:
    BoneKinematicCallback (iMeshWrapper* mesh,
			   CS::Physics::Bullet::iKinematicCallback* callback);
    ~BoneKinematicCallback ();

    void GetBodyTransform (iRigidBody* body, csOrthoTransform& transform) const;

  private:
    csWeakRef<iMeshWrapper> mesh;
    csRef<CS::Physics::Bullet::iKinematicCallback> callback;

    friend class DynamicsDebugger;
  };

}
CS_PLUGIN_NAMESPACE_END(DebugDynamics)

#endif // __CS_DEBUGDYN_H__
