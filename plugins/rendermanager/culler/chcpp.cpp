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

static void ConstructBBoxMesh(csBox3 &box,csSimpleRenderMesh &srm,csRenderMeshType rmtype,csZBufMode zbuf)
{
  csVector3* verts = 0;
  csVector4* cols = 0;

  verts=new csVector3[25];
  cols=new csVector4[25];

  verts[0]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
  verts[1]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
  verts[2]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
  verts[3]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());

  verts[4]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
  verts[5]=csVector3(box.MinX(),box.MinY(),box.MinZ());
  verts[6]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
  verts[7]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());

  verts[8]=csVector3(box.MinX(),box.MinY(),box.MinZ());
  verts[9]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
  verts[10]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());
  verts[11]=csVector3(box.MinX(),box.MaxY(),box.MinZ());

  verts[12]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
  verts[13]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
  verts[14]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());
  verts[15]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());

  if(rmtype==CS_MESHTYPE_QUADS)
  {
    verts[16]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
    verts[17]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());
    verts[18]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
    verts[19]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());

    verts[20]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
    verts[21]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
    verts[22]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
    verts[23]=csVector3(box.MinX(),box.MinY(),box.MinZ());
  }
  else
  {
    verts[16]=csVector3(box.MinX(),box.MinY(),box.MinZ());
    verts[17]=csVector3(box.MinX(),box.MaxY(),box.MinZ());
    verts[18]=csVector3(box.MinX(),box.MinY(),box.MaxZ());
    verts[19]=csVector3(box.MinX(),box.MaxY(),box.MaxZ());

    verts[20]=csVector3(box.MaxX(),box.MinY(),box.MinZ());
    verts[21]=csVector3(box.MaxX(),box.MaxY(),box.MinZ());
    verts[22]=csVector3(box.MaxX(),box.MinY(),box.MaxZ());
    verts[23]=csVector3(box.MaxX(),box.MaxY(),box.MaxZ());
  }
  cols[0]=csVector4(0.0f,1.0f,0.0f);
  for(int i=1;i<24; ++i)
  {
    cols[i]=cols[0];
  }

  srm.vertices=verts;
  srm.colors=cols;
  srm.vertexCount=24;

  srm.meshtype = rmtype;
  csAlphaMode alf;
  alf.alphaType = alf.alphaSmooth;
  alf.autoAlphaMode = false;
  srm.alphaType = alf;
  srm.z_buf_mode=zbuf;
}

static void ReleaseBBoxMesh(csSimpleRenderMesh &srm)
{
  delete [] srm.vertices;
  delete [] srm.colors;
}

int csFrustumVis::GetFinishedQuery(OccQuery &oq)
{
  unsigned int i=0;
  int q=0;
  CHCList<OccQuery>::Iterator it;
  OccQuery oc;

  oc=Q_Queue.Front();
  if(g3d->OQueryFinished(oc.qID[0]))
  {
    Q_Queue.PopFront();
    if(g3d->OQIsVisible(oc.qID[0],VISIBILITY_THRESHOLD))
    {
      oq=oc;
      return 1; // return visible result from query
    }
    else
    {
      oq=oc;
      return -1; // return non visible result from query
    }
  }
  /*it=CHCList<OccQuery>::Iterator(Q_Queue);
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
          Q_Queue.Delete(oq);
          return -1; // return non visible result from query
        }
      }
    }
  }*/
  return 0; // no queries were found while searching
}

void csFrustumVis::IssueQueries(NodeTraverseData &ntdNode, csArray<ObjectRecord> &objArray)
{
  int numq=1;
  unsigned int *queries;
  queries=new unsigned int;
  g3d->OQInitQueries(queries,numq);
  g3d->OQBeginQuery(*queries);
  for(unsigned int j=0 ; j<objArray.GetSize() ; j++)
  {
    ObjectRecord obj=static_cast<ObjectRecord>(objArray.Get(j));
    for (int m = 0; m < obj.numMeshes; ++m)
    {
      for (int i = 0; i < obj.meshList[m].num; ++i)
	    {
        csRenderMesh* rm = obj.meshList[m].rmeshes[i];
        if(!rm->portal)
        {
          csVertexAttrib vA=CS_VATTRIB_POSITION;
          iRenderBuffer *rB=rm->buffers->GetRenderBuffer(CS_BUFFER_POSITION);
          g3d->ActivateBuffers(&vA,&rB,1);
          g3d->DrawMeshBasic(rm,*rm);
          g3d->DeactivateBuffers(&vA,1);
        }
      }
    }
    /*int numMeshes=0;
    iMeshWrapper* const mw=static_cast<csFrustVisObjectWrapper*>(objArray.Get(i)->GetObject())->mesh;
    const uint32 frust_mask=f2bData.rview->GetRenderContext ()->clip_planes_mask;
    
    csRenderMesh **rmeshes=mw->GetRenderMeshes(numMeshes,f2bData.rview,frust_mask);
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
    }*/
  }
  g3d->OQEndQuery();
  OccQuery oc;
  oc.qID=queries;
  oc.numQueries=1;
  oc.ntdNode=ntdNode;
  Q_Queue.PushBack(oc);
}

