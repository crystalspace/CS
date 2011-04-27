/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

/*
-------------------------------------------------------------------------
*
*           OPCODE collision detection plugin for CrystalSpace
*
*           OPCODE library was written by Pierre Terdiman
*                  ported to CS by Charles Quarra
*
-------------------------------------------------------------------------
*/

#include "cssysdef.h"
#include "CSopcode.h"
#include "csqsqrt.h"
#include "csgeom/transfrm.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "ivaria/reporter.h"
#include "csutil/scfarray.h"



//----------------------------------------------------------------------

CS_PLUGIN_NAMESPACE_BEGIN(csOpcode)
{

SCF_IMPLEMENT_FACTORY (csOPCODECollideSystem)

using namespace Opcode;

void Opcode_Log (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csOPCODECollideSystem::OpcodeReportV (CS_REPORTER_SEVERITY_NOTIFY, 
    msg, args);
  va_end (args);
}

bool Opcode_Err (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  // Although it's called "..._Err", Opcode also reports less-than-fatal
  // messages through it
  csOPCODECollideSystem::OpcodeReportV (CS_REPORTER_SEVERITY_WARNING, 
    msg, args);
  va_end (args);
  return false;
}

iObjectRegistry* csOPCODECollideSystem::rep_object_reg = 0;

void csOPCODECollideSystem::OpcodeReportV (int severity, const char* message, 
                                           va_list args)
{
  csReportV (rep_object_reg,
    severity, "crystalspace.collisiondetection.opcode", message, args);
}

csOPCODECollideSystem::csOPCODECollideSystem (iBase *pParent) :
  scfImplementationType(this, pParent)
{
  TreeCollider.SetFirstContact (false);
  TreeCollider.SetFullBoxBoxTest (false);
  TreeCollider.SetFullPrimBoxTest (false);
  // TreeCollider.SetFullPrimPrimTest (true);
  TreeCollider.SetTemporalCoherence (true);

  RayCol.SetCulling (false);
  
  identity_matrix.Identity();
}

csOPCODECollideSystem::~csOPCODECollideSystem ()
{
  rep_object_reg = 0;
}

bool csOPCODECollideSystem::Initialize (iObjectRegistry* iobject_reg)
{
  object_reg = iobject_reg;
  rep_object_reg = object_reg;
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
      object_reg, "crystalspace.shared.stringset");
  trianglemesh_id = strings->Request ("colldet");
  basemesh_id = strings->Request ("base");
  return true;
}

csPtr<iCollider> csOPCODECollideSystem::CreateCollider (iTriangleMesh* mesh)
{
  csOPCODECollider* col = new csOPCODECollider (mesh);
  // here we must store the caches (and the trees)?
  return csPtr<iCollider> (col);
}

csPtr<iCollider> csOPCODECollideSystem::CreateCollider (iTerraFormer* terraformer)
{
  csTerraFormerCollider* col = new csTerraFormerCollider (terraformer, object_reg);
  return csPtr<iCollider> (col);
}

csPtr<iCollider> csOPCODECollideSystem::CreateCollider (iTerrainSystem* mesh)
{
  return csPtr<iCollider> (scfQueryInterface<iCollider> (mesh));
}

