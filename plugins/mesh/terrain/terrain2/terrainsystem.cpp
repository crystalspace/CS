/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#include "csgeom/box.h"
#include "csgeom/math3d.h"
#include "csgeom/plane3.h"
#include "csgeom/sphere.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "cstool/rviewclipper.h"

#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "ivideo/rendermesh.h"
#include "iutil/objreg.h"

#include "terrainsystem.h"
#include "cell.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

csTerrainSystem::csTerrainSystem (iMeshObjectFactory* factory, 
  iTerrainRenderer* renderer, iTerrainCollider* collider,
  iTerrainDataFeeder* feeder)
  : scfImplementationType (this, (iEngine*)0), factory (factory),
    renderer (renderer), collider (collider), dataFeeder (feeder),
    virtualViewDistance (2.0f), maxLoadedCells (~0), autoPreload (false),
    bbStarted (false)
{
  if (renderer)
    renderer->ConnectTerrain (this);
}

csTerrainSystem::~csTerrainSystem ()
{
  cells.Empty();
  if (renderer)
    renderer->DisconnectTerrain (this);
}

void csTerrainSystem::AddCell (csTerrainCell* cell)
{
  cells.Push (cell);

  if (!bbStarted)
  {
    boundingbox.Empty ();
    boundingbox.StartBoundingBox ();
    bbStarted = true;
  }

  boundingbox += cell->GetBBox ();

  ShapeChanged ();
}

void csTerrainSystem::FireLoadCallbacks (csTerrainCell* cell)
{
  for (size_t i = 0; i < loadCallbacks.GetSize (); ++i)
  {
    loadCallbacks[i]->OnCellLoad (cell);
  }
}

void csTerrainSystem::FirePreLoadCallbacks (csTerrainCell* cell)
{
  for (size_t i = 0; i < loadCallbacks.GetSize (); ++i)
  {
    loadCallbacks[i]->OnCellPreLoad (cell);
  }
}

void csTerrainSystem::FireUnloadCallbacks (csTerrainCell* cell)
{
  for (size_t i = 0; i < loadCallbacks.GetSize (); ++i)
  {
    loadCallbacks[i]->OnCellUnload (cell);
  }
}

void csTerrainSystem::FireHeightUpdateCallbacks (csTerrainCell* cell,
    const csRect& rectangle)
{
  for (size_t i = 0; i < heightDataCallbacks.GetSize (); ++i)
  {
    heightDataCallbacks[i]->OnHeightUpdate (cell, rectangle);
  }
}

void csTerrainSystem::CellSizeUpdate (csTerrainCell* cell)
{
  ComputeBBox ();
}


iTerrainCell* csTerrainSystem::GetCell (const char* name, bool loadData)
{
  iTerrainCell* cell = 0;

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    if (!strcmp (cells[i]->GetName (), name))
    {
      cell = cells[i];
    }
  }

  if (loadData && cell)
  {
    cell->SetLoadState (iTerrainCell::Loaded);
  }

  return cell;
}

iTerrainCell* csTerrainSystem::GetCell (const csVector2& pos, bool loadData)
{
  iTerrainCell* cell = 0;

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    const csVector2& cell_pos = cells[i]->GetPosition ();
    const csVector3& cell_size = cells[i]->GetSize ();

    if (cell_pos.x <= pos.x + EPSILON &&
        cell_pos.x + cell_size.x >= pos.x - EPSILON &&
        cell_pos.y <= pos.y + EPSILON &&
        cell_pos.y + cell_size.z >= pos.y - EPSILON)
    {
      cell = cells[i];
    }
  }

  if (loadData && cell)
  {
    cell->SetLoadState (iTerrainCell::Loaded);
  }

  return cell;
}

