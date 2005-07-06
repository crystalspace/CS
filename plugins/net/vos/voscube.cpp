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
#include "imesh/genmesh.h"
#include "imesh/object.h"

#include "csvosa3dl.h"
#include "voscube.h"
#include "vosmaterial.h"

using namespace VUtil;
using namespace VOS;

class ConstructCubeTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaMaterial> metamat;
  vRef<csMetaCube> cube;
  std::string name;
  csRef<iSector> sector;
  csRef<iDynamicSystem> dynsys;
  csVector3 size;

  ConstructCubeTask(iObjectRegistry *objreg, vRef<csMetaMaterial> mat,
                      csMetaCube* c, std::string n, iSector *s);
  virtual ~ConstructCubeTask();
  virtual void doTask();
};

ConstructCubeTask::ConstructCubeTask(iObjectRegistry *objreg,
                                     vRef<csMetaMaterial> mat, csMetaCube* c,
                                     std::string n, iSector *s)
  : object_reg(objreg), metamat(mat), cube(c, true), name(n), sector(s)
{
}

ConstructCubeTask::~ConstructCubeTask()
{
}

void ConstructCubeTask::doTask()
{
  LOG("voscube", 3, "Constructing cube");

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  // should store a single cube factory for everything?  or do we always get
  // the same one back?
  //if(! cube_factory)
  //{
  csRef<iMeshFactoryWrapper> cube_factory = engine->CreateMeshFactory (
                         "crystalspace.mesh.object.genmesh", "cube_factory");
  //}

  csRef<iGeneralFactoryState> cubeLook = SCF_QUERY_INTERFACE(
                   cube_factory->GetMeshObjectFactory(), iGeneralFactoryState);
  if(cubeLook)
  {
    cubeLook->SetMaterialWrapper(metamat->GetMaterialWrapper());
    cubeLook->GenerateBox(csBox3(-.5, -.5, -.5, .5, .5, .5));

    csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper (
                    cube_factory, name.c_str(), sector, csVector3(0, 0, 0));

#if 0
    if (dynsys && !cube->GetCSinterface()->GetCollider())
    {
      // Create a body and attach the mesh.
      csRef<iRigidBody> rb = dynsys->CreateBody ();
      rb->SetProperties (1, csVector3 (0), csMatrix3 ());
      rb->SetPosition (csVector3(0, 0, 0));
      rb->AttachMesh (meshwrapper);
      rb->SetMoveCallback(cube->GetCSinterface());

      // of course this should use an actual collider box...
      csOrthoTransform t;
      rb->AttachColliderMesh(meshwrapper, t, 10, 1, 0);

      cube->GetCSinterface()->SetCollider (rb);
    }
#endif

    cube->GetCSinterface()->SetMeshWrapper(meshwrapper);
  }
}

/// csMetaCube ///

csMetaCube::csMetaCube(VobjectBase* superobject)
  : A3DL::Object3D(superobject),
    csMetaObject3D(superobject),
    A3DL::Cube(superobject),
    alreadyLoaded(false)
{
}

MetaObject* csMetaCube::new_csMetaCube(VobjectBase* superobject,
    const std::string& type)
{
  return new csMetaCube(superobject);
}

void csMetaCube::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  vRef<A3DL::Material> m = getMaterial();
  vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
  LOG("csMetaCube", 3, "getting material " << mat.isValid());
  mat->Setup(vosa3dl);

  LOG("csMetaCube", 3, "setting up cube");
  ConstructCubeTask *t = new ConstructCubeTask(vosa3dl->GetObjectRegistry(),
                                    mat, this, getURLstr(), sect->GetSector());

  t->dynsys = sect->GetDynSys();
  double x = 1, y = 1, z = 1;
  try
  {
    getScaling (x, y, z);
  }
  catch (NoSuchObjectError)
  {
  }
  t->size = csVector3 (x,y,z);

  vosa3dl->mainThreadTasks.push(t);
  LOG("csMetaCube", 3, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
}

