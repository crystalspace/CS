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

#include <string.h>
#include "cssysdef.h"
#include "csver.h"
#include "cssys/sysfunc.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/scfstr.h"
#include "csgeom/matrix3.h"
#include "csgeom/math3d.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"
#include "iutil/objreg.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "iutil/object.h"
#include "ivaria/reporter.h"
#include "dynavis.h"
#include "kdtree.h"
#include "covbuf.h"
#include "wqueue.h"
//#include "exvis.h" // @@@ to be uncommented when file added to cvs.

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csDynaVis)

SCF_EXPORT_CLASS_TABLE (dynavis)
  SCF_EXPORT_CLASS (csDynaVis, "crystalspace.culling.dynavis",
    "Dynamic Visibility System")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csDynaVis)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityCuller)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csDynaVis::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csDynaVis::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csDynaVis::csDynaVis (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  object_reg = NULL;
  kdtree = NULL;
  covbuf = NULL;
  debug_camera = NULL;
  model_mgr = new csObjectModelManager ();
  write_queue = new csWriteQueue ();

  stats_cnt_vistest = 0;
  stats_total_vistest_time = 0;
  stats_total_notvistest_time = 0;

  do_cull_frustum = true;
  do_cull_coverage = COVERAGE_OUTLINE;
  do_cull_history = true;
  do_cull_writequeue = true;

  cfg_view_mode = VIEWMODE_STATS;
  do_state_dump = false;
  debug_origin_z = 50;
}

csDynaVis::~csDynaVis ()
{
  int i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
    	visobj_vector[i];
    visobj_wrap->visobj->DecRef ();
    delete visobj_wrap;
  }
  delete kdtree;
  delete covbuf;
  delete model_mgr;
  delete write_queue;
}

bool csDynaVis::Initialize (iObjectRegistry *object_reg)
{
  csDynaVis::object_reg = object_reg;

  delete kdtree;
  delete covbuf;

  iGraphics3D* g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d)
  {
    scr_width = g3d->GetWidth ();
    scr_height = g3d->GetHeight ();
    g3d->DecRef ();
  }
  else
  {
    // If there is no g3d we currently assume we are testing.
    scr_width = 640;
    scr_height = 480;
  }

  kdtree = new csKDTree (NULL);
  covbuf = new csCoverageBuffer (scr_width, scr_height);

  return true;
}

void csDynaVis::Setup (const char* /*name*/)
{
}

void csDynaVis::CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox)
{
  iMovable* movable = visobj->GetMovable ();
  csBox3 box;
  visobj->GetObjectModel ()->GetObjectBoundingBox (box, CS_BBOX_MAX);
  csReversibleTransform trans = movable->GetFullTransform ();
  bbox.StartBoundingBox (trans.This2Other (box.GetCorner (0)));
  bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (1)));
  bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (2)));
  bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (3)));
  bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (4)));
  bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (5)));
  bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (6)));
  bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (7)));
}

void csDynaVis::RegisterVisObject (iVisibilityObject* visobj)
{
  csVisibilityObjectWrapper* visobj_wrap = new csVisibilityObjectWrapper ();
  visobj_wrap->visobj = visobj;
  visobj->IncRef ();
  iMovable* movable = visobj->GetMovable ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();

  visobj_wrap->model = model_mgr->CreateObjectModel (visobj->GetObjectModel ());
  visobj_wrap->shape_number = visobj_wrap->model->GetShapeNumber ();
  model_mgr->CheckObjectModel (visobj_wrap->model);

  csBox3 bbox;
  CalculateVisObjBBox (visobj, bbox);
  visobj_wrap->child = kdtree->AddObject (bbox, (void*)visobj_wrap);

  visobj_vector.Push (visobj_wrap);
}

void csDynaVis::UnregisterVisObject (iVisibilityObject* visobj)
{
  int i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
      visobj_vector[i];
    if (visobj_wrap->visobj == visobj)
    {
      model_mgr->ReleaseObjectModel (visobj_wrap->model);
      kdtree->RemoveObject (visobj_wrap->child);
      visobj->DecRef ();
      delete visobj_wrap;
      visobj_vector.Delete (i);
      return;
    }
  }
}

void csDynaVis::UpdateObjects ()
{
  int i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
      visobj_vector[i];
    iVisibilityObject* visobj = visobj_wrap->visobj;
    iMovable* movable = visobj->GetMovable ();
    model_mgr->CheckObjectModel (visobj_wrap->model);
    if (visobj_wrap->model->GetShapeNumber () != visobj_wrap->shape_number ||
    	movable->GetUpdateNumber () != visobj_wrap->update_number)
    {
      csBox3 bbox;
      CalculateVisObjBBox (visobj, bbox);
      kdtree->MoveObject (visobj_wrap->child, bbox);
      visobj_wrap->shape_number = visobj_wrap->model->GetShapeNumber ();
      visobj_wrap->update_number = movable->GetUpdateNumber ();
    }
    visobj->MarkInvisible ();
    visobj_wrap->reason = INVISIBLE_PARENT;
  }
}

static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

