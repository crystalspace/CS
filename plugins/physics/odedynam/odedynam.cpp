/*
    Copyright (C) 2002 Anders Stenberg
    Copyright (C) 2003 Leandro Motta Barros

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
#include "iutil/plugin.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "iutil/virtclk.h"
#include "ivaria/collider.h"
#include "ivaria/reporter.h"
#include "csgeom/polytree.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"
#include "iengine/engine.h"
#include "imesh/object.h"
#include "csgeom/obb.h"
#include "csqsqrt.h"

#include "odedynam.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csODEDynamics)
  SCF_IMPLEMENTS_INTERFACE (iDynamics)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iODEDynamicState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEDynamics::ODEDynamicState)
  SCF_IMPLEMENTS_INTERFACE (iODEDynamicState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEDynamics::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csODEDynamics::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE_EXT (csODEDynamicSystem)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iDynamicSystem)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iODEDynamicSystemState);
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEDynamicSystem::DynamicSystem)
  SCF_IMPLEMENTS_INTERFACE (iDynamicSystem)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEDynamicSystem::ODEDynamicSystemState)
  SCF_IMPLEMENTS_INTERFACE (iODEDynamicSystemState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csODEBodyGroup)
  SCF_IMPLEMENTS_INTERFACE (iBodyGroup)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE_EXT (csODERigidBody)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iRigidBody)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODERigidBody::RigidBody)
  SCF_IMPLEMENTS_INTERFACE (iRigidBody)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csODEJoint)
  SCF_IMPLEMENTS_INTERFACE (iJoint)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iODEJointState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csODEJoint::ODEJointState)
  SCF_IMPLEMENTS_INTERFACE (iODEJointState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


SCF_IMPLEMENT_IBASE (csODEDefaultMoveCallback)
  SCF_IMPLEMENTS_INTERFACE (iDynamicsMoveCallback)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csODEDynamics)


void DestroyGeoms( csGeomList & geoms );


int csODEDynamics::geomclassnum = 0;
dJointGroupID csODEDynamics::contactjoints = dJointGroupCreate (0);

csODEDynamics::csODEDynamics (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiODEDynamicState);
  object_reg = 0;
  scfiEventHandler = 0;
  process_events = false;

  // Initialize the colliders so that the class isn't overwritten
  dGeomID id = dCreateSphere (0, 1);
  dGeomDestroy (id);

  dGeomClass c;
  c.bytes = sizeof (MeshInfo);
  c.collider = &CollideSelector;
  c.aabb = &GetAABB;
  c.aabb_test = 0;
  c.dtor = 0;
  geomclassnum = dCreateGeomClass (&c);

  erp = 0.2f;
  cfm = 1e-5f;

  rateenabled = false;
  steptime = 0.1f;
  limittime = 1.0f;
  total_elapsed = 0.0f;

  stepfast = false;
  sfiter = 10;
  fastobjects = false;
}

csODEDynamics::~csODEDynamics ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
      q->RemoveListener (scfiEventHandler);
  }
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiODEDynamicState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csODEDynamics::Initialize (iObjectRegistry* object_reg)
{
  csODEDynamics::object_reg = object_reg;

  clock = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (!clock)
    return false;

  return true;
}

csPtr<iDynamicSystem> csODEDynamics::CreateSystem ()
{
  csODEDynamicSystem* system = new csODEDynamicSystem (erp, cfm);
  csRef<iDynamicSystem> isystem (SCF_QUERY_INTERFACE (system, iDynamicSystem));
  systems.Push (isystem);
  isystem->DecRef ();
  return csPtr<iDynamicSystem> (isystem);
}

void csODEDynamics::RemoveSystem (iDynamicSystem* system)
{
  systems.Delete (system);
}

iDynamicSystem *csODEDynamics::FindSystem (const char *name)
{
  return systems.FindByName (name);
}

void csODEDynamics::Step (float elapsed_time)
{
  float stepsize;
  if (process_events) 
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "csODEDynamics", 
      "Step was called after event processing was enabled");
    return;
  }
  if (rateenabled) 
  {
  	stepsize = steptime;
	if (elapsed_time > limittime) elapsed_time = limittime;
  } 
  else 
  {
  	stepsize = elapsed_time;
  }
  total_elapsed += elapsed_time;

  // TODO handle fractional total_remaining (interpolate render)
  while (total_elapsed > stepsize) 
  {
    total_elapsed -= stepsize;
    for (size_t i=0; i<systems.Length(); i++)
    {
      systems.Get (i)->Step (stepsize);
      for (size_t j = 0; j < updates.Length(); j ++) 
      {
        updates[i]->Execute (stepsize);
      }
      dJointGroupEmpty (contactjoints);
    }
  }
}

void csODEDynamics::NearCallback (void *data, dGeomID o1, dGeomID o2)
{
  if (dGeomIsSpace(o1) || dGeomIsSpace (o2))
  {
    dSpaceCollide2 (o1, o2, data, &csODEDynamics::NearCallback);
    if (dGeomIsSpace(o1))
      dSpaceCollide ((dxSpace*)o1, data, &csODEDynamics::NearCallback);
    if (dGeomIsSpace(o2))
      dSpaceCollide ((dxSpace*)o2, data, &csODEDynamics::NearCallback);
    return;
  }

  csODERigidBody *b1 = 0, *b2 = 0;
  if (dGeomGetBody(o1))
  {
    b1 = (csODERigidBody *)dBodyGetData (dGeomGetBody(o1));
  }
  if (dGeomGetBody(o2))
  {
    b2 = (csODERigidBody *)dBodyGetData (dGeomGetBody(o2));
  }

  if ((!b1 || b1->IsStatic()) && (!b2 || b2->IsStatic())) return;
  if (b1 && b2 && b1->GetGroup() != 0 && b1->GetGroup() == b2->GetGroup()) 
    return;

  dContact contact[10];
  int a = dCollide (o1, o2, 10, &(contact[0].geom), sizeof (dContact));
  if (a > 0)
  {
    /* there is only 1 actual body per set */
    if (b1)
    {
      b1->Collision ((b2) ? &b2->scfiRigidBody : 0);
    }
    if (b2)
    {
      b2->Collision ((b1) ? &b1->scfiRigidBody : 0);
    }

    for( int i=0; i<a; i++ )
    {
      float *f1 = (float *)dGeomGetData (contact[i].geom.g1);
      float *f2 = (float *)dGeomGetData (contact[i].geom.g2);

      contact[i].surface.mode = dContactBounce | dContactSoftCFM
        | dContactSlip1 | dContactSlip2 | dContactApprox1;
      contact[i].surface.mu = f1[0]*f2[0];
      contact[i].surface.bounce = f1[1]*f2[1];
      contact[i].surface.bounce_vel = 0.1f;
      contact[i].surface.slip1 = SMALL_EPSILON;
      contact[i].surface.slip2 = SMALL_EPSILON;
      contact[i].surface.soft_cfm = f1[2]*f2[2];

      dJointID c = dJointCreateContact ( ((csODEDynamicSystem*)data)
          ->GetWorldID(), contactjoints,contact+i );
      dJointAttach (c, dGeomGetBody(o1), dGeomGetBody(o2));
    }
  }
}

