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
#include "csutil/scf.h"

#include "iengine/mesh.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "csgeom/tri.h"

#include "csvosa3dl.h"
#include "voscone.h"
#include "vosmaterial.h"

using namespace VUtil;
using namespace VOS;

class ConstructConeTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaMaterial> metamat;
  vRef<csMetaCone> cone;
  std::string name;
  csRef<iSector> sector;

  ConstructConeTask(iObjectRegistry *objreg, vRef<csMetaMaterial> mat,
                    csMetaCone* c, std::string n, iSector *s);
  virtual ~ConstructConeTask();
  virtual void doTask();
};

ConstructConeTask::ConstructConeTask(iObjectRegistry *objreg,
                                     vRef<csMetaMaterial> mat, csMetaCone* c,
                                     std::string n, iSector *s)
  : object_reg(objreg), metamat(mat), cone(c, true), name(n), sector(s)
{
}

ConstructConeTask::~ConstructConeTask()
{
}

void ConstructConeTask::doTask()
{
  LOG("voscone", 3, "Constructing cone");

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  // should store a single cone factory for everything?  or do we always get
  // the same one back?
  //if (!cone_factory)
  //{

  csRef<iMeshFactoryWrapper> cone_factory = engine->CreateMeshFactory (
                          "crystalspace.mesh.object.genmesh", "cone_factory");
  //}

  csRef<iGeneralFactoryState> coneLook = SCF_QUERY_INTERFACE(
                   cone_factory->GetMeshObjectFactory(), iGeneralFactoryState);
  if(coneLook)
  {
    coneLook->SetMaterialWrapper(metamat->GetMaterialWrapper());

    int hubVertices = 24;

    coneLook->SetVertexCount (1 + (hubVertices+1) * 2);
    coneLook->SetTriangleCount (hubVertices * 2);

    csVector3 *vertices = coneLook->GetVertices();
    csVector3 *normals = coneLook->GetNormals();
    csTriangle *triangles = coneLook->GetTriangles();
    csVector2 *texels = coneLook->GetTexels();

    vertices[0].Set (0, -0.5, 0);
    texels[0].Set(.5, 0);
    normals[0].Set(0, -1, 0);

    int i = 0;
    double angle = (double) i / (double) hubVertices * M_PI * 2;
    vertices[1 + (i*2) + 0].Set(cos(angle) * 0.5, -0.5, sin(angle) * 0.5);
    vertices[1 + (i*2) + 1].Set(0,  0.5, 0);
    normals[1 + (i*2) + 0].Set(cos(angle) * 0.5, 0.5, sin(angle) * 0.5);
    normals[1 + (i*2) + 1].Set(cos(angle) * 0.5, 0.5, sin(angle) * 0.5);
    texels[1 + (i*2) + 0].Set(0, (float)i / (float)hubVertices);
    texels[1 + (i*2) + 1].Set(1, (float)i / (float)hubVertices);


    for (i = 1; i <= hubVertices; i++)
    {
      double angle = (double) i / (double) hubVertices * M_PI * 2;
      vertices[1 + (i*2) + 0].Set (cos(angle) * 0.5, -0.5, sin(angle) * 0.5);
      vertices[1 + (i*2) + 1].Set (0,  0.5, 0);
      normals[1 + (i*2) + 0].Set(cos(angle) * 0.5, .25, sin(angle) * 0.5);
      normals[1 + (i*2) + 1].Set(cos(angle) * 0.5, .25, sin(angle) * 0.5);
      normals[1 + (i*2) + 0].Normalize();
      normals[1 + (i*2) + 1].Normalize();
      texels[1 + (i*2) + 0].Set(0, (float)i / (float)hubVertices);
      texels[1 + (i*2) + 1].Set(1, (float)i / (float)hubVertices);

      // top (slope) triangle
      triangles[(i-1) * 2].a = 1 + (i*2) + 1;
      triangles[(i-1) * 2].b = 1 + (i*2);
      triangles[(i-1) * 2].c = 1 + (i-1)*2;

      // bottom (base) triangle
      triangles[(i-1) * 2 + 1].a = 0;
      triangles[(i-1) * 2 + 1].b = 1 + (i-1)*2;
      triangles[(i-1) * 2 + 1].c = 1 + (i*2);
    }

    coneLook->Invalidate ();
    coneLook->CalculateNormals ();

    csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper (cone_factory,
                                     name.c_str(), sector, csVector3(0, 0, 0));
    cone->GetCSinterface()->SetMeshWrapper(meshwrapper);
  }
}

/// csMetaCone ///

csMetaCone::csMetaCone(VobjectBase* superobject)
  : A3DL::Object3D(superobject),
    csMetaObject3D(superobject),
    A3DL::Cone(superobject),
    alreadyLoaded(false)
{
}

MetaObject* csMetaCone::new_csMetaCone(VobjectBase* superobject,
  const std::string& type)
{
  return new csMetaCone(superobject);
}

void csMetaCone::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  vRef<A3DL::Material> m = getMaterial();
  vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
  LOG("csMetaCone", 3, "getting material " << mat.isValid());
  mat->Setup(vosa3dl);
  LOG("csMetaCone", 3, "setting up cone");
  vosa3dl->mainThreadTasks.push(new ConstructConeTask(
                                      vosa3dl->GetObjectRegistry(), mat, this,
                                      getURLstr(), sect->GetSector()));

  LOG("csMetaCone", 3, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
}
