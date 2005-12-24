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
#include "csgeom/matrix3.h"
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

csPtr<iMeshWrapper> csMeshGeneratorGeometry::AllocMesh (float sqdist,
    size_t& lod)
{
  lod = GetLODLevel (sqdist);
  if (lod == csArrayItemNotFound) return 0;

  csMGGeom& geom = factories[lod];
  // See if we have some mesh ready in the setaside or normal cache.
  if (geom.mesh_setaside.Length () > 0)
  {
    return geom.mesh_setaside.Pop ();
  }
  else if (geom.mesh_cache.Length () > 0)
  {
    return geom.mesh_cache.Pop ();
  }
  else
  {
    return csEngine::currentEngine->CreateMeshWrapper (geom.factory, 0);
  }
}

void csMeshGeneratorGeometry::SetAsideMesh (iMeshWrapper* mesh, size_t lod)
{
  csMGGeom& geom = factories[lod];
  geom.mesh_setaside.Push (mesh);
}

void csMeshGeneratorGeometry::FreeSetAsideMeshes ()
{
  size_t lod;
  for (lod = 0 ; lod < factories.Length () ; lod++)
  {
    csMGGeom& geom = factories[lod];
    while (geom.mesh_setaside.Length () > 0)
    {
      csRef<iMeshWrapper> mesh = geom.mesh_setaside.Pop ();
      mesh->GetMovable ()->ClearSectors ();
      geom.mesh_cache.Push (mesh);
    }
  }
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

bool csMeshGeneratorGeometry::IsRightLOD (iMeshWrapper* mesh, float sqdist,
    size_t current_lod)
{
  // With only one lod level we are always right.
  if (factories.Length () <= 1) return true;
  if (current_lod == 0)
    return (sqdist <= factories[0].sqmaxdistance);
  else
    return (sqdist <= factories[current_lod].sqmaxdistance) &&
      (sqdist > factories[current_lod-1].sqmaxdistance);
}

//--------------------------------------------------------------------------

csMeshGenerator::csMeshGenerator() : scfImplementationType (this)
{
  max_blocks = 100;
  cell_dim = 50;
  use_density_scaling = false;
  use_alpha_scaling = false;

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

  for (i = 0 ; i < CS_GEOM_MAX_ROTATIONS ; i++)
  {
    rotation_matrices[i] = csYRotMatrix3 (2.0f*PI * float (i)
    	/ float (CS_GEOM_MAX_ROTATIONS));
  }

  alpha_priority = csEngine::currentEngine->GetAlphaRenderPriority ();
  object_priority = csEngine::currentEngine->GetObjectRenderPriority ();
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

void csMeshGenerator::SetDensityScale (float mindist, float maxdist,
  	float maxdensityfactor)
{
  use_density_scaling = true;
  density_mindist = mindist;
  sq_density_mindist = density_mindist * density_mindist;
  density_maxdist = maxdist;
  density_maxfactor = maxdensityfactor;

  density_scale = (1.0f-density_maxfactor) / (density_maxdist - density_mindist);
}

void csMeshGenerator::SetAlphaScale (float mindist, float maxdist)
{
  use_alpha_scaling = true;
  alpha_mindist = mindist;
  sq_alpha_mindist = alpha_mindist * alpha_mindist;
  alpha_maxdist = maxdist;

  alpha_scale = 1.0f / (alpha_maxdist - alpha_mindist);
}

void csMeshGenerator::SetCellCount (int number)
{
  if (setup_cells)
  {
    // We already setup so we need to deallocate all blocks first.
    int x, z;
    for (z = 0 ; z < cell_dim ; z++)
      for (x = 0 ; x < cell_dim ; x++)
      {
	int cidx = z*cell_dim + x;
	csMGCell& cell = cells[cidx];
        FreeMeshesInBlock (cell);
	if (cell.block)
	{
	  cell.block->parent_cell = csArrayItemNotFound;
	  cache_blocks.Push (cell.block);
	  cell.block->next = cell.block->prev = 0;
	  cell.block = 0;
        }
      }
    inuse_blocks = 0;
    inuse_blocks_last = 0;
    setup_cells = false;
  }
  cell_dim = number;
  delete[] cells;
  cells = new csMGCell [cell_dim * cell_dim];
}

void csMeshGenerator::SetBlockCount (int number)
{
  if (setup_cells)
  {
    // We already setup so we need to deallocate all blocks first.
    int x, z;
    for (z = 0 ; z < cell_dim ; z++)
      for (x = 0 ; x < cell_dim ; x++)
      {
	int cidx = z*cell_dim + x;
	csMGCell& cell = cells[cidx];
        FreeMeshesInBlock (cell);
	if (cell.block)
	{
	  cell.block->parent_cell = csArrayItemNotFound;
	  cache_blocks.Push (cell.block);
	  cell.block->next = cell.block->prev = 0;
	  cell.block = 0;
        }
      }
    inuse_blocks = 0;
    inuse_blocks_last = 0;

    setup_cells = false;
  }
  size_t i;
  if (number > max_blocks)
  {
    for (i = max_blocks ; i < size_t (number) ; i++)
      cache_blocks.Push (new csMGPositionBlock ());
  }
  else
  {
    for (i = number ; i < size_t (max_blocks) ; i++)
    {
      csMGPositionBlock* block = cache_blocks.Pop ();
      delete block;
    }
  }
  max_blocks = number;
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

size_t csMeshGenerator::CountPositions (int cidx, csMGCell& cell)
{
  random.Initialize ((unsigned int)cidx); // @@@ Consider using a better seed?
  size_t counter = 0;

  const csBox2& box = cell.box;
  float box_area = box.Area ();

  size_t i, j, g;
  for (g = 0 ; g < geometries.Length () ; g++)
  {
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
	  end.y = rc.isect.y + 0.0001;
	  hit = true;
	  break;
	}
      }
      if (hit)
      {
        counter++;
      }
    }
  }
  return counter;
}