iTerrainCell* csTerrainSystem::GetCell (size_t index, bool loadData)
{
  iTerrainCell* cell = 0;

  if (index < cells.GetSize ())
  {
    cell = cells[index];
  }
  
  if (loadData && cell)
  {
    cell->SetLoadState (iTerrainCell::Loaded);
  }

  return cell;
}

size_t csTerrainSystem::GetCellCount () const
{
  return cells.GetSize ();
}

const csRefArray<iMaterialWrapper>& csTerrainSystem::GetMaterialPalette () const
{
  return materialPalette;
}

void csTerrainSystem::SetMaterialPalette (const csRefArray<iMaterialWrapper>& mp)
{
  materialPalette = mp;

  renderer->OnMaterialPaletteUpdate (materialPalette);
}

bool csTerrainSystem::CollideSegment (const csVector3& start, const csVector3&
                               end, bool oneHit, iTerrainVector3Array* points,
                               iMaterialArray* materials)
{
  if (!collider) 
    return false;

  size_t size = points->GetSize ();

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    csSegment3 seg(start, end);
    csBox3 box = cells[i]->GetBBox ();

    csVector3 isect;

    if (csIntersect3::BoxSegment (box, seg, isect) >= 0)
    {
      seg.SetStart (seg.End ());
      seg.SetEnd (isect);
      
      if (csIntersect3::BoxSegment (box, seg, isect) >= 0)
        seg.SetStart (isect);

      if (cells[i]->CollideSegment (seg.End (), seg.Start (), oneHit, points)
          && oneHit)
      {
        if (materials)
          materials->Push(cells[i]->GetBaseMaterial());
        return true;
      }
    }
  }

  return size != points->GetSize ();
}

csTerrainColliderCollideSegmentResult csTerrainSystem::CollideSegment (
      const csVector3& start, const csVector3& end, bool use_ray)
{
  csTerrainColliderCollideSegmentResult rc;
  rc.hit = false;

  if (!collider) 
    return rc;

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    csSegment3 seg (start, end);
    csBox3 box = cells[i]->GetBBox ();

    csVector3 isect;

    if (csIntersect3::ClipSegmentBox (seg, box, use_ray))
    {
      rc = cells[i]->CollideSegment (seg.End (), seg.Start ());
      if (rc.hit)
        return rc;
    }
  }

  return rc;
}

bool csTerrainSystem::CollideTriangles (const csVector3* vertices,
  size_t tri_count, const unsigned int* indices, float radius,
  const csReversibleTransform& trans, bool oneHit,
  iTerrainCollisionPairArray* pairs)
{
  if (!collider) 
    return false;

  size_t size = pairs->GetSize ();

  csSphere sphere (csVector3 (0, 0, 0), radius);
  sphere = trans.This2Other (sphere);
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    csBox3 box = cells[i]->GetBBox ();

    if (csIntersect3::BoxSphere (box, sphere.GetCenter (), sphere.GetRadius ()))
    {
      if (cells[i]->GetLoadState () != csTerrainCell::Loaded)
      {
        cells[i]->SetLoadState (csTerrainCell::Loaded);
      }

      if (cells[i]->CollideTriangles (vertices, tri_count, indices, radius,
          trans, oneHit, pairs) && oneHit)
        return true;
    }
  }

  return size != pairs->GetSize ();
}

bool csTerrainSystem::Collide (iCollider* collider,
  float radius, const csReversibleTransform& trans,
  bool oneHit, iTerrainCollisionPairArray* pairs)
{
  if (!this->collider) 
    return false;

  size_t size = pairs->GetSize ();

  csSphere sphere (csVector3 (0, 0, 0), radius);
  sphere = trans.This2Other (sphere);
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    csBox3 box = cells[i]->GetBBox ();

    if (csIntersect3::BoxSphere (box, sphere.GetCenter (), sphere.GetRadius ()))
    {
      if (cells[i]->Collide (collider, radius, trans, oneHit, pairs) &&
        oneHit)
        return true;
    }
  }

  return size != pairs->GetSize ();
}

