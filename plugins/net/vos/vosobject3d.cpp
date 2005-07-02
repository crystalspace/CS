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
#include "csgeom/quaterni.h"

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
  if(rc) {
    rc->acquire();
    object3d.assign(obj3d, false);
  } else object3d.assign(obj3d, true);

  SCF_CONSTRUCT_IBASE (0);
  meshwrapper = NULL;
  collider = NULL;
}

csVosObject3D::~csVosObject3D()
{
  if (meshwrapper.IsValid())
  {
    csRef<iEngine> engine = CS_QUERY_REGISTRY (object3d->getVosA3DL()->GetObjectRegistry(),
                                               iEngine);
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
  LOG ("csVosObject3D", 3, "Received Execute callback 1 arg");
  //csVector3 pos = collider->GetPosition();
  //csVector3 vel = collider->GetLinearVelocity();
  //csVector3 acc = collider->GetForce();

  object3d->setPosition(t.GetOrigin().x, t.GetOrigin().y, t.GetOrigin().z);
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
  }
}

// Set position task

class PositionTask : public Task
{
  vRef<csMetaObject3D> obj;
  csVector3 pos;

public:
  PositionTask (csMetaObject3D *o, const csVector3 &p) : obj(o, true), pos(p) {}
  ~PositionTask () {}

  void doTask() { obj->changePosition(pos); }
};

// Set orientation task

class OrientateTask : public Task
{
  vRef<csMetaObject3D> obj;
  csMatrix3 ori;

public:
  OrientateTask (csMetaObject3D *o, const csMatrix3 &m)
    : obj(o, true), ori(m) {}
  ~OrientateTask () {}

  void doTask() { obj->changeOrientation (ori); }
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


/// csMetaObject3D ///

csMetaObject3D::csMetaObject3D(VobjectBase* superobject)
    : A3DL::Object3D(superobject),
      alreadyLoaded(false),
      htvalid(false)
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
  if (event.getContextualName() == "a3dl:position" ||
      event.getContextualName() == "a3dl:orientation")
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
                                                                       (float)z)));
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
        vosa3dl->mainThreadTasks.push (new OrientateTask(this,csMatrix3(q)));
      }
    }
  }
  catch (...)
  {}
}

void csMetaObject3D::changePosition (const csVector3 &pos)
{
  csRef<iMeshWrapper> mw = GetCSinterface()->GetMeshWrapper();

  LOG("vosobject3d", 2, "changePosition: " << getURLstr() <<
      " to " << pos.x << " " << pos.y << " " << pos.z);
  if (mw.IsValid())
  {
    mw->GetMovable()->SetPosition(pos);
    mw->GetMovable()->UpdateMove();
  }

  if (csvobj3d->GetCollider().IsValid())
  {
    if (csvobj3d->GetCollider()->GetPosition() != pos)
    {
      LOG("vosobject3d", 2, "changing collider position to " << getURLstr() <<
          " to " << pos.x << " " << pos.y << " " << pos.z);

      csvobj3d->GetCollider()->SetPosition(pos);
      csvobj3d->GetCollider()->SetLinearVelocity(csVector3(0));
    }
  }
}

void csMetaObject3D::changeOrientation (const csMatrix3 &ori)
{
  csRef<iMeshWrapper> mw = GetCSinterface()->GetMeshWrapper();

  if (mw.IsValid())
  {
  // NOTE:Movable doesn't support scaling, if it did we'd need to incorporate
  // it into the matrix
    mw->GetMovable()->SetTransform(ori);
    mw->GetMovable()->UpdateMove();
  }

  if (csvobj3d->GetCollider().IsValid())
  {
    csvobj3d->GetCollider()->SetOrientation(ori);
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
