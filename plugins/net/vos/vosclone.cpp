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

#include "csvosa3dl.h"
#include "vosclone.h"
#include "vosmaterial.h"

using namespace VOS;

class ConstructCloneTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaObject3D> templ;
  vRef<csMetaClone> clone;
  std::string name;
  csRef<iSector> sector;

  ConstructCloneTask(iObjectRegistry *objreg, vRef<csMetaObject3D> obj,
                     csMetaClone *c, std::string n, iSector *s);
  virtual ~ConstructCloneTask();
  virtual void doTask();
};

ConstructCloneTask::ConstructCloneTask(iObjectRegistry *objreg,
      vRef<csMetaObject3D> obj, csMetaClone *c, std::string n,
      iSector *s)
  : object_reg(objreg), templ(obj), clone(c, true), name(n), sector(s)
{
}

ConstructCloneTask::~ConstructCloneTask()
{
}

void ConstructCloneTask::doTask()
{
  LOG("vosclone", 2, "Constructing clone");

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  csRef<iMeshWrapper> wrapper = engine->CreateMeshWrapper (
                        templ->GetCSinterface()->GetMeshWrapper()->GetFactory(),
                        name.c_str(), sector, csVector3(0, 0, 0));

  clone->GetCSinterface()->SetMeshWrapper(wrapper);
}

/// csMetaClone ///

csMetaClone::csMetaClone(VobjectBase* superobject)
  : A3DL::Object3D(superobject),
    csMetaObject3D(superobject),
    A3DL::Clone(superobject),
    alreadyLoaded(false)
{
}

MetaObject* csMetaClone::new_csMetaClone(VobjectBase* superobject,
  const std::string& type)
{
  return new csMetaClone(superobject);
}

void csMetaClone::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

#if 0 // the ConstructCloneTask crashes at the moment...
  /* Can we replace the material without changing the factory? if so
   * we might want to pass this to the ConstructCloneTask and use it
   */
  //vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
  //LOG("csMetaClone", 2, "getting material " << mat.isValid());
  //mat->Setup(vosa3dl);

  vRef<csMetaObject3D> obj = meta_cast<csMetaObject3D> (getTemplate());
  LOG("csMetaClone", 2, "getting template object");
  if (obj->GetCSinterface()->GetMeshWrapper())
  {
    LOG("csMetaClone", 2, "object is already set up");
  }
  else obj->Setup(vosa3dl, sect);

  LOG("csMetaClone", 2, "setting up clone");
  vosa3dl->mainThreadTasks.push(new ConstructCloneTask(
                              vosa3dl->GetObjectRegistry(), obj, this,
                              getURLstr(), sect->GetSector()));

  LOG("csMetaClone", 2, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
#endif
}

