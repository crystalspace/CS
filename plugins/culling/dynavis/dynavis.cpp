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
#include <string.h>
#include "csver.h"
#include "csutil/sysfunc.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/scfstr.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/array.h"
#include "csutil/cfgacc.h"
#include "csgeom/frustum.h"
#include "csgeom/matrix3.h"
#include "csgeom/math3d.h"
#include "csgeom/obb.h"
#include "csgeom/segment.h"
#include "csgeom/sphere.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/kdtree.h"
#include "csgeom/tcovbuf.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"
#include "igeom/clip2d.h"
#include "csutil/flags.h"
#include "iutil/objreg.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/shadcast.h"
#include "iengine/shadows.h"
#include "iengine/fview.h"
#include "imesh/object.h"
#include "iutil/object.h"
#include "ivaria/reporter.h"
#include "ivaria/bugplug.h"
#include "dynavis.h"
#include "wqueue.h"
#include "exvis.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csDynaVis)

CS_LEAKGUARD_IMPLEMENT (csVisibilityObjectWrapper);

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

//----------------------------------------------------------------------

class csDynVisObjIt : public iVisibilityObjectIterator
{
private:
  csArray<iVisibilityObject*>* vector;
  size_t position;
  bool* vistest_objects_inuse;

public:
  SCF_DECLARE_IBASE;

  csDynVisObjIt (csArray<iVisibilityObject*>* vector,
  	bool* vistest_objects_inuse)
  {
    SCF_CONSTRUCT_IBASE (0);
    csDynVisObjIt::vector = vector;
    csDynVisObjIt::vistest_objects_inuse = vistest_objects_inuse;
    if (vistest_objects_inuse) *vistest_objects_inuse = true;
    Reset ();
  }
  virtual ~csDynVisObjIt ()
  {
    // If the vistest_objects_inuse pointer is not 0 we set the
    // bool to false to indicate we're no longer using the base
    // vector. Otherwise we delete the vector.
    if (vistest_objects_inuse) *vistest_objects_inuse = false;
    else delete vector;
    SCF_DESTRUCT_IBASE();
  }

  virtual iVisibilityObject* Next()
  {
    if (position == (size_t)-1) return 0;
    iVisibilityObject* vo = vector->Get (position);
    position++;
    if (position == vector->Length ())
      position = (size_t)-1;
    return vo;
  }

  virtual void Reset()
  {
    if (vector == 0 || vector->Length () < 1)
      position = (size_t)-1;
    else
      position = 0;
  }

  virtual bool HasNext () const
  {
    return ((position != (size_t)-1) && position <= vector->Length ());
  }
};

SCF_IMPLEMENT_IBASE (csDynVisObjIt)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityObjectIterator)
SCF_IMPLEMENT_IBASE_END

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csVisibilityObjectWrapper)
  SCF_IMPLEMENTS_INTERFACE (iObjectModelListener)
  SCF_IMPLEMENTS_INTERFACE (iMovableListener)
SCF_IMPLEMENT_IBASE_END

void csVisibilityObjectWrapper::ObjectModelChanged (iObjectModel* /*model*/)
{
  dynavis->AddObjectToUpdateQueue (this);
}

void csVisibilityObjectWrapper::MovableChanged (iMovable* /*movable*/)
{
  dynavis->AddObjectToUpdateQueue (this);
}

//----------------------------------------------------------------------

// Fast random generator. Only the 16 least significant
// bits are guaranteed to be random.
static unsigned int seed = 362436069;

static inline unsigned int csFastrand ()
{
 static unsigned int b = 30903;
 seed = b*(seed & 0xffff) + (seed>>16);
 return seed;
}

// This function defines the amount to use for keeping
// an object/node visible after it was marked visible
// for some other reason.
static inline int dist_history ()
{
  return 9+(csFastrand () & 0x7);
}
static inline int dist_nowritequeue ()
{
  return 15+(csFastrand () & 0x7);
}
static inline int dist_nooccluder ()
{
  return 8+(csFastrand () & 0x7);
}

bool csDynaVis::do_cull_frustum = true;
int csDynaVis::do_cull_coverage = COVERAGE_OUTLINE;
bool csDynaVis::do_cull_history = true;
bool csDynaVis::do_cull_writequeue = true;
bool csDynaVis::do_cull_ignoresmall = false;
bool csDynaVis::do_cull_clampoccluder = false;
bool csDynaVis::do_cull_vpt = true;
bool csDynaVis::do_cull_outline_splatting = false;	// Fix bug with outlines behind view plane first!!!
bool csDynaVis::do_insert_inverted_clipper = true;
bool csDynaVis::do_cull_ignore_bad_occluders = true;
int csDynaVis::badoccluder_thresshold = 10;
int csDynaVis::badoccluder_maxsweepcount = 50;

csDynaVis::csDynaVis (iBase *iParent) : visobj_wrappers (1000)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  object_reg = 0;
  kdtree = 0;
  tcovbuf = 0;
  debug_camera = 0;
  model_mgr = new csObjectModelManager ();
  write_queue = new csWriteQueue ();
  current_vistest_nr = 1;
  badoccluder_sweepcount = 0;
  history_frame_cnt = 2;
  vistest_objects_inuse = false;

  updating = false;

  do_freeze_vis = false;

  cfg_view_mode = VIEWMODE_STATS;
  do_state_dump = false;
  debug_origin_z = 50;
}

csDynaVis::~csDynaVis ()
{
  while (visobj_vector.Length () > 0)
  {
    csVisibilityObjectWrapper* visobj_wrap = visobj_vector.Pop ();
    iVisibilityObject* visobj = visobj_wrap->visobj;
    visobj_wrap->model->GetModel ()->RemoveListener (
		      (iObjectModelListener*)visobj_wrap);
    iMovable* movable = visobj->GetMovable ();
    movable->RemoveListener ((iMovableListener*)visobj_wrap);
    model_mgr->ReleaseObjectModel (visobj_wrap->model);
    kdtree->RemoveObject (visobj_wrap->child);
    visobj->DecRef ();
    visobj_wrappers.Free (visobj_wrap);
  }
  delete kdtree;
  delete tcovbuf;
  delete model_mgr;
  delete write_queue;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csDynaVis::Initialize (iObjectRegistry *object_reg)
{
  csDynaVis::object_reg = object_reg;

  delete kdtree;
  delete tcovbuf; tcovbuf = 0;

  csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d)
  {
    scr_width = g3d->GetWidth ();
    scr_height = g3d->GetHeight ();
  }
  else
  {
    scr_width = -1;
    scr_height = -1;
  }

  if (scr_width == -1 || scr_height == -1) 
  {
    // If there is no g3d we currently assume we are testing.
    scr_width = 640;
    scr_height = 480;
  }

  csConfigAccess config;
  config.AddConfig(object_reg, "/config/dynavis.cfg");
  reduce_buf = config->GetInt ("Culling.Dynavis.ReduceCoverageBuffer", 0);
  scr_width = scr_width << reduce_buf;
  scr_height = scr_height << reduce_buf;

  do_cull_frustum = config->GetBool ("Culling.Dynavis.FrustumCull", true);
  const char* str = config->GetStr ("Culling.Dynavis.Coverage", "outline");
  if (!strcmp (str, "outline"))
    do_cull_coverage = COVERAGE_OUTLINE;
  else if (!strcmp (str, "polygon"))
    do_cull_coverage = COVERAGE_POLYGON;
  else
    do_cull_coverage = COVERAGE_NONE;

  do_cull_history = config->GetBool ("Culling.Dynavis.History", true);
  do_cull_writequeue = config->GetBool ("Culling.Dynavis.WriteQueue", true);
  do_cull_ignoresmall = config->GetBool ("Culling.Dynavis.IgnoreSmall", false);
  do_cull_clampoccluder = config->GetBool ("Culling.Dynavis.ClampOccluder",
  	false);
  do_cull_vpt = config->GetBool ("Culling.Dynavis.VPT", true);

  // Fix bug with outlines behind view plane first!!!
  do_cull_outline_splatting = config->GetBool (
  	"Culling.Dynavis.OutlineSplatting", false);

  do_insert_inverted_clipper = config->GetBool (
  	"Culling.Dynavis.InvertedClipper", true);
  do_cull_ignore_bad_occluders = config->GetBool (
  	"Culling.Dynavis.IgnoreBadOccluders", true);
  badoccluder_maxsweepcount = config->GetInt (
  	"Culling.Dynavis.RetryOccluders", 50);
  badoccluder_thresshold = config->GetInt (
  	"Culling.Dynavis.BadOccluderThresshold", 10);

  kdtree = new csKDTree ();

  tcovbuf = new csTiledCoverageBuffer (scr_width, scr_height);
  csRef<iBugPlug> bugplug = CS_QUERY_REGISTRY (object_reg, iBugPlug);
  tcovbuf->bugplug = bugplug;

  return true;
}

void csDynaVis::Setup (const char* /*name*/)
{
}

void csDynaVis::CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox,
	bool full_transform_identity)
{
  iMovable* movable = visobj->GetMovable ();
  if (full_transform_identity)
  {
    visobj->GetObjectModel ()->GetObjectBoundingBox (bbox);
  }
  else
  {
    csBox3 box;
    visobj->GetObjectModel ()->GetObjectBoundingBox (box);
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
}

void csDynaVis::RegisterVisObject (iVisibilityObject* visobj)
{
#ifdef CS_DEBUG
  size_t i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    if (visobj_vector[i]->visobj == visobj)
    {
      CS_ASSERT (false);
    }
  }
#endif
  csVisibilityObjectWrapper* visobj_wrap = visobj_wrappers.Alloc ();
  visobj_wrap->SetDynavis (this);
  visobj_wrap->visobj = visobj;
  visobj->IncRef ();
  iMovable* movable = visobj->GetMovable ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();

  visobj_wrap->model = model_mgr->CreateObjectModel (
		  visobj->GetObjectModel ());
  visobj_wrap->shape_number = visobj_wrap->model->GetShapeNumber ();

  iMeshWrapper* mesh = visobj->GetMeshWrapper ();
  visobj_wrap->mesh = mesh;
  if (mesh)
  {
    visobj_wrap->caster = mesh->GetShadowCaster ();
  }
  AddObjectToUpdateQueue (visobj_wrap);

  csBox3 bbox;
  visobj_wrap->full_transform_identity = movable->IsFullTransformIdentity ();
  CalculateVisObjBBox (visobj, bbox, visobj_wrap->full_transform_identity);
  visobj_wrap->child = kdtree->AddObject (bbox, (void*)visobj_wrap);
  kdtree_box += bbox;

  // Only add the listeners at the very last moment to prevent them to
  // be called by the calls above (i.e. especially the calculation of
  // the bounding box could cause a listener to be fired).
  movable->AddListener ((iMovableListener*)visobj_wrap);
  iObjectModel* model = visobj_wrap->model->GetModel ();
  model->AddListener ((iObjectModelListener*)visobj_wrap);

  if (model->GetPolygonMeshViscull ())
    visobj_wrap->hint_closed = model->GetPolygonMeshViscull ()
    	->GetFlags ().Check (CS_POLYMESH_CLOSED);
  else
    visobj_wrap->hint_closed = false;

  if (visobj_wrap->model->IsEmptyObject ())
  {
    visobj_wrap->hint_badoccluder = true;
    visobj_wrap->hint_goodoccluder = false;
  }
  else
  {
    visobj_wrap->hint_badoccluder = visobj->GetCullerFlags ().Check (
  	CS_CULLER_HINT_BADOCCLUDER);
    visobj_wrap->hint_goodoccluder = visobj->GetCullerFlags ().Check (
  	CS_CULLER_HINT_GOODOCCLUDER);
  }

  visobj_wrap->use_outline_filler = (visobj_wrap->hint_closed
  	|| visobj_wrap->model->CanUseOutlineFiller ())
	&& !visobj_wrap->hint_goodoccluder;

  visobj_vector.Push (visobj_wrap);
}

void csDynaVis::UnregisterVisObject (iVisibilityObject* visobj)
{
  size_t i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
    if (visobj_wrap->visobj == visobj)
    {
      update_queue.Delete (visobj_wrap);
      visobj_wrap->model->GetModel ()->RemoveListener (
		      (iObjectModelListener*)visobj_wrap);
      iMovable* movable = visobj->GetMovable ();
      movable->RemoveListener ((iMovableListener*)visobj_wrap);
      model_mgr->ReleaseObjectModel (visobj_wrap->model);
      kdtree->RemoveObject (visobj_wrap->child);
      visobj->DecRef ();
#ifdef CS_DEBUG
      // To easily recognize that the vis wrapper has been deleted:
      visobj_wrap->dynavis = (csDynaVis*)0xdeadbeef;
#endif
      visobj_vector.DeleteIndexFast (i);
      visobj_wrappers.Free (visobj_wrap);
      return;
    }
  }
}

