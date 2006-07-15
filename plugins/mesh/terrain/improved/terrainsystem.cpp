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

#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/transfrm.h"
#include "csgeom/box.h"
#include "csgeom/sphere.h"
#include "csgeom/math3d.h"
#include "csqsqrt.h"

#include "iengine/rview.h"
#include "iengine/camera.h"

#include "iengine/movable.h"

#include "terrainsystem.h"

#include "debug.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

csTerrainSystem::csTerrainSystem (iMeshObjectFactory* factory)
  : scfImplementationType (this)
{
  this->factory = factory;

  vview_distance = 2;
  auto_preload = true;

  bbox_valid = false;
}

csTerrainSystem::~csTerrainSystem ()
{
}

void csTerrainSystem::SetRenderer (iTerrainRenderer* renderer)
{
  this->renderer = renderer;
}

void csTerrainSystem::SetCollider (iTerrainCollider* collider)
{
  this->collider = collider;
}

void csTerrainSystem::AddCell (csTerrainCell* cell)
{
  cells.Push (cell);
}

iTerrainCell* csTerrainSystem::GetCell (const char* name)
{
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    if (!strcmp (cells[i]->GetName (), name))
    {
      return cells[i];
    }
  }

  return NULL;
}

iTerrainCell* csTerrainSystem::GetCell (const csVector2& pos)
{
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    const csVector2& cell_pos = cells[i]->GetPosition ();
    const csVector3& cell_size = cells[i]->GetSize ();

    if (cell_pos.x <= pos.x + EPSILON &&
        cell_pos.x + cell_size.x >= pos.x - EPSILON &&
        cell_pos.y <= pos.y + EPSILON &&
        cell_pos.y + cell_size.y >= pos.y - EPSILON)
    {
      return cells[i];
    }
  }

  return NULL;
}

const csRefArray<iMaterialWrapper>& csTerrainSystem::GetMaterialPalette ()
                                                                       const
{
  return material_palette;
}

void csTerrainSystem::SetMaterialPalette (const csRefArray<iMaterialWrapper>&
                                          array)
{
  material_palette = array;

  renderer->OnMaterialPaletteUpdate (material_palette);
}

bool csTerrainSystem::CollideSegment (const csVector3& start, const csVector3&
                               end, bool oneHit, iTerrainVector3Array& points)
{
  if (!collider) return false;

  size_t size = points.GetSize ();

  for (size_t i = 0; i < cells.GetSize(); ++i)
  {
    csSegment3 seg(start, end);
    csBox3 box = cells[i]->GetBBox ();

    csVector3 isect;

    if (csIntersect3::BoxSegment (box, seg, isect) >= 0)
    {
      seg.SetStart( seg.End ());
      seg.SetEnd (isect);
      
      if (csIntersect3::BoxSegment (box, seg, isect) >= 0)
        seg.SetStart (isect);
      
      if (cells[i]->GetLoadState () != csTerrainCell::Loaded)
      {
        cells[i]->Load ();
      }

      if (cells[i]->CollideSegment (seg.End (), seg.Start (), oneHit, points)
          && oneHit)
        return true;
    }
  }

  return size != points.GetSize ();
}

bool csTerrainSystem::CollideTriangles (const csVector3* vertices,
                       unsigned int tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs)
{
  if (!collider) return false;

  size_t size = pairs.GetSize ();

  csSphere sphere (csVector3 (0, 0, 0), radius);
  
  sphere = trans->This2Other (sphere);
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    csBox3 box = cells[i]->GetBBox ();

    if (csIntersect3::BoxSphere (box, sphere.GetCenter (), sphere.GetRadius ()))
    {
      if (cells[i]->GetLoadState () != csTerrainCell::Loaded)
      {
        cells[i]->Load ();
      }

      if (cells[i]->CollideTriangles (vertices, tri_count, indices, radius,
          trans, oneHit, pairs) && oneHit)
        return true;
    }
  }

  return size != pairs.GetSize ();
}