bool csOPCODECollideSystem::TestTriangleTerraFormer (csVector3 triangle[3], 
                                                   csTerraFormerCollider* c,
                                                   csCollisionPair* pair)
{
  float height[3];
  height[0] = c->SampleFloat (triangle[0].x, triangle[0].z);
  height[1] = c->SampleFloat (triangle[1].x, triangle[1].z);
  height[2] = c->SampleFloat (triangle[2].x, triangle[2].z);
  for (int i = 0; i < 3; i++)
  {
    if (height[i] > triangle[i].y)
    {
      pair->a1 = triangle[2];
      pair->b1 = triangle[1];
      pair->c1 = triangle[0];

      pair->a2 = csVector3 (triangle[2].x, height[2], triangle[2].z);
      pair->b2 = csVector3 (triangle[1].x, height[1], triangle[1].z);
      pair->c2 = csVector3 (triangle[0].x, height[0], triangle[0].z);

      return true;
    }
  }
 return false;
}
void csOPCODECollideSystem::CopyCollisionPairs (csOPCODECollider* col1,
                                                csTerraFormerCollider* terraformer)
{
  int size = (int) (udword(TreeCollider.GetNbPairs ()));
  if (size == 0) return;
  int N_pairs = size;
  const Pair* colPairs=TreeCollider.GetPairs ();
  Point* vertholder0 = col1->vertholder;
  if (!vertholder0) return;
  Point* vertholder1 = terraformer->vertices.GetArray ();
  if (!vertholder1) return;
  udword* indexholder0 = col1->indexholder;
  if (!indexholder0) return;
    udword* indexholder1 = terraformer->indexholder;
  if (!indexholder1) return;
  Point* current;
  int i, j;

  size_t oldlen = pairs.GetSize ();
  pairs.SetSize (oldlen + N_pairs);

  for (i = 0 ; i < N_pairs ; i++)
  {
    j = 3 * colPairs[i].id0;
    current = &vertholder0[indexholder0[j]];		
    pairs[oldlen].a1 = csVector3 (current->x, current->y, current->z);
    current = &vertholder0[indexholder0[j + 1]];		
    pairs[oldlen].b1 = csVector3 (current->x, current->y, current->z);
    current = &vertholder0[indexholder0[j + 2]];		
    pairs[oldlen].c1 = csVector3 (current->x, current->y, current->z);

    j = 3 * colPairs[i].id1;
    current = &vertholder1[indexholder1[j]];		
    pairs[oldlen].a2 = csVector3 (current->x, current->y, current->z);
    current = &vertholder1[indexholder1[j + 1 ]];		
    pairs[oldlen].b2 = csVector3 (current->x, current->y, current->z);
    current = &vertholder1[indexholder1[j + 2 ]];		
    pairs[oldlen].c2 = csVector3 (current->x, current->y, current->z);


    oldlen++;
  }
}
bool csOPCODECollideSystem::Collide (
  csOPCODECollider* collider1, const csReversibleTransform* trans1,
    iTerrainSystem* terrain, const csReversibleTransform* terrainTrans)
{
  unsigned int tri_count = collider1->opcMeshInt.GetNbTriangles ();
  const unsigned int* tris = collider1->indexholder;
  const Point* verts = collider1->vertholder;
  
  scfArray<iTerrainCollisionPairArray> c_pairs;
  
  // Compute the _relative_ transform
  csReversibleTransform t;
  if (trans1)
    t = *trans1;

  if (terrainTrans)
    t /= *terrainTrans;

  if (terrain->CollideTriangles ((const csVector3*)verts, tri_count,
    tris, collider1->radius, t, false, &c_pairs))
// Change to use OPCODE-powered collision
//  if (terrain->Collide (collider1, collider1->radius, trans1, false,
//    c_pairs))
  {
    for (size_t i = 0; i < c_pairs.GetSize (); ++i)
      pairs.Push (c_pairs.Get (i));
      
    return true;
  }
  else return false;
}

bool csOPCODECollideSystem::Collide (
  csOPCODECollider* col1, const csReversibleTransform* trans1,
  csTerraFormerCollider* terraformer, const csReversibleTransform* trans2)
{
  ColCache.Model0 = col1->m_pCollisionModel;
  terraformer->UpdateOPCODEModel (trans1->GetOrigin (), col1->GetRadius ());
  ColCache.Model1 = terraformer->opcode_model;

  csMatrix3 m1;
  if (trans1) m1 = trans1->GetT2O ();
  csVector3 u;

  IceMaths::Matrix4x4 transform1;
  transform1.m[0][3] = 0;
  transform1.m[1][3] = 0;
  transform1.m[2][3] = 0;
  transform1.m[3][3] = 1;
  u = m1.Row1 ();
  transform1.m[0][0] = u.x;
  transform1.m[1][0] = u.y;
  transform1.m[2][0] = u.z;
  u = m1.Row2 ();
  transform1.m[0][1] = u.x;
  transform1.m[1][1] = u.y;
  transform1.m[2][1] = u.z;
  u = m1.Row3 ();
  transform1.m[0][2] = u.x;
  transform1.m[1][2] = u.y;
  transform1.m[2][2] = u.z;

  if (trans1) u = trans1->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform1.m[3][0] = u.x;
  transform1.m[3][1] = u.y;
  transform1.m[3][2] = u.z;


  if (trans1) m1 = trans2->GetT2O ();

  u = m1.Row1 ();
  terraformer->transform.m[0][0] = u.x;
  terraformer->transform.m[1][0] = u.y;
  terraformer->transform.m[2][0] = u.z;
  u = m1.Row2 ();
  terraformer->transform.m[0][1] = u.x;
  terraformer->transform.m[1][1] = u.y;
  terraformer->transform.m[2][1] = u.z;
  u = m1.Row3 ();
  terraformer->transform.m[0][2] = u.x;
  terraformer->transform.m[1][2] = u.y;
  terraformer->transform.m[2][2] = u.z;

  if (trans1) u = trans2->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  terraformer->transform.m[3][0] = u.x;
  terraformer->transform.m[3][1] = u.y;
  terraformer->transform.m[3][2] = u.z;

  bool isOk = TreeCollider.Collide (ColCache, &transform1,
  	&terraformer->transform);
  if (isOk)
  {
    bool status = (TreeCollider.GetContactStatus () != FALSE);
    if (status)
    {
      CopyCollisionPairs (col1, terraformer);
    }
    return status;
  }
  else
  {
    return false;
  }
}