void csDynaVis::AddObjectToUpdateQueue (csVisibilityObjectWrapper* visobj_wrap)
{
  if (updating) return;
  CS_ASSERT (visobj_wrap->dynavis != (csDynaVis*)0xdeadbeef);
  update_queue.Add (visobj_wrap);
}

void csDynaVis::UpdateObjects ()
{
  updating = true;
  {
    csSet<csVisibilityObjectWrapper*>::GlobalIterator it = update_queue.
    	GetIterator ();
    while (it.HasNext ())
    {
      csVisibilityObjectWrapper* vw = it.Next ();
      UpdateObject (vw);
    }
  }
  update_queue.DeleteAll ();
  updating = false;
}

void csDynaVis::UpdateObject (csVisibilityObjectWrapper* visobj_wrap)
{
  CS_ASSERT (visobj_wrap->dynavis != (csDynaVis*)0xdeadbeef);
  iVisibilityObject* visobj = visobj_wrap->visobj;
  iMovable* movable = visobj->GetMovable ();
  model_mgr->CheckObjectModel (visobj_wrap->model, visobj_wrap->mesh);
  visobj_wrap->use_outline_filler = (visobj_wrap->hint_closed
  	|| visobj_wrap->model->CanUseOutlineFiller ())
	&& !visobj_wrap->hint_goodoccluder;

  if (visobj_wrap->model->IsEmptyObject ())
  {
    visobj_wrap->hint_badoccluder = true;
    visobj_wrap->hint_goodoccluder = false;
  }
  else
  {
    visobj_wrap->hint_badoccluder = visobj->GetCullerFlags ().Check (
  	CS_CULLER_HINT_BADOCCLUDER);
    visobj_wrap->hint_goodoccluder = visobj->GetCullerFlags ().Check (
  	CS_CULLER_HINT_GOODOCCLUDER);
  }

  csBox3 bbox;
  visobj_wrap->full_transform_identity = movable->IsFullTransformIdentity ();
  CalculateVisObjBBox (visobj, bbox, visobj_wrap->full_transform_identity);
  kdtree->MoveObject (visobj_wrap->child, bbox);
  kdtree_box += bbox;
  visobj_wrap->shape_number = visobj_wrap->model->GetShapeNumber ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();
}

namespace
{

// Version to cope with z <= 0. This is wrong but it in the places where
// it is used below the result is acceptable because it generates a
// conservative result (i.e. a box or outline that is bigger then reality).
void PerspectiveWrong (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov * 10;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

void InvPerspective (const csVector2& p, float z, csVector3& v,
	float fov, float sx, float sy)
{
  float iz = z / fov;
  v.x = (p.x - sx) * iz;
  v.y = (p.y - sy) * iz;
  v.z = z;
}

void Perspective (const csVector3& v, csVector2& p,
	float fov, float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

bool PrintObjects (csKDTree* treenode, void*, uint32, uint32&)
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
    csRef<iObject> iobj (SCF_QUERY_INTERFACE (visobj_wrap->visobj, iObject));
    if (iobj)
    {
      if (iobj->GetName ()) 
	csPrintf ("'%s' ", iobj->GetName ());
      else 
	csPrintf ("<noname> ");
    }
  }
  return true;
}
} // end of anonymous namespace

struct VisTest_Front2BackData
{
  csVector3 pos;
  iRenderView* rview;
  csDynaVis* dynavis;
  csPlane3* frustum;
  // this is the callback to call when we discover a visible node
  iVisibilityCullerListener* viscallback;
};

bool csDynaVis::TestNodeVisibility (csKDTree* treenode,
	VisTest_Front2BackData* data, uint32& frustum_mask)
{
  csBox3 node_bbox;
  const csBox3& orig_node_bbox = treenode->GetNodeBBox ();
  const csVector3& pos = data->pos;

  csVisibilityObjectHistory* hist = (csVisibilityObjectHistory*)
  	treenode->GetUserObject ();
  if (!hist)
  {
    hist = new csVisibilityObjectHistory ();
    treenode->SetUserObject (hist);
    hist->DecRef ();
  }

  // For coverage testing.
  csBox2 sbox;
  float min_depth = 0;

  bool vis = true;

  if (do_cull_history && hist->vis_cnt >= history_frame_cnt)
  {
    hist->reason = VISIBLE_HISTORY;
    cnt_node_visible++;
    // Here we do a bitwise and of the frustum mask that we have
    // calculated so far and the frustum mask we remembered.
    frustum_mask = frustum_mask & hist->history_frustum_mask;
    goto end;
  }

  if (orig_node_bbox.Contains (pos))
  {
    hist->reason = VISIBLE_INSIDE;
    hist->vis_cnt = history_frame_cnt + dist_history ();
    hist->no_writequeue_vis_cnt = 0;
    hist->history_frustum_mask = frustum_mask;
    cnt_node_visible++;
    goto end;
  }

  node_bbox = orig_node_bbox;
  node_bbox *= kdtree_box;

  if (do_cull_frustum)
  {
    uint32 new_mask;
    if (!csIntersect3::BoxFrustum (node_bbox, data->frustum, frustum_mask,
    	new_mask))
    {
      hist->reason = INVISIBLE_FRUSTUM;
      hist->no_writequeue_vis_cnt = 0;
      vis = false;
      goto end;
    }
    // In VisTest_Front2Back() this is later restored when recursing back to
    // higher level.
    frustum_mask = new_mask;
  }

  if (do_cull_coverage != COVERAGE_NONE)
  {
    // @@@ Do write queue here too? First tests indicate that this is not
    // a good idea.
    float max_depth;
#define DO_OUTLINE_TEST 0
#define DO_WRITEQUEUE_TEST 0
#if DO_OUTLINE_TEST
    static csPoly2D outline;
    outline.MakeEmpty ();
    if (node_bbox.ProjectBoxAndOutline (cam_trans, fov, sx, sy, sbox,
    	outline, min_depth, max_depth))
#else
    if (node_bbox.ProjectBox (cam_trans, fov, sx, sy, sbox,
    	min_depth, max_depth))
#endif
    {
#     ifdef CS_DEBUG
      if (do_state_dump)
      {
        csRef<iString> str = tcovbuf->Debug_Dump ();
        csPrintf ("Before node test:\n%s\n", str->GetData ());
      }
#     endif
      // @@@ VPT tracking for nodes!!!
      csTestRectData testrect_data;
      bool mark_culled_object;
      bool rc = tcovbuf->PrepareTestRectangle (sbox, testrect_data);
      if (rc)
      {
        mark_culled_object = badoccluder_thresshold >= 0;
        rc = tcovbuf->TestRectangle (testrect_data, min_depth);
      }
      else
      {
        mark_culled_object = false;
      }

      if (!rc)
      {
        hist->reason = INVISIBLE_TESTRECT;
        hist->no_writequeue_vis_cnt = 0;
        vis = false;
        if (mark_culled_object)
          tcovbuf->MarkCulledObject (testrect_data);
        goto end;
      }
#if DO_WRITEQUEUE_TEST
      else if (do_cull_writequeue &&
      	 hist->no_writequeue_vis_cnt <= history_frame_cnt)
      {
	bool use_wq = TestWriteQueueRelevance (min_depth, testrect_data, sbox);
	if (use_wq)
	{
	  float out_depth;
	  csVisibilityObjectWrapper* qobj = (csVisibilityObjectWrapper*)
	    	write_queue->Fetch (sbox, min_depth, out_depth);
	  if (qobj)
	  {
#           ifdef CS_DEBUG
	    if (do_state_dump)
	    {
	      csPrintf ("Adding objects from write queue (for node)!\n");
	      fflush (stdout);
	    }
#           endif
	    // We have found one such object. Insert them all.
	    do
	    {
	      // Yes! We found such an object. Insert it now.
	      UpdateCoverageBuffer (qobj);
	      // Now try again.
              rc = tcovbuf->TestRectangle (testrect_data, min_depth);
              if (!rc)
	      {
	        // It really is invisible.
		hist->reason = INVISIBLE_TESTRECT;
		hist->no_writequeue_vis_cnt = 0;
	        vis = false;
		if (mark_culled_object)
		  tcovbuf->MarkCulledObject (testrect_data);
                goto end;
	      }
	      qobj = (csVisibilityObjectWrapper*)
	    	    write_queue->Fetch (sbox, min_depth, out_depth);
	    }
	    while (qobj);
	  }
	}
      }
#endif
#if DO_OUTLINE_TEST
      else
      {
        // @@@ VPT tracking for nodes!!!
        rc = tcovbuf->TestPolygon (outline.GetVertices (),
	  	outline.GetVertexCount (), min_depth);

        if (!rc)
        {
          hist->reason = INVISIBLE_TESTRECT;
	  hist->no_writequeue_vis_cnt = 0;
          vis = false;
	  if (mark_culled_object)
	    tcovbuf->MarkCulledObject (testrect_data);
          goto end;
        }
      }
#endif
    }
  }

  hist->reason = VISIBLE;
  hist->vis_cnt = history_frame_cnt + dist_history ();
#if DO_WRITEQUEUE_TEST
  hist->no_writequeue_vis_cnt = hist->vis_cnt + dist_nowritequeue ();
#endif
  hist->history_frustum_mask = frustum_mask;
  cnt_node_visible++;

end:
# ifdef CS_DEBUG
  if (do_state_dump)
  {
    csPrintf ("Node (%g,%g,%g)-(%g,%g,%g) %s\n",
    	orig_node_bbox.MinX (), orig_node_bbox.MinY (), orig_node_bbox.MinZ (),
    	orig_node_bbox.MaxX (), orig_node_bbox.MaxY (), orig_node_bbox.MaxZ (),
	hist->reason == INVISIBLE_FRUSTUM ? "outside frustum" :
	hist->reason == INVISIBLE_TESTRECT ? "covered" :
	hist->reason == VISIBLE_INSIDE ? "visible inside" :
	hist->reason == VISIBLE ? "visible" :
	hist->reason == VISIBLE_VPT ? "visible vpt" :
	"?"
	);
    if (hist->reason != INVISIBLE_FRUSTUM && hist->reason != VISIBLE_INSIDE)
    {
      csPrintf ("  (%g,%g)-(%g,%g) min_depth=%g\n",
      	sbox.MinX (), sbox.MinY (),
      	sbox.MaxX (), sbox.MaxY (), min_depth);
    }
    if (hist->reason != VISIBLE && hist->reason != VISIBLE_INSIDE
    	&& hist->reason != VISIBLE_VPT)
    {
      csPrintf ("  ");
      treenode->Front2Back (data->pos, PrintObjects, 0, 0);
      csPrintf ("\n");
    }
  }
# endif

  return vis;
}

typedef csDirtyAccessArray<csVector2> dynavis_tr_verts;
CS_IMPLEMENT_STATIC_VAR (GetTrVerts, dynavis_tr_verts, ())

typedef csDirtyAccessArray<csVector3> dynavis_tr_cam;
CS_IMPLEMENT_STATIC_VAR (GetTrCam, dynavis_tr_cam, ())