static bool PrintObjects (csKDTree* treenode, void*, uint32)
{
  int num_objects;
  csKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
    	objects[i]->GetObject ();
    iObject* iobj = SCF_QUERY_INTERFACE (visobj_wrap->visobj, iObject);
    if (iobj)
    {
      char name[255];
      if (iobj->GetName ()) sprintf (name, "'%s'", iobj->GetName ());
      else strcpy (name, "<noname>");
      printf ("%s ", name);
      iobj->DecRef ();
    }
  }
  return true;
}

struct VisTest_Front2BackData
{
  csVector3 pos;
  iRenderView* rview;
  csDynaVis* dynavis;

  // During VisTest() we use the current frustum as five planes.
  // Associated with this frustum we also have a clip mask which
  // is maintained recursively during VisTest() and indicates the
  // planes that are still active for the current kd-tree node.
  csPlane3 frustum[32];
  uint32 frustum_mask;
};

bool csDynaVis::TestNodeVisibility (csKDTree* treenode,
	VisTest_Front2BackData* data)
{
  const csBox3& node_bbox = treenode->GetNodeBBox ();
  const csVector3& pos = data->pos;

  // For coverage testing.
  csBox2 sbox;
  float min_depth = 0;

  csVisReason reason = VISIBLE;
  bool vis = true;

  if (node_bbox.Contains (pos))
  {
    reason = VISIBLE_INSIDE;
    goto end;
  }

  if (do_cull_frustum)
  {
    uint32 new_mask;
    if (!csIntersect3::BoxFrustum (node_bbox, data->frustum, data->frustum_mask,
    	new_mask))
    {
      reason = INVISIBLE_FRUSTUM;
      vis = false;
      goto end;
    }
    // In VisTest_Front2Back() this is later restored when recursing back to
    // higher level.
    data->frustum_mask = new_mask;
  }

  if (do_cull_coverage != COVERAGE_NONE)
  {
    iCamera* camera = data->rview->GetCamera ();
    float max_depth;
    if (node_bbox.ProjectBox (camera->GetTransform (), camera->GetFOV (),
    	camera->GetShiftX (), camera->GetShiftY (), sbox, min_depth, max_depth))
      if (!covbuf->TestRectangle (sbox, min_depth))
      {
        reason = INVISIBLE_TESTRECT;
        vis = false;
        goto end;
      }
  }

end:
  if (do_state_dump)
  {
    printf ("Node (%g,%g,%g)-(%g,%g,%g) %s\n",
    	node_bbox.MinX (), node_bbox.MinY (), node_bbox.MinZ (),
    	node_bbox.MaxX (), node_bbox.MaxY (), node_bbox.MaxZ (),
	reason == INVISIBLE_FRUSTUM ? "outside frustum" :
	reason == INVISIBLE_TESTRECT ? "covered" :
	reason == VISIBLE_INSIDE ? "visible inside" :
	reason == VISIBLE ? "visible" :
	"?"
	);
    if (reason != INVISIBLE_FRUSTUM && reason != VISIBLE_INSIDE)
    {
      printf ("  (%g,%g)-(%g,%g) min_depth=%g\n",
      	sbox.MinX (), sbox.MinY (),
      	sbox.MaxX (), sbox.MaxY (), min_depth);
    }
    if (reason != VISIBLE && reason != VISIBLE_INSIDE)
    {
      printf ("  ");
      treenode->Front2Back (data->pos, PrintObjects, NULL);
      printf ("\n");
    }
  }

  return vis;
}

void csDynaVis::UpdateCoverageBuffer (iCamera* camera,
	iVisibilityObject* visobj, csObjectModel* model)
{
  iMovable* movable = visobj->GetMovable ();
  iPolygonMesh* polymesh = visobj->GetObjectModel ()->GetSmallerPolygonMesh ();

  const csVector3* verts = polymesh->GetVertices ();
  int vertex_count = polymesh->GetVertexCount ();
  int poly_count = polymesh->GetPolygonCount ();

  csReversibleTransform movtrans = movable->GetFullTransform ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  csReversibleTransform trans = camtrans / movtrans;
  float fov = camera->GetFOV ();
  float sx = camera->GetShiftX ();
  float sy = camera->GetShiftY ();

  // Calculate camera position in object space.
  csVector3 campos_object = movtrans.Other2This (camtrans.GetOrigin ());

  int i;
  // First transform all vertices.
  //@@@ Avoid allocate?
  csVector2* tr_verts = new csVector2[vertex_count];
  float* tr_z = new float[vertex_count];
  for (i = 0 ; i < vertex_count ; i++)
  {
    csVector3 camv = trans.Other2This (verts[i]);
    tr_z[i] = camv.z;
    if (camv.z > 0.0)
      Perspective (camv, tr_verts[i], fov, sx, sy);
  }

  if (do_state_dump)
  {
    iObject* iobj = SCF_QUERY_INTERFACE (visobj, iObject);
    if (iobj)
    {
      printf ("CovIns of object %s\n", iobj->GetName () ? iobj->GetName () :
      	"<noname>");
      iobj->DecRef ();
    }
  }

  // Then insert all polygons.
  csMeshedPolygon* poly = polymesh->GetPolygons ();
  const csPlane3* planes = model->GetPlanes ();
  csVector2 verts2d[64];
  for (i = 0 ; i < poly_count ; i++, poly++)
  {
    if (planes[i].Classify (campos_object) >= 0.0)
      continue;

    int num_verts = poly->num_vertices;
    int* vi = poly->vertices;
    float max_depth = -1.0;
    int j;
    for (j = 0 ; j < num_verts ; j++)
    {
      int vertex_idx = vi[j];
      float tz = tr_z[vertex_idx];
      if (tz <= 0.0)
      {
        // @@@ Later we should clamp instead of ignoring this polygon.
	max_depth = -1.0;
	break;
      }
      if (tz > max_depth) max_depth = tz;
      verts2d[j] = tr_verts[vertex_idx];
    }
    if (max_depth > 0.0)
    {
      bool rc = covbuf->InsertPolygon (verts2d, num_verts, max_depth);
      if (do_state_dump)
      {
        printf ("  max_depth=%g rc=%d ", max_depth, rc);
        for (j = 0 ; j < num_verts ; j++)
	  printf ("(%g,%g) ", verts2d[j].x, verts2d[j].y);
        printf ("\n");
      }
    }
  }

  if (do_state_dump)
  {
    iString* str = covbuf->Debug_Dump ();
    printf ("%s\n", str->GetData ());
    str->DecRef ();
  }

  delete[] tr_z;
  delete[] tr_verts;
}

