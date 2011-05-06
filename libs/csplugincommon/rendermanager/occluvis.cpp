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
#include "csgeom/sphere.h"
#include "csgfx/renderbuffer.h"
#include "csplugincommon/rendermanager/occluvis.h"
#include "cstool/rbuflock.h"
#include "cstool/rendermeshholder.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/rview.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "ivideo/material.h"

namespace CS
{
  namespace RenderManager
  {
    csOccluvis::csVisibilityObjectWrapper::csVisibilityObjectWrapper (csOccluvis* culler, iVisibilityObject* vis_obj)
      : scfImplementationType (this), culler (culler), vis_obj (vis_obj)
    {
      // Recalculate the world bounding box.
      vis_obj->GetMeshWrapper ()->GetWorldBoundingBox ();

      // Get the current culling bbox.
      oldBBox = vis_obj->GetBBox ();
    }

    void csOccluvis::csVisibilityObjectWrapper::ObjectModelChanged (iObjectModel* model)
    {
      // Recalculate the world bounding box.
      vis_obj->GetMeshWrapper ()->GetWorldBoundingBox ();

      // Get the current culling bbox.
      const csBox3& newBBox = vis_obj->GetBBox ();
      culler->MoveObject (vis_obj, oldBBox);
      oldBBox = newBBox;
    }

    void csOccluvis::csVisibilityObjectWrapper::MovableChanged (iMovable* movable)
    {
      // Recalculate the world bounding box.
      vis_obj->GetMeshWrapper ()->GetWorldBoundingBox ();

      // Get the current culling bbox.
      const csBox3& newBBox = vis_obj->GetBBox ();
      culler->MoveObject (vis_obj, oldBBox);
      oldBBox = newBBox;
    }

    void csOccluvis::csVisibilityObjectWrapper::MovableDestroyed (iMovable*)
    {
    }

