/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "iutil/document.h"
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
#include "imap/services.h"
#include "pvsvis.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csPVSVis)


SCF_IMPLEMENT_IBASE (csPVSVis)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityCuller)
  SCF_IMPLEMENTS_INTERFACE (iPVSCuller)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPVSVis::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPVSVis::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

//----------------------------------------------------------------------

class csPVSVisObjIt : public iVisibilityObjectIterator
{
private:
  csArray<iVisibilityObject*>* vector;
  size_t position;
  bool* vistest_objects_inuse;

public:
  SCF_DECLARE_IBASE;

  csPVSVisObjIt (csArray<iVisibilityObject*>* vector,
  	bool* vistest_objects_inuse)
  {
    SCF_CONSTRUCT_IBASE (0);
    csPVSVisObjIt::vector = vector;
    csPVSVisObjIt::vistest_objects_inuse = vistest_objects_inuse;
    if (vistest_objects_inuse) *vistest_objects_inuse = true;
    Reset ();
  }
  virtual ~csPVSVisObjIt ()
  {
    // If the vistest_objects_inuse pointer is not 0 we set the
    // bool to false to indicate we're no longer using the base
    // vector. Otherwise we delete the vector.
    if (vistest_objects_inuse)
      *vistest_objects_inuse = false;
    else
      delete vector;
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

SCF_IMPLEMENT_IBASE (csPVSVisObjIt)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityObjectIterator)
SCF_IMPLEMENT_IBASE_END

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csPVSVisObjectWrapper)
  SCF_IMPLEMENTS_INTERFACE (iObjectModelListener)
  SCF_IMPLEMENTS_INTERFACE (iMovableListener)
SCF_IMPLEMENT_IBASE_END

void csPVSVisObjectWrapper::ObjectModelChanged (iObjectModel* /*model*/)
{
  pvsvis->AddObjectToUpdateQueue (this);
}

void csPVSVisObjectWrapper::MovableChanged (iMovable* /*movable*/)
{
  pvsvis->AddObjectToUpdateQueue (this);
}

//----------------------------------------------------------------------

csPVSVis::csPVSVis (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  object_reg = 0;
  current_vistest_nr = 1;
  vistest_objects_inuse = false;
  updating = false;
  debug_node = 0;
}

csPVSVis::~csPVSVis ()
{
  ClearObjects ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

void csPVSVis::ClearObjects ()
{
  while (visobj_vector.Length () > 0)
  {
    csPVSVisObjectWrapper* visobj_wrap = visobj_vector.Pop ();
    iVisibilityObject* visobj = visobj_wrap->visobj;
    visobj->GetObjectModel ()->RemoveListener (
		      (iObjectModelListener*)visobj_wrap);
    iMovable* movable = visobj->GetMovable ();
    movable->RemoveListener ((iMovableListener*)visobj_wrap);
    pvstree.RemoveObject (visobj_wrap);
    visobj->DecRef ();
  }
}

bool csPVSVis::Initialize (iObjectRegistry *object_reg)
{
  csPVSVis::object_reg = object_reg;

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

  pvstree.SetObjectRegistry (object_reg);
  pvstree.SetPVSVis (this);
  return true;
}

void csPVSVis::Setup (const char* name)
{
  pvstree.SetPVSCacheName (name);
  const char* err = pvstree.ReadPVS ();
  if (err)
  {
    // This is not an error as it is possible that we're loading a world
    // for pvscalc so that the pvs is calculated.
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.culler.pvsvis",
    	"Couldn't load PVS because of: %s", err);
  }
}

const char* csPVSVis::ParseCullerParameters (iDocumentNode* node)
{
  if (!node)
  {
    return "The PVS visibility culler requires an outer <box>!";
  }

  bool box_was_given = false;

  csRef<iSyntaxService> syn = CS_QUERY_REGISTRY (object_reg,
        iSyntaxService);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    if (!strcmp ("box", value))
    {
      csBox3 b;
      if (!syn->ParseBox (child, b))
        return "Error parsing <box> for the PVS visibility culler!";
      pvstree.SetBoundingBox (b);
      box_was_given = true;
    }
    else
    {
      return "Unrecognized parameter for the PVS visibility culler!";
    }
  }

  return 0;
}

