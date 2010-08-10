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
#include "csutil/sysfunc.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/scfstr.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "csgeom/frustum.h"
#include "csgeom/matrix3.h"
#include "csgeom/math3d.h"
#include "csgeom/obb.h"
#include "csgeom/segment.h"
#include "csgeom/sphere.h"
#include "csgeom/kdtree.h"
#include "imesh/objmodel.h"
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
#include "iengine/sector.h"
#include "imesh/object.h"
#include "iutil/object.h"
#include "imap/loader.h"
#include "ivaria/reporter.h"
#include "csplugincommon/rendermanager/viscull.h"
#include "frustvis.h"
#include "chcpp.h"

//----------------------------------------------------------------------

void csFrustVisObjectWrapper::ObjectModelChanged (iObjectModel* /*model*/)
{
  frustvis->AddObjectToUpdateQueue (this);
}

void csFrustVisObjectWrapper::MovableChanged (iMovable* /*movable*/)
{
  frustvis->AddObjectToUpdateQueue (this);
}

//----------------------------------------------------------------------

csFrustumVis::csFrustumVis () : scfImplementationType (this),
  vistest_objects (256),
  visobj_vector (256),
  update_queue (151, 59)
{
  object_reg = 0;
  kdtree = 0;
  current_vistest_nr = 1;
  vistest_objects_inuse = false;
  updating = false;
}

csFrustumVis::~csFrustumVis ()
{
  while (visobj_vector.GetSize () > 0)
  {
    csRef<csFrustVisObjectWrapper> visobj_wrap = visobj_vector.Pop ();
    iVisibilityObject* visobj = visobj_wrap->visobj;
    visobj->GetObjectModel ()->RemoveListener (
		      (iObjectModelListener*)visobj_wrap);
    iMovable* movable = visobj->GetMovable ();
    movable->RemoveListener ((iMovableListener*)visobj_wrap);
    kdtree->RemoveObject (visobj_wrap->child);
  }
  delete kdtree;
}

bool csFrustumVis::Initialize (iObjectRegistry *object_reg)
{
  csFrustumVis::object_reg = object_reg;

  delete kdtree;

  g3d = csQueryRegistry<iGraphics3D> (object_reg);
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

  kdtree = new csKDTree ();
  csRef<csFrustVisObjectDescriptor> desc;
  desc.AttachNew (new csFrustVisObjectDescriptor ());
  kdtree->SetObjectDescriptor (desc);

  return true;
}

void csFrustumVis::Setup (const char* /*name*/)
{
}

void csFrustumVis::CalculateVisObjBBox (iVisibilityObject* visobj, csBox3& bbox)
{
  iMovable* movable = visobj->GetMovable ();

  if (movable->IsFullTransformIdentity ())
  {
    bbox = visobj->GetObjectModel ()->GetObjectBoundingBox ();
#ifdef CS_DEBUG
    if (bbox.IsNaN ())
    {
      iMeshWrapper* mesh = visobj->GetMeshWrapper ();
      csPrintfErr ("The bounding box of '%s' is invalid!\n",
	    mesh ? mesh->QueryObject ()->GetName () : "<unknown>");
    }
#endif
  }
  else
  {
    const csBox3& box = visobj->GetObjectModel ()->GetObjectBoundingBox ();
#ifdef CS_DEBUG
    if (box.IsNaN ())
    {
      iMeshWrapper* mesh = visobj->GetMeshWrapper ();
      csPrintfErr ("The bounding box of '%s' is invalid!\n",
	    mesh ? mesh->QueryObject ()->GetName () : "<unknown>");
    }
#endif
    csReversibleTransform trans = movable->GetFullTransform ();
    bbox.StartBoundingBox (trans.This2Other (box.GetCorner (0)));
    bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (1)));
    bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (2)));
    bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (3)));
    bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (4)));
    bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (5)));
    bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (6)));
    bbox.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (7)));
#ifdef CS_DEBUG
    if (bbox.IsNaN ())
    {
      iMeshWrapper* mesh = visobj->GetMeshWrapper ();
      csPrintfErr ("The transformed bounding box of '%s' is invalid!\n",
	    mesh ? mesh->QueryObject ()->GetName () : "<unknown>");
    }
#endif
  }
}

