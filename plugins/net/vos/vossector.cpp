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
  LOG("SetupObjectTask", 2, "Setting up object3D")
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
        wrapper->GetMovable()->GetSectors()->Add(sector->GetSector());
        wrapper->GetMovable()->UpdateMove();
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
      LOG("LoadObjectTask", 2, "Setting up object3D")
	  if (obj3d->GetCSinterface() == NULL)
		LOG("NEIL-LOG", 1, "Uh oh");

	  sector->addObject3D (obj3d->GetCSinterface());
	  TaskQueue::defaultTQ().addTask(new SetupObjectTask(vosa3dl, obj3d, sector));
    }
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

const csSet<iVosObject3D*> &csVosSector::GetObject3Ds()
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
  LOG("csVosSector", 2, "Starting sector load");

  sectorvobj = meta_cast<A3DL::Sector>(Vobject::findObjectFromRoot(url));
  waitingForChildren = sectorvobj->getChildren().size();
  sectorvobj->addChildListener (this);

  LOG("csVosSector", 2, "Started sector load");
}

void csVosSector::notifyChildInserted (VobjectEvent &event)
{
  LOG("csVosSector", 2, "notifyChildInserted");
  try
  {
    vRef<csMetaObject3D> obj3d = meta_cast<csMetaObject3D>(event.getChild());
    LOG("SectorChildInserted", 2, "Looking at " << event.getChild()->getURLstr()
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

  LOG("csVosSector", 2, "leaving notifyChildInserted " << waitingForChildren);

  //waitingForChildren--;
  //if(waitingForChildren <= 0) 
  //{
  //  vosa3dl->mainThreadTasks.push(new RelightTask(vosa3dl, this));
  //}
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
	/// TODO: Add code to remove a light dynamically! ///
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