bool csOPCODECollideSystem::Collide (
  iCollider* collider1, const csReversibleTransform* trans1,
  iCollider* collider2, const csReversibleTransform* trans2)
{
  // csPrintf( " we are in Collide \n");
  if (collider1->GetColliderType () == CS_TERRAFORMER_COLLIDER && 
      collider2->GetColliderType () == CS_MESH_COLLIDER)
    return Collide ((csOPCODECollider*)collider2, trans2,
	(csTerraFormerCollider*)collider1, trans1);
    
  if (collider2->GetColliderType () == CS_TERRAFORMER_COLLIDER && 
      collider1->GetColliderType () == CS_MESH_COLLIDER)
    return Collide ((csOPCODECollider*)collider1, trans1,
	(csTerraFormerCollider*)collider2, trans2);

  if (collider1->GetColliderType () == CS_TERRAIN_COLLIDER && 
      collider2->GetColliderType () == CS_MESH_COLLIDER)
  {
    csRef<iTerrainSystem> terrain = scfQueryInterface<iTerrainSystem> (
	  collider1);
    return Collide ((csOPCODECollider*)collider2, trans2, terrain, trans1);
  }
    
  if (collider2->GetColliderType () == CS_TERRAIN_COLLIDER && 
      collider1->GetColliderType () == CS_MESH_COLLIDER)
  {
    csRef<iTerrainSystem> terrain = scfQueryInterface<iTerrainSystem> (
	  collider2);
    return Collide ((csOPCODECollider*)collider1, trans1, terrain, trans2);
  }

  csOPCODECollider* col1 = (csOPCODECollider*) collider1;
  csOPCODECollider* col2 = (csOPCODECollider*) collider2;
  //if (col1 == col2) return false;

  ColCache.Model0 = col1->m_pCollisionModel;
  ColCache.Model1 = col2->m_pCollisionModel;

  csMatrix3 m1;
  if (trans1) m1 = trans1->GetT2O ();
  csMatrix3 m2;
  if (trans2) m2 = trans2->GetT2O ();
  csVector3 u;

  IceMaths::Matrix4x4 transform1;
  transform1.m[0][3] = 0;
  transform1.m[1][3] = 0;
  transform1.m[2][3] = 0;
  transform1.m[3][3] = 1;
  IceMaths::Matrix4x4 transform2;
  transform2.m[0][3] = 0;
  transform2.m[1][3] = 0;
  transform2.m[2][3] = 0;
  transform2.m[3][3] = 1;

  u = m1.Row1 ();
  transform1.m[0][0] = u.x;
  transform1.m[1][0] = u.y;
  transform1.m[2][0] = u.z;
  u = m2.Row1 ();
  transform2.m[0][0] = u.x;
  transform2.m[1][0] = u.y;
  transform2.m[2][0] = u.z;
  u = m1.Row2 ();
  transform1.m[0][1] = u.x;
  transform1.m[1][1] = u.y;
  transform1.m[2][1] = u.z;
  u = m2.Row2 ();
  transform2.m[0][1] = u.x;
  transform2.m[1][1] = u.y;
  transform2.m[2][1] = u.z;
  u = m1.Row3 ();
  transform1.m[0][2] = u.x;
  transform1.m[1][2] = u.y;
  transform1.m[2][2] = u.z;
  u = m2.Row3();
  transform2.m[0][2] = u.x;
  transform2.m[1][2] = u.y;
  transform2.m[2][2] = u.z;

  if (trans1) u = trans1->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform1.m[3][0] = u.x;
  transform1.m[3][1] = u.y;
  transform1.m[3][2] = u.z;

  if (trans2) u = trans2->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform2.m[3][0] = u.x;
  transform2.m[3][1] = u.y;
  transform2.m[3][2] = u.z;

  bool isOk = TreeCollider.Collide (ColCache, &transform1,
  	&transform2);
  if (isOk)
  {
    bool status = (TreeCollider.GetContactStatus () != FALSE);
    if (status)
    {
      CopyCollisionPairs (col1, col2);
    }
    return status;
  }
  else
  {
    return false;
  }
}

static float max_dist;

