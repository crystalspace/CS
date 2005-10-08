/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
   $Id$

    This file is part of Crystal Space Virtual Object System Abstract
    3D Layer plugin (csvosa3dl).

    Copyright (C) 2004 Peter Amstutz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "cssysdef.h"

#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"
#include "csgeom/quaternion.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"

#include "vosobject3d.h"
#include <vos/metaobjects/a3dl/a3dl.hh>
#include <vos/metaobjects/property/remoteproperty.hh>

using namespace VUtil;
using namespace VOS;

SCF_IMPLEMENT_IBASE (csVosObject3D)
  SCF_IMPLEMENTS_INTERFACE (iVosObject3D)
  SCF_IMPLEMENTS_INTERFACE (iVosApi)
  SCF_IMPLEMENTS_INTERFACE (iDynamicsMoveCallback)
SCF_IMPLEMENT_IBASE_END

/// csVosObject3D ///

csVosObject3D::csVosObject3D(csMetaObject3D* obj3d, VUtil::RefCounted* rc)
{
  if(rc)
  {
    rc->acquire();
    object3d.assign(obj3d, false);
  }
  else object3d.assign(obj3d, true);

  SCF_CONSTRUCT_IBASE (0);
  meshwrapper = 0;
  collider = 0;
}

csVosObject3D::~csVosObject3D()
{
  if (meshwrapper.IsValid())
  {
    csRef<iEngine> engine = CS_QUERY_REGISTRY (
    	object3d->getVosA3DL()->GetObjectRegistry(), iEngine);
    meshwrapper->QueryObject()->ObjRemove(this);
    engine->GetMeshes()->Remove(meshwrapper);
  }
  SCF_DESTRUCT_IBASE();
}

csRef<iMeshWrapper> csVosObject3D::GetMeshWrapper()
{
  return meshwrapper;
}

void csVosObject3D::SetMeshWrapper(iMeshWrapper* mw)
{
  LOG("vosobject3d", 3, "setting mesh wrapper on " << object3d->getURLstr());


  if (meshwrapper.IsValid())
  {
    csRef<iEngine> engine = CS_QUERY_REGISTRY (object3d->getVosA3DL()->GetObjectRegistry(),
                                               iEngine);
    meshwrapper->QueryObject()->ObjRemove(this);
    engine->GetMeshes()->Remove(meshwrapper);
  }

  meshwrapper = mw;
  meshwrapper->QueryObject()->ObjAdd(this);
}

csRef<iRigidBody> csVosObject3D::GetCollider ()
{
  return collider;
}

void csVosObject3D::SetCollider (iRigidBody *col)
{
  collider = col;
}

vRef<VOS::Vobject> csVosObject3D::GetVobject()
{
  return object3d;
}

void csVosObject3D::Execute(iMeshWrapper *, csOrthoTransform &t)
{
  LOG ("csVosObject3D", 3, "Received Execute callback 2 args");
  this->Execute(t);
}

void csVosObject3D::Execute(csOrthoTransform & t)
{
#if 0
  LOG ("csVosObject3D", 3, "Received Execute callback 1 arg");
  csVector3 pos = collider->GetPosition();
  csVector3 vel = collider->GetLinearVelocity();
  csVector3 acc = collider->GetForce();

  LOG("vosobject3d", 2, "pos is "
      << pos.x << " " << pos.y << " " << pos.z);
  LOG("vosobject3d", 2, "force is "
      << acc.x << " " << acc.y << " " << acc.z);
  LOG("vosobject3d", 2, "vel is "
      << vel.x << " " << vel.y << " " << vel.z);
#endif

  try {
    double x, y, z;

    object3d->getPosition(x, y, z);

    if ((csVector3(x, y, z) - t.GetOrigin()).Norm() > .0001)
    {
      LOG("vosobject3d", 4, "changePosition " <<
          " to " << t.GetOrigin().x << " " << t.GetOrigin().y << " " << t.GetOrigin().z);
      object3d->setPosition(t.GetOrigin().x, t.GetOrigin().y, t.GetOrigin().z);

      // XXX this is a horrible hack
      //collider->SetOrientation(csMatrix3());

      //double a, b, c, d;
      //object3d->getOrientation(a, b, c, d);
    }

#if 1
      csQuaternion q(collider->GetOrientation());
      csVector3 axis;
      float angle;

      q.GetAxisAngle(axis, angle);

      //LOG("vosobject3d", 2, "changeOrientation " <<
      //" to " << axis.x << " " << axis.y << " " << axis.z << " " << angle);

      if (axis.x == axis.x && axis.y == axis.y && axis.z == axis.z) {
        object3d->setOrientation(axis.x, axis.y, axis.z, angle * 180.0 / M_PI);
      }
#endif
  }
  catch(std::runtime_error e)
  {
    LOG("vosobject3d", 2, "caught exception " << e.what());
  }
}