void csPVSVis::CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox)
{
  iMovable* movable = visobj->GetMovable ();
  if (movable->IsFullTransformIdentity ())
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

void csPVSVis::RegisterVisObject (iVisibilityObject* visobj)
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
  csPVSVisObjectWrapper* visobj_wrap = new csPVSVisObjectWrapper (
		  this);
  visobj_wrap->visobj = visobj;
  visobj->IncRef ();
  iMovable* movable = visobj->GetMovable ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();
  visobj_wrap->shape_number = visobj->GetObjectModel ()->GetShapeNumber ();

  csBox3 bbox;
  CalculateVisObjBBox (visobj, bbox);
  pvstree.AddObject (bbox, visobj_wrap);
  visobj_wrap->obj_bbox = bbox;

  iMeshWrapper* mesh = visobj->GetMeshWrapper ();
  visobj_wrap->mesh = mesh;
  if (mesh)
  {
    visobj_wrap->caster = mesh->GetShadowCaster ();
  }

  // Only add the listeners at the very last moment to prevent them to
  // be called by the calls above (i.e. especially the calculation of
  // the bounding box could cause a listener to be fired).
  movable->AddListener ((iMovableListener*)visobj_wrap);
  visobj->GetObjectModel ()->AddListener (
		  (iObjectModelListener*)visobj_wrap);

  visobj_vector.Push (visobj_wrap);
}

void csPVSVis::UnregisterVisObject (iVisibilityObject* visobj)
{
  size_t i;
  for (i = 0 ; i < visobj_vector.Length () ; i++)
  {
    csPVSVisObjectWrapper* visobj_wrap = visobj_vector[i];
    if (visobj_wrap->visobj == visobj)
    {
      update_queue.Delete (visobj_wrap);
      visobj->GetMovable ()->RemoveListener (
		  (iMovableListener*)visobj_wrap);
      visobj->GetObjectModel ()->RemoveListener (
		  (iObjectModelListener*)visobj_wrap);
      pvstree.RemoveObject (visobj_wrap);
      visobj->DecRef ();
#ifdef CS_DEBUG
      // To easily recognize that the vis wrapper has been deleted:
      visobj_wrap->pvsvis = (csPVSVis*)0xdeadbeef;
#endif
      visobj_vector.DeleteIndexFast (i);
      return;
    }
  }
}

void csPVSVis::AddObjectToUpdateQueue (csPVSVisObjectWrapper* visobj_wrap)
{
  if (updating) return;
  CS_ASSERT (visobj_wrap->pvsvis != (csPVSVis*)0xdeadbeef);
  update_queue.Add (visobj_wrap);
}

void csPVSVis::UpdateObjects ()
{
  updating = true;
  {
    csSet<csPVSVisObjectWrapper*>::GlobalIterator it = update_queue.GetIterator ();
    while (it.HasNext ())
    {
      csPVSVisObjectWrapper* vw = it.Next ();
      UpdateObject (vw);
    }
  }
  update_queue.DeleteAll ();
  updating = false;
}

void csPVSVis::UpdateObject (csPVSVisObjectWrapper* visobj_wrap)
{
  CS_ASSERT (visobj_wrap->pvsvis != (csPVSVis*)0xdeadbeef);
  iVisibilityObject* visobj = visobj_wrap->visobj;
  iMovable* movable = visobj->GetMovable ();
  csBox3 bbox;
  CalculateVisObjBBox (visobj, bbox);
  pvstree.MoveObject (visobj_wrap, bbox);
  visobj_wrap->obj_bbox = bbox;
  visobj_wrap->shape_number = visobj->GetObjectModel ()->GetShapeNumber ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();
}

