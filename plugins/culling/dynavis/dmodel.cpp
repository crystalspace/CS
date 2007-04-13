/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csutil/sysfunc.h"
#include "csutil/scfstr.h"
#include "iutil/string.h"
#include "csqint.h"
#include "csqsqrt.h"
#include "csgeom/math3d.h"
#include "csgeom/pmtools.h"
#include "csgeom/trimeshtools.h"
#include "imesh/objmodel.h"
#include "igeom/polymesh.h"
#include "iengine/mesh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/strset.h"
#include "dmodel.h"

//---------------------------------------------------------------------------

csDynavisObjectModel::csDynavisObjectModel ()
{
  planes = 0;
  num_planes = ~0;
  edges = 0;
  tri_edges = 0;
  num_edges = ~0;
  dirty_obb = true;
  has_obb = false;
  imodel = 0;
  use_outline_filler = false;
  empty_object = true;
  single_polygon = false;
  use_trianglemesh = true;
  trianglemesh = 0;
}

csDynavisObjectModel::~csDynavisObjectModel ()
{
  delete[] planes;
  delete[] edges;
  delete[] tri_edges;
  if (use_trianglemesh)
  {
    if (trianglemesh)
      trianglemesh->Unlock ();
  }
  else
  {
    if (imodel && imodel->GetPolygonMeshViscull ())
      imodel->GetPolygonMeshViscull ()->Unlock ();
  }
}

bool csDynavisObjectModel::HasVisCullMesh (iObjectModel* obj_model)
{
  if (use_trianglemesh)
    return trianglemesh != 0;
  return obj_model->GetPolygonMeshViscull () != 0;
}

void csDynavisObjectModel::UpdateOutline (const csVector3& pos)
{
  if (num_edges <= 0) return;

  if (use_trianglemesh)
  {
    if (!trianglemesh) return;
  }
  else
  {
    if (!imodel->GetPolygonMeshViscull ()) return;
  }

  size_t num_vertices;
  if (use_trianglemesh)
    num_vertices = trianglemesh->GetVertexCount ();
  else
    num_vertices = imodel->GetPolygonMeshViscull ()->GetVertexCount ();

  bool recalc_outline = false;
  if (!outline_info.outline_edges)
  {
    // @@@ Only allocate active edges.
    outline_info.outline_edges = new size_t [num_edges*2];
    outline_info.outline_verts = new bool [num_vertices];
    recalc_outline = true;
  }
  else
  {
    float sqdist = csSquaredDist::PointPoint (pos, outline_info.outline_pos);
    if (sqdist > outline_info.valid_radius * outline_info.valid_radius)
      recalc_outline = true;
  }

  if (recalc_outline)
  {
    if (use_trianglemesh)
    {
      csTriangleMeshTools::CalculateOutline (tri_edges, num_edges,
    	  planes, num_vertices, pos,
	  outline_info.outline_edges, outline_info.num_outline_edges,
	  outline_info.outline_verts,
	  outline_info.valid_radius);
    }
    else
    {
      int noe;
      csPolygonMeshTools::CalculateOutline (edges, (int)num_edges,
    	  planes, (int)num_vertices, pos,
	  (int*)outline_info.outline_edges, noe,
	  outline_info.outline_verts,
	  outline_info.valid_radius);
      outline_info.num_outline_edges = noe;
    }
    outline_info.outline_pos = pos;
  }
}

bool csDynavisObjectModel::HasOBB ()
{
  GetOBB ();
  return has_obb;
}

const csOBB& csDynavisObjectModel::GetOBB ()
{
  if (dirty_obb)
  {
    dirty_obb = false;
    has_obb = false;
    if (use_trianglemesh)
    {
      if (trianglemesh)
      {
        size_t num_vertices = trianglemesh->GetVertexCount ();
        csVector3* verts = trianglemesh->GetVertices ();
        obb.FindOBB (verts, num_vertices);
        has_obb = true;
      }
    }
    else if (imodel->GetPolygonMeshViscull ())
    {
      int num_vertices = imodel->GetPolygonMeshViscull ()->GetVertexCount ();
      csVector3* verts = imodel->GetPolygonMeshViscull ()->GetVertices ();
      obb.FindOBB (verts, num_vertices);
      has_obb = true;
    }
  }
  return obb;
}

//---------------------------------------------------------------------------

csObjectModelManager::csObjectModelManager () : models (193)
{
}

csObjectModelManager::~csObjectModelManager ()
{
  ModelHash::GlobalIterator it = models.GetIterator();
  while (it.HasNext ())
  {
    csDynavisObjectModel* model = it.Next ();
    delete model;
  }
}

void csObjectModelManager::Initialize (iObjectRegistry* object_reg)
{
  csRef<iStringSet> strset = csQueryRegistryTagInterface<iStringSet> (
      object_reg, "crystalspace.shared.stringset");
  base_id = strset->Request ("base");
  viscull_id = strset->Request ("viscull");
}


csDynavisObjectModel* csObjectModelManager::CreateObjectModel (
	iObjectModel* imodel)
{
  csDynavisObjectModel* model = models.Get (imodel, 0);
  if (model)
  {
    model->ref_cnt++;
  }
  else
  {
    model = new csDynavisObjectModel ();
    model->ref_cnt = 1;
    model->imodel = imodel;
    // To make sure we will recalc we set shape_number to one less.
    model->shape_number = imodel->GetShapeNumber ()-1;
    model->use_trianglemesh = imodel->IsTriangleDataSet (base_id);
    if (model->use_trianglemesh)
    {
      if (imodel->IsTriangleDataSet (viscull_id))
        model->trianglemesh = imodel->GetTriangleData (viscull_id);
      else
        model->trianglemesh = imodel->GetTriangleData (base_id);
      if (model->trianglemesh)
	model->trianglemesh->Lock ();
    }
    else
    {
      if (imodel->GetPolygonMeshViscull ())
        imodel->GetPolygonMeshViscull ()->Lock ();
    }
  }
  return model;
}