void csDynaVis::UpdateCoverageBufferOutline (iCamera* camera,
	iVisibilityObject* visobj, csObjectModel* model)
{
  iMovable* movable = visobj->GetMovable ();
  iPolygonMesh* polymesh = visobj->GetObjectModel ()->GetSmallerPolygonMesh ();

  const csVector3* verts = polymesh->GetVertices ();
  int vertex_count = polymesh->GetVertexCount ();

  csReversibleTransform movtrans = movable->GetFullTransform ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  csReversibleTransform trans = camtrans / movtrans;
  float fov = camera->GetFOV ();
  float sx = camera->GetShiftX ();
  float sy = camera->GetShiftY ();

  // Calculate camera position in object space.
  csVector3 campos_object = movtrans.Other2This (camtrans.GetOrigin ());
  model->UpdateOutline (campos_object);
  const csOutlineInfo& outline_info = model->GetOutlineInfo ();

  int i;
  // First transform all vertices.
  //@@@ Avoid allocate?
  csVector2* tr_verts = new csVector2[vertex_count];
  float max_depth = -1.0;
  for (i = 0 ; i < vertex_count ; i++)
  {
    csVector3 camv = trans.Other2This (verts[i]);
    if (camv.z <= 0.0)
    {
      // @@@ Later we should clamp instead of ignoring this outline.
      delete[] tr_verts;
      return;
    }
    if (camv.z > max_depth) max_depth = camv.z;
    if (outline_info.outline_verts[i])
      Perspective (camv, tr_verts[i], fov, sx, sy);
  }

  if (do_state_dump)
  {
    iObject* iobj = SCF_QUERY_INTERFACE (visobj, iObject);
    if (iobj)
    {
      printf ("CovOutIns of object %s (max_depth=%g)\n",
      	iobj->GetName () ? iobj->GetName () : "<noname>",
	max_depth);
      iobj->DecRef ();
    }
    printf ("  campos_obj=%g,%g,%g\n",
    	campos_object.x, campos_object.y, campos_object.z);
  }

  // Then insert the outline.
  bool rc = covbuf->InsertOutline (tr_verts, vertex_count,
  	outline_info.outline_verts,
  	outline_info.outline_edges, outline_info.num_outline_edges, max_depth);
  if (do_state_dump)
  {
    printf ("  max_depth=%g rc=%d\n", max_depth, rc);
    int j;
    for (j = 0 ; j < vertex_count ; j++)
    {
      if (outline_info.outline_verts[j])
      {
        csVector3 cam = trans.Other2This (verts[j]);
        printf ("  V%d: (%g,%g / %g,%g,%g / %g,%g,%g)\n",
	  j,
	  tr_verts[j].x, tr_verts[j].y,
	  verts[j].x, verts[j].y, verts[j].z,
	  cam.x, cam.y, cam.z);
      }
    }
    for (j = 0 ; j < outline_info.num_outline_edges ; j++)
    {
      int vt1 = outline_info.outline_edges[j*2+0];
      int vt2 = outline_info.outline_edges[j*2+1];
      printf ("  E%d: %d-%d\n", j, vt1, vt2);
    }

    iString* str = covbuf->Debug_Dump ();
    printf ("%s\n", str->GetData ());
    str->DecRef ();
  }

  delete[] tr_verts;
}