void csDynaVis::UpdateCoverageBuffer (csVisibilityObjectWrapper* obj)
{
  if (do_cull_coverage != COVERAGE_POLYGON && obj->use_outline_filler)
  {
    UpdateCoverageBufferOutline (obj);
    return;
  }
  iVisibilityObject* visobj = obj->visobj;
  csDynavisObjectModel* model = obj->model;

  iMovable* movable = visobj->GetMovable ();
  iPolygonMesh* polymesh = visobj->GetObjectModel ()->GetPolygonMeshViscull ();

  const csVector3* verts = polymesh->GetVertices ();
  size_t vertex_count = polymesh->GetVertexCount ();
  size_t poly_count = polymesh->GetPolygonCount ();

  csReversibleTransform trans = cam_trans;
  // Camera position in object space.
  csVector3 campos_object;
  if (obj->full_transform_identity)
  {
    campos_object = trans.GetOrigin ();
  }
  else
  {
    csReversibleTransform movtrans = movable->GetFullTransform ();
    campos_object = movtrans.Other2This (trans.GetOrigin ());
    trans /= movtrans;
  }

  size_t i;

  // First transform all vertices.
  dynavis_tr_verts *tr_verts = GetTrVerts();
  dynavis_tr_cam *tr_cam = GetTrCam();
  
  if (vertex_count > tr_verts->Length())
  {
    tr_verts->SetLength (vertex_count);
    tr_cam->SetLength (vertex_count);
  }

  for (i = 0 ; i < vertex_count ; i++)
  {
    (*tr_cam)[i] = trans.Other2This (verts[i]);
    if ((*tr_cam)[i].z > 0.1)
      Perspective ((*tr_cam)[i], (*tr_verts)[i], fov, sx, sy);
  }

# ifdef CS_DEBUG
  if (do_state_dump)
  {
    csRef<iObject> iobj (SCF_QUERY_INTERFACE (visobj, iObject));
    if (iobj)
    {
      csPrintf ("CovIns of object %s\n", iobj->GetName () ? iobj->GetName () :
      	"<noname>");
    }
  }
# endif

  // Then insert all polygons.
  csMeshedPolygon* poly = polymesh->GetPolygons ();
  const csPlane3* planes = model->GetPlanes ();
  csVector2 verts2d[64];
  int modified = 0;
  csBox2Int occluder_box;
  occluder_box.minx = 10000;
  occluder_box.miny = 10000;
  occluder_box.maxx = -10000;
  occluder_box.maxy = -10000;
  for (i = 0 ; i < poly_count ; i++, poly++)
  {
    if (planes[i].Classify (campos_object) >= 0.0)
      continue;

    bool do_clamp = false;
    size_t num_verts = poly->num_vertices;
    int* vi = poly->vertices;
    float max_depth = -1.0;
    size_t j;
    for (j = 0 ; j < num_verts ; j++)
    {
      int vertex_idx = vi[j];
      float tz = (*tr_cam)[vertex_idx].z;
      // @@@ Note: originally 0.1 was used here. However this could cause
      // very large coordinates to be generated and our coverage line drawer
      // cannot currently cope with that. We need to improve that line
      // drawer considerably.
      if (tz <= 0.2)
      {
	max_depth = -1.0;
	if (do_cull_clampoccluder)
	{
	  if (j > 0)
	  {
	    // If j > 0 we know there was already one vertex which is in front
	    // of the Z=0.1 plane.
	    do_clamp = true;
	  }
	  else
	  {
	    // j == 0 so we still have to test if there are other vertices
	    // in front of the Z=0.1 plane.
	    // NOTE: we reuse the 'j' index here!!! This is not a bug.
	    for (++j ; j < num_verts ; j++)	// The start '++j' is not a bug!
	    {
	      vertex_idx = vi[j];
	      tz = (*tr_cam)[vertex_idx].z;
	      if (tz > 0.1)
	      {
	        do_clamp = true;
		break;
	      }
	    }
	  }
	}
	break;
      }
      if (tz > max_depth) max_depth = tz;
      verts2d[j] = (*tr_verts)[vertex_idx];
    }
    if (max_depth > 0.0)
    {
      int mod = tcovbuf->InsertPolygon (verts2d, (int)num_verts, max_depth,
      	occluder_box);
      modified += mod;
#     ifdef CS_DEBUG
      if (do_state_dump)
      {
        csPrintf ("  (not clamped) max_depth=%g ", max_depth);
        for (j = 0 ; j < num_verts ; j++)
	  csPrintf ("(%g,%g) ", verts2d[j].x, verts2d[j].y);
        csPrintf ("\n");
      }
#     endif
    }
    else if (do_clamp)
    {
      csPoly3D poly;
      for (j = 0 ; j < num_verts ; j++)
      {
	poly.AddVertex ((*tr_cam)[vi[j]]);
      }
      csPoly3D front, back;
      // @@@ Make specific version that doesn't fill 'front' version.
      poly.SplitWithPlaneZ (front, back, 0.1f);
      max_depth = -1.0;
      num_verts = back.GetVertexCount ();
      for (j = 0 ; j < num_verts ; j++)
      {
        const csVector3& v = back[j];
	if (v.z > max_depth) max_depth = v.z;
        Perspective (v, verts2d[j], fov, sx, sy);
      }

      int mod = tcovbuf->InsertPolygon (verts2d, (int)num_verts, max_depth,
      	occluder_box);
      modified += mod;
#     ifdef CS_DEBUG
      if (do_state_dump)
      {
        csPrintf ("  (clamped) max_depth=%g ", max_depth);
        for (j = 0 ; j < num_verts ; j++)
	  csPrintf ("(%g,%g) ", verts2d[j].x, verts2d[j].y);
        csPrintf ("\n");
      }
#     endif
    }
  }
  if (modified <= 0)
  {
    // This is not a good occluder. It didn't modify the coverage or depth
    // buffer at all. Disable this occluder for a while.
    obj->history->no_occluder_vis_cnt = history_frame_cnt + dist_nooccluder ();
  }
  else if (badoccluder_thresshold >= 0)
  {
    // Remember the amount of already occluded objects in all tiles modified
    // by this occluder.
    size_t idx = occluder_info.Length ();
    occluder_info.SetLength (idx+1);
    csOccluderInfo& occinfo = occluder_info[idx];
    occinfo.obj = obj;
    occinfo.occluder_box = occluder_box;
    occinfo.total_notoccluded = tcovbuf->CountNotCulledObjects (
    	occinfo.occluder_box);
  }

# ifdef CS_DEBUG
  if (do_state_dump)
  {
    csRef<iString> str = tcovbuf->Debug_Dump ();
    csPrintf ("%s\n", str->GetData ());
  }
# endif
}

void csDynaVis::UpdateCoverageBufferOutline (csVisibilityObjectWrapper* obj)
{
  iVisibilityObject* visobj = obj->visobj;
  csDynavisObjectModel* model = obj->model;
  iMovable* movable = visobj->GetMovable ();
  iPolygonMesh* polymesh = visobj->GetObjectModel ()->GetPolygonMeshViscull ();

  csVector3* verts = polymesh->GetVertices ();
  int vertex_count = polymesh->GetVertexCount ();

  csReversibleTransform trans = cam_trans;
  // Camera position in object space.
  csVector3 campos_object;
  if (obj->full_transform_identity)
  {
    campos_object = trans.GetOrigin ();
  }
  else
  {
    csReversibleTransform movtrans = movable->GetFullTransform ();
    campos_object = movtrans.Other2This (trans.GetOrigin ());
    trans /= movtrans;
  }

  model->UpdateOutline (campos_object);
  const csOutlineInfo& outline_info = model->GetOutlineInfo ();

# ifdef CS_DEBUG
  if (do_state_dump)
  {
    csRef<iObject> iobj (SCF_QUERY_INTERFACE (visobj, iObject));
    if (iobj)
    {
      csPrintf ("CovOutIns of object %s (max_depth=%g)\n",
      	iobj->GetName () ? iobj->GetName () : "<noname>",
	0.0);//max_depth);
    }
    csPrintf ("  campos_obj=%g,%g,%g\n",
    	campos_object.x, campos_object.y, campos_object.z);
  }
# endif

  // Then insert the outline.
  csBox2Int occluder_box;
  int modified = tcovbuf->InsertOutline (
  	trans, fov, sx, sy, verts, vertex_count,
  	outline_info.outline_verts,
  	outline_info.outline_edges, outline_info.num_outline_edges,
	do_cull_outline_splatting,
	occluder_box);
  if (modified <= 0)
  {
    // This is not a good occluder. It didn't modify the coverage or depth
    // buffer at all. Disable this occluder for a while.
    obj->history->no_occluder_vis_cnt = history_frame_cnt + dist_nooccluder ();
  }
  else if (badoccluder_thresshold >= 0)
  {
    // Remember the amount of already occluded objects in all tiles modified
    // by this occluder.
    size_t idx = occluder_info.Length ();
    occluder_info.SetLength (idx+1);
    csOccluderInfo& occinfo = occluder_info[idx];
    occinfo.obj = obj;
    occinfo.occluder_box = occluder_box;
    occinfo.total_notoccluded = tcovbuf->CountNotCulledObjects (
    	occinfo.occluder_box);
  }
# ifdef CS_DEBUG
  if (do_state_dump)
  {
    //csPrintf ("  max_depth=%g\n", max_depth);
    int j;
    for (j = 0 ; j < vertex_count ; j++)
    {
      if (outline_info.outline_verts[j])
      {
        csVector3 cam = trans.Other2This (verts[j]);
        csPrintf ("  V%d: (%g,%g,%g / %g,%g,%g)\n",
	  j,
	  //tr_verts[j].x, tr_verts[j].y,
	  verts[j].x, verts[j].y, verts[j].z,
	  cam.x, cam.y, cam.z);
      }
    }
    for (j = 0 ; j < outline_info.num_outline_edges ; j++)
    {
      int vt1 = outline_info.outline_edges[j*2+0];
      int vt2 = outline_info.outline_edges[j*2+1];
      csPrintf ("  E%d: %d-%d\n", j, vt1, vt2);
    }

    csRef<iString> str = tcovbuf->Debug_Dump ();
    csPrintf ("%s\n", str->GetData ());
  }
# endif
}

void csDynaVis::AppendWriteQueue (iVisibilityObject* visobj,
  	csDynavisObjectModel* /*model*/, csVisibilityObjectWrapper* obj,
	const csBox2& sbox, float min_depth, float max_depth)
{
  if (!obj->model->HasOBB ()) return;	// Object is not ment for writing.

  // Then append to queue.
  if (do_cull_ignoresmall)
    if ((sbox.MaxX ()-sbox.MinX ())<10 && (sbox.MaxY ()-sbox.MinY ())<10)
      return;

  float depth = max_depth;
  // If we have a good occluder we use the minimum depth instead
  // of the maximum depth to ensure that the object gets selected
  // soon enough (also note that good occluders are inserted polygon
  // by polygon in the coverage buffer).
  if (obj->hint_goodoccluder)
    depth = min_depth;

  write_queue->Append (sbox, depth, obj);
# ifdef CS_DEBUG
  if (do_state_dump)
  {
    csRef<iObject> iobj (SCF_QUERY_INTERFACE (visobj, iObject));
    if (iobj)
    {
      csPrintf (
	    "AppendWriteQueue of object %s (depth=%g) (good occluder=%d)\n",
      	    iobj->GetName () ? iobj->GetName () : "<noname>",
	    depth, (int)obj->hint_goodoccluder);
    }
  }
# endif
}

bool csDynaVis::TestWriteQueueRelevance (float min_depth,
	const csTestRectData& testrect_data, const csBox2& sbox)
{
  bool use_wq = false;
  csWriteQueueElement* el = write_queue->GetFirst ();
  if (el && el->depth <= min_depth)
  {
    // If there are potentially relevant items in the write queue
    // we first attempt to guess if those items can actually contribute
    // to culling (i.e. can mark the object invisible). To do that we
    // do a very quick and rough test where we see if the combined
    // rectangles from the write queue and the current contents of the
    // coverage buffer actually cover the entire rectangle we are
    // testing. For every element in the write queue we also set
    // the 'relevant' flag to true if the element is potentially
    // relevant.
    int tiles_remaining = tcovbuf->PrepareWriteQueueTest (
	  	testrect_data, min_depth);
    while (el)
    {
      if (el->depth > min_depth) break;
      if (el->box.TestIntersect (sbox))
      {
	csTestRectData td;
	tcovbuf->PrepareTestRectangle (el->box, td);
	int affecting_tiles = tcovbuf->AddWriteQueueTest (testrect_data,
	      	  td, el->relevant);
	if (tiles_remaining > 0)
	  tiles_remaining -= affecting_tiles;
      }
      else
      {
	el->relevant = false;
      }
      el = el->next;
    }
    if (tiles_remaining <= 0) use_wq = true;
  }
  return use_wq;
}

