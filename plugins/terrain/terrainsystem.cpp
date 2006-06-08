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
#include "csgeom/math3d.h"
#include "csqsqrt.h"

#include "iengine/rview.h"
#include "iengine/camera.h"

#include "terrainsystem.h"

#include "debug.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainSystem)

csTerrainSystem::csTerrainSystem (iBase* parent)
  : scfImplementationType (this, parent)
{
  vview_distance = 100;
  auto_preload = true;

  bbox.Set(-1000, -1000, -1000, 1000, 1000, 1000);
}

csTerrainSystem::~csTerrainSystem ()
{
}

void csTerrainSystem::SetRenderer(iTerrainRenderer* renderer)
{
  this->renderer = renderer;
}

iTerrainRenderer* csTerrainSystem::GetRenderer()
{
  return renderer;
}

void csTerrainSystem::SetCollider(iTerrainCollider* collider)
{
  this->collider = collider;
}

iTerrainCollider* csTerrainSystem::GetCollider()
{
  return collider;
}

void csTerrainSystem::AddCell(csTerrainCell* cell)
{
  cells.Push(cell);
}

iTerrainCell* csTerrainSystem::GetCell(const char* name)
{
  for (size_t i = 0; i < cells.GetSize(); ++i)
    if (!strcmp(cells[i]->GetName(), name))
      return cells[i];
  
  return NULL;
}

iTerrainCell* csTerrainSystem::GetCell(const csVector2& pos)
{
  for (size_t i = 0; i < cells.GetSize(); ++i)
  {
    const csVector2& cell_pos = cells[i]->GetPosition();
    const csVector2& cell_size = cells[i]->GetSize();

    if (cell_pos.x <= pos.x + EPSILON && cell_pos.x + cell_size.x >= pos.x - EPSILON &&
        cell_pos.y <= pos.y + EPSILON && cell_pos.y + cell_size.y >= pos.y - EPSILON)
      return cells[i];                
  }

  return NULL;
}

bool csTerrainSystem::CollideRay(const csVector3& start, const csVector3& end, bool oneHit, csArray<csVector3>& points)
{
  return false;
}

bool csTerrainSystem::CollideSegment(const csVector3& start, const csVector3& end, bool oneHit, csArray<csVector3>& points)
{
  return false;
}

float csTerrainSystem::GetVirtualViewDistance()
{
  return vview_distance;
}

void csTerrainSystem::SetVirtualViewDistance(float distance)
{
  vview_distance = distance;
}

bool csTerrainSystem::GetAutoPreLoad()
{
  return auto_preload;
}

void csTerrainSystem::SetAutoPreLoad(bool mode)
{
  auto_preload = mode;
}

void csTerrainSystem::PreLoadCells(iRenderView* view)
{
  csPlane3 planes[10];
  uint32 frustum_mask;
  
  /// Here I should not just mul by vview_distance, because it scales the frustum in N times, but does not make far plane distance equal to N
  /// (which is the desired result)
  /// Left for now.

#pragma message(PR_WARNING("vview_distance hack, frustum is enlarged too much"))
  
  csOrthoTransform transform = view->GetCamera()->GetTransform();
  transform *= csOrthoTransform(csMatrix3(1/vview_distance, 0, 0,
                                          0, 1/vview_distance, 0,
                                          0, 0, 1/vview_distance), csVector3(0, 0, 0));
  
  view->SetupClipPlanes(transform, planes, frustum_mask);
  
  for (size_t i = 0; i < cells.GetSize(); ++i)
  {
    int clip_portal, clip_plane, clip_zplane;
    
    csBox3 box = cells[i]->GetBBox();
    
    if (view->ClipBBox(planes, frustum_mask, box, clip_portal, clip_plane, clip_zplane) && cells[i]->GetLoadState() == csTerrainCell::NotLoaded)
      cells[i]->PreLoad();
  }
}

float csTerrainSystem::GetHeight(const csVector2& pos)
{
  iTerrainCell* cell = GetCell(pos);
  
  if (cell) return cell->GetHeight(pos - cell->GetPosition());
  else return 0;
}

csVector3 csTerrainSystem::GetTangent(const csVector2& pos)
{
  iTerrainCell* cell = GetCell(pos);
  
  if (cell) return cell->GetTangent(pos - cell->GetPosition());
  else return csVector3(0, 0, 0);
}

csVector3 csTerrainSystem::GetBinormal(const csVector2& pos)
{
  iTerrainCell* cell = GetCell(pos);
  
  if (cell) return cell->GetBinormal(pos - cell->GetPosition());
  else return csVector3(0, 0, 0);
}

csVector3 csTerrainSystem::GetNormal(const csVector2& pos)
{
  iTerrainCell* cell = GetCell(pos);
  
  if (cell) return cell->GetNormal(pos - cell->GetPosition());
  else return csVector3(0, 0, 0);
}

iMeshObjectFactory* csTerrainSystem::GetFactory () const
{
  return 0;
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
  needed_cells.SetSize(0);
  
  const csOrthoTransform& transform = rview->GetCamera()->GetTransform();
  
  csPlane3 planes[10];
  
  rview->SetupClipPlanes(transform, planes, frustum_mask);
  
  for (size_t i = 0; i < cells.GetSize(); ++i)
  {
    int clip_portal, clip_plane, clip_zplane;
    
    csBox3 box = cells[i]->GetBBox();
    
    if (rview->ClipBBox(planes, frustum_mask, box, clip_portal, clip_plane, clip_zplane))
    {
      if (cells[i]->GetLoadState() != csTerrainCell::Loaded) cells[i]->Load();
      
      needed_cells.Push(cells[i]);
    }
  }
  
  if (auto_preload) PreLoadCells(rview);
  
  return renderer->GetRenderMeshes(num, rview, movable, frustum_mask, needed_cells.GetArray(), (int)needed_cells.GetSize());
}

void csTerrainSystem::SetVisibleCallback (iMeshObjectDrawCallback* cb)
{
  imo_viscb = cb;
}

iMeshObjectDrawCallback* csTerrainSystem::GetVisibleCallback () const
{
  return imo_viscb;
}

void csTerrainSystem::NextFrame (csTicks current_time,const csVector3& pos,
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
  return false;
}

bool csTerrainSystem::HitBeamObject (const csVector3& start, const csVector3& end,
        csVector3& isect, float* pr, int* polygon_idx,
        iMaterialWrapper** material )
{
  return false;
}

void csTerrainSystem::SetMeshWrapper (iMeshWrapper* logparent)
{
  imo_wrapper = logparent;
}

iMeshWrapper* csTerrainSystem::GetMeshWrapper () const
{
  return imo_wrapper;
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

void csTerrainSystem::GetObjectBoundingBox(csBox3& box)
{
  box = bbox;
}

void csTerrainSystem::SetObjectBoundingBox(const csBox3& box)
{
  bbox = box;
  ShapeChanged ();
}

void csTerrainSystem::GetRadius(float& radius, csVector3& center)
{
  csBox3 bbox;
  GetObjectBoundingBox (bbox);
  center = bbox.GetCenter ();
  radius = csQsqrt (csSquaredDist::PointPoint (bbox.Max (), bbox.Min ()));
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