struct PVSTest_Front2BackData
{
  csVector3 pos;
  iRenderView* rview;
  csPlane3* frustum;
  // this is the callback to call when we discover a visible node
  iVisibilityCullerListener* viscallback;
};

int csPVSVis::TestNodeVisibility (csStaticPVSNode* treenode,
	PVSTest_Front2BackData* data, uint32& frustum_mask)
{
  csBox3 node_bbox = treenode->GetNodeBBox ();
  if (node_bbox.Contains (data->pos))
  {
    return NODE_INSIDE;
  }

  uint32 new_mask;
  if (!csIntersect3::BoxFrustum (node_bbox, data->frustum, frustum_mask,
  	new_mask))
  {
    return NODE_INVISIBLE;
  }
  frustum_mask = new_mask;
  return NODE_VISIBLE;
}

bool csPVSVis::TestObjectVisibility (csPVSVisObjectWrapper* obj,
  	PVSTest_Front2BackData* data, uint32 frustum_mask)
{
  if (obj->mesh && obj->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
    return false;

  const csBox3& obj_bbox = obj->obj_bbox;
  if (obj_bbox.Contains (data->pos))
  {
    data->viscallback->ObjectVisible (obj->visobj, obj->mesh, frustum_mask);
    return true;
  }
  
  uint32 new_mask;
  if (!csIntersect3::BoxFrustum (obj_bbox, data->frustum,
		frustum_mask, new_mask))
  {
    return false;
  }

  data->viscallback->ObjectVisible (obj->visobj, obj->mesh, new_mask);

  return true;
}

//======== VisTest =========================================================

static void CallVisibilityCallbacksForSubtree (csStaticPVSNode* treenode,
	PVSTest_Front2BackData* data, uint32 cur_timestamp)
{
  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      iMeshWrapper* mesh = objects[i]->mesh;
      if (!(mesh && mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH)))
        data->viscallback->ObjectVisible (objects[i]->visobj, mesh, 0);
    }
  }

  csStaticPVSNode* child1 = treenode->child1;
  if (child1) CallVisibilityCallbacksForSubtree (child1, data, cur_timestamp);
  csStaticPVSNode* child2 = treenode->child2;
  if (child2) CallVisibilityCallbacksForSubtree (child2, data, cur_timestamp);

}

void csPVSVis::PVSTest_Traverse (csStaticPVSNode* treenode,
	PVSTest_Front2BackData* data,
	uint32 cur_timestamp, uint32 frustum_mask)
{
  // First we see if the current timestamp is equal to the invisibility
  // number of the node. In that case the node is invisible due to the PVS.
  if (cur_timestamp == treenode->invisible_number)
    return;

  // In the first part of this test we are going to test if the node
  // itself is visible. If it is not then we don't need to continue.
  int nodevis = TestNodeVisibility (treenode, data, frustum_mask);
  if (nodevis == NODE_INVISIBLE)
    return;

  if (nodevis == NODE_VISIBLE && frustum_mask == 0)
  {
    // Special case. The node is visible and the frustum mask is 0.
    // This means that the node is completely visible and it doesn't
    // make sense to continue testing visibility. However we need
    // to call the callback on all visible objects. So we traverse the
    // tree manually from this point on. To stop the Front2Back traversal
    // we return false here.
    CallVisibilityCallbacksForSubtree (treenode, data, cur_timestamp);
    return;
  }

  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      TestObjectVisibility (objects[i], data, frustum_mask);
    }
  }

  csStaticPVSNode* child1 = treenode->child1;
  if (child1) PVSTest_Traverse (child1, data, cur_timestamp, frustum_mask);
  csStaticPVSNode* child2 = treenode->child2;
  if (child2) PVSTest_Traverse (child2, data, cur_timestamp, frustum_mask);
}