void csDynaVis::TestSinglePolygonVisibility (csVisibilityObjectWrapper* obj,
  	VisTest_Front2BackData* data, bool& vis,
	csBox2& sbox, float& min_depth, float& max_depth,
	uint32 frustum_mask)
{
  csVisibilityObjectHistory* hist = obj->history;
  iVisibilityObject* visobj = obj->visobj;

  iMovable* movable = visobj->GetMovable ();
  csReversibleTransform trans = cam_trans;
  if (!obj->full_transform_identity)
  {
    csReversibleTransform movtrans = movable->GetFullTransform ();
    trans /= movtrans;
  }

  iPolygonMesh* polymesh = visobj->GetObjectModel ()->GetPolygonMeshBase ();
  const csVector3* verts = polymesh->GetVertices ();
  csMeshedPolygon* poly = polymesh->GetPolygons ();
  int num_verts = poly->num_vertices;
  int* vi = poly->vertices;

  csVector2 v2d;
  csVector3 v = trans * verts[vi[0]];
  min_depth = v.z;
  max_depth = v.z;
  if (v.z < .1)
    PerspectiveWrong (v, v2d, fov, sx, sy);
  else
    Perspective (v, v2d, fov, sx, sy);
  sbox.StartBoundingBox (v2d);
  int i;
  for (i = 1 ; i < num_verts ; i++)
  {
    v = trans * verts[vi[i]];
    if (min_depth > v.z) min_depth = v.z;
    else if (max_depth < v.z) max_depth = v.z;
    if (v.z < .1)
      PerspectiveWrong (v, v2d, fov, sx, sy);
    else
      Perspective (v, v2d, fov, sx, sy);
    sbox.AddBoundingVertexSmart (v2d);
  }

  if (max_depth < .1 || sbox.MaxX () <= 0 || sbox.MaxY () <= 0 ||
        sbox.MinX () >= scr_width || sbox.MinY () >= scr_height)
  {
    obj->MarkInvisible (INVISIBLE_FRUSTUM);
    vis = false;
    return;
  }

  // If previously marked visible we stop here.
  if (vis) return;

  // Now assume that the object is visible. In the following code we
  // will attempt to prove that it is not visible.
  vis = true;

  if (do_cull_coverage != COVERAGE_NONE)
  {
    bool rc = false;

    if (do_cull_vpt)
    {
      csVector2 sbox_center = sbox.GetCenter ();
      rc = tcovbuf->TestPoint (sbox_center, max_depth);
      if (rc)
      {
	// The point is visible. If we have write queue enabled we
	// have to test further.
	if (do_cull_writequeue)
	  if (write_queue->IsPointAffected (sbox_center, max_depth))
	    rc = false;
      }

      if (rc)
      {
	// Point is visible. So we know the entire object is visible.
	obj->MarkVisible (VISIBLE_VPT, dist_history (), 0, current_vistest_nr,
	      	  history_frame_cnt);
	data->viscallback->ObjectVisible (obj->visobj, obj->mesh,
		frustum_mask);
	cnt_visible++;
	return;
      }
    }

    csTestRectData testrect_data;
#   ifdef CS_DEBUG
    if (do_state_dump)
    {
      csRef<iString> str = tcovbuf->Debug_Dump ();
      csPrintf ("Before single-poly test:\n%s\n", str->GetData ());
    }
#   endif
    bool mark_culled_object;
    rc = tcovbuf->PrepareTestRectangle (sbox, testrect_data);
    if (rc)
    {
      mark_culled_object = badoccluder_thresshold >= 0;
      rc = tcovbuf->TestRectangle (testrect_data, min_depth);
    }
    else
    {
      mark_culled_object = false;
    }

    if (rc)
    {
      // Object is visible. If we have a write queue we will first
      // test if there are objects in the queue that may mark the
      // object as non-visible.
      if (do_cull_writequeue &&
      	 hist->no_writequeue_vis_cnt <= history_frame_cnt)
      {
        bool use_wq = TestWriteQueueRelevance (min_depth, testrect_data, sbox);

	// If the write queue is enabled we try to see if there
	// are occluders that are relevant (intersect with this object
	// to test). We will insert those object with the coverage
	// buffer and test again.
	if (use_wq)
	{
	  float out_depth;
	  csVisibilityObjectWrapper* qobj = (csVisibilityObjectWrapper*)
	    	write_queue->Fetch (sbox, min_depth, out_depth);
	  if (qobj)
	  {
#           ifdef CS_DEBUG
	    if (do_state_dump)
	    {
	      csPrintf ("Adding objects from write queue!\n");
	      fflush (stdout);
	    }
#           endif
	    // We have found one such object. Insert them all.
	    do
	    {
	      // Yes! We found such an object. Insert it now.
	      UpdateCoverageBuffer (qobj);
	      // Now try again.
              rc = tcovbuf->TestRectangle (testrect_data, min_depth);
              if (!rc)
	      {
	        // It really is invisible.
	        obj->MarkInvisible (INVISIBLE_TESTRECT);
		if (mark_culled_object)
		  tcovbuf->MarkCulledObject (testrect_data);
	        vis = false;
                return;
	      }
	      qobj = (csVisibilityObjectWrapper*)
	    	    write_queue->Fetch (sbox, min_depth, out_depth);
	    }
	    while (qobj);
	  }
	}
      }
    }
    else
    {
      obj->MarkInvisible (INVISIBLE_TESTRECT);
      if (mark_culled_object)
        tcovbuf->MarkCulledObject (testrect_data);
      vis = false;
      return;
    }

    // If we come here we are visible. VPT tracking for single poly
    // objects is easy so we don't bother calculating a VPT point here.
  }

  //---------------------------------------------------------------
  // Object is visible so we should write it to the coverage buffer.
  obj->MarkVisible (VISIBLE, dist_history (), dist_nowritequeue (),
  	current_vistest_nr, history_frame_cnt);
  data->viscallback->ObjectVisible (obj->visobj, obj->mesh, frustum_mask);
  cnt_visible++;

  return;
}

bool csDynaVis::TestObjectVisibility (csVisibilityObjectWrapper* obj,
  	VisTest_Front2BackData* data, uint32 frustum_mask)
{
  // For coverage test.
  csBox2 sbox;
  float min_depth = 0;
  float max_depth;

  const csBox3& obj_bbox = obj->child->GetBBox ();
  const csVector3& pos = data->pos;

  csVisibilityObjectHistory* hist = obj->history;

  // If already marked visible this frame we don't do anything.
  if (obj->last_visible_vistestnr == current_vistest_nr)
    return true;

  if (obj->mesh && obj->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
    return false;

  iMovable* movable = obj->visobj->GetMovable ();
  csOBBFrozen frozen_obb;

  uint32 new_mask2 = frustum_mask;
  bool vis = false;
  // Before we do anything else we test history culling (if enabled)
  // and also if the position of the camera is inside the bounding box.
  // Finally we test view frustum culling.
  if (do_cull_frustum && !csIntersect3::BoxFrustum (obj_bbox,
	data->frustum, frustum_mask, new_mask2))
  {
    obj->MarkInvisible (INVISIBLE_FRUSTUM);
    goto end;
  }
  else if (do_cull_history && hist->vis_cnt >= history_frame_cnt)
  {
    obj->MarkVisibleForHistory (current_vistest_nr, history_frame_cnt);
    data->viscallback->ObjectVisible (obj->visobj, obj->mesh, new_mask2);
    cnt_visible++;
    vis = true;
  }
  else if (obj_bbox.Contains (pos))
  {
    obj->MarkVisible (VISIBLE_INSIDE, dist_history (), 0, current_vistest_nr,
		      history_frame_cnt);
    data->viscallback->ObjectVisible (obj->visobj, obj->mesh, new_mask2);
    cnt_visible++;
    vis = true;
  }

  // If we come here we know that either the object is already marked
  // visible because of history culling or inside-box test or else the
  // object is at least not invisible due to frustum culling. In this case
  // we calculate the screen bounding box since we know we are going to need
  // it both for additional visibility testing and also for the write queue.

  // If we have a single polygon object (which is usually a portal object)
  // then we use a special case which is computed more accuratelly and
  // more efficiently.
  if (obj->model->IsSinglePolygon ())
  {
    TestSinglePolygonVisibility (obj, data, vis,
    	sbox, min_depth, max_depth, new_mask2);
    goto end;
  }

  bool sbox_rc;	// Is screen box visible.
  sbox_rc = true;
  if (obj->model->HasOBB ())
  {
    csReversibleTransform trans = cam_trans;
    if (!obj->full_transform_identity)
    {
      csReversibleTransform movtrans = movable->GetFullTransform ();
      trans /= movtrans;
    }
    const csOBB& obb = obj->model->GetOBB ();
    frozen_obb.Copy (obb, trans);
    sbox_rc = frozen_obb.ProjectOBB (fov, sx, sy, sbox, min_depth, max_depth);
  }
  else
  {
    // No OBB, so use AABB instead.
    sbox_rc = obj_bbox.ProjectBox (cam_trans, fov, sx, sy, sbox,
		min_depth, max_depth);
  }
  if (!sbox_rc || sbox.MaxX () <= 0 || sbox.MaxY () <= 0 ||
        sbox.MinX () >= scr_width || sbox.MinY () >= scr_height)
  {
    obj->MarkInvisible (INVISIBLE_FRUSTUM);
    goto end;
  }

  // If previously marked visible we stop here.
  if (vis) goto end;
  // Now assume that the object is visible. In the following code we
  // will attempt to prove that it is not visible.
  vis = true;

  if (do_cull_coverage != COVERAGE_NONE)
  {
    bool rc = false;

    if (do_cull_vpt && hist->has_vpt_point)
    {
      csVector3 cam_center = cam_trans * hist->vpt_point;
      if (cam_center.z >= .1)
      {
	csVector2 sbox_center;
	Perspective (cam_center, sbox_center, fov, sx, sy);
	rc = tcovbuf->TestPoint (sbox_center, cam_center.z);
	if (rc)
	{
	  // The point is visible. If we have write queue enabled we
	  // have to test further.
	  if (do_cull_writequeue)
	    if (write_queue->IsPointAffected (sbox_center, cam_center.z))
	      rc = false;
	}

	if (rc)
	{
	  // Point is visible. So we know the entire object is visible.
	  obj->MarkVisible (VISIBLE_VPT, dist_history (), 0, current_vistest_nr,
	      	  history_frame_cnt);
	  data->viscallback->ObjectVisible (obj->visobj, obj->mesh, new_mask2);
	  cnt_visible++;
	  goto end;
	}
      }
    }

    csTestRectData testrect_data;
#   ifdef CS_DEBUG
    if (do_state_dump)
    {
      csRef<iString> str = tcovbuf->Debug_Dump ();
      csPrintf ("Before obj test:\n%s\n", str->GetData ());
    }
#   endif
    bool mark_culled_object;
    rc = tcovbuf->PrepareTestRectangle (sbox, testrect_data);
    if (rc)
    {
      mark_culled_object = badoccluder_thresshold >= 0;
      rc = tcovbuf->TestRectangle (testrect_data, min_depth);
    }
    else
    {
      mark_culled_object = false;
    }

    if (rc)
    {
      // Object is visible. If we have a write queue we will first
      // test if there are objects in the queue that may mark the
      // object as non-visible.
      if (do_cull_writequeue &&
      	 hist->no_writequeue_vis_cnt <= history_frame_cnt)
      {
        bool use_wq = TestWriteQueueRelevance (min_depth, testrect_data, sbox);

	// If the write queue is enabled we try to see if there
	// are occluders that are relevant (intersect with this object
	// to test). We will insert those object with the coverage
	// buffer and test again.
	if (use_wq)
	{
	  float out_depth;
	  csVisibilityObjectWrapper* qobj = (csVisibilityObjectWrapper*)
	    	write_queue->Fetch (sbox, min_depth, out_depth);
	  if (qobj)
	  {
#           ifdef CS_DEBUG
	    if (do_state_dump)
	    {
	      csPrintf ("Adding objects from write queue!\n");
	      fflush (stdout);
	    }
#           endif
	    // We have found one such object. Insert them all.
	    do
	    {
	      // Yes! We found such an object. Insert it now.
	      UpdateCoverageBuffer (qobj);
	      // Now try again.
              rc = tcovbuf->TestRectangle (testrect_data, min_depth);
              if (!rc)
	      {
	        // It really is invisible.
	        obj->MarkInvisible (INVISIBLE_TESTRECT);
		if (mark_culled_object)
		  tcovbuf->MarkCulledObject (testrect_data);
	        vis = false;
                goto end;
	      }
	      qobj = (csVisibilityObjectWrapper*)
	    	    write_queue->Fetch (sbox, min_depth, out_depth);
	    }
	    while (qobj);
	  }
	}
      }
    }
    else
    {
      obj->MarkInvisible (INVISIBLE_TESTRECT);
      if (mark_culled_object)
        tcovbuf->MarkCulledObject (testrect_data);
      vis = false;
      goto end;
    }

    // If we come here we are visible and we can update a point for
    // VPT tracking.
    if (do_cull_vpt)
    {
      hist->has_vpt_point = false;
      int i;
      // First we try to find a visible point on the OBB or AABB.
      if (obj->model->HasOBB ())
      {
	for (i = 0 ; i < 8 ; i++)
	{
	  csVector3 v = frozen_obb.GetCorner (i);
	  if (v.z >= .1)
	  {
	    csVector2 p;
	    Perspective (v, p, fov, sx, sy);
	    bool test = tcovbuf->TestPoint (p, v.z);
	    if (test)
	    {
	      hist->vpt_point = frozen_obb.GetCorner (i);
	      hist->has_vpt_point = true;
	      break;
	    }
	  }
	}
      }
      else
      {
	for (i = 0 ; i < 9 ; i++)	// Also include CS_BOX_CENTER3
	{
	  csVector3 v = cam_trans * obj_bbox.GetCorner (i);
	  if (v.z >= .1)
	  {
	    csVector2 p;
	    Perspective (v, p, fov, sx, sy);
	    bool test = tcovbuf->TestPoint (p, v.z);
	    if (test)
	    {
	      hist->vpt_point = obj_bbox.GetCorner (i);
	      hist->has_vpt_point = true;
	      break;
	    }
	  }
	}
      }
    }
  }

  //---------------------------------------------------------------
  // Object is visible so we should write it to the coverage buffer.
  obj->MarkVisible (VISIBLE, dist_history (), dist_nowritequeue (),
  	current_vistest_nr, history_frame_cnt);
  data->viscallback->ObjectVisible (obj->visobj, obj->mesh, new_mask2);
  cnt_visible++;

end:
  if (vis && do_cull_coverage != COVERAGE_NONE &&
  	obj->visobj->GetObjectModel ()->GetPolygonMeshViscull ())
  {
    if (!obj->hint_badoccluder)
    {
      // First test if this occluder is not marked as bad.
      if (badoccluder_retry)
        hist->no_occluder_vis_cnt = history_frame_cnt-1;

      if (!do_cull_ignore_bad_occluders ||
      	hist->no_occluder_vis_cnt <= history_frame_cnt)
      {
        // Object is visible.
        if (do_cull_writequeue)
        {
          // We are using the write queue so we insert the object there
          // for later culling.
          AppendWriteQueue (obj->visobj, obj->model,
      	    obj, sbox, min_depth, max_depth);
        }
        else
        {
          // Let it update the coverage buffer if we
          // are using cull_coverage.
          UpdateCoverageBuffer (obj);
        }
      }
    }
  }

# ifdef CS_DEBUG
  if (do_state_dump)
  {
    const csBox3& obj_bbox = obj->child->GetBBox ();
    csRef<iObject> iobj (SCF_QUERY_INTERFACE (obj->visobj, iObject));
    csPrintf ("Obj (%g,%g,%g)-(%g,%g,%g) (%s) %s\n",
    	obj_bbox.MinX (), obj_bbox.MinY (), obj_bbox.MinZ (),
    	obj_bbox.MaxX (), obj_bbox.MaxY (), obj_bbox.MaxZ (),
	(iobj && iobj->GetName ()) ? iobj->GetName () : "<noname>",
	hist->reason == INVISIBLE_FRUSTUM ? "outside frustum" :
	hist->reason == INVISIBLE_TESTRECT ? "covered" :
	hist->reason == VISIBLE_INSIDE ? "visible inside" :
	hist->reason == VISIBLE_HISTORY ? "visible history" :
	hist->reason == VISIBLE ? "visible" :
	hist->reason == VISIBLE_VPT ? "visible vpt" :
	"?"
	);
    if (hist->reason != INVISIBLE_FRUSTUM && hist->reason != VISIBLE_INSIDE
    	&& hist->reason != VISIBLE_HISTORY)
    {
      csPrintf ("  (%g,%g)-(%g,%g) min_depth=%g\n",
      	sbox.MinX (), sbox.MinY (),
      	sbox.MaxX (), sbox.MaxY (), min_depth);
    }
  }
# endif

  return vis;
}

//======== VisTest =========================================================

static bool VisTest_Front2Back (csKDTree* treenode, void* userdata,
	uint32 cur_timestamp, uint32& frustum_mask)
{
  VisTest_Front2BackData* data = (VisTest_Front2BackData*)userdata;
  csDynaVis* dynavis = data->dynavis;

  // In the first part of this test we are going to test if the node
  // itself is visible. If it is not then we don't need to continue.
  if (!dynavis->TestNodeVisibility (treenode, data, frustum_mask))
    return false;

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
      dynavis->TestObjectVisibility (visobj_wrap, data, frustum_mask);
    }
  }

  return true;
}

