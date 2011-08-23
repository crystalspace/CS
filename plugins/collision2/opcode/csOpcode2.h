/*
  Copyright (C) 2011 by Liu Lu

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

#ifndef __CS_OPCODE_COLLISION_H__
#define __CS_OPCODE_COLLISION_H__

#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/nobjvec.h"
#include "csutil/scf_implementation.h"
#include "ivaria/collision2.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "csutil/csobject.h"
#include "Opcode.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
class csOpcodeCollisionSystem;
class csOpcodeCollisionObject;

class csOpcodeCollisionSector : public scfImplementationExt1<
  csOpcodeCollisionSector, csObject, CS::Collision2::iCollisionSector>
{

  friend class csOpcodeCollisionObject;
  Opcode::AABBTreeCollider TreeCollider;
  Opcode::RayCollider RayCol;
  Opcode::BVTCache ColCache;

  struct CollisionPortal
  {
    iPortal* portal;
    csOpcodeCollisionObject* ghostPortal1;
  };

  class CollisionGroupVector : public csArray<CS::Collision2::CollisionGroup>
  {
  public:
    CollisionGroupVector () : csArray<CS::Collision2::CollisionGroup> () {}
    static int CompareKey (CS::Collision2::CollisionGroup const& item,
      char const* const& key)
    {
      return strcmp (item.name.GetData (), key);
    }
    static csArrayCmp<CS::Collision2::CollisionGroup, char const*>
      KeyCmp(char const* k)
    {
      return csArrayCmp<CS::Collision2::CollisionGroup, char const*> (k,CompareKey);
    }
  };

  CollisionGroupVector collGroups;
  CS::Collision2::CollisionGroupMask allFilter; 
  int systemFilterCount;

  csRef<iSector> sector;
  csOpcodeCollisionSystem* sys;
  csVector3 gravity;
  csRefArrayObject<csOpcodeCollisionObject> collisionObjects;
  csArray<CollisionPortal> portals;
  csArray<int> collision_faces;
  CS::Collision2::CollisionData curCollisionData;

public:
  csOpcodeCollisionSector (csOpcodeCollisionSystem* sys);
  virtual ~csOpcodeCollisionSector ();

  virtual iObject* QueryObject () {return (iObject*) this;}
  
  //iCollisionSector
  virtual void SetGravity (const csVector3& v);
  virtual csVector3 GetGravity () const {return gravity;}

  virtual void AddCollisionObject(CS::Collision2::iCollisionObject* object);
  virtual void RemoveCollisionObject(CS::Collision2::iCollisionObject* object);

  virtual size_t GetCollisionObjectCount () {return collisionObjects.GetSize ();}
  virtual CS::Collision2::iCollisionObject* GetCollisionObject (size_t index);
  virtual CS::Collision2::iCollisionObject* FindCollisionObject (const char* name);

  virtual void AddPortal(iPortal* portal, const csOrthoTransform& meshTrans);
  virtual void RemovePortal(iPortal* portal);

  virtual void SetSector(iSector* sector) {this->sector = sector;}
  virtual iSector* GetSector(){return sector;}

  virtual CS::Collision2::HitBeamResult HitBeam(const csVector3& start, 
    const csVector3& end);

  virtual CS::Collision2::HitBeamResult HitBeamPortal(const csVector3& start, 
    const csVector3& end);

  virtual CS::Collision2::CollisionGroup& CreateCollisionGroup (const char* name);
  virtual CS::Collision2::CollisionGroup& FindCollisionGroup (const char* name);

  virtual void SetGroupCollision (const char* name1,
    const char* name2, bool collide);
  virtual bool GetGroupCollision (const char* name1,
    const char* name2);

  virtual bool CollisionTest(CS::Collision2::iCollisionObject* object, 
    csArray<CS::Collision2::CollisionData>& collisions);

  virtual void AddCollisionActor (CS::Collision2::iCollisionActor* actor);
  virtual void RemoveCollisionActor ();
  virtual CS::Collision2::iCollisionActor* GetCollisionActor ();

  void AddMovableToSector (CS::Collision2::iCollisionObject* obj);

  void RemoveMovableFromSector (CS::Collision2::iCollisionObject* obj);

  CS::Collision2::HitBeamResult HitBeamCollider (Opcode::Model* model, Point* vertholder, udword* indexholder,
    const csOrthoTransform& trans, const csVector3& start, const csVector3& end, float& depth);

  CS::Collision2::HitBeamResult HitBeamTerrain (csOpcodeCollisionObject* terrainObj, 
    const csVector3& start, const csVector3& end, float& depth);

  CS::Collision2::HitBeamResult HitBeamObject (csOpcodeCollisionObject* object,
    const csVector3& start, const csVector3& end, float& depth);

  bool CollideDetect (Opcode::Model* modelA, Opcode::Model* modelB,
    const csOrthoTransform& transA, const csOrthoTransform& transB);

  void GetCollisionData (Opcode::Model* modelA, Opcode::Model* modelB,
    Point* vertholderA, Point* vertholderB,
    udword* indexholderA, udword* indexholderB,
    const csOrthoTransform& transA, const csOrthoTransform& transB);

  bool CollideObject (csOpcodeCollisionObject* objA, csOpcodeCollisionObject* objB, 
    csArray<CS::Collision2::CollisionData>& collisions, bool recordData = true);

  bool CollideTerrain (csOpcodeCollisionObject* objA, csOpcodeCollisionObject* objB, 
    csArray<CS::Collision2::CollisionData>& collisions, bool recordData = true);
};

class csOpcodeCollisionSystem : public scfImplementation2<
  csOpcodeCollisionSystem, CS::Collision2::iCollisionSystem,
  iComponent>
{
friend class csOpcodeCollider;
private:
  iObjectRegistry* object_reg;
  csRefArrayObject<csOpcodeCollisionSector> collSectors;
  csStringID baseID;
  csStringID colldetID;

public:
  csOpcodeCollisionSystem (iBase* iParent);
  virtual ~csOpcodeCollisionSystem ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* object_reg);

  // iCollisionSystem
  virtual void SetInternalScale (float scale);
  virtual csRef<CS::Collision2::iColliderConvexMesh> CreateColliderConvexMesh (
    iMeshWrapper* mesh, bool simplify = false);
  virtual csRef<CS::Collision2::iColliderConcaveMesh> CreateColliderConcaveMesh (iMeshWrapper* mesh);
  virtual csRef<CS::Collision2::iColliderConcaveMeshScaled> CreateColliderConcaveMeshScaled
    (CS::Collision2::iColliderConcaveMesh* collider, csVector3 scale);
  virtual csRef<CS::Collision2::iColliderCylinder> CreateColliderCylinder (float length, float radius);
  virtual csRef<CS::Collision2::iColliderBox> CreateColliderBox (const csVector3& size);
  virtual csRef<CS::Collision2::iColliderSphere> CreateColliderSphere (float radius);
  virtual csRef<CS::Collision2::iColliderCapsule> CreateColliderCapsule (float length, float radius);
  virtual csRef<CS::Collision2::iColliderCone> CreateColliderCone (float length, float radius);
  virtual csRef<CS::Collision2::iColliderPlane> CreateColliderPlane (const csPlane3& plane);
  virtual csRef<CS::Collision2::iColliderTerrain> CreateColliderTerrain (iTerrainSystem* terrain,
    float minHeight = 0, float maxHeight = 0);

  virtual csRef<CS::Collision2::iCollisionObject> CreateCollisionObject ();
  virtual csRef<CS::Collision2::iCollisionActor> CreateCollisionActor ();
  virtual csRef<CS::Collision2::iCollisionSector> CreateCollisionSector ();
  virtual CS::Collision2::iCollisionSector* FindCollisionSector (const char* name);

  virtual void DecomposeConcaveMesh (CS::Collision2::iCollisionObject* object,
    iMeshWrapper* mesh, bool simplify = false); 

  static void OpcodeReportV (int severity, const char* message, 
    va_list args);

public:
  static iObjectRegistry* rep_object_reg;
};
}
CS_PLUGIN_NAMESPACE_END (Opcode2)
#endif