/// Construct an object3d

class ConstructObject3DTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaObject3D> obj;
  csVector3 pos;
  csMatrix3 ori;
  csVector3 hardpos;
  csMatrix3 hardtrans;
  bool htapplied;

  ConstructObject3DTask(iObjectRegistry *objreg, csMetaObject3D* obj,
                        const csVector3& pos, const csMatrix3& ori,
                        const csVector3& posht, const csMatrix3& htrans);
  virtual ~ConstructObject3DTask();
  virtual void doTask();
};

ConstructObject3DTask::ConstructObject3DTask(iObjectRegistry *objreg,
                                             csMetaObject3D* ob,
                                             const csVector3& p,
                                             const csMatrix3& o,
                                             const csVector3& hpos,
                                             const csMatrix3& htrans)
  : object_reg(objreg), obj(ob, true),
    pos(p), ori(o),
    hardpos(hpos),
    hardtrans(htrans)
{
}

ConstructObject3DTask::~ConstructObject3DTask()
{
}

void ConstructObject3DTask::doTask()
{
  csRef<iMeshWrapper> mw = obj->GetCSinterface()->GetMeshWrapper();

  LOG("vosobject3d", 3, "ConstructObject3DTask: creating " << obj->getURLstr()
      << " at " << pos.x << " " << pos.y << " " << pos.z);
  if(hardpos.x != 0 || hardpos.y != 0 || hardpos.z != 0) {
    LOG("vosobject3d", 3, "setting hard position of " << obj->getURLstr()
        << " to " << hardpos.x << " " << hardpos.y << " " << hardpos.z);
  }

  if (mw.IsValid())
  {
    csReversibleTransform ht(hardtrans, hardpos);
    mw->GetFactory()->HardTransform(ht);

    mw->GetMovable()->SetPosition(pos);
    mw->GetMovable()->SetTransform(ori);
    mw->GetMovable()->UpdateMove();

    csRef<iThingState> thingstate = SCF_QUERY_INTERFACE(
                             mw->GetMeshObject(), iThingState);
    if (thingstate.IsValid()) thingstate->Unprepare();

    obj->setupCollider();
  }
}

// Set position task

class PositionTask : public Task
{
  vRef<csMetaObject3D> obj;
  csVector3 pos;
  bool setCollider;

public:
  PositionTask (csMetaObject3D *o, const csVector3 &p, bool c = false)
    : obj(o, true), pos(p), setCollider(c) {}
  ~PositionTask () {}

  void doTask() { obj->changePosition(pos, setCollider); }
};

// Set orientation task

class OrientateTask : public Task
{
  vRef<csMetaObject3D> obj;
  csMatrix3 ori;
  bool setCollider;

public:
  OrientateTask (csMetaObject3D *o, const csMatrix3 &m, bool c = false)
    : obj(o, true), ori(m), setCollider(c) {}
  ~OrientateTask () {}

  void doTask() { obj->changeOrientation (ori, setCollider); }
};

// Set material task

class MaterialTask : public Task
{
  vRef<csMetaObject3D> obj;
  csRef<iMaterialWrapper> material;

public:
  MaterialTask (csMetaObject3D *o, csRef<iMaterialWrapper> m)
    : obj(o, true), material(m) {}
  ~MaterialTask () {}

  void doTask() { obj->changeMaterial (material); }
};

// Apply force task

class ApplyForceTask : public Task
{
  vRef<csMetaObject3D> obj;
  csVector3 force;

public:
  ApplyForceTask (csMetaObject3D *o, const csVector3 &f)
    : obj(o, true), force(f) {}
  ~ApplyForceTask () {}

  void doTask() {
    LOG("vosobject3d", 3, "add force " << force.x << " " << force.y << " " << force.z);
    csRef<iRigidBody> rb = obj->GetCSinterface()->GetCollider();
    if (rb) {
      obj->GetCSinterface()->GetCollider()->Enable();
      obj->GetCSinterface()->GetCollider()->AddForce(force);
    }
  }
};


// Apply torque task

class ApplyTorqueTask : public Task
{
  vRef<csMetaObject3D> obj;
  csVector3 torque;

public:
  ApplyTorqueTask (csMetaObject3D *o, const csVector3 &t)
    : obj(o, true), torque(t) {}
  ~ApplyTorqueTask () {}