csReversibleTransform GetGeomTransform (dGeomID id)
{
  const dReal* pos = dGeomGetPosition (id);
  const dReal* mat = dGeomGetRotation (id);
  /* Need to use the inverse in this case */
  csMatrix3 rot;
  rot.m11 = mat[0]; rot.m12 = mat[4];   rot.m13 = mat[8];
  rot.m21 = mat[1]; rot.m22 = mat[5];   rot.m23 = mat[9];
  rot.m31 = mat[2]; rot.m32 = mat[6];   rot.m33 = mat[10];
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

/*
  The ODE version contained in the win32libs package defines the dCollide*
  functions as extern "C" (otherwise the DLL couldn't be shared by different
  compilers). For other platforms, it's not the case. For Mingw and Cygwin
  users, this setting will be detected automatically by the CS configure
  script.
 */
#if defined(CS_USE_ODE_EXTERN_C)
#define ODE_EXTERN  extern "C"
#else
#define ODE_EXTERN  extern
#endif
/* defined in ode */
ODE_EXTERN int dCollideBoxPlane (dxGeom *o1, dxGeom *o2, int flags, 
				 dContactGeom *outcontacts, int skip);
ODE_EXTERN int dCollideCCylinderPlane (dxGeom *o1, dxGeom *o2, int flags, 
				       dContactGeom *outcontacts, int skip);
ODE_EXTERN int dCollideRayPlane (dxGeom *o1, dxGeom *o2, int flags, 
				 dContactGeom *contact, int skip);

typedef csDirtyAccessArray<csMeshedPolygon> csPolyMeshList;

int csODEDynamics::CollideMeshBox (dGeomID mesh, dGeomID box, int flags,
    dContactGeom *outcontacts, int skip)
{
  /* need to check aabb's since GeomGroup doesn't */
  dReal aabb1[6];
  dReal aabb2[6];
  dGeomGetAABB (mesh, aabb1);
  dGeomGetAABB (box, aabb2);
  if (aabb1[0] > aabb2[1] || aabb1[1] < aabb2[0] ||
      aabb1[2] > aabb2[3] || aabb1[3] < aabb2[2] ||
      aabb1[4] > aabb2[5] || aabb1[5] < aabb2[4])
  {
    return 0;
  }
  int N = flags & 0xFF;

  // rotate everything so that box is axis aligned.
  dVector3 sides;
  dGeomBoxGetLengths (box, sides);
  // box is symetric.
  csVector3 aabb(sides[0]/2, sides[1]/2, sides[2]/2);
  csBox3 boxbox;
  boxbox.AddBoundingVertex (aabb);
  boxbox.AddBoundingVertex (-aabb);
  csReversibleTransform boxt = GetGeomTransform (box);

  MeshInfo *mi = (MeshInfo*)dGeomGetClassData (mesh);
  iMeshWrapper *m = mi->mesh;
  CS_ASSERT (m);
  iPolygonMesh* p = m->GetMeshObject()->GetObjectModel()
  	->GetPolygonMeshColldet();
  csVector3 *vertex_table = p->GetVertices ();
  csMeshedPolygon *polygon_list = p->GetPolygons ();

  csReversibleTransform mesht = GetGeomTransform (mesh);

  csPolygonTree* tree = mi->tree;
  csArray<int> polyidx;
  csBox3 transformedbox;
  transformedbox.StartBoundingBox (mesht.Other2This(boxbox.GetCorner(0)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(boxbox.GetCorner(1)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(boxbox.GetCorner(2)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(boxbox.GetCorner(3)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(boxbox.GetCorner(4)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(boxbox.GetCorner(5)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(boxbox.GetCorner(6)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(boxbox.GetCorner(7)));
  tree->IntersectBox (polyidx, transformedbox);
  tree->RemoveDoubles (polyidx);

  csPolyMeshList polycollide;
  // test for overlap
  size_t i;
  int j, k;
  for (i = 0; i < polyidx.Length (); i ++)
  {
    csBox3 polybox;
    csMeshedPolygon& poly = polygon_list[polyidx[i]];
    for (j = 0; j < poly.num_vertices; j ++)
    {
      polybox.AddBoundingVertex (boxt * (vertex_table[poly.vertices[j]]/mesht));
    }
    // aabb poly against aabb box for overlap.
    // Full collision later will weed out the rest;

    if (polybox.Overlap (boxbox))
    {
      polycollide.Push (poly);
    }
  }

  int outcount = 0;
  // the value of N should be large, just in case.
  for (i = 0; i < polycollide.Length() && outcount < N; i ++)
  {
    csPlane3 plane(vertex_table[polycollide[i].vertices[0]] / mesht,
      vertex_table[polycollide[i].vertices[1]] / mesht,
      vertex_table[polycollide[i].vertices[2]] / mesht);
    // dCollideBP only works if box center is on the outside of the plane
    if (plane.Classify (boxt.GetOrigin()) < 0)
    {
      continue;
    }
    plane.Normalize ();
    dGeomID odeplane = dCreatePlane(0,plane.norm.x, plane.norm.y, plane.norm.z,
      -plane.DD);
    dContactGeom tempcontacts[5];
    int count = dCollideBoxPlane (box, odeplane, 5, tempcontacts, sizeof (dContactGeom));
    dGeomDestroy (odeplane);
    for (j = 0; j < count; j ++)
    {
      dContactGeom *c = &tempcontacts[j];
      csVector3 contactpos(c->pos[0], c->pos[1], c->pos[2]);
      // make sure the point lies inside the polygon
      int vcount = polycollide[i].num_vertices;
      int ind1 = polycollide[i].vertices[vcount-1];
      csVector3 v1 = vertex_table[ind1] / mesht;
      csVector3 v2;
      for (k = 0; k < vcount; v1 = v2, k++)
      {
        int ind2 = polycollide[i].vertices[k];
        v2 = vertex_table[ind2] / mesht;
        csPlane3 edgeplane(v1, v2, v2 - plane.Normal());
        //edgeplane.Normalize();
        if (edgeplane.Classify (contactpos) < 0)
        {
          c->depth = -1;
          break;
        }
      }
      if (c->depth >= 0)
      {
        dContactGeom *out = (dContactGeom *)((char *)outcontacts + skip * outcount);
        out->pos[0] = c->pos[0];
        out->pos[1] = c->pos[1];
        out->pos[2] = c->pos[2];
        out->normal[0] = -c->normal[0];
        out->normal[1] = -c->normal[1];
        out->normal[2] = -c->normal[2];
        out->depth = c->depth;
        out->g1 = mesh;
        out->g2 = box;
        outcount ++;
      }
    }
  }
  return outcount;
}

int csODEDynamics::CollideMeshCylinder (dGeomID mesh, dGeomID cyl, int flags,
    dContactGeom *outcontacts, int skip)
{
  /* need to check aabb's since GeomGroup doesn't */
  dReal aabb1[6];
  dReal aabb2[6];
  dGeomGetAABB (mesh, aabb1);
  dGeomGetAABB (cyl, aabb2);
  if (aabb1[0] > aabb2[1] || aabb1[1] < aabb2[0] ||
      aabb1[2] > aabb2[3] || aabb1[3] < aabb2[2] ||
      aabb1[4] > aabb2[5] || aabb1[5] < aabb2[4])
  {
    return 0;
  }
  int N = flags & 0xFF;

  // rotate everything so that box is axis aligned.
  dReal length, radius;
  dGeomCCylinderGetParams (cyl, &length, &radius);
  // box is symetric.
  csVector3 aabb(radius, radius, length/2);
  csBox3 cylbox;
  cylbox.AddBoundingVertex (aabb);
  cylbox.AddBoundingVertex (-aabb);
  csReversibleTransform cylt = GetGeomTransform (cyl);

  MeshInfo *mi = (MeshInfo*)dGeomGetClassData (mesh);
  iMeshWrapper *m = mi->mesh;
  CS_ASSERT (m);
  iPolygonMesh* p = m->GetMeshObject()->GetObjectModel()
  	->GetPolygonMeshColldet();
  csVector3 *vertex_table = p->GetVertices ();
  csMeshedPolygon *polygon_list = p->GetPolygons ();

  csReversibleTransform mesht = GetGeomTransform (mesh);

  csPolygonTree* tree = mi->tree;
  csArray<int> polyidx;
  csBox3 transformedbox;
  transformedbox.StartBoundingBox (mesht.Other2This(cylbox.GetCorner(0)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(cylbox.GetCorner(1)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(cylbox.GetCorner(2)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(cylbox.GetCorner(3)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(cylbox.GetCorner(4)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(cylbox.GetCorner(5)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(cylbox.GetCorner(6)));
  transformedbox.AddBoundingVertexSmart (mesht.Other2This(cylbox.GetCorner(7)));
  tree->IntersectBox (polyidx, transformedbox);
  tree->RemoveDoubles (polyidx);

  csPolyMeshList polycollide;
  // test for overlap
  size_t i;
  int j, k;
  for (i = 0; i < polyidx.Length (); i ++)
  {
    csBox3 polybox;
    csMeshedPolygon& poly = polygon_list[polyidx[i]];
    for (j = 0; j < poly.num_vertices; j ++)
    {
      polybox.AddBoundingVertex (cylt * (vertex_table[poly.vertices[j]] / mesht));
    }
    // aabb poly against aabb box for overlap.
    // Full collision later will weed out the rest;
    if (polybox.Overlap (cylbox))
    {
      polycollide.Push (poly);
    }
  }
  int outcount = 0;
  // the value of N should be large, just in case.
  for (i = 0; i < polycollide.Length() && outcount < N; i ++)
  {
    csPlane3 plane(vertex_table[polycollide[i].vertices[0]] / mesht,
      vertex_table[polycollide[i].vertices[1]] / mesht,
      vertex_table[polycollide[i].vertices[2]] / mesht);
    // dCollideBP only works if box center is on the outside of the plane
    if (plane.Classify (cylt.GetOrigin()) < 0)
    {
      continue;
    }
    plane.Normalize ();
    dGeomID odeplane = dCreatePlane(0,plane.norm.x, plane.norm.y, plane.norm.z,
      -plane.DD);
    dContactGeom tempcontacts[5];
    int count = dCollideCCylinderPlane (cyl, odeplane, 5, tempcontacts, sizeof (dContactGeom));
    dGeomDestroy (odeplane);
    for (j = 0; j < count; j ++)
    {
      dContactGeom *c = &tempcontacts[j];
      csVector3 contactpos(c->pos[0], c->pos[1], c->pos[2]);
      // make sure the point lies inside the polygon
      int vcount = polycollide[i].num_vertices;
      int ind1 = polycollide[i].vertices[vcount-1];
      csVector3 v1 = vertex_table[ind1] / mesht;
      csVector3 v2;
      for (k = 0; k < vcount; v1 = v2, k++)
      {
        int ind2 = polycollide[i].vertices[k];
        v2 = vertex_table[ind2] / mesht;
        csPlane3 edgeplane(v1, v2, v2 - plane.Normal());
        //edgeplane.Normalize();
        if (edgeplane.Classify (contactpos) < 0)
        {
          c->depth = -1;
          break;
        }
      }
      if (c->depth >= 0)
      {
        dContactGeom *out = (dContactGeom *)((char *)outcontacts + skip * outcount);
        out->pos[0] = c->pos[0];
        out->pos[1] = c->pos[1];
        out->pos[2] = c->pos[2];
        out->normal[0] = -c->normal[0];
        out->normal[1] = -c->normal[1];
        out->normal[2] = -c->normal[2];
        out->depth = c->depth;
        out->g1 = mesh;
        out->g2 = cyl;
        outcount ++;
      }
    }
  }
  return outcount;
}

int csODEDynamics::CollideMeshSphere (dGeomID mesh, dGeomID sphere, int flags,
    dContactGeom *outcontacts, int skip)
{
  /* need to check aabb's since GeomGroup doesn't */
  dReal aabb1[6];
  dReal aabb2[6];
  dGeomGetAABB (mesh, aabb1);
  dGeomGetAABB (sphere, aabb2);
  if (aabb1[0] > aabb2[1] || aabb1[1] < aabb2[0] ||
      aabb1[2] > aabb2[3] || aabb1[3] < aabb2[2] ||
      aabb1[4] > aabb2[5] || aabb1[5] < aabb2[4])
  {
    return 0;
  }
  int N = flags & 0xFF;
  const dReal *pos = dGeomGetPosition (sphere);
  csVector3 center(pos[0], pos[1], pos[2]);
  dReal rad = dGeomSphereGetRadius (sphere);
  MeshInfo *mi = (MeshInfo *)dGeomGetClassData (mesh);
  iMeshWrapper *m = mi->mesh;
  CS_ASSERT (m);
  iPolygonMesh* p = m->GetMeshObject()->GetObjectModel()
  	->GetPolygonMeshColldet();
  csVector3 *vertex_table = p->GetVertices ();
  csMeshedPolygon *polygon_list = p->GetPolygons ();

  csReversibleTransform mesht = GetGeomTransform (mesh);

  int outcount = 0;

  csPolygonTree* tree = mi->tree;
  csArray<int> polyidx;
  tree->IntersectSphere (polyidx, mesht.Other2This (center), rad*rad);
  tree->RemoveDoubles (polyidx);
  for (size_t i = 0; i < polyidx.Length () && outcount < N; i ++)
  {
    csMeshedPolygon& poly = polygon_list[polyidx[i]];
    csPlane3 plane(vertex_table[poly.vertices[0]] / mesht,
      vertex_table[poly.vertices[1]] / mesht,
      vertex_table[poly.vertices[2]] / mesht);
    if (plane.Classify (center) < 0)
    {
      continue;
    }
    plane.Normalize();
    float depth = rad - plane.Distance (center);
    if (depth < 0)
    {
      continue;
    }
    int vcount = poly.num_vertices;
    int ind1 = poly.vertices[vcount-1];
    csVector3 v1 = vertex_table[ind1] / mesht;
    csVector3 v2;
    for (int j = 0; j < vcount; v1 = v2, j++)
    {
      int ind2 = poly.vertices[j];
      v2 = vertex_table[ind2] / mesht;
      csPlane3 edgeplane(v1, v2, v2 - plane.Normal());
      //edgeplane.Normalize();
      if (edgeplane.Classify (center) < SMALL_EPSILON)
      {
        csVector3 line = v2 - v1;
	float linelen = line.SquaredNorm();
        line.Normalize();
        float proj = center * line;
        /* if the point projects on this edge, but outside the poly test
         * for depth to this edge (especially important on sharp corners)
         */
        if ((proj >= (v1 * line)) && (proj <= (v2 * line)))
        {
          float t = ((v1 - center) * (v2 - center)) / linelen;
          csVector3 projpt = v1 + (line * t);
          csVector3 newnorm = center - projpt;
          float dist = newnorm.Norm();
          depth = rad - dist;
          newnorm /= dist;
          plane.Set (newnorm.x, newnorm.y, newnorm.z, 0);
          break;
        }
        else
        {
          /* this is an invalid the ball is outside the poly at this edge */
          depth = -1;
        }
      }
    }
    if (depth < 0)
    {
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
    out->g1 = mesh;
    out->g2 = sphere;
    outcount ++;
  }
  return outcount;
}

int csODEDynamics::CollideMeshPlane (dGeomID mesh, dGeomID plane, int flags,
    dContactGeom *outcontacts, int skip)
{
  int N = flags & 0xFF;
  
  //Make a bounding Box
  MeshInfo *mi = (MeshInfo*)dGeomGetClassData (mesh);
  iMeshWrapper *m = mi->mesh;
  CS_ASSERT (m);
  iPolygonMesh* p = m->GetMeshObject()->GetObjectModel()
        ->GetPolygonMeshColldet();
  csVector3 *vertex_table = p->GetVertices ();
  csMeshedPolygon *polygon_list = p->GetPolygons ();

  csReversibleTransform mesht = GetGeomTransform (mesh);
 
  int i;
  csBox3 box;
  box.StartBoundingBox ();
  for (i = 0; i < p->GetVertexCount(); i ++)
  {
    box.AddBoundingVertex (vertex_table[i] / mesht);
  }

  //Fast Check
  
  csVector3 current;
  int j,k;
  bool planecut = false;
  current = box.GetCorner(0);
  float last = dGeomPlanePointDepth(plane,current.x,current.y,current.z);
  for(i=1;i<8;i++)
  {
    current = box.GetCorner(i);
    float now = dGeomPlanePointDepth(plane,current.x,current.y,current.z);
    if((now<0.0f && last>0.0f)||(now>0.0f && last<0.0f))
    {
      planecut = true;
      break;
    }
    last = now;
  }
  
  if(!planecut)
  {
    return 0;
  }
  
  dVector4 result;
  dGeomPlaneGetParams (plane, result);
  
  //Serious Check
  int outcount = 0;
  int polycount = p->GetPolygonCount();
  for(i=0;i<polycount && outcount < N;i++)
  {
    int vcount = polygon_list[i].num_vertices;
    //Checking if the poly is cut
    planecut = false;
    current = vertex_table[polygon_list[i].vertices[0]]/mesht;
    last = dGeomPlanePointDepth(plane,current.x,current.y,current.z);
    for (j = 0; j < vcount; j++)
    {
      current = vertex_table[polygon_list[i].vertices[j]]/mesht;
      float now = dGeomPlanePointDepth(plane,current.x,current.y,current.z);
      if((now<0.0f && last>0.0f)||(now>0.0f && last<0.0f))
      {
        planecut = true;
        break;
      }
      last = now;
    }
    if(planecut)
    {
      //Determining the places where the poly is cut
      int ind1 = polygon_list[i].vertices[vcount-1];
      csVector3 v1 = vertex_table[ind1] / mesht;
      csVector3 v2;
      for (j = 0; j < vcount && outcount < N; v1 = v2, j++)
      {
        int ind2 = polygon_list[i].vertices[j];
        v2 = vertex_table[ind2] / mesht;
        csVector3 diff = v2-v1;
        
        dGeomID oderay = dCreateRay (0, diff.Norm());
        dGeomRaySet (oderay, v1.x, v1.y, v1.z,
                    diff.x, diff.y, diff.z);
        dContactGeom tempcontacts[1];
        int count = dCollideRayPlane (oderay, plane, 1, tempcontacts, sizeof (dContactGeom));
        dGeomDestroy (oderay);
        if(count==1)
        {
          dContactGeom *c = &tempcontacts[0];
          bool doubles = false;
          for(k=0;k<outcount;k++)
          {
            dContactGeom *kc = (dContactGeom *)((char *)outcontacts + skip * k);
            if(kc->pos[0] == c->pos[0] &&
               kc->pos[1] == c->pos[1] &&
               kc->pos[2] == c->pos[2])
            {
              doubles = true;
              break;
            }
          }
          if(!doubles)
          {
            dContactGeom *out = (dContactGeom *)((char *)outcontacts + skip * outcount);
            out->pos[0] = c->pos[0];
            out->pos[1] = c->pos[1];
            out->pos[2] = c->pos[2];
            out->normal[0] = result[0];
            out->normal[1] = result[1];
            out->normal[2] = result[2];
            float depth1 = dGeomPlanePointDepth(plane,v1.x,v1.y,v1.z);
            float depth2 = dGeomPlanePointDepth(plane,v2.x,v2.y,v2.z);
            if(depth1>0.0f)
              out->depth = depth1;
            else if (depth2 > 0.0f)
              out->depth = depth2;
            else
              out->depth = 0.0f;
            out->g1 = mesh;
            out->g2 = plane;
            outcount ++;
          }
        }
      }
    }
  }

  return outcount;
}

dColliderFn* csODEDynamics::CollideSelector (int num)
{
  if (num == geomclassnum) return (dColliderFn *)&CollideMeshMesh;
  if (num == dBoxClass) return (dColliderFn *)&CollideMeshBox;
  if (num == dCCylinderClass) return (dColliderFn *)&CollideMeshCylinder;
  if (num == dSphereClass) return (dColliderFn *)&CollideMeshSphere;
  if (num == dPlaneClass) return (dColliderFn *)&CollideMeshPlane;
  return 0;
}

void csODEDynamics::GetAABB (dGeomID g, dReal aabb[6])
{
  csBox3 box;
  csReversibleTransform mesht = GetGeomTransform (g);
  MeshInfo *mi = (MeshInfo *)dGeomGetClassData (g);
  iMeshWrapper *m = mi->mesh;
  iPolygonMesh* p = m->GetMeshObject()->GetObjectModel()->GetPolygonMeshColldet();
  csVector3 *vertex_table = p->GetVertices ();
  box.StartBoundingBox ();
  for (int i = 0; i < p->GetVertexCount(); i ++)
  {
    box.AddBoundingVertex (vertex_table[i] / mesht);
  }
  aabb[0] = box.MinX(); aabb[1] = box.MaxX();
  aabb[2] = box.MinY(); aabb[3] = box.MaxY();
  aabb[4] = box.MinZ(); aabb[5] = box.MaxZ();
}

void csODEDynamics::SetGlobalERP (float erp)
{
  csODEDynamics::erp = erp;

  for (size_t i = 0; i < systems.Length(); i ++) 
  {
    csRef<iODEDynamicSystemState> sys = SCF_QUERY_INTERFACE (systems[i],
	   iODEDynamicSystemState);
  	 sys->SetERP (erp);
  }
}

void csODEDynamics::SetGlobalCFM (float cfm)
{
  csODEDynamics::cfm = cfm;
  for (size_t i = 0; i < systems.Length(); i ++) 
  {
    csRef<iODEDynamicSystemState> sys = SCF_QUERY_INTERFACE (systems[i],
      iODEDynamicSystemState);
    sys->SetCFM (cfm);
  }
}

void csODEDynamics::EnableStepFast (bool enable)
{
  stepfast = enable;

  for (size_t i = 0; i < systems.Length(); i ++) 
  {
    csRef<iODEDynamicSystemState> sys = SCF_QUERY_INTERFACE (systems[i],
	   iODEDynamicSystemState);
  	 sys->EnableStepFast (enable);
  }
}

void csODEDynamics::SetStepFastIterations (int iter)
{
  sfiter = iter;

  for (size_t i = 0; i < systems.Length(); i ++) 
  {
    csRef<iODEDynamicSystemState> sys = SCF_QUERY_INTERFACE (systems[i],
      iODEDynamicSystemState);
    sys->SetStepFastIterations (iter);
  }
}

void csODEDynamics::EnableEventProcessing (bool enable)
{
  if (enable && !process_events)
  {
    process_events = true;

    if (!scfiEventHandler) 
      scfiEventHandler = csPtr<EventHandler> (new EventHandler (this));
    csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
      q->RegisterListener (scfiEventHandler, CSMASK_Nothing);
  }
  else if (!enable && process_events)
  {
    process_events = false;

    if (scfiEventHandler)
    {
      csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
      if (q)
        q->RemoveListener (scfiEventHandler);
      scfiEventHandler = 0;
    }
  }
}

bool csODEDynamics::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast && Event.Command.Code == cscmdPreProcess)
  {
    float stepsize = steptime;
    float elapsed_time = ((float)clock->GetElapsedTicks ())/1000.0;
    if (elapsed_time > limittime) elapsed_time = limittime;
    total_elapsed += elapsed_time;

    // TODO handle fractional total_remaining (interpolate render)
    while (total_elapsed > stepsize) 
    {
      total_elapsed -= stepsize;
      for (size_t i=0; i<systems.Length(); i++)
      {
        systems.Get (i)->Step (stepsize);
        for (size_t j = 0; j < updates.Length(); j ++) 
        {
          updates[i]->Execute (stepsize);
        }
        dJointGroupEmpty (contactjoints);
      }
    }
    return true;
  }
  return false;
}

csODEDynamicSystem::csODEDynamicSystem (float erp, float cfm)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDynamicSystem);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiODEDynamicSystemState);

  //TODO: QUERY for collidesys

  worldID = dWorldCreate ();
  spaceID = dHashSpaceCreate (0);
  dWorldSetERP (worldID, erp);
  dWorldSetCFM (worldID, cfm);
  roll_damp = 1.0;
  lin_damp = 1.0;
  move_cb = (iDynamicsMoveCallback*)new csODEDefaultMoveCallback ();

  rateenabled = false;
  steptime = limittime = total_elapsed = 0.0;

  stepfast = false;
  sfiter = 10;
  fastobjects = false;
}

csODEDynamicSystem::~csODEDynamicSystem ()
{
  // destroy all static collider geoms
  DestroyGeoms (geoms);

  // must delete all these before deleting the actual world
  joints.DeleteAll ();
  groups.DeleteAll ();
  bodies.DeleteAll ();

  dSpaceDestroy (spaceID);
  dWorldDestroy (worldID);
  if (move_cb) move_cb->DecRef ();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiODEDynamicSystemState);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDynamicSystem);
}


csPtr<iRigidBody> csODEDynamicSystem::CreateBody ()
{
  csODERigidBody* body = new csODERigidBody (this);
  bodies.Push (&body->scfiRigidBody);
  body->scfiRigidBody.SetMoveCallback(move_cb);
  return &body->scfiRigidBody;
}


void csODEDynamicSystem::RemoveBody (iRigidBody* body)
{
  bodies.Delete (body);
}

iRigidBody *csODEDynamicSystem::FindBody (const char *name)
{
  return bodies.FindByName (name);
}

csPtr<iBodyGroup> csODEDynamicSystem::CreateGroup ()
{
  csODEBodyGroup* group = new csODEBodyGroup (this);
  groups.Push (group);
  return csPtr<iBodyGroup> (group);
}

void csODEDynamicSystem::RemoveGroup (iBodyGroup *group)
{
  groups.Delete ((csODEBodyGroup*)group);
}

csPtr<iJoint> csODEDynamicSystem::CreateJoint ()
{
  csODEJoint* joint = new csODEJoint (this);
  joints.Push (joint);
  return csPtr<iJoint> (joint);
}

void csODEDynamicSystem::RemoveJoint (iJoint *joint)
{
  joints.Delete ((csODEJoint*)joint);
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

void csODEDynamicSystem::Step (float elapsed_time)
{
  dSpaceCollide (spaceID, this, &csODEDynamics::NearCallback);
  float stepsize;
  if (rateenabled) 
  {
  	stepsize = steptime;
	if (elapsed_time > limittime) { elapsed_time = limittime; }
  } 
  else 
  {
  	stepsize = elapsed_time;
  }
  total_elapsed += elapsed_time;

  // TODO handle fractional total_remaining (interpolate render)
  while (total_elapsed > stepsize) 
  {
    total_elapsed -= stepsize;
    if (!stepfast) 
    {
      dWorldStep (worldID, stepsize);
    } 
    else 
    {
      dWorldStepFast1 (worldID, stepsize, sfiter);
    }
    for (size_t i = 0; i < bodies.Length(); i ++) 
    {
        iRigidBody *b = bodies.Get(i);
        b->SetAngularVelocity (b->GetAngularVelocity () * roll_damp);
        b->SetLinearVelocity (b->GetLinearVelocity () * lin_damp);
    }
    for (size_t j = 0; j < updates.Length(); j ++) 
    {
      updates[j]->Execute (stepsize);
    }
  }

  for (size_t i=0; i<bodies.Length(); i++)
  {
    iRigidBody *b = bodies.Get(i);
    b->Update ();
  }
}

bool csODEDynamicSystem::AttachColliderMesh (iMeshWrapper* mesh,
  	const csOrthoTransform& trans, float friction, float elasticity, float softness)
{
  dGeomID id = dCreateGeom (csODEDynamics::GetGeomClassNum());
  geoms.Push(id);

  MeshInfo *gdata = (MeshInfo*)dGeomGetClassData (id);
  gdata->mesh = mesh;
  gdata->tree = new csPolygonTree ();
  iPolygonMesh* p = mesh->GetMeshObject()->GetObjectModel()
  	->GetPolygonMeshColldet();
  gdata->tree->Build (p);
  dSpaceAdd (spaceID, id);

  dMatrix3 mat;
  mat[0] = trans.GetO2T().m11; mat[1] = trans.GetO2T().m12; mat[2] = trans.GetO2T().m13; mat[3] = 0;
  mat[4] = trans.GetO2T().m21; mat[5] = trans.GetO2T().m22; mat[6] = trans.GetO2T().m23; mat[7] = 0;
  mat[8] = trans.GetO2T().m31; mat[9] = trans.GetO2T().m32; mat[10] = trans.GetO2T().m33; mat[11] = 0;
  dGeomSetRotation (id, mat);

  dGeomSetPosition (id,
    trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (id, (void*)f);

  return true;
}

bool csODEDynamicSystem::AttachColliderCylinder (float length, float radius,
  	const csOrthoTransform& trans, float friction, float elasticity, float softness)
{
  dGeomID id = dCreateCCylinder (spaceID, radius, length);  

  dMatrix3 mat;
  mat[0] = trans.GetO2T().m11; mat[1] = trans.GetO2T().m12; mat[2] = trans.GetO2T().m13; mat[3] = 0;
  mat[4] = trans.GetO2T().m21; mat[5] = trans.GetO2T().m22; mat[6] = trans.GetO2T().m23; mat[7] = 0;
  mat[8] = trans.GetO2T().m31; mat[9] = trans.GetO2T().m32; mat[10] = trans.GetO2T().m33; mat[11] = 0;
  dGeomSetRotation (id, mat);

  dGeomSetPosition (id,
    trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (id, (void*)f);

  return true;
}

bool csODEDynamicSystem::AttachColliderBox (const csVector3 &size,
  	const csOrthoTransform& trans, float friction, float elasticity, float softness)
{
  dGeomID id = dCreateBox (spaceID, size.x, size.y, size.z);

  dMatrix3 mat;
  mat[0] = trans.GetO2T().m11; mat[1] = trans.GetO2T().m12; mat[2] = trans.GetO2T().m13; mat[3] = 0;
  mat[4] = trans.GetO2T().m21; mat[5] = trans.GetO2T().m22; mat[6] = trans.GetO2T().m23; mat[7] = 0;
  mat[8] = trans.GetO2T().m31; mat[9] = trans.GetO2T().m32; mat[10] = trans.GetO2T().m33; mat[11] = 0;
  dGeomSetRotation (id, mat);

  dGeomSetPosition (id,
    trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (id, (void*)f);

  return true;
}

bool csODEDynamicSystem::AttachColliderSphere (float radius, 
    const csVector3 &offset, float friction, float elasticity, float softness)
{
  dGeomID id = dCreateSphere (spaceID, radius);

  dGeomSetPosition (id, offset.x, offset.y, offset.z);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (id, (void*)f);

  return true;
}
bool csODEDynamicSystem::AttachColliderPlane (const csPlane3 &plane, 
    float friction, float elasticity, float softness)
{
  dGeomID id = dCreatePlane (spaceID, -plane.A(), -plane.B(), -plane.C(), plane.D());

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (id, (void*)f);

  return true;
}

csODEBodyGroup::csODEBodyGroup (csODEDynamicSystem* sys)
{
  SCF_CONSTRUCT_IBASE (0);
  system = sys;
}

csODEBodyGroup::~csODEBodyGroup ()
{
  for (size_t i = 0; i < bodies.Length(); i ++)
  {
    ((csODERigidBody *)(iRigidBody*)bodies[i])->UnsetGroup ();
  }
  SCF_DESTRUCT_IBASE();
}

void csODEBodyGroup::AddBody (iRigidBody *body)
{
  body->IncRef ();
  bodies.Push (body);
  ((csODERigidBody *)(body->QueryObject()))->SetGroup (this);
}

void csODEBodyGroup::RemoveBody (iRigidBody *body)
{
  bodies.Delete (body);
  ((csODERigidBody *)(body->QueryObject()))->UnsetGroup ();
}

bool csODEBodyGroup::BodyInGroup (iRigidBody *body)
{
  return bodies.Find (body) != csArrayItemNotFound;
}

csODERigidBody::csODERigidBody (csODEDynamicSystem* sys) : geoms (1,4)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiRigidBody);

  dynsys = sys;

  bodyID = dBodyCreate (dynsys->GetWorldID());
  dBodySetData (bodyID, this);
  groupID = dSimpleSpaceCreate (dynsys->GetSpaceID ());
  statjoint = 0;
  collision_group = 0;

  mesh = 0;
  move_cb = 0;
  coll_cb = 0;
}

csODERigidBody::~csODERigidBody ()
{
  DestroyGeoms (geoms);

  if (move_cb) move_cb->DecRef ();
  if (coll_cb) coll_cb->DecRef ();
  dSpaceDestroy (groupID);
  dBodyDestroy (bodyID);

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiRigidBody);
}


void DestroyGeoms( csGeomList & geoms )
{
  dGeomID tempID;
  size_t i=0;

  for (;i < geoms.Length(); i++)
  {
    tempID = geoms[i];
    if (dGeomGetClass (geoms[i]) == dGeomTransformClass)
      tempID = dGeomTransformGetGeom (geoms[i]);

    float *properties = (float *)dGeomGetData (tempID);
    delete [] properties;

    if( dGeomGetClass (tempID) == csODEDynamics::GetGeomClassNum() )
    {
      MeshInfo *gdata = (MeshInfo*)dGeomGetClassData (tempID);
      delete gdata->tree;
    }

    //for transform geoms, only need to destroy the container,
    //they've been set herein to destroy their contained geom
    dGeomDestroy (geoms[i]);
  }
}


bool csODERigidBody::MakeStatic ()
{
  if (statjoint == 0)
  {
    statjoint = dJointCreateFixed (dynsys->GetWorldID(), 0);
    dJointAttach (statjoint, bodyID, 0);
    dJointSetFixed(statjoint);
    dBodySetGravityMode (bodyID, 0);
  }
  return true;
}

bool csODERigidBody::MakeDynamic ()
{
  if (statjoint != 0)
  {
    dJointDestroy (statjoint);
    dBodySetGravityMode (bodyID, 1);
    statjoint = 0;
  }
  return true;
}

void csODERigidBody::SetGroup(iBodyGroup *group)
{
  if (collision_group)
  {
    collision_group->RemoveBody (&scfiRigidBody);
  }
  collision_group = group;
}

bool csODERigidBody::AttachColliderMesh (iMeshWrapper *mesh,
    const csOrthoTransform &trans, float friction, float density,
    float elasticity, float softness)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (0);
  dGeomTransformSetCleanup (id, 1);
  geoms.Push(id);

  dGeomID gid = dCreateGeom (csODEDynamics::GetGeomClassNum());
  MeshInfo *gdata = (MeshInfo*)dGeomGetClassData (gid);
  gdata->mesh = mesh;
  gdata->tree = new csPolygonTree ();
  iPolygonMesh* p = mesh->GetMeshObject()->GetObjectModel()
  	->GetPolygonMeshColldet();
  gdata->tree->Build (p);
  dGeomTransformSetGeom (id, gid);

  csOBB b;
  b.FindOBB (p->GetVertices(), p->GetVertexCount());
  dMassSetBox (&m, density, b.MaxX()-b.MinX(), b.MaxY()-b.MinY(), b.MaxZ()-b.MinZ());

  dMatrix3 mat;
  mat[0] = b.GetMatrix().m11; mat[1] = b.GetMatrix().m12; mat[2] = b.GetMatrix().m13;
  mat[3] = b.GetMatrix().m21; mat[4] = b.GetMatrix().m22; mat[5] = b.GetMatrix().m23;
  mat[6] = b.GetMatrix().m31; mat[7] = b.GetMatrix().m32; mat[8] = b.GetMatrix().m33;
  dMassRotate (&m, mat);

  mat[0] = trans.GetO2T().m11; mat[1] = trans.GetO2T().m12; mat[2] = trans.GetO2T().m13; mat[3] = 0;
  mat[4] = trans.GetO2T().m21; mat[5] = trans.GetO2T().m22; mat[6] = trans.GetO2T().m23; mat[7] = 0;
  mat[8] = trans.GetO2T().m31; mat[9] = trans.GetO2T().m32; mat[10] = trans.GetO2T().m33; mat[11] = 0;
  dGeomSetRotation (gid, mat);

  dGeomSetPosition (gid,
    trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dMassTranslate (&m,
    trans.GetOrigin().x, trans.GetOrigin().y, trans.GetOrigin().z);
  dBodyGetMass (bodyID, &om);
  dMassAdd (&om, &m);

  dBodySetMass (bodyID, &om);

  dGeomSetBody (id, bodyID);
  dSpaceAdd (groupID, id);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (gid, (void*)f);

  return true;
}

bool csODERigidBody::AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (0);
  dGeomTransformSetCleanup (id, 1);
  geoms.Push(id);

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
  dSpaceAdd (groupID, id);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (gid, (void*)f);

  return true;
}

bool csODERigidBody::AttachColliderBox (const csVector3 &size,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (0);
  dGeomTransformSetCleanup (id, 1);
  geoms.Push(id);

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
  dSpaceAdd (groupID, id);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (gid, (void*)f);

  return true;
}

bool csODERigidBody::AttachColliderSphere (float radius, const csVector3 &offset,
    float friction, float density, float elasticity, float softness)
{
  dMass m, om;
  dMassSetZero (&m);

  dGeomID id = dCreateGeomTransform (0);
  dGeomTransformSetCleanup (id, 1);
  geoms.Push(id);

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
  dSpaceAdd (groupID, id);

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (gid, (void*)f);

  return true;
}

bool csODERigidBody::AttachColliderPlane (const csPlane3& plane,
  float friction, float density, float elasticity, float softness)
{
  dSpaceID space = dynsys->GetSpaceID();
  dGeomID id = dCreatePlane (space, -plane.A(), -plane.B(), -plane.C(), plane.D());

  //causes non=placeable geom run-time error w/debug build of ode.
  //dGeomSetBody (id, bodyID); 

  float *f = new float[3];
  f[0] = friction;
  f[1] = elasticity;
  f[2] = softness;
  dGeomSetData (id, (void*)f);

  return true;
}

void csODERigidBody::SetPosition (const csVector3& pos)
{
    dBodySetPosition (bodyID, pos.x, pos.y, pos.z);
    if (statjoint != 0) dJointSetFixed(statjoint);
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
  if (mass != 0) *mass = m.mass;
  if (center != 0) center->Set (m.c[0], m.c[1], m.c[2]);
  if (inertia != 0)
  {
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
  if (coll_cb) coll_cb->Execute (&scfiRigidBody, other);
}

void csODERigidBody::Update ()
{
  if (bodyID && !statjoint && move_cb)
  {
    csOrthoTransform trans;
    trans = GetTransform ();
    if (mesh) move_cb->Execute (mesh, trans);
    /* remainder case for all other callbacks */
    move_cb->Execute (trans);
  }
}

csODEJoint::csODEJoint (csODEDynamicSystem *sys)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiODEJointState);

  jointID = 0;

  body[0] = body[1] = 0;
  bodyID[0] = bodyID[1] = 0;

  transConstraint[0] = 1;
  transConstraint[1] = 1;
  transConstraint[2] = 1;
  rotConstraint[0] = 1;
  rotConstraint[1] = 1;
  rotConstraint[2] = 1;

  dynsys = sys;
}

csODEJoint::~csODEJoint ()
{
  if (jointID)
    dJointDestroy (jointID);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiODEJointState);
  SCF_DESTRUCT_IBASE();
}

void csODEJoint::Attach (iRigidBody *b1, iRigidBody *b2)
{
  if (b1)
  {
    bodyID[0] = ((csODERigidBody *)(b1->QueryObject()))->GetID();
  }
  else
  {
    bodyID[0] = 0;
  }
  if (b2)
  { 
    bodyID[1] = ((csODERigidBody *)(b2->QueryObject()))->GetID();
  }
  else
  {
    bodyID[1] = 0;
  }
  body[0] = b1;
  body[1] = b2;
  BuildJoint ();
}

csRef<iRigidBody> csODEJoint::GetAttachedBody (int b)
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
  if (max > min)
  {
    dJointSetHingeParam (jointID, dParamLoStop, min);
    dJointSetHingeParam (jointID, dParamHiStop, max);
  }
  else
  {
    dJointSetHingeParam (jointID, dParamLoStop, -dInfinity);
    dJointSetHingeParam (jointID, dParamHiStop, dInfinity);
  }
}

void csODEJoint::BuildHinge2 (const csVector3 &axis1, float min1, float max1,
    const csVector3 &axis2, float min2, float max2)
{
  dJointSetHinge2Axis1 (jointID, axis1.x, axis1.y, axis1.z);
  dJointSetHinge2Axis2 (jointID, axis2.x, axis2.y, axis2.z);
  if (max1 > min1)
  {
    dJointSetHinge2Param (jointID, dParamLoStop, min1);
    dJointSetHinge2Param (jointID, dParamHiStop, max1);
  }
  else
  {
    dJointSetHinge2Param (jointID, dParamLoStop, -dInfinity);
    dJointSetHinge2Param (jointID, dParamHiStop, dInfinity);
  }
  if (max2 > min2)
  {
    dJointSetHinge2Param (jointID, dParamLoStop2, min2);
    dJointSetHinge2Param (jointID, dParamHiStop2, max2);
  }
  else
  {
    dJointSetHinge2Param (jointID, dParamLoStop2, -dInfinity);
    dJointSetHinge2Param (jointID, dParamHiStop2, dInfinity);
  }
}

void csODEJoint::BuildSlider (const csVector3 &axis, float min, float max)
{
  dJointSetSliderAxis (jointID, axis.x, axis.y, axis.z);
  if (max > min)
  {
    dJointSetSliderParam (jointID, dParamLoStop, min);
    dJointSetSliderParam (jointID, dParamHiStop, max);
  }
  else
  {
    dJointSetSliderParam (jointID, dParamLoStop, -dInfinity);
    dJointSetSliderParam (jointID, dParamHiStop, dInfinity);
  }
}

void csODEJoint::SetBounce (const csVector3 & bounce) 
{
  stopBounce = bounce;
  ApplyJointProperty (dParamBounce, stopBounce);
}


// parameter: one of ODE joint parameters.  
// values: up to three possible values for up to 3 possible axis
// For slider joints, property must correspond to axis with 
// translational constraint.  For hinges, the first element is used.
// for 2 axis 'steering' type joints, the first 2 elements are used.
// for ball and socket joints and angular motors, all three elements 
// are used (NYI).

void csODEJoint::ApplyJointProperty (int parameter, csVector3 & values)
{
  int jointType = dJointGetType (jointID);
  switch(jointType)
  {
    case dJointTypeHinge:
      dJointSetHingeParam (jointID, parameter, values.x);
      break;
    case dJointTypeSlider:
      if (transConstraint[0])        
        dJointSetSliderParam (jointID, parameter, values.x);
      else if (transConstraint[1])
        dJointSetSliderParam (jointID, parameter, values.y);
      else 
        dJointSetSliderParam (jointID, parameter, values.z);
      break;
    case dJointTypeHinge2:
        //looks like axis 2 is meant to be axle, 
        //axis 1 is steering, I may need to check that later though.
        dJointSetHinge2Param (jointID, parameter, values.x);
        dJointSetHinge2Param (jointID, parameter + dParamGroup, values.y);
                                   //dParamXi = dParamX + dParamGroup * (i-1)
    default:
    //case dJointTypeBall:       // maybe supported later via AMotor
    //case dJointTypeAMotor:     // not supported here
    //case dJointTypeUniversal:  // not sure if that's supported in here
      break;
  }
}

csVector3 csODEJoint::GetBounce ()
{
  return stopBounce;
}

void csODEJoint::SetDesiredVelocity (const csVector3 & velocity)
{
  desiredVelocity = velocity;
  ApplyJointProperty (dParamVel, desiredVelocity);
}

csVector3 csODEJoint::GetDesiredVelocity ()
{
  return desiredVelocity;
}

void csODEJoint::SetMaxForce (const csVector3 & maxForce)
{
  fMax = maxForce;
  ApplyJointProperty (dParamFMax, fMax);
}

csVector3 csODEJoint::GetMaxForce ()
{
  return fMax;
}

void csODEJoint::BuildJoint ()
{
  if (!(bodyID[0] || bodyID[1]))
  {
    return;
  }
  if (jointID)
  {
    dJointDestroy (jointID);
  }
  int transcount = transConstraint[0] + transConstraint[1] + transConstraint[2];
  int rotcount = rotConstraint[0] + rotConstraint[1] + rotConstraint[2];

  csVector3 pos;
  csMatrix3 rot;
  if (transcount == 0)
  {
    switch (rotcount)
    {
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
        if (rotConstraint[0])
        {
          BuildHinge (rot.Col1(), minAngle.x, maxAngle.x);
        }
        else if (rotConstraint[1])
        {
          BuildHinge (rot.Col2(), minAngle.y, maxAngle.y);
        }
        else if (rotConstraint[2])
        {
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

        if (rotConstraint[0])
        {
          if (rotConstraint[1])
          {
            BuildHinge2 (rot.Col2(), minAngle.y, maxAngle.y,
              rot.Col1(), minAngle.x, maxAngle.x);
          }
          else
          {
            BuildHinge2 (rot.Col3(), minAngle.z, maxAngle.z,
              rot.Col1(), minAngle.x, maxAngle.x);
          }
        }
        else
        {
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
  else if (rotcount == 0)
  {
    switch (transcount)
    {
      /* 0 is accounted for in the previous condition */
      case 1:
        jointID = dJointCreateSlider (dynsys->GetWorldID(), 0);
          dJointAttach (jointID, bodyID[0], bodyID[1]);
        rot = transform.GetO2T();
        if (transConstraint[0])
        {
          BuildSlider (rot.Col1(), minTrans.x, maxTrans.x);
        }
        else if (transConstraint[1])
        {
          BuildSlider (rot.Col2(), minTrans.y, maxTrans.y);
        }
        else
        {
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

void csODEJoint::ODEJointState::SetParam (int parameter, float value)
{
  switch(GetType())
  {
    case CS_ODE_JOINT_TYPE_HINGE:
      dJointSetHingeParam (scfParent->jointID, parameter, value);
      break;
    case CS_ODE_JOINT_TYPE_SLIDER:
      dJointSetSliderParam (scfParent->jointID, parameter, value);
      break;
    case CS_ODE_JOINT_TYPE_HINGE2:
      dJointSetHinge2Param (scfParent->jointID, parameter, value);
      break;
    case CS_ODE_JOINT_TYPE_AMOTOR:
      dJointSetAMotorParam (scfParent->jointID, parameter, value);
      break;
    default:
      ; // do nothing
  }
}

float csODEJoint::ODEJointState::GetParam (int parameter)
{
  switch(GetType())
  {
    case CS_ODE_JOINT_TYPE_HINGE:
      return dJointGetHingeParam (scfParent->jointID, parameter);
    case CS_ODE_JOINT_TYPE_SLIDER:
      return dJointGetSliderParam (scfParent->jointID, parameter);
    case CS_ODE_JOINT_TYPE_HINGE2:
      return dJointGetHinge2Param (scfParent->jointID, parameter);
    case CS_ODE_JOINT_TYPE_AMOTOR:
      return dJointGetAMotorParam (scfParent->jointID, parameter);
    default:
      return 0.0; // this is not a good... the error is ignored silently...
  }
}

void csODEJoint::ODEJointState::SetHinge2Axis1 (const csVector3& axis)
{
  if (GetType() == CS_ODE_JOINT_TYPE_HINGE2)
  {
    dJointSetHinge2Axis1 (scfParent->jointID, axis[0], axis[1], axis[2]);
  }
}

void csODEJoint::ODEJointState::SetHinge2Axis2 (const csVector3& axis)
{
  if (GetType() == CS_ODE_JOINT_TYPE_HINGE2)
  {
    dJointSetHinge2Axis2 (scfParent->jointID, axis[0], axis[1], axis[2]);
  }
}

void csODEJoint::ODEJointState::SetHinge2Anchor (const csVector3& point)
{
  if (GetType() == CS_ODE_JOINT_TYPE_HINGE2)
  {
    dJointSetHinge2Anchor (scfParent->jointID, point[0], point[1], point[2]);
  }
}

csODEDefaultMoveCallback::csODEDefaultMoveCallback ()
{
  SCF_CONSTRUCT_IBASE (0);
}

csODEDefaultMoveCallback::~csODEDefaultMoveCallback ()
{
  SCF_DESTRUCT_IBASE();
}

void csODEDefaultMoveCallback::Execute (iMeshWrapper* mesh,
 csOrthoTransform& t)
{
  // Dont do anything if nothing has changed
  if (mesh->GetMovable()->GetPosition() == t.GetOrigin() &&
	  mesh->GetMovable()->GetTransform().GetT2O() == t.GetO2T())
	return;

  // Update movable
  mesh->GetMovable ()->SetPosition (t.GetOrigin ());
  mesh->GetMovable ()->GetTransform ().SetT2O (t.GetO2T ());
  mesh->GetMovable ()->UpdateMove ();
}

void csODEDefaultMoveCallback::Execute (csOrthoTransform& t)
{
  /* do nothing by default */
}
