/*
    Copyright (C) 2002 Anders Stenberg

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
#include "cstool/collider.h"
#include "iutil/objreg.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/collider.h"
#include "igeom/polymesh.h"
#include "iengine/engine.h"
#include "imesh/object.h"
#include "csgeom/obb.h"
#include "qsqrt.h"

#include "odedynam.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csODEDynamics)
  SCF_IMPLEMENTS_INTERFACE (iDynamics)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEDynamics::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csODEDynamicSystem)
  SCF_IMPLEMENTS_INTERFACE (iDynamicSystem)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csODERigidBody)
  SCF_IMPLEMENTS_INTERFACE (iRigidBody)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csODEJoint)
  SCF_IMPLEMENTS_INTERFACE (iJoint)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csODEDefaultMoveCallback)
  SCF_IMPLEMENTS_INTERFACE (iDynamicsMoveCallback)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csODEDynamics)

SCF_EXPORT_CLASS_TABLE (odedynam)
  SCF_EXPORT_CLASS (csODEDynamics, "crystalspace.dynamics.ode",
	"Dynamics system using ODE.")
SCF_EXPORT_CLASS_TABLE_END


int csODEDynamics::geomclassnum = 0;
dJointGroupID csODEDynamics::contactjoints = dJointGroupCreate (0);

csODEDynamics::csODEDynamics (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  object_reg = NULL;

  dGeomClass c;
  c.bytes = sizeof (iMeshWrapper *);
  c.collider = &CollideSelector;
  c.aabb = &GetAABB;
  c.aabb_test = 0;
  c.dtor = 0;
  geomclassnum = dCreateGeomClass (&c); 
}

csODEDynamics::~csODEDynamics ()
{
  systems.DeleteAll();
}

bool csODEDynamics::Initialize (iObjectRegistry* object_reg)
{
  csODEDynamics::object_reg = object_reg;

  return true;
}

iDynamicSystem* csODEDynamics::CreateSystem ()
{
  csODEDynamicSystem* system = new csODEDynamicSystem ();
  iDynamicSystem* isystem = SCF_QUERY_INTERFACE (system, iDynamicSystem);
  systems.Push (isystem);
  return isystem;
}

void csODEDynamics::RemoveSystem (iDynamicSystem* system)
{
  systems.Delete (system, true);
}

void csODEDynamics::Step (float stepsize)
{
  for (long i=0; i<systems.Length(); i++)
  {
    ((iDynamicSystem*)systems.Get (i))->Step (stepsize);
    dJointGroupEmpty (contactjoints);
  }
}

void csODEDynamics::NearCallback (void *data, dGeomID o1, dGeomID o2)
{
  dContact contact[10];
  int a = dCollide (o1, o2, 10, &(contact[0].geom), sizeof (dContact));

  if (a > 0) 
  {
    iRigidBody *b1 = (iRigidBody *)dBodyGetData (dGeomGetBody(o1));
    iRigidBody *b2 = (iRigidBody *)dBodyGetData (dGeomGetBody(o2));

    if (b1) b1->Collision (b2);
    if (b2) b2->Collision (b1);

    for( int i=0; i<a; i++ )
    {
      float *f1 = (float*)dGeomGetData (o1);
      float *f2 = (float*)dGeomGetData (o2);
      
      contact[i].surface.mode = dContactBounce;
      contact[i].surface.mu = f1[0]*f2[0];
      contact[i].surface.bounce = f1[1]*f2[1];
      dJointID c = dJointCreateContact ( ((csODEDynamicSystem*)data)
    	  ->GetWorldID(), contactjoints,contact+i );
      dJointAttach (c,dGeomGetBody (o1),dGeomGetBody (o2)); 
    }
  }
  return;
}

csReversibleTransform GetGeomTransform (dGeomID id)
{
  const dReal* pos = dGeomGetPosition (id);
  const dReal* mat = dGeomGetRotation (id);
  /* Need to use the inverse in this case */
  csMatrix3 rot;
  rot.m11 = mat[0];	rot.m12 = mat[4];	rot.m13 = mat[8];
  rot.m21 = mat[1];	rot.m22 = mat[5];	rot.m23 = mat[9];
  rot.m31 = mat[2];	rot.m32 = mat[6];	rot.m33 = mat[10];
  return csReversibleTransform (rot, csVector3 (pos[0], pos[1], pos[2]));
}

int csODEDynamics::CollideMeshMesh (dGeomID o1, dGeomID o2, int flags,
	dContactGeom *contact, int skip)
{
  return 0;
  // TODO: Implement collision for meshes 
  // (old code left, but not even close to working)
  /*colliderdata* cd1 = (colliderdata*)dGeomGetClassData (o1);
  colliderdata* cd2 = (colliderdata*)dGeomGetClassData (o2);

  if (cd1->collsys != cd2->collsys) return 0;

  const csReversibleTransform t1 = GetGeomTransform(o1);
  const csReversibleTransform t2 = GetGeomTransform(o2);

  cd1->collsys->SetOneHitOnly (false);
  cd1->collsys->Collide( cd1->collider, &t1, cd2->collider, &t2);

  csCollisionPair* cp = cd1->collsys->GetCollisionPairs();

  for (int i=0; (i<cd1->collsys->GetCollisionPairCount()) && (i<flags); i++)
  {
    csPlane3 plane (cp[i].a1*t1, cp[i].b1*t1, cp[i].c1*t1);

    csVector3 v = cp[i].a2*t2;
    float depth = plane.Distance(v);

    csVector3 v2 = cp[i].b2*t2;
    float d = plane.Distance(v2);
    if (d>depth)
    {
      depth = d;
      v = v2;
    }

    v2 = cp[i].c2*t2;
    d = plane.Distance(v2);
    if (d>depth)
    {
      depth = d;
      v = v2;
    }

    contact[i].depth = depth;
    contact[i].g1 = o1;
    contact[i].g2 = o2;
    contact[i].normal[0] = plane.norm.x;
    contact[i].normal[1] = plane.norm.y;
    contact[i].normal[2] = plane.norm.z;
    contact[i].pos[0] = v.x;
    contact[i].pos[1] = v.y;
    contact[i].pos[2] = v.z;
  }
  return i;*/
}

