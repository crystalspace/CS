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
#include "csutil/event.h"
#include "ivaria/dynamics.h"
#include "ivaria/ode.h"

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

using namespace VUtil;
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
  csRef<iProgressMeter> meter;

  RelightTask(csVosA3DL* va, iProgressMeter* m = 0);
  virtual ~RelightTask() { }
  virtual void doTask();
};

RelightTask::RelightTask(csVosA3DL* va, iProgressMeter* m)
  : vosa3dl(va), meter(m)
{
}

void RelightTask::doTask()
{
  iObjectRegistry* objreg = vosa3dl->GetObjectRegistry();
  csRef<iEngine> engine = CS_QUERY_REGISTRY(objreg, iEngine);

  LOG ("RelightTask", 2, "Performing relight");
  engine->ForceRelight(0, meter);
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
  vRef<Vobject> v = Vobject::findObjectFromRoot(s);
  vRef<csMetaSector> sec = meta_cast<csMetaSector>(v);
  if(! sec) return csRef<iVosSector>();

  csRef<iVosSector> r = sec->GetCsVosSector();
  if(! r.IsValid()) r.AttachNew(new csVosSector(objreg, this, sec));

  return r;
}

#define REPLACE_FACTORY(type, cls, oldfac, newfac)     \
  Site::removeRemoteMetaObjectFactory(type, &oldfac); \
  Site::removeLocalMetaObjectFactory(type, &oldfac); \
  Site::removeLocalMetaObjectFactory(typeid(cls).name(), &oldfac);  \
  Site::addRemoteMetaObjectFactory(type, type, &newfac); \
  Site::addLocalMetaObjectFactory(type, &newfac); \
  Site::addLocalMetaObjectFactory(typeid(cls).name(), &newfac);


bool csVosA3DL::Initialize (iObjectRegistry *o)
{
  LOG("csVosA3DL", 2, "Initializing");

  REPLACE_FACTORY("a3dl:object3D", A3DL::Object3D,
                  A3DL::Object3D::new_Object3D,
                  csMetaObject3D::new_csMetaObject3D);

  REPLACE_FACTORY("a3dl:object3D.cube", A3DL::Cube,
                  A3DL::Cube::new_Cube,
                  csMetaCube::new_csMetaCube);

  REPLACE_FACTORY("a3dl:object3D.cone", A3DL::Cone,
                  A3DL::Cone::new_Cone,
                  csMetaCone::new_csMetaCone);

  REPLACE_FACTORY("a3dl:object3D.clone", A3DL::Clone,
                  A3DL::Clone::new_Clone,
                  csMetaClone::new_csMetaClone);

  REPLACE_FACTORY("a3dl:object3D.polygonmesh", A3DL::PolygonMesh,
                  A3DL::PolygonMesh::new_PolygonMesh,
                  csMetaPolygonMesh::new_csMetaPolygonMesh);

  REPLACE_FACTORY("a3dl:object3D.billboard", A3DL::Billboard,
                  A3DL::Billboard::new_Billboard,
                  csMetaBillboard::new_csMetaBillboard);

  REPLACE_FACTORY("a3dl:object3D.model", A3DL::Model,
                  A3DL::Model::new_Model,
                  csMetaModel::new_csMetaModel);

  REPLACE_FACTORY("a3dl:object3D.sphere", A3DL::Sphere,
                  A3DL::Sphere::new_Sphere,
                  csMetaSphere::new_csMetaSphere);

  REPLACE_FACTORY("a3dl:object3D.cylinder", A3DL::Cylinder,
                  A3DL::Cylinder::new_Cylinder,
                  csMetaCylinder::new_csMetaCylinder);

  REPLACE_FACTORY("a3dl:texture", A3DL::Texture,
                  A3DL::Texture::new_Texture,
                  csMetaTexture::new_csMetaTexture);

  REPLACE_FACTORY("a3dl:material", A3DL::Material,
                  A3DL::Material::new_Material,
                  csMetaMaterial::new_csMetaMaterial);

  REPLACE_FACTORY("a3dl:light", A3DL::Light,
                  A3DL::Light::new_Light,
                  csMetaLight::new_csMetaLight);

  REPLACE_FACTORY("a3dl:sector", A3DL::Sector,
                  A3DL::Sector::new_Sector,
                  csMetaSector::new_csMetaSector);

  objreg = o;

  csMetaMaterial::object_reg = objreg;

  eventq = CS_QUERY_REGISTRY (objreg, iEventQueue);
  if (! eventq)
  {
    LOG("csVosA3DL", 1, "Error initializing: no event queue in registry!");
    return false;
  }
  eventq->RegisterListener (this, CSMASK_FrameProcess);

  localsite.assign(new Site(true), false);
  //localsite->addSiteExtension(new LocalSocketSiteExtension());
  localsite->addSiteExtension(new LocalVipSiteExtension());

  clock = CS_QUERY_REGISTRY (objreg, iVirtualClock);

  csRef<iDynamics> dynamics;
#if 0 // dynamics isn't ready yet
  CS_QUERY_REGISTRY_PLUGIN(dynamics, objreg,
                           "crystalspace.dynamics.ode",
                           iDynamics);
#endif

  if (dynamics)
  {
    LOG("csVosA3DL", 2, "Initializing dynamics system");
    dynsys = dynamics->CreateSystem();
    dynsys->SetGravity (csVector3 (0, .1, 0));
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
  if (ev.Type == csevBroadcast &&
    csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    double start = getRealTime();

    for(unsigned int n = mainThreadTasks.size(); n > 0 && getRealTime() < (start+.5); n--)
    {
      LOG("csVosA3DL", 3, "starting main thread task");
      Task* t = mainThreadTasks.pop();
      t->doTask();
      delete t;
      LOG("csVosA3DL", 3, "completed main thread task");
    }


    if (dynsys) {
      //LOG("csVosA3DL", 2, "Stepping dynamic system");

      // First get elapsed time from the virtual clock.
      csTicks elapsed_time = clock->GetElapsedTicks ();
      const float speed = elapsed_time / 1000.0;

      // Take small steps.
      const float maxStep = 0.01f;
      float ta = 0;
      float tb = speed;
      int maxSteps=4;
      while (ta < speed && maxSteps)
      {
        if (tb - ta > maxStep) tb = ta + maxStep;

        dynsys->Step (tb - ta);
        ta = tb;
        tb = speed;

        maxSteps--;
      }
    }

    start = getRealTime();

    for(unsigned int n = mainThreadTasks.size(); n > 0 && getRealTime() < (start+.5); n--)
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


vRef<Vobject> csVosA3DL::GetVobject()
{
  return localsite;
}

void csVosA3DL::setProgressMeter(iProgressMeter* pm)
{
  progress = pm;
}

void csVosA3DL::incrementRelightCounter()
{
  boost::mutex::scoped_lock lk (relightCounterMutex);
  relightCounter++;
  LOG ("csVosA3DL", 3, "relight counter incremented to " << relightCounter);
}

void csVosA3DL::decrementRelightCounter()
{
  boost::mutex::scoped_lock lk (relightCounterMutex);
  if (--relightCounter == 0)
  {
    mainThreadTasks.push(new RelightTask (this, progress));
  }
  LOG ("csVosA3DL", 3, "relight counter decremented to " << relightCounter);
}