void csFrustumVis::RegisterVisObject (iVisibilityObject* visobj)
{
#ifdef CS_DEBUG
  size_t i;
  for (i = 0 ; i < visobj_vector.GetSize () ; i++)
  {
    if (visobj_vector[i]->visobj == visobj)
    {
      CS_ASSERT (false);
    }
  }
#endif
  csRef<csFrustVisObjectWrapper> visobj_wrap;
  visobj_wrap.AttachNew (new csFrustVisObjectWrapper (this));
  visobj_wrap->visobj = visobj;
  iMovable* movable = visobj->GetMovable ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();
  visobj_wrap->shape_number = visobj->GetObjectModel () ? visobj->GetObjectModel ()->GetShapeNumber () : 0;

  csBox3 bbox;
  CalculateVisObjBBox (visobj, bbox);
  visobj_wrap->child = kdtree->AddObject (bbox, (void*)visobj_wrap);
  kdtree_box += bbox;

  iMeshWrapper* mesh = visobj->GetMeshWrapper ();
  visobj_wrap->mesh = mesh;

  // Only add the listeners at the very last moment to prevent them to
  // be called by the calls above (i.e. especially the calculation of
  // the bounding box could cause a listener to be fired).
  movable->AddListener ((iMovableListener*)visobj_wrap);
  visobj->GetObjectModel ()->AddListener (
		  (iObjectModelListener*)visobj_wrap);

  visobj_vector.Push (visobj_wrap);


  printf("Inserting into aabb (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",bbox.MinX(),bbox.MinY(),bbox.MinZ(),
          bbox.MaxX(),bbox.MaxY(),bbox.MaxZ());

  NodeLeafData *nld=new NodeLeafData(g3d,mesh,bbox);
  mapNodeLeafData[mesh]=nld;
  aabbTree.AddObject(nld);
}

void csFrustumVis::UnregisterVisObject (iVisibilityObject* visobj)
{
  size_t i;
  for (i = 0 ; i < visobj_vector.GetSize () ; i++)
  {
    csFrustVisObjectWrapper* visobj_wrap = visobj_vector[i];
    if (visobj_wrap->visobj == visobj)
    {
      update_queue.Delete (visobj_wrap);
      visobj->GetMovable ()->RemoveListener ( (iMovableListener*)visobj_wrap );
      iObjectModel* objmodel = visobj->GetObjectModel ();
      objmodel->RemoveListener ( (iObjectModelListener*)visobj_wrap );
      kdtree->RemoveObject (visobj_wrap->child);

      iMeshWrapper* mesh = visobj->GetMeshWrapper ();

      printf("Deleting aabb (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          mapNodeLeafData[mesh]->bbox.MinX(),mapNodeLeafData[mesh]->bbox.MinY(),mapNodeLeafData[mesh]->bbox.MinZ(),
          mapNodeLeafData[mesh]->bbox.MaxX(),mapNodeLeafData[mesh]->bbox.MaxY(),mapNodeLeafData[mesh]->bbox.MaxZ());

      aabbTree.RemoveObject(mapNodeLeafData[mesh]);
      delete mapNodeLeafData[mesh];
      mapNodeLeafData.erase(mesh);

#ifdef CS_DEBUG
      // To easily recognize that the vis wrapper has been deleted:
      visobj_wrap->frustvis = (csFrustumVis*)0xdeadbeef;
#endif
      visobj_vector.DeleteIndexFast (i);

      return;
    }
  }
}

void csFrustumVis::AddObjectToUpdateQueue (csFrustVisObjectWrapper* visobj_wrap)
{
  if (updating) return;
  CS_ASSERT (visobj_wrap->frustvis != (csFrustumVis*)0xdeadbeef);
  update_queue.Add (visobj_wrap);
}

void csFrustumVis::UpdateObjects ()
{
  updating = true;
  {
    csSet<csPtrKey<csFrustVisObjectWrapper> >::GlobalIterator it = 
      update_queue.GetIterator ();
    while (it.HasNext ())
    {
      csFrustVisObjectWrapper* vw = it.Next ();
      UpdateObject (vw);
    }
  }
  update_queue.DeleteAll ();
  updating = false;
}