bool csTerrainSystem::Collide (iCollider* collider,
                       float radius, const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs)
{
  if (!this->collider) return false;

  size_t size = pairs.GetSize ();

  csSphere sphere (csVector3 (0, 0, 0), radius);
  
  sphere = trans->This2Other (sphere);
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    csBox3 box = cells[i]->GetBBox ();

    if (csIntersect3::BoxSphere (box, sphere.GetCenter (), sphere.GetRadius ()))
    {
      if (cells[i]->GetLoadState () != csTerrainCell::Loaded)
      {
        cells[i]->Load ();
      }

      if (cells[i]->Collide (collider, radius, trans, oneHit, pairs) &&
        oneHit)
        return true;
    }
  }

  return size != pairs.GetSize ();
}

float csTerrainSystem::GetVirtualViewDistance () const
{
  return vview_distance;
}

void csTerrainSystem::SetVirtualViewDistance (float distance)
{
  vview_distance = distance;
}

bool csTerrainSystem::GetAutoPreLoad () const
{
  return auto_preload;
}

void csTerrainSystem::SetAutoPreLoad (bool mode)
{
  auto_preload = mode;
}

void csTerrainSystem::PreLoadCells (iRenderView* rview, iMovable* movable)
{
  csPlane3 planes[10];
  uint32 frustum_mask;
  
  csOrthoTransform c2ot = rview->GetCamera ()->GetTransform ();
  c2ot /= movable->GetFullTransform ();
  
  rview->SetupClipPlanes (c2ot, planes, frustum_mask);
  
  /// Here I should not just multiply by vview_distance, because it scales the frustum in N times, and N has nothing to do with distance :)
  /// Left for now, because, well, I'm not sure of the desired behavior

#pragma message(PR_WARNING("vview_distance hack, frustum is enlarged too much"))

  for (int pi = 0; pi < 10; ++pi)
  {
    planes[pi].DD *= vview_distance;
  }
  
  for (size_t i = 0; i < cells.GetSize(); ++i)
  {
    uint32 out_mask;
    
    csBox3 box = cells[i]->GetBBox ();
    
    if (csIntersect3::BoxFrustum (box, planes, frustum_mask, out_mask) &&
        cells[i]->GetLoadState () == csTerrainCell::NotLoaded)
    {
      cells[i]->PreLoad ();
    }
  }
}

float csTerrainSystem::GetHeight (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) return cell->GetHeight (pos - cell->GetPosition ());
  else return 0;
}

csVector3 csTerrainSystem::GetTangent (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) return cell->GetTangent (pos - cell->GetPosition ());
  else return csVector3(0, 0, 0);
}

csVector3 csTerrainSystem::GetBinormal (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) return cell->GetBinormal (pos - cell->GetPosition ());
  else return csVector3(0, 0, 0);
}

csVector3 csTerrainSystem::GetNormal (const csVector2& pos)
{
  iTerrainCell* cell = GetCell (pos);
  
  if (cell) return cell->GetNormal (pos - cell->GetPosition ());
  else return csVector3(0, 0, 0);
}

iMeshObjectFactory* csTerrainSystem::GetFactory () const
{
  return factory;
}

csFlags& csTerrainSystem::GetFlags ()
{
  return imo_flags;
}

csPtr<iMeshObject> csTerrainSystem::Clone ()
{
  return 0;
}

csRenderMesh** csTerrainSystem::GetRenderMeshes (int& num, iRenderView* rview, 
    iMovable* movable, uint32 frustum_mask)
{
  needed_cells.SetSize (0);
  
  csOrthoTransform c2ot = rview->GetCamera ()->GetTransform ();
  c2ot /= movable->GetFullTransform ();
  
  csPlane3 planes[10];
  
  rview->SetupClipPlanes (c2ot, planes, frustum_mask);
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    uint32 out_mask;
    
    csBox3 box = cells[i]->GetBBox ();

    if (csIntersect3::BoxFrustum (box, planes, frustum_mask, out_mask))
    {
      if (cells[i]->GetLoadState () != csTerrainCell::Loaded)
      {
        cells[i]->Load ();
      }
      
      needed_cells.Push (cells[i]);
    }
  }
  
  if (auto_preload) PreLoadCells (rview, movable);
  
  if (imo_viscb) imo_viscb->BeforeDrawing (this, rview);
  
  return renderer->GetRenderMeshes (num, rview, movable, frustum_mask,
           needed_cells.GetArray (), (int)needed_cells.GetSize ());
}

