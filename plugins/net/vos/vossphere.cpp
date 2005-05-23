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
#include "imesh/ball.h"
#include "imesh/object.h"

#include "csvosa3dl.h"
#include "vossphere.h"
#include "vosmaterial.h"

using namespace VUtil;
using namespace VOS;

class ConstructSphereTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaMaterial> metamat;
  vRef<csMetaSphere> sphere;
  std::string name;
  csRef<iSector> sector;
  csRef<iDynamicSystem> dynsys;
  int rim_vertices;

  ConstructSphereTask(iObjectRegistry *objreg, vRef<csMetaMaterial> mat,
                      csMetaSphere* s, std::string n, iSector *sec, int rv);
  virtual ~ConstructSphereTask();
  virtual void doTask();
};

ConstructSphereTask::ConstructSphereTask(iObjectRegistry *objreg,
                                     vRef<csMetaMaterial> mat, csMetaSphere* s,
                                     std::string n, iSector *sec, int rv)
  : object_reg(objreg), metamat(mat), sphere(s, true), name(n), sector(sec),
    rim_vertices(rv)
{
}

ConstructSphereTask::~ConstructSphereTask()
{
}

void ConstructSphereTask::doTask()
{
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  // should store a single sphere factory for everything?  or do we always get
  // the same one back?
  //if(! sphere_factor)
  //{
  csRef<iMeshFactoryWrapper> ball_factory = engine->CreateMeshFactory (
                         "crystalspace.mesh.object.ball", "ball_factory");
  //}

  csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper
      (ball_factory, name.c_str(), sector, csVector3(0, 0, 0));

  csRef<iBallState> ballLook = SCF_QUERY_INTERFACE (
                          meshwrapper->GetMeshObject(), iBallState);

  if(ballLook)
  {
    ballLook->SetMaterialWrapper(metamat->GetMaterialWrapper());
    ballLook->SetRadius (.5, .5, .5);
    ballLook->SetRimVertices (rim_vertices);

    if (dynsys)
    {
#if 0
      csRef<iRigidBody> collider = dynsys->CreateBody ();
      collider->SetProperties (1, csVector3 (0), csMatrix3 ());
      collider->SetPosition (csVector3(0, 0, 0));
      collider->AttachMesh (meshwrapper);
      //collider->SetMoveCallback(this);
      const csMatrix3 tm;
      const csVector3 tv (0);
      csOrthoTransform t (tm, tv);

      collider->AttachColliderBox (size, t, 0, 1, 0);
      //if(isRemote()) collider->MakeStatic();
      cube->GetCSinterface()->SetCollider (collider);

      collider->MakeStatic();
#endif
    }

    sphere->GetCSinterface()->SetMeshWrapper(meshwrapper);
  }
}

/// csMetaSphere ///

csMetaSphere::csMetaSphere(VobjectBase* superobject)
  : A3DL::Object3D(superobject),
    csMetaObject3D(superobject),
    A3DL::Sphere(superobject),
    alreadyLoaded(false)
{
}

MetaObject* csMetaSphere::new_csMetaSphere(VobjectBase* superobject,
    const std::string& type)
{
  return new csMetaSphere(superobject);
}

void csMetaSphere::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  vRef<A3DL::Material> m = getMaterial();
  vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
  LOG("csMetaSphere", 3, "getting material " << mat.isValid());
  mat->Setup(vosa3dl);

  int rim_vertices = getRimVertices();

  LOG("csMetaSphere", 3, "setting up sphere");
  ConstructSphereTask *t = new ConstructSphereTask(vosa3dl->GetObjectRegistry(),
                                    mat, this, getURLstr(), sect->GetSector(),
                                    rim_vertices);

  t->dynsys = vosa3dl->GetDynSys();

  vosa3dl->mainThreadTasks.push(t);
  LOG("csMetaSphere", 3, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
}

