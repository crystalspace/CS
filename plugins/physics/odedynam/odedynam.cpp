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
#include "csqsqrt.h"

#include "csgeom/obb.h"
#include "csgeom/plane3.h"
#include "csgeom/sphere.h"
#include "csgeom/pmtools.h"
#include "cstool/collider.h"
#include "csutil/event.h"
#include "iengine/engine.h"
#include "igeom/objmodel.h"
#include "igeom/polymesh.h"
#include "imesh/object.h"
#include "imesh/genmesh.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "ivaria/collider.h"
#include "ivaria/reporter.h"
#include "ivaria/dynamics.h"

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

SCF_IMPLEMENT_IBASE (csODECollider)
  SCF_IMPLEMENTS_INTERFACE (iDynamicsSystemCollider)
SCF_IMPLEMENT_IBASE_END

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

SCF_IMPLEMENT_IBASE (ODEBallJoint)
  SCF_IMPLEMENTS_INTERFACE (iODEBallJoint)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (ODESliderJoint)
  SCF_IMPLEMENTS_INTERFACE (iODESliderJoint)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (ODEHingeJoint)
  SCF_IMPLEMENTS_INTERFACE (iODEHingeJoint)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (ODEAMotorJoint)
  SCF_IMPLEMENTS_INTERFACE (iODEAMotorJoint)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (ODEUniversalJoint)
  SCF_IMPLEMENTS_INTERFACE (iODEUniversalJoint)
SCF_IMPLEMENT_IBASE_END

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

  //dGeomClass c;
  //c.bytes = sizeof (MeshInfo);
  //c.collider = &CollideSelector;
  //c.aabb = &GetAABB;
  //c.aabb_test = 0;
  //c.dtor = 0;
  //geomclassnum = dCreateGeomClass (&c);

  erp = 0.2f;
  cfm = 1e-5f;

  rateenabled = false;
  steptime = 0.1f;
  limittime = 1.0f;
  total_elapsed = 0.0f;

  stepfast = false;
  sfiter = 10;
  quickstep = false;
  qsiter = 10;
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
  if(stepfast) system->EnableStepFast(true);
  else if(quickstep) system->EnableQuickStep(true);
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

  dContact contact[512];
  int a = dCollide (o1, o2, 512, &(contact[0].geom), sizeof (dContact));
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
  quickstep = false;

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

void csODEDynamics::EnableQuickStep (bool enable)
{
  quickstep = enable;
  stepfast = false;

  for (size_t i = 0; i < systems.Length(); i ++)
  {
    csRef<iODEDynamicSystemState> sys = SCF_QUERY_INTERFACE (systems[i],
           iODEDynamicSystemState);
         sys->EnableQuickStep (enable);
  }
}

