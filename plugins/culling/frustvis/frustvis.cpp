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
#include "csgeom/frustum.h"
#include "csgeom/matrix3.h"
#include "csgeom/math3d.h"
#include "csgeom/obb.h"
#include "csgeom/segment.h"
#include "csgeom/sphere.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"
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
#include "imesh/thing/thing.h"
#include "iutil/object.h"
#include "ivaria/reporter.h"
#include "frustvis.h"
#include "fkdtree.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csFrustumVis)

SCF_EXPORT_CLASS_TABLE (frustvis)
  SCF_EXPORT_CLASS (csFrustumVis, "crystalspace.culling.frustvis",
    "Simple Frustum Visibility System")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csFrustumVis)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityCuller)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFrustumVis::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

//----------------------------------------------------------------------

class csFrustVisObjIt : public iVisibilityObjectIterator
{
private:
  csVector* vector;
  int position;

public:
  SCF_DECLARE_IBASE;

  csFrustVisObjIt (csVector* vector)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csFrustVisObjIt::vector = vector;
    Reset ();
  }
  virtual ~csFrustVisObjIt ()
  {
  }

  virtual bool Next()
  {
    if (position < 0) return false;
    position++;
    if (position == vector->Length ())
    {
      position = -1;
      return false;
    }
    return true;
  }

  virtual void Reset()
  {
    if (vector == NULL || vector->Length () < 1)
      position = -1;
    else
      position = 0;
  }

  virtual iVisibilityObject* GetObject () const
  {
    return (iVisibilityObject*)(vector->Get (position));
  }
  virtual bool IsFinished () const
  {
    return (position < 0);
  }
};

SCF_IMPLEMENT_IBASE (csFrustVisObjIt)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityObjectIterator)
SCF_IMPLEMENT_IBASE_END

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csFrustVisObjectWrapper)
  SCF_IMPLEMENTS_INTERFACE (iObjectModelListener)
  SCF_IMPLEMENTS_INTERFACE (iMovableListener)
SCF_IMPLEMENT_IBASE_END

void csFrustVisObjectWrapper::ObjectModelChanged (iObjectModel* /*model*/)
{
  frustvis->UpdateObject (this);
}

void csFrustVisObjectWrapper::MovableChanged (iMovable* /*movable*/)
{
  frustvis->UpdateObject (this);
}

//----------------------------------------------------------------------

csFrustumVis::csFrustumVis (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  object_reg = NULL;
  kdtree = NULL;
  current_visnr = 1;
}

csFrustumVis::~csFrustumVis ()
{
  int i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
    	visobj_vector[i];
    visobj_wrap->visobj->DecRef ();
    delete visobj_wrap;
  }
  delete kdtree;
}

bool csFrustumVis::Initialize (iObjectRegistry *object_reg)
{
  csFrustumVis::object_reg = object_reg;

  delete kdtree;

  csRef<iGraphics3D> g3d (CS_QUERY_REGISTRY (object_reg, iGraphics3D));
  if (g3d)
  {
    scr_width = g3d->GetWidth ();
    scr_height = g3d->GetHeight ();
  }
  else
  {
    // If there is no g3d we currently assume we are testing.
    scr_width = 640;
    scr_height = 480;
  }

  kdtree = new csSimpleKDTree (NULL);

  return true;
}

void csFrustumVis::Setup (const char* /*name*/)
{
}

void csFrustumVis::CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox)
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

void csFrustumVis::RegisterVisObject (iVisibilityObject* visobj)
{
  csFrustVisObjectWrapper* visobj_wrap = new csFrustVisObjectWrapper (
		  this);
  visobj_wrap->visobj = visobj;
  visobj->IncRef ();
  iMovable* movable = visobj->GetMovable ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();
  visobj_wrap->shape_number = visobj->GetObjectModel ()->GetShapeNumber ();

  csBox3 bbox;
  CalculateVisObjBBox (visobj, bbox);
  visobj_wrap->child = kdtree->AddObject (bbox, (void*)visobj_wrap);

  csRef<iMeshWrapper> mesh (SCF_QUERY_INTERFACE (visobj, iMeshWrapper));
  visobj_wrap->mesh = mesh;
  if (mesh)
  {
    visobj_wrap->caster = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
    	iShadowCaster);
    visobj_wrap->receiver = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
    	iShadowReceiver);
    visobj_wrap->thing_state = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
	iThingState);
  }

  // Only add the listeners at the very last moment to prevent them to
  // be called by the calls above (i.e. especially the calculation of
  // the bounding box could cause a listener to be fired).
  movable->AddListener ((iMovableListener*)visobj_wrap);
  visobj->GetObjectModel ()->AddListener (
		  (iObjectModelListener*)visobj_wrap);

  visobj_vector.Push (visobj_wrap);
}

