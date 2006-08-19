/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include <iengine/material.h>
#include <iengine/engine.h>
#include <iutil/objreg.h>
#include <iengine/mesh.h>

#include "opentree.h"

#include <opentree/utils/otutils.h>

#include <opentree/mesher/treemesher.h>
#include <opentree/mesher/leafmesher.h>

#include <opentree/weber/weber.h>

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(OpenTree)
{


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
       csverts[index].x = x;
       csverts[index].y = y;
       csverts[index].z = z;
       csnorms[index].x = nx;
       csnorms[index].y = ny;
       csnorms[index].z = nz;
       csuvs[index].x = u;
       csuvs[index].y = v;
       cscolors[index].Set(r, g, b, a);
    };
  };










//---------------------------------------------------------------------------

/*
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("tree", "/this/data/bark.jpg"))
    ReportError("Error loading 'tree' texture!");

  if (!loader->LoadTexture ("leaf", "/this/data/leaf.png"))
    ReportError("Error loading 'leaf' texture!");

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  iMaterialWrapper* treemat = engine->GetMaterialList ()->FindByName ("tree");
  iMaterialWrapper* leafmat = engine->GetMaterialList ()->FindByName ("leaf");

  engine->GetTextureList()->FindByName("leaf")->GetTextureHandle()->SetAlphaType(csAlphaMode::alphaBinary);


  opentree::otTree* ottree = genTree("data/ca_black_oak.xml");

  int vertexCount = 0;

  opentree::MesherTree tree(ottree);
  opentree::MesherLeaf leaf(ottree);

  tree.setLOD(0,1);
  tree.setLOD(1,1);
  tree.setLOD(2,1);

  unsigned int offset_s = 0, offset_b = 0, offset_sb = 0;

  //Get the amount of vertices used with the current LoD.
  tree.getVerticesCount(0, &vertexCount);
  offset_s = vertexCount;
  tree.getVerticesCount(1, &vertexCount);
  offset_b = vertexCount;
  tree.getVerticesCount(2, &vertexCount);
  offset_sb = vertexCount;

  tree.getVertices(1, vertices+offset_s, vertexCount);
  tree.getVertices(2, vertices+offset_b, vertexCount);

  treefactstate->SetVertexCount(vertexCount);
  csVector3* csverts = treefactstate->GetVertices();
  csVector3* csnorms = treefactstate->GetNormals();
  csVector2* csuvs = treefactstate->GetTexels();
  for (int i=0; i<vertexCount; i++)
  {
     csverts[i].x = vertices[i].x;
     csverts[i].y = vertices[i].y;
     csverts[i].z = vertices[i].z;
     csnorms[i].x = vertices[i].nx;
     csnorms[i].y = vertices[i].ny;
     csnorms[i].z = vertices[i].nz;
     csuvs[i].x = vertices[i].u;
     csuvs[i].y = vertices[i].v;
  }

  delete [] vertices;

  vertexCount = leaf.getVerticesCount();
  vertices = new otVertexHelper();
  leaf.getVertices(vertices);

  leaffactstate->SetVertexCount(vertexCount);
  csVector3* csleafverts = leaffactstate->GetVertices();
  csVector3* csleafnorms = leaffactstate->GetNormals();
  csVector2* csleafuvs = leaffactstate->GetTexels();
  for (int i=0; i<vertexCount; i++)
  {
     csleafverts[i].x = vertices[i].x;
     csleafverts[i].y = vertices[i].y;
     csleafverts[i].z = vertices[i].z;
     csleafnorms[i].x = vertices[i].nx;
     csleafnorms[i].y = vertices[i].ny;
     csleafnorms[i].z = vertices[i].nz;
     csleafuvs[i].x = vertices[i].u;
     csleafuvs[i].y = vertices[i].v;
  }

  delete [] vertices;

  int trunk_triangleCount, branch_triangleCount, subbranch_triangleCount, leaves_triangleCount;

  tree.getIndicesCount(0, &trunk_triangleCount);
  opentree::Triangle* trunk_triangle = new opentree::Triangle[trunk_triangleCount];
  tree.getIndices(0, trunk_triangle, trunk_triangleCount, 0);

  tree.getIndicesCount(1, &branch_triangleCount);
  opentree::Triangle* branch_triangle = new opentree::Triangle[branch_triangleCount];
  tree.getIndices(1, branch_triangle, branch_triangleCount, offset_s);

  tree.getIndicesCount(2, &subbranch_triangleCount);
  opentree::Triangle* subbranch_triangle = new opentree::Triangle[subbranch_triangleCount];
  tree.getIndices(2, subbranch_triangle, subbranch_triangleCount, offset_b);

  treefactstate->SetTriangleCount(trunk_triangleCount + branch_triangleCount 
    + subbranch_triangleCount);

  csTriangle* cstris = treefactstate->GetTriangles();

  for (int i=0; i<treefactstate->GetTriangleCount(); i++)
  {
    if (i<trunk_triangleCount)
    {
      cstris[i].a = trunk_triangle[i].v1;
      cstris[i].b = trunk_triangle[i].v2;
      cstris[i].c = trunk_triangle[i].v3;
    }
    else if (i<trunk_triangleCount + branch_triangleCount)
    {
      cstris[i].a = branch_triangle[i-trunk_triangleCount].v1;
      cstris[i].b = branch_triangle[i-trunk_triangleCount].v2;
      cstris[i].c = branch_triangle[i-trunk_triangleCount].v3;
    }
    else
    {
      cstris[i].a = subbranch_triangle[i-trunk_triangleCount-branch_triangleCount].v1;
      cstris[i].b = subbranch_triangle[i-trunk_triangleCount-branch_triangleCount].v2;
      cstris[i].c = subbranch_triangle[i-trunk_triangleCount-branch_triangleCount].v3;
    }
  }

  delete [] trunk_triangle;
  delete [] branch_triangle;
  delete [] subbranch_triangle;

  leaves_triangleCount = leaf.getIndicesCount();
  opentree::Triangle* leaves_triangle = new opentree::Triangle[leaves_triangleCount];
  leaf.getIndices(leaves_triangle, 0);

  leaffactstate->SetTriangleCount(leaves_triangleCount);
  csTriangle* csleaftris = leaffactstate->GetTriangles();

  for (int i=0; i<leaffactstate->GetTriangleCount(); i++)
  {
    csleaftris[i].a = leaves_triangle[i].v1;
    csleaftris[i].b = leaves_triangle[i].v2;
    csleaftris[i].c = leaves_triangle[i].v3;
  }

  delete [] leaves_triangle;

  csXRotMatrix3 mtx(-PI/2);

  // Now create an instance:
  csRef<iMeshWrapper> treemesh =
    engine->CreateMeshWrapper (treefact, "tree");
  csRef<iGeneralMeshState> treemeshstate = SCF_QUERY_INTERFACE (
    treemesh->GetMeshObject (), iGeneralMeshState);
  treemeshstate->SetMaterialWrapper (treemat);

  treemesh->GetMovable()->SetPosition(room, csVector3(0,0,0));
  treemesh->GetMovable()->Transform(mtx);
  treemesh->GetMovable()->UpdateMove();

  // Now create an instance:
  csRef<iMeshWrapper> leafmesh =
    engine->CreateMeshWrapper (leaffact, "leaf");
  csRef<iGeneralMeshState> leafmeshstate = SCF_QUERY_INTERFACE (
    leafmesh->GetMeshObject (), iGeneralMeshState);
  leafmeshstate->SetMaterialWrapper (leafmat);

  leafmesh->GetMovable()->SetPosition(room, csVector3(0,0,0));
  leafmesh->GetMovable()->Transform(mtx);
  leafmesh->GetMovable()->UpdateMove();
}

opentree::otTree* genTree(const char* file)
{
  opentree::TreeData params;

//  opentree::xmlParserArbaro parser;
//  if (!parser.Load(params, file))
//    return 0;

  params.leafQuality = 0.1f;

  opentree::iWeber* gen = 0;
  
  gen = opentree::newWeber();

  gen->setParams(params);

  opentree::otTree* ottree = gen->generateTree();

  delete gen;
}

*/
//---------------------------------------------------------------------------












