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

#include <vos/metaobjects/a3dl/a3dl.hh>

#include <vos/metaobjects/misc/search.hh>

using namespace VUtil;
using namespace VOS;

SCF_IMPLEMENT_IBASE (csVosSector)
  SCF_IMPLEMENTS_INTERFACE (iVosSector)
  SCF_IMPLEMENTS_INTERFACE (iVosApi)
SCF_IMPLEMENT_IBASE_END


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

/// Task for setting up the object from within a non-CS thread///
class SetupObjectTask : public Task
{
public:
  csVosA3DL* vosa3dl;
  vRef<csMetaObject3D> obj3d;
  csRef<csVosSector> sector;

  SetupObjectTask(csVosA3DL* va, csMetaObject3D *o, csVosSector* vs);
  virtual ~SetupObjectTask();
  virtual void doTask();
};

SetupObjectTask::SetupObjectTask (csVosA3DL *va, csMetaObject3D *o, csVosSector *vs)
    : vosa3dl(va), obj3d(o, true), sector(vs)
{
}

SetupObjectTask::~SetupObjectTask ()
{
}

void SetupObjectTask::doTask()
{
  LOG("SetupObjectTask", 3, "Setting up object3D")
  obj3d->Setup(vosa3dl, sector);
}

/** Task for loading or removing an object from the sector.  Will kick off
 *  the SetupObjectTask if the object has not been setup yet **/
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
      LOG("LoadObjectTask", 3, "Removing object from sector");
      sector->removeObject3D (obj3d->GetCSinterface());
      sector->GetSector()->GetMeshes()->Remove (wrapper);
      //wrapper->GetMovable()->GetSectors()->Remove (sector->GetSector());
      //wrapper->GetMovable()->UpdateMove();
    }
    else
    {
      sector->addObject3D (obj3d->GetCSinterface());
      if (wrapper->GetMovable()->GetSectors()->Find (sector->GetSector()))
      {
        LOG("LoadObjectTask", 3, "Object already setup and in sector");
      }
      else
      {
        LOG("LoadObjectTask", 3, "Object already setup, setting sector");
        if (wrapper->GetMovable()->GetSectors()->Find(sector->GetSector()) < 0) {
          wrapper->GetMovable()->GetSectors()->Add(sector->GetSector());
          wrapper->GetMovable()->UpdateMove();
        }
      }
    }
  }
  else
  {
    if (toRemove)
    {
      sector->removeObject3D (obj3d->GetCSinterface());
      LOG("LoadObjectTask", 2, "Attempting to remove empty meshwrapper!");
    }
    else
    {
      LOG("LoadObjectTask", 3, "Setting up object3D")
        if (obj3d->GetCSinterface() == NULL)
          LOG("NEIL-LOG", 1, "Uh oh");

      sector->addObject3D (obj3d->GetCSinterface());
      TaskQueue::defaultTQ().addTask(new SetupObjectTask(vosa3dl, obj3d, sector));
    }
  }
}

/// Task for setting up the object from within a non-CS thread///
class LoadSectorTask : public Task
{
public:
  csVosA3DL* vosa3dl;
  csRef<csVosSector> sector;

  LoadSectorTask(csVosA3DL* va, csVosSector* vs);
  virtual ~LoadSectorTask();
  virtual void doTask();
};

LoadSectorTask::LoadSectorTask (csVosA3DL *va, csVosSector *vs)
    : vosa3dl(va), sector(vs)
{
}

LoadSectorTask::~LoadSectorTask ()
{
}

void LoadSectorTask::doTask()
{
  LOG("csVosSector", 2, "Starting sector load");

  vRef<RemoteSearch> rs = meta_cast<RemoteSearch>(sector->GetVobject()->getSite());
  if(rs.isValid()) {

    {
        vRef<MessageBlock> mb(new MessageBlock(), false);
        vRef<Message> m(new Message(), false);
        m->setType("message");
        m->setMethod("core:start-listening");
        m->insertField(-1, "listen", "parents");
        mb->insertMessage(-1, m);
        mb->setName("parent-listen");
        rs->sendMessage(mb);
    }
    {
        vRef<MessageBlock> mb(new MessageBlock(), false);
        vRef<Message> m(new Message(), false);
        m->setType("message");
        m->setMethod("core:start-listening");
        m->insertField(-1, "listen", "children");
        mb->insertMessage(-1, m);
        mb->setName("children-listen");
        rs->sendMessage(mb);
    }
    {
        vRef<MessageBlock> mb(new MessageBlock(), false);
        vRef<Message> m(new Message(), false);
        m->setType("message");
        m->setMethod("property:start-listening");
        m->insertField(-1, "listen", "property");
        mb->insertMessage(-1, m);
        mb->setName("property-listen");
        rs->sendMessage(mb);
    }

  LOG("csVosSector", 2, "Starting search");

    rs->search(sector->GetVobject(), "sector", 0,
                       "rule sector\n"
                       "do acquire and parent-listen and children-listen to this object\n"
                       "select children with type a3dl:object3D* or with type a3dl:light* or with type a3dl:viewpoint* and apply rule 3Dobject\n"
                       "\n"
                       "rule 3Dobject\n"
                       "do acquire and parent-listen and children-listen to this object\n"
                       "select children with type a3dl:material and apply rule material\n"
                       "select children with type a3dl:portal and apply rule portal\n"
                       "select children with type property:property and apply rule property\n"
                       "select children with type property:property.extrapolated and apply rule extrap-property\n"
                       "\n"
                       "rule material\n"
                       "do acquire and parent-listen and children-listen to this object\n"
                       "select children with type property:property and apply rule property\n"
                       "select children with type a3dl:texture and apply rule texture\n"
                       "\n"
                       "rule texture\n"
                       "do acquire and parent-listen to this object\n"
                       "select children with type property:property and apply rule property\n"
                       "\n"
                       "rule portal\n"
                       "do acquire and parent-listen to this object\n"
                       "select children with type property:property and apply rule property\n"
                       "\n"
                       "rule property\n"
                       "do acquire and parent-listen and property-listen to this object \n"
                       "\n"
                       "rule extrap-property\n"
                       "do acquire and parent-listen and extrap-property-listen to this object\n"
                       );

  LOG("csVosSector", 2, "Search completed");
  }

  sector->GetVobject()->addChildListener (sector);
}