    template<bool bQueryVisibility>
    void csOccluvis::RenderMeshes(AABBVisTreeNode* node,
                                  iRenderView* rview,
                                  size_t& lastTicket,
                                  iShader*& lastShader,
                                  iShaderVariableContext* shadervars,
                                  NodeMeshList*& nodeMeshList)
    {
      if (bQueryVisibility)
      {
        BeginNodeQuery (node, rview);
      }

      // Save the current zmode.
      csZBufMode oldZMode = g3d->GetZMode ();
      // Also, make sure wireframe is off.
      bool wireframe = g3d->GetEdgeDrawing();
      if (wireframe)
	g3d->SetEdgeDrawing (false);

      for (int iCurrRenderMesh = 0, m = 0; m < nodeMeshList->numMeshes; ++m)
      {
        iMeshWrapper* mw = nodeMeshList->meshList[m].imesh;

        for (int i = 0; i < nodeMeshList->meshList[m].num; ++i, ++iCurrRenderMesh)
        {
          csRenderMesh* rm = nodeMeshList->meshList[m].rmeshes[i];

          // Check whether to draw this RM to the z-buffer or just test against it.
          if (bQueryVisibility)
          {
            g3d->SetZMode (nodeMeshList->onlyTestZ[iCurrRenderMesh] ? CS_ZBUF_TEST : CS_ZBUF_USE);
          }
          else
          {
            // If we're not writing, skip.
            if (nodeMeshList->onlyTestZ[iCurrRenderMesh])
              continue;

            // Else test against and write to the z-buffer.
            g3d->SetZMode (CS_ZBUF_USE);
          }

          // Check for a depth shader to execute.
          iShader* depthShader = nullptr;
          if (rm->material)
          {
            iMaterial* mat = rm->material->GetMaterial ();

            if (nodeMeshList->onlyTestZ[iCurrRenderMesh])
            {
              depthShader = mat->GetShader (depthTestID);
            }
            else
            {
              depthShader = mat->GetShader (depthWriteID);

              if (!depthShader)
              {
                depthShader = mat->GetShader (fbDepthWriteID);
              }
            }

            if (!depthShader)
            {
              // TODO: Come up with some better check or remove check when shader setup perf is better.
              if (shadervars)
              {
                depthShader = shaderMgr->GetShader (defaultShader);
              }
            }
          }

          // Disable the alpha test (since alpha test disables fast GPU depth writing paths).
          // If the alpha mode is smooth, ztest must be set on the mesh.
          // If the alpha mode is binary, the *null depthwrite shader must be used.
          CS::Graphics::RenderMeshModes modes (*rm);
          modes.mixmode &= ~CS_MIXMODE_ALPHATEST_MASK;
          modes.mixmode |= CS_MIXMODE_ALPHATEST_DISABLE;

          if(!rm->portal)
          {
            if (!depthShader)
            {
              csVertexAttrib vA = CS_VATTRIB_POSITION;
              iRenderBuffer *rB = rm->buffers->GetRenderBuffer (CS_BUFFER_POSITION);
              g3d->ActivateBuffers (&vA, &rB, 1);
              g3d->DrawMeshBasic (rm, modes);
              g3d->DeactivateBuffers (&vA, 1);
            }
            else
            {
              // Set up the shadervar stack.
              mw->GetSVContext ()->PushVariables (shaderVarStack);
              if (shadervars) shadervars->PushVariables (shaderVarStack);

              // Get the shader ticket.
              size_t ticket = depthShader->GetTicket (modes, shaderVarStack);

              // Check whether we need to change the shader.
              if (depthShader != lastShader || ticket != lastTicket)
              {
                if (lastShader)
                  lastShader->DeactivatePass (lastTicket);

                // We only support single-pass here (hence the 0).
                if (!depthShader->ActivatePass (ticket, 0))
                  continue;

                lastShader = depthShader;
                lastTicket = ticket;
              }

              // Set up the pass for this mesh.
              if (!depthShader->SetupPass (ticket, rm, modes, shaderVarStack))
                continue;

              // Draw the mesh.
              g3d->DrawMesh (rm, modes, shaderVarStack);

              // Prepare for the next mesh.
              depthShader->TeardownPass (ticket);
              shaderVarStack.Clear ();
            }
          }
          else
          {
            // TODO: We're recalculating the rendermesh here, it would
            // be better to reuse the existing portal rendermesh.. but how?? o.0
            csRenderBufferHolder holder;
            csDirtyAccessArray<csVector2> allPortalVerts2d (64);
            csDirtyAccessArray<csVector3> allPortalVerts3d (64);
            csDirtyAccessArray<size_t> allPortalVertsNums;

            allPortalVerts2d.SetSize (rm->portal->GetTotalVertexCount () * 3);
            allPortalVerts3d.SetSize (rm->portal->GetTotalVertexCount () * 3);
            allPortalVertsNums.SetSize (rm->portal->GetPortalCount ());

            csVector3* portalVerts3d = allPortalVerts3d.GetArray ();
            rm->portal->ComputeScreenPolygons (rview, allPortalVerts2d.GetArray (),
              allPortalVerts3d.GetArray (), rm->portal->GetTotalVertexCount () * 3,
              allPortalVertsNums.GetArray (), rview->GetGraphics3D()->GetWidth (),
              rview->GetGraphics3D()->GetHeight ());

            csRenderMeshHolder rmHolder;
            for (int p = 0; p < rm->portal->GetPortalCount (); ++p)
            {
              size_t count = allPortalVertsNums[p];

              csRef<csRenderBuffer> vertBuffer, indexBuffer;
              vertBuffer = csRenderBuffer::CreateRenderBuffer (count, CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
              indexBuffer = csRenderBuffer::CreateIndexRenderBuffer(count, CS_BUF_STREAM, CS_BUFCOMP_UNSIGNED_INT, 0, 4);
              holder.SetRenderBuffer (CS_BUFFER_POSITION, vertBuffer);
              holder.SetRenderBuffer (CS_BUFFER_INDEX, indexBuffer);

              {
                csRenderBufferLock<csVector3> coords (vertBuffer);
                for (size_t c = 0; c < count; c++)
                  coords[c].Set (portalVerts3d[c]);

                csRenderBufferLock<uint> indices (indexBuffer);
                for (size_t c = 0; c < count; c++)
                  *indices++ = uint (c);
              }

              bool meshCreated;
              csRenderMesh* pViscullRM = rmHolder.GetUnusedMesh (meshCreated, rview->GetCurrentFrameNumber());
              pViscullRM->meshtype = CS_MESHTYPE_TRIANGLEFAN;
              pViscullRM->buffers = &holder;
              pViscullRM->z_buf_mode = CS_ZBUF_TEST;
              pViscullRM->object2world = rview->GetCamera()->GetTransform();
              pViscullRM->indexstart = 0;
              pViscullRM->indexend = uint (count);
              pViscullRM->mixmode &= ~CS_MIXMODE_ALPHATEST_MASK;
              pViscullRM->mixmode |= CS_MIXMODE_ALPHATEST_DISABLE;

              csVertexAttrib vA = CS_VATTRIB_POSITION;
              iRenderBuffer *rB = pViscullRM->buffers->GetRenderBuffer (CS_BUFFER_POSITION);
              g3d->ActivateBuffers (&vA, &rB, 1);
              g3d->DrawMeshBasic (pViscullRM, *pViscullRM);
              g3d->DeactivateBuffers (&vA, 1);

              portalVerts3d += count;
            }
          }
        }
      }

      // Restore the zmode.
      g3d->SetZMode (oldZMode);
      // And wireframe.
      if (wireframe)
	g3d->SetEdgeDrawing (true);

      if (bQueryVisibility)
      {
        g3d->OQEndQuery ();
      }
    }

    template<bool bDoFrustumCulling>
    void csOccluvis::TraverseTreeF2B(AABBVisTreeNode* node,
                                     uint32 frustum_mask,
                                     Front2BackData& f2bData,
                                     csRefArray<NodeMeshList>& meshList)
    {
      if (bDoFrustumCulling)
      {
        NodeVisibility nodevis = TestNodeVisibility (node, f2bData, frustum_mask);

        if (nodevis == NODE_INVISIBLE)
          return;

        if (nodevis == NODE_VISIBLE && frustum_mask == 0)
        {
          TraverseTreeF2B<false> (node, frustum_mask, f2bData, meshList);
          return;
        }
      }

      if (!node->IsLeaf())
      {
        AABBVisTreeNode* frontNode, *backNode;
        GetF2BChildren (node, f2bData, frontNode, backNode);
        TraverseTreeF2B<bDoFrustumCulling> (frontNode, frustum_mask, f2bData, meshList);
        TraverseTreeF2B<bDoFrustumCulling> (backNode, frustum_mask, f2bData, meshList);
      }
      else
      {
        // One object only in this AABBTree
        iVisibilityObject* visobj = node->GetLeafData (0);

        // Only test an object via occlusion if it's not flagged invisible.
        if(!visobj->GetMeshWrapper ()->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
        {
          csSectorVisibleRenderMeshes* sectorMeshList;
          const int numMeshes = f2bData.viscallback->GetVisibleMeshes (visobj->GetMeshWrapper (), frustum_mask, sectorMeshList);

          bool hasMeshes = false;
          for (int i = 0; i < numMeshes; ++i)
          {
            hasMeshes |= (sectorMeshList->num > 0);
          }

          if (hasMeshes)
          {
            VisObjMeshHash& visobjMeshHash = visobjMeshHashes.GetOrCreate (f2bData.rview);
            csRef<NodeMeshList> meshes = visobjMeshHash.Get (csPtrKey<iVisibilityObject> (visobj), csRef<NodeMeshList> ());
            if (!meshes.IsValid ())
            {
              meshes.AttachNew (new NodeMeshList ());
              visobjMeshHash.Put (visobj, meshes);
            }

            // Update the meshes data.
            meshes->node = node;
            meshes->framePassed = engine->GetCurrentFrameNumber ();

            // Resize the meshlist if needed.
            if (!meshes->meshList || meshes->numMeshes != numMeshes)
            {
              for (int m = 0; m < meshes->numMeshes; ++m)
              {
                delete[] meshes->meshList[m].rmeshes;
              }

              delete[] meshes->meshList;
              meshes->meshList = new csSectorVisibleRenderMeshes[numMeshes];
              meshes->numMeshes = numMeshes;

              for (int m = 0; m < numMeshes; ++m)
              {
                // Do a semi-deep copy.
                meshes->meshList[m].imesh = sectorMeshList[m].imesh;
                meshes->meshList[m].num = sectorMeshList[m].num;
                meshes->meshList[m].rmeshes = new csRenderMesh*[sectorMeshList[m].num];
                memcpy (meshes->meshList[m].rmeshes, sectorMeshList[m].rmeshes, sizeof (csRenderMesh*) * sectorMeshList[m].num);
              }
            }
            else
            {
              // Resize the rendermeshes if needed, else just copy.
              for (int m = 0; m < numMeshes; ++m)
              {
                // Do a semi-deep copy.
                if (meshes->meshList[m].num != sectorMeshList[m].num)
                {
                  meshes->meshList[m].num = sectorMeshList[m].num;
                  delete[] meshes->meshList[m].rmeshes;
                  meshes->meshList[m].rmeshes = new csRenderMesh*[sectorMeshList[m].num];
                }

                meshes->meshList[m].imesh = sectorMeshList[m].imesh;
                memcpy (meshes->meshList[m].rmeshes, sectorMeshList[m].rmeshes, sizeof (csRenderMesh*) * sectorMeshList[m].num);
              }
            }

            // We will store per-rendermesh data.
            size_t numRenderMeshes = 0;
            for (int i = 0; i < numMeshes; ++i)
              numRenderMeshes += sectorMeshList[i].num;

            meshes->onlyTestZ.SetSize (numRenderMeshes);

            // Check for the 'always visible' state.
            switch (visobj->GetMeshWrapper ()->GetZBufMode ())
            {
            case CS_ZBUF_NONE:
            case CS_ZBUF_INVERT:
            case CS_ZBUF_FILL:
              {
                meshes->alwaysVisible = true;
                break;
              }
            default:
              {
                meshes->alwaysVisible = visobj->GetMeshWrapper ()->GetFlags ().Check (CS_ENTITY_ALWAYSVISIBLE);
                break;
              }
            }

            // Check for a mesh 'never draw' Z state.
            bool bNeverDrawAny = false;

            // Portals are never drawn to Z.
            if (visobj->GetMeshWrapper ()->GetPortalContainer ())
            {
              bNeverDrawAny = true;
            }
            else
            {
              // Check the z buffer mode of the mesh.
              switch (visobj->GetMeshWrapper ()->GetZBufMode ())
              {
              case CS_ZBUF_NONE:
              case CS_ZBUF_INVERT:
              case CS_ZBUF_TEST:
              case CS_ZBUF_EQUAL:
                {
                  bNeverDrawAny = true;
                  break;
                }
              default:
                break;
              }
            }

            // Check the 'never draw' state of each render mesh.
            for (int iCurrRenderMesh = 0, m = 0; m < numMeshes; ++m)
            {
              // For each render mesh; check if there is a depth-test shader - only test the z-buffer.
              for (int r = 0; r < sectorMeshList[m].num; ++r, ++iCurrRenderMesh)
              {
                bool bOnlyTestZ = bNeverDrawAny;
                if (!bOnlyTestZ)
                {
                  switch (sectorMeshList[m].rmeshes[r]->z_buf_mode)
                  {
                  case CS_ZBUF_NONE:
                  case CS_ZBUF_INVERT:
                  case CS_ZBUF_TEST:
                  case CS_ZBUF_EQUAL:
                    {
                      bOnlyTestZ = true;
                      break;
                    }
                  default:
                    break;
                  }

                  if (!bOnlyTestZ && sectorMeshList[m].rmeshes[r]->material)
                  {
                    iMaterial* mat = sectorMeshList[m].rmeshes[r]->material->GetMaterial ();
                    iShader* depthShader = mat->GetShader (depthWriteID);
                    if (!depthShader)
                    {
                      depthShader = mat->GetShader (fbDepthWriteID);
                    }

                    if (depthShader == shaderMgr->GetShader ("*null"))
                    {
                      bOnlyTestZ = true;
                    }
                  }
                }

                meshes->onlyTestZ.Set (iCurrRenderMesh, bOnlyTestZ);
              }
            }

            meshList.PushSmart (meshes);
          }
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
        queryData->uNextCheck += visibilityFrameSkip * (uint32)nodeMeshHash.GetSize ();
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
      : scfImplementationType (this), object_reg (object_reg),
        shaderVarStack (0,0)
    {
      g3d = csQueryRegistry<iGraphics3D> (object_reg);
      engine = csQueryRegistry<iEngine> (object_reg);
      shaderMgr = csQueryRegistry<iShaderManager> (object_reg);
      stringSet = csQueryRegistryTagInterface<iStringSet> (
        object_reg, "crystalspace.shared.stringset");
      svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
        object_reg, "crystalspace.shader.variablenameset");

      bAllVisible = false;
    }

    csOccluvis::~csOccluvis ()
    {
      csArray<csRefArray<NodeMeshList>*> nodeMeshLists = nodeMeshHash.GetAll ();
      for (size_t i = 0; i < nodeMeshLists.GetSize (); ++i)
      {
        delete nodeMeshLists[i];
      }
    }

    void csOccluvis::Setup (const char* defaultShaderName)
    {
      defaultShader = defaultShaderName;
      depthWriteID = stringSet->Request ("oc_depthwrite");
      depthTestID = stringSet->Request ("oc_depthtest");
      fbDepthWriteID = stringSet->Request ("depthwrite");
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
      CS_ASSERT(visobj);

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

      csArray<csRefArray<NodeMeshList>*> nodeMeshLists = nodeMeshHash.GetAll ();

      VisObjMeshHashes::GlobalIterator itr = visobjMeshHashes.GetIterator ();
      while (itr.HasNext ())
      {
        VisObjMeshHash& visobjMeshHash = itr.Next ();
        csArray<NodeMeshList*> meshLists = visobjMeshHash.GetAll(csPtrKey<iVisibilityObject> (visobj));
        for(size_t i = 0; i < nodeMeshLists.GetSize(); ++i)
        {
          for(size_t j = 0; j < meshLists.GetSize(); ++j)
          {
            nodeMeshLists[i]->Delete(meshLists[j]);
          }
        }

        visobjMeshHash.DeleteAll (csPtrKey<iVisibilityObject> (visobj));
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

    int csOccluvis::NodeMeshListCompare (NodeMeshList* const& object, AABBVisTreeNode* const& key)
    {
      if (object->node == key)
        return 0;

      return 1;
    }

    /*------------------------------------------------------------------*/
    /*--------------------------- MAIN DADDY ---------------------------*/
    /*------------------------------------------------------------------*/
    bool csOccluvis::VisTest (iRenderView* rview, iVisibilityCullerListener* viscallback, int, int)
    {
      csRenderContext* ctxt = rview->GetRenderContext ();
      uint32 frustum_mask = ctxt->clip_planes_mask;

      Front2BackData f2bData;
      f2bData.rview = rview;
      f2bData.viscallback = viscallback;
      f2bData.frustum = ctxt->clip_planes;
      f2bData.pos = rview->GetCamera ()->GetTransform ().GetOrigin ();

      // Set up the shader variable stack and IDs
      shaderVarStack.Setup (svStrings->GetSize ());

      /**
       * If the 'all visible' flag is set, render everything without any culling.
       */
      if (bAllVisible)
      {
        // Mark all visible.
        MarkAllVisible (rootNode, f2bData);
        return false;
      }

      /**
       * Look up the node mesh lists array for this render view.
       * Create a new one if needed.
       */
      csRefArray<NodeMeshList>* nodeMeshListsPtr = nodeMeshHash.Get (csPtrKey<iRenderView> (rview), nullptr);
      if (nodeMeshListsPtr == static_cast<csRefArray<NodeMeshList>*>(nullptr))
      {
        nodeMeshListsPtr = new csRefArray<NodeMeshList> ();
        nodeMeshHash.Put (csPtrKey<iRenderView> (rview), nodeMeshListsPtr);
      }

      /**
       * Traverse the tree approximately front to back and fill the array of visible nodes.
       */
      csRefArray<NodeMeshList>& nodeMeshLists = *nodeMeshListsPtr;
      TraverseTreeF2B<true> (rootNode, frustum_mask, f2bData, nodeMeshLists);

      /**
       * Sort the array F2B.
       */
      F2BSorter sorter (engine, f2bData.pos);
      nodeMeshLists.Sort (sorter);

      /**
       * Iterate in reverse (B2F) over the node list marking visibility.
       */
      for(size_t n = nodeMeshLists.GetSize (); n > 0; --n)
      {
        NodeMeshList*& nodeMeshList = nodeMeshLists[n-1];

        // If frustum checks passed...
        if (nodeMeshList->framePassed == engine->GetCurrentFrameNumber ())
        {
          // If occlusion checks passed...
          if (nodeMeshList->alwaysVisible ||
              GetNodeVisibility (nodeMeshList->node, rview) == VISIBLE)
          {
            // mark the mesh visible.
            iMeshWrapper* mw = nodeMeshList->node->GetLeafData (0)->GetMeshWrapper ();
            f2bData.viscallback->MarkVisible(mw, nodeMeshList->numMeshes,
              nodeMeshList->meshList);
          }
        }
      }

      return true;
    }

    void csOccluvis::RenderViscull (iRenderView* rview, iShaderVariableContext* shadervars)
    {
      // If we're marking all visible this frame, just return.
      if (bAllVisible)
        return;

      /**
       * Look up the node mesh lists array for this render view.
       * Create a new one if needed.
       */
      csRefArray<NodeMeshList>* nodeMeshListsPtr = nodeMeshHash.Get (csPtrKey<iRenderView> (rview), nullptr);
      if (nodeMeshListsPtr == static_cast<csRefArray<NodeMeshList>*>(nullptr))
      {
        nodeMeshListsPtr = new csRefArray<NodeMeshList> ();
        nodeMeshHash.Put (csPtrKey<iRenderView> (rview), nodeMeshListsPtr);
      }

      // Set up g3d for rendering a z-only pass.
      g3d->SetWriteMask (false, false, false, false);

      // The last shader used for occlusion rendering
      iShader* lastShader = 0;

      // The last ticket used for occlusion rendering.
      size_t lastTicket = 0;

      /**
       * Iterate over the node list (F2B) rendering the z-only pass
       * and any occlusion queries.
       */
      csRefArray<NodeMeshList>& nodeMeshLists = *nodeMeshListsPtr;
      for(size_t n = 0; n < nodeMeshLists.GetSize (); ++n)
      {
        NodeMeshList*& nodeMeshList = nodeMeshLists[n];
       
        // If frustum checks passed...
        if (nodeMeshList->framePassed == engine->GetCurrentFrameNumber ())
        {
          if (!nodeMeshList->alwaysVisible &&
              CheckNodeVisibility (nodeMeshList->node, rview))
          {
            // Render with occlusion queries.
            RenderMeshes<true> (nodeMeshList->node, rview, lastTicket, lastShader, shadervars, nodeMeshList);
          }
          else
          {
            // Render without occlusion queries.
            RenderMeshes<false> (nodeMeshList->node, rview, lastTicket, lastShader, shadervars, nodeMeshList);
          }
        }
      }

      // Deactivate the last shader+ticket.
      if (lastShader)
        lastShader->DeactivatePass (lastTicket);

      // Reset rendering settings.
      g3d->SetWriteMask (true, true, true, true);
      
      // In wireframe mode force a depth clear, for the Authentic Wireframe Experience
      if (g3d->GetEdgeDrawing())
	g3d->BeginDraw (g3d->GetCurrentDrawFlags() | CSDRAW_CLEARZBUFFER);
    }

    bool F2BSorter::operator() (csOccluvis::NodeMeshList* const& m1,
                                csOccluvis::NodeMeshList* const& m2)
    {
      iMeshWrapper* mw1 = m1->meshList->imesh;
      iMeshWrapper* mw2 = m2->meshList->imesh;

      // Equal priorities - check distance.
      if (mw1->GetRenderPriority () == mw2->GetRenderPriority ())
      {
        csBox3& m1box = m1->node->GetBBox ();
        csBox3& m2box = m2->node->GetBBox ();

        const float distSqRm1 = m1box.SquaredPosDist (cameraOrigin);
        const float distSqRm2 = m2box.SquaredPosDist (cameraOrigin);

        return distSqRm1 < distSqRm2;
      }

      // Portals get special treatment - always rendered last.
      if (mw1->GetRenderPriority () == portalPriority)
        return false;
      if (mw2->GetRenderPriority () == portalPriority)
        return true;

      // Else order by priority.
      return (mw1->GetRenderPriority () < mw2->GetRenderPriority ());
    }

    //======== VisTest box =====================================================
    void csOccluvis::TraverseTreeBox (AABBVisTreeNode* node,
                                      VistestObjectsArray* voArray,
                                      const csBox3& box)
    {
      if (!box.TestIntersect (node->GetBBox ()))
        return;

      if (!node->IsLeaf())
      {
        TraverseTreeBox (node->GetChild (0), voArray, box);
        TraverseTreeBox (node->GetChild (1), voArray, box);
      }
      else
      {
        voArray->Push (node->GetLeafData (0));
      }
    }

    csPtr<iVisibilityObjectIterator> csOccluvis::VisTest (const csBox3& box)
    {
      VistestObjectsArray* v;
      if (vistest_objects_inuse)
      {
        // Vector is already in use by another iterator. Allocate a new vector.
        v = new VistestObjectsArray ();
      }
      else
      {
        v = &vistest_objects;
        vistest_objects.Empty ();
      }

      TraverseTreeBox (rootNode, v, box);

      csOccluvisObjIt* vobjit = new csOccluvisObjIt (v,
        vistest_objects_inuse ? 0 : &vistest_objects_inuse);

      return csPtr<iVisibilityObjectIterator> (vobjit);
    }

    //======== VisTest sphere ==================================================
    void csOccluvis::TraverseTreeSphere (AABBVisTreeNode* node,
                                         VistestObjectsArray* voArray,
                                         const csVector3& centre,
                                         const float sqradius)
    {
      if (!csIntersect3::BoxSphere (node->GetBBox (), centre, sqradius))
        return;

      if (!node->IsLeaf())
      {
        TraverseTreeSphere (node->GetChild (0), voArray, centre, sqradius);
        TraverseTreeSphere (node->GetChild (1), voArray, centre, sqradius);
      }
      else
      {
        voArray->Push (node->GetLeafData (0));
      }
    }

    csPtr<iVisibilityObjectIterator> csOccluvis::VisTest (const csSphere& sphere)
    {
      VistestObjectsArray* v;
      if (vistest_objects_inuse)
      {
        // Vector is already in use by another iterator. Allocate a new vector.
        v = new VistestObjectsArray ();
      }
      else
      {
        v = &vistest_objects;
        vistest_objects.Empty ();
      }

      TraverseTreeSphere (rootNode, v, sphere.GetCenter (),
        sphere.GetRadius () * sphere.GetRadius ());

      csOccluvisObjIt* vobjit = new csOccluvisObjIt (v,
        vistest_objects_inuse ? 0 : &vistest_objects_inuse);

      return csPtr<iVisibilityObjectIterator> (vobjit);
    }

    void csOccluvis::TraverseTreeSphere (AABBVisTreeNode* node,
                                         iVisibilityCullerListener* viscallback,
                                         const csVector3& centre,
                                         const float sqradius)
    {
      if (!csIntersect3::BoxSphere (node->GetBBox (), centre, sqradius))
        return;

      if (!node->IsLeaf())
      {
        TraverseTreeSphere (node->GetChild (0), viscallback, centre, sqradius);
        TraverseTreeSphere (node->GetChild (1), viscallback, centre, sqradius);
      }
      else
      {
        iVisibilityObject* visobj = node->GetLeafData (0);
        viscallback->ObjectVisible (visobj, visobj->GetMeshWrapper (), 0);
      }
    }

    void csOccluvis::VisTest (const csSphere& sphere, 
                              iVisibilityCullerListener* viscallback)
    {
      TraverseTreeSphere (rootNode, viscallback, sphere.GetCenter (),
        sphere.GetRadius () * sphere.GetRadius ());
    }

    //======== VisTest planes ==================================================
    void csOccluvis::TraverseTreePlanes (AABBVisTreeNode* node,
                                         VistestObjectsArray* voArray,
                                         csPlane3* planes,
                                         uint32 frustum_mask)
    {
      uint32 new_mask;
      if (!csIntersect3::BoxFrustum (node->GetBBox (), planes, frustum_mask, new_mask))
        return;

      if (!node->IsLeaf())
      {
        TraverseTreePlanes (node->GetChild (0), voArray, planes, new_mask);
        TraverseTreePlanes (node->GetChild (1), voArray, planes, new_mask);
      }
      else
      {
        voArray->Push (node->GetLeafData (0));
      }
    }

    csPtr<iVisibilityObjectIterator> csOccluvis::VisTest (csPlane3* planes, int num_planes)
    {
      VistestObjectsArray* v;
      if (vistest_objects_inuse)
      {
        // Vector is already in use by another iterator. Allocate a new vector.
        v = new VistestObjectsArray ();
      }
      else
      {
        v = &vistest_objects;
        vistest_objects.Empty ();
      }

      TraverseTreePlanes (rootNode, v, planes, (1 << num_planes) - 1);

      csOccluvisObjIt* vobjit = new csOccluvisObjIt (v,
        vistest_objects_inuse ? 0 : &vistest_objects_inuse);

      return csPtr<iVisibilityObjectIterator> (vobjit);
    }

    void csOccluvis::TraverseTreePlanes (AABBVisTreeNode* node,
                                         iVisibilityCullerListener* viscallback,
                                         csPlane3* planes,
                                         uint32 frustum_mask)
    {
      uint32 new_mask;
      if (!csIntersect3::BoxFrustum (node->GetBBox (), planes, frustum_mask, new_mask))
        return;

      if (!node->IsLeaf())
      {
        TraverseTreePlanes (node->GetChild (0), viscallback, planes, new_mask);
        TraverseTreePlanes (node->GetChild (1), viscallback, planes, new_mask);
      }
      else
      {
        iVisibilityObject* visobj = node->GetLeafData (0);
        viscallback->ObjectVisible (visobj, visobj->GetMeshWrapper (), 0);
      }
    }

    void csOccluvis::VisTest (csPlane3* planes, int num_planes,
                              iVisibilityCullerListener* viscallback)
    {
      TraverseTreePlanes (rootNode, viscallback, planes, (1 << num_planes) - 1);
    }

    //======== IntersectSegment ================================================

    struct Common
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

    struct LeafNodeIntersectSegmentSloppy : public InnerNodeIntersectSegmentSloppy
    {
      uint32 cur_timestamp;
      bool operator() (AABBVisTreeNode* n)
      {
        // first test node intersection
        if(!InnerNodeIntersectSegmentSloppy::operator()(n))
        {
          return true;
        }

        int num_objects = n->GetObjectCount ();
        iVisibilityObject** objects = n->GetLeafObjects();
        csVector3 box_isect;

        bool hit = false;
        for (int i = 0 ; i < num_objects ; i++)
        {
          //if (objects[i]->timestamp != cur_timestamp)
          {
            //objects[i]->timestamp = cur_timestamp;

            // validate this is a proper mesh
            iMeshWrapper* mesh = objects[i]->GetMeshWrapper();
            if (!mesh || mesh->GetFlags().Check(CS_ENTITY_NOHITBEAM))
            {
              continue;
            }

            // First test the bounding box of the object.
            const csBox3& obj_bbox = objects[i]->GetBBox();

            if (csIntersect3::BoxSegment(obj_bbox, data->seg, box_isect) != -1)
            {
              // This object is possibly intersected by this beam.
	      data->vector->Push (objects[i]);
	      hit = true;
            }
          }
        }
        return !hit;
      }
    };

    struct InnerNodeIntersectSegment : public Common
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

        // If mesh != 0 then we have already found our mesh. In that
        // case we will compare the distance of the origin with the the
        // box of the treenode and the already found shortest distance to
        // see if we have to proceed.
        if (data->mesh)
        {
          float sqdist = csSquaredDist::PointPoint(data->seg.Start(), box_isect);
          if (sqdist > data->sqdist)
          {
            return false;
          }
        }

        return true;
      }
    };

    struct LeafNodeIntersectSegment : public InnerNodeIntersectSegment
    {
      uint32 cur_timestamp;
      iGraphics3D *g3d;
      bool operator() (AABBVisTreeNode* n)
      {
        // test node intersection and distance to best found match
        if(!InnerNodeIntersectSegment::operator()(n))
        {
            return true;
        }

        int num_objects = n->GetObjectCount();
        iVisibilityObject** objects = n->GetLeafObjects ();

        bool hit = false;
        csVector3 obj_isect;
        for (int i = 0 ; i < num_objects ; i++)
        {
          //if (objects[i]->timestamp != cur_timestamp)
          //if( n->GetQueryData()->uQueryFrame != cur_timestamp )
          {
            //objects[i]->timestamp = cur_timestamp

            // check whether we have a proper mesh here
            iMeshWrapper *mesh = objects[i]->GetMeshWrapper();
            if(!mesh || mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM))
            {
              continue;
            }

            // First test the bounding box of the object.
            const csBox3& obj_bbox = objects[i]->GetBBox ();

            if (csIntersect3::BoxSegment (obj_bbox, data->seg, obj_isect) != -1)
            {
              // This object is possibly intersected by this beam.

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
              float r;

              bool rc;
              int pidx = -1;
              if (data->accurate)
              {
                rc = mesh->GetMeshObject ()->HitBeamObject (
                     obj_start, obj_end, obj_isect, &r, &pidx);
              }
              else
              {
                rc = mesh->GetMeshObject ()->HitBeamOutline (
                     obj_start, obj_end, obj_isect, &r);
              }

              if (rc)
              {
                if (data->vector)
                {
                  hit = true;
                  data->vector->Push (objects[i]);
                }
                else if (r < data->r)
                {
                  hit = true;
                  data->r = r;
                  data->polygon_idx = pidx;
                  if (identity)
                  {
                    data->isect = obj_isect;
                  }
                  else
                  {
                    data->isect = movtrans.This2Other(obj_isect);
                  }

                  data->sqdist = csSquaredDist::PointPoint (data->seg.Start(),
                                                            data->isect);
                  data->mesh = mesh;
                }
              }
            }
          }
        }
        return !hit;
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