static void ray_cb (const CollisionFace& hit, void* user_data)
{
  csArray<int>* collision_faces = (csArray<int>*)user_data;
  if (hit.mDistance <= max_dist)
    collision_faces->Push (hit.mFaceID);
}

bool csOPCODECollideSystem::CollideRaySegment (
  	csOPCODECollider* col, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray)
{
  ColCache.Model0 = col->m_pCollisionModel;

  csMatrix3 m;
  if (trans) m = trans->GetT2O ();
  csVector3 u;

  IceMaths::Matrix4x4 transform;
  transform.m[0][3] = 0;
  transform.m[1][3] = 0;
  transform.m[2][3] = 0;
  transform.m[3][3] = 1;
  u = m.Row1 ();
  transform.m[0][0] = u.x;
  transform.m[1][0] = u.y;
  transform.m[2][0] = u.z;
  u = m.Row2 ();
  transform.m[0][1] = u.x;
  transform.m[1][1] = u.y;
  transform.m[2][1] = u.z;
  u = m.Row3 ();
  transform.m[0][2] = u.x;
  transform.m[1][2] = u.y;
  transform.m[2][2] = u.z;

  if (trans) u = trans->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform.m[3][0] = u.x;
  transform.m[3][1] = u.y;
  transform.m[3][2] = u.z;

  Ray ray (Point (start.x, start.y, start.z),
  	   Point (end.x-start.x, end.y-start.y, end.z-start.z));

  RayCol.SetHitCallback (ray_cb);
  RayCol.SetUserData ((void*)&collision_faces);
  intersecting_triangles.SetSize (0);
  collision_faces.SetSize (0);
  if (use_ray)
  {
    max_dist = MAX_FLOAT;
    RayCol.SetMaxDist ();
  }
  else
  {
    max_dist = csQsqrt (csSquaredDist::PointPoint (start, end));
    RayCol.SetMaxDist (max_dist);
  }
  bool isOk = RayCol.Collide (ray, *ColCache.Model0, &transform);
  if (isOk)
  {
    bool status = (RayCol.GetContactStatus () != FALSE);
    if (status)
    {
      // Now calculate the real intersection points for all hit faces.
      Point* vertholder = col->vertholder;
      if (!vertholder) return true;
      udword* indexholder = col->indexholder;
      if (!indexholder) return true;
      if (collision_faces.GetSize () == 0) return false;
      Point* c;
      size_t i;
      for (i = 0 ; i < collision_faces.GetSize () ; i++)
      {
        int idx = collision_faces[i] * 3;
	int it_idx = (int)intersecting_triangles.Push (csIntersectingTriangle ());
	c = &vertholder[indexholder[idx+0]];
	intersecting_triangles[it_idx].a.Set (c->x, c->y, c->z);
	c = &vertholder[indexholder[idx+1]];
	intersecting_triangles[it_idx].b.Set (c->x, c->y, c->z);
	c = &vertholder[indexholder[idx+2]];
	intersecting_triangles[it_idx].c.Set (c->x, c->y, c->z);
      }
    }
    return status;
  }
  else
  {
    return false;
  }
}

bool csOPCODECollideSystem::CollideRaySegment (
  	iTerrainSystem* terrain, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray)
{
  csVector3 s, e;
  if (trans)
  {
    s = trans->Other2This (start);
    e = trans->Other2This (end);
  }
  else
  {
    s = start;
    e = end;
  }
  csTerrainColliderCollideSegmentResult rc = terrain->CollideSegment (s, e);
  if (!rc.hit) return false;
  
  intersecting_triangles.SetSize (0);
  size_t idx = intersecting_triangles.Push (csIntersectingTriangle ());
  intersecting_triangles[idx].a = rc.a;
  intersecting_triangles[idx].b = rc.b;
  intersecting_triangles[idx].c = rc.c;

  return true;
}

bool csOPCODECollideSystem::CalculateIntersections (
    csTerraFormerCollider* terraformer)
{
  // Now calculate the real intersection points for all hit faces.
  Point* vertholder = terraformer->vertices.GetArray ();
  if (!vertholder) return true;
  udword* indexholder = terraformer->indexholder;
  if (!indexholder) return true;
  if (collision_faces.GetSize () == 0) return false;
  Point* c;
  size_t i;
  for (i = 0 ; i < collision_faces.GetSize () ; i++)
  {
    int idx = collision_faces[i] * 3;
    int it_idx = (int)intersecting_triangles.Push (
	    csIntersectingTriangle ());
    c = &vertholder[indexholder[idx+0]];
    intersecting_triangles[it_idx].a.Set (c->x, c->y, c->z);
    c = &vertholder[indexholder[idx+1]];
    intersecting_triangles[it_idx].b.Set (c->x, c->y, c->z);
    c = &vertholder[indexholder[idx+2]];
    intersecting_triangles[it_idx].c.Set (c->x, c->y, c->z);
  }
  return true;
}

