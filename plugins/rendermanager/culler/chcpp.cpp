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
#include "imesh/object.h"
#include "iutil/object.h"
#include "ivaria/reporter.h"
#include "frustvis.h"
#include "chcpp.h"

int csFrustumVis::GetFinishedQuery(OccQuery &oq)
{
  unsigned int i;
  int q=0;
  CHCList<OccQuery>::Iterator it;
  OccQuery oc;

  it=CHCList<OccQuery>::Iterator(Q_Queue);
  for(q=0;q<Q_Queue.Size();++q,++it)
  {
    if(it.HasCurrent()) oc=it.FetchCurrent();
    for(i=0;i<(*it).numQueries;i++)
    {
      if(g3d->OQueryFinished((*it).qID[i]))
      {
        if(g3d->OQIsVisible((*it).qID[i],VISIBILITY_THRESHOLD))
        {
          oq=*it;
          Q_Queue.Delete(oq);
          return 1; // return visible result from query
        }
        else
        {
          oq=*it;
          Q_Queue.Delete(oq);;
          return -1; // return non visible result from query
        }
      }
    }
  }
  return 0; // no queries were found while searching
}

void csFrustumVis::IssueQueries(iRenderView* rview,csArray<csKDTreeChild*> &objArray)
{
  int numq=1;
  //printf("Start\n");
  for(unsigned int i=0 ; i<objArray.GetSize() ; i++)
  {
    int numMeshes=0;
    iMeshWrapper* const mw=static_cast<csFrustVisObjectWrapper*>(objArray.Get(i)->GetObject())->mesh;
    const uint32 frust_mask=rview->GetRenderContext ()->clip_planes_mask;

    unsigned int *queries;
    queries=new unsigned int;
    g3d->OQInitQueries(queries,numq);
    g3d->OQBeginQuery(*queries);
    
    csRenderMesh **rmeshes=mw->GetRenderMeshes(numMeshes,rview,frust_mask);
    for (int m = 0; m < numMeshes; m++)
    {
      if (!rmeshes[m]->portal)
      {
        csVertexAttrib vA=CS_VATTRIB_POSITION;
        iRenderBuffer *rB=rmeshes[m]->buffers->GetRenderBuffer(CS_BUFFER_POSITION);
        g3d->ActivateBuffers(&vA,&rB,1);
        g3d->DrawMeshBasic(rmeshes[m],*rmeshes[m]);
        g3d->DeactivateBuffers(&vA,1);
      }
    }
    g3d->OQEndQuery();

    OccQuery oc;
    oc.qID=queries;
    oc.numQueries=1;
    Q_Queue.PushBack(oc);
  }
}

void csFrustumVis::QueryPreviouslyInvisibleNode(NodeTraverseData &ntdNode)
{
  I_Queue.PushBack(ntdNode);
  if(I_Queue.Size()>=PREV_INV_BATCH_SIZE)
  {
    // here we'll issue multi queries
  }
}

/* Pulls up the visibility */
void csFrustumVis::PullUpVisibility(const NodeTraverseData &ntdNode)
{
  NodeTraverseData ntdAux=ntdNode;
  while(ntdAux.kdtParent && !ntdAux.GetVisibility())
  {
    ntdAux.SetVisibility(true);
    ntdAux.kdtNode=ntdAux.kdtParent;
    ntdAux.kdtParent=ntdAux.kdtParent->GetParent();
  }
}

void csFrustumVis::TraverseNode(iRenderView* rview, NodeTraverseData &ntdNode,
                                const csVector3& pos, const int cur_timestamp)
{
  ntdNode.SetTimestamp(cur_timestamp);
  if (ntdNode.IsLeaf()) // if node is leaf we render it
  {
    const int num_objects = ntdNode.kdtNode->GetObjectCount ();
    csKDTreeChild** objects = ntdNode.kdtNode->GetObjects ();
    csArray<csKDTreeChild*> objArray(10);

    for (int i = 0 ; i < num_objects ; i++)
    {
      if (objects[i]->timestamp != cur_timestamp)
      {
        objects[i]->timestamp = cur_timestamp;
        csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	  objects[i]->GetObject ();
        // only test an element via occlusion if it first passes frustum testing
        if(TestObjectVisibility (visobj_wrap, &f2bData, ntdNode.GetFrustumMask()))
        {
          objArray.Push(objects[i]);
        }
      }
    }
    if(objArray.GetSize())
      IssueQueries(rview,objArray);
  }
  else // else we queue its children on to the traverse queue
  {
    ntdNode.SetVisibility(false);
    csKDTree* child1 = ntdNode.kdtNode->GetChild1 ();
    csKDTree* child2 = ntdNode.kdtNode->GetChild2 ();
    if (pos[ntdNode.GetSplitAxis()] <= ntdNode.GetSplitLocation())
    {
      if(child1)
        T_Queue.PushBack(NodeTraverseData(child1,ntdNode.kdtNode,ntdNode.GetFrustumMask()));
      if(child2)
        T_Queue.PushBack(NodeTraverseData(child2,ntdNode.kdtNode,ntdNode.GetFrustumMask()));
    }
    else
    {
      if(child2)
        T_Queue.PushBack(NodeTraverseData(child2,ntdNode.kdtNode,ntdNode.GetFrustumMask()));
      if(child1)
        T_Queue.PushBack(NodeTraverseData(child1,ntdNode.kdtNode,ntdNode.GetFrustumMask()));
    }
  }
}

bool csFrustumVis::WasVisible(NodeTraverseData &ntdNode,const int cur_timestamp) const
{
  return (ntdNode.GetVisibility() && ntdNode.GetTimestamp()!=cur_timestamp);
}
