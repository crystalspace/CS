#include "cssysdef.h"
#include "csqsqrt.h"
#include "csgeom/transfrm.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "csutil/scfarray.h"
#include "csOpcode2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
using namespace Opcode;

void Opcode_Log (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csOpcodeCollisionSystem::OpcodeReportV (CS_REPORTER_SEVERITY_NOTIFY, 
    msg, args);
  va_end (args);
}

bool Opcode_Err (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  // Although it's called "..._Err", Opcode also reports less-than-fatal
  // messages through it
  csOpcodeCollisionSystem::OpcodeReportV (CS_REPORTER_SEVERITY_WARNING, 
    msg, args);
  va_end (args);
  return false;
}

csOpcodeCollisionSector::csOpcodeCollisionSector (csOpcodeCollisionSystem* sys)
: scfImplementationType (this), sys (sys)
{

}

csOpcodeCollisionSector::~csOpcodeCollisionSector ()
{

}

void csOpcodeCollisionSector::SetGravity (const csVector3& v)
{

}

void csOpcodeCollisionSector::AddCollisionObject (CS::Collision2::iCollisionObject* object)
{

}

void csOpcodeCollisionSector::RemoveCollisionObject (CS::Collision2::iCollisionObject* object)
{

}

CS::Collision2::iCollisionObject* csOpcodeCollisionSector::GetCollisionObject (size_t index)
{
  return NULL;
}

void csOpcodeCollisionSector::AddPortal (iPortal* portal)
{

}

void csOpcodeCollisionSector::RemovePortal (iPortal* portal)
{

}

CS::Collision2::HitBeamResult csOpcodeCollisionSector::HitBeam(const csVector3& start, 
                                                                const csVector3& end)
{
  CS::Collision2::HitBeamResult result;
  return result;
}

CS::Collision2::HitBeamResult csOpcodeCollisionSector::HitBeamPortal(const csVector3& start, 
                                                                      const csVector3& end)
{
  CS::Collision2::HitBeamResult result;
  return result;
}

CS::Collision2::CollisionGroup& csOpcodeCollisionSector::CreateCollisionGroup (const char* name)
{
  CS::Collision2::CollisionGroup group;
  return group;
}

CS::Collision2::CollisionGroup& csOpcodeCollisionSector::FindCollisionGroup (const char* name)
{
  CS::Collision2::CollisionGroup group;
  return group;
}

void csOpcodeCollisionSector::SetGroupCollision (const char* name1, const char* name2, bool collide)
{

}

bool csOpcodeCollisionSector::GetGroupCollision (const char* name1, const char* name2)
{

  return false;
}

bool csOpcodeCollisionSector::CollisionTest (CS::Collision2::iCollisionObject* object, csArray<CS::Collision2::CollisionData>& collisions)
{

  return false;
}

void csOpcodeCollisionSector::AddCollisionActor (CS::Collision2::iCollisionActor* actor)
{

}

void csOpcodeCollisionSector::RemoveCollisionActor ()
{

}

CS::Collision2::iCollisionActor* csOpcodeCollisionSector::GetCollisionActor ()
{
  return NULL;
}

SCF_IMPLEMENT_FACTORY (csOpcodeCollisionSystem)

iObjectRegistry* csOpcodeCollisionSystem::rep_object_reg = NULL;

csOpcodeCollisionSystem::csOpcodeCollisionSystem (iBase* iParent)
: scfImplementationType (this, iParent)
{
}

csOpcodeCollisionSystem::~csOpcodeCollisionSystem ()
{
  rep_object_reg = 0;
  collSectors.DeleteAll ();
}

bool csOpcodeCollisionSystem::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  rep_object_reg = object_reg;
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  baseID = strings->Request ("base");
  colldetID = strings->Request ("colldet");
  return true;
}

void csOpcodeCollisionSystem::SetInternalScale (float scale)
{
  //use internal scale?
}

csRef<CS::Collision2::iColliderConvexMesh> csOpcodeCollisionSystem::CreateColliderConvexMesh (iMeshWrapper* mesh, bool simplify /* = false */)
{
  return NULL;
}

csRef<CS::Collision2::iColliderConcaveMesh> csOpcodeCollisionSystem::CreateColliderConcaveMesh (iMeshWrapper* mesh)
{
  return NULL;
}

csRef<CS::Collision2::iColliderConcaveMeshScaled> csOpcodeCollisionSystem::CreateColliderConcaveMeshScaled
(CS::Collision2::iColliderConcaveMesh* collider, csVector3 scale)
{
  return NULL;
}

csRef<CS::Collision2::iColliderCylinder> csOpcodeCollisionSystem::CreateColliderCylinder (float length, float radius)
{
  return NULL;
}

csRef<CS::Collision2::iColliderBox> csOpcodeCollisionSystem::CreateColliderBox (const csVector3& size)
{
  return NULL;
}

csRef<CS::Collision2::iColliderSphere> csOpcodeCollisionSystem::CreateColliderSphere (float radius)
{
  return NULL;
}

csRef<CS::Collision2::iColliderCapsule> csOpcodeCollisionSystem::CreateColliderCapsule (float length, float radius)
{
  return NULL;
}

csRef<CS::Collision2::iColliderCone> csOpcodeCollisionSystem::CreateColliderCone (float length, float radius)
{
  return NULL;
}

csRef<CS::Collision2::iColliderPlane> csOpcodeCollisionSystem::CreateColliderPlane (const csPlane3& plane)
{
  return NULL;
}

csRef<CS::Collision2::iColliderTerrain> csOpcodeCollisionSystem::CreateColliderTerrain (iTerrainSystem* terrain,
                                                               float minHeight, float maxHeight)
{
  return NULL;
}

csRef<CS::Collision2::iCollisionObject> csOpcodeCollisionSystem::CreateCollisionObject ()
{
  return NULL;
}

csRef<CS::Collision2::iCollisionActor> csOpcodeCollisionSystem::CreateCollisionActor ()
{
  return NULL;
}

csRef<CS::Collision2::iCollisionSector> csOpcodeCollisionSystem::CreateCollisionSector ()
{
  return NULL;
}

void csOpcodeCollisionSystem::DecomposeConcaveMesh (CS::Collision2::iCollisionObject* object,
                                   iMeshWrapper* mesh, bool simplify)
{

}

void csOpcodeCollisionSystem::OpcodeReportV (int severity, const char* message, 
                           va_list args)
{
  csReportV (rep_object_reg,
    severity, "crystalspace.collisiondetection.opcode", message, args);
}
}
CS_PLUGIN_NAMESPACE_END (Opcode2)