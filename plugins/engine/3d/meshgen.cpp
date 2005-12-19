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
#include "plugins/engine/3d/meshgen.h"

csMeshGeneratorGeometry::csMeshGeneratorGeometry () :
	scfImplementationType (this)
{
  radius = 0.0f;
  density = 1.0f;
  total_max_dist = 0.0f;
}

void csMeshGeneratorGeometry::AddFactory (iMeshFactoryWrapper* factory,
    float maxdist)
{
  factories.Push (factory);
  maxdistances.Push (maxdist);
  if (maxdist > total_max_dist) total_max_dist = maxdist;
}

void csMeshGeneratorGeometry::RemoveFactory (size_t idx)
{
  factories.DeleteIndex (idx);
  maxdistances.DeleteIndex (idx);
}

void csMeshGeneratorGeometry::SetRadius (float radius)
{
  csMeshGeneratorGeometry::radius = radius;
}

void csMeshGeneratorGeometry::SetDensity (float density)
{
  csMeshGeneratorGeometry::density = density;
}

//--------------------------------------------------------------------------

csMeshGenerator::csMeshGenerator() : scfImplementationType (this)
{
  max_blocks = 100;
  cell_dim = 50;

  last_cellx = -1000000000;
  last_cellz = -1000000000;

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
}

void csMeshGenerator::AllocateBlock (int cx, int cz)
{
  int cidx = cz*cell_dim + cx;

  if (cells[cidx].block)
  {
    // Our block is already there. We just push it back to the
    // front of 'inuse_blocks' if it is not already there.
    csMGPositionBlock* block = cells[cidx].block;
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
    CS_ASSERT (block->parent_cell == 0);
    CS_ASSERT (block->next == block->prev == 0);
    block->parent_cell = cidx;
    cells[cidx].block = block;
    // Link block to the front.
    block->next = inuse_blocks;
    block->prev = 0;
    if (inuse_blocks) inuse_blocks->prev = block;
    else inuse_blocks_last = block;
    inuse_blocks = block;
  }
  else
  {
    // We need a new block and the cache is empty.
    // Now we take the last used block from 'inuse_blocks'.
    csMGPositionBlock* block = inuse_blocks_last;
    CS_ASSERT (block->parent_cell != csArrayItemNotFound);
    CS_ASSERT (block == cells[block->parent_cell].block);
    cells[block->parent_cell].block = 0;
    block->parent_cell = csArrayItemNotFound;

    // Unlink first.
    block->prev->next = 0;
    inuse_blocks_last = block->prev;

    // Link to front.
    block->next = inuse_blocks;
    block->prev = 0;
    inuse_blocks->prev = block;
    inuse_blocks = block;
  }
}

void csMeshGenerator::AllocateBlocks (const csVector3& pos)
{
  int cellx = GetCellX (pos.x);
  int cellz = GetCellZ (pos.z);

  if (cellx == last_cellx && cellz == last_cellz)
    return;

  last_cellx = cellx;
  last_cellz = cellz;

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
	  float wx = GetWorldX (x);
	  float wz = GetWorldX (z);
	  // @@@ Precalculate cell boxes or at least top-left coordinate for x/z?
	  csBox2 cell_box (wx, wz, wx + samplecellwidth_x,
	      wz + samplecellheight_z);
	  float sqdist = cell_box.SquaredPosDist (pos2d);
	  if (sqdist < sqmd)
	    AllocateBlock (x, z);
	  else
	    FreeMeshesInBlock (x, z);
        }
}

void csMeshGenerator::FreeMeshesInBlock (int cx, int cz)
{
  int cidx = cz*cell_dim + cx;
  if (cells[cidx].block)
  {
    // @@@ TODO
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

