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
#include "collisionobject2.h"
#include "Opcode.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
class csOpcodeCollisionSystem;
class csOpcodeCollisionObject;

class csOpcodeCollisionSector : public scfImplementationExt1<
  csOpcodeCollisionSector, csObject, CS::Collision2::iCollisionSector>
{
  csRef<iSector> sector;
  csOpcodeCollisionSystem* sys;
  csVector3 gravity;
  csRefArrayObject<csOpcodeCollisionObject> collisionObjects;
  //actor?
  //TODO portal, collision group

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

  virtual void AddPortal(iPortal* portal);
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
};

class csOpcodeCollisionSystem : public scfImplementation2<
  csOpcodeCollisionSystem, CS::Collision2::iCollisionSystem,
  iComponent>
{
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