void csFrustumVis::RenderQuery(NodeTraverseData &ntdNode,bool bUseBB)
{
  if(ntdNode.IsLeaf())
  {
    int num_objects=ntdNode.kdtNode->GetObjectCount();
    csKDTreeChild **objects=ntdNode.kdtNode->GetObjects();
    for(int i=0;i<num_objects;i++)
    {
      iMeshWrapper* const mw=static_cast<csFrustVisObjectWrapper*>(objects[i]->GetObject())->mesh;
      if(bUseBB)
      {
        csSimpleRenderMesh srm;
        csBox3 bbox=mw->GetWorldBoundingBox();
        ConstructBBoxMesh(bbox,srm,CS_MESHTYPE_QUADS,CS_ZBUF_USE);
        g3d->DrawSimpleMesh(srm);
        ReleaseBBoxMesh(srm);
      }
      else
      {
        int numMeshes=0;
        const uint32 frust_mask=f2bData.rview->GetRenderContext ()->clip_planes_mask;
    
        csRenderMesh **rmeshes=mw->GetRenderMeshes(numMeshes,f2bData.rview,frust_mask);
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
      }
    }
  }
  else
  {
    /*csSimpleRenderMesh srm;
    csBox3 bbox=ntdNode.kdtNode->GetNodeBBox();
    ConstructBBoxMesh(bbox,srm,CS_MESHTYPE_QUADS,CS_ZBUF_USE);
    g3d->DrawSimpleMesh(srm,0,true);
    ReleaseBBoxMesh(srm);*/
    csKDTree* child1 = ntdNode.kdtNode->GetChild1 ();
    csKDTree* child2 = ntdNode.kdtNode->GetChild2 ();
    NodeTraverseData ntd;
    if(child2)
    {
      ntd=NodeTraverseData(child2,ntdNode.kdtNode,ntdNode.GetFrustumMask(),
                             ntdNode.IsCompletelyVisible());
      RenderQuery(ntd,bUseBB);
    }
    if(child1)
    {
      ntd=NodeTraverseData(child1,ntdNode.kdtNode,ntdNode.GetFrustumMask(),
                             ntdNode.IsCompletelyVisible());
      RenderQuery(ntd,bUseBB);
    }
  }
}

unsigned int csFrustumVis::IssueSingleQuery(NodeTraverseData &ntdNode,bool bUseBB)
{
  int numq=1;
  unsigned int *queries;
  queries=new unsigned int;
  g3d->OQInitQueries(queries,numq);
  g3d->OQBeginQuery(*queries);

  RenderQuery(ntdNode,bUseBB);
  
  g3d->OQEndQuery();
  OccQuery oc;
  oc.qID=queries;
  oc.numQueries=1;
  oc.ntdNode=ntdNode;
  Q_Queue.PushBack(oc);
  return *oc.qID;
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

void csFrustumVis::TraverseNode(NodeTraverseData &ntdNode, const int cur_timestamp)
{
  if (ntdNode.IsLeaf()) // if node is leaf we render it
  {
    const int num_objects = ntdNode.kdtNode->GetObjectCount ();
    csKDTreeChild** objects = ntdNode.kdtNode->GetObjects ();
    csArray<ObjectRecord> objArray(10);

    for (int i = 0 ; i < num_objects ; i++)
    {
      if (objects[i]->timestamp != cur_timestamp)
      {
        objects[i]->timestamp = cur_timestamp;
        csFrustVisObjectWrapper* visobj_wrap = (csFrustVisObjectWrapper*)
      	  objects[i]->GetObject ();
        ObjectRecord obj;
        // only test an element via occlusion if it first passes frustum testing
        if(TestObjectVisibility (visobj_wrap, &f2bData, ntdNode.GetFrustumMask(),obj))
        {
          if(obj.numMeshes) // don't add records that don't have anything to draw
            objArray.Push(obj);
        }
      }
    }
    //if(!objArray.IsEmpty())
    //  IssueQueries(ntdNode, objArray);
    //IssueSingleQuery(ntdNode);
  }
  else // else we queue its children on to the traverse queue
  {
    csKDTree* child1 = ntdNode.kdtNode->GetChild1 ();
    csKDTree* child2 = ntdNode.kdtNode->GetChild2 ();
    NodeTraverseData ntd;
    if (f2bData.pos[ntdNode.GetSplitAxis()] <= ntdNode.GetSplitLocation())
    {
      if(child1)
      {
        ntd=NodeTraverseData(child1,ntdNode.kdtNode,ntdNode.GetFrustumMask(),
                             ntdNode.IsCompletelyVisible());
        T_Queue.PushBack(ntd);
      }
      if(child2)
      {
        ntd=NodeTraverseData(child2,ntdNode.kdtNode,ntdNode.GetFrustumMask(),
                              ntdNode.IsCompletelyVisible());
        T_Queue.PushBack(ntd);
      }
    }
    else
    {
      if(child2)
      {
        ntd=NodeTraverseData(child2,ntdNode.kdtNode,ntdNode.GetFrustumMask(),
                             ntdNode.IsCompletelyVisible());
        T_Queue.PushBack(ntd);
      }
      if(child1)
      {
        ntd=NodeTraverseData(child1,ntdNode.kdtNode,ntdNode.GetFrustumMask(),
                             ntdNode.IsCompletelyVisible());
        T_Queue.PushBack(ntd);
      }
    }
  }
}

bool csFrustumVis::WasVisible(const NodeTraverseData &ntdNode,const int cur_timestamp) const
{
  return (ntdNode.GetVisibility() && ntdNode.GetTimestamp()!=cur_timestamp);
}