/// csVosSector ///

csVosSector::csVosSector(iObjectRegistry *o, csVosA3DL* va, csMetaSector* sec)
{
  SCF_CONSTRUCT_IBASE(0);
  didLoad = false;
  objreg = o;
  sectorvobj.assign(sec, true);
  sec->SetCsVosSector(this);
  vosa3dl = va;
  engine = CS_QUERY_REGISTRY (objreg, iEngine);
  sector = engine->CreateSector(sec->getURLstr().c_str());
  isLit = false;
  waitingForChildren = 0;
}

csVosSector::~csVosSector()
{
  SCF_DESTRUCT_IBASE();
}

const csSet< csPtrKey<iVosObject3D> > &csVosSector::GetObject3Ds()
{
  return loadedObjects;
}

void csVosSector::addObject3D (iVosObject3D *obj)
{
  loadedObjects.Add (obj);
}

void csVosSector::removeObject3D (iVosObject3D *obj)
{
  loadedObjects.Delete (obj);
}

void csVosSector::Load()
{
  if(! didLoad) {
  didLoad = true;
    waitingForChildren = sectorvobj->numChildren();
    TaskQueue::defaultTQ().addTask(new LoadSectorTask(vosa3dl, this));
  }
}

void csVosSector::notifyChildInserted (VobjectEvent &event)
{
  LOG("csVosSector", 3, "notifyChildInserted");
  try
  {
    vRef<csMetaObject3D> obj3d = meta_cast<csMetaObject3D>(event.getChild());
    LOG("SectorChildInserted", 3, "Looking at " << event.getChild()->getURLstr()
        << " " << obj3d.isValid())

      if(obj3d.isValid())
      {
        //obj3d->Setup(vosa3dl, this);
        vosa3dl->mainThreadTasks.push(new LoadObjectTask( vosa3dl, obj3d, this,
                              false));
    if (obj3d->getTypes().hasItem ("a3dl:object3D.polygonmesh")
      && obj3d->getTypes().hasItem ("a3dl:static"))
      vosa3dl->incrementRelightCounter();
      }
      else
      {
        vRef<csMetaLight> light = meta_cast<csMetaLight>(event.getChild());
        if(light.isValid())
        {
          light->Setup(vosa3dl, this);
      vosa3dl->incrementRelightCounter();
        }
      }
  }
  catch(std::runtime_error e)
  {
    LOG("csVosSector", 2, "caught runtime error setting up "
      << event.getChild()->getURLstr() << ": " << e.what());
  }

  LOG("csVosSector", 3, "leaving notifyChildInserted " << waitingForChildren);

  //waitingForChildren--;
  //if(waitingForChildren <= 0)
  //{
  //  vosa3dl->mainThreadTasks.push(new RelightTask(vosa3dl, this));
  //}
}

void csVosSector::notifyChildRemoved (VobjectEvent &event)
{
  LOG("csVosSector", 3, "notifyChildRemoved");

  vRef<csMetaObject3D> obj3d = meta_cast<csMetaObject3D>(event.getChild());
  if(obj3d.isValid())
  {
    vosa3dl->mainThreadTasks.push(new LoadObjectTask( vosa3dl, obj3d,
                                                        this, true));
  }
  else
  {
  /// TODO: Add code to remove a light dynamically! ///
  }
  LOG("csVosSector", 3, "leaving notifyChildRemoved");
}

void csVosSector::notifyChildReplaced (VobjectEvent &event)
{
  if((*event.getNewChild()) == (*event.getOldChild())) return;
  LOG("csVosSector", 3, "notifyChildReplaced");
  notifyChildRemoved(event);
  notifyChildInserted(event);
}

csRef<iSector> csVosSector::GetSector()
{
  return sector;
}

vRef<VOS::Vobject> csVosSector::GetVobject()
{
  return sectorvobj;
}


/// csMetaSector ///

csMetaSector::csMetaSector(VOS::VobjectBase* superobject)
  : A3DL::Sector(superobject)
{
}

VOS::MetaObject* csMetaSector::new_csMetaSector(VOS::VobjectBase* superobject,
                    const std::string& type)
{
  return new csMetaSector(superobject);
}
