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
#include "voslight.h"
#include "iengine/movable.h"

#include "vos/metaobjects/a3dl/a3dl.hh"

using namespace VOS;

SCF_IMPLEMENT_IBASE (csVosSector)
  SCF_IMPLEMENTS_INTERFACE (iVosSector)
  SCF_IMPLEMENTS_INTERFACE (iVosApi)
SCF_IMPLEMENT_IBASE_END

/// Relight task ///
class RelightTask : public Task
{
public:
  csVosA3DL* vosa3dl;
  csRef<csVosSector> sector;

  RelightTask(csVosA3DL* va, csVosSector* vs);
  virtual ~RelightTask() { }
  virtual void doTask();
};

RelightTask::RelightTask(csVosA3DL* va, csVosSector* vs)
  : vosa3dl(va), sector(vs)
{
}

void RelightTask::doTask()
{
  if (sector->isLit) return;

  csRef<iObjectRegistry> objreg = vosa3dl->GetObjectRegistry();
  csRef<iEngine> engine = CS_QUERY_REGISTRY(objreg, iEngine);

  LOG ("RelightTask", 2, "Performing relight");
  engine->ForceRelight();
  sector->isLit = true;
}


/// Set ambient task ///
class SetAmbientTask : public Task
{
public:
  csVosA3DL* vosa3dl;
  csRef<csVosSector> sector;

  SetAmbientTask(csVosA3DL* va, csVosSector* vs);
  virtual ~SetAmbientTask() { }
  virtual void doTask();
};

SetAmbientTask::SetAmbientTask(csVosA3DL* va, csVosSector* vs)
  : vosa3dl(va), sector(vs)
{
}

void SetAmbientTask::doTask()
{
  iObjectRegistry *objreg = vosa3dl->GetObjectRegistry();
  csRef<iEngine> engine = CS_QUERY_REGISTRY(objreg, iEngine);
  engine->SetAmbientLight(csColor(.2, .2, .2));
}

/// Load object task ///
class LoadObjectTask : public Task
{
public:
  csVosA3DL* vosa3dl;
  vRef<csMetaObject3D> obj3d;
  csRef<csVosSector> sector;
  bool toRemove;

  LoadObjectTask(csVosA3DL* va, csMetaObject3D *o, csVosSector* vs, bool rem);
  virtual ~LoadObjectTask();
  virtual void doTask();
};

LoadObjectTask::LoadObjectTask (csVosA3DL *va, csMetaObject3D *o, csVosSector *vs, bool rem)
    : vosa3dl(va), obj3d(o, true), sector(vs), toRemove (rem)
{
}

LoadObjectTask::~LoadObjectTask ()
{
}

void LoadObjectTask::doTask()
{
  csRef<iMeshWrapper> wrapper = obj3d->GetCSinterface()->GetMeshWrapper();
  if (wrapper)
  {
    if (toRemove)
    {
      wrapper->GetMovable()->GetSectors()->Remove (sector->GetSector());
    }
    else
    {
      if (wrapper->GetMovable()->GetSectors()->Find (sector->GetSector()))
      {
        LOG("LoadObjectTask", 3, "Object already setup and in sector");
      }
      else
      {
        LOG("LoadObjectTask", 3, "Object already setup, setting sector");
        wrapper->GetMovable()->GetSectors()->Add(sector->GetSector());
        wrapper->GetMovable()->UpdateMove();
      }
    }
  }
  else
  {
    if (toRemove)
    {
      LOG("LoadObjectTask", 2, "Attempting to remove empty meshwrapper!");
    }
/*
    else
    {
      LOG("LoadObjectTask", 2, "Setting up object3D")
        obj3d->Setup(vosa3dl, sector);
    }
*/
  }
}

/// Load light task ///
class LoadLightTask : public Task
{
public:
  csVosA3DL* vosa3dl;
  vRef<csMetaLight> light;
  csRef<csVosSector> sector;

  LoadLightTask(csVosA3DL* va, csMetaLight *o, csVosSector* vs);
  virtual ~LoadLightTask();
  virtual void doTask();
};

LoadLightTask::LoadLightTask (csVosA3DL *va, csMetaLight *l, csVosSector *vs)
    : vosa3dl(va), light(l, true), sector(vs)
{
}

LoadLightTask::~LoadLightTask ()
{
}

void LoadLightTask::doTask()
{
  try
  {
    light->Setup(vosa3dl, sector);
  }
  catch(std::runtime_error& e)
  {
    LOG("LoadLightTask", 2, "Light Setup emitted error: " << e.what());
  }
}


/// csVosSector ///

csVosSector::csVosSector(iObjectRegistry *o, csVosA3DL* va, const char* s)
{
  SCF_CONSTRUCT_IBASE(0);
  objreg = o;
  url = strdup(s);
  vosa3dl = va;
  engine = CS_QUERY_REGISTRY (objreg, iEngine);
  sector = engine->CreateSector(s);
  isLit = false;
  waitingForChildren = 0;
}

csVosSector::~csVosSector()
{
  free(url);
  SCF_DESTRUCT_IBASE();
}

void csVosSector::Load()
{
  LOG("csVosSector", 2, "Starting sector load");

  sectorvobj = meta_cast<A3DL::Sector>(Vobject::findObjectFromRoot(url));
  waitingForChildren = sectorvobj->getChildren().size();
  sectorvobj->addChildListener (this);

  LOG("csVosSector", 2, "Started sector load");
}

void csVosSector::notifyChildInserted (VobjectEvent &event)
{
  LOG("csVosSector", 2, "notifyChildInserted");
  vRef<csMetaObject3D> obj3d = meta_cast<csMetaObject3D>(event.getChild());
  LOG("SectorChildInserted", 2, "Looking at " << event.getChild()->getURLstr()
                                               << " " << obj3d.isValid())

  if(obj3d.isValid())
  {
    LOG("SectorChildInserted", 2, "Calling obj3d->Setup()");
    obj3d->Setup(vosa3dl, this);
  }
  else
  {
    vRef<csMetaLight> light = meta_cast<csMetaLight>(event.getChild());
    if(light.isValid())
    {
      light->Setup(vosa3dl, this);
    }
  }
  LOG("csVosSector", 2, "leaving notifyChildInserted " << waitingForChildren);

  waitingForChildren--;
  if(waitingForChildren <= 0) {
    vosa3dl->mainThreadTasks.push(new RelightTask(vosa3dl, this));
  }
}

void csVosSector::notifyChildRemoved (VobjectEvent &event)
{
  LOG("csVosSector", 2, "notifyChildRemoved");

  vRef<csMetaObject3D> obj3d = meta_cast<csMetaObject3D>(event.getChild());
  if(obj3d.isValid())
  {
    vosa3dl->mainThreadTasks.push(new LoadObjectTask( vosa3dl, obj3d,
                                                      this, true));
  }
  else
  {
    //vRef<csMetaLight> light = meta_cast<csMetaLight>(event.getChild());
    //if(light.isValid())
    //{
      //vosa3dl->mainThreadTasks.push(new LoadLightTask( vosa3dl, &light, this));
    //}
  }
  LOG("csVosSector", 2, "leaving notifyChildRemoved");
}

void csVosSector::notifyChildReplaced (VobjectEvent &event)
{
  LOG("csVosSector", 2, "notifyChildReplaced");
  notifyChildRemoved(event);
  notifyChildInserted(event);
}

csRef<iSector> csVosSector::GetSector()
{
  return sector;
}

VOS::vRef<VOS::Vobject> csVosSector::GetVobject()
{
  return sectorvobj;
}


