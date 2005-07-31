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

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#include <vos/metaobjects/a3dl/material.hh>
#include <vos/metaobjects/a3dl/portal.hh>

#include "iengine/mesh.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "csgeom/math3d.h"
#include "csgeom/matrix2.h"
#include "csgeom/matrix3.h"
#include "csgeom/tri.h"
#include "csutil/flags.h"

#include "csvosa3dl.h"
#include "vospolygonmesh.h"
#include "vosmaterial.h"

using namespace VUtil;
using namespace VOS;

class ConstructPolygonMeshTask : public Task
{
public:
  iObjectRegistry *object_reg;
  A3DL::MaterialIterator materials;
  A3DL::PortalIterator portals;
  vRef<csMetaPolygonMesh> polygonmesh;
  std::string name;
  csRef<iSector> sector;
  bool isStatic;
  csRef<iDynamicSystem> dynsys;
  Task* chainedtask;
  csVosA3DL *vosa3dl;
  csVector3 startingPos;

  std::vector<A3DL::PolygonMesh::Vertex> verts;
  std::vector<A3DL::PolygonMesh::Polygon> polys;
  std::vector<A3DL::PolygonMesh::Texel> texels;
  std::vector<A3DL::PolygonMesh::TextureSpace> texsp;

  ConstructPolygonMeshTask(iObjectRegistry *objreg,  csMetaPolygonMesh* c,
                           std::string n, iSector *s, const csVector3& pos);
  virtual ~ConstructPolygonMeshTask();
  virtual void doTask();
  void doStatic(iEngine* engine);
  void doGenmesh(iEngine* engine);
  void doTexturing(std::vector<int>& polymap, iThingFactoryState* thingfac);
};

ConstructPolygonMeshTask::ConstructPolygonMeshTask(iObjectRegistry *objreg,
                                                   csMetaPolygonMesh* pm,
                                                   std::string n,
                                                   iSector *s,
                                                   const csVector3& pos)
  : object_reg(objreg), polygonmesh(pm, true), name(n), sector(s),
    isStatic(false), chainedtask(0), startingPos(pos)
{
}

ConstructPolygonMeshTask::~ConstructPolygonMeshTask()
{
}

void ConstructPolygonMeshTask::doStatic(iEngine* engine)
{
    LOG("ConstructPolygonMeshTask", 3, "is static mesh");

    csRef<iMeshFactoryWrapper> factory = engine->CreateMeshFactory (
      "crystalspace.mesh.object.thing", "polygonmesh_factory");
    csRef<iThingFactoryState> thingfac = SCF_QUERY_INTERFACE(
      factory->GetMeshObjectFactory(), iThingFactoryState);

    for(size_t i = 0; i < verts.size(); i++)
    {
      thingfac->CreateVertex(csVector3(verts[i].x, verts[i].y, verts[i].z));
    }
    std::vector<int> polymap(polys.size());

    for(size_t i = 0; i < polys.size(); i++)
    {
      polymap[i] = thingfac->AddEmptyPolygon();

      bool flat = false;
      if(polys[i].size() < 3) flat = true;
      for(size_t n = 0; n < polys[i].size(); n++)
      {
        thingfac->AddPolygonVertex(CS_POLYRANGE_SINGLE(polymap[i]), polys[i][n]);

        // Does this code reject (say) a quad with one colinear vertex?
        // Is this desireable?
        if(n > 2)
        {
          float a = csMath3::Direction3(
      thingfac->GetPolygonVertex(polymap[i], n-2),
                thingfac->GetPolygonVertex(polymap[i], n-1),
                thingfac->GetPolygonVertex(polymap[i], n));
          if(ABS(a) < EPSILON) flat = true;
        }
      }

      if(flat)
      {
        thingfac->RemovePolygon(polymap[i]);
        polymap[i] = -1;
        std::string coords;
        for(size_t c = 0; c < polys[i].size(); c++) {
            if(c != 0) coords += ", ";
            coords += polys[i][c];
        }
        LOG("ConstructPolygonMeshTask", 2, "Discarded polygon "
                     << i << " with three colinear or coincident vertices: ("
                     << coords << ")");
      }
      else
      {
        thingfac->SetPolygonTextureMapping(CS_POLYRANGE_SINGLE(polymap[i]),
                                           thingfac->GetPolygonVertex(polymap[i], 0),
                                           thingfac->GetPolygonVertex(polymap[i], 1),
                                           1);
      }
    }

    doTexturing(polymap, thingfac);

    thingfac->SetSmoothingFlag(true);

    LOG("ConstructPolygonMeshTask", 3, "creating mesh wrapper for "
        << name << " in sector " << sector);

    csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper(
      factory, name.c_str(), sector, startingPos);

    if(materials.size())
    {
      meshwrapper->GetMeshObject()->SetMaterialWrapper
        ((meta_cast<csMetaMaterial>(*materials))->GetMaterialWrapper());
    }
    else
    {
      meshwrapper->GetMeshObject()->SetMaterialWrapper(
                                            csMetaMaterial::GetCheckerboard());
    }

    csRef<iThingState> thingstate = SCF_QUERY_INTERFACE(
                             meshwrapper->GetMeshObject(), iThingState);

    //thingstate->SetMovingOption(CS_THING_MOVE_OCCASIONAL);
    thingstate->SetMovingOption(CS_THING_MOVE_NEVER);

    factory->GetMeshObjectFactory()->GetFlags().Set(CS_THING_NOCOMPRESS);
    meshwrapper->GetFlags().Set(CS_ENTITY_NOSHADOWS);

    //transformgroup->GetChildren()->Add(meshwrapper);

    //if(portals.size()) meshwrapper->SetRenderPriority(
    //  engine->GetRenderPriority("portal"));

    LOG("ConstructPolygonMeshTask", 3, "done with " << name);

    polygonmesh->GetCSinterface()->SetMeshWrapper(meshwrapper);

    vosa3dl->decrementRelightCounter();
}

