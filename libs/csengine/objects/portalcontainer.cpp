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
#include "igeom/objmodel.h"
#include "csengine/portalcontainer.h"
#include "csgeom/transfrm.h"

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
}

csPortalContainer::~csPortalContainer ()
{
}

void csPortalContainer::Prepare ()
{
  if (prepared) return;
  prepared = true;
  data_nr++;
  csCompressVertex* vt = csVector3Array::CompressVertices (vertices);
  if (vt == 0) return;
  int i;
  for (i = 0 ; i < portals.Length () ; i++)
  {
    csPortal* prt = portals[i];
    int j;
    csArray<int>& vidx = prt->GetVertexIndices ();
    for (j = 0 ; j < vidx.Length () ; j++)
    {
      vidx[j] = vt[vidx[j]].new_idx;
    }
  }
  object_bbox.StartBoundingBox ();
  for (i = 0 ; i < vertices.Length () ; i++)
    object_bbox.AddBoundingVertex (vertices[i]);
  object_radius = object_bbox.Max () - object_bbox.GetCenter ();
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

//--------------------- For iMeshObject ------------------------------//

bool csPortalContainer::DrawTest (iRenderView* rview, iMovable* movable)
{
  Prepare ();
  (void)rview;
  (void)movable;
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

