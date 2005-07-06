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
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "imesh/mdlconv.h"
#include "imesh/crossbld.h"
#include "cstool/mdltool.h"
#include "csutil/databuf.h"
#include "igeom/objmodel.h"
#include "iutil/plugin.h"
#include "ivaria/dynamics.h"

#include "csvosa3dl.h"
#include "vosmodel.h"

using namespace VUtil;
using namespace VOS;

class SetModelAnimTask : public Task
{
public:
  vRef<csMetaModel> model;
  char* action;
  A3DL::ActionEvent::EventType evtype;

  SetModelAnimTask(csMetaModel* m, const char* a, A3DL::ActionEvent::EventType et)
    : model(m, true), evtype(et)
    {
      action = strdup(a);
    }

  virtual ~SetModelAnimTask()
    {
      free(action);
    }

  virtual void doTask()
    {
      if(model->GetCSinterface()->GetMeshWrapper().IsValid()) {
        csRef<iSprite3DState> spstate = SCF_QUERY_INTERFACE(
          model->GetCSinterface()->GetMeshWrapper()->GetMeshObject (),
          iSprite3DState);

        if(evtype == A3DL::ActionEvent::SetActionCycle) {
          spstate->SetAction (action);
        } else if(evtype == A3DL::ActionEvent::DoActionOnce) {
          spstate->SetOverrideAction (action);
        }
      }
    }
};


/// ConstructModelTask ///

class ConstructModelTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaModel> model;
  csRef<iSector> sector;
  csRef<iDynamicSystem> dynsys;
  csVector3 startingPos;

  ConstructModelTask(iObjectRegistry *objreg,
                     csMetaModel* m,
                     iSector *s,
                     iDynamicSystem* d,
                     const csVector3& startingPos);
  virtual ~ConstructModelTask();
  virtual void doTask();
};

ConstructModelTask::ConstructModelTask (iObjectRegistry *objreg,
                                        csMetaModel* m,
                                        iSector *s,
                                        iDynamicSystem* d,
                                        const csVector3& p)
  : object_reg(objreg), model(m, true), sector(s), dynsys(d), startingPos(p)
{
}

ConstructModelTask::~ConstructModelTask()
{
}

#ifndef _MAX
# define _MAX(a, b) ((a > b) ? a : b)
#endif

static void NormalizeModel(csRef<iMeshWrapper> wrapper, bool recenter,
                           bool fixalign, std::string datatype)
{
  csBox3 b;

  if (!wrapper || !wrapper->GetMeshObject() ||
      !wrapper->GetMeshObject()->GetObjectModel())
  {
    LOG ("NormalizeModel", 2, "Bad factory wrapper passed to function");
    return;
  }

  wrapper->GetMeshObject()->GetObjectModel()->GetObjectBoundingBox(b);

  LOG("NormalizeModel", 3, "maxes (" << b.MaxX() << ", " << b.MaxY() <<
      ", " << b.MaxZ() << ")");
  LOG("NormlizeModel", 3, "mins (" << b.MinX() << ", " << b.MinY() <<
      ", " << b.MinZ() << ")");

  float xextent = b.MaxX() - b.MinX();
  float yextent = b.MaxY() - b.MinY();
  float zextent = b.MaxZ() - b.MinZ();

  float scale = 1.0 / _MAX(xextent, _MAX(yextent, zextent));
  LOG("NormalizeModel", 3, "scaling extents of (" << xextent <<
      ", " << yextent << ", " << zextent << ") by " << scale);

  csVector3 newcenter(0,0,0);
  if (recenter)
  {
    newcenter.Set((b.MaxX() + b.MinX())/2,
                  (b.MaxY() + b.MinY())/2,
                  (b.MaxZ() + b.MinZ())/2);
    newcenter *= -scale;
    LOG("NormalizeModel", 4, "setting center to " << newcenter.x <<
        ", " << newcenter.y << ", " << newcenter.z << ")");
  }

  scale = 1.0 / scale;
  csReversibleTransform rt (csZScaleMatrix3 (scale) * csYScaleMatrix3 (scale) *
                            csXScaleMatrix3 (scale), newcenter);

  if (fixalign)
  {
    if(datatype == "model/md2") rt.SetO2T(csYRotMatrix3(-M_PI/2) * rt.GetO2T());
  }

  wrapper->GetFactory()->HardTransform (rt);
}
#undef _MAX