static bool Dummy_Front2Back (csKDTree* treenode, void*,
	uint32, uint32&)
{
  treenode->Distribute ();
  return true;
}

bool csDynaVis::VisTest (iRenderView* rview, 
			 iVisibilityCullerListener* viscallback)
{
  // We update the objects before testing the callback so that
  // we can use this VisTest() call to make sure the objects in the
  // culler are precached.
  UpdateObjects ();
  current_vistest_nr++;

  // just make sure we have a callback
  if (viscallback == 0)
  {
    // No callback. Assume we are precaching and pre-fetch the kdtree.
    kdtree->Front2Back (csVector3 (0), Dummy_Front2Back, 0, 0);
    return false;
  }

  cnt_visible = 0;
  cnt_node_visible = 0;

  // Statistics and debugging.
  debug_camera = rview->GetOriginalCamera ();
  float lx, rx, ty, by;
  rview->GetFrustum (lx, rx, ty, by);
  debug_lx = lx;
  debug_rx = rx;
  debug_ty = ty;
  debug_by = by;

  iCamera* camera = rview->GetCamera ();
  fov = float (camera->GetFOV ());
  sx = camera->GetShiftX ();
  sy = camera->GetShiftY ();
  int rb = reduce_buf;
  while (rb)
  {
    fov /= 2.0f;
    sx /= 2.0f;
    sy /= 2.0f;
    rb >>= 1;
  }
  csVector3 old_camera_pos = cam_trans.GetOrigin ();
  cam_trans = camera->GetTransform ();
  float sqdist = csSquaredDist::PointPoint (old_camera_pos,
  	cam_trans.GetOrigin ());
  history_frame_cnt++;	// Update for history culling.
  if (sqdist > .005f)
  {
    history_frame_cnt += 2;
    if (sqdist > 0.1f)
    {
      history_frame_cnt += 2;
      if (sqdist > 0.5f)
        history_frame_cnt += 5;
    }
  }

  // Test if we must retry all bad occluders.
  if (badoccluder_sweepcount <= history_frame_cnt)
  {
    badoccluder_sweepcount += badoccluder_maxsweepcount;
    badoccluder_retry = true;
  }
  else
  {
    badoccluder_retry = false;
  }

  // Just keep vis information from last frame.
  if (do_freeze_vis)
  {
    size_t i;
    for (i = 0 ; i < visobj_vector.Length () ; i++)
    {
      csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
      if (visobj_wrap->history->history_frame_cnt == history_frame_cnt-1)
      {
	visobj_wrap->history->history_frame_cnt = history_frame_cnt;
        viscallback->ObjectVisible (visobj_wrap->visobj, visobj_wrap->mesh,
		0xff);
      }
    }
    return true;
  }

  // Initialize the coverage buffer to all empty.
  tcovbuf->Initialize ();

  // Initialize the write queue to empty.
  write_queue->Initialize ();

  // Clear the list of occluders.
  occluder_info.SetLength (0);

  // If BugPlug is currently showing the debug sector we return here
  // so that all is marked invisible and rendering goes faster.
  if (bugplug && bugplug->CheckDebugSector ())
  {
    return true;
  }

  // Data for the vis tester.
  VisTest_Front2BackData data;

  csRenderContext* ctxt = rview->GetRenderContext ();
  data.frustum = ctxt->clip_planes;
  uint32 frustum_mask = ctxt->clip_planes_mask;

  // Using the last used portal, fill the coverage buffer with the
  // inverted portal outline to improve culling.
  if (do_insert_inverted_clipper)
  {
    iPortal* last_portal = rview->GetLastPortal ();
    if (last_portal)
    {
      iClipper2D* clipper = rview->GetClipper ();
      tcovbuf->InsertPolygonInverted (clipper->GetClipPoly (),
    	  clipper->GetVertexCount (), .01f);
    }
  }

# ifdef CS_DEBUG
  if (do_state_dump)
  {
    csPrintf ("=============================================================\n");
    uint32 mk = frustum_mask;
    int i;
    i = 0;
    while (mk)
    {
      csPrintf ("frustum %g,%g,%g,%g\n",
      	data.frustum[i].A (), data.frustum[i].B (),
      	data.frustum[i].C (), data.frustum[i].D ());
      i++;
      mk >>= 1;
    }
  }
# endif

  // The big routine: traverse from front to back and mark all objects
  // visible that are visible. In the mean time also update the coverage
  // buffer for further culling.
  data.pos = cam_trans.GetOrigin ();
  data.rview = rview;
  data.dynavis = this;
  data.viscallback = viscallback;
  kdtree->Front2Back (data.pos, VisTest_Front2Back, (void*)&data, frustum_mask);

  if (badoccluder_thresshold >= 0)
  {
    size_t i;
    for (i = 0 ; i < occluder_info.Length () ; i++)
    {
      csOccluderInfo& occinfo = occluder_info[i];
      // Get the total number of occluded objects on all tiles covered by
      // this occluder.
      int total_occluded = tcovbuf->CountNotCulledObjects (
      	occinfo.occluder_box);
      // Subtract the total number of occluded objects BEFORE this occluder
      // was used.
      total_occluded -= occinfo.total_notoccluded;
      // 'total_occluded' now contains the number of objects that were occluded
      // on all tiles covered by this occluder. This doesn't mean that this
      // occluder really helped occlude those objects but it is possible.
      if (total_occluded < badoccluder_thresshold)
        occinfo.obj->history->no_occluder_vis_cnt = history_frame_cnt
      	  + dist_nooccluder ();
    }
  }

  do_state_dump = false;

  return true;
}

//======== VisTest planes ==================================================

struct VisTestPlanes_Front2BackData
{
  uint32 current_vistest_nr;
  uint32 current_visnr;
  csArray<iVisibilityObject*>* vistest_objects;

  // During VisTest() we use the current frustum as five planes.
  // Associated with this frustum we also have a clip mask which
  // is maintained recursively during VisTest() and indicates the
  // planes that are still active for the current kd-tree node.
  csPlane3* frustum;

  iVisibilityCullerListener* viscallback;
};

