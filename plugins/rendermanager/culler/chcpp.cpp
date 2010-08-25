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
#include "ivaria/reporter.h"
#include "csplugincommon/rendermanager/viscull.h"
#include "frustvis.h"
#include "chcpp.h"

template<bool bQueryVisibility>
void csFrustumVis::RenderMeshes(NodeTraverseData &ntdNode, csArray<MeshList*> &meshList)
{
  if (bQueryVisibility)
  {
    ntdNode.BeginQuery ();
  }

  for(unsigned int j=0 ; j < meshList.GetSize() ; ++j)
  {
    MeshList& obj = *meshList.Get(j);
    for (int m = 0; m < obj.numMeshes; ++m)
    {
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
    ntdNode.EndQuery ();
  }
}

void csFrustumVis::TraverseNodeF2B(NodeTraverseData &ntdNode,
                                   csArray<MeshList*>& meshList,
                                   bool parentVisible,
                                   bool bDoFrustumCulling)
{
  if (bDoFrustumCulling)
  {
    NodeVisibility nodevis = TestNodeVisibility (ntdNode.kdtNode, &f2bData, ntdNode.u32Frustum_Mask);

    if (nodevis == NODE_INVISIBLE)
      return;

    if (nodevis == NODE_VISIBLE && frustum_mask == 0)
    {
      TraverseNodeF2B (ntdNode, meshList, parentVisible, false);
      return;
    }

    ntdNode.Distribute ();
  }

  if (!ntdNode.IsLeaf())
  {
    csKDTree *firstNode, *secondNode;
    csArray<MeshList*> firstMeshList, secondMeshList;
    bool visible = parentVisible && (ntdNode.GetVisibility () == VISIBLE);

    if (f2bData.pos[ntdNode.GetSplitAxis()] <= ntdNode.GetSplitLocation())
    {
      firstNode = ntdNode.kdtNode->GetChild1 ();
      secondNode = ntdNode.kdtNode->GetChild2 ();
    }
    else
    {
      firstNode = ntdNode.kdtNode->GetChild2 ();
      secondNode = ntdNode.kdtNode->GetChild1 ();
    }

    NodeTraverseData& ntd1 = NodeTraverseData (g3d, firstNode, ntdNode.GetFrustumMask(), ntdNode.GetFrame ());
    NodeTraverseData& ntd2 = NodeTraverseData (g3d, secondNode, ntdNode.GetFrustumMask(), ntdNode.GetFrame ());

    if(firstNode)
    {
      TraverseNodeF2B (ntd1, firstMeshList, visible, bDoFrustumCulling);

      if (!firstMeshList.IsEmpty () && ntd2.GetVisibility () == VISIBLE)
      {
        ++numQueries;
        ++numNormQueries;
        RenderMeshes<true> (ntd1, firstMeshList);
        firstMeshList.Empty ();
      }
    }

    if(secondNode)
    {
      TraverseNodeF2B (ntd2, secondMeshList, visible, bDoFrustumCulling);

      if (!secondMeshList.IsEmpty () && ntd1.GetVisibility () == VISIBLE)
      {
        ++numQueries;
        ++numNormQueries;
        RenderMeshes<true> (ntd2, secondMeshList);
        secondMeshList.Empty ();
      }
    }

    if (visible)
    {
      if (!firstMeshList.IsEmpty () || !secondMeshList.IsEmpty ())
      {
        ++numQueries;
        ++numPullUpQueries;
        ntdNode.BeginQuery ();

        RenderMeshes<false> (ntd1, firstMeshList);
        RenderMeshes<false> (ntd2, secondMeshList);

        ntdNode.EndQuery ();
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
    OcclusionVisibility eOccVis = ntdNode.GetVisibility ();

    const int num_objects = ntdNode.kdtNode->GetObjectCount ();
    csKDTreeChild** objects = ntdNode.kdtNode->GetObjects ();

    for (int i = 0; i < num_objects; ++i)
    {
      if (objects[i]->timestamp != cur_timestamp)
      {
        if (eOccVis == VISIBLE)
        {
          objects[i]->timestamp = cur_timestamp;
        }

        uint32 frustum_mask = ntdNode.GetFrustumMask();
        csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*) objects[i]->GetObject ();

        // Only test an element via occlusion if it first passes frustum testing
        if((!bDoFrustumCulling && !(visobj_wrap->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH)))
          || TestObjectVisibility (visobj_wrap, &f2bData, frustum_mask))
        {
          csSectorVisibleRenderMeshes* sectorMeshList;
          const int numMeshes = f2bData.viscallback->GetVisibleMeshes (visobj_wrap->mesh, frustum_mask, sectorMeshList);

          if (numMeshes > 0)
          {
            meshList.Push (new MeshList (sectorMeshList, numMeshes));

            // If occlusion checks also passed, mark the mesh visible.
            if (parentVisible && eOccVis == VISIBLE)
            {
              ++visible;
              f2bData.viscallback->MarkVisible(visobj_wrap->mesh, numMeshes, sectorMeshList);
            }
          }
        }
      }
    }

    if (!meshList.IsEmpty () && eOccVis == VISIBLE)
    {
      if (ntdNode.CheckVisibility ())
      {
        ++numQueries;
        ++numNormQueries;
        RenderMeshes<true> (ntdNode, meshList);
      }
      else
      {
        RenderMeshes<false> (ntdNode, meshList);
      }

      meshList.Empty ();
    }
  }
}