void csFrustumVis::UnregisterVisObject (iVisibilityObject* visobj)
{
  int i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      visobj_vector[i];
    if (visobj_wrap->visobj == visobj)
    {
      visobj->GetMovable ()->RemoveListener (
		  (iMovableListener*)visobj_wrap);
      visobj->GetObjectModel ()->RemoveListener (
		  (iObjectModelListener*)visobj_wrap);
      kdtree->RemoveObject (visobj_wrap->child);
      visobj->DecRef ();
      delete visobj_wrap;
      visobj_vector.Delete (i);
      return;
    }
  }
}

void csFrustumVis::UpdateObject (csFrustVisObjectWrapper* visobj_wrap)
{
  iVisibilityObject* visobj = visobj_wrap->visobj;
  iMovable* movable = visobj->GetMovable ();
  csBox3 bbox;
  CalculateVisObjBBox (visobj, bbox);
  kdtree->MoveObject (visobj_wrap->child, bbox);
  visobj_wrap->shape_number = visobj->GetObjectModel ()->GetShapeNumber ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();
}

struct FrustTest_Front2BackData
{
  csVector3 pos;
  iRenderView* rview;
  csFrustumVis* frustvis;

  // During VisTest() we use the current frustum as five planes.
  // Associated with this frustum we also have a clip mask which
  // is maintained recursively during VisTest() and indicates the
  // planes that are still active for the current kd-tree node.
  csPlane3 frustum[32];
  uint32 frustum_mask;
};

bool csFrustumVis::TestNodeVisibility (csSimpleKDTree* treenode,
	FrustTest_Front2BackData* data)
{
  const csBox3& node_bbox = treenode->GetNodeBBox ();

  if (node_bbox.Contains (data->pos))
  {
    return true;
  }

  uint32 new_mask;
  if (!csIntersect3::BoxFrustum (node_bbox, data->frustum, data->frustum_mask,
  	new_mask))
  {
    return false;
  }
  data->frustum_mask = new_mask;
  return true;
}

bool csFrustumVis::TestObjectVisibility (csFrustVisObjectWrapper* obj,
  	FrustTest_Front2BackData* data)
{
  if (obj->visobj->GetVisibilityNumber () != current_visnr)
  {
    const csBox3& obj_bbox = obj->child->GetBBox ();
    if (obj_bbox.Contains (data->pos))
    {
      obj->visobj->SetVisibilityNumber (current_visnr);
      return true;
    }
  
    uint32 new_mask;
    if (!csIntersect3::BoxFrustum (obj_bbox, data->frustum,
		data->frustum_mask, new_mask))
    {
      return false;
    }

    obj->visobj->SetVisibilityNumber (current_visnr);
  }

  return true;
}

//======== VisTest =========================================================

static bool FrustTest_Front2Back (csSimpleKDTree* treenode, void* userdata,
	uint32 cur_timestamp)
{
  FrustTest_Front2BackData* data = (FrustTest_Front2BackData*)userdata;
  csFrustumVis* frustvis = data->frustvis;

  // Visible or not...
  bool vis = false;

  // Remember current frustum mask.
  uint32 old_frustum_mask = data->frustum_mask;

  // In the first part of this test we are going to test if the node
  // itself is visible. If it is not then we don't need to continue.
  if (!frustvis->TestNodeVisibility (treenode, data))
  {
    vis = false;
    goto end;
  }

  treenode->Distribute ();

  int num_objects;
  csSimpleKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	objects[i]->GetObject ();
      frustvis->TestObjectVisibility (visobj_wrap, data);
    }
  }

  vis = true;