void csODEDynamics::SetQuickStepIterations (int iter)
{
  qsiter = iter;

  for (size_t i = 0; i < systems.Length(); i ++)
  {
    csRef<iODEDynamicSystemState> sys = SCF_QUERY_INTERFACE (systems[i],
      iODEDynamicSystemState);
    sys->SetQuickStepIterations (iter);
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
  if (Event.Type == csevBroadcast && csCommandEventHelper::GetCode(&Event) == cscmdPreProcess)
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
  quickstep = false;
  qsiter = 10;
  fastobjects = false;
  autodisable = false;
}

csODEDynamicSystem::~csODEDynamicSystem ()
{
  // must delete all these before deleting the actual world
  joints.DeleteAll ();
  groups.DeleteAll ();
  bodies.DeleteAll ();
  colliders.DeleteAll ();

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
iRigidBody *csODEDynamicSystem::GetBody (unsigned int index)
{
  if ((unsigned)index < bodies.GetSize ())
    return bodies[index];
  else return NULL;
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

csPtr<iODEUniversalJoint> csODEDynamicSystem::CreateUniversalJoint ()
{
  ODEUniversalJoint* joint = new ODEUniversalJoint (GetWorldID());
  strict_joints.Push (joint);
  return joint;
}

csPtr<iODESliderJoint> csODEDynamicSystem::CreateSliderJoint ()
{
  ODESliderJoint* joint = new ODESliderJoint (GetWorldID());
  strict_joints.Push (joint);
  return  csPtr<iODESliderJoint> (joint);
}

csPtr<iODEHingeJoint> csODEDynamicSystem::CreateHingeJoint ()
{
  ODEHingeJoint* joint = new ODEHingeJoint (GetWorldID());
  strict_joints.Push (joint);
  return  csPtr<iODEHingeJoint> (joint);
}

csPtr<iODEAMotorJoint> csODEDynamicSystem::CreateAMotorJoint ()
{
  ODEAMotorJoint* joint = new ODEAMotorJoint (GetWorldID());
  strict_joints.Push (joint);
  return  csPtr<iODEAMotorJoint> (joint);
}

csPtr<iODEBallJoint> csODEDynamicSystem::CreateBallJoint ()
{
  ODEBallJoint* joint = new ODEBallJoint (GetWorldID());
  strict_joints.Push (joint);
  return  csPtr<iODEBallJoint> (joint);
}

void csODEDynamicSystem::RemoveJoint (iODEUniversalJoint *joint)
{
  strict_joints.Delete ((ODEUniversalJoint*)joint);
}

void csODEDynamicSystem::RemoveJoint (iODEHingeJoint *joint)
{
  strict_joints.Delete ((ODEHingeJoint*)joint);
}
void csODEDynamicSystem::RemoveJoint (iODEAMotorJoint *joint)
{
  strict_joints.Delete ((ODEAMotorJoint*)joint);
}
void csODEDynamicSystem::RemoveJoint (iODEBallJoint *joint)
{
  strict_joints.Delete ((ODEBallJoint*)joint);
}
void csODEDynamicSystem::RemoveJoint (iODESliderJoint *joint)
{
  strict_joints.Delete ((ODESliderJoint*) joint);
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
      if (!quickstep)
      {
        dWorldStep (worldID, stepsize);
      }
      else
      {
        dWorldQuickStep (worldID, stepsize);
      }
    }
    else
    {
      dWorldStepFast1 (worldID, stepsize, sfiter);
    }
    for (size_t i = 0; i < bodies.Length(); i ++)
    {
        iRigidBody *b = bodies.Get(i);
        // only do this if the body is enabled
        if (b->IsEnabled())
        {
          b->SetAngularVelocity (b->GetAngularVelocity () * roll_damp);
          b->SetLinearVelocity (b->GetLinearVelocity () * lin_damp);
        }
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
  csODECollider *odec = new csODECollider ();
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->CreateMeshGeometry (mesh);
  c->SetTransform (trans);
  odec->AddToSpace (spaceID);
  colliders.Push (c);

  return true;
}

bool csODEDynamicSystem::AttachColliderCylinder (float length, float radius,
        const csOrthoTransform& trans, float friction, float elasticity, float softness)
{
  csODECollider *odec = new csODECollider ();
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->CreateCCylinderGeometry (length, radius);
  c->SetTransform (trans);
  odec->AddToSpace (spaceID);
  colliders.Push (c);

  return true;
}

bool csODEDynamicSystem::AttachColliderBox (const csVector3 &size,
        const csOrthoTransform& trans, float friction, float elasticity, float softness)
{
  csODECollider *odec = new csODECollider ();
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->CreateBoxGeometry (size);
  c->SetTransform (trans);
  odec->AddToSpace (spaceID);
  colliders.Push (c);

  return true;
}

bool csODEDynamicSystem::AttachColliderSphere (float radius,
    const csVector3 &offset, float friction, float elasticity, float softness)
{
  csODECollider *odec = new csODECollider ();
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->CreateSphereGeometry (csSphere (offset, radius));
  odec->AddToSpace (spaceID);
  colliders.Push (c);

  return true;
}
bool csODEDynamicSystem::AttachColliderPlane (const csPlane3 &plane,
    float friction, float elasticity, float softness)
{
  csODECollider *odec = new csODECollider ();
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->CreatePlaneGeometry (plane);
  odec->AddToSpace (spaceID);
  colliders.Push (c);

  return true;
}
csRef<iDynamicsSystemCollider> csODEDynamicSystem::GetCollider (unsigned int index)
{
  if (index < colliders.GetSize ())
    return colliders[index];
  else return NULL;
}
csRef<iDynamicsSystemCollider> csODEDynamicSystem::CreateCollider ()
{
  csODECollider *odec = new csODECollider ();
  odec->AddToSpace (spaceID);
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  colliders.Push (c);
  return c;
}
void csODEDynamicSystem::AttachCollider (iDynamicsSystemCollider* collider)
{
  ((csODECollider*)collider)->AddToSpace (spaceID);
  colliders.Push (collider);
}
void csODEDynamicSystem::EnableAutoDisable (bool enable)
{
  autodisable=enable;
  dWorldSetAutoDisableFlag (worldID, enable);
}

void csODEDynamicSystem::SetAutoDisableParams (float linear, float angular,
    int steps, float time)
{
  if(linear!=0.0f) dWorldSetAutoDisableLinearThreshold (worldID, linear);
  if(angular!=0.0f) dWorldSetAutoDisableAngularThreshold (worldID, angular);
  if(steps!=0.0f) dWorldSetAutoDisableSteps (worldID, steps);
  if(time!=0.0f) dWorldSetAutoDisableTime (worldID, time);
}

void csODEDynamicSystem::SetContactMaxCorrectingVel (float v)
{
    dWorldSetContactMaxCorrectingVel (worldID, v);
}

float csODEDynamicSystem::GetContactMaxCorrectingVel ()
{
    return dWorldGetContactMaxCorrectingVel (worldID);
}

void csODEDynamicSystem::SetContactSurfaceLayer (float depth)
{
    dWorldSetContactSurfaceLayer(worldID, depth);
}

float csODEDynamicSystem::GetContactSurfaceLayer ()
{
    return dWorldGetContactSurfaceLayer(worldID);
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

//--------------------------csODECollider-------------------------------------
csODECollider::csODECollider ()
{
  surfacedata[0] = 0;
  surfacedata[1] = 0;
  surfacedata[2] = 0;
  density = 0;
  geomID = NULL;
  spaceID = NULL;
  coll_cb = 0;
  transformID = dCreateGeomTransform (0);
  dGeomTransformSetCleanup (transformID, 1);
  geom_type = (csColliderGeometryType) 0;
}

csODECollider::~csODECollider ()
{
  ClearContents ();
}

void csODECollider::ClearContents ()
{
  dGeomDestroy (geomID);
  dGeomDestroy (transformID);
  surfacedata[0] = 0;
  surfacedata[1] = 0;
  surfacedata[2] = 0;
  density = 0;
  geomID = NULL;
  spaceID = NULL;
  coll_cb = 0;
  transformID = dCreateGeomTransform (0);
  dGeomTransformSetCleanup (transformID, 1);
  geom_type = (csColliderGeometryType) 0;
}

void csODECollider::MassCorrection ()
{
  dMass m, om;
  dMassSetZero (&m);
  
  switch (geom_type)
  {
  case BOX_COLLIDER_GEOMETRY:
    {
      dVector3 size;
      dGeomBoxGetLengths (geomID, size);
      dMassSetBox (&m, density, size[0], size[1], size[2]);
    }
    break;
  case SPHERE_COLLIDER_GEOMETRY:
    dMassSetSphere (&m, density, dGeomSphereGetRadius (geomID));
    break;
  case CYLINDER_COLLIDER_GEOMETRY:
    {
      dReal radius, length;
      dGeomCCylinderGetParams (geomID, &radius, &length);
      dMassSetCappedCylinder (&m, density, 3, radius, length);
    }
    break;
  case TRIMESH_COLLIDER_GEOMETRY:
    {
      // ODE doesn't have any function to get the mass of arbitrary triangles
      // so we will just use the OBB
      int tri_count = dGeomTriMeshGetTriangleCount (geomID);
      csOBB b;
      for (int i = 0; i < tri_count; i++)
      {
        dVector3 v0, v1, v2;
        dGeomTriMeshGetTriangle (geomID, i, &v0, &v1, &v2);
        b.AddBoundingVertex (csVector3 (v0[0], v0[1], v0[2]));
        b.AddBoundingVertex (csVector3 (v1[0], v1[1], v1[2]));
        b.AddBoundingVertex (csVector3 (v2[0], v2[1], v2[2]));
      }
      dMassSetBox (&m, density, b.MaxX()-b.MinX(), b.MaxY()-b.MinY(), b.MaxZ()-b.MinZ());
    }
  default:
    break;
  }

  const dReal *pos = dGeomGetPosition (geomID);
  dMassTranslate (&m, pos[0], pos[1], pos[2]);

  dMassRotate (&m, dGeomGetRotation (geomID));

  dBodyID bodyID = dGeomGetBody (transformID);
  dBodyGetMass (bodyID, &om);
  dMassAdd (&om, &m);
  dBodySetMass (bodyID, &om);
}
void csODECollider::AttachBody (dBodyID bodyID)
{
  dGeomSetBody (transformID, bodyID);
  if (geomID)
  {
    MassCorrection ();
  }
}
bool csODECollider::CreateMeshGeometry (iMeshWrapper *mesh)
{
  if (geomID) ClearContents ();

  geom_type = TRIMESH_COLLIDER_GEOMETRY;

  // From Eroroman & Marc Rochel with modifications by Mike Handverger and Piotr Obrzut

  iPolygonMesh* p = mesh->GetMeshObject()->GetObjectModel()->GetPolygonMeshColldet();
  
  if (p->GetVertexCount () == 0 || p->GetTriangleCount () == 0)
    return false;

  csTriangle *c_triangle;
  int tr_num;
  // Slight problem here is that we need to keep vertices and indices around
  // since ODE only uses the pointers. I am not sure if ODE cleans them up
  // on exit or not. If not, we need some way to keep track of all mesh colliders
  // and clean them up on destruct.
  csPolygonMeshTools::Triangulate(p, c_triangle, tr_num);
  float *vertices = new float[p->GetVertexCount()*3];
  int *indeces = new int[tr_num*3];
  csVector3 *c_vertex = p->GetVertices();
  //csFPrintf(stderr, "vertex count: %d\n", p->GetVertexCount());
  //csFPrintf(stderr, "triangles count: %d\n", tr_num);
  int i=0, j=0;
  for (i=0, j=0; i < p->GetVertexCount(); i++)
  {
    vertices[j++] = c_vertex[i].x;
    vertices[j++] = c_vertex[i].y;
    vertices[j++] = c_vertex[i].z;
    //csFPrintf(stderr, "vertex %d coords -> x=%.1f, y=%.1f, z=%.1f\n", i,c_vertex[i].x, c_vertex[i].y, c_vertex[i].z);
  }
  for (i=0, j=0; i < tr_num; i++)
  {
    indeces[j++] = c_triangle[i].c;
    indeces[j++] = c_triangle[i].b;
    indeces[j++] = c_triangle[i].a;
    //csFPrintf(stderr, "triangle %d -> a=%d, b=%d, c=%d\n", i,c_triangle[i].a, c_triangle[i].b, c_triangle[i].c);
  }
  dTriMeshDataID TriData = dGeomTriMeshDataCreate();

  dGeomTriMeshDataBuildSingle(TriData, vertices, 3*sizeof(float),
    p->GetVertexCount(), indeces, 3*tr_num, 3*sizeof(int));

  geomID = dCreateTriMesh(0, TriData, 0, 0, 0);

  if (spaceID) AddToSpace (spaceID);

  return true;
}
bool csODECollider::CreateCCylinderGeometry (float length, float radius)
{
  if (geomID) ClearContents ();

  geom_type = CYLINDER_COLLIDER_GEOMETRY;

  geomID = dCreateCCylinder (0, radius, length);
  dGeomTransformSetGeom (transformID, geomID);

  if (dGeomGetBody (transformID))
    MassCorrection ();
  
  dGeomSetData (geomID, (void*)surfacedata);

  if (spaceID) AddToSpace (spaceID);

  return true;
}
bool csODECollider::CreatePlaneGeometry (const csPlane3& plane)
{
  if (geomID) ClearContents ();

  geom_type = PLANE_COLLIDER_GEOMETRY;
  
  geomID = dCreatePlane (0, -plane.A(), -plane.B(), -plane.C(), plane.D());

  dGeomSetData (geomID, (void*)surfacedata);

  if (spaceID) AddToSpace (spaceID);

  return true;
}
bool csODECollider::CreateSphereGeometry (const csSphere& sphere)
{
  if (geomID) ClearContents ();

  geom_type = SPHERE_COLLIDER_GEOMETRY;

  geomID = dCreateSphere (0, sphere.GetRadius ());
  dGeomTransformSetGeom (transformID, geomID);
  
  csVector3 offset = sphere.GetCenter ();
  dGeomSetPosition (transformID, offset.x, offset.y, offset.z);

  if (dGeomGetBody (transformID))
    MassCorrection ();

  dGeomSetData (geomID, (void*)surfacedata);

  if (spaceID) AddToSpace (spaceID);

  return true;
}
bool csODECollider::CreateBoxGeometry (const csVector3& size)
{
  if (geomID) ClearContents ();

  geom_type = BOX_COLLIDER_GEOMETRY;

  geomID = dCreateBox (0, size.x, size.y, size.z);

  dGeomTransformSetGeom (transformID, geomID);

  if (dGeomGetBody (transformID))
    MassCorrection ();

  dGeomSetData (geomID, (void*)surfacedata);

  if (spaceID) AddToSpace (spaceID);

  return true;
}

void csODECollider::CS2ODEMatrix (const csMatrix3& csmat, dMatrix3& odemat)
{  
  odemat[0] = csmat.m11; odemat[1] = csmat.m21; odemat[2] = csmat.m31; odemat[3] = 0;
  odemat[4] = csmat.m12; odemat[5] = csmat.m22; odemat[6] = csmat.m32; odemat[7] = 0;
  odemat[8] = csmat.m13; odemat[9] = csmat.m23; odemat[10] = csmat.m33; odemat[11] = 0;
}
void csODECollider::ODE2CSMatrix (const dReal* odemat, csMatrix3& csmat)
{  
  csmat.m11 = odemat[0]; csmat.m12 = odemat[4]; csmat.m13 = odemat[8];
  csmat.m21 = odemat[1]; csmat.m22 = odemat[5]; csmat.m23 = odemat[9];
  csmat.m31 = odemat[2]; csmat.m32 = odemat[6]; csmat.m33 = odemat[10];
}
void csODECollider::SetTransform (const csOrthoTransform& transform)
{
  // can't set plane's transform b/c it causes non placeable geom run-time 
  // error w/debug build of ode
  if (!geomID || geom_type == PLANE_COLLIDER_GEOMETRY) 
    return;

  csVector3 pos = transform.GetOrigin ();
  dGeomSetPosition (geomID, pos.x, pos.y, pos.z);

  dMatrix3 rot;
  CS2ODEMatrix (transform.GetO2T (), rot);
  dGeomSetRotation (geomID, rot);

  if (dGeomGetBody (transformID))
    MassCorrection ();
}
csOrthoTransform csODECollider::GetTransform ()
{
  const dReal *tv = dGeomGetPosition (transformID);
  csVector3 t_pos (tv[0], tv[1], tv[2]);

  csMatrix3 t_rot;
  ODE2CSMatrix (dGeomGetRotation (transformID), t_rot);

  csOrthoTransform t_transf (t_rot, t_pos);

  if (!geomID || geom_type == PLANE_COLLIDER_GEOMETRY)
  {
    return t_transf;
  }

  const dReal *gv = dGeomGetPosition (geomID);
  csVector3 g_pos (gv[0], gv[1], gv[2]);

  csMatrix3 g_rot;
  ODE2CSMatrix (dGeomGetRotation (geomID), g_rot);

  return t_transf * csOrthoTransform (g_rot, g_pos);
}
void csODECollider::AddTransformToSpace (dSpaceID spaceID)
{
  dSpaceID prev = dGeomGetSpace (transformID);
  if (prev)dSpaceRemove (prev, transformID);
  if (geomID) dSpaceAdd (spaceID, transformID); 
}
void csODECollider::AddToSpace (dSpaceID spaceID) 
{
  dSpaceID prev = dGeomGetSpace (geomID);
  if (prev)dSpaceRemove (prev, geomID);
  if (geomID) dSpaceAdd (spaceID, geomID); 
  csODECollider::spaceID = spaceID;
}
void csODECollider::FillWithColliderGeometry (csRef<iGeneralFactoryState> genmesh_fact)
{
  switch (geom_type)
  {
  case BOX_COLLIDER_GEOMETRY:
    {
      dVector3 size;
      dGeomBoxGetLengths (geomID, size);

      csBox3 box (csVector3 (0));
      box.SetSize (csVector3 (size[0], size[1], size[2]));
      genmesh_fact->GenerateBox (box);
      genmesh_fact->CalculateNormals (); 
    }
    break;
  case SPHERE_COLLIDER_GEOMETRY:
    {
      csSphere sphere (csVector3 (0), dGeomSphereGetRadius (geomID));
      genmesh_fact->GenerateSphere (sphere, 30);
      genmesh_fact->CalculateNormals (); 
    }
  case PLANE_COLLIDER_GEOMETRY:
    {
      dVector4 plane;
      dGeomPlaneGetParams (geomID, plane);
      genmesh_fact->GeneratePlane (csPlane3 (-plane[0], -plane[1], -plane[2], plane[3]),
        csBox3 (csVector3 (-CS_BOUNDINGBOX_MAXVALUE),csVector3 (CS_BOUNDINGBOX_MAXVALUE)));
    }
    break;
  case TRIMESH_COLLIDER_GEOMETRY:
    {
      int tri_count = dGeomTriMeshGetTriangleCount (geomID);
      genmesh_fact->SetVertexCount (tri_count * 3);
      genmesh_fact->SetTriangleCount (tri_count);
      csVector3* vertices = genmesh_fact->GetVertices ();
      csTriangle* triangles = genmesh_fact->GetTriangles ();
      
      for (int i = 0; i < tri_count; i++)
      {
        dVector3 v0, v1, v2;
        dGeomTriMeshGetTriangle (geomID, i, &v0, &v1, &v2);
        vertices[i*3] = csVector3 (v0[0], v0[1], v0[2]);
        vertices[i*3+1] = csVector3 (v1[0], v1[1], v1[2]);
        vertices[i*3+2] = csVector3 (v2[0], v2[1], v2[2]);
        triangles[i].a = i*3; triangles[i].b = i*3+1; triangles[i].c = i*3+2; 
      }
      genmesh_fact->CalculateNormals ();
    }
    break;
  default:
    break;
  }
}
//--------------------------csODERigidBody-------------------------------------
csODERigidBody::csODERigidBody (csODEDynamicSystem* sys)
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
  if (move_cb) move_cb->DecRef ();
  if (coll_cb) coll_cb->DecRef ();
  dSpaceDestroy (groupID);
  dBodyDestroy (bodyID);
  colliders.DeleteAll ();

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
      //MeshInfo *gdata = (MeshInfo*)dGeomGetClassData (tempID);
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

bool csODERigidBody::Disable ()
{
  dBodyDisable (bodyID);
  return true;
}

bool csODERigidBody::Enable ()
{
  dBodyEnable (bodyID);
  return true;
}

bool csODERigidBody::IsEnabled ()
{
  return dBodyIsEnabled (bodyID);
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
  csODECollider *odec = new csODECollider ();
  
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->SetDensity (density);
  c->CreateMeshGeometry (mesh);
  c->SetTransform (trans);
  odec->AttachBody (bodyID);
  odec->AddTransformToSpace (groupID);
  colliders.Push (c);

  return true;
}

bool csODERigidBody::AttachColliderCylinder (float length, float radius,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness)
{
  csODECollider *odec = new csODECollider ();
  
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->SetDensity (density);
  c->CreateCCylinderGeometry (length, radius);
  c->SetTransform (trans);
  odec->AttachBody (bodyID);
  odec->AddTransformToSpace (groupID);
  colliders.Push (c);

  return true;
}
bool csODERigidBody::AttachColliderBox (const csVector3 &size,
    const csOrthoTransform& trans, float friction, float density,
    float elasticity, float softness)
{
  csODECollider *odec = new csODECollider ();
  
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->SetDensity (density);
  c->CreateBoxGeometry (size);
  odec->AttachBody (bodyID);
  c->SetTransform (trans);
  odec->AddTransformToSpace (groupID);
  colliders.Push (c);

  return true;
}

bool csODERigidBody::AttachColliderSphere (float radius, const csVector3 &offset,
    float friction, float density, float elasticity, float softness)
{
  csODECollider *odec = new csODECollider ();
  
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->SetDensity (density);
  c->CreateSphereGeometry (csSphere (offset, radius));
  odec->AttachBody (bodyID);
  odec->AddTransformToSpace (groupID);
  colliders.Push (c);

  return true;
}

bool csODERigidBody::AttachColliderPlane (const csPlane3& plane,
  float friction, float density, float elasticity, float softness)
{
  csODECollider *odec = new csODECollider (); 
  csRef<iDynamicsSystemCollider> c = (csPtr<iDynamicsSystemCollider>) odec;
  c->SetElasticity (elasticity);
  c->SetFriction (friction);
  c->SetSoftness (softness);
  c->SetDensity (density);
  c->CreatePlaneGeometry (plane);
  colliders.Push (c);
  //causes non placeable geom run-time error w/debug build of ode.
  //odec->AttachBody (bodyID);   
  odec->AddToSpace (dynsys->GetSpaceID());

  return true;
}
void csODERigidBody::AttachCollider (iDynamicsSystemCollider* collider)
{
  dynsys->DestroyCollider (collider);
  ((csODECollider*) collider)->AttachBody (bodyID);
  colliders.Push (collider);
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
  mat[0] = rot.m11; mat[1] = rot.m21; mat[2] = rot.m31; mat[3] = 0;
  mat[4] = rot.m12; mat[5] = rot.m22; mat[6] = rot.m32; mat[7] = 0;
  mat[8] = rot.m13; mat[9] = rot.m23; mat[10] = rot.m33; mat[11] = 0;
  dBodySetRotation (bodyID, mat);
}

const csMatrix3 csODERigidBody::GetOrientation () const
{
  const dReal* mat = dBodyGetRotation (bodyID);
  csMatrix3 rot;
  rot.m11 = mat[0]; rot.m12 = mat[4]; rot.m13 = mat[8];
  rot.m21 = mat[1]; rot.m22 = mat[5]; rot.m23 = mat[9];
  rot.m31 = mat[2]; rot.m32 = mat[6]; rot.m33 = mat[10];
  return rot;
}

void csODERigidBody::SetTransform (const csOrthoTransform& trans)
{
  csVector3 pos = trans.GetOrigin ();
  dBodySetPosition (bodyID, pos.x, pos.y, pos.z);
  csMatrix3 rot = trans.GetT2O ();
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
  rot.m11 = mat[0]; rot.m12 = mat[4]; rot.m13 = mat[8];
  rot.m21 = mat[1]; rot.m22 = mat[5]; rot.m23 = mat[9];
  rot.m31 = mat[2]; rot.m32 = mat[6]; rot.m33 = mat[10];
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
float csODERigidBody::GetMass ()
{
  dMass m;
  dBodyGetMass (bodyID, &m);
  return m.mass;
}
csVector3 csODERigidBody::GetCenter ()
{
  dMass m;
  dBodyGetMass (bodyID, &m);
  return csVector3 (m.c[0], m.c[1], m.c[2]);
}
csMatrix3 csODERigidBody::GetInertia ()
{
  dMass m;
  dBodyGetMass (bodyID, &m);
  return csMatrix3 (m.I[0], m.I[1], m.I[2],
                    m.I[4], m.I[5], m.I[6],
                    m.I[8], m.I[9], m.I[10]);
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
csRef<iDynamicsSystemCollider> csODERigidBody::GetCollider (unsigned int index)
{
  if (index < colliders.GetSize ())
    return colliders[index];
  else return NULL;
}
//--------------------------csStrictODEJoint-----------------------------------------------
void csStrictODEJoint::Attach (iRigidBody *b1, iRigidBody *b2)
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
  dJointAttach (jointID, bodyID[0], bodyID[1]);
}
csRef<iRigidBody> csStrictODEJoint::GetAttachedBody (int b)
{
  return (b == 0) ? body[0] : body[1];
}
void  csStrictODEJoint::SetParam (int joint_type, int parameter, int axis, float value)
{
  int param = parameter;

  switch (axis)
  {
    case 1:
      switch (param)
      {
      case dParamLoStop:
        param = dParamLoStop2;
      break;
      case dParamHiStop:
        param = dParamHiStop2;
      break;
      case dParamVel:
        param = dParamVel2;
      break;
      case dParamFMax:
        param = dParamFMax2;
      break;
      case dParamFudgeFactor:
        param = dParamFudgeFactor2;
      break;
      case dParamBounce:
        param = dParamBounce2;
      break;
      case dParamCFM:
        param = dParamCFM2;
      break;
      case dParamStopERP:
        param = dParamStopERP2;
      break;
      case dParamStopCFM:
        param = dParamStopCFM2;
      break;
      case dParamSuspensionERP:
        param = dParamSuspensionERP2;
      break;
      case dParamSuspensionCFM:
        param = dParamSuspensionCFM2;
      break;
      }
      break;

    case 2:
      switch (param)
      {
      case dParamLoStop:
        param = dParamLoStop3;
      break;
      case dParamHiStop:
        param = dParamHiStop3;
      break;
      case dParamVel:
        param = dParamVel3;
      break;
      case dParamFMax:
        param = dParamFMax3;
      break;
      case dParamFudgeFactor:
        param = dParamFudgeFactor3;
      break;
      case dParamBounce:
        param = dParamBounce3;
      break;
      case dParamCFM:
        param = dParamCFM3;
      break;
      case dParamStopERP:
        param = dParamStopERP3;
      break;
      case dParamStopCFM:
        param = dParamStopCFM3;
      break;
      case dParamSuspensionERP:
        param = dParamSuspensionERP3;
      break;
      case dParamSuspensionCFM:
        param = dParamSuspensionCFM3;
      break;
      }
      break;
    default:
      ; // do nothing
  }

  switch (joint_type)
  {
    case CS_ODE_JOINT_TYPE_HINGE:
      dJointSetHingeParam (jointID, param, value);
      break;
    case CS_ODE_JOINT_TYPE_SLIDER:
      dJointSetSliderParam (jointID, param, value);
      break;
    case CS_ODE_JOINT_TYPE_HINGE2:
      dJointSetHinge2Param (jointID, param, value);
      break;
    case CS_ODE_JOINT_TYPE_AMOTOR:
      dJointSetAMotorParam (jointID, param, value);
      break;
    case CS_ODE_JOINT_TYPE_UNIVERSAL:
      dJointSetUniversalParam (jointID, param, value);
      break;
    default:
      ; // do nothing
  }
}

float csStrictODEJoint::GetParam (int joint_type, int parameter, int axis)
{
  int param = parameter;

  switch (axis)
  {
    case 1:
      switch (param)
      {
      case dParamLoStop:
        param = dParamLoStop2;
      break;
      case dParamHiStop:
        param = dParamHiStop2;
      break;
      case dParamVel:
        param = dParamVel2;
      break;
      case dParamFMax:
        param = dParamFMax2;
      break;
      case dParamFudgeFactor:
        param = dParamFudgeFactor2;
      break;
      case dParamBounce:
        param = dParamBounce2;
      break;
      case dParamCFM:
        param = dParamCFM2;
      break;
      case dParamStopERP:
        param = dParamStopERP2;
      break;
      case dParamStopCFM:
        param = dParamStopCFM2;
      break;
      case dParamSuspensionERP:
        param = dParamSuspensionERP2;
      break;
      case dParamSuspensionCFM:
        param = dParamSuspensionCFM2;
      break;
      }
      break;

    case 2:
      switch (param)
      {
      case dParamLoStop:
        param = dParamLoStop3;
      break;
      case dParamHiStop:
        param = dParamHiStop3;
      break;
      case dParamVel:
        param = dParamVel3;
      break;
      case dParamFMax:
        param = dParamFMax3;
      break;
      case dParamFudgeFactor:
        param = dParamFudgeFactor3;
      break;
      case dParamBounce:
        param = dParamBounce3;
      break;
      case dParamCFM:
        param = dParamCFM3;
      break;
      case dParamStopERP:
        param = dParamStopERP3;
      break;
      case dParamStopCFM:
        param = dParamStopCFM3;
      break;
      case dParamSuspensionERP:
        param = dParamSuspensionERP3;
      break;
      case dParamSuspensionCFM:
        param = dParamSuspensionCFM3;
      break;
      }
      break;
    default:
      ; // do nothing
  }

  switch (joint_type)
  {
    case CS_ODE_JOINT_TYPE_HINGE:
      return dJointGetHingeParam (jointID, param);
      break;
    case CS_ODE_JOINT_TYPE_SLIDER:
      return dJointGetSliderParam (jointID, param);
      break;
    case CS_ODE_JOINT_TYPE_HINGE2:
      return dJointGetHinge2Param (jointID, param);
      break;
    case CS_ODE_JOINT_TYPE_AMOTOR:
      return dJointGetAMotorParam (jointID, param);
      break;
    case CS_ODE_JOINT_TYPE_UNIVERSAL:
      return dJointGetUniversalParam (jointID, param);
      break;
    default:
      return 0.0;
  }
}

void csStrictODEJoint::CreateFeedback ()
{
    feedback = new dJointFeedback ();

    feedback->f1[0] = 0;
    feedback->f1[1] = 0;
    feedback->f1[2] = 0;

    feedback->f2[0] = 0;
    feedback->f2[1] = 0;
    feedback->f2[2] = 0;

    feedback->t1[0] = 0;
    feedback->t1[1] = 0;
    feedback->t1[2] = 0;

    feedback->t2[0] = 0;
    feedback->t2[1] = 0;
    feedback->t2[2] = 0;

    dJointSetFeedback (jointID, feedback);
}

csVector3 csStrictODEJoint::GetFeedbackForce1 ()
{
  if (feedback == NULL) CreateFeedback ();
  return csVector3(feedback->f1[0], feedback->f1[1], feedback->f1[2]);
}

csVector3 csStrictODEJoint::GetFeedbackTorque1 ()
{
  if (feedback == NULL) CreateFeedback ();
  return csVector3(feedback->f2[0], feedback->f2[1], feedback->f2[2]);
}

csVector3 csStrictODEJoint::GetFeedbackForce2 ()
{
  if (feedback == NULL) CreateFeedback ();
  return csVector3(feedback->t1[0], feedback->t1[1], feedback->t1[2]);
}

csVector3 csStrictODEJoint::GetFeedbackTorque2 ()
{
  if (feedback == NULL) CreateFeedback ();
  return csVector3(feedback->t2[0], feedback->t2[1], feedback->t2[2]);
}

//-------------------------------------------------------------------------------
ODESliderJoint::ODESliderJoint (dWorldID w_id)
{
  SCF_CONSTRUCT_IBASE (0);
  jointID = dJointCreateSlider (w_id, 0);
}
ODESliderJoint::~ODESliderJoint ()
{
  SCF_DESTRUCT_IBASE();
  dJointDestroy (jointID);
}
csVector3 ODESliderJoint::GetSliderAxis ()
{
  dVector3 pos;
  dJointGetSliderAxis (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}
//-------------------------------------------------------------------------------
ODEUniversalJoint::ODEUniversalJoint (dWorldID w_id)
{
  SCF_CONSTRUCT_IBASE (0);
  jointID = dJointCreateUniversal (w_id, 0);
}
ODEUniversalJoint::~ODEUniversalJoint ()
{
  SCF_DESTRUCT_IBASE();
  dJointDestroy (jointID);
}
csVector3 ODEUniversalJoint::GetUniversalAnchor1 ()
{
  dVector3 pos;
  dJointGetUniversalAnchor (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}
csVector3 ODEUniversalJoint::GetUniversalAnchor2 ()
{
  dVector3 pos;
  dJointGetUniversalAnchor2 (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}
csVector3 ODEUniversalJoint::GetUniversalAxis1 ()
{
  dVector3 pos;
  dJointGetUniversalAxis1 (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}
csVector3 ODEUniversalJoint::GetUniversalAxis2 ()
{
  dVector3 pos;
  dJointGetUniversalAxis2 (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}
//-------------------------------------------------------------------------------

ODEAMotorJoint::ODEAMotorJoint (dWorldID w_id)
{
  SCF_CONSTRUCT_IBASE (0);
  jointID = dJointCreateAMotor (w_id, 0);
}

ODEAMotorJoint::~ODEAMotorJoint ()
{
  SCF_DESTRUCT_IBASE();
  dJointDestroy (jointID);
}


csVector3 ODEAMotorJoint::GetAMotorAxis (int axis_num)
{
  dVector3 pos;
  dJointGetAMotorAxis (jointID, axis_num, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}

//-------------------------------------------------------------------------------

ODEHingeJoint::ODEHingeJoint (dWorldID w_id)
{
  SCF_CONSTRUCT_IBASE (0);
  jointID = dJointCreateHinge (w_id, 0);
}

ODEHingeJoint::~ODEHingeJoint ()
{
  SCF_DESTRUCT_IBASE();
  dJointDestroy (jointID);
}

csVector3 ODEHingeJoint::GetHingeAnchor1 ()
{
  dVector3 pos;
  dJointGetHingeAnchor (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}

csVector3 ODEHingeJoint::GetHingeAnchor2 ()
{
  dVector3 pos;
  dJointGetHingeAnchor2 (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}

csVector3 ODEHingeJoint::GetHingeAxis ()
{
  dVector3 pos;
  dJointGetHingeAxis (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}

csVector3 ODEHingeJoint::GetAnchorError ()
{
  csVector3 pos1 = GetHingeAnchor1 ();
  csVector3 pos2 = GetHingeAnchor2 ();

  csVector3 result =  pos1 - pos2;
  if (result.x < 0) result.x = - result.x;
  if (result.y < 0) result.y = - result.y;
  if (result.z < 0) result.z = - result.z;

  return result;
}

//-------------------------------------------------------------------------------

ODEBallJoint::ODEBallJoint (dWorldID w_id)
{
  SCF_CONSTRUCT_IBASE (0);
  jointID = dJointCreateBall (w_id, 0);
}

ODEBallJoint::~ODEBallJoint ()
{
  SCF_DESTRUCT_IBASE();
  dJointDestroy (jointID);
}

csVector3 ODEBallJoint::GetBallAnchor1 ()
{
  dVector3 pos;
  dJointGetBallAnchor (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}

csVector3 ODEBallJoint::GetBallAnchor2 ()
{
  dVector3 pos;
  dJointGetBallAnchor2 (jointID, pos);
  return csVector3 (pos[0], pos[1], pos[2]);
}

csVector3 ODEBallJoint::GetAnchorError ()
{
  csVector3 pos1, pos2;
  pos1 = GetBallAnchor1 ();
  pos2 = GetBallAnchor2 ();

  csVector3 result =  pos1 - pos2;
  if (result.x < 0) result.x = - result.x;
  if (result.y < 0) result.y = - result.y;
  if (result.z < 0) result.z = - result.z;

  return result;
}

//-------------------------------------------------------------------------------

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
          mesh->GetMovable()->GetTransform().GetT2O() == t.GetT2O())
        return;

  // Update movable
  mesh->GetMovable ()->SetPosition (t.GetOrigin ());
  mesh->GetMovable ()->GetTransform ().SetT2O (t.GetT2O ());
  mesh->GetMovable ()->UpdateMove ();
}

void csODEDefaultMoveCallback::Execute (csOrthoTransform& t)
{
  /* do nothing by default */
}