void ConstructPolygonMeshTask::doGenmesh(iEngine* engine)
{
    csRef<iMeshFactoryWrapper> factory = engine->CreateMeshFactory (
      "crystalspace.mesh.object.genmesh", "polygonmesh_factory");
    csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper(
      factory, name.c_str(), sector, startingPos);

    if(materials.hasMore())
      meshwrapper->GetMeshObject()->SetMaterialWrapper(
                (meta_cast<csMetaMaterial>(*materials))->GetMaterialWrapper());
    else
      meshwrapper->GetMeshObject()->SetMaterialWrapper(
                                            csMetaMaterial::GetCheckerboard());
    csRef<iGeneralFactoryState> genmesh = SCF_QUERY_INTERFACE(
                        factory->GetMeshObjectFactory(), iGeneralFactoryState);

    // Load vertices
    genmesh->SetVertexCount(verts.size());
    csVector3* vertices = genmesh->GetVertices();
    size_t i;
    for(i = 0; i < verts.size(); i++)
    {
      vertices[i].x = verts[i].x;
      vertices[i].y = verts[i].y;
      vertices[i].z = verts[i].z;
    }

    int num = 0;
    for(i = 0; i < polys.size(); i++)
    {
      num += polys[i].size() - 2;
    }
    // XXX check this property below and save it in the task structure above
    // so we can do this check
    //bool dbl = getDoubleSided();
    bool dbl = false;
    if(dbl) num *= 2;
    genmesh->SetTriangleCount(num);
    csTriangle* triangles = genmesh->GetTriangles();
    num = 0;
    for(size_t i = 0; i < polys.size(); i++)
    {
      for(size_t c = 2; c < polys[i].size(); c++)
      {
        triangles[num].a = polys[i][0];
        triangles[num].b = polys[i][c-1];
        triangles[num].c = polys[i][c];
        num++;
        if(dbl)
        {
          triangles[num].a = polys[i][0];
          triangles[num].b = polys[i][c];
          triangles[num].c = polys[i][c-1];
          num++;
        }
      }
    }

    if((size_t)genmesh->GetVertexCount() < texels.size())
    {
      LOG("terangreal::polygonmesh", 2, "Warning: there are more texels than existing vertices; increasing mesh's vertex count...");
      genmesh->SetVertexCount(texels.size());
    }
    csVector2* gmtexels = genmesh->GetTexels();
    for(size_t i = 0; i < texels.size(); i++)
    {
      gmtexels[i].x = texels[i].x;
      gmtexels[i].y = texels[i].y;
    }

    genmesh->Invalidate();
    genmesh->CalculateNormals();

    csRef<iGeneralMeshState> gm = SCF_QUERY_INTERFACE(
                            meshwrapper->GetMeshObject(), iGeneralMeshState);
    gm->SetLighting(true);

    /* XXX TODO: Make these be optional properties, on by default */
    /* Do we need to call engine->ForceRelight() now? XXX */
    gm->SetShadowReceiving(false);
    gm->SetShadowCasting(false);

    polygonmesh->GetCSinterface()->SetMeshWrapper(meshwrapper);
}