void csFrustumVis::UpdateObject (csFrustVisObjectWrapper* visobj_wrap)
{
  CS_ASSERT (visobj_wrap->frustvis != (csFrustumVis*)0xdeadbeef);
  iVisibilityObject* visobj = visobj_wrap->visobj;
  iMovable* movable = visobj->GetMovable ();
  csBox3 bbox;
  CalculateVisObjBBox (visobj, bbox);
  kdtree->MoveObject (visobj_wrap->child, bbox);
  kdtree_box += bbox;
  visobj_wrap->shape_number = visobj->GetObjectModel ()->GetShapeNumber ();
  visobj_wrap->update_number = movable->GetUpdateNumber ();
}

/*--------------------------------------------------------------------*/
/*        IMPORTANT...does the essense of the frustum culling         */
/*--------------------------------------------------------------------*/
int csFrustumVis::TestNodeVisibility (csKDTree* treenode,
	FrustTest_Front2BackData* data, uint32& frustum_mask)
{
  csBox3 node_bbox = treenode->GetNodeBBox ();
  node_bbox *= kdtree_box;

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

bool csFrustumVis::TestObjectVisibility (csFrustVisObjectWrapper* obj,
  	FrustTest_Front2BackData* data, uint32 frustum_mask)
{
  if (obj->mesh && obj->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
    return false;

  const csBox3& obj_bbox = obj->child->GetBBox ();
  if (obj_bbox.Contains (data->pos))
  {
    data->viscallback->ObjectVisible (obj->visobj, obj->mesh, frustum_mask);
    return true;
  }
  
  uint32 new_mask;
  if (!csIntersect3::BoxFrustum (obj_bbox, data->frustum, frustum_mask, new_mask))
  {
    return false;
  }

  csSectorVisibleRenderMeshes* meshList=0;
  static_cast<CS::RenderManager::Implementation::ViscullCallback<RenderTreeType>*>
    (data->viscallback)->ObjectVisible (obj->visobj, obj->mesh, new_mask);

  return true;
}

struct InnerNodeProcessOP
{
  InnerNodeProcessOP()
  {
  }
  InnerNodeProcessOP(const FrustTest_Front2BackData *data)
  {
    f2bData=data;
  }

  const FrustTest_Front2BackData *f2bData;

  void DrawQuery(NodePtr n) const
  {
    n->GetGraphics3D()->OQBeginQuery(n->GetQueryID());
    n->GetGraphics3D()->DrawSimpleMesh(n->srmSimpRendMesh);
    n->GetGraphics3D()->OQEndQuery();
    csBox3 box=n->GetBBox();
    if(n->GetGraphics3D()->OQIsVisible(n->GetQueryID(),0))
    {
      csPrintf("BB Visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      //csPrintf("BB Visible\n");
    }
    else
    {
      csPrintf("BB Not visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      //csPrintf("BB Not visible\n");
    }
  }

  void NodeVisible(NodePtr n) const
  {
    n->SetCameraTimestamp(f2bData->rview->GetCamera(),f2bData->current_timestamp);
    n->SetVisibilityForCamera(f2bData->rview->GetCamera(),false);
    //DrawQuery(n);
  }

  bool operator() (NodePtr n, uint32 &frustum_mask) const
  {
    CS_ASSERT_MSG("Invalid AABB-tree", !n->IsLeaf ());
    csBox3 node_bbox = n->GetBBox();
    node_bbox *= f2bData->global_bbox;

    if (node_bbox.Contains (f2bData->pos))
    {
      NodeVisible(n);
      return true; // node completely visible
    }
    uint32 new_mask;
    if (!csIntersect3::BoxFrustum (node_bbox,
                                   f2bData->frustum,
                                   frustum_mask,
  	                               new_mask))
    {
      return false; //node invisible so discontinue traversal for this subtree
    }
    frustum_mask=new_mask;

    NodeVisible(n);
    return true; // node visible
  }
};


struct LeafNodeProcessOP
{
  LeafNodeProcessOP()
  {
  }
  LeafNodeProcessOP(const FrustTest_Front2BackData *data)
  {
    f2bData=data;
  }

  const FrustTest_Front2BackData *f2bData;

  bool IsRMPortal(const NodePtr n,const csSectorVisibleRenderMeshes* meshList,const int numMeshes) const
  {
    for (int m = 0; m < numMeshes; ++m)
    {
      for (int i = 0; i < meshList[m].num; ++i)
	    {
        csRenderMesh* rm = meshList[m].rmeshes[i];
        if(rm->portal)
        {
          return true;
        }
      }
    }
    return false;
  }

  void DrawQuery(const NodePtr n,const csSectorVisibleRenderMeshes* meshList,const int numMeshes) const
  {
    n->GetGraphics3D()->OQBeginQuery(n->GetQueryID());
    for (int m = 0; m < numMeshes; ++m)
    {
      for (int i = 0; i < meshList[m].num; ++i)
	    {
        csRenderMesh* rm = meshList[m].rmeshes[i];
        if(!rm->portal)
        {
          csVertexAttrib vA = CS_VATTRIB_POSITION;
          iRenderBuffer *rB = rm->buffers->GetRenderBuffer (CS_BUFFER_POSITION);
          n->GetGraphics3D()->ActivateBuffers (&vA, &rB, 1);
          n->GetGraphics3D()->DrawMeshBasic (rm, *rm);
          n->GetGraphics3D()->DeactivateBuffers (&vA, 1);
        }
      }
    }
    n->GetGraphics3D()->OQEndQuery();
  }

  bool CheckOQ(const NodePtr n) const
  {
    const unsigned int oqID=n->GetQueryID();
    if(oqID)
    {
      return n->GetGraphics3D()->OQIsVisible(n->GetQueryID(),0);
    }
    return true;
  }

  void NodeVisible(const NodePtr n,uint32 frustum_mask) const
  {
    n->SetCameraTimestamp(f2bData->rview->GetCamera(),f2bData->current_timestamp);
    n->SetVisibilityForCamera(f2bData->rview->GetCamera(),false);

    iMeshWrapper* const mw=n->GetLeafData(0)->mesh;
    const uint32 frust_mask=f2bData->rview->GetRenderContext ()->clip_planes_mask;
    csSectorVisibleRenderMeshes* meshList;
    const int numMeshes = f2bData->viscallback->GetVisibleMeshes(mw,frustum_mask,meshList);
    if(numMeshes > 0 )
    {
      const bool isp=IsRMPortal(n,meshList,numMeshes);
      if(isp || CheckOQ(n))
        f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
      if(!isp)
        DrawQuery(n,meshList,numMeshes);
    }
  }

  bool operator() (NodePtr n,const uint32 frustum_mask) const
  { 
    CS_ASSERT_MSG("Invalid AABB-tree", n->IsLeaf ());
    const int num_objects=n->GetObjectCount();
    const NodeLeafData* visobj_wrap = n->GetLeafData(0);

    if (visobj_wrap->mesh && visobj_wrap->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
    {
      return true; // node invisible, but continue traversal
    }

    const csBox3& obj_bbox = visobj_wrap->GetBBox ();
    if (obj_bbox.Contains (f2bData->pos))
    {
      NodeVisible(n,frustum_mask);
      return true;
    }
  
    uint32 new_mask;
    if (!csIntersect3::BoxFrustum (obj_bbox, f2bData->frustum, frustum_mask, new_mask))
    {
      return true; // node invisible, but continue traversal
    }

    NodeVisible(n,new_mask);
    return true;
  }
};

/*------------------------------------------------------------------*/
/*--------------------------- MAIN DADDY ---------------------------*/
/*------------------------------------------------------------------*/
bool csFrustumVis::VisTest (iRenderView* rview, iVisibilityCullerListener* viscallback,
                            int, int)
{
  UpdateObjects ();
  current_vistest_nr++;

  // just make sure we have a callback
  if (viscallback == 0)
    return false;

  csRenderContext* ctxt = rview->GetRenderContext ();
  f2bData.frustum = ctxt->clip_planes;
  uint32 frustum_mask = ctxt->clip_planes_mask;
  const uint32 cur_timestamp = kdtree->NewTraversal ();

  f2bData.pos = rview->GetCamera ()->GetTransform ().GetOrigin ();
  f2bData.rview = rview;
  f2bData.viscallback = viscallback;
  f2bData.global_bbox=kdtree_box;
  f2bData.current_timestamp=cur_timestamp;

  //g3d=rview->GetGraphics3D();
  if (!g3d->BeginDraw(rview->GetEngine()->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN))
  {
	      csPrintf("Cannot prepare renderer for 3D drawing\n");
        return false;
  }
  g3d->FinishDraw();


  const csReversibleTransform& camt = rview->GetCamera()->GetTransform ();
  g3d->SetClipper (rview->GetClipper(), CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  g3d->ResetNearPlane ();
  g3d->SetProjectionMatrix (rview->GetCamera()->GetProjectionMatrix ());
  if (!g3d->BeginDraw(rview->GetEngine()->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN))
  {
    csPrintf("Cannot prepare renderer for 3D drawing\n");
  }
  g3d->SetWorldToCamera (camt.GetInverse ());
  g3d->SetZMode(CS_ZBUF_USE);
  g3d->SetWriteMask(false,false,false,false);

  const InnerNodeProcessOP opIN(&f2bData);
  const LeafNodeProcessOP opLN(&f2bData);
  const csVector3 dir=rview->GetCamera ()->GetTransform ().GetFront()-rview->GetCamera ()->GetTransform ().GetOrigin ();
  //aabbTree.SetRootFrustumMask(frustum_mask);
  //aabbTree.TraverseF2B(opIN,opLN,dir);
  //aabbTree.TraverseF2BWithData(opIN,opLN,frustum_mask,dir);
  aabbTree.TraverseIterF2BWithData(opIN,opLN,frustum_mask,dir);

  // The big routine: traverse from front to back and mark all objects
  // visible that are visible.
  /*NodeTraverseData ntdRoot(kdtree,0,frustum_mask);
  ntdRoot.SetVisibility(true);
  T_Queue.PushBack(ntdRoot);

  while(!T_Queue.IsEmpty())
  {
    if(!T_Queue.IsEmpty())
    {
      NodeTraverseData ntdCurrent=T_Queue.Front();
      T_Queue.PopFront();

      if(!ntdCurrent.IsCompletelyVisible())
      {
        int nodevis = TestNodeVisibility (ntdCurrent.kdtNode, &f2bData,ntdCurrent.u32Frustum_Mask);
        if (nodevis == NODE_INVISIBLE)
          continue;

        if (nodevis == NODE_VISIBLE && frustum_mask == 0)
        {
          ntdCurrent.SetCompletelyVisible(true);
          //CallVisibilityCallbacksForSubtree (ntdAux, cur_timestamp);
          //continue;
        }
      }
      ntdCurrent.SetVisibility(false);

      // update the timestamp flag
      ntdCurrent.SetTimestamp(cur_timestamp);

      // important...never try and traverse before doing a distribute
      // unless the node is fully visible, in which case it doesn't matter
      ntdCurrent.kdtNode->Distribute ();
      TraverseNode(ntdCurrent,cur_timestamp);
    }
  }*/

  // here we should process the remaining nodes in the multi query

  //printf("\n\n");

  g3d->SetWriteMask(true,true,true,true);
  g3d->SetClipper (0, CS_CLIPPER_NONE);
  g3d->FinishDraw();

  return true;
}

//======== VisTest planes ==================================================

struct FrustTestPlanes_Front2BackData
{

  uint32 current_vistest_nr;
  csFrustumVis::VistestObjectsArray* vistest_objects;

  // During VisTest() we use the current frustum as five planes.
  // Associated with this frustum we also have a clip mask which
  // is maintained recursively during VisTest() and indicates the
  // planes that are still active for the current kd-tree node.
  csPlane3* frustum;

  iVisibilityCullerListener* viscallback;
};

static bool FrustTestPlanes_Front2Back (csKDTree* treenode,
	void* userdata, uint32 cur_timestamp, uint32& frustum_mask)
{
  FrustTestPlanes_Front2BackData* data
  	= (FrustTestPlanes_Front2BackData*)userdata;

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

  for (int i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	objects[i]->GetObject ();
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
	        data->vistest_objects->Push (visobj_wrap->visobj);
	      }
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csFrustumVis::VisTest (csPlane3* planes,
	int num_planes)
{
  UpdateObjects ();
  current_vistest_nr++;

  VistestObjectsArray* v;
  if (vistest_objects_inuse)
  {
    // Vector is already in use by another iterator. Allocate a new vector
    // here.
    v = new VistestObjectsArray ();
  }
  else
  {
    v = &vistest_objects;
    vistest_objects.Empty ();
  }
  
  FrustTestPlanes_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.vistest_objects = v;
  data.frustum = planes;
  data.viscallback = 0;
  uint32 frustum_mask = (1 << num_planes)-1;

  kdtree->TraverseRandom (FrustTestPlanes_Front2Back,
  	(void*)&data, frustum_mask);

  csFrustVisObjIt* vobjit = new csFrustVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

void csFrustumVis::VisTest (csPlane3* planes,
	int num_planes, iVisibilityCullerListener* viscallback)
{
  UpdateObjects ();
  current_vistest_nr++;

  FrustTestPlanes_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.frustum = planes;
  data.viscallback = viscallback;
  uint32 frustum_mask = (1 << num_planes)-1;

  kdtree->TraverseRandom (FrustTestPlanes_Front2Back,
  	(void*)&data, frustum_mask);
}

//======== VisTest box =====================================================

struct FrustTestBox_Front2BackData
{
  uint32 current_vistest_nr;
  csBox3 box;
  csFrustumVis::VistestObjectsArray* vistest_objects;
};

static bool FrustTestBox_Front2Back (csKDTree* treenode, void* userdata,
	uint32 cur_timestamp, uint32&)
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
  csKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  for (int i = 0 ; i < num_objects ; i++)
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
	      data->vistest_objects->Push (visobj_wrap->visobj);
      }
    }
  }

  return true;
}

csPtr<iVisibilityObjectIterator> csFrustumVis::VisTest (const csBox3& box)
{
  UpdateObjects ();
  current_vistest_nr++;

  VistestObjectsArray* v;
  if (vistest_objects_inuse)
  {
    // Vector is already in use by another iterator. Allocate a new vector
    // here.
    v = new VistestObjectsArray ();
  }
  else
  {
    v = &vistest_objects;
    vistest_objects.Empty ();
  }
  
  FrustTestBox_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.box = box;
  data.vistest_objects = v;
  kdtree->Front2Back (box.GetCenter (), FrustTestBox_Front2Back, (void*)&data,
  	0);

  csFrustVisObjIt* vobjit = new csFrustVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

//======== VisTest sphere ==================================================

struct FrustTestSphere_Front2BackData
{
  uint32 current_vistest_nr;
  csVector3 pos;
  float sqradius;
  csFrustumVis::VistestObjectsArray* vistest_objects;

  iVisibilityCullerListener* viscallback;
};

static bool FrustTestSphere_Front2Back (csKDTree* treenode,
	void* userdata, uint32 cur_timestamp, uint32&)
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
  csKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  for (int i = 0 ; i < num_objects ; i++)
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
	      if (data->viscallback)
	      {
	        data->viscallback->ObjectVisible (
	          visobj_wrap->visobj, visobj_wrap->mesh, 0xff);
	      }
	      else
	      {
	        data->vistest_objects->Push (visobj_wrap->visobj);
	      }
      }
    }
  }
  return true;
}

csPtr<iVisibilityObjectIterator> csFrustumVis::VisTest (const csSphere& sphere)
{
  UpdateObjects ();
  current_vistest_nr++;

  VistestObjectsArray* v;
  if (vistest_objects_inuse)
  {
    // Vector is already in use by another iterator. Allocate a new vector
    // here.
    v = new VistestObjectsArray ();
  }
  else
  {
    v = &vistest_objects;
    vistest_objects.Empty ();
  }

  FrustTestSphere_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.pos = sphere.GetCenter ();
  data.sqradius = sphere.GetRadius () * sphere.GetRadius ();
  data.vistest_objects = v;
  data.viscallback = 0;
  kdtree->Front2Back (data.pos, FrustTestSphere_Front2Back, (void*)&data,
  	0);

  csFrustVisObjIt* vobjit = new csFrustVisObjIt (v,
  	vistest_objects_inuse ? 0 : &vistest_objects_inuse);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

void csFrustumVis::VisTest (const csSphere& sphere, 
			    iVisibilityCullerListener* viscallback)
{
  UpdateObjects ();
  current_vistest_nr++;

  FrustTestSphere_Front2BackData data;
  data.current_vistest_nr = current_vistest_nr;
  data.pos = sphere.GetCenter ();
  data.sqradius = sphere.GetRadius () * sphere.GetRadius ();
  data.viscallback = viscallback;
  kdtree->Front2Back (data.pos, FrustTestSphere_Front2Back, (void*)&data,
  	0);
}

//============================ Segment intersection code =======================================

struct IntersectSegment_Front2BackData
{
  csSegment3 seg;
  csVector3 isect;
  float sqdist;		// Squared distance between seg.start and isect.
  float r;
  iMeshWrapper* mesh;
  int polygon_idx;
  csFrustumVis::VistestObjectsArray* vector;	// If not-null we need all objects.
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

  for (int i = 0 ; i < num_objects ; i++)
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
	        if (!visobj_wrap->mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
	          data->vector->Push (visobj_wrap->visobj);
      }
    }
  }
  return true;
}

