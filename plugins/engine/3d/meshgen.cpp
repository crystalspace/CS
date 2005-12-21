/*
    Copyright (C) 2005 by Jorrit Tyberghein

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
#include "csgeom/math3d.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/meshgen.h"

csMeshGeneratorGeometry::csMeshGeneratorGeometry () :
	scfImplementationType (this)
{
  radius = 0.0f;
  density = 1.0f;
  total_max_dist = 0.0f;
}

static int CompareGeom (csMGGeom const& m1, csMGGeom const& m2)
{
  if (m1.maxdistance < m2.maxdistance) return -1;
  else if (m1.maxdistance > m2.maxdistance) return 1;
  else return 0;
}

void csMeshGeneratorGeometry::AddFactory (iMeshFactoryWrapper* factory,
    float maxdist)
{
  csMGGeom g;
  g.factory = factory;
  g.maxdistance = maxdist;
  g.sqmaxdistance = maxdist * maxdist;
  factories.InsertSorted (g, CompareGeom);
  if (maxdist > total_max_dist) total_max_dist = maxdist;
}

void csMeshGeneratorGeometry::RemoveFactory (size_t idx)
{
  factories.DeleteIndex (idx);
}

void csMeshGeneratorGeometry::SetRadius (float radius)
{
  csMeshGeneratorGeometry::radius = radius;
}

void csMeshGeneratorGeometry::SetDensity (float density)
{
  csMeshGeneratorGeometry::density = density;
}

size_t csMeshGeneratorGeometry::GetLODLevel (float sqdist)
{
  size_t i;
  for (i = 0 ; i < factories.Length () ; i++)
  {
    csMGGeom& geom = factories[i];
    if (sqdist <= geom.sqmaxdistance)
    {
      return i;
    }
  }
  CS_ASSERT (false);
  return csArrayItemNotFound;
}

csPtr<iMeshWrapper> csMeshGeneratorGeometry::AllocMesh (float sqdist)
{
  size_t lod = GetLODLevel (sqdist);
  if (lod == csArrayItemNotFound) return 0;

  csMGGeom& geom = factories[lod];
  // See if we have some mesh ready in the cache.
  if (geom.mesh_cache.Length () > 0)
  {
    return geom.mesh_cache.Pop ();
  }
  else
  {
    // Make a new mesh.
    csString name = "__mg__";
    // @@@ This technique gives us only support for 10 lod levels.
    CS_ASSERT (lod < 10);
    name += lod;
    name += geom.factory->QueryObject ()->GetName ();
    return csEngine::currentEngine->CreateMeshWrapper (geom.factory, name);
  }
}

void csMeshGeneratorGeometry::FreeMesh (iMeshWrapper* mesh)
{
  mesh->GetMovable ()->ClearSectors ();

  const char* name = mesh->QueryObject ()->GetName ();
  // First we have the string __mg__ and then a single digit with
  // the lod level.
  int lod = name[6]-'0';
  CS_ASSERT (lod >= 0 && lod < 10);
  csMGGeom& geom = factories[lod];
  CS_ASSERT (mesh->GetFactory () == geom.factory);
  geom.mesh_cache.Push (mesh);
}

bool csMeshGeneratorGeometry::IsRightLOD (iMeshWrapper* mesh, float sqdist)
{
  const char* name = mesh->QueryObject ()->GetName ();
  // First we have the string __mg__ and then a single digit with
  // the lod level.
  int lod = name[6]-'0';
  CS_ASSERT (lod >= 0 && lod < 10);

  size_t new_lod = GetLODLevel (sqdist);
  return new_lod == (size_t)lod;
}

//--------------------------------------------------------------------------

csMeshGenerator::csMeshGenerator() : scfImplementationType (this)
{
  max_blocks = 100;
  cell_dim = 50;

  sector = 0;

  total_max_dist = -1.0f;

  cells = new csMGCell [cell_dim * cell_dim];
  size_t i;
  for (i = 0 ; i < size_t (max_blocks) ; i++)
    cache_blocks.Push (new csMGPositionBlock ());
  inuse_blocks = 0;
  inuse_blocks_last = 0;

  prev_cells.MakeEmpty ();

  setup_cells = false;
}

csMeshGenerator::~csMeshGenerator ()
{
  delete[] cells;
  while (inuse_blocks)
  {
    csMGPositionBlock* n = inuse_blocks->next;
    delete inuse_blocks;
    inuse_blocks = n;
  }
}

float csMeshGenerator::GetTotalMaxDist ()
{
  if (total_max_dist < 0.0f)
  {
    total_max_dist = 0.0f;
    size_t i;
    for (i = 0 ; i < geometries.Length () ; i++)
    {
      float md = geometries[i]->GetTotalMaxDist ();
      if (md > total_max_dist) total_max_dist = md;
    }
    sq_total_max_dist = total_max_dist * total_max_dist;
  }
  return total_max_dist;
}

void csMeshGenerator::SetSampleBox (const csBox3& box)
{
  samplebox = box;
  setup_cells = false;
}

void csMeshGenerator::SetupSampleBox ()
{
  if (setup_cells) return;
  setup_cells = true;
  samplefact_x = float (cell_dim) / (samplebox.MaxX () - samplebox.MinX ());
  samplefact_z = float (cell_dim) / (samplebox.MaxZ () - samplebox.MinZ ());
  samplecellwidth_x = 1.0f / samplefact_x;
  samplecellheight_z = 1.0f / samplefact_z;
  int x, z;
  int idx = 0;
  for (z = 0 ; z < cell_dim ; z++)
  {
    float wz = GetWorldX (z);
    for (x = 0 ; x < cell_dim ; x++)
    {
      float wx = GetWorldX (x);
      cells[idx].box.Set (wx, wz, wx + samplecellwidth_x,
	      wz + samplecellheight_z);
      // Here we need to calculate the meshe relevant for this cell (i.e.
      // meshes that intersect this cell as seen in 2D space).
      // @@@ For now we just copy the list of meshes from csMeshGenerator.
      cells[idx].meshes = meshes;
      idx++;
    }
  }
}

void csMeshGenerator::GeneratePositions (int cidx, csMGCell& cell,
	csMGPositionBlock* block)
{
  random.Initialize ((unsigned int)cidx); // @@@ Consider using a better seed?

  block->positions.Empty ();

  const csBox2& box = cell.box;
  float box_area = box.Area ();

  size_t i, j, g;
  for (g = 0 ; g < geometries.Length () ; g++)
  {
    csMGPosition pos;
    pos.geom_type = g;
    float density = geometries[g]->GetDensity ();
    size_t count = size_t (density * box_area);
    for (j = 0 ; j < count ; j++)
    {
      float x = random.Get (box.MinX (), box.MaxX ());
      float z = random.Get (box.MinY (), box.MaxY ());
      csVector3 start (x, samplebox.MaxY (), z);
      csVector3 end = start;
      end.y = samplebox.MinY ();
      bool hit = false;
      for (i = 0 ; i < cell.meshes.Length () ; i++)
      {
        csHitBeamResult rc = cell.meshes[i]->HitBeam (start, end);
	if (rc.hit)
	{
          pos.position = rc.isect;
	  end.y = rc.isect.y + 0.0001;
	  hit = true;
	}
      }
      if (hit)
      {
        block->positions.Push (pos);
      }
    }
  }
}

void csMeshGenerator::AllocateBlock (int cidx, csMGCell& cell)
{
  if (cell.block)
  {
    // Our block is already there. We just push it back to the
    // front of 'inuse_blocks' if it is not already there.
    csMGPositionBlock* block = cell.block;
    if (block->prev)
    {
      // Unlink first.
      block->prev->next = block->next;
      if (block->next) block->next->prev = block->prev;
      else inuse_blocks_last = block->prev;

      // Link to front.
      block->next = inuse_blocks;
      block->prev = 0;
      inuse_blocks->prev = block;
      inuse_blocks = block;
    }
  }
  else if (cache_blocks.Length () > 0)
  {
    // We need a new block and one is available in the cache.
    csMGPositionBlock* block = cache_blocks.Pop ();
    CS_ASSERT (block->parent_cell == csArrayItemNotFound);
    CS_ASSERT (block->next == 0 && block->prev == 0);
    block->parent_cell = cidx;
    cell.block = block;
    // Link block to the front.
    block->next = inuse_blocks;
    block->prev = 0;
    if (inuse_blocks) inuse_blocks->prev = block;
    else inuse_blocks_last = block;
    inuse_blocks = block;

    GeneratePositions (cidx, cell, block);
  }
  else
  {
    // We need a new block and the cache is empty.
    // Now we take the last used block from 'inuse_blocks'.
    csMGPositionBlock* block = inuse_blocks_last;
    CS_ASSERT (block->parent_cell != csArrayItemNotFound);
    CS_ASSERT (block == cells[block->parent_cell].block);
    cells[block->parent_cell].block = block;
    block->parent_cell = cidx;
    cell.block = block;

    // Unlink first.
    block->prev->next = 0;
    inuse_blocks_last = block->prev;

    // Link to front.
    block->next = inuse_blocks;
    block->prev = 0;
    inuse_blocks->prev = block;
    inuse_blocks = block;

    GeneratePositions (cidx, cell, block);
  }
}

void csMeshGenerator::AllocateMeshes (csMGCell& cell, const csVector3& pos)
{
  CS_ASSERT (cell.block != 0);
  CS_ASSERT (sector != 0);
  csArray<csMGPosition>& positions = cell.block->positions;
  GetTotalMaxDist ();
  size_t i;
  for (i = 0 ; i < positions.Length () ; i++)
  {
    csMGPosition& p = positions[i];
    float sqdist = csSquaredDist::PointPoint (pos, p.position);
    if (sqdist < sq_total_max_dist)
    {
      if (!p.mesh)
      {
        // We didn't have a mesh here so we allocate one.
        csRef<iMeshWrapper> mesh = geometries[p.geom_type]->AllocMesh (sqdist);
	if (mesh)
	{
          p.mesh = mesh;
          mesh->GetMovable ()->SetSector (sector);
	  mesh->GetMovable ()->SetPosition (sector, p.position);
	  mesh->GetMovable ()->UpdateMove ();
        }
      }
      else
      {
        // We already have a mesh but we check here if we should switch LOD level.
	if (!geometries[p.geom_type]->IsRightLOD (p.mesh, sqdist))
	{
	  // We need a different mesh here.
          geometries[p.geom_type]->FreeMesh (p.mesh);
          csRef<iMeshWrapper> mesh = geometries[p.geom_type]->AllocMesh (sqdist);
          p.mesh = mesh;
	  if (mesh)
	  {
            mesh->GetMovable ()->SetSector (sector);
	    mesh->GetMovable ()->SetPosition (sector, p.position);
	    mesh->GetMovable ()->UpdateMove ();
	  }
	}
      }
    }
    else
    {
      if (p.mesh)
      {
        geometries[p.geom_type]->FreeMesh (p.mesh);
        p.mesh = 0;
      }
    }
  }
}

void csMeshGenerator::AllocateBlocks (const csVector3& pos)
{
  SetupSampleBox ();
  
  int cellx = GetCellX (pos.x);
  int cellz = GetCellZ (pos.z);

  // Total maximum distance for all geometries.
  float md = GetTotalMaxDist ();
  float sqmd = md * md;
  // Same in cell counts:
  int cell_x_md = 1+int (md * samplefact_x);
  int cell_z_md = 1+int (md * samplefact_z);

  // @@@ This can be done more efficiently.
  csVector2 pos2d (pos.x, pos.z);
  int x, z;

  int minz = cellz - cell_z_md;
  if (minz < 0) minz = 0;
  int maxz = cellz + cell_z_md+1;
  if (maxz > cell_dim) maxz = cell_dim;

  int minx = cellx - cell_x_md;
  if (minx < 0) minx = 0;
  int maxx = cellx + cell_x_md+1;
  if (maxx > cell_dim) maxx = cell_dim;

  // Calculate the intersection of minx, ... with prev_minx, ...
  // This intersection represent all cells that are not used this frame
  // but may potentially have gotten meshes the previous frame. We need
  // to free those meshes.
  csRect cur_cells (minx, minz, maxx, maxz);
  if (!prev_cells.IsEmpty ())
  {
    for (z = prev_cells.ymin ; z < prev_cells.ymax ; z++)
      for (x = prev_cells.xmin ; x < prev_cells.xmax ; x++)
        if (!cur_cells.Contains (x, z))
	{
	  int cidx = z*cell_dim + x;
	  csMGCell& cell = cells[cidx];
          FreeMeshesInBlock (cell);
        }
  }
  prev_cells = cur_cells;

  // Now allocate the cells that are close enough.
  for (z = minz ; z < maxz ; z++)
    for (x = minx ; x < maxx ; x++)
    {
      int cidx = z*cell_dim + x;
      csMGCell& cell = cells[cidx];
      float sqdist = cell.box.SquaredPosDist (pos2d);

      if (sqdist < sqmd)
      {
        AllocateBlock (cidx, cell);
        AllocateMeshes (cell, pos);
      }
      else
      {
        // Block is out of range. We keep the block in the cache
        // but free all meshes that are in it.
        FreeMeshesInBlock (cell);
      }
    }
}

void csMeshGenerator::FreeMeshesInBlock (csMGCell& cell)
{
  if (cell.block)
  {
    csArray<csMGPosition>& positions = cell.block->positions;
    size_t i;
    for (i = 0 ; i < positions.Length () ; i++)
    {
      if (positions[i].mesh)
      {
        CS_ASSERT (positions[i].geom_type >= 0);
        CS_ASSERT (positions[i].geom_type < geometries.Length ());
        geometries[positions[i].geom_type]->FreeMesh (positions[i].mesh);
        positions[i].mesh = 0;
      }
    }
  }
}

iMeshGeneratorGeometry* csMeshGenerator::CreateGeometry ()
{
  csMeshGeneratorGeometry* geom = new csMeshGeneratorGeometry ();
  geometries.Push (geom);
  total_max_dist = -1.0f;	// Requires a recalculate.
  geom->DecRef ();
  return geom;
}

void csMeshGenerator::RemoveGeometry (size_t idx)
{
  geometries.DeleteIndex (idx);
}

void csMeshGenerator::RemoveMesh (size_t idx)
{
  meshes.DeleteIndex (idx);
}