bool csOPCODECollideSystem::TestSegmentTerraFormer (
    csTerraFormerCollider* terraformer,
    const IceMaths::Matrix4x4& transform,
    const csVector3& start, const csVector3& end)
{
  terraformer->UpdateOPCODEModel (start, end);
  ColCache.Model0 = terraformer->opcode_model;
  Ray ray (Point (start.x, start.y, start.z),
  	   Point (end.x-start.x, end.y-start.y, end.z-start.z));

  bool status = RayCol.Collide (ray, *ColCache.Model0, &transform);
  if (status)
  {
    status = (RayCol.GetContactStatus () != FALSE);
    if (status)
      return CalculateIntersections (terraformer);
  }
  return false;
}

#define MAX_BEAM_LENGTH 10
bool csOPCODECollideSystem::CollideRaySegment (
  	csTerraFormerCollider* terraformer, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray)
{
  csMatrix3 m;
  if (trans) m = trans->GetT2O ();
  csVector3 u;

  IceMaths::Matrix4x4 transform;
  transform.m[0][3] = 0;
  transform.m[1][3] = 0;
  transform.m[2][3] = 0;
  transform.m[3][3] = 1;
  u = m.Row1 ();
  transform.m[0][0] = u.x;
  transform.m[1][0] = u.y;
  transform.m[2][0] = u.z;
  u = m.Row2 ();
  transform.m[0][1] = u.x;
  transform.m[1][1] = u.y;
  transform.m[2][1] = u.z;
  u = m.Row3 ();
  transform.m[0][2] = u.x;
  transform.m[1][2] = u.y;
  transform.m[2][2] = u.z;

  if (trans) u = trans->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform.m[3][0] = u.x;
  transform.m[3][1] = u.y;
  transform.m[3][2] = u.z;

  RayCol.SetHitCallback (ray_cb);
  RayCol.SetUserData ((void*)&collision_faces);
  intersecting_triangles.SetSize (0);
  collision_faces.SetSize (0);

  float totalLength = sqrtf (csSquaredDist::PointPoint (start, end));
  if (use_ray)
  {
    max_dist = MAX_FLOAT;
    RayCol.SetMaxDist ();
  }
  else
  {
    max_dist = totalLength;
    RayCol.SetMaxDist (max_dist);
  }

  if (totalLength > MAX_BEAM_LENGTH)
  {
    csVector3 norm = end-start;
    norm.Normalize ();
    csVector3 s = start;
    float totlen = 0;
    while (totlen < totalLength)
    {
      float len = MAX_BEAM_LENGTH;
      totlen += MAX_BEAM_LENGTH;
      if (totlen > totalLength)
      {
	len -= totlen - totalLength;
      }
      csVector3 e = s + len*norm;
      if (TestSegmentTerraFormer (terraformer, transform, s, e))
	return true;
      s = e;
    }
  }
  else
  {
    return TestSegmentTerraFormer (terraformer, transform, start, end);
  }
  return false;
}

bool csOPCODECollideSystem::CollideRaySegment (
  	iCollider* collider, const csReversibleTransform* trans,
	const csVector3& start, const csVector3& end, bool use_ray)
{
  if (!collider) return false;
  if (collider->GetColliderType () == CS_MESH_COLLIDER)
  {
    return CollideRaySegment ((csOPCODECollider*) collider,
	trans, start, end, use_ray);
  }
  else if (collider->GetColliderType () == CS_TERRAIN_COLLIDER)
  {
    csRef<iTerrainSystem> terrain = scfQueryInterface<iTerrainSystem> (
	  collider);
    return CollideRaySegment (terrain, trans, start, end, use_ray);
  }
  else if (collider->GetColliderType () == CS_TERRAFORMER_COLLIDER)
  {
    return CollideRaySegment ((csTerraFormerCollider*) collider,
	trans, start, end, use_ray);
  }
  return false;
}