void csDynaVis::AppendWriteQueue (iCamera* camera, iVisibilityObject* visobj,
  	csObjectModel* model, csVisibilityObjectWrapper* obj)
{
  iMovable* movable = visobj->GetMovable ();
  iPolygonMesh* polymesh = visobj->GetObjectModel ()->GetSmallerPolygonMesh ();

  const csVector3* verts = polymesh->GetVertices ();
  int vertex_count = polymesh->GetVertexCount ();

  csReversibleTransform movtrans = movable->GetFullTransform ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  csReversibleTransform trans = camtrans / movtrans;
  float fov = camera->GetFOV ();
  float sx = camera->GetShiftX ();
  float sy = camera->GetShiftY ();

  int i;
  // First calculate the bounding box of this occluder in 2D.
  csBox2 box;
  box.StartBoundingBox ();
  float max_depth = -1.0;
  for (i = 0 ; i < vertex_count ; i++)
  {
    csVector3 camv = trans.Other2This (verts[i]);
    if (camv.z <= 0.0)
    {
      // @@@ Later we should clamp instead of ignoring this outline.
      return;
    }
    if (camv.z > max_depth) max_depth = camv.z;
    //@@@@@@@@ If we have up-to-date outline information
    // we could use that here to prevent perspective projection
    // @@@@@@@ if (outline_info.outline_verts[i])
    csVector2 tr_vert;
    Perspective (camv, tr_vert, fov, sx, sy);
    box.AddBoundingVertex (tr_vert);
  }

  if (do_state_dump)
  {
    iObject* iobj = SCF_QUERY_INTERFACE (visobj, iObject);
    if (iobj)
    {
      printf ("AppendWriteQueue of object %s (max_depth=%g)\n",
      	iobj->GetName () ? iobj->GetName () : "<noname>",
	max_depth);
      iobj->DecRef ();
    }
  }

  // Then append to queue if box is actually on screen.
  if (box.MaxX () > 0 && box.MaxY () > 0 &&
      box.MinX () < scr_width && box.MinY () < scr_height)
  {
    write_queue->Append (box, max_depth, obj);
  }
}

bool csDynaVis::TestObjectVisibility (csVisibilityObjectWrapper* obj,
  	VisTest_Front2BackData* data)
{
  bool vis = true;
  bool do_write_object = false;

  // For coverage test.
  csBox2 sbox;
  float min_depth = 0;

  if (!obj->visobj->IsVisible ())
  {
    if (do_cull_history && obj->vis_cnt > 0)
    {
      obj->MarkVisible (VISIBLE_HISTORY, obj->vis_cnt-1);
      do_write_object = true;
      goto end;
    }

    const csBox3& obj_bbox = obj->child->GetBBox ();
    const csVector3& pos = data->pos;
    if (obj_bbox.Contains (pos))
    {
      obj->MarkVisible (VISIBLE_INSIDE, 1+((rand ()>>3)&0x3));
      do_write_object = true;
      goto end;
    }
  
    uint32 new_mask;
    if (do_cull_frustum && !csIntersect3::BoxFrustum (obj_bbox, data->frustum,
		data->frustum_mask, new_mask))
    {
      obj->reason = INVISIBLE_FRUSTUM;
      vis = false;
      goto end;
    }

    if (do_cull_coverage != COVERAGE_NONE)
    {
      iCamera* camera = data->rview->GetCamera ();
      float max_depth;
      if (obj_bbox.ProjectBox (camera->GetTransform (), camera->GetFOV (),
    	  camera->GetShiftX (), camera->GetShiftY (), sbox,
	  min_depth, max_depth))
        if (covbuf->TestRectangle (sbox, min_depth))
        {
	  // Object is visible. If we have a write queue we will first
	  // test if there are objects in the queue that may mark the
	  // object as non-visible.
	  if (do_cull_writequeue)
	  {
	    // If the write queue is enabled we try to see if there
	    // are occluders that are relevant (intersect with this object
	    // to test). We will insert those object with the coverage
	    // buffer and test again.
	    float out_depth;
	    csVisibilityObjectWrapper* qobj = (csVisibilityObjectWrapper*)
	    	write_queue->Fetch (sbox, min_depth, out_depth);
	    if (qobj)
	    {
	      // We have found one such object. Insert them all.
	      do
	      {
	        // Yes! We found such an object. Insert it now.
	        if (do_cull_coverage == COVERAGE_POLYGON)
		  UpdateCoverageBuffer (data->rview->GetCamera (), qobj->visobj,
		    qobj->model);
	        else
		  UpdateCoverageBufferOutline (data->rview->GetCamera (),
			  qobj->visobj, qobj->model);
	        qobj = (csVisibilityObjectWrapper*)
	    	  write_queue->Fetch (sbox, min_depth, out_depth);
	      }
	      while (qobj);
	      // Now try again.
              if (!covbuf->TestRectangle (sbox, min_depth))
	      {
	        // It really is invisible.
                obj->reason = INVISIBLE_TESTRECT;
	        vis = false;
                goto end;
	      }
	    }
	  }
	}
	else
	{
          obj->reason = INVISIBLE_TESTRECT;
	  vis = false;
          goto end;
        }
    }

    //---------------------------------------------------------------

    // Object is visible so we should write it to the coverage buffer.
    obj->MarkVisible (VISIBLE, 1+((rand ()>>3)&0x3));
    do_write_object = true;
  }

end:
  if (do_write_object && do_cull_coverage != COVERAGE_NONE &&
  	obj->visobj->GetObjectModel ()->GetSmallerPolygonMesh ())
  {
    // Object is visible.
    if (do_cull_writequeue)
    {
      // We are using the write queue so we insert the object there
      // for later culling.
      AppendWriteQueue (data->rview->GetCamera (), obj->visobj, obj->model,
      	obj);
    }
    else
    {
      // Let it update the coverage buffer if we
      // are using do_cull_coverage.
      if (do_cull_coverage == COVERAGE_POLYGON)
        UpdateCoverageBuffer (data->rview->GetCamera (), obj->visobj,
    	    obj->model);
      else
        UpdateCoverageBufferOutline (data->rview->GetCamera (), obj->visobj,
      	    obj->model);
    }
  }

  if (do_state_dump)
  {
    const csBox3& obj_bbox = obj->child->GetBBox ();
    iObject* iobj = SCF_QUERY_INTERFACE (obj->visobj, iObject);
    printf ("Obj (%g,%g,%g)-(%g,%g,%g) (%s) %s\n",
    	obj_bbox.MinX (), obj_bbox.MinY (), obj_bbox.MinZ (),
    	obj_bbox.MaxX (), obj_bbox.MaxY (), obj_bbox.MaxZ (),
	(iobj && iobj->GetName ()) ? iobj->GetName () : "<noname>",
	obj->reason == INVISIBLE_FRUSTUM ? "outside frustum" :
	obj->reason == INVISIBLE_TESTRECT ? "covered" :
	obj->reason == VISIBLE_INSIDE ? "visible inside" :
	obj->reason == VISIBLE_HISTORY ? "visible history" :
	obj->reason == VISIBLE ? "visible" :
	"?"
	);
    if (iobj) iobj->DecRef ();
    if (obj->reason != INVISIBLE_FRUSTUM && obj->reason != VISIBLE_INSIDE
    	&& obj->reason != VISIBLE_HISTORY)
    {
      printf ("  (%g,%g)-(%g,%g) min_depth=%g\n",
      	sbox.MinX (), sbox.MinY (),
      	sbox.MaxX (), sbox.MaxY (), min_depth);
    }
  }

  return vis;
}

