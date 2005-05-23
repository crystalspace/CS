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
#include "voscylinder.h"
#include "vosmaterial.h"

using namespace VUtil;
using namespace VOS;

class ConstructCylinderTask : public Task
{
public:
  iObjectRegistry *object_reg;
  vRef<csMetaMaterial> metamat;
  vRef<csMetaCylinder> cylinder;
  std::string name;
  csRef<iSector> sector;
  csRef<iDynamicSystem> dynsys;
  int hub_vertices;

  ConstructCylinderTask(iObjectRegistry *objreg, vRef<csMetaMaterial> mat,
                      csMetaCylinder* c, std::string n, iSector *s, int hv);
  virtual ~ConstructCylinderTask();
  virtual void doTask();
};

ConstructCylinderTask::ConstructCylinderTask(iObjectRegistry *objreg,
                                     vRef<csMetaMaterial> mat, csMetaCylinder*c,
                                     std::string n, iSector *s, int hv)
  : object_reg(objreg), metamat(mat), cylinder(c, true), name(n), sector(s),
    hub_vertices(hv)
{
}

ConstructCylinderTask::~ConstructCylinderTask()
{
}

void ConstructCylinderTask::doTask()
{
  LOG("voscylinder", 3, "Constructing cylinder");

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  // should store a single sphere factory for everything?  or do we always get
  // the same one back?
  //if(! sphere_factor)
  //{
  csRef<iMeshFactoryWrapper> cylinder_factory = engine->CreateMeshFactory (
                        "crystalspace.mesh.object.genmesh","cylinder_factory");
  //}

  csRef<iGeneralFactoryState> cylinderLook = SCF_QUERY_INTERFACE (
               cylinder_factory->GetMeshObjectFactory(), iGeneralFactoryState);

  if (cylinderLook)
  {
    cylinderLook->SetMaterialWrapper(metamat->GetMaterialWrapper());
    cylinderLook->SetVertexCount (2 + (hub_vertices+1) * 4);
    cylinderLook->SetTriangleCount (hub_vertices * 4);

    csVector3* vertices=cylinderLook->GetVertices();
    csVector3* normals=cylinderLook->GetNormals();
    csVector2* texels=cylinderLook->GetTexels();
    csTriangle* triangles=cylinderLook->GetTriangles();
    vertices[0].Set(0, .5,  0);
    vertices[1].Set(0, -.5, 0);

    normals[0].Set(0, 1,  0);
    normals[1].Set(0, -1, 0);

    texels[0].Set(1, 1);
    texels[1].Set(0, 0);

    int i = 0;
    double angle = (double) i / (double) hub_vertices * M_PI * 2;
    vertices[2 + i*4 + 0].Set (cos(angle) * .5,  .5, sin(angle) * .5);
    vertices[2 + i*4 + 1].Set (cos(angle) * .5, -.5, sin(angle) * .5);
    vertices[2 + i*4 + 2].Set (cos(angle) * .5 , .5, sin(angle) * .5);
    vertices[2 + i*4 + 3].Set (cos(angle) * .5, -.5, sin(angle) * .5);

    normals[2 + i*4 + 0].Set (cos(angle) * .5, 0, sin(angle) * .5);
    normals[2 + i*4 + 0].Normalize();
    normals[2 + i*4 + 1].Set (cos(angle) * .5, 0, sin(angle) * .5);
    normals[2 + i*4 + 1].Normalize();
    normals[2 + i*4 + 2].Set (0, 1, 0);
    normals[2 + i*4 + 3].Set (0, -1, 0);

    texels[2 + i*4 + 0].Set((float)i / (float)hub_vertices, 0);
    texels[2 + i*4 + 1].Set((float)i / (float)hub_vertices, 1);
    texels[2 + i*4 + 2].Set((float)i / (float)hub_vertices, 0);
    texels[2 + i*4 + 3].Set((float)i / (float)hub_vertices, 1);

    for (i = 1; i <= hub_vertices; i++)
    {
      double angle = (double) i / (double) hub_vertices * M_PI * 2;
      vertices[2 + i*4 + 0].Set (cos(angle) * .5,  .5, sin(angle) * .5);
      vertices[2 + i*4 + 1].Set (cos(angle) * .5, -.5, sin(angle) * .5);
      vertices[2 + i*4 + 2].Set (cos(angle) * .5,  .5, sin(angle) * .5);
      vertices[2 + i*4 + 3].Set (cos(angle) * .5, -.5, sin(angle) * .5);

      normals[2 + i*4 + 0].Set (cos(angle) * .5, 0, sin(angle) * .5);
      normals[2 + i*4 + 0].Normalize();
      normals[2 + i*4 + 1].Set (cos(angle) * .5, 0, sin(angle) * .5);
      normals[2 + i*4 + 1].Normalize();
      normals[2 + i*4 + 2].Set (0, 1, 0);
      normals[2 + i*4 + 3].Set (0, -1, 0);

      texels[2 + i*4 + 0].Set((float)i / (float)hub_vertices, 0);
      texels[2 + i*4 + 1].Set((float)i / (float)hub_vertices, 1);
      texels[2 + i*4 + 2].Set((float)i / (float)hub_vertices, 0);
      texels[2 + i*4 + 3].Set((float)i / (float)hub_vertices, 1);

      // top triangle
      triangles[(i-1)*4].a = 0;
      triangles[(i-1)*4].b = 2 + i*4 + 2;
      triangles[(i-1)*4].c = 2 + (i-1)*4 + 2;

      // bottom triangle
      triangles[(i-1)*4 + 1].c = 1;
      triangles[(i-1)*4 + 1].b = 2 + i*4 + 3;
      triangles[(i-1)*4 + 1].a = 2 + (i-1)*4 + 3;

      // first side triangle
      triangles[(i-1)*4 + 2].a = 2 + i*4 + 0;
      triangles[(i-1)*4 + 2].b = 2 + i*4 + 1;
      triangles[(i-1)*4 + 2].c = 2 + (i-1)*4 + 0;

      // second side triangle
      triangles[(i-1)*4 + 3].c = 2 + i*4 + 1;
      triangles[(i-1)*4 + 3].b = 2 + (i-1)*4 + 0;
      triangles[(i-1)*4 + 3].a = 2 + (i-1)*4 + 1;
    }

    cylinderLook->Invalidate();
    cylinderLook->CalculateNormals ();

    /*
    csVector3* n = cylinderLook->GetNormals();
    n[2] += n[2 + hub_vertices*2 + 0];
    n[3] += n[2 + hub_vertices*2 + 1];

    n[2] /= 2;
    n[3] /= 2;

    n[2 + hub_vertices*2 + 0] = n[2];
    n[2 + hub_vertices*2 + 1] = n[3];
    */

    csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper (
      cylinder_factory, name.c_str(), sector, csVector3(0,0,0));

    if(dynsys)
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
#endif
    }

    cylinder->GetCSinterface()->SetMeshWrapper(meshwrapper);
  }
}

/// csMetaCylinder ///

csMetaCylinder::csMetaCylinder(VobjectBase* superobject)
  : A3DL::Object3D(superobject),
    csMetaObject3D(superobject),
    A3DL::Cylinder(superobject),
    alreadyLoaded(false)
{
}

MetaObject* csMetaCylinder::new_csMetaCylinder(VobjectBase* superobject,
    const std::string& type)
{
  return new csMetaCylinder(superobject);
}

void csMetaCylinder::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  vRef<A3DL::Material> m = getMaterial();
  vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
  LOG("csMetaCylinder", 3, "getting material " << mat.isValid());
  mat->Setup(vosa3dl);

  // TODO: Perhaps store this within a3dl
  int hub_vertices = 24;

  LOG("csMetaCylinder", 3, "setting up cylinder");
  ConstructCylinderTask *t = new ConstructCylinderTask(vosa3dl->GetObjectRegistry(),
                                    mat, this, getURLstr(), sect->GetSector(),
                                    hub_vertices);

  t->dynsys = vosa3dl->GetDynSys();

  vosa3dl->mainThreadTasks.push(t);
  LOG("csMetaCylinder", 3, "calling csMetaObject3D::setup");
  csMetaObject3D::Setup(vosa3dl, sect);
}