bool csOPCODECollideSystem::CollideLSS (
  	csOPCODECollider* col, const csReversibleTransform* trans1,
  	const csVector3& start, const csVector3& end, float radius,
  	const csReversibleTransform* trans2)
{
  csMatrix3 m1;
  if (trans1) m1 = trans1->GetT2O ();
  csMatrix3 m2;
  if (trans2) m2 = trans2->GetT2O ();
  csVector3 u;

  IceMaths::Matrix4x4 transform1;
  transform1.m[0][3] = 0;
  transform1.m[1][3] = 0;
  transform1.m[2][3] = 0;
  transform1.m[3][3] = 1;
  IceMaths::Matrix4x4 transform2;
  transform2.m[0][3] = 0;
  transform2.m[1][3] = 0;
  transform2.m[2][3] = 0;
  transform2.m[3][3] = 1;

  u = m1.Row1 ();
  transform1.m[0][0] = u.x;
  transform1.m[1][0] = u.y;
  transform1.m[2][0] = u.z;
  u = m2.Row1 ();
  transform2.m[0][0] = u.x;
  transform2.m[1][0] = u.y;
  transform2.m[2][0] = u.z;
  u = m1.Row2 ();
  transform1.m[0][1] = u.x;
  transform1.m[1][1] = u.y;
  transform1.m[2][1] = u.z;
  u = m2.Row2 ();
  transform2.m[0][1] = u.x;
  transform2.m[1][1] = u.y;
  transform2.m[2][1] = u.z;
  u = m1.Row3 ();
  transform1.m[0][2] = u.x;
  transform1.m[1][2] = u.y;
  transform1.m[2][2] = u.z;
  u = m2.Row3();
  transform2.m[0][2] = u.x;
  transform2.m[1][2] = u.y;
  transform2.m[2][2] = u.z;

  if (trans1) u = trans1->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform1.m[3][0] = u.x;
  transform1.m[3][1] = u.y;
  transform1.m[3][2] = u.z;

  if (trans2) u = trans2->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform2.m[3][0] = u.x;
  transform2.m[3][1] = u.y;
  transform2.m[3][2] = u.z;
  
  LSS capsule(Segment(Point (start.x, start.y, start.z),
  	   Point (end.x, end.y, end.z)), radius);

  intersecting_triangles.SetSize (0);
  collision_faces.SetSize (0);
  bool isOk = LSSCol.Collide (LSSCache, capsule, *col->m_pCollisionModel, &transform2, &transform1);
  if (isOk)
  {
    bool status = (LSSCol.GetContactStatus () != FALSE);
    if (status)
    {
      udword ntouched_primitives = LSSCol.GetNbTouchedPrimitives();
      const	udword* touched_primitives = LSSCol.GetTouchedPrimitives();
      
      // Now calculate the real intersection points for all hit faces.
      Point* vertholder = col->vertholder;
      if (!vertholder) return true;
      udword* indexholder = col->indexholder;
      if (!indexholder) return true;
      if (ntouched_primitives == 0) return false;
      Point* c;
      
      udword i;
      for (i = 0 ; i < ntouched_primitives ; i++)
      {
        int idx = touched_primitives[i] * 3;
        int it_idx = (int)intersecting_triangles.Push (csIntersectingTriangle ());
        c = &vertholder[indexholder[idx+0]];
        intersecting_triangles[it_idx].a.Set (c->x, c->y, c->z);
        c = &vertholder[indexholder[idx+1]];
        intersecting_triangles[it_idx].b.Set (c->x, c->y, c->z);
        c = &vertholder[indexholder[idx+2]];
        intersecting_triangles[it_idx].c.Set (c->x, c->y, c->z);
      }
    }
    return status;
  }
  else
  {
    return false;
  }
}

bool csOPCODECollideSystem::CollideLSS (
	iTerrainSystem* terrain, const csReversibleTransform* trans1,
	const csVector3& start, const csVector3& end, float radius,
	const csReversibleTransform* trans2)
{
	// TODO: To be implemented!
	return false;
}
bool csOPCODECollideSystem::CollideLSS (
	csTerraFormerCollider* terraformer, const csReversibleTransform* trans1,
	const csVector3& start, const csVector3& end, float radius,
	const csReversibleTransform* trans2)
{
	// TODO: To be implemented!
	return false;
}


bool csOPCODECollideSystem::CollideLSS (
  	iCollider* collider, const csReversibleTransform* trans1,
	const csVector3& start, const csVector3& end, float radius,
  	const csReversibleTransform* trans2)
{
  if (!collider) return false;
  if (collider->GetColliderType () == CS_MESH_COLLIDER)
  {
    return CollideLSS ((csOPCODECollider*) collider,
	trans1, start, end, radius, trans2);
  }
  else if (collider->GetColliderType () == CS_TERRAIN_COLLIDER)
  {
    csRef<iTerrainSystem> terrain = scfQueryInterface<iTerrainSystem> (
	  collider);
    return CollideLSS (terrain, trans1, start, end, radius, trans2);
  }
  else if (collider->GetColliderType () == CS_TERRAFORMER_COLLIDER)
  {
    return CollideLSS ((csTerraFormerCollider*) collider,
	trans1, start, end, radius, trans2);
  }
  return false;
}