end:
  // Restore the frustum mask.
  data->frustum_mask = old_frustum_mask;

  return vis;
}

bool csFrustumVis::VisTest (iRenderView* rview)
{
  current_visnr++;

{
// @@@ Temporariy work around until a bug is fixed with the kdtree
// and moving objects!
  int i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      visobj_vector[i];
    iVisibilityObject* visobj = visobj_wrap->visobj;
    visobj->SetVisibilityNumber (current_visnr);
  }

return true;
}

  // Data for the vis tester.
  FrustTest_Front2BackData data;

  // First get the current view frustum from the rview.
  float lx, rx, ty, by;
  rview->GetFrustum (lx, rx, ty, by);
  csVector3 p0 (lx, by, 1);
  csVector3 p1 (lx, ty, 1);
  csVector3 p2 (rx, ty, 1);
  csVector3 p3 (rx, by, 1);
  const csReversibleTransform& trans = rview->GetCamera ()->GetTransform ();
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
  if (rview->GetCamera ()->IsMirrored ())
  {
    data.frustum[0].Invert ();
    data.frustum[1].Invert ();
    data.frustum[2].Invert ();
    data.frustum[3].Invert ();
  }

  // The big routine: traverse from front to back and mark all objects
  // visible that are visible.
  data.pos = rview->GetCamera ()->GetTransform ().GetOrigin ();
  data.rview = rview;
  data.frustvis = this;
  kdtree->Front2Back (data.pos, FrustTest_Front2Back, (void*)&data);

  return true;
}

//======== VisTest box =====================================================

struct FrustTestBox_Front2BackData
{
  uint32 current_visnr;
  csBox3 box;
  csVector* vistest_objects;
};