/* defined in ode */
extern int dCollideBP (const dxGeom *o1, const dxGeom *o2, int flags, dContactGeom *outcontacts, int skip);

CS_TYPEDEF_GROWING_ARRAY(csPolyMeshList, csMeshedPolygon);

int csODEDynamics::CollideMeshBox (dGeomID mesh, dGeomID box, int flags,
	dContactGeom *outcontacts, int skip)
{
  int N = flags & 0xFF;

  // rotate everything so that box is axis aligned.
  dVector3 sides;
  dGeomBoxGetLengths (box, sides);
  // box is symetric.
  csVector3 aabb(sides[0]/2, sides[1]/2, sides[2]);
  csBox3 boxbox(aabb, -aabb);
  csReversibleTransform boxt = GetGeomTransform (box);

  iMeshWrapper *m = *(iMeshWrapper **)dGeomGetClassData (mesh);
  CS_ASSERT (m);
  iPolygonMesh *p = SCF_QUERY_INTERFACE (m->GetMeshObject(), iPolygonMesh);
  csVector3 *vertex_table = p->GetVertices ();
  csMeshedPolygon *polygon_list = p->GetPolygons ();

  csReversibleTransform mesht = GetGeomTransform (mesh);

  csPolyMeshList polycollide;
  // test for overlap
  int i, j, k;
  for (i = 0; i < p->GetPolygonCount(); i ++) {
    csBox3 polybox;
    for (j = 0; j < polygon_list[i].num_vertices; j ++) {
      polybox.AddBoundingVertex (boxt * (vertex_table[polygon_list[i].vertices[j]] / mesht));
    }
    // aabb poly against aabb box for overlap.
    // Full collision later will weed out the rest;
    if (polybox.Overlap (boxbox)) {
      polycollide.Push (polygon_list[i]);
    }
  }
  int outcount = 0;
  // the value of N should be large, just in case.
  for (i = 0; i < polycollide.Length() && outcount < N; i ++) {
    csPlane3 plane(vertex_table[polycollide[i].vertices[0]] / mesht,
      vertex_table[polycollide[i].vertices[1]] / mesht,
      vertex_table[polycollide[i].vertices[2]] / mesht);
    plane.Normalize ();
    // dCollideBP only works if box center is on the outside of the plane
    if (plane.Classify (boxt.GetOrigin()) < 0) {
      continue;
    } 
    dGeomID odeplane = dCreatePlane(0,plane.norm.x, plane.norm.y, plane.norm.z,
      -plane.DD);
    dContactGeom tempcontacts[5];
    int count = dCollideBP (box, odeplane, 5, tempcontacts, sizeof (dContactGeom));
    dGeomDestroy (odeplane);
    for (j = 0; j < count; j ++) {
      dContactGeom *c = &tempcontacts[j];
      csVector3 contactpos(c->pos[0], c->pos[1], c->pos[2]);
      // make sure the point lies inside the polygon
      int vcount = polycollide[i].num_vertices;
      for (k = 0; k < vcount-1; k ++) {
	int ind1 = polycollide[i].vertices[k];
	int ind2 = polycollide[i].vertices[k+1];
        csPlane3 edgeplane(vertex_table[ind1] / mesht,
          vertex_table[ind2] / mesht,
          vertex_table[ind2] / mesht - plane.Normal());
        edgeplane.Normalize();
        if (edgeplane.Classify (contactpos) < 0) {
          c->depth = -1;
          break;
        }
      }
      /* get the wrap around case */
      int ind1 = polycollide[i].vertices[vcount-1];
      int ind2 = polycollide[i].vertices[0];
      csPlane3 edgeplane(vertex_table[ind1] / mesht,
        vertex_table[ind2] / mesht,
        vertex_table[ind2] / mesht - plane.Normal());
      edgeplane.Normalize();
      if (edgeplane.Classify (contactpos) < 0) {
        c->depth = -1;
      }
      if (c->depth >= 0) {
        dContactGeom *out = (dContactGeom *)((char *)outcontacts + skip * outcount);
        out->pos[0] = c->pos[0];
        out->pos[1] = c->pos[1];
        out->pos[2] = c->pos[2];
        out->normal[0] = c->normal[0];
        out->normal[1] = c->normal[1];
        out->normal[2] = c->normal[2];
        out->depth = c->depth;
        out->g1 = mesh;
        out->g2 = box;
        outcount ++;
      }
    }
  }
  return outcount;
}