float csTerrainSystem::GetVirtualViewDistance () const
{
  return virtualViewDistance;
}

void csTerrainSystem::SetVirtualViewDistance (float distance)
{
  virtualViewDistance = distance;
}

bool csTerrainSystem::GetAutoPreLoad () const
{
  return autoPreload;
}

void csTerrainSystem::SetAutoPreLoad (bool mode)
{
  autoPreload = mode;
}

void csTerrainSystem::PreLoadCells (iRenderView* rview, iMovable* movable)
{
  csPlane3 planes[10];
  uint32 frustum_mask;
  
  csOrthoTransform c2ot = rview->GetCamera ()->GetTransform ();
  c2ot /= movable->GetFullTransform ();
  
  CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext (),
      c2ot, planes, frustum_mask);
  
  /// Here I should not just multiply by vview_distance, because it scales the
  /// frustum in N times, and N has nothing to do with distance :)
  /// Left for now, because, well, I'm not sure of the desired behavior
  for (int pi = 0; pi < 10; ++pi)
  {
    planes[pi].DD *= virtualViewDistance;
  }
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    if (!cells[i]->GetRenderProperties ()->GetVisible ()) 
      continue;

    uint32 out_mask;
    
    csBox3 box = cells[i]->GetBBox ();
    
    if (csIntersect3::BoxFrustum (box, planes, frustum_mask, out_mask) &&
        cells[i]->GetLoadState () == csTerrainCell::NotLoaded)
    {
      cells[i]->SetLoadState (csTerrainCell::PreLoaded);
    }
  }
}

float csTerrainSystem::GetHeight (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) 
    return cell->GetHeight (pos - cell->GetPosition ());
  else 
    return 0;
}

csVector3 csTerrainSystem::GetTangent (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) 
    return cell->GetTangent (pos - cell->GetPosition ());
  else 
    return csVector3(0, 0, 0);
}

csVector3 csTerrainSystem::GetBinormal (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) 
    return cell->GetBinormal (pos - cell->GetPosition ());
  else 
    return csVector3(0, 0, 0);
}

csVector3 csTerrainSystem::GetNormal (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) 
    return cell->GetNormal (pos - cell->GetPosition ());
  else 
    return csVector3(0, 0, 0);
}

size_t csTerrainSystem::GetMaxLoadedCells () const
{
  return maxLoadedCells;
}

void csTerrainSystem::SetMaxLoadedCells (size_t value)
{
  maxLoadedCells = value;
}

static int CellLRUCompare (csTerrainCell* const& r1, csTerrainCell* const& r2)
{
  return r1->GetLRU () - r2->GetLRU ();
}

void csTerrainSystem::UnloadOldCells ()
{
  if (maxLoadedCells == 0)
    return;

  // count loaded cells
  csArray<csTerrainCell*> loadedCells;

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    if (cells[i]->GetLoadState () == iTerrainCell::Loaded)
    {
      loadedCells.InsertSorted (cells[i], CellLRUCompare);
    }
  }

  if (loadedCells.GetSize () <= maxLoadedCells) 
    return;

  size_t to_delete = loadedCells.GetSize () - maxLoadedCells;

  for (size_t i = 0; i < to_delete; ++i)
  {
    csTerrainCell* min_cell = loadedCells[i];    

    min_cell->SetLoadState (iTerrainCell::NotLoaded);
  }
}

void csTerrainSystem::AddCellLoadListener (iTerrainCellLoadCallback* cb)
{
  loadCallbacks.Push (cb);
}

void csTerrainSystem::RemoveCellLoadListener (iTerrainCellLoadCallback* cb)
{
  loadCallbacks.Delete (cb);
}

void csTerrainSystem::AddCellHeightUpdateListener (iTerrainCellHeightDataCallback* cb)
{
  heightDataCallbacks.Push (cb);
}

