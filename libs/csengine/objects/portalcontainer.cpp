/*
    Copyright (C) 2003 by Jorrit Tyberghein

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
#include "qsqrt.h"
#include "csgeom/sphere.h"
#include "csgeom/poly3d.h"
#include "csgeom/transfrm.h"
#include "igeom/objmodel.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "csengine/portalcontainer.h"

// ---------------------------------------------------------------------------
// csPortalContainerPolyMeshHelper
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csPortalContainerPolyMeshHelper)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

void csPortalContainerPolyMeshHelper::SetPortalContainer (csPortalContainer* pc)
{
  parent = pc;
  data_nr = pc->GetDataNumber ()-1;
}

void csPortalContainerPolyMeshHelper::Setup ()
{
  parent->Prepare ();
  if (data_nr != parent->GetDataNumber () || !vertices)
  {
    data_nr = parent->GetDataNumber ();
    Cleanup ();

    vertices = parent->GetVertices ();
    // Count number of needed polygons.
    num_poly = 0;

    int i;
    const csRefArray<csPortal>& portals = parent->GetPortals ();
    for (i = 0 ; i < portals.Length () ; i++)
    {
      csPortal *p = portals[i];
      if (p->flags.CheckAll (poly_flag)) num_poly++;
    }

    if (num_poly)
    {
      polygons = new csMeshedPolygon[num_poly];
      num_poly = 0;
      for (i = 0 ; i < portals.Length () ; i++)
      {
        csPortal *p = portals[i];
        if (p->flags.CheckAll (poly_flag))
        {
	  csDirtyAccessArray<int>& vidx = p->GetVertexIndices ();
          polygons[num_poly].num_vertices = vidx.Length ();
          polygons[num_poly].vertices = vidx.GetArray ();
          num_poly++;
        }
      }
    }
  }
}

void csPortalContainerPolyMeshHelper::Cleanup ()
{
  delete[] polygons;
  polygons = 0;
  vertices = 0;
  delete[] triangles;
  triangles = 0;
}

// ---------------------------------------------------------------------------
// csPortalContainer
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT(csPortalContainer)
  SCF_IMPLEMENTS_INTERFACE (iPortalContainer)
SCF_IMPLEMENT_IBASE_EXT_END

csPortalContainer::csPortalContainer (iEngine* engine) :
	csMeshObject (engine),
	scfiPolygonMesh (0),
	scfiPolygonMeshCD (CS_PORTAL_COLLDET),
	scfiPolygonMeshLOD (CS_PORTAL_VISCULL)
{
  prepared = false;
  data_nr = 0;
  scfiPolygonMesh.SetPortalContainer (this);
  scfiPolygonMeshCD.SetPortalContainer (this);
  scfiPolygonMeshLOD.SetPortalContainer (this);
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMeshCD);
  scfiObjectModel.SetPolygonMeshViscull (&scfiPolygonMeshLOD);
  scfiObjectModel.SetPolygonMeshShadows (&scfiPolygonMeshLOD);

  movable_nr = -1;
}

csPortalContainer::~csPortalContainer ()
{
}

void csPortalContainer::Prepare ()
{
  if (prepared) return;
  prepared = true;
  movable_nr = -1; // Make sure move stuff gets updated.
  data_nr++;
  csCompressVertex* vt = csVector3Array::CompressVertices (vertices);
  if (vt == 0) return;
  int i;
  planes.DeleteAll ();
  for (i = 0 ; i < portals.Length () ; i++)
  {
    csPortal* prt = portals[i];
    int j;
    csArray<int>& vidx = prt->GetVertexIndices ();
    csPoly3D poly;
    for (j = 0 ; j < vidx.Length () ; j++)
    {
      vidx[j] = vt[vidx[j]].new_idx;
      poly.AddVertex (vertices[vidx[j]]);
    }
    planes.Push (poly.ComputePlane ());
  }
  object_bbox.StartBoundingBox ();
  for (i = 0 ; i < vertices.Length () ; i++)
    object_bbox.AddBoundingVertex (vertices[i]);
  object_radius = object_bbox.Max () - object_bbox.GetCenter ();
  max_object_radius = qsqrt (csSquaredDist::PointPoint (
  	object_bbox.Max (), object_bbox.Min ())) * 0.5f;
}

//------------------- For iPortalContainer ---------------------------//

iPortal* csPortalContainer::CreatePortal (csVector3* vertices, int num)
{
  prepared = false;
  csPortal* prt = new csPortal (this);
  portals.Push (prt);

  int i;
  for (i = 0 ; i < num ; i++)
  {
    int idx = csPortalContainer::vertices.Push (vertices[i]);
    prt->AddVertexIndex (idx);
  }

  prt->DecRef ();
  return prt;
}

void csPortalContainer::RemovePortal (iPortal* portal)
{
  prepared = false;
  portals.Delete ((csPortal*)portal);
}

//------------------------- General ----------------------------------//

void csPortalContainer::ObjectToWorld (iMovable* movable,
	const csReversibleTransform& movtrans)
{
  if (movable_nr == movable->GetUpdateNumber ()) return;
  movable_nr = movable->GetUpdateNumber ();
  int i;
  world_vertices.SetLength (vertices.Length ());
  if (movable->IsFullTransformIdentity ())
  {
    world_vertices = vertices;
    world_planes = planes;
  }
  else
  {
    for (i = 0 ; i < vertices.Length () ; i++)
      world_vertices[i] = movtrans.This2Other (vertices[i]);
    world_planes.DeleteAll ();
    for (i = 0 ; i < planes.Length () ; i++)
    {
      csPlane3 p;
      csVector3& world_vec = world_vertices[portals[i]->GetVertexIndices ()[0]];
      movtrans.This2Other (planes[i], world_vec, p);
      p.Normalize ();
      world_planes.Push (p);
    }
  }
}

//--------------------- For iMeshObject ------------------------------//

bool csPortalContainer::DrawTest (iRenderView* rview, iMovable* movable)
{
  Prepare ();

  iCamera *icam = rview->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();
  const csReversibleTransform& movtrans = movable->GetFullTransform ();

  csSphere sphere;
  sphere.SetCenter (object_bbox.GetCenter ());
  sphere.SetRadius (max_object_radius);
  csReversibleTransform tr_o2c = camtrans;
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movtrans;
  if (!rview->ClipBSphere (tr_o2c, sphere, clip_portal,
  	clip_plane, clip_z_plane))
    return false;

  ObjectToWorld (movable, movtrans);

  return false;
}

bool csPortalContainer::Draw (iRenderView* rview, iMovable* movable,
  	csZBufMode zbufMode)
{
  Prepare ();
  (void)rview;
  (void)movable;
  (void)zbufMode;
  return false;
}

void csPortalContainer::HardTransform (const csReversibleTransform& t)
{
  Prepare ();
  (void)t;
}

bool csPortalContainer::HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr)
{
  Prepare ();
  (void)start;
  (void)end;
  (void)isect;
  (void)pr;
  return false;
}

bool csPortalContainer::HitBeamObject (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr)
{
  Prepare ();
  (void)start;
  (void)end;
  (void)isect;
  (void)pr;
  return false;
}

void csPortalContainer::GetRadius (csVector3& radius, csVector3& center)
{
  Prepare ();
  center = object_bbox.GetCenter ();
  radius = object_radius;
}