void csTerrainSystem::SetVisibleCallback (iMeshObjectDrawCallback* cb)
{
  imo_viscb = cb;
}

iMeshObjectDrawCallback* csTerrainSystem::GetVisibleCallback () const
{
  return imo_viscb;
}

void csTerrainSystem::NextFrame (csTicks current_time, const csVector3& pos,
    uint currentFrame)
{
}

void csTerrainSystem::HardTransform (const csReversibleTransform& t)
{
}

bool csTerrainSystem::SupportsHardTransform () const
{
  return false;
}

bool csTerrainSystem::HitBeamOutline (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr)
{
  collision_result.SetSize (0);

  if (CollideSegment (start, end, true, collision_result))
  {
    isect = collision_result.Get (0);

    if (pr)
    {
#pragma message(PR_WARNING("hrm... this is ugly."))
      int gr = 0;
      float gr_max = fabsf(end.x - start.x);

      if (fabsf(end.y - start.y) > gr_max)
      {
        gr = 1;
        gr_max = fabsf(end.y - start.y);
      }

      if (fabsf(end.z - start.z) > gr_max)
      {
        gr = 2;
        gr_max = fabsf(end.y - start.y);
      }

      *pr = fabsf(collision_result.Get (0)[gr] - start[gr]) / gr_max;
    }

    return true;
  }

  return false;
}

bool csTerrainSystem::HitBeamObject (const csVector3& start,
        const csVector3& end,
        csVector3& isect, float* pr, int* polygon_idx,
        iMaterialWrapper** material )
{
  if (polygon_idx) *polygon_idx = -1;
  if (material) *material = NULL;

  return HitBeamOutline (start, end, isect, pr);
}

void csTerrainSystem::SetMeshWrapper (iMeshWrapper* lp)
{
  logparent = lp;
}

iMeshWrapper* csTerrainSystem::GetMeshWrapper () const
{
  return logparent;
}

iObjectModel* csTerrainSystem::GetObjectModel ()
{
  return this;
}

bool csTerrainSystem::SetColor (const csColor& color)
{
  return false;
}

bool csTerrainSystem::GetColor (csColor& color) const
{
  return false;
}

bool csTerrainSystem::SetMaterialWrapper (iMaterialWrapper* material)
{
  return false;
}

iMaterialWrapper* csTerrainSystem::GetMaterialWrapper () const
{
  return 0;
}

void csTerrainSystem::SetMixMode (uint mode)
{
}

uint csTerrainSystem::GetMixMode () const
{
  return 0;
}

void csTerrainSystem::InvalidateMaterialHandles ()
{
}

void csTerrainSystem::PositionChild (iMeshObject* child,csTicks current_time)
{
}

void csTerrainSystem::ComputeBBox ()
{
  bbox.StartBoundingBox ();
  
  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    bbox = bbox + cells[i]->GetBBox ();
  }
  
  bbox_valid = true;
}

void csTerrainSystem::GetObjectBoundingBox (csBox3& box)
{
  if ( !bbox_valid ) ComputeBBox ();
  box = bbox;
}

void csTerrainSystem::SetObjectBoundingBox (const csBox3& box)
{
  bbox = box;
  ShapeChanged ();
}

void csTerrainSystem::GetRadius (float& radius, csVector3& center)
{
  csBox3 box;
  GetObjectBoundingBox (box);
  center = box.GetCenter ();
  radius = csQsqrt (csSquaredDist::PointPoint (box.Max (), box.Min ()));
}

csColliderType csTerrainSystem::GetColliderType ()
{
  return CS_TERRAIN_COLLIDER;
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