void ConstructPolygonMeshTask::doTexturing(std::vector<int>& polymap,
                                           iThingFactoryState* thingfac)
{
    if(texels.size() > 0)
    {
      for(size_t i = 0; i < polys.size(); i++)
      {
        if(polymap[i] == -1) continue;
        // Convenience: prevents us from having to say CS_POLYRANGE_SINGLE
        // in every thingfac->SetPolygonFoo method
        csPolygonRange p = CS_POLYRANGE_SINGLE(polymap[i]);

        int * polyindices = thingfac->GetPolygonVertexIndices(polymap[i]);
        csVector2 uv1(texels[polyindices[0]].x, texels[polyindices[0]].y);
        csVector2 uv2(texels[polyindices[1]].x, texels[polyindices[1]].y);
        csVector2 uv3(texels[polyindices[2]].x, texels[polyindices[2]].y);

        csMatrix2 m(uv2.x - uv1.x, uv3.x - uv1.x, uv2.y - uv1.y, uv3.y - uv1.y);
        float det = m.Determinant ();
        csVector3 vert1 = thingfac->GetPolygonVertex(polymap[i],0);
        csVector3 vert2 = thingfac->GetPolygonVertex(polymap[i],1);
        csVector3 vert3 = thingfac->GetPolygonVertex(polymap[i],2);

        if (ABS (det) < 0.0001f)
        {
          float norm12 = (vert1 - vert2).Norm();
          float norm23 = (vert2 - vert3).Norm();
          float norm13 = (vert3 - vert1).Norm();

          if(norm12 > norm23 && norm12 > norm13)
            thingfac->SetPolygonTextureMapping(p,vert1, vert2, 1);
          else if(norm23 > norm13)
            thingfac->SetPolygonTextureMapping(p,vert2, vert3, 1);
          else
            thingfac->SetPolygonTextureMapping(p,vert1, vert3, 1);
        }
        else
        {
          thingfac->SetPolygonTextureMapping(p, vert1, uv1, vert2,
                                                       uv2, vert3, uv3);
        }
        thingfac->SetPolygonMaterial(p,
                (meta_cast<csMetaMaterial>(*materials))->GetMaterialWrapper());
      }
    }
    else
    {
      for(size_t i = 0; i < polys.size(); i++)
      {
        if(polymap[i] == -1) continue;
        //Convenience: prevents us from having to say CS_POLYRANGE_SINGLE
        //in every thingfac->SetPolygonFoo method
        csPolygonRange p = CS_POLYRANGE_SINGLE(polymap[i]);

        // iPolygon3DStatic* p = thingfac->GetPolygon(polymap[i]);
          /* This is the Old and Bad Way of Doing Portals. Needs to be changed
             if(texsp[i].isPortal && texsp[i].material < portals.size()) {
             A3DL::Portal* portalvob = portals[texsp[i].material];
             vRef<Sector> sec = meta_cast<S3_VR::Sector*>(portalvob->getTargetSector());
             sec->setCSobjs(engine, object_reg, g3d, dynsys);
             sec->loadSector();
             csRef<iSector> target = sec->getCSSector();

             iPortal* portal = p->CreatePortal((iSector*)target);

             portal->GetFlags ().Set (CS_PORTAL_ZFILL);
             portal->GetFlags ().Set (CS_PORTAL_CLIPDEST);

             // This flag totally kills performance
             // portal->GetFlags ().Set (CS_PORTAL_FLOAT);
             float m11, m12, m13,
             m21, m22, m23,
             m31, m32, m33,
             tx, ty, tz;
             portalvob->getWarpingTransform(m11, m12, m13,
             m21, m22, m23,
             m31, m32, m33,
             tx, ty, tz);
             csMatrix3 mat(m11, m12, m13,
             m21, m22, m23,
             m31, m32, m33);
             csVector3 vec(tx, ty, tz);
             csTransform warp(mat, vec);
             portal->SetWarp(warp);
             }
          */

        if(i >= texsp.size()) {
          // Just throw the material at the polygon.
          // (this takes care of objects with color but no textures.)
          vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(*materials);
          thingfac->SetPolygonMaterial(p, mat->GetMaterialWrapper());
          continue;
        }

        switch(texsp[i].type)
        {
        case A3DL::PolygonMesh::NoTexture:
          break;
        case A3DL::PolygonMesh::UVcoord:
        {
          csVector2 uv1(texsp[i].Rep.UVcoord.u1, texsp[i].Rep.UVcoord.v1);
          csVector2 uv2(texsp[i].Rep.UVcoord.u2, texsp[i].Rep.UVcoord.v2);
          csVector2 uv3(texsp[i].Rep.UVcoord.u3, texsp[i].Rep.UVcoord.v3);

          csMatrix2 m ( uv2.x - uv1.x,
                        uv3.x - uv1.x,
                        uv2.y - uv1.y,
                        uv3.y - uv1.y);
          float det = m.Determinant ();

          if (ABS (det) < 0.0001f)
          {
            // Set the u-v axis to the two sides which join at the
            // greatest angle (another idea: set it to the greatest side
            // and a purpendicular?)

            csVector3 vt1 = thingfac->GetVertex(texsp[i].Rep.UVcoord.vert1);
            csVector3 vt2 = thingfac->GetVertex(texsp[i].Rep.UVcoord.vert2);
            csVector3 vt3 = thingfac->GetVertex(texsp[i].Rep.UVcoord.vert3);

            float norm12 = (vt1 - vt2).Norm();
            float norm23 = (vt2 - vt3).Norm();
            float norm13 = (vt3 - vt1).Norm();

            if(norm12 > norm23 && norm12 > norm13)
              thingfac->SetPolygonTextureMapping(p, vt1, vt2, 1);
            else if(norm23 > norm13)
              thingfac->SetPolygonTextureMapping(p, vt2, vt3, 1);
            else
              thingfac->SetPolygonTextureMapping(p, vt1, vt3, 1);
          }
          else
          {
            thingfac->SetPolygonTextureMapping(
              p, thingfac->GetVertex(texsp[i].Rep.UVcoord.vert1), uv1,
              thingfac->GetVertex(texsp[i].Rep.UVcoord.vert2), uv2,
              thingfac->GetVertex(texsp[i].Rep.UVcoord.vert3), uv3);
          }
        }
        break;
        case A3DL::PolygonMesh::PolygonPlane:
        {
          csVector3 org(texsp[i].Rep.PolygonPlane.xorg,
                        texsp[i].Rep.PolygonPlane.yorg,
                        texsp[i].Rep.PolygonPlane.zorg);
          csVector3 vec(texsp[i].Rep.PolygonPlane.x,
                        texsp[i].Rep.PolygonPlane.y,
                        texsp[i].Rep.PolygonPlane.z);
          thingfac->SetPolygonTextureMapping(p, org, vec,
                                             texsp[i].Rep.PolygonPlane.len);
        }
        break;
        case A3DL::PolygonMesh::ArbitraryPlane:
        {
          csVector3 org(texsp[i].Rep.ArbitraryPlane.xorg,
                        texsp[i].Rep.ArbitraryPlane.yorg,
                        texsp[i].Rep.ArbitraryPlane.zorg);
          csVector3 vec1(texsp[i].Rep.ArbitraryPlane.x1,
                         texsp[i].Rep.ArbitraryPlane.y1,
                         texsp[i].Rep.ArbitraryPlane.z1);
          csVector3 vec2(texsp[i].Rep.ArbitraryPlane.x2,
                         texsp[i].Rep.ArbitraryPlane.y2,
                         texsp[i].Rep.ArbitraryPlane.z2);
          thingfac->SetPolygonTextureMapping(
            p, org, vec1, texsp[i].Rep.ArbitraryPlane.len1,
            vec2, texsp[i].Rep.ArbitraryPlane.len2);
        }
        break;
        case A3DL::PolygonMesh::TexMatrix:
        {
          csMatrix3 mx(texsp[i].Rep.Matrix.m11, texsp[i].Rep.Matrix.m12,
                       texsp[i].Rep.Matrix.m13, texsp[i].Rep.Matrix.m21,
                       texsp[i].Rep.Matrix.m22, texsp[i].Rep.Matrix.m23,
                       texsp[i].Rep.Matrix.m31, texsp[i].Rep.Matrix.m32,
                       texsp[i].Rep.Matrix.m33);

          csVector3 vec(texsp[i].Rep.Matrix.x, texsp[i].Rep.Matrix.y,
                        texsp[i].Rep.Matrix.z);

          thingfac->SetPolygonTextureMapping(p, mx, vec);
        }
        break;
        }
#if 0
        if(texsp[i].isPortal)
        {
          thingfac->SetPolygonMaterial(p,checkerboard);
          if(texsp[i].material < portals.size())
          {
            A3DL::Portal* portalvob = portals[texsp[i].material];
            try
            {
              vRef<A3DL::Material> m = portalvob->getMaterial(false);
              if(&m)
              {
                S3_VR::Material* mat = meta_cast<S3_VR::Material*>(&m);
                if(mat)
                {
                  thingfac->SetPolygonMaterial(p,mat->getMaterialWrapper());
                }
              }
            }
            catch(NoSuchObjectError) { }
          }
        }
        else
#endif
          // XXX "no texture" doesn't make sense (need a color at least);
          // should allow specifying colors in the TextureSpace structure
          if(texsp[i].type != A3DL::PolygonMesh::NoTexture)
          {
            if(texsp[i].material < materials.size())
            {
              materials.setPos(texsp[i].material);
              vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(*materials);
              thingfac->SetPolygonMaterial(p, mat->GetMaterialWrapper());
            }
            else
              thingfac->SetPolygonMaterial(p,csMetaMaterial::GetCheckerboard());
          }
          else
          {
            thingfac->SetPolygonMaterial(p, csMetaMaterial::GetCheckerboard());
          }
      }
    }

}