static bool VisTest_Front2Back (csKDTree* treenode, void* userdata,
	uint32 cur_timestamp)
{
  VisTest_Front2BackData* data = (VisTest_Front2BackData*)userdata;
  csDynaVis* dynavis = data->dynavis;

  // Visible or not...
  bool vis = false;

  // Remember current frustum mask.
  uint32 old_frustum_mask = data->frustum_mask;

  // In the first part of this test we are going to test if the node
  // itself is visible. If it is not then we don't need to continue.
  if (!dynavis->TestNodeVisibility (treenode, data))
  {
    vis = false;
    goto end;
  }

  treenode->Distribute ();

  int num_objects;
  csKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
      	objects[i]->GetObject ();
      dynavis->TestObjectVisibility (visobj_wrap, data);
    }
  }

  vis = true;

end:
  // Restore the frustum mask.
  data->frustum_mask = old_frustum_mask;

  return vis;
}

bool csDynaVis::VisTest (iRenderView* rview)
{
  // Statistics and debugging.
  static csTicks t2 = 0;
  csTicks t1 = csGetTicks ();
  debug_camera = rview->GetOriginalCamera ();

  // Initialize the coverage buffer to all empty.
  covbuf->Initialize ();

  // Initialize the write queue to empty.
  write_queue->Initialize ();

  // Update all objects (mark them invisible and update in kdtree if needed).
  UpdateObjects ();

  // Data for the vis tester.
  VisTest_Front2BackData data;

  // First get the current view frustum from the rview.
  float lx, rx, ty, by;
  rview->GetFrustum (lx, rx, ty, by);
  csVector3 p0 (lx, by, 1);
  csVector3 p1 (lx, ty, 1);
  csVector3 p2 (rx, ty, 1);
  csVector3 p3 (rx, by, 1);
  const csReversibleTransform& trans = debug_camera->GetTransform ();
  csVector3 origin = trans.This2Other (csVector3 (0));
  p0 = trans.This2Other (p0);
  p1 = trans.This2Other (p1);
  p2 = trans.This2Other (p2);
  p3 = trans.This2Other (p3);
  data.frustum[0].Set (origin, p0, p1);
  data.frustum[1].Set (origin, p2, p3);
  data.frustum[2].Set (origin, p1, p2);
  data.frustum[3].Set (origin, p3, p0);
  //data.frustum[4].Set (origin, p0, p1);	// @@@ DO z=0 plane too!
  data.frustum_mask = 0xf;

  if (do_state_dump)
  {
    printf ("==============================================================\n");
    printf ("origin %g,%g,%g lx=%g rx=%g ty=%g by=%g\n",
    	trans.GetOrigin ().x, trans.GetOrigin ().y, trans.GetOrigin ().z,
	lx, rx, ty, by);
    uint32 mk = data.frustum_mask;
    int i;
    i = 0;
    while (mk)
    {
      printf ("frustum %g,%g,%g,%g\n",
      	data.frustum[i].A (), data.frustum[i].B (),
      	data.frustum[i].C (), data.frustum[i].D ());
      i++;
      mk >>= 1;
    }
  }

  // The big routine: traverse from front to back and mark all objects
  // visible that are visible. In the mean time also update the coverage
  // buffer for further culling.
  data.pos = rview->GetCamera ()->GetTransform ().GetOrigin ();
  data.rview = rview;
  data.dynavis = this;
  kdtree->Front2Back (data.pos, VisTest_Front2Back, (void*)&data);

  // Conclude statistics.
  if (t2 != 0)
    stats_total_notvistest_time += t1-t2;
  t2 = csGetTicks ();
  stats_total_vistest_time += t2-t1;
  stats_cnt_vistest++;

  do_state_dump = false;

  return true;
}