  void doTask() {
    //LOG("vosobject3d", 2, "add torque " << torque.x << " " << torque.y << " " << torque.z);
    csRef<iRigidBody> rb = obj->GetCSinterface()->GetCollider();
    if (rb) {
      obj->GetCSinterface()->GetCollider()->Enable();
      obj->GetCSinterface()->GetCollider()->AddTorque(torque);
    }
  }
};


/// csMetaObject3D ///

csMetaObject3D::csMetaObject3D(VobjectBase* superobject)
    : A3DL::Object3D(superobject),
      alreadyLoaded(false),
      htvalid(false),
      setupCA(false)
{
  csvobj3d = new csVosObject3D(this, superobject);
  csvobj3d->IncRef();
}

csMetaObject3D::~csMetaObject3D()
{
  delete csvobj3d;
}

MetaObject* csMetaObject3D::new_csMetaObject3D(VobjectBase* superobject,
                                               const std::string& type)
{
  return new csMetaObject3D(superobject);
}

Task* csMetaObject3D::GetSetupTask(csVosA3DL* vosa3dl, csVosSector* sect)
{
  double x, y, z;
  try
  {
    getPosition(x, y, z);
  }
  catch(...)
  {
    x = y = z = 0;
  }

  LOG("csMetaObject3D", 3, "got position");

  double a, b, c, d;
  try
  {
    getOrientation(a, b, c, d);
  }
  catch(...)
  {
    a = b = d = 0;
    c = 1;
  }

  LOG("csMetaObject3D", 3, "got orientation ");

  double xht, yht, zht;
  try
  {
    getPositionHT(xht, yht, zht);
    LOG("csMetaObject3D", 3, "got hard position");
  }
  catch(...)
  {
    xht = yht = zht = 0;
  }

  double aht = 0, bht = 0, cht = 1, dht = 0;
  double sxht = 1, syht = 1, szht = 1;
  if(! htvalid) {
    htvalid = true;
    try
    {
      getOrientationHT(aht, bht, cht, dht);
      LOG("csMetaObject3D", 3, "using hard orientation "
          << aht << " " << bht << " " << cht << " " << dht);
    }
    catch(...)
    {
    }

    try
    {
      getScalingHT(sxht, syht, szht);
      LOG("csMetaObject3D", 3, "got hard scaling");
    }
    catch(...)
    {
    }
  }

  csQuaternion q;
  q.SetWithAxisAngle(csVector3(a, b, c), d * M_PI / 180.0);

  csQuaternion qht;
  qht.SetWithAxisAngle(csVector3(aht, bht, cht), dht * M_PI/180.0);

  LOG("vosobject3d", 3, "setting position " << x << " " << y << " " << z);

  return (new ConstructObject3DTask(
            vosa3dl->GetObjectRegistry(), this,
            csVector3(x, y, z),                    // pos
            csMatrix3(q),                          // ori
            csVector3(xht, yht, zht),              // hardpos
            csMatrix3(qht) * csMatrix3(sxht, 0, 0, // hardtrans
                                       0, syht, 0,
                                       0, 0, szht)));
}

void csMetaObject3D::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  LOG("csMetaObject3D", 3, "now in csMetaObject3D::setup");

  this->vosa3dl = vosa3dl;
  sector = sect;

  vosa3dl->mainThreadTasks.push(GetSetupTask(vosa3dl, sect));

  addChildListener (this);
}

void csMetaObject3D::notifyChildInserted (VobjectEvent &event)

{
  LOG ("vosobject3d", 4, "notifyChildInserted " << event.getParent()->getURLstr()
       << " " << event.getContextualName());
  if (event.getContextualName() == "a3dl:position"
      || event.getContextualName() == "a3dl:orientation")
  {
    try
    {
      LOG("vosobject3d", 4, "adding property listener");
      vRef<Property> p = meta_cast<Property> (event.getChild());
      if(p.isValid())
      {
        p->addParentListener (&DoNothingListener::static_);
        p->addPropertyListener (this);
        p->setPriority (Message::LowLatency);
        vRef<RemoteProperty> rp = meta_cast<RemoteProperty> (event.getChild());
        if(rp.isValid())
        {
          rp->enableAsyncReplace (true);
        }
      }
    }
    catch (...)
    {
    }
  }

  if(event.getContextualName() == "a3dl:material") {
    try
    {
      metamaterial = VOS::meta_cast<csMetaMaterial> (getMaterial());
      LOG("vosobject3d", 3, "getting material " << metamaterial.isValid());
      metamaterial->Setup (vosa3dl);

      updateMaterial();
    }
    catch(std::runtime_error& e)
    {
      LOG("vosobject3d", 2, "Got error " << e.what());
    }
  }
}