static bool IntersectSegment_Front2Back (csKDTree* treenode,
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
      csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
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
	          iMovable* movable = visobj_wrap->visobj->GetMovable ();
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

struct InnerNodeIntersectSegment
{
  IntersectSegment_Front2BackData data;
  bool operator() (AABBTree<csFrustVisObjectWrapper,1,NodeData>::Node* n)
  {
    const csBox3& node_bbox = n->GetBBox ();

    // If mesh != 0 then we have already found our mesh. In that
    // case we will compare the distance of the origin with the the
    // box of the treenode and the already found shortest distance to
    // see if we have to proceed.
    if (data.mesh)
    {
      csBox3 b (node_bbox.Min ()-data.seg.Start (),
    	        node_bbox.Max ()-data.seg.Start ());
      if (b.SquaredOriginDist () > data.sqdist) return false;
    }

    // In the first part of this test we are going to test if the
    // start-end vector intersects with the node. If not then we don't
    // need to continue.
    csVector3 box_isect;
    if (csIntersect3::BoxSegment (node_bbox, data.seg, box_isect) == -1)
    {
      return false;
    }
    return true;
  }
};

struct LeafNodeIntersectSegment
{
  IntersectSegment_Front2BackData data;
  bool operator() (AABBTree<csFrustVisObjectWrapper,1,NodeData>::Node* n)
  {
    return true;
  }
};

bool csFrustumVis::IntersectSegment (const csVector3& start,
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
  kdtree->Front2Back (start, IntersectSegment_Front2Back, (void*)&data, 0);

  InnerNodeIntersectSegment isIN;
  isIN.data=data;
  LeafNodeIntersectSegment isLN;
  isLN.data=data;

  //aabbTree.TraverseF2B(isIN,isLN

  if (p_mesh) *p_mesh = data.mesh;
  if (pr) *pr = data.r;
  if (poly_idx) *poly_idx = data.polygon_idx;
  isect = data.isect;

  return data.mesh != 0;
}

csPtr<iVisibilityObjectIterator> csFrustumVis::IntersectSegment (
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
  data.vector = new VistestObjectsArray ();
  data.accurate = accurate;
  kdtree->Front2Back (start, IntersectSegment_Front2Back, (void*)&data, 0);

  InnerNodeIntersectSegment isIN;
  isIN.data=data;
  LeafNodeIntersectSegment isLN;
  isLN.data=data;

  csFrustVisObjIt* vobjit = new csFrustVisObjIt (data.vector, 0);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

csPtr<iVisibilityObjectIterator> csFrustumVis::IntersectSegmentSloppy (
    const csVector3& start, const csVector3& end)
{
  UpdateObjects ();
  current_vistest_nr++;
  IntersectSegment_Front2BackData data;
  data.seg.Set (start, end);
  data.vector = new VistestObjectsArray ();
  kdtree->Front2Back (start, IntersectSegmentSloppy_Front2Back,
  	(void*)&data, 0);

  InnerNodeIntersectSegment isIN;
  isIN.data=data;
  LeafNodeIntersectSegment isLN;
  isLN.data=data;

  csFrustVisObjIt* vobjit = new csFrustVisObjIt (data.vector, 0);
  return csPtr<iVisibilityObjectIterator> (vobjit);
}

//============================ End segment intersection code ===================================