size_t csMeshGenerator::CountAllPositions ()
{
  size_t counter = 0;
  int x, z;
  for (z = 0 ; z < cell_dim ; z++)
    for (x = 0 ; x < cell_dim ; x++)
    {
      int cidx = z*cell_dim + x;
      csMGCell& cell = cells[cidx];
      counter += CountPositions (cidx, cell);
    }
  return counter;
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
        int rot = int (random.Get (CS_GEOM_MAX_ROTATIONS));
	if (rot < 0) rot = 0;
	else if (rot >= CS_GEOM_MAX_ROTATIONS) rot = CS_GEOM_MAX_ROTATIONS-1;
        pos.rotation = rot;
	pos.random = random.Get ();
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
    cells[block->parent_cell].block = 0;
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

void csMeshGenerator::SetFade (csMGPosition& p, float factor)
{
  if (factor < .01)
  {
    if (p.last_mixmode == CS_FX_TRANSPARENT)
      return;
    p.last_mixmode = CS_FX_TRANSPARENT;
    p.mesh->GetMeshObject ()->SetMixMode (CS_FX_TRANSPARENT);
  }
  else if (factor < .99)
  {
    if (p.mesh->GetRenderPriority () != alpha_priority)
    {
      p.mesh->SetRenderPriority (alpha_priority);
      p.mesh->SetZBufMode (CS_ZBUF_TEST);
    }
    p.last_mixmode = CS_FX_SETALPHA (1.0f-factor);
    p.mesh->GetMeshObject ()->SetMixMode (p.last_mixmode);
  }
  else
  {
    if (p.last_mixmode == CS_FX_COPY)
      return;
    p.last_mixmode = CS_FX_COPY;
    p.mesh->GetMeshObject ()->SetMixMode (CS_FX_COPY);
    p.mesh->SetRenderPriority (object_priority);
    p.mesh->SetZBufMode (CS_ZBUF_USE);
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
	// But first we test if we have density scaling.
	bool show = true;
        if (use_density_scaling && sqdist > sq_density_mindist)
        {
          float dist = sqrt (sqdist);
          float factor = (density_maxdist - dist) * density_scale
	    + density_maxfactor;
          if (factor < 0) factor = 0;
          else if (factor > 1) factor = 1;
          if (p.random > factor) show = false;
        }

	if (show)
	{
          csRef<iMeshWrapper> mesh = geometries[p.geom_type]->AllocMesh (
	      sqdist, p.lod);
	  if (mesh)
	  {
            p.mesh = mesh;
	    p.last_mixmode = ~0;
	    mesh->GetMovable ()->SetPosition (sector, p.position);
	    mesh->GetMovable ()->SetTransform (rotation_matrices[p.rotation]);
	    mesh->GetMovable ()->UpdateMove ();
          }
	}
      }
      else
      {
        // We already have a mesh but we check here if we should switch LOD level.
	if (!geometries[p.geom_type]->IsRightLOD (p.mesh, sqdist, p.lod))
	{
	  // We need a different mesh here.
          geometries[p.geom_type]->SetAsideMesh (p.mesh, p.lod);
          csRef<iMeshWrapper> mesh = geometries[p.geom_type]->AllocMesh (
	      sqdist, p.lod);
          p.mesh = mesh;
	  if (mesh)
	  {
	    p.last_mixmode = ~0;
	    mesh->GetMovable ()->SetPosition (sector, p.position);
	    mesh->GetMovable ()->SetTransform (rotation_matrices[p.rotation]);
	    mesh->GetMovable ()->UpdateMove ();
	  }
	}
      }
      if (p.mesh && use_alpha_scaling)
      {
	// This is used only if we have both density scaling and
	// alpha scaling. It is the offset to correct the distance at which
	// the object will disappear. We can adapt alpha scaling to scale
	// based on that new distance.
	float correct_dist = 0;
	float correct_sq_alpha_mindist = sq_alpha_mindist;

#if 0
        if (use_density_scaling && sqdist > sq_density_mindist)
        {
	  correct_dist = (p.random - density_maxfactor) / density_scale;
	  if (correct_dist < 0) correct_dist = 0;
	  correct_sq_alpha_mindist = alpha_mindist-correct_dist;
	  correct_sq_alpha_mindist *= correct_sq_alpha_mindist;
//printf ("p.mesh='%s' sqdist=%g dist=%g correct_sq_alpha_mindist=%g correct_dist=%g p.random=%g", p.mesh->QueryObject ()->GetName (), sqdist, sqrt (sqdist), correct_sq_alpha_mindist, correct_dist, p.random); fflush (stdout);
        }
#endif

	float factor = 1.0;
        if (sqdist > correct_sq_alpha_mindist)
        {
          float dist = sqrt (sqdist);
          factor = (alpha_maxdist-correct_dist - dist) * alpha_scale;
	}
	//printf (" -> factor=%g\n", factor); fflush (stdout);
	SetFade (p, factor);
      }
    }
    else
    {
      if (p.mesh)
      {
        geometries[p.geom_type]->SetAsideMesh (p.mesh, p.lod);
        p.mesh = 0;
      }
    }
  }
}