void ConstructModelTask::doTask()
{
  LOG("vosmodel", 3, "Constructing model");



  // Engine
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  // Get the model converter and cross builder
  csRef<iModelConverter> modconv;
  csRef<iCrossBuilder> xbuild;
  CS_QUERY_REGISTRY_PLUGIN(modconv, object_reg, "crystalspace.modelconverter.multiplexer", iModelConverter);
  CS_QUERY_REGISTRY_PLUGIN(xbuild, object_reg, "crystalspace.mesh.crossbuilder", iCrossBuilder);

  // Check they were loaded
  if (!modconv || !xbuild)
  {
    LOG ("ConstructModelTask", 2, "Failed to load model converter and cross builder plugins, " <<
         "ignoring model (" << model->getURLstr() << ")");
    return;
  }

  LOG ("ConstructModelTask", 3, "Loading into model converter");
  csRef<iModelData> data = modconv->Load (model->getDatabuf()->GetUint8(),
                                          model->getDatabuf()->GetSize());
  if (!data)
  {
    LOG ("ConstructModelTask", 2, "Could not load model using converter");
    return;
  }

  LOG ("ConstructModelTask", 3, "Splitting objects");
  csModelDataTools::SplitObjectsByMaterial (data);
  csModelDataTools::MergeObjects (data, false);

  LOG ("ConstructModelTask", 3, "Creating factory");
  csRef<iMeshFactoryWrapper> factory;
  if(model->getMetaMaterial().isValid()) {
    factory = xbuild->BuildSpriteFactoryHierarchy (
      data, engine, model->getMetaMaterial()->GetMaterialWrapper());
  }
  else
  {
    factory = xbuild->BuildSpriteFactoryHierarchy (
      data, engine, csMetaMaterial::GetCheckerboard());
  }

  if (!factory)
  {
    LOG ("ConstructModelTask", 2, "Could not build factory");
    return;
  }
  else factory->QueryObject()->SetName (model->getURLstr().c_str());

  csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper (
    factory, model->getURLstr().c_str(), sector, startingPos);

  csRef<iSprite3DFactoryState> fac3d = SCF_QUERY_INTERFACE(
    meshwrapper->GetMeshObject()->GetFactory(),
    iSprite3DFactoryState);

  //if(fac3d.IsValid()) fac3d->MergeNormals();

  NormalizeModel (meshwrapper, true, true, model->getModelDatatype());


  if (dynsys && !model->GetCSinterface()->GetCollider())
  {
    // Create a body and attach the mesh.
    csRef<iRigidBody> rb = dynsys->CreateBody ();
    rb->SetProperties (1, csVector3 (0), csMatrix3 ());
    rb->SetPosition (startingPos);
    rb->AttachMesh (meshwrapper);
    rb->SetMoveCallback(model->GetCSinterface());

    csOrthoTransform t;
    rb->AttachColliderMesh(meshwrapper, t, 10, 1, 0);

    model->GetCSinterface()->SetCollider (rb);
  }

  model->GetCSinterface()->SetMeshWrapper(meshwrapper);

  if(model->getAction() != "") {
    SetModelAnimTask smat(model, model->getAction().c_str(), model->getEventType());
    smat.doTask();
  }
}


/// csMetaModel ///

csMetaModel::csMetaModel(VobjectBase* superobject)
  : A3DL::Object3D(superobject),
    csMetaObject3D(superobject),
    A3DL::Model(superobject),
    alreadyLoaded(false),
    valid(false)
{
}