iString* csDynaVis::Debug_UnitTest ()
{
  csKDTree* kdtree = new csKDTree (NULL);
  iDebugHelper* dbghelp = SCF_QUERY_INTERFACE (kdtree, iDebugHelper);
  if (dbghelp)
  {
    iString* rc = dbghelp->UnitTest ();
    dbghelp->DecRef ();
    if (rc)
    {
      delete kdtree;
      return rc;
    }
  }
  delete kdtree;

  csCoverageBuffer* covbuf = new csCoverageBuffer (640, 480);
  dbghelp = SCF_QUERY_INTERFACE (covbuf, iDebugHelper);
  if (dbghelp)
  {
    iString* rc = dbghelp->UnitTest ();
    dbghelp->DecRef ();
    if (rc)
    {
      delete covbuf;
      return rc;
    }
  }
  delete covbuf;

  return NULL;
}

iString* csDynaVis::Debug_StateTest ()
{
  return NULL;
}

iString* csDynaVis::Debug_Dump ()
{
  return NULL;
}

void csDynaVis::Debug_Dump (iGraphics3D* g3d)
{
  struct color { int r, g, b; };
  static color reason_colors[] =
  {
    { 48, 48, 48 },
    { 64, 128, 64 },
    { 128, 64, 64 },
    { 255, 255, 255 },
    { 255, 255, 128 },
    { 255, 255, 196 }
  };

  if (debug_camera)
  {
    csTicks t1 = csGetTicks ();

    iGraphics2D* g2d = g3d->GetDriver2D ();
    iFontServer* fntsvr = g2d->GetFontServer ();
    iFont* fnt = NULL;
    if (fntsvr)
    {
      fnt = fntsvr->GetFont (0);
      if (fnt == NULL)
      {
        fnt = fntsvr->LoadFont (CSFONT_COURIER);
      }
    }

    iTextureManager* txtmgr = g3d->GetTextureManager ();
    g3d->BeginDraw (CSDRAW_2DGRAPHICS);
    int col_cam = txtmgr->FindRGB (0, 255, 0);
    int col_fgtext = txtmgr->FindRGB (0, 0, 0);
    int col_bgtext = txtmgr->FindRGB (255, 255, 255);

    if (cfg_view_mode == VIEWMODE_CLEARSTATSOVERLAY)
    {
      g2d->Clear (col_fgtext);
    }

    if (stats_cnt_vistest > 1)
    {
      char buf[200];
      float average_vistest_time = stats_total_vistest_time
    	  / float (stats_cnt_vistest);
      float average_notvistest_time = stats_total_notvistest_time
    	  / float (stats_cnt_vistest-1);
      sprintf (buf,
    	  "  cnt:%d vistest:%1.2g notvistest:%1.2g tot:%1.2g cull:%1.2g%%  ",
    	  stats_cnt_vistest,
	  average_vistest_time,
	  average_notvistest_time,
	  average_vistest_time+average_notvistest_time,
	  average_vistest_time * 100.0
		  / (average_vistest_time+average_notvistest_time));
      g2d->Write (fnt, 10, 5, col_fgtext, col_bgtext, buf);
    }

    if (cfg_view_mode == VIEWMODE_STATSOVERLAY
    	|| cfg_view_mode == VIEWMODE_CLEARSTATSOVERLAY)
    {
      int i;
      int reason_cols[LAST_REASON];
      for (i = 0 ; i < LAST_REASON ; i++)
      {
        reason_cols[i] = txtmgr->FindRGB (reason_colors[i].r,
      	  reason_colors[i].g, reason_colors[i].b);
      }
      csReversibleTransform trans = debug_camera->GetTransform ();
      trans = csTransform (csYRotMatrix3 (-PI/2.0), csVector3 (0)) * trans;
      float fov = g3d->GetPerspectiveAspect ();
      int sx, sy;
      g3d->GetPerspectiveCenter (sx, sy);

      csVector3 view_origin;
      // This is the z at which we want to view the origin.
      view_origin.z = debug_origin_z;
      // The x,y values are then calculated with inverse perspective
      // projection given that we want the view origin to be visualized
      // at view_persp_x and view_persp_y.
      float view_persp_x = sx;
      float view_persp_y = sy;
      view_origin.x = (view_persp_x-sx) * view_origin.z / fov;
      view_origin.y = (view_persp_y-sy) * view_origin.z / fov;
      trans.SetOrigin (trans.This2Other (-view_origin));

      csVector3 trans_origin = trans.Other2This (
    	  debug_camera->GetTransform ().GetOrigin ());
      csVector2 to;
      Perspective (trans_origin, to, fov, sx, sy);
      g2d->DrawLine (to.x-3,  to.y-3, to.x+3,  to.y+3, col_cam);
      g2d->DrawLine (to.x+3,  to.y-3, to.x-3,  to.y+3, col_cam);
      g2d->DrawLine (to.x,    to.y,   to.x+30, to.y,   col_cam);
      g2d->DrawLine (to.x+30, to.y,   to.x+24, to.y-4, col_cam);
      g2d->DrawLine (to.x+30, to.y,   to.x+24, to.y+4, col_cam);

      for (i = 0 ; i < visobj_vector.Length () ; i++)
      {
        csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
    	  visobj_vector[i];
        int col = reason_cols[visobj_wrap->reason];
        const csBox3& b = visobj_wrap->child->GetBBox ();
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xyz)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_Xyz)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xyz)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xYz)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xyz)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xyZ)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XYZ)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xYZ)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XYZ)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XyZ)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XYZ)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XYz)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_Xyz)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XYz)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_Xyz)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XyZ)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xYz)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xYZ)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xYz)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XYz)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xyZ)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_XyZ)), fov, col);
        g3d->DrawLine (
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xyZ)),
      	  trans.Other2This (b.GetCorner (CS_BOX_CORNER_xYZ)), fov, col);
      }
    }
    else if (cfg_view_mode == VIEWMODE_OUTLINES)
    {
      int i;
      for (i = 0 ; i < visobj_vector.Length () ; i++)
      {
	csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
	  visobj_vector[i];
	iVisibilityObject* visobj = visobj_wrap->visobj;
        if (visobj->IsVisible ())
	{
	  // Only render outline if visible.
          const csReversibleTransform& camtrans = debug_camera->GetTransform ();
	  iMovable* movable = visobj->GetMovable ();
	  csReversibleTransform movtrans = movable->GetFullTransform ();
	  csReversibleTransform trans = camtrans / movtrans;

	  csVector3 campos_object = movtrans.Other2This (camtrans.GetOrigin ());
	  visobj_wrap->model->UpdateOutline (campos_object);
	  const csOutlineInfo& outline_info = visobj_wrap->model
	  	->GetOutlineInfo ();
	  float fov = debug_camera->GetFOV ();
	  float sx = debug_camera->GetShiftX ();
	  float sy = debug_camera->GetShiftY ();

	  iPolygonMesh* polymesh = visobj->GetObjectModel ()->
	  	GetSmallerPolygonMesh ();
	  const csVector3* verts = polymesh->GetVertices ();

	  int j;
	  int* e = outline_info.outline_edges;
	  for (j = 0 ; j < outline_info.num_outline_edges ; j++)
	  {
	    int vt1 = *e++;
	    int vt2 = *e++;
	    csVector3 camv1 = trans.Other2This (verts[vt1]);
	    if (camv1.z <= 0.0) continue;
	    csVector3 camv2 = trans.Other2This (verts[vt2]);
	    if (camv2.z <= 0.0) continue;
	    csVector2 tr_vert1, tr_vert2;
	    Perspective (camv1, tr_vert1, fov, sx, sy);
	    Perspective (camv2, tr_vert2, fov, sx, sy);
	    g2d->DrawLine (tr_vert1.x,  g2d->GetHeight ()-tr_vert1.y,
	    	tr_vert2.x,  g2d->GetHeight ()-tr_vert2.y, col_bgtext);
	  }
	}
      }
    }

    // Try to correct for the time taken to print this debug info.
    csTicks t2 = csGetTicks ();
    stats_total_notvistest_time -= t2-t1;
  }
}