void csMeshGenerator::AllocateBlocks (const csVector3& pos)
{
  SetupSampleBox ();
  //printf ("positions=%d\n", CountAllPositions ()); fflush (stdout);
  
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

  int total_needed = 0;

  // Now allocate the cells that are close enough.
  for (z = minz ; z < maxz ; z++)
  {
    int cidx = z*cell_dim + minx;
    for (x = minx ; x < maxx ; x++)
    {
      csMGCell& cell = cells[cidx];
      float sqdist = cell.box.SquaredPosDist (pos2d);

      if (sqdist < sqmd)
      {
        total_needed++;
	if (total_needed >= max_blocks)
	{
	  csEngine::currentEngine->Error (
	  	"The mesh generator need more blocks then %d!", max_blocks);
	  return;	// @@@ What to do here???
	}
        AllocateBlock (cidx, cell);
        AllocateMeshes (cell, pos);
      }
      else
      {
        // Block is out of range. We keep the block in the cache
        // but free all meshes that are in it.
        FreeMeshesInBlock (cell);
      }
      cidx++;
    }
  }

  // Now really free the meshes we didn't reuse.
  size_t i;
  for (i = 0 ; i < geometries.Length () ; i++)
    geometries[i]->FreeSetAsideMeshes ();
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
        geometries[positions[i].geom_type]->SetAsideMesh (positions[i].mesh,
	    positions[i].lod);
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