void csObjectModelManager::ReleaseObjectModel (csDynavisObjectModel* model)
{
  CS_ASSERT (model->ref_cnt > 0);
  if (model->ref_cnt == 1)
  {
    // We are about to delete the model.
    models.DeleteAll (model->imodel);
    delete model;
    return;
  }
  model->ref_cnt--;
}

static int show_notclosed = 6;

bool csObjectModelManager::CheckObjectModel (csDynavisObjectModel* model,
	iMeshWrapper* mw)
{
  CS_ASSERT (model->ref_cnt > 0);
  if (model->imodel->GetShapeNumber () != model->shape_number)
  {
    model->shape_number = model->imodel->GetShapeNumber ();
    model->use_outline_filler = true;
    model->outline_info.Clear ();
    model->dirty_obb = true;
    iPolygonMesh* mesh = model->imodel->GetPolygonMeshViscull ();
    iTriangleMesh* trimesh = model->trianglemesh;
    if (model->use_trianglemesh && trimesh)
    {
      if (trimesh->GetTriangleCount () == 0)
      {
        model->empty_object = true;
	model->use_outline_filler = false;
	return false;
      }

      model->empty_object = false;

      if (model->num_planes != trimesh->GetTriangleCount ())
      {
        delete[] model->planes;
        model->num_planes = trimesh->GetTriangleCount ();
	if (model->num_planes)
          model->planes = new csPlane3 [model->num_planes];
        else
          model->planes = 0;
      }
      csTriangleMeshTools::CalculatePlanes (trimesh, model->planes);
      delete[] model->tri_edges;
      model->tri_edges = csTriangleMeshTools::CalculateEdges (
      	trimesh, model->num_edges);

      // If the mesh is empty then it is possible that num_edges will be ~0.
      // The code below will correctly handle that.
      csTriangleMeshTools::CheckActiveEdges (model->tri_edges, model->num_edges,
      	model->planes);

      // If we have less then some number of polygons then we don't
      // use outline culling because it is not worth the effort (and
      // lack of accuracy).
      if (trimesh->GetTriangleCount () < 16)
      {
        model->use_outline_filler = false;
      }

      // Here we scan all edges and see if there are edges that have only
      // one adjacent polygon. If we find such an edge then we will not use
      // outline based culling for this object. This is not good as it will
      // slow down culling so you should try to avoid this situation in levels.
      if (model->use_outline_filler)
      {
        size_t i;
        for (i = 0 ; i < model->num_edges ; i++)
          if (model->tri_edges[i].tri2 == -1)
	  {
	    model->use_outline_filler = false;
	    if (show_notclosed > 0)
	    {
	      csPrintf ("WARNING! Object '%s' is not closed!\n", mw != 0 ?
	  	  mw->QueryObject ()->GetName () : "<no mesh>");
	      fflush (stdout);
	      show_notclosed--;
	    }
	    else if (show_notclosed == 0)
	    {
	      csPrintf ("...\n");
	      fflush (stdout);
	      show_notclosed--;
	    }
	    break;
	  }
      }
      model->single_polygon = trimesh->GetTriangleCount () == 1;
    }
    else if (!model->use_trianglemesh && mesh)
    {
      if (mesh->GetPolygonCount () == 0)
      {
        model->empty_object = true;
	model->use_outline_filler = false;
	return false;
      }

      model->empty_object = false;

      if (model->num_planes != (size_t)mesh->GetPolygonCount ())
      {
        delete[] model->planes;
        model->num_planes = mesh->GetPolygonCount ();
	if (model->num_planes)
          model->planes = new csPlane3 [model->num_planes];
        else
          model->planes = 0;
      }
      csPolygonMeshTools::CalculatePlanes (mesh, model->planes);
      delete[] model->edges;
      int ne;
      model->edges = csPolygonMeshTools::CalculateEdges (
      	mesh, ne);
      model->num_edges = ne;

      // If the mesh is empty then it is possible that num_edges will be ~0.
      // The code below will correctly handle that.
      csPolygonMeshTools::CheckActiveEdges (model->edges, model->num_edges,
      	model->planes);

      // If we have less then some number of polygons then we don't
      // use outline culling because it is not worth the effort (and
      // lack of accuracy).
      if (mesh->GetPolygonCount () < 10)
      {
        model->use_outline_filler = false;
      }

      // Here we scan all edges and see if there are edges that have only
      // one adjacent polygon. If we find such an edge then we will not use
      // outline based culling for this object. This is not good as it will
      // slow down culling so you should try to avoid this situation in levels.
      if (model->use_outline_filler)
      {
        size_t i;
        for (i = 0 ; i < model->num_edges ; i++)
          if (model->edges[i].poly2 == -1)
	  {
	    model->use_outline_filler = false;
	    if (show_notclosed > 0)
	    {
	      csPrintf ("WARNING! Object '%s' is not closed!\n", mw != 0 ?
	  	  mw->QueryObject ()->GetName () : "<no mesh>");
	      fflush (stdout);
	      show_notclosed--;
	    }
	    else if (show_notclosed == 0)
	    {
	      csPrintf ("...\n");
	      fflush (stdout);
	      show_notclosed--;
	    }
	    break;
	  }
      }
      model->single_polygon = mesh->GetPolygonCount () == 1;
    }
    else
    {
      model->empty_object = true;
      model->single_polygon = false;
    }
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------