static bool VisTestPlanes_Front2Back (csKDTree* treenode,
	void* userdata, uint32 cur_timestamp, uint32& frustum_mask)
{
  VisTestPlanes_Front2BackData* data
  	= (VisTestPlanes_Front2BackData*)userdata;

  // In the first part of this test we are going to test if the node
  // itself is visible. If it is not then we don't need to continue.
  const csBox3& node_bbox = treenode->GetNodeBBox ();
  uint32 new_mask;
  if (!csIntersect3::BoxFrustum (node_bbox, data->frustum, frustum_mask,
  	new_mask))
  {
    return false;
  }
  frustum_mask = new_mask;

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
      if (visobj_wrap->last_visible_vistestnr != data->current_vistest_nr)
      {
	const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();
	uint32 new_mask2;
	if (csIntersect3::BoxFrustum (obj_bbox, data->frustum,
		frustum_mask, new_mask2))
	{
	  if (data->viscallback)
	  {
	    data->viscallback->ObjectVisible (visobj_wrap->visobj,
	      visobj_wrap->mesh, new_mask2);
	  }
	  else
	  {
	    visobj_wrap->last_visible_vistestnr = data->current_vistest_nr;
	    data->vistest_objects->Push (visobj_wrap->visobj);
	  }
	}
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csDynaVis::VisTest (csPlane3* planes,
	int num_planes)
{
  UpdateObjects ();
  current_vistest_nr++;

  csArray<iVisibilityObject*>* v;
  if (vistest_objects_inuse)
  {
    // Vector is already in use by another iterator. Allocate a new vector
    // here.
    v = new csArray<iVisibilityObject*> ();
  }
  else
  {
    v = &vistest_objects;
    vistest_objects.DeleteAll ();
  }
  
  VisTestPlanes_Front2BackData data;
//  data.current_visnr = current_visnr;
  data.vistest_objects = v;
  data.frustum = planes;
  data.viscallback = 0;
  uint32 frustum_mask = (1 << num_planes)-1;

  kdtree->TraverseRandom (VisTestPlanes_Front2Back,
  	(void*)&data, frustum_mask);

  csDynVisObjIt* vobjit = new csDynVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

void csDynaVis::VisTest (csPlane3* planes, int num_planes,
			 iVisibilityCullerListener* viscallback)
{
  UpdateObjects ();
  current_vistest_nr++;

  VisTestPlanes_Front2BackData data;
//  data.current_visnr = current_visnr;
  data.frustum = planes;
  data.viscallback = viscallback;
  uint32 frustum_mask = (1 << num_planes)-1;

  kdtree->TraverseRandom (VisTestPlanes_Front2Back,
  	(void*)&data, frustum_mask);
}

//======== VisTest box =====================================================

struct VisTestBox_Front2BackData
{
  uint32 current_vistestnr;
  csBox3 box;
  csArray<iVisibilityObject*>* vistest_objects;
};

static bool VisTestBox_Front2Back (csKDTree* treenode, void* userdata,
	uint32 cur_timestamp, uint32&)
{
  VisTestBox_Front2BackData* data = (VisTestBox_Front2BackData*)userdata;

  // In the first part of this test we are going to test if the
  // box vector intersects with the node. If not then we don't
  // need to continue.
  const csBox3& node_bbox = treenode->GetNodeBBox ();
  if (!node_bbox.TestIntersect (data->box))
  {
    return false;
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

      // Test the bounding box of the object.
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();
      if (obj_bbox.TestIntersect (data->box))
      {
	visobj_wrap->last_visible_vistestnr = data->current_vistestnr;
	data->vistest_objects->Push (visobj_wrap->visobj);
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csDynaVis::VisTest (const csBox3& box)
{
  UpdateObjects ();
  current_vistest_nr++;

  csArray<iVisibilityObject*>* v;
  if (vistest_objects_inuse)
  {
    // Vector is already in use by another iterator. Allocate a new vector
    // here.
    v = new csArray<iVisibilityObject*> ();
  }
  else
  {
    v = &vistest_objects;
    vistest_objects.DeleteAll ();
  }

  VisTestBox_Front2BackData data;
  data.box = box;
  data.current_vistestnr = current_vistest_nr;
  data.vistest_objects = v;
  kdtree->Front2Back (box.GetCenter (), VisTestBox_Front2Back, (void*)&data,
  	0);

  csDynVisObjIt* vobjit = new csDynVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

//======== VisTest sphere ==================================================

struct VisTestSphere_Front2BackData
{
  uint32 current_vistestnr;
  csVector3 pos;
  float sqradius;
  csArray<iVisibilityObject*>* vistest_objects;

  iVisibilityCullerListener* viscallback;
};

static bool VisTestSphere_Front2Back (csKDTree* treenode, void* userdata,
	uint32 cur_timestamp, uint32&)
{
  VisTestSphere_Front2BackData* data = (VisTestSphere_Front2BackData*)userdata;

  // In the first part of this test we are going to test if the
  // box vector intersects with the node. If not then we don't
  // need to continue.
  const csBox3& node_bbox = treenode->GetNodeBBox ();
  if (!csIntersect3::BoxSphere (node_bbox, data->pos, data->sqradius))
  {
    return false;
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

      // Test the bounding box of the object.
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();
      if (csIntersect3::BoxSphere (obj_bbox, data->pos, data->sqradius))
      {
	if (data->viscallback)
	{
	  data->viscallback->ObjectVisible (visobj_wrap->visobj,
	  	visobj_wrap->mesh, 0xff);
	}
	else
	{
	  visobj_wrap->last_visible_vistestnr = data->current_vistestnr;
	  data->vistest_objects->Push (visobj_wrap->visobj);
	}
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csDynaVis::VisTest (const csSphere& sphere)
{
  UpdateObjects ();
  current_vistest_nr++;

  csArray<iVisibilityObject*>* v;
  if (vistest_objects_inuse)
  {
    // Vector is already in use by another iterator. Allocate a new vector
    // here.
    v = new csArray<iVisibilityObject*> ();
  }
  else
  {
    v = &vistest_objects;
    vistest_objects.DeleteAll ();
  }

  VisTestSphere_Front2BackData data;
  data.current_vistestnr = current_vistest_nr;
  data.vistest_objects = v;
  data.pos = sphere.GetCenter ();
  data.sqradius = sphere.GetRadius () * sphere.GetRadius ();
  data.viscallback = 0;
  kdtree->Front2Back (data.pos, VisTestSphere_Front2Back, (void*)&data, 0);

  csDynVisObjIt* vobjit = new csDynVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

void csDynaVis::VisTest (const csSphere& sphere, 
			 iVisibilityCullerListener* viscallback)
{
  UpdateObjects ();
  current_vistest_nr++;

  VisTestSphere_Front2BackData data;
  data.current_vistestnr = current_vistest_nr;
  data.viscallback = viscallback;
  data.pos = sphere.GetCenter ();
  data.sqradius = sphere.GetRadius () * sphere.GetRadius ();

  kdtree->Front2Back (data.pos, VisTestSphere_Front2Back, (void*)&data, 0);
}

//======== IntersectSegment ================================================

struct IntersectSegment_Front2BackData
{
  csSegment3 seg;
  csVector3 isect;
  float sqdist;		// Squared distance between seg.start and isect.
  float r;
  iMeshWrapper* mesh;
  int polygon_idx;
  csArray<iVisibilityObject*>* vector;	// If not-null we need all objects.
  bool accurate;
};

static bool IntersectSegmentSloppy_Front2Back (csKDTree* treenode,
	void* userdata, uint32 cur_timestamp, uint32&)
{
  IntersectSegment_Front2BackData* data
  	= (IntersectSegment_Front2BackData*)userdata;

  const csBox3& node_bbox = treenode->GetNodeBBox ();

  // In the first part of this test we are going to test if the
  // start-end vector intersects with the node. If not then we don't
  // need to continue.
  csVector3 box_isect;
  if (csIntersect3::BoxSegment (node_bbox, data->seg, box_isect) == -1)
  {
    return false;
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

      // First test the bounding box of the object.
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();

      if (csIntersect3::BoxSegment (obj_bbox, data->seg, box_isect) != -1)
      {
        // This object is possibly intersected by this beam.
	if (visobj_wrap->mesh)
	  if (!visobj_wrap->mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
	    data->vector->Push (visobj_wrap->visobj);
      }
    }
  }

  return true;
}

static bool IntersectSegment_Front2Back (csKDTree* treenode, void* userdata,
	uint32 cur_timestamp, uint32&)
{
  IntersectSegment_Front2BackData* data
  	= (IntersectSegment_Front2BackData*)userdata;

  const csBox3& node_bbox = treenode->GetNodeBBox ();

  // If mesh != 0 then we have already found our mesh. In that
  // case we will compare the distance of the origin with the the
  // box of the treenode and the already found shortest distance to
  // see if we have to proceed.
  if (data->mesh)
  {
    csBox3 b (node_bbox.Min ()-data->seg.Start (),
    	      node_bbox.Max ()-data->seg.Start ());
    if (b.SquaredOriginDist () > data->sqdist) return false;
  }

  // In the first part of this test we are going to test if the
  // start-end vector intersects with the node. If not then we don't
  // need to continue.
  csVector3 box_isect;
  if (csIntersect3::BoxSegment (node_bbox, data->seg, box_isect) == -1)
  {
    return false;
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

      // First test the bounding box of the object.
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();

      if (csIntersect3::BoxSegment (obj_bbox, data->seg, box_isect) != -1)
      {
        // This object is possibly intersected by this beam.
	if (visobj_wrap->mesh)
	{
	  if (!visobj_wrap->mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
	  {
	    // Transform our vector to object space.
	    csVector3 obj_start;
	    csVector3 obj_end;
	    bool identity = visobj_wrap->full_transform_identity;
	    csReversibleTransform movtrans;
	    if (identity)
	    {
	      obj_start = data->seg.Start ();
	      obj_end = data->seg.End ();
	    }
	    else
	    {
	      iMovable* movable = visobj_wrap->visobj->GetMovable ();
	      movtrans = movable->GetFullTransform ();
	      obj_start = movtrans.Other2This (data->seg.Start ());
	      obj_end = movtrans.Other2This (data->seg.End ());
	    }

	    csVector3 obj_isect;
	    float r;

	    bool rc;
	    int pidx = -1;
	    if (data->accurate)
	      rc = visobj_wrap->mesh->GetMeshObject ()->HitBeamObject (
	      	  obj_start, obj_end, obj_isect, &r, &pidx);
	    else
	      rc = visobj_wrap->mesh->GetMeshObject ()->HitBeamOutline (
	      	  obj_start, obj_end, obj_isect, &r);
	    if (rc)
	    {
	      if (data->vector)
	      {
		data->vector->Push (visobj_wrap->visobj);
	      }
	      else if (r < data->r)
	      {
		data->r = r;
		data->polygon_idx = pidx;
		if (identity)
		  data->isect = obj_isect;
		else
		  data->isect = movtrans.This2Other (obj_isect);
		data->sqdist = csSquaredDist::PointPoint (
			data->seg.Start (), data->isect);
		data->mesh = visobj_wrap->mesh;
	      }
	    }
	  }
	}
      }
    }
  }

  return true;
}

bool csDynaVis::IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr,
    iMeshWrapper** p_mesh, int* poly_idx, bool accurate)
{
  UpdateObjects ();
  current_vistest_nr++;

  IntersectSegment_Front2BackData data;
  data.seg.Set (start, end);
  data.sqdist = 10000000000.0;
  data.isect.Set (0, 0, 0);
  data.r = 10000000000.;
  data.mesh = 0;
  data.polygon_idx = -1;
  data.vector = 0;
  data.accurate = accurate;
  kdtree->Front2Back (start, IntersectSegment_Front2Back, (void*)&data, 0);

  if (p_mesh) *p_mesh = data.mesh;
  if (pr) *pr = data.r;
  if (poly_idx) *poly_idx = data.polygon_idx;
  isect = data.isect;

  return data.mesh != 0;
}

csPtr<iVisibilityObjectIterator> csDynaVis::IntersectSegment (
    const csVector3& start, const csVector3& end, bool accurate)
{
  UpdateObjects ();
  current_vistest_nr++;
  IntersectSegment_Front2BackData data;
  data.seg.Set (start, end);
  data.sqdist = 10000000000.0;
  data.r = 10000000000.;
  data.mesh = 0;
  data.polygon_idx = -1;
  data.vector = new csArray<iVisibilityObject*> ();
  data.accurate = accurate;
  kdtree->Front2Back (start, IntersectSegment_Front2Back, (void*)&data, 0);

  csDynVisObjIt* vobjit = new csDynVisObjIt (data.vector, 0);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

csPtr<iVisibilityObjectIterator> csDynaVis::IntersectSegmentSloppy (
    const csVector3& start, const csVector3& end)
{
  UpdateObjects ();
  current_vistest_nr++;
  IntersectSegment_Front2BackData data;
  data.seg.Set (start, end);
  data.vector = new csArray<iVisibilityObject*> ();
  kdtree->Front2Back (start, IntersectSegmentSloppy_Front2Back,
  	(void*)&data, 0);

  csDynVisObjIt* vobjit = new csDynVisObjIt (data.vector, 0);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

//======== CastShadows =====================================================

struct ShadObj
{
  float sqdist;
  iShadowCaster* caster;
  iMeshWrapper* mesh;
  iMovable* movable;
};

struct CastShadows_Front2BackData
{
  uint32 current_vistestnr;
  iFrustumView* fview;
  csPlane3 planes[32];
  ShadObj* shadobjs;
  int num_shadobjs;
};

static int compare_shadobj (const void* el1, const void* el2)
{
  ShadObj* m1 = (ShadObj*)el1;
  ShadObj* m2 = (ShadObj*)el2;
  if (m1->sqdist < m2->sqdist) return -1;
  if (m1->sqdist > m2->sqdist) return 1;
  return 0;
}

static bool CastShadows_Front2Back (csKDTree* treenode, void* userdata,
	uint32 cur_timestamp, uint32& planes_mask)
{
  CastShadows_Front2BackData* data = (CastShadows_Front2BackData*)userdata;

  iFrustumView* fview = data->fview;
  const csVector3& center = fview->GetFrustumContext ()->GetLightFrustum ()
    ->GetOrigin ();
  float sqrad = fview->GetSquaredRadius ();

  // First check the distance between the origin and the node box and see
  // if we are within the radius.
  const csBox3& node_bbox = treenode->GetNodeBBox ();
  csBox3 b (node_bbox.Min ()-center, node_bbox.Max ()-center);
  if (b.SquaredOriginDist () > sqrad)
    return false;

  // First we do frustum checking if relevant. See if the current node
  // intersects with the frustum.
  if (planes_mask)
  {
    const csBox3& node_bbox = treenode->GetNodeBBox ();
    uint32 out_mask;
    if (!csIntersect3::BoxFrustum (node_bbox, data->planes, planes_mask,
    	out_mask))
      return false;
    planes_mask = out_mask;
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
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();
      csBox3 b (obj_bbox.Min ()-center, obj_bbox.Max ()-center);
      if (b.SquaredOriginDist () > sqrad)
	continue;

      if (visobj_wrap->caster && fview->ThingShadowsEnabled () &&
            fview->CheckShadowMask (visobj_wrap->mesh->GetFlags ().Get ()))
      {
        data->shadobjs[data->num_shadobjs].sqdist = b.SquaredOriginDist ();
	data->shadobjs[data->num_shadobjs].caster = visobj_wrap->caster;
	data->shadobjs[data->num_shadobjs].mesh = 0;
	data->shadobjs[data->num_shadobjs].movable =
		visobj_wrap->visobj->GetMovable ();
	data->num_shadobjs++;
      }
      if (fview->CheckProcessMask (visobj_wrap->mesh->GetFlags ().Get ()))
      {
        data->shadobjs[data->num_shadobjs].sqdist = b.SquaredOriginMaxDist ();
	data->shadobjs[data->num_shadobjs].mesh = visobj_wrap->mesh;
	data->shadobjs[data->num_shadobjs].caster = 0;
	data->shadobjs[data->num_shadobjs].movable =
		visobj_wrap->visobj->GetMovable ();
	data->num_shadobjs++;
      }
    }
  }

  return true;
}


void csDynaVis::CastShadows (iFrustumView* fview)
{
  UpdateObjects ();
  current_vistest_nr++;
  CastShadows_Front2BackData data;
  data.current_vistestnr = current_vistest_nr;
  data.fview = fview;

  const csVector3& center = fview->GetFrustumContext ()->GetLightFrustum ()
    ->GetOrigin ();

  //======================================
  // First we find all relevant objects. For all these objects we add
  // both the shadow-caster as the receiver (as mesh) to the array (as
  // two different entries). The caster is added with the distance from
  // the light position to the nearest point on the bounding box while
  // the receiver is added with the distance from the light position to
  // the farthest point on the bounding box. Later on we can then traverse
  // the resulting list so that all relevant shadow casters are added before
  // the receivers are processed.
  //======================================

  data.shadobjs = new ShadObj [visobj_vector.Length () * 2];
  data.num_shadobjs = 0;

  // First check if we need to do frustum clipping.
  csFrustum* lf = fview->GetFrustumContext ()->GetLightFrustum ();
  uint32 planes_mask = 0;
  int i;
  // Traverse the kd-tree to find all relevant objects.
  // @@@ What if the frustum is bigger???
  CS_ASSERT (lf->GetVertexCount () <= 31);
  if (lf->GetVertexCount () > 31)
  {
    csPrintf ("INTERNAL ERROR! #vertices in GetVisibleObjects() exceeded!\n");
    fflush (stdout);
    return;
  }
  int i1 = lf->GetVertexCount () - 1;
  for (i = 0 ; i < lf->GetVertexCount () ; i1 = i, i++)
  {
    planes_mask = (planes_mask<<1)|1;
    const csVector3 &v1 = lf->GetVertex (i);
    const csVector3 &v2 = lf->GetVertex (i1);
    data.planes[i].Set (center, v1+center, v2+center);
  }
  if (lf->GetBackPlane ())
  {
    // @@@ UNTESTED CODE! There are no backplanes yet in frustums.
    // It is possible this plane has to be inverted.
    planes_mask = (planes_mask<<1)|1;
    data.planes[i] = *(lf->GetBackPlane ());
  }

  kdtree->Front2Back (center, CastShadows_Front2Back, (void*)&data,
  	planes_mask);

  // Now sort the list of shadow objects on radius.
  qsort (data.shadobjs, data.num_shadobjs, sizeof (ShadObj), compare_shadobj);

  // Mark a new region so that we can restore the shadows later.
  iShadowBlockList *shadows = fview->GetFrustumContext ()->GetShadows ();
  uint32 prev_region = shadows->MarkNewRegion ();

  // Now scan all objects and cast and receive shadows as appropriate. 
  ShadObj* so = data.shadobjs;
  for (i = 0 ; i < data.num_shadobjs ; i++)
  {
    if (so->caster) so->caster->AppendShadows (so->movable, shadows, center);
    if (so->mesh) fview->CallObjectFunction (so->mesh, true);
    so++;
  }
  delete[] data.shadobjs;

  // Restore the shadow list in 'fview' and then delete
  // all the shadow frustums that were added in this recursion
  // level.
  while (shadows->GetLastShadowBlock ())
  {
    iShadowBlock *sh = shadows->GetLastShadowBlock ();
    if (!shadows->FromCurrentRegion (sh))
      break;
    shadows->RemoveLastShadowBlock ();
    sh->DecRef ();
  }
  shadows->RestoreRegion (prev_region);
}

//======== Debugging =======================================================

csPtr<iString> csDynaVis::Debug_UnitTest ()
{
  csKDTree* kdtree = new csKDTree ();
  csRef<iDebugHelper> dbghelp (SCF_QUERY_INTERFACE (kdtree, iDebugHelper));
  if (dbghelp)
  {
    csRef<iString> rc (dbghelp->UnitTest ());
    if (rc)
    {
      kdtree->DecRef();
      return csPtr<iString> (rc);
    }
  }
  kdtree->DecRef();

  csTiledCoverageBuffer* tcovbuf = new csTiledCoverageBuffer (640, 480);
  dbghelp = SCF_QUERY_INTERFACE (tcovbuf, iDebugHelper);
  if (dbghelp)
  {
    csRef<iString> rc (dbghelp->UnitTest ());
    if (rc)
    {
      tcovbuf->DecRef();
      return csPtr<iString> (rc);
    }
  }
  tcovbuf->DecRef();

  return 0;
}

csPtr<iString> csDynaVis::Debug_StateTest ()
{
  return 0;
}

csPtr<iString> csDynaVis::Debug_Dump ()
{
  return 0;
}

struct color { int r, g, b; };
static color reason_colors[] =
{
  { 48, 48, 48 },
  { 64, 128, 64 },
  { 128, 64, 64 },
  { 255, 255, 255 },
  { 255, 255, 128 },
  { 255, 255, 196 },
  { 255, 128, 196 }
};

void csDynaVis::Debug_Dump (iGraphics3D* g3d)
{
  if (debug_camera)
  {
    iGraphics2D* g2d = g3d->GetDriver2D ();
    iFontServer* fntsvr = g2d->GetFontServer ();
    csRef<iFont> fnt;
    if (fntsvr)
    {
      fnt = fntsvr->LoadFont (CSFONT_COURIER);
    }

    g3d->BeginDraw (CSDRAW_2DGRAPHICS);
    int col_cam = g2d->FindRGB (0, 255, 0);
    int col_fgtext = g2d->FindRGB (0, 0, 0);
    int col_bgtext = g2d->FindRGB (255, 255, 255);

    if (cfg_view_mode == VIEWMODE_CLEARSTATSOVERLAY)
    {
      g2d->Clear (col_fgtext);
    }

    csString buf;
    buf.Format (
        "FR%c COV%c HIS%c WQ%c VPT%c IS%c CO%c OS%c IC%c BO%c #visobj=%d #visnode=%d",
        do_cull_frustum ? '+' : '-',
	do_cull_coverage == COVERAGE_OUTLINE ? 'o' :
	do_cull_coverage == COVERAGE_POLYGON ? 'p' :
	'-',
	do_cull_history ? '+' : '-',
	do_cull_writequeue ? '+' : '-',
	do_cull_vpt ? '+' : '-',
	do_cull_ignoresmall ? '+' : '-',
	do_cull_clampoccluder ? '+' : '-',
	do_cull_outline_splatting ? '+' : '-',
	do_insert_inverted_clipper ? '+' : '-',
	do_cull_ignore_bad_occluders ? '+' : '-',
    	cnt_visible, cnt_node_visible);
    g2d->Write (fnt, 10, 5, col_fgtext, col_bgtext, buf);

    if (cfg_view_mode == VIEWMODE_STATSOVERLAY
    	|| cfg_view_mode == VIEWMODE_CLEARSTATSOVERLAY)
    {
      size_t i;
      int reason_cols[LAST_REASON];
      for (i = 0 ; i < LAST_REASON ; i++)
      {
        reason_cols[i] = g2d->FindRGB (reason_colors[i].r,
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
        csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
        int col = reason_cols[visobj_wrap->history->reason];
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
      size_t i;
      for (i = 0 ; i < visobj_vector.Length () ; i++)
      {
	csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
	iVisibilityObject* visobj = visobj_wrap->visobj;
        if (visobj_wrap->last_visible_vistestnr == current_vistest_nr)
	{
	  // Only render outline if visible.
          const csReversibleTransform& camtrans = debug_camera->GetTransform();
	  iMovable* movable = visobj->GetMovable ();
	  csReversibleTransform movtrans = movable->GetFullTransform ();
	  csReversibleTransform trans = camtrans / movtrans;

	  csVector3 campos_object = movtrans.Other2This (camtrans.GetOrigin());
	  visobj_wrap->model->UpdateOutline (campos_object);
	  const csOutlineInfo& outline_info = visobj_wrap->model
	  	->GetOutlineInfo ();
	  iPolygonMesh* polymesh = visobj->GetObjectModel ()->
	  	GetPolygonMeshViscull ();
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
  }
}

class DynavisRenderObject : public iBugPlugRenderObject
{
private:
  csDynaVis* dynavis;
  csTiledCoverageBuffer* tcovbuf;
  struct outline
  {
    int num_edges;
    int* edges;
    float depth;
    int num_verts;
    csVector2* verts;
    csVector3* verts3d;
    int* verts_idx;
    bool* used_verts;
    ~outline ()
    {
      delete[] edges;
      delete[] verts;
      delete[] verts3d;
      delete[] verts_idx;
      delete[] used_verts;
    }
  };
  outline o1, o2, o3;
  int box_idx1;
  int box_idx2;

public:
  SCF_DECLARE_IBASE;

  DynavisRenderObject (csDynaVis* dynavis, iBugPlug* bugplug, int w, int h)
  {
    SCF_CONSTRUCT_IBASE (0);
    DynavisRenderObject::dynavis = dynavis;
    tcovbuf = new csTiledCoverageBuffer (w, h);

    int* e;

    int p1 = bugplug->DebugViewPoint (csVector2 (100, 100));
    int p2 = bugplug->DebugViewPoint (csVector2 (200, 110));
    int p3 = bugplug->DebugViewPoint (csVector2 (150, 200));
    int p4 = bugplug->DebugViewPoint (csVector2 (160, 150));
    int p5 = bugplug->DebugViewPoint (csVector2 (50, 50));
    bugplug->DebugViewLine (p1, p2);
    bugplug->DebugViewLine (p2, p3);
    bugplug->DebugViewLine (p3, p4);
    bugplug->DebugViewLine (p4, p5);
    bugplug->DebugViewLine (p5, p1);
    o1.num_edges = 5;
    o1.edges = new int[o1.num_edges*2];
    o1.num_verts = 5;
    o1.verts = new csVector2[o1.num_verts];
    o1.verts3d = new csVector3[o1.num_verts];
    o1.verts_idx = new int[o1.num_verts];
    o1.used_verts = new bool[o1.num_verts];
    o1.used_verts[0] = true;
    o1.used_verts[1] = true;
    o1.used_verts[2] = true;
    o1.used_verts[3] = true;
    o1.used_verts[4] = true;
    o1.verts_idx[0] = p1;
    o1.verts_idx[1] = p2;
    o1.verts_idx[2] = p3;
    o1.verts_idx[3] = p4;
    o1.verts_idx[4] = p5;
    e = o1.edges;
    *e++ = 0; *e++ = 1;
    *e++ = 1; *e++ = 2;
    *e++ = 2; *e++ = 3;
    *e++ = 3; *e++ = 4;
    *e++ = 4; *e++ = 0;
    o1.depth = 20;

    p1 = bugplug->DebugViewPoint (csVector2 (400, 100));
    p2 = bugplug->DebugViewPoint (csVector2 (500, 110));
    p3 = bugplug->DebugViewPoint (csVector2 (450, 200));
    p4 = bugplug->DebugViewPoint (csVector2 (460, 150));
    p5 = bugplug->DebugViewPoint (csVector2 (350, 50));
    bugplug->DebugViewLine (p1, p2);
    bugplug->DebugViewLine (p2, p3);
    bugplug->DebugViewLine (p3, p4);
    bugplug->DebugViewLine (p4, p5);
    bugplug->DebugViewLine (p5, p1);

    o2.num_edges = 5;
    o2.edges = new int[o2.num_edges*2];
    o2.num_verts = 5;
    o2.verts = new csVector2[o2.num_verts];
    o2.verts3d = new csVector3[o2.num_verts];
    o2.verts_idx = new int[o2.num_verts];
    o2.used_verts = new bool[o2.num_verts];
    o2.used_verts[0] = true;
    o2.used_verts[1] = true;
    o2.used_verts[2] = true;
    o2.used_verts[3] = true;
    o2.used_verts[4] = true;
    o2.verts_idx[0] = p1;
    o2.verts_idx[1] = p2;
    o2.verts_idx[2] = p3;
    o2.verts_idx[3] = p4;
    o2.verts_idx[4] = p5;
    e = o2.edges;
    *e++ = 0; *e++ = 1;
    *e++ = 1; *e++ = 2;
    *e++ = 2; *e++ = 3;
    *e++ = 3; *e++ = 4;
    *e++ = 4; *e++ = 0;
    o2.depth = 70;

    p1 = bugplug->DebugViewPoint (csVector2 (400, 300));
    p2 = bugplug->DebugViewPoint (csVector2 (500, 310));
    p3 = bugplug->DebugViewPoint (csVector2 (450, 400));
    p4 = bugplug->DebugViewPoint (csVector2 (460, 350));
    p5 = bugplug->DebugViewPoint (csVector2 (350, 250));
    bugplug->DebugViewLine (p1, p2);
    bugplug->DebugViewLine (p2, p3);
    bugplug->DebugViewLine (p3, p4);
    bugplug->DebugViewLine (p4, p5);
    bugplug->DebugViewLine (p5, p1);

    o3.num_edges = 5;
    o3.edges = new int[o3.num_edges*2];
    o3.num_verts = 5;
    o3.verts = new csVector2[o3.num_verts];
    o3.verts3d = new csVector3[o3.num_verts];
    o3.verts_idx = new int[o3.num_verts];
    o3.used_verts = new bool[o3.num_verts];
    o3.used_verts[0] = true;
    o3.used_verts[1] = true;
    o3.used_verts[2] = true;
    o3.used_verts[3] = true;
    o3.used_verts[4] = true;
    o3.verts_idx[0] = p1;
    o3.verts_idx[1] = p2;
    o3.verts_idx[2] = p3;
    o3.verts_idx[3] = p4;
    o3.verts_idx[4] = p5;
    e = o3.edges;
    *e++ = 0; *e++ = 1;
    *e++ = 1; *e++ = 2;
    *e++ = 2; *e++ = 3;
    *e++ = 3; *e++ = 4;
    *e++ = 4; *e++ = 0;
    o3.depth = 150;

    box_idx1 = bugplug->DebugViewPoint (csVector2 (10, 10));
    box_idx2 = bugplug->DebugViewPoint (csVector2 (200, 200));
    bugplug->DebugViewBox (box_idx1, box_idx2);
  }
  virtual ~DynavisRenderObject ()
  {
    delete tcovbuf;
    SCF_DESTRUCT_IBASE();
  }

  void RenderOutline (const outline& ol, iBugPlug* bugplug)
  {
    int i;
    for (i = 0 ; i < ol.num_verts ; i++)
    {
      ol.verts[i] = bugplug->DebugViewGetPoint (ol.verts_idx[i]);
      ol.verts3d[i].Set (ol.verts[i].x, ol.verts[i].y, ol.depth);
    }
    csReversibleTransform trans;
    csBox2Int modified_bbox;
    tcovbuf->InsertOutline (trans, ol.depth, 0.0, 0.0,
    	ol.verts3d, ol.num_verts, ol.used_verts,
    	ol.edges, ol.num_edges, false, modified_bbox);
  }

  virtual void Render (iGraphics3D* g3d, iBugPlug* bugplug)
  {
    tcovbuf->Initialize ();
    RenderOutline (o1, bugplug);
    RenderOutline (o2, bugplug);
    RenderOutline (o3, bugplug);
    csBox2 box;
    box.Set (bugplug->DebugViewGetPoint (box_idx1),
    	bugplug->DebugViewGetPoint (box_idx2));
    csTestRectData testrect_data;
    bool rc = tcovbuf->PrepareTestRectangle (box, testrect_data);
    if (rc) rc = tcovbuf->TestRectangle (testrect_data, 100);
    if (rc)
    {
      int colred = g3d->GetDriver2D ()->FindRGB (255, 0, 0);
      g3d->GetDriver2D ()->DrawBox (5, 5, 10, 10, colred);
    }
    tcovbuf->Debug_Dump (g3d);
  }
};

SCF_IMPLEMENT_IBASE (DynavisRenderObject)
  SCF_IMPLEMENTS_INTERFACE (iBugPlugRenderObject)
SCF_IMPLEMENT_IBASE_END

bool csDynaVis::Debug_DebugCommand (const char* cmd)
{
  if (!strcmp (cmd, "setup_debugsector"))
  {
    if (!bugplug)
      bugplug = CS_QUERY_REGISTRY (object_reg, iBugPlug);
    if (bugplug)
    {
      bugplug->SetupDebugSector ();
      size_t i;
      for (i = 0 ; i < visobj_vector.Length () ; i++)
      {
        csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
        iVisibilityObject* visobj = visobj_wrap->visobj;
	csBox3 box;
	iMovable* movable = visobj->GetMovable ();
	visobj_wrap->full_transform_identity =
		movable->IsFullTransformIdentity ();
	CalculateVisObjBBox (visobj, box, visobj_wrap->full_transform_identity);
	bugplug->DebugSectorBox (box,
		float (reason_colors[visobj_wrap->history->reason].r) / 256.0,
		float (reason_colors[visobj_wrap->history->reason].g) / 256.0,
		float (reason_colors[visobj_wrap->history->reason].b) / 256.0);
      }
      csVector3 origin (0);
      const csReversibleTransform& trans = debug_camera->GetTransform ();
      csVector3 topleft (debug_lx, debug_ty, 1); topleft *= 100.0;
      csVector3 topright (debug_rx, debug_ty, 1); topright *= 100.0;
      csVector3 botleft (debug_lx, debug_by, 1); botleft *= 100.0;
      csVector3 botright (debug_rx, debug_by, 1); botright *= 100.0;
      origin = trans.This2Other (origin);
      topleft = trans.This2Other (topleft);
      topright = trans.This2Other (topright);
      botleft = trans.This2Other (botleft);
      botright = trans.This2Other (botright);

      for (i = 0 ; i < 5 ; i++)
      {
	csVector3 v1, v2;
	float fact = float (i) / 5.0;
	v1 = topleft + fact * (topright-topleft);
	v2 = topleft + (fact+.1) * (topright-topleft);
        bugplug->DebugSectorTriangle (origin, v1, v2, .5, 0, 0);
	v1 = botright + fact * (topright-botright);
	v2 = botright + (fact+.1) * (topright-botright);
        bugplug->DebugSectorTriangle (origin, v1, v2, 0, .5, 0);
	v1 = topleft + fact * (botleft-topleft);
	v2 = topleft + (fact+.1) * (botleft-topleft);
        bugplug->DebugSectorTriangle (origin, v1, v2, 0, 0, .5);
	v1 = botleft + fact * (botright-botleft);
	v2 = botleft + (fact+.1) * (botright-botleft);
        bugplug->DebugSectorTriangle (origin, v1, v2, .5, .5, 0);
      }
      bugplug->SwitchDebugSector (trans);
    }
    else
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.dynavis", "BugPlug not found!");
    }
    return true;
  }
  else if (!strcmp (cmd, "setup_debugview"))
  {
    if (!bugplug)
      bugplug = CS_QUERY_REGISTRY (object_reg, iBugPlug);
    if (bugplug)
    {
      bugplug->SetupDebugView ();
      DynavisRenderObject* dro = new DynavisRenderObject (this, bugplug,
      	scr_width, scr_height);
      bugplug->DebugViewRenderObject (dro);
      dro->DecRef ();
      bugplug->SwitchDebugView ();
    }
    else
    {
      csReport(object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"BugPlug not found!");
    }
    return true;
  }
  else if (!strcmp (cmd, "toggle_ignorebadoccluder"))
  {
    do_cull_ignore_bad_occluders = !do_cull_ignore_bad_occluders;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s ignoring of bad occluders!",
	do_cull_clampoccluder ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_clampoccluder"))
  {
    do_cull_clampoccluder = !do_cull_clampoccluder;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s clamp occluders to Z=0!",
	do_cull_clampoccluder ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_ignoresmall"))
  {
    do_cull_ignoresmall = !do_cull_ignoresmall;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s ignore small objects!",
	do_cull_ignoresmall ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_frustum"))
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
  else if (!strcmp (cmd, "toggle_vpt"))
  {
    do_cull_vpt = !do_cull_vpt;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s vpt culling!", do_cull_vpt ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_splatting"))
  {
    do_cull_outline_splatting = !do_cull_outline_splatting;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s outline splatting!",
	do_cull_outline_splatting ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_invertedclipper"))
  {
    do_insert_inverted_clipper = !do_insert_inverted_clipper;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s inserting of inverted clipper!",
	do_insert_inverted_clipper ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_history"))
  {
    do_cull_history = !do_cull_history;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s history culling!", do_cull_history ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "toggle_freeze"))
  {
    do_freeze_vis = !do_freeze_vis;
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"%s visibility freezing!", do_freeze_vis ? "Enabled" : "Disabled");
    return true;
  }
  else if (!strcmp (cmd, "exact_freeze"))
  {
    do_freeze_vis = true;
    csExactCuller* excul = new csExactCuller (scr_width, scr_height);
    size_t i;
    for (i = 0 ; i < visobj_vector.Length () ; i++)
    {
      csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
      iVisibilityObject* visobj = visobj_wrap->visobj;
      iMovable* movable = visobj->GetMovable ();
      iPolygonMesh* polymesh = visobj->GetObjectModel ()
      	->GetPolygonMeshViscull ();
      if (polymesh)
        excul->AddObject (visobj_wrap, polymesh, movable, debug_camera,
  	  visobj_wrap->model->GetPlanes ());
    }
    excul->VisTest ();
    for (i = 0 ; i < visobj_vector.Length () ; i++)
    {
      csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
      iPolygonMesh* polymesh = visobj_wrap->visobj->GetObjectModel ()
      	->GetPolygonMeshViscull ();
      visobj_wrap->history->history_frame_cnt = 0;
      if (polymesh)
      {
        int vispix, totpix;
        excul->GetObjectStatus (visobj_wrap, vispix, totpix);
	if (vispix)
	{
	  visobj_wrap->last_visible_vistestnr = current_vistest_nr;
          visobj_wrap->history->history_frame_cnt = history_frame_cnt;
        }
      }
    }
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"Exact visibility freezing!");
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
	cfg_view_mode == VIEWMODE_CLEARSTATSOVERLAY ? "statistics and map(c)" :
	cfg_view_mode == VIEWMODE_OUTLINES ? "outlines" :
	"?");
    return true;
  }
  else if (!strcmp (cmd, "dump_state"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"Dumped current state.");
    do_state_dump = true;

    csRef<iDebugHelper> dbghelp (SCF_QUERY_INTERFACE (kdtree, iDebugHelper));
    if (dbghelp)
    {
      csRef<iString> rc (dbghelp->Dump ());
      csPrintf ("%s\n", rc->GetData ());
      fflush (stdout);
    }

    return true;
  }
  else if (!strcmp (cmd, "analyze_vis"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.dynavis",
    	"Analyze visibility status.");
    csExactCuller* excul = new csExactCuller (scr_width, scr_height);
    size_t i;
    for (i = 0 ; i < visobj_vector.Length () ; i++)
    {
      csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
      iVisibilityObject* visobj = visobj_wrap->visobj;
      iMovable* movable = visobj->GetMovable ();
      iPolygonMesh* polymesh = visobj->GetObjectModel ()
      	->GetPolygonMeshViscull ();
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
      csVisibilityObjectWrapper* visobj_wrap = visobj_vector[i];
      iPolygonMesh* polymesh = visobj_wrap->visobj->GetObjectModel ()
      	->GetPolygonMeshViscull ();
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
	if (visobj_wrap->history->reason >= VISIBLE)
	{
	  tot_vis_dynavis++;
	  tot_poly_dynavis += polymesh->GetPolygonCount ();
	}

        csRef<iObject> iobj (
		SCF_QUERY_INTERFACE (visobj_wrap->visobj, iObject));
        csPrintf ("  obj(%zu,'%s')  vis=%s   vispix=%d totpix=%d      %s\n",
      	  i,
	  (iobj && iobj->GetName ()) ? iobj->GetName () : "?",
	  visobj_wrap->history->reason == INVISIBLE_PARENT ? "invis parent" :
	  visobj_wrap->history->reason == INVISIBLE_FRUSTUM ? "invis frustum" :
	  visobj_wrap->history->reason == INVISIBLE_TESTRECT?"invis testrect" :
	  visobj_wrap->history->reason == VISIBLE ? "vis" :
	  visobj_wrap->history->reason == VISIBLE_INSIDE ? "vis inside" :
	  visobj_wrap->history->reason == VISIBLE_HISTORY ? "vis history" :
	  visobj_wrap->history->reason == VISIBLE_VPT ? "vis vpt" :
	  "?",
	  vispix, totpix,
	  vispix != 0 && visobj_wrap->history->reason < VISIBLE
	  	? "????" : "");
      }
    }
    csPrintf ("Summary: #objects=%d #vis(exact)=%d #vis(dynavis)=%d\n",
    	tot_objects, tot_vis_exact, tot_vis_dynavis);
    csPrintf ("Summary: #poly=%d #poly(exact)=%d #poly(dynavis)=%d\n",
    	tot_poly, tot_poly_exact, tot_poly_dynavis);
    fflush (stdout);

    delete excul;
    return true;
  }
  return false;
}

csTicks csDynaVis::Debug_Benchmark (int num_iterations)
{
  csTicks rc = 0;

  csKDTree* kdtree = new csKDTree ();
  csRef<iDebugHelper> dbghelp (SCF_QUERY_INTERFACE (kdtree, iDebugHelper));
  if (dbghelp)
  {
    csTicks r = dbghelp->Benchmark (num_iterations);
    csPrintf ("kdtree:   %zu ms\n", r);
    rc += r;
  }
  delete kdtree;

  return rc;
}
