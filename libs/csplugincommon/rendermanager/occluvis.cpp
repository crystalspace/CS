/*
  Copyright (C) 2010 by Mike Gist and Claudiu Mihail

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#include <cssysdef.h>

#include "csgeom/math3d.h"
#include "csplugincommon/rendermanager/occluvis.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iutil/object.h"
#include "iengine/rview.h"
#include "iutil/objreg.h"

namespace CS
{
  namespace RenderManager
  {
    csOccluvis::csVisibilityObjectWrapper::csVisibilityObjectWrapper (csOccluvis* culler, iVisibilityObject* vis_obj)
      : scfImplementationType (this), culler (culler), vis_obj (vis_obj)
    {
      oldBBox = vis_obj->GetMeshWrapper ()->GetWorldBoundingBox ();
    }

    void csOccluvis::csVisibilityObjectWrapper::ObjectModelChanged (iObjectModel* model)
    {
      const csBox3& newBBox = vis_obj->GetMeshWrapper ()->GetWorldBoundingBox ();
      culler->MoveObject (vis_obj, oldBBox);
      oldBBox = newBBox;
    }

    void csOccluvis::csVisibilityObjectWrapper::MovableChanged (iMovable* movable)
    {
      const csBox3& newBBox = vis_obj->GetMeshWrapper ()->GetWorldBoundingBox ();
      culler->MoveObject (vis_obj, oldBBox);
      oldBBox = newBBox;
    }

    void csOccluvis::csVisibilityObjectWrapper::MovableDestroyed (iMovable*)
    {
    }

    template<bool bQueryVisibility>
    void csOccluvis::RenderMeshes(AABBVisTreeNode* node, Front2BackData& f2bData, csArray<MeshList*> &meshList)
    {
      if (bQueryVisibility)
      {
        BeginNodeQuery (node, f2bData.rview);
      }

      for(unsigned int j=0 ; j < meshList.GetSize() ; ++j)
      {
        MeshList& obj = *meshList.Get(j);

        for (int m = 0; m < obj.numMeshes; ++m)
        {
          iMeshWrapper* mw = obj.meshList[m].imesh;
          g3d->SetZMode (mw->GetZBufMode ());

          for (int i = 0; i < obj.meshList[m].num; ++i)
          {
            csRenderMesh* rm = obj.meshList[m].rmeshes[i];
            if(!rm->portal)
            {
              csVertexAttrib vA = CS_VATTRIB_POSITION;
              iRenderBuffer *rB = rm->buffers->GetRenderBuffer (CS_BUFFER_POSITION);
              g3d->ActivateBuffers (&vA, &rB, 1);
              g3d->DrawMeshBasic (rm, *rm);
              g3d->DeactivateBuffers (&vA, 1);
            }
          }
        }

        delete &obj;
      }

      if (bQueryVisibility)
      {
        g3d->OQEndQuery ();
      }
    }

    template<bool bDoFrustumCulling>
    void csOccluvis::TraverseNodeF2B(AABBVisTreeNode* node,
                                     bool parentVisible,
                                     uint32 frustum_mask,
                                     Front2BackData& f2bData,
                                     csArray<MeshList*>& meshList)
    {
      if (bDoFrustumCulling)
      {
        NodeVisibility nodevis = TestNodeVisibility (node, f2bData, frustum_mask);

        if (nodevis == NODE_INVISIBLE)
          return;

        if (nodevis == NODE_VISIBLE && frustum_mask == 0)
        {
          TraverseNodeF2B<false> (node, parentVisible, frustum_mask, f2bData, meshList);
          return;
        }
      }

      if (!node->IsLeaf())
      {
        AABBVisTreeNode* frontNode, *backNode;
        csArray<MeshList*> firstMeshList, secondMeshList;
        bool visible = parentVisible && (GetNodeVisibility (node, f2bData.rview) == VISIBLE);

        GetF2BChildren (node, f2bData, frontNode, backNode);

        if(frontNode)
        {
          TraverseNodeF2B<bDoFrustumCulling> (frontNode, visible, frustum_mask, f2bData, firstMeshList);

          if (!firstMeshList.IsEmpty () && GetNodeVisibility (backNode, f2bData.rview) == VISIBLE)
          {
            RenderMeshes<true> (frontNode, f2bData, firstMeshList);
            firstMeshList.Empty ();
          }
        }

        if(backNode)
        {
          TraverseNodeF2B<bDoFrustumCulling> (backNode, visible, frustum_mask, f2bData, secondMeshList);

          if (!secondMeshList.IsEmpty () && GetNodeVisibility (frontNode, f2bData.rview) == VISIBLE)
          {
            RenderMeshes<true> (backNode, f2bData, secondMeshList);
            secondMeshList.Empty ();
          }
        }

        if (visible)
        {
          if (!firstMeshList.IsEmpty () || !secondMeshList.IsEmpty ())
          {
            BeginNodeQuery (node, f2bData.rview);

            RenderMeshes<false> (frontNode, f2bData, firstMeshList);
            RenderMeshes<false> (backNode, f2bData, secondMeshList);

            g3d->OQEndQuery ();
          }
        }
        else
        {
          for (size_t i = 0; i < firstMeshList.GetSize (); ++i)
          {
            meshList.Push (firstMeshList[i]);
          }

          for (size_t i = 0; i < secondMeshList.GetSize (); ++i)
          {
            meshList.Push (secondMeshList[i]);
          }
        }
      }
      else
      {
        OcclusionVisibility eOccVis = GetNodeVisibility (node, f2bData.rview);

        // One object only in this AABBTree
        iVisibilityObject* visobj = node->GetLeafData (0);

        // Only test an object via occlusion if it's not flagged invisible.
        if(!visobj->GetMeshWrapper ()->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
        {
          csSectorVisibleRenderMeshes* sectorMeshList;
          const int numMeshes = f2bData.viscallback->GetVisibleMeshes (visobj->GetMeshWrapper (), frustum_mask, sectorMeshList);

          bool hasMeshes = false;
          for (size_t i = 0; i < numMeshes; ++i)
          {
            hasMeshes |= (sectorMeshList->num > 0);
          }

          if (hasMeshes)
          {
            meshList.Push (new MeshList (sectorMeshList, numMeshes));

            // If occlusion checks also passed, mark the mesh visible.
            if (parentVisible && eOccVis == VISIBLE)
            {
              f2bData.viscallback->MarkVisible(visobj->GetMeshWrapper (), numMeshes, sectorMeshList);
            }
          }
        }

        if (!meshList.IsEmpty () && eOccVis == VISIBLE)
        {
          if (CheckNodeVisibility (node, f2bData.rview))
          {
            RenderMeshes<true> (node, f2bData, meshList);
          }
          else
          {
            RenderMeshes<false> (node, f2bData, meshList);
          }

          meshList.Empty ();
        }
      }
    }

    OcclusionVisibility csOccluvis::GetNodeVisibility (AABBVisTreeNode* node, iRenderView* rview)
    {
      uint32 uFrame = engine->GetCurrentFrameNumber ();
      QueryData* queryData = GetNodeVisData (node).GetQueryData (g3d, rview);

      if (uFrame >= queryData->uNextCheck)
      {
        queryData->uNextCheck = uFrame;
      }

      if (queryData->eResult == INVALID || uFrame != queryData->uQueryFrame + 1)
      {
        return VISIBLE;
      }

      if (queryData->eResult != UNKNOWN)
      {
        return queryData->eResult;
      }

      queryData->eResult = g3d->OQIsVisible (queryData->uOQuery) ? VISIBLE : INVISIBLE;

      if (queryData->eResult == VISIBLE)
      {
        queryData->uNextCheck += visibilityFrameSkip;
      }

      return queryData->eResult;
    }

    bool csOccluvis::CheckNodeVisibility (AABBVisTreeNode* node, iRenderView* rview)
    {
      QueryData* queryData = GetNodeVisData (node).GetQueryData (g3d, rview);

      if (engine->GetCurrentFrameNumber () >= queryData->uNextCheck)
      {
        return true;
      }

      return false;
    }

    void csOccluvis::BeginNodeQuery (AABBVisTreeNode* node, iRenderView* rview)
    {
      QueryData* queryData = GetNodeVisData (node).GetQueryData (g3d, rview);

      queryData->eResult = UNKNOWN;
      queryData->uQueryFrame = engine->GetCurrentFrameNumber ();

      g3d->OQBeginQuery (queryData->uOQuery);
    }

    void csOccluvis::GetF2BChildren (AABBVisTreeNode* node, Front2BackData& data,
      AABBVisTreeNode*& fChild, AABBVisTreeNode*& bChild)
    {
      csVector3 direction = data.rview->GetCamera ()->GetTransform ().GetFront ();
      const csVector3 centerDiff = node->GetChild2 ()->GetBBox ().GetCenter () -
        node->GetChild1 ()->GetBBox ().GetCenter ();

      const size_t firstIdx = (centerDiff * direction > 0) ? 0 : 1;

      fChild = node->GetChild (firstIdx);
      bChild = node->GetChild (1 - firstIdx);
    }

    csOccluvis::csOccluvis (iObjectRegistry* object_reg)
      : object_reg (object_reg)
    {
      g3d = csQueryRegistry<iGraphics3D> (object_reg);
      engine = csQueryRegistry<iEngine> (object_reg);

      bAllVisible = false;
    }

    void csOccluvis::RegisterVisObject (iVisibilityObject* visobj)
    {
      csRef<csVisibilityObjectWrapper> visobj_wrap;
      visobj_wrap.AttachNew (new csVisibilityObjectWrapper (this, visobj));

      AddObject (visobj);

      iMovable* movable = visobj->GetMovable ();
      movable->AddListener (visobj_wrap);

      iObjectModel* objmodel = visobj->GetObjectModel ();
      objmodel->AddListener (visobj_wrap);

      visObjects.Push (visobj_wrap);
    }

    void csOccluvis::UnregisterVisObject (iVisibilityObject* visobj)
    {
      for (size_t i = 0 ; i < visObjects.GetSize () ; i++)
      {
        csVisibilityObjectWrapper* visobj_wrap = visObjects[i];

        if (visobj_wrap->GetVisObject () == visobj)
        {
          iMovable* movable = visobj->GetMovable ();
          movable->RemoveListener (visobj_wrap);

          iObjectModel* objmodel = visobj->GetObjectModel ();
          objmodel->RemoveListener (visobj_wrap);

          visObjects.DeleteIndexFast (i);
          break;
        }
      }

      RemoveObject (visobj);
    }

    /*--------------------------------------------------------------------*/
    /*        IMPORTANT...does the essence of the frustum culling         */
    /*--------------------------------------------------------------------*/
    NodeVisibility csOccluvis::TestNodeVisibility (AABBVisTreeNode* node,
      Front2BackData& data, uint32& frustum_mask)
    {
      csBox3 node_bbox = node->GetBBox ();

      if (node_bbox.Contains (data.pos))
      {
        return NODE_INSIDE;
      }

      uint32 new_mask;
      if (!csIntersect3::BoxFrustum (node_bbox, data.frustum, frustum_mask,
        new_mask))
      {
        return NODE_INVISIBLE;
      }

      frustum_mask = new_mask;
      return NODE_VISIBLE;
    }

    //======== VisTest =========================================================

    void csOccluvis::MarkAllVisible (AABBVisTreeNode* node, Front2BackData& f2bData)
    {
      if (node->IsLeaf ())
      {
        const int num_objects = node->GetObjectCount ();
        iVisibilityObject** objects = node->GetLeafObjects ();

        // Continue with frustum and other checks.
        for (int i = 0 ; i < num_objects ; i++)
        {
          iVisibilityObject* obj = objects[i];
          f2bData.viscallback->ObjectVisible (obj, obj->GetMeshWrapper (), 0);
        }
      }
      else
      {
        AABBVisTreeNode* child1 = node->GetChild1 ();
        AABBVisTreeNode* child2 = node->GetChild2 ();

        if (child1) MarkAllVisible (child1, f2bData);
        if (child2) MarkAllVisible (child2, f2bData);
      }
    }

    /*------------------------------------------------------------------*/
    /*--------------------------- MAIN DADDY ---------------------------*/
    /*------------------------------------------------------------------*/
    void csOccluvis::VisTest (iRenderView* rview, iVisibilityCullerListener* viscallback)
    {
      // just make sure we have a callback
      if (viscallback == 0)
        return;

      Front2BackData f2bData;
      csRenderContext* ctxt = rview->GetRenderContext ();
      f2bData.frustum = ctxt->clip_planes;
      uint32 frustum_mask = ctxt->clip_planes_mask;

      f2bData.pos = rview->GetCamera ()->GetTransform ().GetOrigin ();
      f2bData.rview = rview;
      f2bData.viscallback = viscallback;

      // Check for the 'all visible' flag
      if (bAllVisible)
      {
        // Mark all visible.
        MarkAllVisible (rootNode, f2bData);
        bAllVisible = false;
        return;
      }

      const csReversibleTransform& camt = rview->GetCamera()->GetTransform ();
      g3d->SetClipper (rview->GetClipper(), CS_CLIPPER_TOPLEVEL);  // We are at top-level.
      g3d->ResetNearPlane ();
      g3d->SetProjectionMatrix (rview->GetCamera()->GetProjectionMatrix ());
      if (!g3d->BeginDraw(rview->GetEngine()->GetBeginDrawFlags() | CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN))
      {
        csPrintf("Cannot prepare renderer for 3D drawing\n");
      }
      g3d->SetWorldToCamera (camt.GetInverse ());
      g3d->SetWriteMask(false,false,false,false);

      // The big routine: traverse from front to back and mark all objects
      // visible that are visible.
      csArray<MeshList*> meshList;
      TraverseNodeF2B<true> (rootNode, true, frustum_mask, f2bData, meshList);
      RenderMeshes<false> (rootNode, f2bData, meshList);

      g3d->SetWriteMask(true,true,true,true);
      g3d->SetClipper (0, CS_CLIPPER_NONE);
      g3d->FinishDraw();
    }

    //======== IntersectSegment ================================================

    struct  Common
    {
      IntersectSegmentFront2BackData *data;
    };

    struct InnerNodeIntersectSegmentSloppy : public Common
    {
      bool operator() (AABBVisTreeNode* n)
      {
        const csBox3& node_bbox = n->GetBBox ();

        // In the first part of this test we are going to test if the
        // start-end vector intersects with the node. If not then we don't
        // need to continue.
        csVector3 box_isect;
        if (csIntersect3::BoxSegment (node_bbox, data->seg, box_isect) == -1)
        {
          return false;
        }
        return true;
      }
    };

    struct LeafNodeIntersectSegmentSloppy : public Common
    {
      uint32 cur_timestamp;
      bool operator() (AABBVisTreeNode* n)
      {
        const csBox3& node_bbox = n->GetBBox ();

        // In the first part of this test we are going to test if the
        // start-end vector intersects with the node. If not then we don't
        // need to continue.
        csVector3 box_isect;
        if (csIntersect3::BoxSegment (node_bbox, data->seg, box_isect) == -1)
        {
          return false;
        }

        int num_objects;
        num_objects = n->GetObjectCount ();
        iVisibilityObject** objects = n->GetLeafObjects();

        for (int i = 0 ; i < num_objects ; i++)
        {
          //if (objects[i]->timestamp != cur_timestamp)
          {
            //objects[i]->timestamp = cur_timestamp;

            // First test the bounding box of the object.
            const csBox3& obj_bbox = node_bbox;

            if (csIntersect3::BoxSegment (obj_bbox, data->seg, box_isect) != -1)
            {
              // This object is possibly intersected by this beam.
	            if (objects[i]->GetMeshWrapper())
	              if (!objects[i]->GetMeshWrapper()->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
	                data->vector->Push (objects[i]);
            }
          }
        }
        return true;
      }
    };

    struct InnerNodeIntersectSegment : public Common
    {
      bool operator() (AABBVisTreeNode* n)
      {
        const csBox3& node_bbox = n->GetBBox ();

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
        return true;
      }
    };

    struct LeafNodeIntersectSegment : public Common
    {
      uint32 cur_timestamp;
      iGraphics3D *g3d;
      bool operator() (AABBVisTreeNode* n)
      {
        const csBox3& node_bbox = n->GetBBox ();

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

        int num_objects=n->GetObjectCount();
        iVisibilityObject** objects = n->GetLeafObjects ();
        for (int i = 0 ; i < num_objects ; i++)
        {
          //if (objects[i]->timestamp != cur_timestamp)
          //if( n->GetQueryData()->uQueryFrame != cur_timestamp )
          {
            //objects[i]->timestamp = cur_timestamp

            iMeshWrapper *mesh = objects[i]->GetMeshWrapper();

            // First test the bounding box of the object.
            const csBox3& obj_bbox = n->GetBBox ();

            if (csIntersect3::BoxSegment (obj_bbox, data->seg, box_isect) != -1)
            {
              // This object is possibly intersected by this beam.
	            if (mesh)
	            {
	              if (!mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
	              {
	                // Transform our vector to object space.
	                csVector3 obj_start;
	                csVector3 obj_end;
	                iMovable* movable = n->GetLeafData(i)->GetMovable ();
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
	                  rc = mesh->GetMeshObject ()->HitBeamObject (
	    	              obj_start, obj_end, obj_isect, &r, &pidx);
	                else
	                  rc = mesh->GetMeshObject ()->HitBeamOutline (
	    	              obj_start, obj_end, obj_isect, &r);
	                if (rc)
	                {
	                  if (data->vector)
	                  {
		                  data->vector->Push (objects[i]);
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
		                  data->mesh = mesh;
	                  }
	                }
	              }
	            }
            }
          }
        }
        return true;
      }
    };

    csPtr<iVisibilityObjectIterator> csOccluvis::IntersectSegment (
        const csVector3& start, const csVector3& end, bool accurate)
    {
      IntersectSegmentFront2BackData data;
      data.seg.Set (start, end);
      data.sqdist = 10000000000.0;
      data.r = 10000000000.;
      data.mesh = 0;
      data.polygon_idx = -1;
      data.vector = new VistestObjectsArray ();
      data.accurate = accurate;

      const csVector3 dir = end-start;
      LeafNodeIntersectSegment leaf;
      InnerNodeIntersectSegment inner;
      leaf.data=&data;
      leaf.cur_timestamp=engine->GetCurrentFrameNumber();
      inner.data=&data;

      TraverseF2B(inner, leaf, dir);

      csOccluvisObjIt* vobjit = new csOccluvisObjIt (data.vector, 0);
      return csPtr<iVisibilityObjectIterator> (vobjit);
    }

    bool csOccluvis::IntersectSegment (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr,
        iMeshWrapper** p_mesh, int* poly_idx,
        bool accurate)
    {
      IntersectSegmentFront2BackData data;
      data.seg.Set (start, end);
      data.sqdist = 10000000000.0;
      data.isect.Set (0, 0, 0);
      data.r = 10000000000.;
      data.mesh = 0;
      data.polygon_idx = -1;
      data.vector = 0;
      data.accurate = accurate;
      data.isect = 0;

      const csVector3 dir = end-start;
      LeafNodeIntersectSegment leaf;
      InnerNodeIntersectSegment inner;
      leaf.data=&data;
      leaf.cur_timestamp=engine->GetCurrentFrameNumber();
      inner.data=&data;

      TraverseF2B(inner, leaf, dir);

      if (p_mesh) *p_mesh = data.mesh;
      if (pr) *pr = data.r;
      if (poly_idx) *poly_idx = data.polygon_idx;
      isect = data.isect;

      return data.mesh != 0;
    }

    csPtr<iVisibilityObjectIterator> csOccluvis::IntersectSegmentSloppy (
        const csVector3& start, const csVector3& end)
    {
      IntersectSegmentFront2BackData data;
      data.seg.Set (start, end);
      data.vector = new VistestObjectsArray ();
      
      const csVector3 dir = end-start;
      LeafNodeIntersectSegmentSloppy leaf;
      InnerNodeIntersectSegmentSloppy inner;
      leaf.data=&data;
      leaf.cur_timestamp=engine->GetCurrentFrameNumber();
      inner.data=&data;

      TraverseF2B(inner, leaf, dir);

      csOccluvisObjIt* vobjit = new csOccluvisObjIt (data.vector, 0);
      return csPtr<iVisibilityObjectIterator> (vobjit);
    }
  }
}