void csMetaObject3D::notifyChildRemoved (VobjectEvent &event)
{
  LOG ("vosobject3d", 4, "notifyChildRemoved " << event.getContextualName());
  if (event.getContextualName() == "a3dl:position" ||
      event.getContextualName() == "a3dl:orientation")
  {
    try
    {
      vRef<Property> prop = meta_cast<Property> (event.getOldChild());
      if (prop.isValid()) prop->removePropertyListener (this);
    }
    catch (...)
    {
    }
  }
}

void csMetaObject3D::notifyChildReplaced (VobjectEvent &event)
{
  LOG ("vosobject3d", 4, "notifyChildReplaced " << event.getContextualName());

  notifyChildRemoved(event);
  notifyChildInserted(event);
}

void csMetaObject3D::notifyPropertyChange(const PropertyEvent &event)
{
  LOG("vosobject3d", 5, "entered notifyPropertyChange");

  try
  {
    if (event.getEvent() != PropertyEvent::PropertyRead)
    {
      vRef<ParentChildRelation> pcr = event.getProperty()->findParent (this);
      LOG("vosobject3d", 5, "found parent");

      if (pcr->getContextualName() == "a3dl:position")
      {
        double x = 0.0, y = 0.0, z = 0.0;
        getPosition (x,y,z);
        LOG("vosobject3d", 3, getURLstr() << " event value is \"" << event.getValue()
            << "\", prop read is \""
            << event.getProperty()->read() << "\" and getPos() gave us "
            << x << " " << y << " " << z);

        vosa3dl->mainThreadTasks.push (new PositionTask(this,csVector3((float)x,
                                                                       (float)y,
                                                                       (float)z),
                                                        isLocal() || event.getInitiator()->isRemote()));
      }
      else if (pcr->getContextualName() == "a3dl:orientation")
      {
        double x = 0.0, y = 0.0, z = 0.0, angle = 0.0;
        getOrientation (x,y,z,angle);

        LOG("vosobject3d", 3, getURLstr() << " event value is \"" << event.getValue()
            << "\", prop read is \""
            << event.getProperty()->read() << "\" and getPos() gave us "
            << x << " " << y << " " << z << " " << angle);

        csQuaternion q;
        q.SetWithAxisAngle (csVector3((float)x, (float)y, (float)z), angle * M_PI/180.0);
        vosa3dl->mainThreadTasks.push (new OrientateTask(this,csMatrix3(q),
                                                         isLocal() || event.getInitiator()->isRemote()));
      }
    }
  }
  catch (...)
  {}
}

void csMetaObject3D::changePosition (const csVector3 &pos, bool setCollider)
{
  csRef<iMeshWrapper> mw = GetCSinterface()->GetMeshWrapper();

  LOG("vosobject3d", 3, "changePosition: " << getURLstr() <<
      " to " << pos.x << " " << pos.y << " " << pos.z);
  if (mw.IsValid())
  {
    mw->GetMovable()->SetPosition(pos);
    mw->GetMovable()->UpdateMove();
  }

  if (setCollider && csvobj3d->GetCollider().IsValid())
  {
    LOG("vosobject3d", 2, "changing collider position to " << getURLstr() <<
        " to " << pos.x << " " << pos.y << " " << pos.z);

    csvobj3d->GetCollider()->SetPosition(pos);
    csvobj3d->GetCollider()->SetLinearVelocity(csVector3(0));
    csvobj3d->GetCollider()->Enable();
  }
}

void csMetaObject3D::changeOrientation (const csMatrix3 &ori, bool setCollider)
{
  csRef<iMeshWrapper> mw = GetCSinterface()->GetMeshWrapper();

  if (mw.IsValid())
  {
  // NOTE:Movable doesn't support scaling, if it did we'd need to incorporate
  // it into the matrix
    mw->GetMovable()->SetTransform(ori);
    mw->GetMovable()->UpdateMove();
  }

  if (setCollider && csvobj3d->GetCollider().IsValid())
  {
    csvobj3d->GetCollider()->SetOrientation(ori);
    csvobj3d->GetCollider()->SetAngularVelocity(csVector3(0));
    csvobj3d->GetCollider()->Enable();
  }
}

