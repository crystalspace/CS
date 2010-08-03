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

void csFrustumVis::IssueQueries(NodeTraverseData &ntdNode, csArray<MeshList*> &objArray)
{
  ntdNode.BeginQuery ();

  for(unsigned int j=0 ; j < objArray.GetSize() ; ++j)
  {
    MeshList& obj = *objArray.Get(j);
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

  ntdNode.EndQuery ();
}

void csFrustumVis::TraverseNode(NodeTraverseData &ntdNode, const int cur_timestamp)
{
  ntdNode.SetTimestamp(cur_timestamp);

  if (ntdNode.IsLeaf())
  {
    OcclusionVisibility eOccVis = ntdNode.GetVisibility ();

    // FIXME
    while (eOccVis == UNKNOWN)
    {
      eOccVis = ntdNode.GetVisibility ();
    }

    const int num_objects = ntdNode.kdtNode->GetObjectCount ();
    csKDTreeChild** objects = ntdNode.kdtNode->GetObjects ();
    csArray<MeshList*> objArray;

    for (int i = 0; i < num_objects; ++i)
    {
      if (objects[i]->timestamp != cur_timestamp)
      {
        objects[i]->timestamp = cur_timestamp;
        uint32 frustum_mask = ntdNode.GetFrustumMask();
        csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*) objects[i]->GetObject ();

        // Only test an element via occlusion if it first passes frustum testing
        if(TestObjectVisibility (visobj_wrap, &f2bData, frustum_mask))
        {
          csSectorVisibleRenderMeshes* meshList;
          const int numMeshes = f2bData.viscallback->GetVisibleMeshes (visobj_wrap->mesh, frustum_mask, meshList);

          if (numMeshes > 0)
          {
            objArray.Push(new MeshList(meshList, numMeshes));

            // If occlusion checks also passed, mark the mesh visible.
            if (eOccVis == VISIBLE)
            {
              f2bData.viscallback->MarkVisible(visobj_wrap->mesh, numMeshes, meshList);
            }
          }
        }
      }
    }

    if (!objArray.IsEmpty())
    {
      IssueQueries(ntdNode, objArray);
    }
  }
  else // else we queue its children on to the traverse queue
  {
    NodeTraverseData ntd;
    csKDTree* child1 = ntdNode.kdtNode->GetChild1 ();
    csKDTree* child2 = ntdNode.kdtNode->GetChild2 ();
    
    if (f2bData.pos[ntdNode.GetSplitAxis()] <= ntdNode.GetSplitLocation())
    {
      if(child1)
      {
        T_Queue.Push (NodeTraverseData (g3d, child1, ntdNode.kdtNode, ntdNode.GetFrustumMask(), cur_timestamp));
      }

      if(child2)
      {
        T_Queue.Push (NodeTraverseData (g3d, child2, ntdNode.kdtNode, ntdNode.GetFrustumMask(), cur_timestamp));
      }
    }
    else
    {
      if(child2)
      {
        T_Queue.Push (NodeTraverseData (g3d, child2, ntdNode.kdtNode, ntdNode.GetFrustumMask(), cur_timestamp));
      }

      if(child1)
      {
        T_Queue.Push (NodeTraverseData (g3d, child1, ntdNode.kdtNode, ntdNode.GetFrustumMask(), cur_timestamp));
      }
    }
  }
}