csOpenTreeObject::csOpenTreeObject
  (csOpenTreeObjectFactory* factory) : scfImplementationType (this)
{
printf("constructor!\n");
  csOpenTreeObject::factory = factory;
  logparent = 0;
  material = 0;
//  MixMode = 0;

  csRef<iEngine> engine;
  engine = csQueryRegistry<iEngine>(factory->object_reg);

  treemesh = engine->CreateMeshWrapper (factory->treefact, "fixtree0864");
  treemeshstate = scfQueryInterface<iGeneralMeshState> 
    (treemesh->GetMeshObject ());
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
  return treemesh->GetMeshObject ()->GetRenderMeshes(n, rview, mov, frustum);
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

bool csOpenTreeObject::SetMaterialWrapper (iMaterialWrapper* mat)
{
  //@@@
  material = mat;

  return treemesh->GetMeshObject ()->SetMaterialWrapper(mat);
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

}

csOpenTreeObjectFactory::~csOpenTreeObjectFactory ()
{
}

csPtr<iMeshObject> csOpenTreeObjectFactory::NewInstance ()
{
  printf("new Instance\n");
  if (!generated) GenerateTree();

  csRef<csOpenTreeObject> cm;
  cm.AttachNew (new csOpenTreeObject (this));

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
printf("Tree vertex count: %i ", vertexCount);

  treefactstate->SetVertexCount(vertexCount);
  opentree::otVertices* vertices = new VertexHelper (treefactstate);
  tree->getVertices(0, *vertices);
  delete vertices;

  tree->getLeavesVerticesCount(&vertexCount);
printf("leaf vertex count: %i ", vertexCount);
  leaffactstate->SetVertexCount(vertexCount);
  vertices = new VertexHelper (leaffactstate);
  tree->getLeavesVertices(*vertices);
  delete vertices;

  delete ottree;
  delete gen;

  generated = true;
  printf("done\n");
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
