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
#include "voscone.h"
#include "vosmaterial.h"

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
  LOG("voscone", 2, "Constructing cone");

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

    coneLook->SetVertexCount (2 + hubVertices + 1);
    coneLook->SetTriangleCount (hubVertices * 2);

    csVector3 *vertices = coneLook->GetVertices();
    csTriangle *triangles = coneLook->GetTriangles();
    csVector2 *texels = coneLook->GetTexels();

    vertices[0].Set (0,  0.5, 0);
    vertices[1].Set (0, -0.5, 0);

    texels[0].Set(.5, 0);
    texels[1].Set(.5, 1);

    int i = 0;
    double angle = (double) i / (double) hubVertices * M_PI * 2;
    vertices[i+2].Set (cos(angle) * 0.5, -0.5, sin(angle) * 0.5);
    texels[i+2].Set(0, (float)i / (float)hubVertices);

    for (i = 1; i <= hubVertices; i++)
    {
      if(i < hubVertices) {
        double angle = (double) i / (double) hubVertices * M_PI * 2;
        vertices[i+2].Set (cos(angle) * 0.5, -0.5, sin(angle) * 0.5);
        texels[i+2].Set(0, (float)i / (float)hubVertices);
      }

      int n;
      if(i < hubVertices) n = i;
      else n = 0;

      // top (slope) triangle
      triangles[(i-1) * 2].a = 0;
      triangles[(i-1) * 2].b = 2 + n;
      triangles[(i-1) * 2].c = 2 + i-1;

      // bottom (base) triangle
      triangles[(i-1) * 2 + 1].a = 1;
      triangles[(i-1) * 2 + 1].b = 2 + i-1;
      triangles[(i-1) * 2 + 1].c = 2 + n;
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
    A3DL::Cone(superobject)
{
}

MetaObject* csMetaCone::new_csMetaCone(VobjectBase* superobject,
  const std::string& type)
{
  return new csMetaCone(superobject);
}

void csMetaCone::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  vRef<A3DL::Material> m = getMaterial();
  vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
  LOG("csMetaCone", 2, "getting material " << mat.isValid());
  mat->Setup(vosa3dl);
  LOG("csMetaCone", 2, "setting up cone");
  vosa3dl->mainThreadTasks.push(new ConstructConeTask(
                                      vosa3dl->GetObjectRegistry(), mat, this,
                                      getURLstr(), sect->GetSector()));

  LOG("csMetaCone", 2, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
}