void ConstructPolygonMeshTask::doTask()
{
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  LOG("ConstructPolygonMeshTask", 3, "constructing polygon mesh " << name);

  if(isStatic)
  {
    doStatic(engine);
  }
  else
  {
    doGenmesh(engine);
  }

  // Set up dynamics for mesh
  if (dynsys && !polygonmesh->GetCSinterface()->GetCollider())
  {
    csRef<iRigidBody> collider = dynsys->CreateBody ();
    collider->SetPosition (startingPos);
    collider->SetMoveCallback(polygonmesh->GetCSinterface());
    collider->MakeStatic ();

    csOrthoTransform t;
    collider->AttachColliderMesh (polygonmesh->GetCSinterface()->GetMeshWrapper(),
                                  t, 10, 1, 0);

    polygonmesh->GetCSinterface()->SetCollider (collider);
  }

  if(chainedtask) {
    chainedtask->doTask();
    delete chainedtask;
  }
}

/// csMetaPolygonMesh ///

csMetaPolygonMesh::csMetaPolygonMesh(VobjectBase* superobject)
    : A3DL::Object3D(superobject),
      csMetaObject3D(superobject),
      A3DL::PolygonMesh(superobject),
    alreadyLoaded(false)
{
}

MetaObject* csMetaPolygonMesh::new_csMetaPolygonMesh(VobjectBase* superobject,
  const std::string& type)
{
    return new csMetaPolygonMesh(superobject);
}

