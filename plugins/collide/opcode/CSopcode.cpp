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
#include "igeom/polymesh.h"
#include "csgeom/transfrm.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"

CS_IMPLEMENT_PLUGIN

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csOPCODECollideSystem)
  SCF_IMPLEMENTS_INTERFACE (iCollideSystem)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csOPCODECollideSystem::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csOPCODECollideSystem)

SCF_EXPORT_CLASS_TABLE (opcode)
  SCF_EXPORT_CLASS (csOPCODECollideSystem,
    "crystalspace.collisiondetection.opcode",
    "Crystal Space OPCODE CD System")
SCF_EXPORT_CLASS_TABLE_END

using namespace Opcode;

static void _opcodeCallback (udword triangleIndex, VertexPointers& triangle, udword userdata)
{
  udword *tri_array = ((csOPCODECollider *) userdata)->indexholder;
  Point *vertholder = ((csOPCODECollider *) userdata)->vertholder;
  int index = 3 * triangleIndex;
  triangle.Vertex[0] = &vertholder [tri_array[index]] ;
  triangle.Vertex[1] = &vertholder [tri_array[index + 1]];
  triangle.Vertex[2] = &vertholder [tri_array[index + 2]];
}

csOPCODECollideSystem::csOPCODECollideSystem (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  TreeCollider.SetFirstContact (false);
  TreeCollider.SetFullBoxBoxTest (false);
  TreeCollider.SetFullPrimBoxTest (false);
  // TreeCollider.SetFullPrimPrimTest (true);
  TreeCollider.SetTemporalCoherence (true);
  N_pairs = 0;
  pairs = NULL;
}

csOPCODECollideSystem::~csOPCODECollideSystem ()
{
  delete[] pairs;
}

bool csOPCODECollideSystem::Initialize (iObjectRegistry* iobject_reg)
{
  object_reg = iobject_reg;
  return true; 
}

csPtr<iCollider> csOPCODECollideSystem::CreateCollider (iPolygonMesh* mesh)
{
  csOPCODECollider* col = new csOPCODECollider (mesh);
  // here we must store the caches ¿and the trees?
  return csPtr<iCollider> (col);
}

csPtr<iCollider> csOPCODECollideSystem::CreateSphereCollider (iMeshObject*) 
{ return NULL; };  //unimplemented yet

csPtr<iCollider> csOPCODECollideSystem::CreateBoxCollider (iMeshObject*) 
{ return NULL; };  //unimplemented yet

bool csOPCODECollideSystem::Collide (
  iCollider* collider1, const csReversibleTransform* trans1,
  iCollider* collider2, const csReversibleTransform* trans2)
{
  // printf( " we are in Collide \n");
  col1 = (csOPCODECollider*) collider1;
  col2 = (csOPCODECollider*) collider2;
 
  TreeCollider.SetCallback0 (_opcodeCallback , udword(col1));
  TreeCollider.SetCallback1 (_opcodeCallback , udword(col2));

  ColCache.Model0 = col1->m_pCollisionModel;
  ColCache.Model1 = col2->m_pCollisionModel;
  
  csReversibleTransform NetTransform = *trans1 / *trans2;
  
  csMatrix3 m1 = NetTransform.GetO2T ();
  // csMatrix3 m2 = trans2->GetO2T ();
  csVector3 u;
 
  u = m1.Row1 ();
  col1->transform.m[0][0] = u.x;
  col1->transform.m[1][0] = u.y;
  col1->transform.m[2][0] = u.z;
  // u = m2.Row1 ();
  // col2->transform.m[0][0] = u.x;
  // col2->transform.m[1][0] = u.y;
  // col2->transform.m[2][0] = u.z;
  u = m1.Row2 ();
  col1->transform.m[0][1] = u.x;
  col1->transform.m[1][1] = u.y;
  col1->transform.m[2][1] = u.z;
  // u = m2.Row2 ();
  // col2->transform.m[0][1] = u.x;
  // col2->transform.m[1][1] = u.y;
  // col2->transform.m[2][1] = u.z;
  u = m1.Row3 ();
  col1->transform.m[0][2] = u.x;
  col1->transform.m[1][2] = u.y;
  col1->transform.m[2][2] = u.z;
  // u = m2.Row3();
  // col2->transform.m[0][2] = u.x;
  // col2->transform.m[1][2] = u.y;
  // col2->transform.m[2][2] = u.z;
  
  u = NetTransform.GetO2TTranslation ();
  col1->transform.m[3][0] = u.x;
  col1->transform.m[3][1] = u.y;
  col1->transform.m[3][2] = u.z;

  // u = trans2->GetO2TTranslation();
  // col2->transform.m[3][0] = u.x;
  // col2->transform.m[3][1] = u.y;
  // col2->transform.m[3][2] = u.z;

  bool isOk = TreeCollider.Collide (ColCache ,&col1->transform , NULL);
  if (isOk)
  {
    bool Status = TreeCollider.GetContactStatus ();
    if (Status) 
    {
      // UGLY: I know, it sucks to have to copy them by value, but then how can i be
      // sure that it would not be modified by the app until the GetCollisionPairs
      // calls use them?!
      this->T1 = *trans1;
      this->T2 = *trans2;
      //	int size = (int)(udword(TreeCollider.GetNbPairs()));
      //	N_pairs = size;
      return true;
    }
    else
    {
      return false;
    } 
  }
  else
  {
    return false; 
  }
}