int csODEDynamics::CollideMeshSphere (dGeomID mesh, dGeomID sphere, int flags,
	dContactGeom *outcontacts, int skip)
{
  int N = flags & 0xFF;
  const dReal *pos = dGeomGetPosition (sphere);
  csVector3 center(pos[0], pos[1], pos[2]);
  dReal rad = dGeomSphereGetRadius (sphere); 
  iMeshWrapper *m = *(iMeshWrapper **)dGeomGetClassData (mesh);
  CS_ASSERT (m);
  iPolygonMesh *p = SCF_QUERY_INTERFACE (m->GetMeshObject(), iPolygonMesh);
  csVector3 *vertex_table = p->GetVertices ();
  csMeshedPolygon *polygon_list = p->GetPolygons ();

  csReversibleTransform mesht = GetGeomTransform (mesh);

  int outcount = 0;

  for (int i = 0; i < p->GetPolygonCount() && outcount < N; i ++) {
    csPlane3 plane(vertex_table[polygon_list[i].vertices[0]] / mesht,
      vertex_table[polygon_list[i].vertices[1]] / mesht,
      vertex_table[polygon_list[i].vertices[2]] / mesht);
    plane.Normalize();
    if (plane.Classify (center) < 0) {
      continue;
    }
    float depth = rad - plane.Distance (center);
    if (depth < 0) {
      continue;
    }
    int vcount = polygon_list[i].num_vertices;
    for (int j = 0; j < vcount-1; j ++) {
      int ind1 = polygon_list[i].vertices[j];
      int ind2 = polygon_list[i].vertices[j+1];
      csPlane3 edgeplane(vertex_table[ind1] / mesht,
        vertex_table[ind2] / mesht,
        vertex_table[ind2] / mesht - plane.Normal());
      edgeplane.Normalize();	
      if (edgeplane.Classify (center) < 0) {
        csVector3 line = vertex_table[ind2] - vertex_table[ind1];
        line.Normalize();
        float proj = center * line;
        /* if the point projects on this edge, but outside the poly test 
         * for depth to this edge (especially important on sharp corners)
         */
        if ((proj > (vertex_table[ind1] * line)) &&
            (proj < (vertex_table[ind2] * line))) {
          csVector3 projpt = vertex_table[ind1] + line * proj;
          csVector3 newnorm = center - projpt;
          float dist = newnorm.Norm();
          depth = rad - dist;
          newnorm /= dist;
          plane.Set (newnorm.x, newnorm.y, newnorm.z, 0);
        } else {
          /* this is an invalid the ball is outside the poly at this edge */
          depth = -1;
        }
      }
    }
    /* get the one last edge from END to 0 */
    int ind1 = polygon_list[i].vertices[vcount-1];
    int ind2 = polygon_list[i].vertices[0];
    csPlane3 edgeplane (vertex_table[ind1] / mesht,
      vertex_table[ind2] / mesht,
      vertex_table[ind2] / mesht - plane.Normal());
    edgeplane.Normalize ();
    if (edgeplane.Classify (center) < 0) {
      csVector3 line = vertex_table[ind2] - vertex_table[ind1];
      line.Normalize();
      float proj = center * line;
      /* if the point projects on this edge, but outside the poly test 
       * for depth to this edge (especially important on sharp corners)
       */
      if ((proj > (vertex_table[ind1] * line)) &&
          (proj < (vertex_table[ind2] * line))) {
        csVector3 projpt = vertex_table[ind1] + line * proj;
        csVector3 newnorm = center - projpt;
        float dist = newnorm.Norm();
        depth = rad - dist;
        newnorm /= dist;
        plane.Set (newnorm.x, newnorm.y, newnorm.z, 0);
      } else {
        /* this is an invalid the ball is outside the poly at this edge */
        depth = -1;
      }
    }
    if (depth < 0) {
      continue;
    }
    dContactGeom *out = (dContactGeom *)((char *)outcontacts + skip * outcount);
    csVector3 cpos = center - (plane.Normal() * rad);
    out->pos[0] = cpos.x;
    out->pos[1] = cpos.y;
    out->pos[2] = cpos.z;
    out->normal[0] = -plane.Normal().x;
    out->normal[1] = -plane.Normal().y;
    out->normal[2] = -plane.Normal().z;
    out->depth = depth;
    outcount ++;
  }
  return outcount;
}

void csODEDynamics::GetAABB (dGeomID g, dReal aabb[6])
{
  csBox3 box;
  csReversibleTransform mesht = GetGeomTransform (g);
  iMeshWrapper *m = *(iMeshWrapper **)dGeomGetClassData (g);
  iPolygonMesh *p = SCF_QUERY_INTERFACE (m->GetMeshObject(), iPolygonMesh);
  csVector3 *vertex_table = p->GetVertices ();
  box.StartBoundingBox ();
  for (int i = 0; i < p->GetVertexCount(); i ++) {
    box.AddBoundingVertex (vertex_table[i] / mesht);
  }
  aabb[0] = box.MinX(); aabb[1] = box.MaxX();
  aabb[2] = box.MinY(); aabb[3] = box.MaxY();
  aabb[4] = box.MinZ(); aabb[5] = box.MaxZ();
}

csODEDynamicSystem::csODEDynamicSystem ()
{
  SCF_CONSTRUCT_IBASE (NULL);

  //TODO: QUERY for collidesys

  worldID = dWorldCreate ();
  spaceID = dHashSpaceCreate ();
  dWorldSetCFM (worldID,1e-5);
  move_cb = (iDynamicsMoveCallback*)new csODEDefaultMoveCallback ();
}