void csMetaPolygonMesh::Setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  this->vosa3dl = vosa3dl;

  double x, y, z;
  getPosition(x, y, z);

  ConstructPolygonMeshTask* cpmt =
    new ConstructPolygonMeshTask(vosa3dl->GetObjectRegistry(), this, getURLstr(),
                                 sect->GetSector(), csVector3(x, y, z));

  LOG("csMetaPolygonMesh", 3, "getting vertices");

  getVertices(cpmt->verts);
  getPolygons(cpmt->polys);

  try
  {
    LOG("csMetaPolygonMesh", 3, "getting texels");
    getTexels(cpmt->texels);
    LOG("csMetaPolygonMesh", 3, "got texels");
  }
  catch(NoSuchObjectError&)
  {
    LOG("csMetaPolygonMesh", 3, "getting texturespaces");
    try
    {
      getTextureSpaces(cpmt->texsp);
    } catch(NoSuchObjectError&)
    {
    }
    LOG("csMetaPolygonMesh", 3, "got texturespaces");
  }

  LOG("csMetaPolygonMesh", 3, "getting materials");

  cpmt->materials = getMaterials();
  cpmt->portals = getPortals();

  for(; cpmt->materials.hasMore(); cpmt->materials++)
  {
    (meta_cast<csMetaMaterial>(*(cpmt->materials)))->Setup(vosa3dl);
  }
  cpmt->materials.reset();

#if 0
  for(; cpmt->portals.hasMore(); cpmt->portals++)
  {
    cpmt->portals.setup();
  }
  cpmt->portals.reset();
#endif

  LOG("csMetaPolygonMesh", 3, "looking at types");

  for(TypeSetIterator ti = getTypes(); ti.hasMore(); ti++)
  {
    LOG("csMetaPolygonMesh", 3, "has type " << *ti);
    if(*ti == "a3dl:static") cpmt->isStatic = true;
  }
  LOG("csMetaPolygonMesh", 3, "is static " << cpmt->isStatic);

  cpmt->dynsys = sect->GetDynSys();

  cpmt->chainedtask = GetSetupTask(vosa3dl, sect);

  cpmt->vosa3dl = vosa3dl;

  vosa3dl->mainThreadTasks.push(cpmt);

  addChildListener (this);
}