bool csOPCODECollideSystem::CollideOBB (
  	csOPCODECollider* col, const csReversibleTransform* trans1,
  	const csVector3& min, const csVector3& max,
  	const csReversibleTransform* trans2)
{
  csMatrix3 m1;
  if (trans1) m1 = trans1->GetT2O ();
  csMatrix3 m2;
  if (trans2) m2 = trans2->GetT2O ();
  csVector3 u;

  IceMaths::Matrix4x4 transform1;
  transform1.m[0][3] = 0;
  transform1.m[1][3] = 0;
  transform1.m[2][3] = 0;
  transform1.m[3][3] = 1;
  IceMaths::Matrix4x4 transform2;
  transform2.m[0][3] = 0;
  transform2.m[1][3] = 0;
  transform2.m[2][3] = 0;
  transform2.m[3][3] = 1;

  u = m1.Row1 ();
  transform1.m[0][0] = u.x;
  transform1.m[1][0] = u.y;
  transform1.m[2][0] = u.z;
  u = m2.Row1 ();
  transform2.m[0][0] = u.x;
  transform2.m[1][0] = u.y;
  transform2.m[2][0] = u.z;
  u = m1.Row2 ();
  transform1.m[0][1] = u.x;
  transform1.m[1][1] = u.y;
  transform1.m[2][1] = u.z;
  u = m2.Row2 ();
  transform2.m[0][1] = u.x;
  transform2.m[1][1] = u.y;
  transform2.m[2][1] = u.z;
  u = m1.Row3 ();
  transform1.m[0][2] = u.x;
  transform1.m[1][2] = u.y;
  transform1.m[2][2] = u.z;
  u = m2.Row3();
  transform2.m[0][2] = u.x;
  transform2.m[1][2] = u.y;
  transform2.m[2][2] = u.z;

  if (trans1) u = trans1->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform1.m[3][0] = u.x;
  transform1.m[3][1] = u.y;
  transform1.m[3][2] = u.z;

  if (trans2) u = trans2->GetO2TTranslation ();
  else u.Set (0, 0, 0);
  transform2.m[3][0] = u.x;
  transform2.m[3][1] = u.y;
  transform2.m[3][2] = u.z;
  
  AABB box;
  box.SetMinMax(Point (min.x, min.y, min.z),
		  Point (max.x, max.y, max.z));
  OBB obb_box;
  obb_box.Create(box, identity_matrix);

  intersecting_triangles.SetSize (0);
  collision_faces.SetSize (0);
  bool isOk = OBBCol.Collide (OBBCache, obb_box, *col->m_pCollisionModel, &transform2, &transform1);
  if (isOk)
  {
    bool status = (OBBCol.GetContactStatus () != FALSE);
    if (status)
    {
      udword ntouched_primitives = OBBCol.GetNbTouchedPrimitives();
      const	udword* touched_primitives = OBBCol.GetTouchedPrimitives();
      
      // Now calculate the real intersection points for all hit faces.
      Point* vertholder = col->vertholder;
      if (!vertholder) return true;
      udword* indexholder = col->indexholder;
      if (!indexholder) return true;
      if (ntouched_primitives == 0) return false;
      Point* c;
      
      udword i;
      for (i = 0 ; i < ntouched_primitives ; i++)
      {
        int idx = touched_primitives[i] * 3;
        int it_idx = (int)intersecting_triangles.Push (csIntersectingTriangle ());
        c = &vertholder[indexholder[idx+0]];
        intersecting_triangles[it_idx].a.Set (c->x, c->y, c->z);
        c = &vertholder[indexholder[idx+1]];
        intersecting_triangles[it_idx].b.Set (c->x, c->y, c->z);
        c = &vertholder[indexholder[idx+2]];
        intersecting_triangles[it_idx].c.Set (c->x, c->y, c->z);
      }
    }
    return status;
  }
  else
  {
    return false;
  }
}

bool csOPCODECollideSystem::CollideOBB (
	iTerrainSystem* terrain, const csReversibleTransform* trans1,
	const csVector3& min, const csVector3& max,
	const csReversibleTransform* trans2)
{
	// TODO: To be implemented!
	return false;
}
bool csOPCODECollideSystem::CollideOBB (
	csTerraFormerCollider* terraformer, const csReversibleTransform* trans1,
	const csVector3& min, const csVector3& max,
	const csReversibleTransform* trans2)
{
	// TODO: To be implemented!
	return false;
}

