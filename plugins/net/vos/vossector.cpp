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

#include "vos/metaobjects/a3dl/a3dl.hh"

using namespace VOS;

SCF_IMPLEMENT_IBASE (csVosSector)
  SCF_IMPLEMENTS_INTERFACE (iVosSector)
SCF_IMPLEMENT_IBASE_END

class LoadSectorTask : public Task
{
public:
  csVosA3DL* vosa3dl;
  char* url;
  csRef<csVosSector> sector;

  LoadSectorTask(csVosA3DL* va, char* u, csVosSector* vs);
  virtual ~LoadSectorTask();
  virtual void doTask();
};

LoadSectorTask::LoadSectorTask(csVosA3DL* va, char* u, csVosSector* vs)
  : vosa3dl(va), sector(vs)
{
  url = strdup(u);
}

LoadSectorTask::~LoadSectorTask()
{
  free(url);
}

void LoadSectorTask::doTask()
{
  vRef<A3DL::Sector> sec = meta_cast<A3DL::Sector>(Vobject::findObjectFromRoot(url));
  for (ChildListIterator ci = sec->getChildren(); ci.hasMore(); ci++)
  {
    vRef<csMetaObject3D> obj3d = meta_cast<csMetaObject3D>((*ci)->getChild());
    std::cout << "looking at " << (*ci)->getChild()->getURLstr() << " " << obj3d.isValid() << std::endl;

    if(obj3d.isValid())
    {
      obj3d->setup(vosa3dl, (csVosSector*)sector);
    }
  }
}

/// csVosSector ///

csVosSector::csVosSector(csRef<iObjectRegistry> o, csVosA3DL* va, const char* s)
{
  SCF_CONSTRUCT_IBASE(0);
  objreg = o;
  url = strdup(s);
  vosa3dl = va;
  engine = CS_QUERY_REGISTRY (objreg, iEngine);
  sector = engine->CreateSector(s);
}

csVosSector::~csVosSector()
{
  free(url);
  SCF_DESTRUCT_IBASE();
}

void csVosSector::Load()
{
  TaskQueue::defaultTQ.addTask(new LoadSectorTask(vosa3dl, url, this));
}

csRef<iSector> csVosSector::GetSector()
{
  return sector;
}
