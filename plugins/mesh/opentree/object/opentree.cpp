/*
    Copyright (C) 2006 by Christoph "Fossi" Mewes
    Based on code by Pascal Kirchdorfer

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "iengine/material.h"
#include "iengine/engine.h"
#include "iutil/objreg.h"
#include "iengine/mesh.h"
#include "csutil/sysfunc.h"
#include "csgeom/tri.h"

#include "opentree.h"

#include <opentree/utils/otutils.h>

#include <opentree/mesher/treemesher.h>
#include <opentree/mesher/leafmesher.h>

#include <opentree/weber/weber.h>

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(OpenTree)
{

  class IndexHelper : public opentree::otTriangles
  {
  private:
    csTriangle* csindizes;
  public:
    IndexHelper::IndexHelper(iGeneralFactoryState *factstate)
    {
      csindizes = factstate->GetTriangles();
    };
    IndexHelper::~IndexHelper() {};

    void addTriangle(int v1, int v2, int v3)
    {
      //OTL face clock is the other way around
      int cnt = getCount();
      csindizes[cnt].a = v3;
      csindizes[cnt].b = v2;
      csindizes[cnt].c = v1;
    };
  };

  class VertexHelper : public opentree::otVertices
  {
  private:
    csVector3* csverts;
    csVector3* csnorms;
    csVector2* csuvs;
    csColor4* cscolors;
  public:
    VertexHelper::VertexHelper(iGeneralFactoryState *factstate)
    {
      csverts = factstate->GetVertices();
      csnorms = factstate->GetNormals();
      csuvs = factstate->GetTexels();
      cscolors = factstate->GetColors();
    };
    VertexHelper::~VertexHelper() {};

    void add(int index, float x, float y, float z, float nx, float ny,
      float nz, float r, float g, float b, float a, float u, float v)
    {
      //OTL coordinate system is z up, y forwards
      csverts[index].x = x;
      csverts[index].y = z;
      csverts[index].z = y;
      csnorms[index].x = nx;
      csnorms[index].y = nz;
      csnorms[index].z = ny;
      csuvs[index].x = u;
      csuvs[index].y = v;
      cscolors[index].Set(1, 1, 1, 0);
    };
  };



csOpenTreeObject::csOpenTreeObject
  (csOpenTreeObjectFactory* factory) : scfImplementationType (this)
{
  csOpenTreeObject::factory = factory;
  logparent = 0;
  MixMode = 0;

  csRef<iEngine> engine;
  engine = csQueryRegistry<iEngine>(factory->object_reg);

  treemesh = factory->treefact->CreateMeshWrapper ();
  treemesh->GetMeshObject ()->SetMeshWrapper (treemesh);
  treemeshstate = scfQueryInterface<iGeneralMeshState> 
    (treemesh->GetMeshObject ());

  leafmesh = factory->leaffact->CreateMeshWrapper ();
  leafmesh->GetMeshObject ()->SetMeshWrapper (leafmesh);
//  treemeshstate = scfQueryInterface<iGeneralMeshState> 
//    (treemesh->GetMeshObject ());
}

csOpenTreeObject::~csOpenTreeObject ()
{
}

iMeshObjectFactory* csOpenTreeObject::GetFactory () const
{
  return (csOpenTreeObjectFactory*)factory;
}

csRenderMesh** csOpenTreeObject::GetRenderMeshes (int &n, iRenderView* 
  rview, iMovable* mov, uint32 frustum)
{
  csRenderMesh** tree = 
    treemesh->GetMeshObject ()->GetRenderMeshes(n, rview, mov, frustum);
  csRenderMesh** leaves = 
    leafmesh->GetMeshObject ()->GetRenderMeshes(n, rview, mov, frustum);
  n = 2;

  csRenderMesh** meshes = new csRenderMesh*[2];
  meshes[0] = *tree;
  meshes[1] = *leaves;

  return meshes;
}

bool csOpenTreeObject::HitBeamOutline (const csVector3& start, 
  const csVector3& end, csVector3& isect, float *pr)
{
  printf("hitBeamOutline\n");
  return false;
}

bool csOpenTreeObject::HitBeamObject (const csVector3& start, 
  const csVector3& end, csVector3& isect, float* pr, int* polygon_idx,
  iMaterialWrapper** material)
{
  printf("hitBeamObject\n");
  return false;
}

iObjectModel* csOpenTreeObject::GetObjectModel ()
{
  return factory->GetObjectModel ();
}

bool csOpenTreeObject::SetMaterialWrapper (char level, iMaterialWrapper* mat)
{
  if (level == 5) leafmesh->GetMeshObject ()->SetMaterialWrapper(mat);
  else treemesh->GetMeshObject ()->SetMaterialWrapper(mat);

  return true;
}

iMaterialWrapper* csOpenTreeObject::GetMaterialWrapper (char level)
{
  return 0;
}

bool csOpenTreeObject::SetMaterialWrapper (iMaterialWrapper* mat)
{
  treemesh->GetMeshObject ()->SetMaterialWrapper(mat);
  leafmesh->GetMeshObject ()->SetMaterialWrapper(mat);
  return true;
}

//----------------------------------------------------------------------

csOpenTreeObjectFactory::csOpenTreeObjectFactory (iMeshObjectType* pParent, 
  iObjectRegistry* obj_reg) : scfImplementationType (this, pParent)
{
  object_reg = obj_reg;

  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);

  //@@@ fix names
  treefact = 
    engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh", "fixme1357");

  treefactstate = 
    SCF_QUERY_INTERFACE (treefact->GetMeshObjectFactory (), iGeneralFactoryState);

  leaffact = 
    engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh", "fixme2468");

  leaffactstate = 
    scfQueryInterface<iGeneralFactoryState> (leaffact->GetMeshObjectFactory ());

  generated = false;

  material = new iMaterialWrapper*[6];

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  string_ratiopower = strings->Request("RatioPower");
  string_lobedepth = strings->Request("LobeDepth");
  string_basesize = strings->Request("BaseSize");
  string_attractionup = strings->Request("AttractionUp");
  string_leafscalex = strings->Request("LeafScaleX");
  string_scale = strings->Request("Scale");
  string_scalev = strings->Request("ScaleV");
  string_ratio = strings->Request("Ratio");
  string_leafquality = strings->Request("LeafQuality");
  string_flare = strings->Request("Flare");
  string_leafscale = strings->Request("LeafScale");
  string_leafbend = strings->Request("LeafBend");
  string_leafdistrib = strings->Request("LeafDistrib");
  string_prunewidth = strings->Request("PruneWidth");
  string_prunewidthpeak = strings->Request("PruneWidthPeak");
  string_pruneratio = strings->Request("PruneRatio");
  string_prunepowerhigh = strings->Request("PrunePowerHigh");
  string_prunepowerlow = strings->Request("PrunePowerLow");
  string_leaves = strings->Request("Leaves");
  string_shape = strings->Request("Shape");
  string_lobes = strings->Request("Lobes");
  string_levels = strings->Request("Levels");
  string_levelnumber = strings->Request("LevelNumber");
  string_basesplits = strings->Request("BaseSplits");
  string_branchdist = strings->Request("BranchDist");
  string_downangle = strings->Request("DownAngle");
  string_downanglev = strings->Request("DownAngleV");
  string_rotate = strings->Request("Rotate");
  string_rotatev = strings->Request("RotateV");
  string_length = strings->Request("Length");
  string_lengthv = strings->Request("LengthV");
  string_taper = strings->Request("Taper");
  string_segsplits = strings->Request("SegSplits");
  string_splitangle = strings->Request("SplitAngle");
  string_splitanglev = strings->Request("SplitAngleV");
  string_curve = strings->Request("Curve");
  string_curveback = strings->Request("CurveBack");
  string_curvev = strings->Request("CurveV");
  string_branches = strings->Request("Branches");
  string_curveres = strings->Request("CurveRes");

  treedata.trunk.scale = 0;
  treedata.trunk.scaleV = 0;
  treedata.trunk.baseSplits = 0;
  treedata.trunk.dist = 0;

  for (int i = 0; i < 5; i++)
  {
    treedata.level[i].levelNumber = 0;
    treedata.level[i].downAngle = 0;
    treedata.level[i].downAngleV = 0;
    treedata.level[i].rotate = 0;
    treedata.level[i].rotateV = 0;
    treedata.level[i].branches = 0;
    treedata.level[i].branchDist = 0;
    treedata.level[i].length = 0;
    treedata.level[i].lengthV = 0;
    treedata.level[i].taper = 0;
    treedata.level[i].segSplits = 0;
    treedata.level[i].splitAngle = 0;
    treedata.level[i].splitAngleV = 0;
    treedata.level[i].curveRes = 0;
    treedata.level[i].curve = 0;
    treedata.level[i].curveBack = 0;
    treedata.level[i].curveV = 0;
  }

  treedata.shape = 0;
  treedata.baseSize = 0;
  treedata.scale = 0;
  treedata.scaleV = 0;
  treedata.levels = 0;
  treedata.ratio = 0;
  treedata.ratioPower = 0;
  treedata.lobes = 0;
  treedata.lobeDepth = 0;
  treedata.flare = 0;
  treedata.leaves = 0;
  treedata.leafScale = 0;
  treedata.leafScaleX = 0;
  treedata.leafQuality = 0;
  treedata.leafShapeRatio = 0;
  treedata.leafBend = 0;
  treedata.attractionUp = 0;
  treedata.pruneRatio = 0;
  treedata.prunePowerLow = 0;
  treedata.prunePowerHigh = 0;
  treedata.pruneWidth = 0;
  treedata.pruneWidthPeak = 0;
}

csOpenTreeObjectFactory::~csOpenTreeObjectFactory ()
{
}

csPtr<iMeshObject> csOpenTreeObjectFactory::NewInstance ()
{
  if (!generated) GenerateTree();

  csRef<csOpenTreeObject> cm;
  cm.AttachNew (new csOpenTreeObject (this));
  csRef<iOpenTreeState> state = scfQueryInterface<iOpenTreeState> (cm);
  state->SetMaterialWrapper(0, material[0]);
  state->SetMaterialWrapper(5, material[5]);

  csRef<iMeshObject> im = scfQueryInterface<iMeshObject> (cm);
  return csPtr<iMeshObject> (im);
}

void csOpenTreeObjectFactory::SetObjectBoundingBox (const csBox3&)
{
  printf("setBB\n");
}

void csOpenTreeObjectFactory::GetObjectBoundingBox (csBox3& bbox)
{
  treefact->GetMeshObjectFactory()->GetObjectModel()->GetObjectBoundingBox(bbox);
}

void csOpenTreeObjectFactory::GetRadius (float&, csVector3&)
{
  printf("getRadius\n");
}

bool csOpenTreeObjectFactory::SetParam (char level, 
  csStringID name, float value)
{
  if (level == -1) {

  if (name == string_ratiopower) { treedata.ratioPower = value; } else
  if (name == string_lobedepth) { treedata.lobeDepth = value; } else
  if (name == string_basesize) { treedata.baseSize = value; } else
  if (name == string_attractionup) { treedata.attractionUp = value; } else
  if (name == string_leafscalex) { treedata.leafScaleX = value; } else
  if (name == string_scale) { treedata.scale = value; } else
  if (name == string_scalev) { treedata.scaleV = value; } else
  if (name == string_ratio) { treedata.ratio = value; } else
  if (name == string_leafquality) { treedata.leafQuality = value; } else
  if (name == string_flare) { treedata.flare = value; } else
  if (name == string_leafscale) { treedata.leafScale = value; } else
  if (name == string_leafbend) { treedata.leafBend = value; } else
  if (name == string_prunewidth) { treedata.pruneWidth = value; } else
  if (name == string_prunewidthpeak) { treedata.pruneWidthPeak = value; } else
  if (name == string_prunepowerlow) { treedata.prunePowerLow = value; } else
  if (name == string_prunepowerhigh) { treedata.prunePowerHigh = value; } else
  if (name == string_pruneratio) { treedata.pruneRatio = value; } else
  {
     printf("Wrong Parameter!\n");
     return false;
  }

  } else {

    opentree::Level *treelevel = &treedata.level[(int)level];

    if (name == string_scale)
    { 
      if (level != 0) return false;
      treedata.trunk.scale = value; 
    } else 
    if (name == string_scalev)
    { 
      if (level != 0) return false;
      treedata.trunk.scaleV = value; 
    } else 
    if (name == string_basesplits)
    { 
      if (level != 0) return false;
      treedata.trunk.baseSplits = value; 
    } else 
    if (name == string_branchdist)
    { 
      if (level != 0) return false;
      treedata.trunk.dist = value; 
      treedata.level[0].branchDist = value; 
    } else 
    if (name == string_downangle) { treelevel->downAngle = value; } else
    if (name == string_downanglev) { treelevel->downAngleV = value; } else
    if (name == string_rotate) { treelevel->rotate = value; } else
    if (name == string_rotatev) { treelevel->rotateV = value; } else
    if (name == string_length) { treelevel->length = value; } else
    if (name == string_lengthv) { treelevel->lengthV = value; } else
    if (name == string_taper) { treelevel->taper = value; } else
    if (name == string_segsplits) { treelevel->segSplits = value; } else
    if (name == string_splitangle) { treelevel->splitAngle = value; } else
    if (name == string_splitanglev) { treelevel->splitAngleV = value; } else
    if (name == string_curve) { treelevel->curve = value; } else
    if (name == string_curveback) { treelevel->curveBack = value; } else
    if (name == string_curvev) { treelevel->curveV = value; } else
    {
     printf("Wrong Parameter!\n");
     return false;
    }
  }

  return true;
}

bool csOpenTreeObjectFactory::SetParam (char level,
  csStringID name, int value)
{
  if (level == -1) {

    if (name == string_shape) { treedata.shape = value; } else
    if (name == string_levels) { treedata.levels = value; } else
    if (name == string_lobes) { treedata.lobes = value; } else
    if (name == string_leaves) { treedata.leaves = value; } else
    if (name == string_leafdistrib) { treedata.leafShapeRatio = value; } else
    {
      printf("Wrong Parameter!\n");
      return false;
    }

  } else {

    opentree::Level *treelevel = &treedata.level[(int)level];

    if (name == string_levelnumber) { treelevel->levelNumber = value; } else
    if (name == string_branches) { treelevel->branches = value; } else
    if (name == string_curveres) { treelevel->curveRes = value; } else
    {
      printf("Wrong Parameter!\n");
      return false;
    }
  }

  return true;
}

bool csOpenTreeObjectFactory::SetParam (char, csStringID, const char*)
{
  return false;  //no supported parameters
}

void csOpenTreeObjectFactory::GenerateTree ()
{
  printf("generating Tree... ");

  //@@@ ?
  treedata.leafQuality = 0.6f;

  opentree::iWeber* gen = opentree::newWeber();
  gen->setParams(treedata);

  opentree::otTree* ottree = gen->generate();

  opentree::MesherTree* tree = new opentree::MesherTree(ottree);

  for (unsigned int i = 0; i < 4; i++)
  {
    tree->setCurveRes(i,6);
    tree->setCircleRes(i,7);
  }

  tree->useQuadLeaves(); //TriangleLeaves();

  int vertexCount = 0;
  tree->getVerticesCount(0, &vertexCount);
  tree->getVerticesCount(1, &vertexCount);
  tree->getVerticesCount(2, &vertexCount);
printf("Tree vertex count: %i ", vertexCount);

  treefactstate->SetVertexCount(vertexCount);
  opentree::otVertices* vertices = new VertexHelper (treefactstate);
  tree->getVertices(0, *vertices);
  tree->getVertices(1, *vertices);
  tree->getVertices(2, *vertices);
  delete vertices;

  int triangleCount = 0;
  int trunk_triangleCount = 0;
  int branch_triangleCount = 0;
  int subbranch_triangleCount = 0;
  tree->getIndicesCount(0, &trunk_triangleCount);
  tree->getIndicesCount(1, &branch_triangleCount);
  //tree->getIndicesCount(2, &subbranch_triangleCount);
  triangleCount = trunk_triangleCount + branch_triangleCount;// + subbranch_triangleCount
printf("Tree index count: %i ", triangleCount);

  treefactstate->SetTriangleCount(triangleCount);
  opentree::otTriangles* indizes = new IndexHelper (treefactstate);
  tree->getIndices(0, *indizes, 0);
  tree->getIndices(1, *indizes, trunk_triangleCount);
  //tree->getIndices(2, *indizes, branch_triangleCount);
  printf("added indexes: %i\n", indizes->getCount());


  vertexCount = 0;
  tree->getLeavesVerticesCount(&vertexCount);
printf("leaf vertex count: %i ", vertexCount);
  leaffactstate->SetVertexCount(vertexCount);

  vertices = new VertexHelper (leaffactstate);
  tree->getLeavesVertices(*vertices);
  delete vertices;

  triangleCount = 0;
  tree->getLeavesIndicesCount(&triangleCount);
printf("leaf index count: %i ", triangleCount);

  leaffactstate->SetTriangleCount(triangleCount);
  indizes = new IndexHelper (leaffactstate);
  tree->getLeavesIndices(*indizes, 0);
  printf("added indexes: %i\n", indizes->getCount());

  delete ottree;
  delete gen;

  generated = true;
  printf("done\n");
}

bool csOpenTreeObjectFactory::SetMaterialWrapper (char level, iMaterialWrapper* mat)
{
  material[level] = mat;
  return true;
}

iMaterialWrapper* csOpenTreeObjectFactory::GetMaterialWrapper (char level)
{
  return material[level];
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csOpenTreeObjectType)

csOpenTreeObjectType::csOpenTreeObjectType (iBase* pParent) : 
scfImplementationType (this, pParent)
{
}

csOpenTreeObjectType::~csOpenTreeObjectType ()
{
}

csPtr<iMeshObjectFactory> csOpenTreeObjectType::NewFactory ()
{
  csRef<csOpenTreeObjectFactory> cm;
  cm.AttachNew (new csOpenTreeObjectFactory (this, object_reg));
  csRef<iMeshObjectFactory> ifact (scfQueryInterface<iMeshObjectFactory> (cm));
  return csPtr<iMeshObjectFactory> (ifact);
}

bool csOpenTreeObjectType::Initialize (iObjectRegistry* object_reg)
{
  csOpenTreeObjectType::object_reg = object_reg;
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(OpenTree)