bool csOPCODECollideSystem::CollideOBB (
  	iCollider* collider, const csReversibleTransform* trans1,
  	const csVector3& min, const csVector3& max,
  	const csReversibleTransform* trans2)
{
  if (!collider) return false;
  if (collider->GetColliderType () == CS_MESH_COLLIDER)
  {
    return CollideOBB ((csOPCODECollider*) collider,
	trans1, min, max, trans2);
  }
  else if (collider->GetColliderType () == CS_TERRAIN_COLLIDER)
  {
    csRef<iTerrainSystem> terrain = scfQueryInterface<iTerrainSystem> (
	  collider);
    return CollideOBB (terrain, trans1, min, max, trans2);
  }
  else if (collider->GetColliderType () == CS_TERRAFORMER_COLLIDER)
  {
    return CollideOBB ((csTerraFormerCollider*) collider,
	trans1, min, max, trans2);
  }
  return false;
}

void csOPCODECollideSystem::CopyCollisionPairs (csOPCODECollider* col1,
	csOPCODECollider* col2)
{
  int size = (int) (udword(TreeCollider.GetNbPairs ()));
  if (size == 0) return;
  int N_pairs = size;
  const Pair* colPairs=TreeCollider.GetPairs ();
  Point* vertholder0 = col1->vertholder;
  if (!vertholder0) return;
  Point* vertholder1 = col2->vertholder;
  if (!vertholder1) return;
  udword* indexholder0 = col1->indexholder;
  if (!indexholder0) return;
  udword* indexholder1 = col2->indexholder;
  if (!indexholder1) return;
  Point* current;
  int i, j;

  size_t oldlen = pairs.GetSize ();
  pairs.SetSize (oldlen + N_pairs);

  // it really sucks having to copy all this each Collide query, but
  // since opcode library uses his own vector types,
  // but there are some things to consider:
  // first, since opcode doesnt store the mesh, it relies on a callback
  // to query for precise positions of vertices when doing close-contact
  // checks, hence are two options:
  // 1. Do the triangulation only at the creation of the collider, but
  //    keeping copies of the meshes inside the colliders, which are then used
  //    for constructing the csCollisionPair and the opcode callbacks, but
  //    duplicates mesh data in memory (which is how is it currently)
  // 2. Do the triangulation again when a collision returns inside this
  //    precise routine (csOPCODECollider::Collide), which wouldn't be that bad,
  //    but doing it at the Callback would be plain wrong, since that Callback
  //    should be as fast as possible
  //    so, the question is, what we should prefer? memory or speed?
  //	   csPrintf( " 1) \n");

  for (i = 0 ; i < N_pairs ; i++)
  {
    j = 3 * colPairs[i].id0;
    current = &vertholder0[indexholder0[j]];		
    pairs[oldlen].a1 = csVector3 (current->x, current->y, current->z);
    current = &vertholder0[indexholder0[j + 1]];		
    pairs[oldlen].b1 = csVector3 (current->x, current->y, current->z);
    current = &vertholder0[indexholder0[j + 2]];		
    pairs[oldlen].c1 = csVector3 (current->x, current->y, current->z);

    j = 3 * colPairs[i].id1;
    current = &vertholder1[indexholder1[j]];		
    pairs[oldlen].a2 = csVector3 (current->x, current->y, current->z);
    current = &vertholder1[indexholder1[j + 1 ]];		
    pairs[oldlen].b2 = csVector3 (current->x, current->y, current->z);
    current = &vertholder1[indexholder1[j + 2 ]];		
    pairs[oldlen].c2 = csVector3 (current->x, current->y, current->z);

    oldlen++;
  }
}

csCollisionPair* csOPCODECollideSystem::GetCollisionPairs ()
{
  return pairs.GetArray ();
}

size_t csOPCODECollideSystem::GetCollisionPairCount ()
{
  return pairs.GetSize ();
}

void csOPCODECollideSystem::ResetCollisionPairs ()
{
  pairs.Empty ();
}

void csOPCODECollideSystem::SetOneHitOnly (bool on)
{
  TreeCollider.SetFirstContact (on);
}

/**
* Return true if this CD system will only return the first hit
* that is found. For CD systems that support multiple hits this
* will return the value set by the SetOneHitOnly() function.
* For CD systems that support one hit only this will always return true.
*/
bool csOPCODECollideSystem::GetOneHitOnly ()
{
  return (TreeCollider.FirstContactEnabled () != FALSE);
}

}
CS_PLUGIN_NAMESPACE_END(csOpcode)