csODEDynamicSystem::~csODEDynamicSystem ()
{
  SCF_DEC_REF (collidesys);
  dSpaceDestroy (spaceID);
  dWorldDestroy (worldID);
  if (move_cb) move_cb->DecRef ();
}

iRigidBody* csODEDynamicSystem::CreateBody ()
{
  csODERigidBody* body = new csODERigidBody (this);
  bodies.Push (body);
  body->SetMoveCallback(move_cb);
  return (iRigidBody*)body;
}

void csODEDynamicSystem::RemoveBody (iRigidBody* body)
{
  bodies.Delete (body, true);
}

iJoint* csODEDynamicSystem::CreateJoint ()
{
  csODEJoint* joint = new csODEJoint (this);
  joints.Push (joint);
  return (iJoint*)joint;
}

void csODEDynamicSystem::RemoveJoint (iJoint *joint)
{
  joints.Delete (joint, true);
}

void csODEDynamicSystem::SetGravity (const csVector3& v)
{
  dWorldSetGravity (worldID, v.x, v.y, v.z);
}

const csVector3 csODEDynamicSystem::GetGravity () const
{
  dVector3 grav;
  dWorldGetGravity (worldID, grav);
  return csVector3 (grav[0], grav[1], grav[2]);
}

void csODEDynamicSystem::Step (float stepsize)
{
  dSpaceCollide (spaceID, this, &csODEDynamics::NearCallback);
  dWorldStep (worldID, stepsize);
  for (long i=0; i<bodies.Length(); i++)
    ((iRigidBody*)bodies.Get (i))->Update ();
}

csODERigidBody::csODERigidBody (csODEDynamicSystem* sys)
{
  SCF_CONSTRUCT_IBASE (NULL);

  dynsys = sys;
  SCF_INC_REF (dynsys);

  bodyID = dBodyCreate (dynsys->GetWorldID());
  dBodySetData (bodyID, this);
  groupID = dCreateGeomGroup (dynsys->GetSpaceID());
  statjoint = 0;

  mesh = NULL;
  bone = NULL;
  move_cb = NULL;
  coll_cb = NULL;
}

csODERigidBody::~csODERigidBody ()
{
  if (move_cb) move_cb->DecRef ();
  if (coll_cb) coll_cb->DecRef ();
  SCF_DEC_REF (dynsys);
  dBodyDestroy (bodyID);
}

bool csODERigidBody::MakeStatic ()
{
  if (statjoint == 0)
  {
    statjoint = dJointCreateFixed (dynsys->GetWorldID(), 0);
    dJointAttach (statjoint, bodyID, 0);
    dBodySetGravityMode (bodyID, 0);
  }
  return true;
}

bool csODERigidBody::MakeDynamic ()
{
  if (statjoint != 0) {
    dJointDestroy (statjoint);
    dBodySetGravityMode (bodyID, 1);
  }
  return true;
}