void csTerrainSystem::RemoveCellHeightUpdateListener (iTerrainCellHeightDataCallback* cb)
{
  heightDataCallbacks.Delete (cb);
}


iMeshObjectFactory* csTerrainSystem::GetFactory () const
{
  return factory;
}

csRenderMesh** csTerrainSystem::GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask)
{
  if (!renderer)
  {
    num = 0;
    return 0;
  }

  csArray<iTerrainCell*> neededCells;
  
  csOrthoTransform c2ot = rview->GetCamera ()->GetTransform ();
  c2ot /= movable->GetFullTransform ();
  
  csPlane3 planes[10];
  
  CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext (),
      c2ot, planes, frustum_mask);
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    if (!cells[i]->GetRenderProperties ()->GetVisible ()) 
      continue;

    uint32 out_mask;
    
    csBox3 box = cells[i]->GetBBox ();

    if (csIntersect3::BoxFrustum (box, planes, frustum_mask, out_mask))
    {
      if (cells[i]->GetLoadState () != csTerrainCell::Loaded)
      {
        cells[i]->SetLoadState (csTerrainCell::Loaded);
      }
      
      cells[i]->Touch ();

      neededCells.Push (cells[i]);
    }
  }
  
  if (autoPreload) 
    PreLoadCells (rview, movable);
  
  if (VisCallback) 
    VisCallback->BeforeDrawing (this, rview);
  
  csRenderMesh** allMeshes = renderer->GetRenderMeshes (num, rview, movable, 
    frustum_mask, neededCells);

  // Fill in some common info
  const csReversibleTransform& trO2W = movable->GetFullTransform ();
  bool isMirrored = rview->GetCamera ()->IsMirrored ();
  uint mixMode = GetMixMode ();

  for (int i = 0; i < num; ++i)
  {
    csRenderMesh* mesh = allMeshes[i];
    mesh->object2world = trO2W;
    mesh->do_mirror = isMirrored;
    mesh->mixmode = mixMode;
  }

  return allMeshes;
}

bool csTerrainSystem::HitBeamOutline (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr,
        iMaterialArray* materials)
{
  //@@TODO: See if this needs some touch-up
  csRef<iTerrainVector3Array> collArray;
  collArray.AttachNew (new scfArray<iTerrainVector3Array> );

  if (CollideSegment (start, end, true, collArray, materials))
  {
    isect = collArray->Get (0);

    if (pr)
    {
      int gr = 0;
      float gr_max = fabsf (end.x - start.x);

      if (fabsf (end.y - start.y) > gr_max)
      {
        gr = 1;
        gr_max = fabsf (end.y - start.y);
      }

      if (fabsf (end.z - start.z) > gr_max)
      {
        gr = 2;
        gr_max = fabsf (end.y - start.y);
      }

      *pr = fabsf (collArray->Get (0)[gr] - start[gr]) / gr_max;
    }

    return true;
  }

  return false;
}

bool csTerrainSystem::HitBeamOutline (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr)
{
  return HitBeamOutline(start, end, isect, pr, 0);
}

bool csTerrainSystem::HitBeamObject (const csVector3& start,
        const csVector3& end,
        csVector3& isect, float* pr, int* polygon_idx,
        iMaterialWrapper** material,
        iMaterialArray* materials)
{
  if (polygon_idx) *polygon_idx = -1;
  if (material) *material = 0;

  bool rc = HitBeamOutline (start, end, isect, pr, materials);
  if (material && materials->GetSize () >= 1)
    *material = materials->Get (0);
  return rc;
}

csColliderType csTerrainSystem::GetColliderType ()
{
  return CS_TERRAIN_COLLIDER;
}

void csTerrainSystem::ComputeBBox ()
{
  boundingbox.Empty ();
  boundingbox.StartBoundingBox ();
  bbStarted = true;

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    boundingbox += cells[i]->GetBBox ();
  }

  ShapeChanged ();
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
