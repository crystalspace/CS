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
#include "csgeom/vector3.h"
#include "csgeom/matrix3.h"
#include "csgeom/quaterni.h"

#include "vosobject3d.h"
#include <vos/metaobjects/a3dl/a3dl.hh>

using namespace VOS;

SCF_IMPLEMENT_IBASE (csVosObject3D)
  SCF_IMPLEMENTS_INTERFACE (iVosObject3D)
SCF_IMPLEMENT_IBASE_END

/// csVosObject3D ///

csVosObject3D::csVosObject3D()
{
  SCF_CONSTRUCT_IBASE (0);
}

csVosObject3D::~csVosObject3D()
{
  SCF_DESTRUCT_IBASE();
}

csRef<iMeshWrapper> csVosObject3D::GetMeshWrapper()
{
  return meshwrapper;
}

void csVosObject3D::SetMeshWrapper(iMeshWrapper* mw)
{
  meshwrapper = mw;
}


class ConstructObject3DTask : public Task
{
public:
  csRef<iObjectRegistry> object_reg;
  vRef<csMetaObject3D> obj;
  csVector3 pos;
  csMatrix3 ori;

  ConstructObject3DTask(csRef<iObjectRegistry> objreg, csMetaObject3D* obj, const csVector3& pos, const csMatrix3& ori);
  virtual ~ConstructObject3DTask();
  virtual void doTask();
};

ConstructObject3DTask::ConstructObject3DTask(csRef<iObjectRegistry> objreg, csMetaObject3D* ob,
                                             const csVector3& p, const csMatrix3& o)
  : object_reg(objreg), obj(ob, true), pos(p), ori(o)
{
}

ConstructObject3DTask::~ConstructObject3DTask()
{
}

void ConstructObject3DTask::doTask()
{
  csRef<iMeshWrapper> mw = obj->getCSinterface()->GetMeshWrapper();
  LOG("vosobject3d", 2, "setting position " << pos.x << " " << pos.y << " " << pos.z);
  if(mw.IsValid())
  {
    mw->GetMovable()->SetPosition(pos);
    mw->GetMovable()->SetTransform(ori);
    mw->GetMovable()->UpdateMove();
  }
}


/// csMetaObject3D ///

csMetaObject3D::csMetaObject3D(VobjectBase* superobject) : A3DL::Object3D(superobject)
{
  csvobj3d = new csVosObject3D();
  csvobj3d->IncRef();
}

csMetaObject3D::~csMetaObject3D()
{
  delete csvobj3d;
}

MetaObject* csMetaObject3D::new_csMetaObject3D(VobjectBase* superobject, const std::string& type)
{
  return new csMetaObject3D(superobject);
}

void csMetaObject3D::setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  LOG("csMetaObject3D", 2, "now in csMetaObject3D::setup");

  double x, y, z;
  try
  {
    getPosition(x, y, z);
  }
  catch(...)
  {
    x = y = z = 0;
  }

  LOG("csMetaObject3D", 2, "got position");

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

  LOG("csMetaObject3D", 2, "got orientation ");

  csQuaternion q;
  q.SetWithAxisAngle(csVector3(a, b, c), d);

  LOG("vosobject3d", 2, "setting position " << x << " " << y << " " << z);

  vosa3dl->mainThreadTasks.push(new ConstructObject3DTask(vosa3dl->GetObjectRegistry(), this,
                                                          csVector3(x, y, z), csMatrix3(q)));
}

csRef<csVosObject3D> csMetaObject3D::getCSinterface()
{
  return csvobj3d;
}