bool csPVSVis::VisTest (iRenderView* rview, 
                            iVisibilityCullerListener* viscallback)
{
  // We update the objects before testing the callback so that
  // we can use this VisTest() call to make sure the objects in the
  // culler are precached.
  UpdateObjects ();
  current_vistest_nr++;

  // just make sure we have a callback
  if (viscallback == 0)
    return false;

  // Data for the vis tester.
  PVSTest_Front2BackData data;
  data.rview = rview;
  data.viscallback = viscallback;
  data.pos = rview->GetCamera ()->GetTransform ().GetOrigin ();

  // First get the current view frustum from the rview.
  csRenderContext* ctxt = rview->GetRenderContext ();
  data.frustum = ctxt->clip_planes;
  uint32 frustum_mask = ctxt->clip_planes_mask;

  // First we mark all invisible nodes.
  uint32 cur_timestamp = pvstree.NewTraversal ();
  pvstree.MarkInvisible (data.pos, cur_timestamp);

  // The big routine: traverse from front to back and mark all objects
  // visible that are visible.
  PVSTest_Traverse (pvstree.GetRealRootNode (), &data, cur_timestamp,
  	frustum_mask);

  return true;
}

//======== VisTest planes ==================================================

struct PVSTestPlanes_Front2BackData
{

  uint32 current_vistest_nr;
  csArray<iVisibilityObject*>* vistest_objects;

  // During VisTest() we use the current frustum as five planes.
  // Associated with this frustum we also have a clip mask which
  // is maintained recursively during VisTest() and indicates the
  // planes that are still active for the current kd-tree node.
  csPlane3* frustum;

  iVisibilityCullerListener* viscallback;
};