static bool FrustTestBox_Front2Back (csSimpleKDTree* treenode, void* userdata,
	uint32 cur_timestamp)
{
  FrustTestBox_Front2BackData* data = (FrustTestBox_Front2BackData*)userdata;

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
  csSimpleKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	objects[i]->GetObject ();

      // Test the bounding box of the object.
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();
      if (obj_bbox.TestIntersect (data->box))
      {
	visobj_wrap->visobj->SetVisibilityNumber (data->current_visnr);
	data->vistest_objects->Push (visobj_wrap->visobj);
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csFrustumVis::VisTest (const csBox3& box)
{
  current_visnr++;
  vistest_objects.DeleteAll ();
  FrustTestBox_Front2BackData data;
  data.current_visnr = current_visnr;
  data.box = box;
  data.vistest_objects = &vistest_objects;
  kdtree->Front2Back (box.GetCenter (), FrustTestBox_Front2Back, (void*)&data);

  csRef<iVisibilityObjectIterator> visit = csPtr<iVisibilityObjectIterator> (
  	(iVisibilityObjectIterator*)(new csFrustVisObjIt (&vistest_objects)));
  return csPtr<iVisibilityObjectIterator> (visit);
}

//======== VisTest sphere ==================================================

struct FrustTestSphere_Front2BackData
{
  uint32 current_visnr;
  csVector3 pos;
  float sqradius;
  csVector* vistest_objects;
};

static bool FrustTestSphere_Front2Back (csSimpleKDTree* treenode,
	void* userdata, uint32 cur_timestamp)
{
  FrustTestSphere_Front2BackData* data =
  	(FrustTestSphere_Front2BackData*)userdata;

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
  csSimpleKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	objects[i]->GetObject ();

      // Test the bounding box of the object.
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();
      if (csIntersect3::BoxSphere (obj_bbox, data->pos, data->sqradius))
      {
	visobj_wrap->visobj->SetVisibilityNumber (data->current_visnr);
	data->vistest_objects->Push (visobj_wrap->visobj);
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csFrustumVis::VisTest (const csSphere& sphere)
{
  current_visnr++;
  vistest_objects.DeleteAll ();
  FrustTestSphere_Front2BackData data;
  data.current_visnr = current_visnr;
  data.pos = sphere.GetCenter ();
  data.sqradius = sphere.GetRadius () * sphere.GetRadius ();
  data.vistest_objects = &vistest_objects;
  kdtree->Front2Back (data.pos, FrustTestSphere_Front2Back, (void*)&data);

  csRef<iVisibilityObjectIterator> visit = csPtr<iVisibilityObjectIterator> (
  	(iVisibilityObjectIterator*)(new csFrustVisObjIt (&vistest_objects)));
  return csPtr<iVisibilityObjectIterator> (visit);
}

//======== IntersectSegment ================================================

struct IntersectSegment_Front2BackData
{
  csSegment3 seg;
  csVector3 isect;
  float sqdist;	// squared distance between seg.start and isect.
  float r;
  iMeshWrapper* mesh;
  iPolygon3D* polygon;
};

static bool IntersectSegment_Front2Back (csSimpleKDTree* treenode,
	void* userdata, uint32 cur_timestamp)
{
  IntersectSegment_Front2BackData* data
  	= (IntersectSegment_Front2BackData*)userdata;

  const csBox3& node_bbox = treenode->GetNodeBBox ();

  // If mesh != NULL then we have already found our mesh. In that
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
  csSimpleKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  data->r = 10000000000.;
  data->polygon = NULL;

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	objects[i]->GetObject ();

      // First test the bounding box of the object.
      const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();

      if (csIntersect3::BoxSegment (obj_bbox, data->seg, box_isect) != -1)
      {
        // This object is possibly intersected by this beam.
	if (visobj_wrap->mesh)
	{
	  if (!visobj_wrap->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLE))
	  {
	    // Transform our vector to object space.
	    //@@@ Consider the ability to check if
	    // object==world space for objects in general?
	    csReversibleTransform movtrans (visobj_wrap->visobj->
		  GetMovable ()->GetFullTransform ());
	    csVector3 obj_start = movtrans.Other2This (data->seg.Start ());
	    csVector3 obj_end = movtrans.Other2This (data->seg.End ());
	    csVector3 obj_isect;
	    float r;

	    if (visobj_wrap->thing_state)
	    {
	      iThingState* st = visobj_wrap->thing_state;
	      iPolygon3D* p = st->IntersectSegment (
			obj_start, obj_end,
			obj_isect, &r, false);
	      if (p && r < data->r)
	      {
		data->r = r;
		data->polygon = p;
		data->isect = movtrans.This2Other (obj_isect);
		data->sqdist = csSquaredDist::PointPoint (
			data->seg.Start (), data->isect);
		data->mesh = visobj_wrap->mesh;
	      }
	    }
	    else
	    {
	      if (visobj_wrap->mesh->GetMeshObject ()->HitBeamOutline (obj_start,
	      	obj_end, obj_isect, &r))
	      {
	        if (r < data->r)
		{
		  data->r = r;
		  data->polygon = NULL;
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
  }
  return true;
}

bool csFrustumVis::IntersectSegment (const csVector3& start,
    const csVector3& end, csVector3& isect, float* pr,
    iMeshWrapper** p_mesh, iPolygon3D** poly)
{
  current_visnr++;
  IntersectSegment_Front2BackData data;
  data.seg.Set (start, end);
  data.r = 0;
  data.mesh = NULL;
  data.polygon = NULL;
  kdtree->Front2Back (start, IntersectSegment_Front2Back, (void*)&data);

  if (p_mesh) *p_mesh = data.mesh;
  if (pr) *pr = data.r;
  if (poly) *poly = data.polygon;
  isect = data.isect;

  return data.mesh != NULL;
}

//======== CastShadows =====================================================

struct CastShadows_Front2BackData
{
  uint32 current_visnr;
  iFrustumView* fview;
  csPlane3 planes[32];
  uint32 planes_mask;
};

static bool CastShadows_Front2Back (csSimpleKDTree* treenode, void* userdata,
	uint32 cur_timestamp)
{
  CastShadows_Front2BackData* data = (CastShadows_Front2BackData*)userdata;

  // First we do frustum checking if relevant. See if the current node
  // intersects with the frustum.
  if (data->planes_mask)
  {
    const csBox3& node_bbox = treenode->GetNodeBBox ();
    uint32 out_mask;
    if (!csIntersect3::BoxFrustum (node_bbox, data->planes, data->planes_mask,
    	out_mask))
      return false;
  }

  treenode->Distribute ();

  int num_objects;
  csSimpleKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  iFrustumView* fview = data->fview;
  const csVector3& center = fview->GetFrustumContext ()->GetLightFrustum ()
    ->GetOrigin ();
  iShadowBlockList *shadows = fview->GetFrustumContext ()->GetShadows ();

  int i;
  // The first time through the loop we just append shadows.
  // We also don't mark the timestamp yet so that we can easily
  // go a second time through the loop.
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	objects[i]->GetObject ();

      // Test the bounding box of the object with the frustum.
      bool vis = false;
      if (data->planes_mask)
      {
        const csBox3& obj_bbox = visobj_wrap->child->GetBBox ();
	uint32 out_mask;
	if (csIntersect3::BoxFrustum (obj_bbox, data->planes, data->planes_mask,
		out_mask))
	{
	  vis = true;
	}
      }
      else
      {
        vis = true;
      }
      // If visible we mark as visible and add shadows if possible.
      if (vis)
      {
	visobj_wrap->visobj->SetVisibilityNumber (data->current_visnr);
        if (visobj_wrap->caster && fview->ThingShadowsEnabled () &&
            fview->CheckShadowMask (visobj_wrap->mesh->GetFlags ().Get ()))
        {
          visobj_wrap->caster->AppendShadows (
	  	visobj_wrap->visobj->GetMovable (), shadows, center);
	}
      }
    }
  }
  // Here we go a second time through the loop. Here we will
  // actually send shadows to the receivers.
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	objects[i]->GetObject ();

      // If visible we mark as visible and add shadows if possible.
      if (visobj_wrap->visobj->GetVisibilityNumber () == data->current_visnr)
      {
        if (visobj_wrap->receiver
		&& fview->CheckProcessMask (
			visobj_wrap->mesh->GetFlags ().Get ()))
        {
          visobj_wrap->receiver->CastShadows (
	  	visobj_wrap->visobj->GetMovable (), fview);
	}
      }
    }
  }

  return true;
}

void csFrustumVis::CastShadows (iFrustumView* fview)
{
  current_visnr++;
  CastShadows_Front2BackData data;
  data.current_visnr = current_visnr;
  data.fview = fview;

  const csVector3& center = fview->GetFrustumContext ()->GetLightFrustum ()
    ->GetOrigin ();

  // First check if we need to do frustum clipping.
  csFrustum* lf = fview->GetFrustumContext ()->GetLightFrustum ();
  data.planes_mask = 0;
  bool infinite = lf->IsInfinite ();
  if (!infinite)
  {
    // @@@ What if the frustum is bigger???
    CS_ASSERT (lf->GetVertexCount () <= 31);
    if (lf->GetVertexCount () > 31)
    {
      printf ("INTERNAL ERROR! #vertices in GetVisibleObjects() exceeded!\n");
      fflush (stdout);
      return;
    }
    int i;
    int i1 = lf->GetVertexCount () - 1;
    for (i = 0 ; i < lf->GetVertexCount () ; i1 = i, i++)
    {
      data.planes_mask = (data.planes_mask<<1)|1;
      const csVector3 &v1 = lf->GetVertex (i);
      const csVector3 &v2 = lf->GetVertex (i1);
      data.planes[i].Set (center, v1+center, v2+center);
    }
    if (lf->GetBackPlane ())
    {
      // @@@ UNTESTED CODE! There are no backplanes yet in frustums.
      // It is possible this plane has to be inverted.
      data.planes_mask = (data.planes_mask<<1)|1;
      data.planes[i] = *(lf->GetBackPlane ());
    }
  }

  // Mark a new region so that we can restore the shadows later.
  iShadowBlockList *shadows = fview->GetFrustumContext ()->GetShadows ();
  uint32 prev_region = shadows->MarkNewRegion ();

  kdtree->Front2Back (center, CastShadows_Front2Back, (void*)&data);

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