void csMetaObject3D::changeMaterial (iMaterialWrapper* mat)
{
  LOG("vosobject3d", 3, "changing material on " << getURLstr());
  csRef<iMeshWrapper> mw = GetCSinterface()->GetMeshWrapper();

  if (mw.IsValid())
  {
    mw->GetMeshObject()->SetMaterialWrapper(mat);
  }
}

void csMetaObject3D::updateMaterial ()
{
  vosa3dl->mainThreadTasks.push (new MaterialTask(this,
                                                  metamaterial->GetMaterialWrapper()));
}

csRef<csVosObject3D> csMetaObject3D::GetCSinterface()
{
  return csvobj3d;
}

bool csMetaObject3D::supportsPhysics()
{
  return csvobj3d->GetCollider().IsValid();
}

void csMetaObject3D::applyForce(double x, double y, double z)
{
  if (vosa3dl.IsValid()) {
    CS_ASSERT(x == x);
    vosa3dl->mainThreadTasks.push (new ApplyForceTask(this,
                                                      csVector3((float)x,
                                                                (float)y,
                                                                (float)z)));
  }
}

void csMetaObject3D::applyTorque(double x, double y, double z)
{
  if (vosa3dl.IsValid()) {
    CS_ASSERT(x == x);
    vosa3dl->mainThreadTasks.push (new ApplyTorqueTask(this,
                                                       csVector3((float)x,
                                                                 (float)y,
                                                                 (float)z)));
  }
}

void csMetaObject3D::getLinearVelocity(double& x, double& y, double& z)
{
  if (csvobj3d.IsValid()) {
    csRef<iRigidBody> rb = csvobj3d->GetCollider();
    if (rb) {
      csVector3 v = rb->GetLinearVelocity();
      x = v.x;
      y = v.y;
      z = v.z;
    } else {
      x = 0;
      y = 0;
      z = 0;
    }
  }
}

void csMetaObject3D::getAngularVelocity(double& x, double& y, double& z)
{
  if (csvobj3d.IsValid()) {
    csRef<iRigidBody> rb = csvobj3d->GetCollider();
    if (rb) {
      csVector3 v = rb->GetAngularVelocity();
      x = v.x;
      y = v.y;
      z = v.z;
    } else {
      x = 0;
      y = 0;
      z = 0;
    }
  }
}

// Set position task

class MoveToTask : public Task
{
  vRef<csMetaObject3D> obj;
  csVector3 pos;
  float timestep;

public:
  MoveToTask (csMetaObject3D *o, const csVector3 &p, float _timestep)
    : obj(o, true), pos(p), timestep(_timestep) {}
  ~MoveToTask () {}

  void doTask() { obj->doMoveTo(pos, timestep); }
};

void csMetaObject3D::doMoveTo(const csVector3& vec, float timestep)
{
  collider_actor.Move(timestep, 1.0, vec, csVector3());
  csVector3 pos = csvobj3d->GetMeshWrapper()->GetMovable()->GetPosition();
  setPosition(pos.x, pos.y, pos.z);
}

void csMetaObject3D::moveTo(double x, double y, double z, double timestep)
{
  if (setupCA) {
    //LOG("object3d", 1, "vec is " << x << " " << y << " " << z << " ts is " << timestep);
    vosa3dl->mainThreadTasks.push (new MoveToTask(this,
                                                  csVector3((float)x,
                                                            (float)y,
                                                            (float)z),
                                                  timestep));
  }
}

void csMetaObject3D::setupCollider()
{
  csRef<iEngine> engine = CS_QUERY_REGISTRY(vosa3dl->GetObjectRegistry(), iEngine);
  collider_actor.SetEngine(engine);
  collider_actor.SetCollideSystem(sector->GetCollideSystem());

  csRef<iPolygonMesh> pm = SCF_QUERY_INTERFACE(
    csvobj3d->GetMeshWrapper()->GetMeshObject()->GetObjectModel()->GetPolygonMeshColldet(),
    iPolygonMesh);

  (new csColliderWrapper(csvobj3d->GetMeshWrapper()->QueryObject(),
                         sector->GetCollideSystem(), pm))->DecRef();

  csVector3 bbox = csvobj3d->GetMeshWrapper()->GetWorldBoundingBox().GetSize();
  collider_actor.InitializeColliders(csvobj3d->GetMeshWrapper(), csVector3(bbox.x, bbox.y/2, bbox.z),
                                     csVector3(bbox.x, bbox.y/2, bbox.z), csVector3(0, -bbox.y/2, 0));
  collider_actor.SetGravity(1);

  setupCA = true;
}