static bool PVSTestPlanes_Front2Back (csStaticPVSNode* treenode,
	void* userdata, uint32 cur_timestamp, uint32& frustum_mask)
{
  PVSTestPlanes_Front2BackData* data
  	= (PVSTestPlanes_Front2BackData*)userdata;

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

  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();
  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      const csBox3& obj_bbox = objects[i]->obj_bbox;
      uint32 new_mask2;
      if (csIntersect3::BoxFrustum (obj_bbox, data->frustum,
		frustum_mask, new_mask2))
      {
	if (data->viscallback)
	{
	  data->viscallback->ObjectVisible (objects[i]->visobj, 
	      objects[i]->mesh, new_mask2);
	}
	else
	{
	  data->vistest_objects->Push (objects[i]->visobj);
	}
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csPVSVis::VisTest (csPlane3* planes,
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
  
  PVSTestPlanes_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.vistest_objects = v;
  data.frustum = planes;
  data.viscallback = 0;
  uint32 frustum_mask = (1 << num_planes)-1;

  pvstree.TraverseRandom (PVSTestPlanes_Front2Back,
  	(void*)&data, frustum_mask);

  csPVSVisObjIt* vobjit = new csPVSVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

void csPVSVis::VisTest (csPlane3* planes,
	int num_planes, iVisibilityCullerListener* viscallback)
{
  UpdateObjects ();
  current_vistest_nr++;

  PVSTestPlanes_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.frustum = planes;
  data.viscallback = viscallback;
  uint32 frustum_mask = (1 << num_planes)-1;

  pvstree.TraverseRandom (PVSTestPlanes_Front2Back,
  	(void*)&data, frustum_mask);
}

//======== VisTest box =====================================================

struct PVSTestBox_Front2BackData
{
  uint32 current_vistest_nr;
  csBox3 box;
  csArray<iVisibilityObject*>* vistest_objects;
};

static bool PVSTestBox_Front2Back (csStaticPVSNode* treenode, void* userdata,
	uint32 cur_timestamp, uint32&)
{
  PVSTestBox_Front2BackData* data = (PVSTestBox_Front2BackData*)userdata;

  // In the first part of this test we are going to test if the
  // box vector intersects with the node. If not then we don't
  // need to continue.
  const csBox3& node_bbox = treenode->GetNodeBBox ();
  if (!node_bbox.TestIntersect (data->box))
  {
    return false;
  }

  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;

      // Test the bounding box of the object.
      const csBox3& obj_bbox = objects[i]->obj_bbox;
      if (obj_bbox.TestIntersect (data->box))
      {
	data->vistest_objects->Push (objects[i]->visobj);
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csPVSVis::VisTest (const csBox3& box)
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
  
  PVSTestBox_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.box = box;
  data.vistest_objects = v;
  pvstree.Front2Back (box.GetCenter (), PVSTestBox_Front2Back, (void*)&data,
  	0);

  csPVSVisObjIt* vobjit = new csPVSVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

//======== VisTest sphere ==================================================

struct PVSTestSphere_Front2BackData
{
  uint32 current_vistest_nr;
  csVector3 pos;
  float sqradius;
  csArray<iVisibilityObject*>* vistest_objects;

  iVisibilityCullerListener* viscallback;
};

static bool PVSTestSphere_Front2Back (csStaticPVSNode* treenode,
	void* userdata, uint32 cur_timestamp, uint32&)
{
  PVSTestSphere_Front2BackData* data =
  	(PVSTestSphere_Front2BackData*)userdata;

  // In the first part of this test we are going to test if the
  // box vector intersects with the node. If not then we don't
  // need to continue.
  const csBox3& node_bbox = treenode->GetNodeBBox ();
  if (!csIntersect3::BoxSphere (node_bbox, data->pos, data->sqradius))
  {
    return false;
  }

  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;

      // Test the bounding box of the object.
      const csBox3& obj_bbox = objects[i]->obj_bbox;
      if (csIntersect3::BoxSphere (obj_bbox, data->pos, data->sqradius))
      {
	if (data->viscallback)
	{
	  data->viscallback->ObjectVisible (
	    objects[i]->visobj, objects[i]->mesh, 0xff);
	}
	else
	{
	  data->vistest_objects->Push (objects[i]->visobj);
	}
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csPVSVis::VisTest (const csSphere& sphere)
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

  PVSTestSphere_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.pos = sphere.GetCenter ();
  data.sqradius = sphere.GetRadius () * sphere.GetRadius ();
  data.vistest_objects = v;
  data.viscallback = 0;
  pvstree.Front2Back (data.pos, PVSTestSphere_Front2Back, (void*)&data,
  	0);

  csPVSVisObjIt* vobjit = new csPVSVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

void csPVSVis::VisTest (const csSphere& sphere, 
			    iVisibilityCullerListener* viscallback)
{
  UpdateObjects ();
  current_vistest_nr++;

  PVSTestSphere_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.pos = sphere.GetCenter ();
  data.sqradius = sphere.GetRadius () * sphere.GetRadius ();
  data.viscallback = viscallback;
  pvstree.Front2Back (data.pos, PVSTestSphere_Front2Back, (void*)&data,
  	0);
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

static bool IntersectSegmentSloppy_Front2Back (csStaticPVSNode* treenode,
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

  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;

      // First test the bounding box of the object.
      const csBox3& obj_bbox = objects[i]->obj_bbox;

      if (csIntersect3::BoxSegment (obj_bbox, data->seg, box_isect) != -1)
      {
        // This object is possibly intersected by this beam.
	if (objects[i]->mesh)
	  if (!objects[i]->mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
	    data->vector->Push (objects[i]->visobj);
      }
    }
  }
  return true;
}

static bool IntersectSegment_Front2Back (csStaticPVSNode* treenode,
	void* userdata, uint32 cur_timestamp, uint32&)
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

  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;

      // First test the bounding box of the object.
      const csBox3& obj_bbox = objects[i]->obj_bbox;

      if (csIntersect3::BoxSegment (obj_bbox, data->seg, box_isect) != -1)
      {
        // This object is possibly intersected by this beam.
	if (objects[i]->mesh)
	{
	  if (!objects[i]->mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
	  {
	    // Transform our vector to object space.
	    csVector3 obj_start;
	    csVector3 obj_end;
	    iMovable* movable = objects[i]->visobj->GetMovable ();
	    bool identity = movable->IsFullTransformIdentity ();
	    csReversibleTransform movtrans;
	    if (identity)
	    {
	      obj_start = data->seg.Start ();
	      obj_end = data->seg.End ();
	    }
	    else
	    {
	      movtrans = movable->GetFullTransform ();
	      obj_start = movtrans.Other2This (data->seg.Start ());
	      obj_end = movtrans.Other2This (data->seg.End ());
	    }
	    csVector3 obj_isect;
	    float r;

	    bool rc;
	    int pidx = -1;
	    if (data->accurate)
	      rc = objects[i]->mesh->GetMeshObject ()->HitBeamObject (
	    	  obj_start, obj_end, obj_isect, &r, &pidx);
	    else
	      rc = objects[i]->mesh->GetMeshObject ()->HitBeamOutline (
	    	  obj_start, obj_end, obj_isect, &r);
	    if (rc)
	    {
	      if (data->vector)
	      {
		data->vector->Push (objects[i]->visobj);
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
		data->mesh = objects[i]->mesh;
	      }
	    }
	  }
	}
      }
    }
  }
  return true;
}

bool csPVSVis::IntersectSegment (const csVector3& start,
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
  data.isect = 0;
  pvstree.Front2Back (start, IntersectSegment_Front2Back, (void*)&data, 0);

  if (p_mesh) *p_mesh = data.mesh;
  if (pr) *pr = data.r;
  if (poly_idx) *poly_idx = data.polygon_idx;
  isect = data.isect;

  return data.mesh != 0;
}

csPtr<iVisibilityObjectIterator> csPVSVis::IntersectSegment (
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
  pvstree.Front2Back (start, IntersectSegment_Front2Back, (void*)&data, 0);

  csPVSVisObjIt* vobjit = new csPVSVisObjIt (data.vector, 0);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

csPtr<iVisibilityObjectIterator> csPVSVis::IntersectSegmentSloppy (
    const csVector3& start, const csVector3& end)
{
  UpdateObjects ();
  current_vistest_nr++;
  IntersectSegment_Front2BackData data;
  data.seg.Set (start, end);
  data.vector = new csArray<iVisibilityObject*> ();
  pvstree.Front2Back (start, IntersectSegmentSloppy_Front2Back,
  	(void*)&data, 0);

  csPVSVisObjIt* vobjit = new csPVSVisObjIt (data.vector, 0);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

//======== CastShadows =====================================================

struct ShadObj
{
  float sqdist;
  iMeshWrapper* mesh;
  iShadowCaster* caster;
  iMovable* movable;
};

struct CastShadows_Front2BackData
{
  uint32 current_vistest_nr;
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

static bool CastShadows_Front2Back (csStaticPVSNode* treenode, void* userdata,
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
    uint32 out_mask;
    if (!csIntersect3::BoxFrustum (node_bbox, data->planes, planes_mask,
    	out_mask))
      return false;
    planes_mask = out_mask;
  }

  const csArray<csPVSVisObjectWrapper*>& objects = treenode->objects;
  int num_objects = objects.Length ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      const csBox3& obj_bbox = objects[i]->obj_bbox;
      csBox3 b (obj_bbox.Min ()-center, obj_bbox.Max ()-center);
      if (b.SquaredOriginDist () > sqrad)
	continue;

      if (objects[i]->caster && fview->ThingShadowsEnabled () &&
            fview->CheckShadowMask (objects[i]->mesh->GetFlags ().Get ()))
      {
        data->shadobjs[data->num_shadobjs].sqdist = b.SquaredOriginDist ();
	data->shadobjs[data->num_shadobjs].caster = objects[i]->caster;
	data->shadobjs[data->num_shadobjs].mesh = 0;
	data->shadobjs[data->num_shadobjs].movable =
		objects[i]->visobj->GetMovable ();
	data->num_shadobjs++;
      }
      if (fview->CheckProcessMask (objects[i]->mesh->GetFlags ().Get ()))
      {
        data->shadobjs[data->num_shadobjs].sqdist = b.SquaredOriginMaxDist ();
	data->shadobjs[data->num_shadobjs].mesh = objects[i]->mesh;
	data->shadobjs[data->num_shadobjs].caster = 0;
	data->shadobjs[data->num_shadobjs].movable =
		objects[i]->visobj->GetMovable ();
	data->num_shadobjs++;
      }
    }
  }

  return true;
}


void csPVSVis::CastShadows (iFrustumView* fview)
{
  UpdateObjects ();
  current_vistest_nr++;
  CastShadows_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
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
    printf ("INTERNAL ERROR! #vertices in GetVisibleObjects() exceeded!\n");
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

  pvstree.Front2Back (center, CastShadows_Front2Back, (void*)&data,
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

bool csPVSVis::Debug_DebugCommand (const char* cmd)
{
  if (!debug_node)
  {
    debug_node = pvstree.GetRealRootNode ();
    debug_nodepath.SetLength (0);
  }
  csRef<iBugPlug> bugplug = CS_QUERY_REGISTRY (object_reg, iBugPlug);
  if (!bugplug || !debug_node) return false;

  if (!strcmp (cmd, "setup_debugsector"))
  {
    bugplug->SetupDebugSector ();
    bugplug->DebugSectorBox (debug_node->GetNodeBBox (),
      	1.0, 1.0, 1.0, 0, 0, CS_FX_ADD);
    csReversibleTransform trans;
    bugplug->SwitchDebugSector (trans, false);
    return true;
  }
  else if (!strcmp (cmd, "navigate_child1"))
  {
    if (debug_node->child1)
    {
      debug_node = debug_node->child1;
      debug_nodepath.Push (1);
      printf ("Going to child1 %p at (%g,%g,%g)-(%g,%g,%g)\n",
      	debug_node,
	debug_node->GetNodeBBox ().MinX (),
	debug_node->GetNodeBBox ().MinY (),
	debug_node->GetNodeBBox ().MinZ (),
	debug_node->GetNodeBBox ().MaxX (),
	debug_node->GetNodeBBox ().MaxY (),
	debug_node->GetNodeBBox ().MaxZ ());
    }
  }
  else if (!strcmp (cmd, "navigate_child2"))
  {
    if (debug_node->child2)
    {
      debug_node = debug_node->child2;
      debug_nodepath.Push (2);
      printf ("Going to child2 %p at (%g,%g,%g)-(%g,%g,%g)\n",
      	debug_node,
	debug_node->GetNodeBBox ().MinX (),
	debug_node->GetNodeBBox ().MinY (),
	debug_node->GetNodeBBox ().MinZ (),
	debug_node->GetNodeBBox ().MaxX (),
	debug_node->GetNodeBBox ().MaxY (),
	debug_node->GetNodeBBox ().MaxZ ());
    }
  }
  else if (!strcmp (cmd, "navigate_parent"))
  {
    debug_node = pvstree.GetRealRootNode ();
    debug_nodepath.SetLength (debug_nodepath.Length ()-1);
    size_t i;
    for (i = 0 ; i < debug_nodepath.Length () ; i++)
      if (debug_nodepath[i] == 1)
        debug_node = debug_node->child1;
      else
        debug_node = debug_node->child2;
      printf ("Going to parent %p at (%g,%g,%g)-(%g,%g,%g)\n",
      	debug_node,
	debug_node->GetNodeBBox ().MinX (),
	debug_node->GetNodeBBox ().MinY (),
	debug_node->GetNodeBBox ().MinZ (),
	debug_node->GetNodeBBox ().MaxX (),
	debug_node->GetNodeBBox ().MaxY (),
	debug_node->GetNodeBBox ().MaxZ ());
  }
  else
    return false;

  bugplug->SetupDebugSector ();
  bugplug->DebugSectorBox (debug_node->GetNodeBBox (),
      	1.0, 1.0, 1.0, 0, 0, CS_FX_ADD);
  csReversibleTransform trans;
  if (!bugplug->CheckDebugSector ())
    bugplug->SwitchDebugSector (trans, false);

  return true;
}