bool csDynaVis::Debug_DebugCommand (const char* cmd)
{
  if (!strcmp (cmd, "toggle_frustum"))
  {
    do_cull_frustum = !do_cull_frustum;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s frustum culling!", do_cull_frustum ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_queue"))
  {
    do_cull_writequeue = !do_cull_writequeue;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s write queue!", do_cull_writequeue ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_history"))
  {
    do_cull_history = !do_cull_history;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s history culling!", do_cull_history ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_coverage"))
  {
    do_cull_coverage++;
    if (do_cull_coverage > COVERAGE_OUTLINE) do_cull_coverage = COVERAGE_NONE;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s coverage culling!",
	do_cull_coverage == COVERAGE_NONE ? "Disabled" :
	do_cull_coverage == COVERAGE_POLYGON ? "Polygon" :
	"Outline");
    return true;
  }
  else if (!strncmp (cmd, "origin_z", 8))
  {
    if (!strcmp (cmd+9, "+"))
      debug_origin_z += 1.0;
    else if (!strcmp (cmd+9, "++"))
      debug_origin_z += 10.0;
    else if (!strcmp (cmd+9, "-"))
      debug_origin_z -= 1.0;
    else if (!strcmp (cmd+9, "--"))
      debug_origin_z -= 10.0;
    else
      sscanf (cmd+9, "%f", &debug_origin_z);
  }
  else if (!strcmp (cmd, "clear_stats"))
  {
    stats_cnt_vistest = 0;
    stats_total_vistest_time = 0;
    stats_total_notvistest_time = 0;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"Statistics cleared.");
    return true;
  }
  else if (!strcmp (cmd, "cycle_view"))
  {
    cfg_view_mode++;
    if (cfg_view_mode > VIEWMODE_LAST)
      cfg_view_mode = VIEWMODE_FIRST;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"View mode %s",
	cfg_view_mode == VIEWMODE_STATS ? "statistics only" :
	cfg_view_mode == VIEWMODE_STATSOVERLAY ? "statistics and map" :
	cfg_view_mode == VIEWMODE_CLEARSTATSOVERLAY ? "statistics and map (c)" :
	cfg_view_mode == VIEWMODE_OUTLINES ? "outlines" :
	"?");
    return true;
  }
  else if (!strcmp (cmd, "dump_state"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"Dumped current state.");
    do_state_dump = true;
    return true;
  }
  else if (!strcmp (cmd, "analyze_vis"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"Analyze visibility status.");
    csExactCuller* excul = new csExactCuller (scr_width, scr_height);
    int i;
    for (i = 0 ; i < visobj_vector.Length () ; i++)
    {
      csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
        visobj_vector[i];
      iVisibilityObject* visobj = visobj_wrap->visobj;
      iMovable* movable = visobj->GetMovable ();
      iPolygonMesh* polymesh = visobj->GetObjectModel ()
      	->GetSmallerPolygonMesh ();
      if (polymesh)
        excul->AddObject (visobj_wrap, polymesh, movable, debug_camera,
  	  visobj_wrap->model->GetPlanes ());
    }
    excul->VisTest ();
    int tot_vis_exact = 0;
    int tot_vis_dynavis = 0;
    int tot_objects = 0;
    int tot_poly_exact = 0;
    int tot_poly_dynavis = 0;
    int tot_poly = 0;
    for (i = 0 ; i < visobj_vector.Length () ; i++)
    {
      csVisibilityObjectWrapper* visobj_wrap = (csVisibilityObjectWrapper*)
        visobj_vector[i];
      iPolygonMesh* polymesh = visobj_wrap->visobj->GetObjectModel ()
      	->GetSmallerPolygonMesh ();
      if (polymesh)
      {
        int vispix, totpix;
        excul->GetObjectStatus (visobj_wrap, vispix, totpix);

	tot_objects++;
	tot_poly += polymesh->GetPolygonCount ();
	if (vispix)
	{
	  tot_vis_exact++;
	  tot_poly_exact += polymesh->GetPolygonCount ();
	}
	if (visobj_wrap->reason >= VISIBLE)
	{
	  tot_vis_dynavis++;
	  tot_poly_dynavis += polymesh->GetPolygonCount ();
	}

        iObject* iobj = SCF_QUERY_INTERFACE (visobj_wrap->visobj, iObject);
        printf ("  obj(%d,'%s')  vis=%s   vispix=%d totpix=%d      %s\n",
      	  i,
	  (iobj && iobj->GetName ()) ? iobj->GetName () : "?",
	  visobj_wrap->reason == INVISIBLE_PARENT ? "invis parent" :
	  visobj_wrap->reason == INVISIBLE_FRUSTUM ? "invis frustum" :
	  visobj_wrap->reason == INVISIBLE_TESTRECT ? "invis testrect" :
	  visobj_wrap->reason == VISIBLE ? "vis" :
	  visobj_wrap->reason == VISIBLE_INSIDE ? "vis inside" :
	  visobj_wrap->reason == VISIBLE_HISTORY ? "vis history" :
	  "?",
	  vispix, totpix,
	  vispix != 0 && visobj_wrap->reason < VISIBLE ? "????!!!!" : "");
        if (iobj) iobj->DecRef ();
      }
    }
    printf ("Summary: #objects=%d #vis(exact)=%d #vis(dynavis)=%d\n",
    	tot_objects, tot_vis_exact, tot_vis_dynavis);
    printf ("Summary: #poly=%d #poly(exact)=%d #poly(dynavis)=%d\n",
    	tot_poly, tot_poly_exact, tot_poly_dynavis);
    fflush (stdout);

    delete excul;
  }
  return false;
}

csTicks csDynaVis::Debug_Benchmark (int num_iterations)
{
  csTicks rc = 0;

  csKDTree* kdtree = new csKDTree (NULL);
  iDebugHelper* dbghelp = SCF_QUERY_INTERFACE (kdtree, iDebugHelper);
  if (dbghelp)
  {
    csTicks r = dbghelp->Benchmark (num_iterations);
    printf ("kdtree:   %d ms\n", r);
    rc += r;
    dbghelp->DecRef ();
  }
  delete kdtree;

  csCoverageBuffer* covbuf = new csCoverageBuffer (640, 480);
  dbghelp = SCF_QUERY_INTERFACE (covbuf, iDebugHelper);
  if (dbghelp)
  {
    csTicks r = dbghelp->Benchmark (num_iterations);
    printf ("covbuf:   %d ms\n", r);
    rc += r;
    dbghelp->DecRef ();
  }
  delete covbuf;

  return rc;
}