int csOPCODECollideSystem::CollidePath (
  iCollider* collider, const csReversibleTransform* trans,
	csVector3& newpos,
	int num_colliders,
	iCollider** colliders,
	csReversibleTransform** transforms)
{
  (void)collider;
  (void)trans;
  (void)newpos;
  (void)num_colliders;
  (void)colliders;
  (void)transforms;
	return 0;
  // csOPCODECollider* thiscol = (csOPCODECollider*) collider;
  // return thiscol->CollidePath (trans, newpos,	num_colliders, colliders, transforms);
}

csCollisionPair* csOPCODECollideSystem::GetCollisionPairs ()
{     
  int size = (int) (udword(TreeCollider.GetNbPairs ()));
  N_pairs = size;
  if (N_pairs == 0) { return NULL; };
  const Pair* colPairs=TreeCollider.GetPairs ();
  Point* vertholder0 = col1->vertholder;
  Point* vertholder1 = col2->vertholder;
  udword* indexholder0 = col1->indexholder;
  udword* indexholder1 = col2->indexholder;
  Point* current;
  int i, j;
  
  if (pairs)
  {
    delete[] pairs;
  }	   
  pairs = new csCollisionPair[N_pairs];
   
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
  //	   printf( " 1) \n");

  for(i = 0; i < N_pairs; i++)
  {
    j = 3 * colPairs[i].id0;
    current = &vertholder0[indexholder0[j]];		 
    pairs[i].a1 = T1.This2Other (csVector3 (current->x, current->y, current->z)); 
    current = &vertholder0[indexholder0[j + 1]];		 
    pairs[i].b1 = T1.This2Other (csVector3 (current->x, current->y, current->z)); 
    current = &vertholder0[indexholder0[j + 2]];		 
    pairs[i].c1 = T1.This2Other (csVector3 (current->x, current->y, current->z)); 

    j = 3 * colPairs[i].id1; 
    current = &vertholder1[indexholder1[j ]];		 
    pairs[i].a2 = T2.This2Other (csVector3 (current->x, current->y, current->z)); 
    current = &vertholder1[indexholder1[j + 1 ]];		 
    pairs[i].b2 = T2.This2Other (csVector3 (current->x, current->y, current->z)); 
    current = &vertholder1[indexholder1[j + 2 ]];		 
    pairs[i].c2 = T2.This2Other (csVector3 (current->x, current->y, current->z)); 
  }
  return pairs;
}

int csOPCODECollideSystem::GetCollisionPairCount ()
{
  int size = (int) (udword(TreeCollider.GetNbPairs ()));
  N_pairs = size;
  return N_pairs;
}

void csOPCODECollideSystem::ResetCollisionPairs ()
{
  if (pairs)
  {
    delete[] pairs;
    pairs = NULL;
    N_pairs = 0;
  }
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
  return TreeCollider.FirstContactEnabled ();
}

/**
* Test if an object can move to a new position. The new position
* vector will be modified to reflect the maximum new position that the
* object could move to without colliding with something. This function
* will return:
* <ul>
* <li>-1 if the object could not move at all (i.e. stuck at start position).
* <li>0 if the object could not move fully to the desired position.
* <li>1 if the object can move unhindered to the end position.
* </ul>
* <p>
* This function will reset the collision pair array. If there was a
* collision along the way the array will contain the information for
* the first collision preventing movement.
* <p>
* The given transform should be the transform of the object corresponding
* with the old position. 'colliders' and 'transforms' should be arrays
* with 'num_colliders' elements for all the objects that we should
* test against.
*/
