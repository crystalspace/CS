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

/*
 * There are a few known problems.
 * transparency doesn't work right with PNG images (fine with GIFs) at least with
 * software renderer
 * you can't rescale sprites yet (so you're stuck with the inital size)
 * no support for UVanimation
 */

#include "cssysdef.h"

#include "iengine/mesh.h"
#include "imesh/sprite2d.h"
#include "imesh/object.h"

#include "csvosa3dl.h"
#include "vosbillboard.h"
#include "vosmaterial.h"

using namespace VOS;

class ConstructBillboardTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaMaterial> metamat;
  vRef<csMetaBillboard> billboard;
  std::string name;
  csRef<iSector> sector;

  ConstructBillboardTask(iObjectRegistry *objreg, vRef<csMetaMaterial> mat,
                         csMetaBillboard* b, std::string n, iSector *s);
  virtual ~ConstructBillboardTask();
  virtual void doTask();
};

ConstructBillboardTask::ConstructBillboardTask (iObjectRegistry *objreg,
                                                vRef<csMetaMaterial> mat,
            csMetaBillboard* b,
                                                std::string n, iSector *s)
  : object_reg(objreg), metamat(mat), billboard(b, true), name(n), sector(s)
{
}

ConstructBillboardTask::~ConstructBillboardTask()
{
}

void ConstructBillboardTask::doTask()
{
  LOG("vosbillboard", 2, "Constructing billboard");

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  // Create factory and give it the material
  csRef<iMeshFactoryWrapper> billboard_factory = engine->CreateMeshFactory (
                     "crystalspace.mesh.object.sprite.2d", "billboard_factory");

  csRef<iSprite2DFactoryState> fact = SCF_QUERY_INTERFACE (
          billboard_factory->GetMeshObjectFactory(), iSprite2DFactoryState);
  if (!fact) return;
  fact->SetMaterialWrapper (metamat->GetMaterialWrapper());

  // Create the mesh object and query the state
  csRef<iMeshObject> mesh = billboard_factory->GetMeshObjectFactory ()
    ->NewInstance();
  csRef<iSprite2DState> state = SCF_QUERY_INTERFACE (mesh, iSprite2DState);
  if (!state) return;

  // Create 1x1 square polygon.  This creates a diamond, the vertices need to be
  // moved a bit
  state->CreateRegularVertices (4, false);

  // Move the vertices.
  // csColoredVertices is defined in sprite2d.h as an array of
  // csSprite2DVertex's.
  // csSprite2dVertex is defined in sprite2d.h as a struct containing a
  // csVector2 named pos, and two floats named u and v. The polygon vertices
  // are moved to create a square (-.5,-.5), (-.5,.5), (.5,.5), (.5,-.5)
  // with the texxture mapped appropriately. (and right-side up).
  csColoredVertices& v = state->GetVertices();
  v[0].pos.x = -0.5; v[0].u = 0;
  v[0].pos.y = -0.5; v[0].v = 1;
  v[1].pos.x = -0.5; v[1].u = 0;
  v[1].pos.y =  0.5; v[1].v = 0;
  v[2].pos.x =  0.5; v[2].u = 1;
  v[2].pos.y =  0.5; v[2].v = 0;
  v[3].pos.x =  0.5; v[3].u = 1;
  v[3].pos.y = -0.5; v[3].v = 1;

  csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper (
                             mesh, name.c_str(), sector, csVector3(0, 0, 0));

  // For some reason this doesn't get set in the CreateMeshWrapper
  meshwrapper->SetFactory (billboard_factory);

  meshwrapper->SetRenderPriority (engine->GetRenderPriority("alpha"));
  meshwrapper->SetZBufMode (CS_ZBUF_TEST);

  billboard->GetCSinterface()->SetMeshWrapper(meshwrapper);
}

/// csMetaBillboard ///

csMetaBillboard::csMetaBillboard(VobjectBase* superobject)
  : A3DL::Object3D(superobject),
    csMetaObject3D(superobject),
    A3DL::Billboard(superobject),
    alreadyLoaded(false)
{
}

MetaObject* csMetaBillboard::new_csMetaBillboard(VobjectBase* superobject, const std::string& type)
{
  return new csMetaBillboard(superobject);
}

void csMetaBillboard::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  vRef<A3DL::Material> m = getMaterial();
  vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
  LOG("csMetaBillboard", 2, "getting material " << mat.isValid());
  mat->Setup(vosa3dl);
  LOG("csMetaBillboard", 2, "setting up billboard");
  vosa3dl->mainThreadTasks.push(new ConstructBillboardTask(
    vosa3dl->GetObjectRegistry(), mat, this, getURLstr(),
  sect->GetSector()));

  LOG("csMetaBillboard", 2, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
}

