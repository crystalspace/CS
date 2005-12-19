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

csPtr<iMeshWrapper> csMeshGeneratorGeometry::AllocMesh (float sqdist)
{
  size_t i;
  for (i = 0 ; i < factories.Length () ; i++)
  {
    csMGGeom& geom = factories[i];
    if (sqdist <= geom.sqmaxdistance)
    {
      // See if we have some mesh ready in the cache.
      if (geom.mesh_cache.Length () > 0)
      {
        return geom.mesh_cache.Pop ();
      }
      else
      {
        // Make a new mesh.
	csString name = "__mg__";
	name += geom.factory->QueryObject ()->GetName ();
	return csEngine::currentEngine->CreateMeshWrapper (geom.factory,
		name);
      }
    }
  }
  return 0;
}

void csMeshGeneratorGeometry::FreeMesh (iMeshWrapper* mesh)
{
  mesh->GetMovable ()->ClearSectors ();

  size_t i;
  iMeshFactoryWrapper* fact = mesh->GetFactory ();
  for (i = 0 ; i < factories.Length () ; i++)
  {
    csMGGeom& geom = factories[i];
    if (geom.factory == fact)
    {
      geom.mesh_cache.Push (mesh);
      return;
    }
  }
  CS_ASSERT (false);
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
}

csMeshGenerator::~csMeshGenerator ()
{
  delete[] cells;
  while (inuse_blocks)
  {
    csMGPositionBlock* n = inuse_blocks->next;
    delete n;
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
    }
    idx++;
  }
}

void csMeshGenerator::GeneratePositions (int cidx, csMGCell& cell,
	csMGPositionBlock* block)
{
  srand (cidx);	// @@@ Consider using a better seed?

  block->positions.Empty ();

  const csBox2& box = cell.box;

  // For now just generate four positions in every cell as a test.
  // @@@ TODO: use density.
  csMGPosition pos;
  pos.position.Set (box.MinX () + (box.MaxX ()-box.MinX ()) / .25f, 0.0f,
		    box.MinY () + (box.MaxY ()-box.MinY ()) / .25f);
  pos.geom_type = 0;	// @@@ Temporary.
  block->positions.Push (pos);
  pos.position.Set (box.MinX () + (box.MaxX ()-box.MinX ()) / .25f, 0.0f,
		    box.MinY () + (box.MaxY ()-box.MinY ()) / .75f);
  pos.geom_type = 0;	// @@@ Temporary.
  block->positions.Push (pos);
  pos.position.Set (box.MinX () + (box.MaxX ()-box.MinX ()) / .75f, 0.0f,
		    box.MinY () + (box.MaxY ()-box.MinY ()) / .25f);
  pos.geom_type = 0;	// @@@ Temporary.
  block->positions.Push (pos);
  pos.position.Set (box.MinX () + (box.MaxX ()-box.MinX ()) / .75f, 0.0f,
		    box.MinY () + (box.MaxY ()-box.MinY ()) / .75f);
  pos.geom_type = 0;	// @@@ Temporary.
  block->positions.Push (pos);
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
    CS_ASSERT (block->next == block->prev == 0);
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
    if (!positions[i].mesh)
    {
      float sqdist = csSquaredDist::PointPoint (pos, positions[i].position);
      if (sqdist < sq_total_max_dist)
      {
        csRef<iMeshWrapper> mesh = geometries[positions[i].geom_type]->AllocMesh (
      	  sqdist);
        positions[i].mesh = mesh;
        mesh->GetMovable ()->SetSector (sector);
	mesh->GetMovable ()->SetPosition (sector, positions[i].position);
	mesh->GetMovable ()->UpdateMove ();
      }
    }
  }
}

void csMeshGenerator::AllocateBlocks (const csVector3& pos)
{
  int cellx = GetCellX (pos.x);
  int cellz = GetCellZ (pos.z);

  // Total maximum distance for all geometries.
  float md = GetTotalMaxDist ();
  float sqmd = md * md;
  // Same in cell counts:
  int cell_x_md = int (md * samplefact_x);
  int cell_z_md = int (md * samplefact_z);

  // @@@ This can be done more efficiently.
  csVector2 pos2d (pos.x, pos.z);
  int x, z;
  for (z = cellz - cell_z_md ; z <= cellz + cell_z_md ; z++)
    if (z >= 0 && z < cell_dim)
      for (x = cellx - cell_x_md ; x <= cellx + cell_x_md ; x++)
	if (x >= 0 && x < cell_dim)
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