MetaObject* csMetaModel::new_csMetaModel(VobjectBase* superobject,
  const std::string& type)
{
  return new csMetaModel(superobject);
}

csRef<iDataBuffer> csMetaModel::getDatabuf() { return databuf; }

void csMetaModel::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  LOG("csMetaModel", 3, "entered setup for " << getURLstr());

  vRef<A3DL::Actor> actor = meta_cast<A3DL::Actor>(this);
  if(actor.isValid()) actor->addActionListener(this);

  LOG("csMetaModel", 3, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
}

void csMetaModel::notifyActionChange(const A3DL::ActionEvent& ae)
{
  LOG("csMetaModel", 4, "Got notifyActionChange with action " << ae.getAction());

  eventType = ae.getEventType();
  action = ae.getAction();

  if(GetCSinterface()->GetMeshWrapper().IsValid())
  {
    vosa3dl->mainThreadTasks.push(new SetModelAnimTask(this, ae.getAction().c_str(),
                                                       ae.getEventType()));
  }
}

void csMetaModel::notifyChildInserted(VobjectEvent& e)
{
  csMetaObject3D::notifyChildInserted(e);

  if(e.getContextualName() == "a3dl:model"
     || e.getContextualName() == "a3dl:hardorientation")
  {
    vRef<VOS::Property> p = VOS::meta_cast<VOS::Property>(e.getChild());
    if(p.isValid()) p->addPropertyListener(this);
  }

  if(!valid && metamaterial.isValid() && databuf.IsValid())
  {
    valid = true;
    htvalid = false;

    double x, y, z;
    getPosition(x, y, z);

    // Create task
    vosa3dl->mainThreadTasks.push(new ConstructModelTask (vosa3dl->GetObjectRegistry(),
                                                          this, sector->GetSector(),
                                                          sector->GetDynSys(),
                                                          csVector3(x, y, z)));
    vosa3dl->mainThreadTasks.push(GetSetupTask(vosa3dl, sector));
  }
}

void csMetaModel::notifyChildRemoved(VobjectEvent& e)
{
  csMetaObject3D::notifyChildRemoved(e);

  if(e.getContextualName() == "a3dl:model") {
    vRef<VOS::Property> p = VOS::meta_cast<VOS::Property>(e.getChild());
    if(p.isValid()) p->removePropertyListener(this);
  }
}

void csMetaModel::notifyPropertyChange(const PropertyEvent& event)
{
  csMetaObject3D::notifyPropertyChange(event);

  try
  {
    VUtil::vRef<ParentChildRelation> pcr = event.getProperty()->findParent(this);
    if(pcr->getContextualName() == "a3dl:model")
    {
      vRef<Property> property = getModelObj();

      LOG("vosmodel", 2, "model data size " << event.getNewValue().size());

      // Create databuffer for model.  Need a new buffer for this as CS will
      // delete[] it
      char *buffer = new char[event.getNewValue().size()];
      for (std::string::size_type i = 0; i < event.getNewValue().size(); i++)
        buffer[i] = event.getNewValue()[i];
      databuf.AttachNew (new csDataBuffer (buffer, event.getNewValue().size()));

      modeldatatype = event.getDataType();

      valid = false;
    }
    if (pcr->getContextualName() == "a3dl:hardorientation")
    {
      valid = false;
    }
  } catch(std::runtime_error& e) {
    LOG("csMetaModel", 2, "error " << e.what());
  }

  if(!valid && metamaterial.isValid() && databuf.IsValid())
  {
    valid = true;
    htvalid = false;

    double x, y, z;
    getPosition(x, y, z);

    // Create task
    vosa3dl->mainThreadTasks.push(new ConstructModelTask (vosa3dl->GetObjectRegistry(),
                                                          this, sector->GetSector(),
                                                          sector->GetDynSys(),
                                                          csVector3(x, y, z)));
    vosa3dl->mainThreadTasks.push(GetSetupTask(vosa3dl, sector));
  }
}
