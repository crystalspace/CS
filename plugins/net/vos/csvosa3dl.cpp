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
#include "csvosa3dl.h"
#include "vossector.h"
#include "vosobject3d.h"
#include "voscube.h"
#include "voscone.h"
#include "vosbillboard.h"
#include "vostexture.h"
#include "vosmaterial.h"
#include "vospolygonmesh.h"
#include "voslight.h"
#include "vosmodel.h"
#include "vosclone.h"
#include "vossphere.h"
#include "voscylinder.h"

#include <vos/metaobjects/a3dl/a3dl.hh>

using namespace VOS;
using namespace A3DL;

SCF_IMPLEMENT_IBASE (csVosA3DL)
  SCF_IMPLEMENTS_INTERFACE (iVosApi)
  SCF_IMPLEMENTS_INTERFACE (iVosA3DL)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csVosA3DL)

/// Relight task ///
class RelightTask : public Task
{
public:
  csVosA3DL* vosa3dl;

  RelightTask(csVosA3DL* va);
  virtual ~RelightTask() { }
  virtual void doTask();
};

RelightTask::RelightTask(csVosA3DL* va)
  : vosa3dl(va)
{
}

void RelightTask::doTask()
{
  csRef<iObjectRegistry> objreg = vosa3dl->GetObjectRegistry();
  csRef<iEngine> engine = CS_QUERY_REGISTRY(objreg, iEngine);

  LOG ("RelightTask", 2, "Performing relight");
  engine->ForceRelight();
  LOG ("RelightTask", 2, "Done");
}


/// csVosA3DL ///

csVosA3DL::csVosA3DL (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  relightCounter = 0;
}

csVosA3DL::~csVosA3DL()
{
  SCF_DESTRUCT_IBASE();
}

csRef<iVosSector> csVosA3DL::GetSector(const char* s)
{
  csRef<iVosSector> r;
  r.AttachNew(new csVosSector(objreg, this, s));
  return r;
}

bool csVosA3DL::Initialize (iObjectRegistry *o)
{
  LOG("csVosA3DL", 2, "Initializing");

  Site::removeRemoteMetaObjectFactory("a3dl:object3D",
                &A3DL::Object3D::new_Object3D);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.cube",
                &A3DL::Cube::new_Cube);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.cone",
                &A3DL::Cone::new_Cone);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.polygonmesh",
                &A3DL::PolygonMesh::new_PolygonMesh);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.billboard",
                &A3DL::Billboard::new_Billboard);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.model",
                &A3DL::Billboard::new_Billboard);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.clone",
                &A3DL::Clone::new_Clone);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.sphere",
                &A3DL::Sphere::new_Sphere);
  Site::removeRemoteMetaObjectFactory("a3dl:object3D.cylinder",
                &A3DL::Cylinder::new_Cylinder);
  Site::removeRemoteMetaObjectFactory("a3dl:texture",
                &A3DL::Texture::new_Texture);
  Site::removeRemoteMetaObjectFactory("a3dl:material",
                &A3DL::Material::new_Material);
  Site::removeRemoteMetaObjectFactory("a3dl:light",
                &A3DL::Light::new_Light);

  Site::addRemoteMetaObjectFactory("a3dl:object3D", "a3dl:object3D",
                                   &csMetaObject3D::new_csMetaObject3D);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.cube", "a3dl:object3D.cube",
                                   &csMetaCube::new_csMetaCube);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.cone", "a3dl:object3D.cone",
                                   &csMetaCone::new_csMetaCone);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.clone",
                                   "a3dl:object3D.clone",
                                   &csMetaClone::new_csMetaClone);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.polygonmesh",
                                   "a3dl:object3D.polygonmesh",
                                   &csMetaPolygonMesh::new_csMetaPolygonMesh);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.billboard",
                                   "a3dl:object3D.billboard",
                                   &csMetaBillboard::new_csMetaBillboard);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.model", "a3dl:object3D.model",
                                   &csMetaModel::new_csMetaModel);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.sphere",
                                   "a3dl:object3D.sphere",
                                   &csMetaSphere::new_csMetaSphere);
  Site::addRemoteMetaObjectFactory("a3dl:object3D.cylinder",
                                   "a3dl:object3D.cylinder",
                                   &csMetaCylinder::new_csMetaCylinder);
  Site::addRemoteMetaObjectFactory("a3dl:texture", "a3dl:texture",
                                   &csMetaTexture::new_csMetaTexture);
  Site::addRemoteMetaObjectFactory("a3dl:material", "a3dl:material",
                                   &csMetaMaterial::new_csMetaMaterial);
  Site::addRemoteMetaObjectFactory("a3dl:light", "a3dl:light",
                                   &csMetaLight::new_csMetaLight);

  objreg = o;

  csMetaMaterial::object_reg = objreg;

  eventq = CS_QUERY_REGISTRY (objreg, iEventQueue);
  if (! eventq) return false;
  eventq->RegisterListener (this, CSMASK_FrameProcess);

  localsite.assign(new Site(true), false);
  localsite->addSiteExtension(new LocalSocketSiteExtension());

  csRef<iDynamics> dynamics = CS_QUERY_REGISTRY (objreg, iDynamics);
  if (dynamics)
  {
    LOG("csVosA3DL", 2, "Initializing dynamics system");
    dynsys = dynamics->CreateSystem();
  }
  else
  {
    LOG("csVosA3DL", 2, "Not using dynamics system");
    dynsys = NULL;
  }

  return true;
}

bool csVosA3DL::HandleEvent (iEvent &ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    while(! mainThreadTasks.empty())
    {
      LOG("csVosA3DL", 3, "starting main thread task");
      Task* t = mainThreadTasks.pop();
      t->doTask();
      delete t;
      LOG("csVosA3DL", 3, "completed main thread task");
    }
  }
  return false;
}


VOS::vRef<Vobject> csVosA3DL::GetVobject()
{
  return localsite;
}

void csVosA3DL::incrementRelightCounter()
{
  boost::mutex::scoped_lock lk (relightCounterMutex);
  relightCounter++;
  LOG ("csVosA3DL", 2, "relight counter incremented to " << relightCounter);
}

void csVosA3DL::decrementRelightCounter()
{
  boost::mutex::scoped_lock lk (relightCounterMutex);
  if (--relightCounter == 0)
  {
  mainThreadTasks.push(new RelightTask (this));
  }
  LOG ("csVosA3DL", 2, "relight counter decremented to " << relightCounter);
}