bool csODERigidBody::AttachColliderMesh (iMeshWrapper *mesh,
	const csOrthoTransform &trans, float friction, float density,
	float elasticity)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (dynsys->GetSpaceID());
  dGeomTransformSetCleanup (id, 1);

  dGeomID gid = dCreateGeom (csODEDynamics::GetGeomClassNum());
  // dSpaceAdd (dynsys->GetSpaceID(), gid);
  iMeshWrapper **gdata = (iMeshWrapper **)dGeomGetClassData (gid);
  *gdata = mesh;
  dGeomTransformSetGeom (id, gid);

  csOBB b;
  iPolygonMesh *p = SCF_QUERY_INTERFACE (mesh->GetMeshObject(), iPolygonMesh);
  b.FindOBB (p->GetVertices(), p->GetVertexCount());
  dMassSetBox (&m, density, b.MaxX()-b.MinX(), b.MaxY()-b.MinY(), b.MaxZ()-b.MinZ());

  dMatrix3 mat;
  mat[0] = b.GetMatrix().m11; mat[1] = b.GetMatrix().m12; mat[2] = b.GetMatrix().m13;
  mat[3] = b.GetMatrix().m21; mat[4] = b.GetMatrix().m22; mat[5] = b.GetMatrix().m23;
  mat[6] = b.GetMatrix().m31; mat[7] = b.GetMatrix().m32; mat[8] = b.GetMatrix().m33;

  mat[0] = trans.GetO2T().m11; mat[1] = trans.GetO2T().m12; mat[2] = trans.GetO2T().m13; mat[3] = 0;
  mat[4] = trans.GetO2T().m21; mat[5] = trans.GetO2T().m22; mat[6] = trans.GetO2T().m23; mat[7] = 0;
  mat[8] = trans.GetO2T().m31; mat[9] = trans.GetO2T().m32; mat[10] = trans.GetO2T().m33; mat[11] = 0;
  dGeomSetRotation (gid, mat);
  dMassRotate (&m, mat);

  dGeomSetPosition (gid,
	trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dMassTranslate (&m,
  	trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dBodyGetMass (bodyID, &om);
  dMassAdd (&om, &m);

  dBodySetMass (bodyID, &om);
  
  dGeomSetBody (id, bodyID);
  ids.Push (id);

  float *f = new float[2];
  f[0] = friction;
  f[1] = elasticity;
  dGeomSetData (id, (void*)f);

  return true;
}

bool csODERigidBody::AttachColliderCylinder (float length, float radius,
	const csOrthoTransform& trans, float friction, float density,
	float elasticity)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (dynsys->GetSpaceID());
  dGeomTransformSetCleanup (id, 1);
	
  dGeomID gid = dCreateCCylinder (0, radius, length);
  dGeomTransformSetGeom (id, gid);
  dMassSetCappedCylinder (&m, density, 3, radius, length);

  dMatrix3 mat;
  mat[0] = trans.GetO2T().m11; mat[1] = trans.GetO2T().m12; mat[2] = trans.GetO2T().m13; mat[3] = 0;
  mat[4] = trans.GetO2T().m21; mat[5] = trans.GetO2T().m22; mat[6] = trans.GetO2T().m23; mat[7] = 0;
  mat[8] = trans.GetO2T().m31; mat[9] = trans.GetO2T().m32; mat[10] = trans.GetO2T().m33; mat[11] = 0;
  dGeomSetRotation (gid, mat);
  dMassRotate (&m, mat);

  dGeomSetPosition (gid,
	trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dMassTranslate (&m,
  	trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dBodyGetMass (bodyID, &om);
  dMassAdd (&om, &m);

  // Old correction of center of mass. 
  // Just stored in case it's actually needed.
  /*dGeomSetPosition( gid, trans.GetOrigin().x-om.c[0], trans.GetOrigin().y-om.c[1], trans.GetOrigin().z-om.c[2] );
  dMassTranslate (&om, -om.c[0], -om.c[1], -om.c[2]);*/

  dBodySetMass (bodyID, &om);
  
  dGeomSetBody (id, bodyID);
  ids.Push (id);

  float *f = new float[2];
  f[0] = friction;
  f[1] = elasticity;
  dGeomSetData (id, (void*)f);

  return true;
}

bool csODERigidBody::AttachColliderBox (csVector3 size,
	const csOrthoTransform& trans, float friction, float density,
	float elasticity)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (dynsys->GetSpaceID());
  dGeomTransformSetCleanup (id, 1);
	
  dGeomID gid = dCreateBox (0, size.x, size.y, size.z);
  dGeomTransformSetGeom (id, gid);
  dMassSetBox (&m, density, size.x, size.y, size.z);

  dMatrix3 mat;
  mat[0] = trans.GetO2T().m11; mat[1] = trans.GetO2T().m12; mat[2] = trans.GetO2T().m13; mat[3] = 0;
  mat[4] = trans.GetO2T().m21; mat[5] = trans.GetO2T().m22; mat[6] = trans.GetO2T().m23; mat[7] = 0;
  mat[8] = trans.GetO2T().m31; mat[9] = trans.GetO2T().m32; mat[10] = trans.GetO2T().m33; mat[11] = 0;
  dGeomSetRotation (gid, mat);
  dMassRotate (&m, mat);

  dGeomSetPosition (gid,
	trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dMassTranslate (&m,
  	trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dBodyGetMass (bodyID, &om);
  dMassAdd (&om, &m);

  // Old correction of center of mass. 
  // Just stored in case it's actually needed.
  /*dGeomSetPosition( gid, trans.GetOrigin().x-om.c[0], trans.GetOrigin().y-om.c[1], trans.GetOrigin().z-om.c[2] );
  dMassTranslate (&om, -om.c[0], -om.c[1], -om.c[2]);*/

  dBodySetMass (bodyID, &om);

  dGeomSetBody (id, bodyID);
  ids.Push (id);

  float *f = new float[2];
  f[0] = friction;
  f[1] = elasticity;
  dGeomSetData (id, (void*)f);

  return true;
}

bool csODERigidBody::AttachColliderSphere (float radius, const csVector3 &offset,
	float friction, float density, float elasticity)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (dynsys->GetSpaceID());
  dGeomTransformSetCleanup (id, 1);
	
  dGeomID gid = dCreateSphere (0, radius);
  dGeomTransformSetGeom (id, gid);
  dMassSetSphere (&m, density, radius);

  dGeomSetPosition (gid, offset.x, offset.y, offset.z);
  dMassTranslate (&m, offset.x, offset.y, offset.z);
  dBodyGetMass (bodyID, &om);
  dMassAdd (&om, &m);

  // Old correction of center of mass.
  // Just stored in case it's actually needed.
  /*dGeomSetPosition( gid, offset.x-om.c[0], offset.y-om.c[1], offset.z-om.c[2] );
  dMassTranslate (&om, -om.c[0], -om.c[1], -om.c[2]);*/

  dBodySetMass (bodyID, &om);

  dGeomSetBody (id, bodyID);
  ids.Push (id);

  float *f = new float[2];
  f[0] = friction;
  f[1] = elasticity;
  dGeomSetData (id, (void*)f);

  return true;
}

void csODERigidBody::SetPosition (const csVector3& pos)
{
  dBodySetPosition (bodyID, pos.x, pos.y, pos.z);
}

const csVector3 csODERigidBody::GetPosition () const
{
  const dReal* pos = dBodyGetPosition (bodyID);
  return csVector3 (pos[0], pos[1], pos[2]);
}

void csODERigidBody::SetOrientation (const csMatrix3& rot)
{
  dMatrix3 mat;
  mat[0] = rot.m11; mat[1] = rot.m12; mat[2] = rot.m13; mat[3] = 0;
  mat[4] = rot.m21; mat[5] = rot.m22; mat[6] = rot.m23; mat[7] = 0;
  mat[8] = rot.m31; mat[9] = rot.m32; mat[10] = rot.m33; mat[11] = 0;
  dBodySetRotation (bodyID, mat);
}

const csMatrix3 csODERigidBody::GetOrientation () const
{
  const dReal* mat = dBodyGetRotation (bodyID);
  csMatrix3 rot;
  rot.m11 = mat[0]; rot.m12 = mat[1]; rot.m13 = mat[2];
  rot.m21 = mat[4]; rot.m22 = mat[5]; rot.m23 = mat[6];
  rot.m31 = mat[8]; rot.m32 = mat[9]; rot.m33 = mat[10];
  return rot;
}

void csODERigidBody::SetTransform (const csOrthoTransform& trans)
{
  csVector3 pos = trans.GetOrigin ();
  dBodySetPosition (bodyID, pos.x, pos.y, pos.z);
  csMatrix3 rot = trans.GetO2T ();
  dMatrix3 mat;
  mat[0] = rot.m11; mat[1] = rot.m12; mat[2] = rot.m13; mat[3]  = 0;
  mat[4] = rot.m21; mat[5] = rot.m22; mat[6] = rot.m23; mat[7]  = 0;
  mat[8] = rot.m31; mat[9] = rot.m32; mat[10] = rot.m33; mat[11] = 0;
  dBodySetRotation (bodyID, mat);
}

const csOrthoTransform csODERigidBody::GetTransform () const
{
  const dReal* pos = dBodyGetPosition (bodyID);
  const dReal* mat = dBodyGetRotation (bodyID);
  csMatrix3 rot;
  rot.m11 = mat[0]; rot.m12 = mat[1]; rot.m13 = mat[2];
  rot.m21 = mat[4]; rot.m22 = mat[5]; rot.m23 = mat[6];
  rot.m31 = mat[8]; rot.m32 = mat[9]; rot.m33 = mat[10];
  return csOrthoTransform (rot, csVector3 (pos[0], pos[1], pos[2]));
}

void csODERigidBody::SetLinearVelocity (const csVector3& vel)
{
  dBodySetLinearVel (bodyID, vel.x, vel.y, vel.z);
}

const csVector3 csODERigidBody::GetLinearVelocity () const
{
  const dReal* vel = dBodyGetLinearVel (bodyID);
  return csVector3 (vel[0], vel[1], vel[2]);
}

void csODERigidBody::SetAngularVelocity (const csVector3& vel)
{
  dBodySetAngularVel (bodyID, vel.x, vel.y, vel.z);
}

const csVector3 csODERigidBody::GetAngularVelocity () const
{
  const dReal* vel = dBodyGetAngularVel (bodyID);
  return csVector3 (vel[0], vel[1], vel[2]);
}

void csODERigidBody::SetProperties (float mass,
	const csVector3& center, const csMatrix3& inertia)
{
  dMass* m = new dMass;
  m->mass = mass;
  m->c[0] = center.x; m->c[1] = center.y; m->c[2] = center.z; m->c[3] = 0;
  m->I[0] = inertia.m11; m->I[1] = inertia.m12; m->I[2] = inertia.m13; m->I[3] = 0;
  m->I[4] = inertia.m21; m->I[5] = inertia.m22; m->I[6] = inertia.m23; m->I[7] = 0;
  m->I[8] = inertia.m31; m->I[9] = inertia.m32; m->I[10] = inertia.m33; m->I[11] = 0;
  dBodySetMass (bodyID, m);
}

void csODERigidBody::GetProperties (float* mass,
  csVector3* center, csMatrix3* inertia)
{
  dMass m;
  dBodyGetMass (bodyID, &m);
  if (mass != NULL) *mass = m.mass;
  if (center != NULL) center->Set (m.c[0], m.c[1], m.c[2]);
  if (inertia != NULL) {
    inertia->Set (m.I[0], m.I[1], m.I[2],
     m.I[4], m.I[5], m.I[6],
     m.I[8], m.I[9], m.I[10]);
  }
}

void csODERigidBody::AdjustTotalMass (float targetmass)
{
  dMass m;
  dBodyGetMass (bodyID, &m);
  dMassAdjust (&m, targetmass);
  dBodySetMass (bodyID, &m);
}

void csODERigidBody::AddForce (const csVector3& force)
{
  dBodyAddForce (bodyID, force.x, force.y, force.z);
}

void csODERigidBody::AddTorque (const csVector3& force)
{
  dBodyAddTorque (bodyID, force.x, force.y, force.z);
}

void csODERigidBody::AddRelForce (const csVector3& force)
{
  dBodyAddRelForce (bodyID, force.x, force.y, force.z);
}

void csODERigidBody::AddRelTorque (const csVector3& force)
{
  dBodyAddRelTorque (bodyID, force.x, force.y, force.z);
}

void csODERigidBody::AddForceAtPos (const csVector3& force,
	const csVector3& pos)
{
  dBodyAddForceAtPos (bodyID, force.x, force.y, force.z, pos.x, pos.y, pos.z);
}

void csODERigidBody::AddForceAtRelPos (const csVector3& force,
	const csVector3& pos)
{
  dBodyAddForceAtRelPos (bodyID, force.x, force.y, force.z,
	pos.x, pos.y, pos.z);
}

void csODERigidBody::AddRelForceAtPos (const csVector3& force,
	const csVector3& pos)
{
  dBodyAddRelForceAtPos (bodyID, force.x, force.y, force.z,
  	pos.x, pos.y, pos.z);
}

void csODERigidBody::AddRelForceAtRelPos (const csVector3& force,
	const csVector3& pos)
{
  dBodyAddRelForceAtRelPos (bodyID, force.x, force.y, force.z,
  	pos.x, pos.y, pos.z);
}

const csVector3 csODERigidBody::GetForce () const
{
  const dReal* force = dBodyGetForce (bodyID);
  return csVector3 (force[0], force[1], force[2]);
}

const csVector3 csODERigidBody::GetTorque () const
{
  const dReal* force = dBodyGetTorque (bodyID);
  return csVector3 (force[0], force[1], force[2]);
}

void csODERigidBody::AttachMesh (iMeshWrapper* m)
{
  if (m) m->IncRef ();
  if (mesh) mesh->DecRef ();
  mesh = m;
}

void csODERigidBody::AttachBone (iSkeletonBone* b)
{
  if (b) b->IncRef ();
  if (bone) bone->DecRef ();
  bone = b;
}

void csODERigidBody::SetMoveCallback (iDynamicsMoveCallback* cb)
{
  if (cb) cb->IncRef ();
  if (move_cb) move_cb->DecRef ();
  move_cb = cb;
}

void csODERigidBody::SetCollisionCallback (iDynamicsCollisionCallback* cb)
{
  if (cb) cb->IncRef ();
  if (coll_cb) coll_cb->DecRef();
  coll_cb = cb;
}

void csODERigidBody::Collision (iRigidBody *other)
{
  if (coll_cb) coll_cb->Execute (this, other);
}

void csODERigidBody::Update ()
{
  if (bodyID && !statjoint && move_cb)
  {
    csOrthoTransform trans;
    trans = GetTransform ();

    if (mesh) move_cb->Execute (mesh, trans);
    if (bone) move_cb->Execute (bone, trans);
  }
}

csODEJoint::csODEJoint (csODEDynamicSystem *sys)
{
  SCF_CONSTRUCT_IBASE (NULL);
  jointID = 0;

  body[0] = body[1] = NULL;
  bodyID[0] = bodyID[1] = 0;

  transConstraint[0] = 1;
  transConstraint[1] = 1;
  transConstraint[2] = 1;
  rotConstraint[0] = 1;
  rotConstraint[1] = 1;
  rotConstraint[2] = 1;

  dynsys = sys;
  SCF_INC_REF(dynsys);
}

csODEJoint::~csODEJoint ()
{
  SCF_DEC_REF (dynsys);
  if (jointID) { dJointDestroy (jointID); }
}

void csODEJoint::Attach (iRigidBody *b1, iRigidBody *b2)
{
  bodyID[0] = ((csODERigidBody *)b1)->GetID();
  bodyID[1] = ((csODERigidBody *)b2)->GetID();
  body[0] = b1;
  body[1] = b2;
  BuildJoint ();
}

iRigidBody *csODEJoint::GetAttachedBody (int b)
{
  return (b == 0) ? body[0] : body[1];
}

void csODEJoint::SetTransform (const csOrthoTransform &trans)
{
  transform = trans;	
  BuildJoint ();
}

csOrthoTransform csODEJoint::GetTransform ()
{
  return transform;
}

void csODEJoint::SetTransConstraints (bool X, bool Y, bool Z)
{
  /* 1 means free and 0 means constrained */
  transConstraint[0] = (X) ? 0 : 1;
  transConstraint[1] = (Y) ? 0 : 1;
  transConstraint[2] = (Z) ? 0 : 1;
  BuildJoint ();
}

void csODEJoint::SetMinimumDistance (const csVector3 &min)
{
  minTrans = min;
  BuildJoint ();
}
csVector3 csODEJoint::GetMinimumDistance ()
{
  return minTrans;
}
void csODEJoint::SetMaximumDistance (const csVector3 &max)
{
  maxTrans = max;
  BuildJoint ();
}
csVector3 csODEJoint::GetMaximumDistance ()
{
  return maxTrans;
}

void csODEJoint::SetRotConstraints (bool X, bool Y, bool Z)
{
  /* 1 means free and 0 means constrained */
  rotConstraint[0] = (X) ? 0 : 1;
  rotConstraint[1] = (Y) ? 0 : 1;
  rotConstraint[2] = (Z) ? 0 : 1;
  BuildJoint ();
}
void csODEJoint::SetMinimumAngle (const csVector3 &min)
{
  minAngle = min;
  BuildJoint ();
}
csVector3 csODEJoint::GetMinimumAngle ()
{
  return minAngle;
}
void csODEJoint::SetMaximumAngle (const csVector3 &max)
{
  maxAngle = max;
  BuildJoint ();
}
csVector3 csODEJoint::GetMaximumAngle ()
{
  return maxAngle;
}

void csODEJoint::BuildHinge (const csVector3 &axis, float min, float max) 
{
  dJointSetHingeAxis (jointID, axis.x, axis.y, axis.z);
  if (max > min) {
    dJointSetHingeParam (jointID, dParamLoStop, min);
    dJointSetHingeParam (jointID, dParamHiStop, max);
  } else {
    dJointSetHingeParam (jointID, dParamLoStop, -dInfinity);
    dJointSetHingeParam (jointID, dParamHiStop, dInfinity);
  }
}

void csODEJoint::BuildHinge2 (const csVector3 &axis1, float min1, float max1, 
	const csVector3 &axis2, float min2, float max2) 
{
  dJointSetHinge2Axis1 (jointID, axis1.x, axis1.y, axis1.z);
  dJointSetHinge2Axis2 (jointID, axis2.x, axis2.y, axis2.z);
  if (max1 > min1) {
    dJointSetHinge2Param (jointID, dParamLoStop, min1);
    dJointSetHinge2Param (jointID, dParamHiStop, max1);
  } else {
    dJointSetHinge2Param (jointID, dParamLoStop, -dInfinity);
    dJointSetHinge2Param (jointID, dParamHiStop, dInfinity);
  } 
  if (max2 > min2) {
    dJointSetHinge2Param (jointID, dParamLoStop2, min2);
    dJointSetHinge2Param (jointID, dParamHiStop2, max2);
  } else {
    dJointSetHinge2Param (jointID, dParamLoStop2, -dInfinity);
    dJointSetHinge2Param (jointID, dParamHiStop2, dInfinity);
  } 
}

void csODEJoint::BuildSlider (const csVector3 &axis, float min, float max) 
{
  dJointSetSliderAxis (jointID, axis.x, axis.y, axis.z);
  if (max > min) {
    dJointSetSliderParam (jointID, dParamLoStop, min);
    dJointSetSliderParam (jointID, dParamHiStop, max);
  } else {
    dJointSetSliderParam (jointID, dParamLoStop, -dInfinity);
    dJointSetSliderParam (jointID, dParamHiStop, dInfinity);
  }
}

void csODEJoint::BuildJoint () 
{
  if (!(bodyID[0] && bodyID[1])) {
    return;
  }
  if (jointID) {
    dJointDestroy (jointID);
  }
  int transcount = transConstraint[0] + transConstraint[1] + transConstraint[2];
  int rotcount = rotConstraint[0] + rotConstraint[1] + rotConstraint[2];

  csVector3 pos;
  csMatrix3 rot;
  if (transcount == 0) {
    switch (rotcount) {
      case 0:
        jointID = dJointCreateFixed (dynsys->GetWorldID(), 0);
          dJointAttach (jointID, bodyID[0], bodyID[1]);
          dJointSetFixed (jointID);
        break;
      case 1:
        jointID = dJointCreateHinge (dynsys->GetWorldID(), 0);
          dJointAttach (jointID, bodyID[0], bodyID[1]);
        pos = transform.GetOrigin();
        dJointSetHingeAnchor (jointID, pos.x, pos.y, pos.z);
        rot = transform.GetO2T();
        if (rotConstraint[0]) {
          BuildHinge (rot.Col1(), minAngle.x, maxAngle.x);
        } else if (rotConstraint[1]) {
          BuildHinge (rot.Col2(), minAngle.y, maxAngle.y);
        } else if (rotConstraint[2]) {
          BuildHinge (rot.Col3(), minAngle.z, maxAngle.z);
        }
        // TODO: insert some mechanism for bounce, erp and cfm
        break;
      case 2:
        jointID = dJointCreateHinge2 (dynsys->GetWorldID(), 0);
          dJointAttach (jointID, bodyID[0], bodyID[1]);
        pos = transform.GetOrigin();
        dJointSetHinge2Anchor (jointID, pos.x, pos.y, pos.z);
        rot = transform.GetO2T();

        if (rotConstraint[0]) {
          if (rotConstraint[1]) {
            BuildHinge2 (rot.Col2(), minAngle.y, maxAngle.y,
             rot.Col1(), minAngle.x, maxAngle.x);
          } else {
            BuildHinge2 (rot.Col3(), minAngle.z, maxAngle.z,
             rot.Col1(), minAngle.x, maxAngle.x);
          }
        } else {
          BuildHinge2 (rot.Col2(), minAngle.y, maxAngle.y,
           rot.Col3(), minAngle.z, maxAngle.z);
        }
        break;
      case 3:
        jointID = dJointCreateBall (dynsys->GetWorldID(), 0);
          dJointAttach (jointID, bodyID[0], bodyID[1]);
        pos = transform.GetOrigin();
        dJointSetBallAnchor (jointID, pos.x, pos.y, pos.z);
        break;
    }
  }
  else if (rotcount == 0) {
    switch (transcount) {
      /* 0 is accounted for in the previous condition */
      case 1:
        jointID = dJointCreateSlider (dynsys->GetWorldID(), 0);
          dJointAttach (jointID, bodyID[0], bodyID[1]);
        rot = transform.GetO2T();
        if (transConstraint[0]) {
          BuildSlider (rot.Col1(), minTrans.x, maxTrans.x);
        } else if (transConstraint[1]) {
          BuildSlider (rot.Col2(), minTrans.y, maxTrans.y);
        } else {
          BuildSlider (rot.Col3(), minTrans.z, maxTrans.z);
        }
        break;
      case 2:
// TODO fill this in with a contact joint
        break;
      case 3:
/* doesn't exist */
        break;
    }
  } else {
    /* too unconstrained, don't create joint */
  }
}

csODEDefaultMoveCallback::csODEDefaultMoveCallback ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csODEDefaultMoveCallback::~csODEDefaultMoveCallback ()
{
}

void csODEDefaultMoveCallback::Execute (iMeshWrapper* mesh,
 csOrthoTransform& t)
{
  mesh->GetMovable ()->SetPosition (t.GetOrigin ());
  mesh->GetMovable ()->GetTransform ().SetT2O (t.GetO2T ());
  mesh->GetMovable ()->UpdateMove ();
  mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
}

void csODEDefaultMoveCallback::Execute (iSkeletonBone* bone,
  csOrthoTransform& t)
{
  bone->SetTransformation (t);
}
