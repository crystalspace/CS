/*
  Copyright (C) 2011 by Liu Lu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csgeom/tri.h"
#include "iengine/movable.h"
#include "igeom/trimesh.h"
#include "iengine/mesh.h"
#include "csutil/stringquote.h"
#include "imesh/objmodel.h"
#include "colliders2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
csRef<iTriangleMesh> FindColdetTriangleMesh (iMeshWrapper* mesh,
  csStringID baseID, csStringID colldetID)
{
  iObjectModel* objModel = mesh->GetMeshObject ()->GetObjectModel ();
  csRef<iTriangleMesh> triMesh;
  bool use_trimesh = objModel->IsTriangleDataSet (baseID);
  if (use_trimesh)
  {
    if  (objModel->GetTriangleData (colldetID))
      triMesh = objModel->GetTriangleData (colldetID);
    else
      triMesh = objModel->GetTriangleData (baseID);
  }

  if (!triMesh || triMesh->GetVertexCount () == 0
    || triMesh->GetTriangleCount () == 0)
  {
    csFPrintf (stderr, "iCollider: No collision polygons, triangles or vertices on mesh factory %s\n",
      CS::Quote::Single (mesh->QueryObject ()->GetName ()));

    return NULL;
  }
  return triMesh;
}

csOpcodeCollider::csOpcodeCollider (iMeshWrapper* mesh, csOpcodeCollisionSystem* sys)
  : scfImplementationType (this), system (sys), mesh (mesh), model (NULL),
  indexholder (NULL), vertholder (NULL), scale (1.0, 1.0, 1.0)
{
  opcMeshInt.SetCallback (&MeshCallback, this);

  csRef<iTriangleMesh> triMesh = FindColdetTriangleMesh (mesh, 
    system->baseID, system->colldetID);
  if (!triMesh)
    return;

  csVector3* vertices = triMesh->GetVertices ();
  size_t vertcount = triMesh->GetVertexCount ();
  csTriangle* triangles = triMesh->GetTriangles ();
  size_t tri_count = triMesh->GetTriangleCount ();

  Opcode::OPCODECREATE OPCC;
  size_t i;

  if (tri_count>=1)
  {
    model = new Opcode::Model;
    if (!model)
      return;

    vertholder = new Point [vertcount];
    indexholder = new unsigned int[3*tri_count];

    aabbox.StartBoundingBox ();
    for (i = 0; i < vertcount; i++)
    {
      aabbox.AddBoundingVertex (vertices[i]);
      vertholder[i].Set (vertices[i].x , vertices[i].y , vertices[i].z);
    }

    volume = aabbox.Volume ();

    int index = 0;
    for (i = 0 ; i < tri_count ; i++)
    {
      indexholder[index++] = triangles[i].a;
      indexholder[index++] = triangles[i].b;
      indexholder[index++] = triangles[i].c;
    }

    opcMeshInt.SetNbTriangles ((udword)tri_count);
    opcMeshInt.SetNbVertices ((udword)vertcount);

    // Mesh data
    OPCC.mIMesh = &opcMeshInt;
    OPCC.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
    OPCC.mNoLeaf = true;
    OPCC.mQuantized = true;
    OPCC.mKeepOriginal = false;
    OPCC.mCanRemap = false;
  }
  else
    return;

  // this should create the OPCODE model
  bool status = model->Build (OPCC);
  if (!status) 
  {
    delete model;
    model = NULL;
  }
}

csOpcodeCollider::~csOpcodeCollider ()
{
  if (model)
  {
    delete model;
    model = NULL;
  }
  delete[] indexholder;
  delete[] vertholder;
}

void csOpcodeCollider::MeshCallback (udword triangle_index,
                                     Opcode::VertexPointers& triangle,
                                     void* user_data)
{
  csOpcodeCollider* collider = (csOpcodeCollider*)user_data;

  udword *tri_array = collider->indexholder;
  Point *vertholder = collider->vertholder;
  int index = 3 * triangle_index;
  triangle.Vertex[0] = &vertholder [tri_array[index]] ;
  triangle.Vertex[1] = &vertholder [tri_array[index + 1]];
  triangle.Vertex[2] = &vertholder [tri_array[index + 2]];
}

TerrainCellCollider::TerrainCellCollider (iTerrainCell* cell, csOrthoTransform trans)
: cell (cell), cellTransform (trans)
{
  opcMeshInt.SetCallback (&MeshCallback, this);

  Opcode::OPCODECREATE OPCC;

  unsigned int width = cell->GetGridWidth ();
  unsigned int height = cell->GetGridHeight ();

  vertholder = new Point [width * height];
  indexholder = new unsigned int[3 * 2 * (width - 1) * (height - 1)];

  model = new Opcode::Model;

  opcMeshInt.SetNbTriangles (2 * (width-1) * (height-1));
  opcMeshInt.SetNbVertices ((udword)width * height);

  // Mesh data
  OPCC.mIMesh = &opcMeshInt;
  OPCC.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS |
    Opcode::SPLIT_GEOM_CENTER;
  OPCC.mNoLeaf = true;
  OPCC.mQuantized = true;
  OPCC.mKeepOriginal = false;
  OPCC.mCanRemap = true;

  float offset_x = cell->GetPosition ().x;
  float offset_y = cell->GetPosition ().y;

  float scale_x = cell->GetSize ().x / (width - 1);
  float scale_z = cell->GetSize ().z / (height - 1);

  for (unsigned int y = 0 ; y < height ; y++)
  {
    for (unsigned int x = 0 ; x < width ; x++)
    {
      int index = y*width + x;

      vertholder[index].Set (x * scale_x + offset_x,
        cell->GetHeight(x, height - y - 1),
        y * scale_z + offset_y);
    }
  }

  int i = 0;
  for (unsigned int y = 0 ; y < height-1 ; y++)
  {
    int yr = y * width;
    for (unsigned int x = 0 ; x < width-1 ; x++)
    {
      indexholder[i++] = yr + x;
      indexholder[i++] = yr+width + x;
      indexholder[i++] = yr + x+1;
      indexholder[i++] = yr + x+1;
      indexholder[i++] = yr+width + x;
      indexholder[i++] = yr+width + x+1;
    }
  }

  model->Build (OPCC);
}

TerrainCellCollider::~TerrainCellCollider ()
{
  if (model)
  {
    delete model;
    model = NULL;
  }
  if (indexholder)
    delete[] indexholder;
  indexholder = NULL;

  if (vertholder)
    delete[] vertholder;
  vertholder = NULL;
}

void TerrainCellCollider::MeshCallback (udword triangle_index,
                                     Opcode::VertexPointers& triangle,
                                     void* user_data)
{
  TerrainCellCollider* collider = (TerrainCellCollider*)user_data;

  udword *tri_array = collider->indexholder;
  Point *vertholder = collider->vertholder;
  int index = 3 * triangle_index;
  triangle.Vertex[0] = &vertholder [tri_array[index]] ;
  triangle.Vertex[1] = &vertholder [tri_array[index + 1]];
  triangle.Vertex[2] = &vertholder [tri_array[index + 2]];
}

csOpcodeColliderTerrain::csOpcodeColliderTerrain (iTerrainSystem* terrain,
                                                  csOpcodeCollisionSystem* sys)
 : scfImplementationType (this), terrainSystem (terrain), system (sys), scale(1.0, 1.0, 1.0)
{
  unload = true;

  terrain->AddCellLoadListener (this);
  // Find the transform of the terrain
  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (terrain);
  terrainTransform = mesh->GetMeshWrapper ()->GetMovable ()->GetFullTransform ();
  if(unload)
  {
    for (size_t i =0; i<terrainSystem->GetCellCount (); i++)
    {
      iTerrainCell* cell = terrainSystem->GetCell (i);

      if (cell->GetLoadState () != iTerrainCell::Loaded)
        continue;
      LoadCellToCollider (cell);       
    }
    unload = true;
  }

  volume = FLT_MAX;
}

csOpcodeColliderTerrain::~csOpcodeColliderTerrain ()
{
  for (size_t i = 0; i < colliders.GetSize (); i++)
    delete colliders[i];
}

void csOpcodeColliderTerrain::OnCellLoad (iTerrainCell *cell)
{
  LoadCellToCollider (cell);
}

void csOpcodeColliderTerrain::OnCellPreLoad (iTerrainCell *cell)
{

}

void csOpcodeColliderTerrain::OnCellUnload (iTerrainCell *cell)
{
  for (size_t i = 0;i<colliders.GetSize ();i++)
    if (colliders[i]->cell == cell)
    {
      delete colliders[i];
      colliders.DeleteIndexFast (i);
      break;
    }
}

void csOpcodeColliderTerrain::LoadCellToCollider (iTerrainCell* cell)
{
  csOrthoTransform cellTransform;
  cellTransform.Identity ();

  TerrainCellCollider* colliderData = new TerrainCellCollider (cell, cellTransform);
  colliders.Push (colliderData);
}
}
CS_PLUGIN_NAMESPACE_END (Opcode2)