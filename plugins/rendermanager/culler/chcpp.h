#ifndef _CHCPP_H_
#define _CHCPP_H_

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include <csutil/list.h>
#include "frustvis.h"

// empirically robust constant (might need tweaking)
#define PREV_INV_BATCH_SIZE 25
// visibility threshold parameter
#define VISIBILITY_THRESHOLD 0

// macro for toggling between frustvis and oocclusion query culling
//#define USE_FRUSTVIS

/*-------------------------------- Frustum culling -------------------------------------*/

struct InnerTraverse
{
  InnerTraverse()
  {
  }
  InnerTraverse(const FrustTest_Front2BackData *data)
  {
    f2bData=data;
  }

  const FrustTest_Front2BackData *f2bData;

  bool operator() (const NodePtr n, uint32 &frustum_mask) const
  {
    CS_ASSERT_MSG("Invalid AABB-tree", !n->IsLeaf ());
    csBox3 node_bbox = n->GetBBox();
    node_bbox *= f2bData->global_bbox;

    if (node_bbox.Contains (f2bData->pos))
    {
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

    return true; // node visible
  }
};

struct LeafTraverse
{
  LeafTraverse()
  {
  }
  LeafTraverse(const FrustTest_Front2BackData *data)
  {
    f2bData=data;
  }

  const FrustTest_Front2BackData *f2bData;

  void NodeVisible(const NodePtr n,const uint32 frustum_mask) const
  {
    n->SetCameraTimestamp(f2bData->rview->GetCamera(),f2bData->current_timestamp);
    n->SetVisibilityForCamera(f2bData->rview->GetCamera(),false);

    csBox3 box=n->GetBBox();
    iMeshWrapper* const mw=n->GetLeafData(0)->mesh;
    const uint32 frust_mask=f2bData->rview->GetRenderContext ()->clip_planes_mask;
    csSectorVisibleRenderMeshes* meshList;
    const int numMeshes = f2bData->viscallback->GetVisibleMeshes(mw,frustum_mask,meshList);
    if(numMeshes > 0 )
    {
      f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
    }
  }

  bool operator() (const NodePtr n,const uint32 frustum_mask) const
  { 
    CS_ASSERT_MSG("Invalid AABB-tree", n->IsLeaf ());
    const int num_objects=n->GetObjectCount();
    const NodeLeafData* nld = n->GetLeafData(0);

    if (nld->mesh && nld->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
    {
      return true; // node invisible, but continue traversal
    }

    const csBox3& obj_bbox = nld->GetBBox ();
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

/*------------------- Occlusion query based visibility culling ----------------------*/

struct Common
{
  std::queue<const NodePtr> Queries;
  const FrustTest_Front2BackData *f2bData;

  void PullUpVisibility(const NodePtr n) const
  {
    NodePtr naux=n;
    while(naux->GetParent())
    {
      naux->SetVisibilityForCamera(f2bData->rview->GetCamera(),true);
      naux=naux->GetParent();
    }
    naux->SetVisibilityForCamera(f2bData->rview->GetCamera(),true);
  }
};

struct InnerNodeProcessOP : public Common
{
  InnerNodeProcessOP()
  {
  }
  InnerNodeProcessOP(const FrustTest_Front2BackData *data,const std::queue<const NodePtr> &Queries)
  {
    f2bData=data;
    this->Queries=Queries;
  }

  void DrawQuery(const NodePtr n) const
  {
    n->GetGraphics3D()->OQBeginQuery(n->GetQueryID());
    n->GetGraphics3D()->DrawSimpleMesh(n->srmSimpRendMesh);
    n->GetGraphics3D()->OQEndQuery();
    /*csBox3 box=n->GetBBox();
    if(box.In(f2bData->pos) || n->GetGraphics3D()->OQIsVisible(n->GetQueryID(),0))
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
    }*/
  }

  bool IsQueryFinished(const NodePtr n) const
  {
    return n->GetGraphics3D()->OQueryFinished(n->GetQueryID());
  }

  bool CheckOQ(const NodePtr n) const
  {
    const unsigned int oqID=n->GetQueryID();
    if(oqID)
    {
      return n->GetGraphics3D()->OQIsVisible(oqID,0);
    }
    return true;
  }

  bool NodeVisible(NodePtr n) const
  {
    n->SetCameraTimestamp(f2bData->rview->GetCamera(),f2bData->current_timestamp);
    n->SetVisibilityForCamera(f2bData->rview->GetCamera(),false);
    csBox3 box=n->GetBBox();
    return true;
  }

  bool operator() (NodePtr n, uint32 &frustum_mask) const
  {
    CS_ASSERT_MSG("Invalid AABB-tree", !n->IsLeaf ());
    csBox3 node_bbox = n->GetBBox();
    node_bbox *= f2bData->global_bbox;

    if (node_bbox.Contains (f2bData->pos))
    {
      return NodeVisible(n); // node completely in frustum
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

    return NodeVisible(n); // node visible
  }
};


struct LeafNodeProcessOP : public Common
{
  LeafNodeProcessOP()
  {
  }
  LeafNodeProcessOP(const FrustTest_Front2BackData *data,const std::queue<const NodePtr> &Queries)
  {
    f2bData=data;
    this->Queries=Queries;
  }

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

  void DrawQuery(const NodePtr n) const
  {
    n->GetGraphics3D()->OQBeginQuery(n->GetQueryID());
    n->GetGraphics3D()->DrawSimpleMesh(n->srmSimpRendMesh);
    n->GetGraphics3D()->OQEndQuery();
    /*csBox3 box=n->GetLeafData(0)->GetBBox();
    if(n->GetGraphics3D()->OQIsVisible(n->GetQueryID(),0))
    {
      csPrintf("-BB Visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      csPrintf("BB Visible\n");
    }
    else
    {
      csPrintf("-BB Not visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      csPrintf("BB Not visible\n");
    }*/
  }

  void DrawLeafQuery(const NodePtr n,const csSectorVisibleRenderMeshes* meshList,const int numMeshes) const
  {
    n->GetGraphics3D()->OQBeginQuery(n->GetLeafData(0)->GetQueryLeafID());
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

  bool IsQueryFinished(const NodePtr n,const unsigned int oqID) const
  {
    return n->GetGraphics3D()->OQueryFinished(oqID);
  }

  bool CheckOQ(const NodePtr n,const unsigned int oqID) const
  {
    if(oqID)
    {
      return n->GetGraphics3D()->OQIsVisible(oqID,0);
    }
    return true;
  }

  void NodeVisible(const NodePtr n,uint32 frustum_mask) const
  {
    n->SetCameraTimestamp(f2bData->rview->GetCamera(),f2bData->current_timestamp);
    n->SetVisibilityForCamera(f2bData->rview->GetCamera(),false);

    csBox3 box=n->GetBBox();
    /*csPrintf("Leaf BB (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());*/
    iMeshWrapper* const mw=n->GetLeafData(0)->mesh;
    csSectorVisibleRenderMeshes* meshList;
    const uint32 frust_mask=f2bData->rview->GetRenderContext ()->clip_planes_mask;
    const int numMeshes = f2bData->viscallback->GetVisibleMeshes(mw,frustum_mask,meshList);
    if(numMeshes > 0 )
    {
      const bool isp=IsRMPortal(n,meshList,numMeshes); // see if it's a portal
      const unsigned int oqID=n->GetLeafData(0)->GetQueryLeafID();
      {
        if(isp)
        {
          f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
        }
        else if(IsQueryFinished(n,oqID))
        {
          if(CheckOQ(n,oqID))
            f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
          DrawLeafQuery(n,meshList,numMeshes);
        }
        else
          f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
      }
    }
  }

  bool operator() (NodePtr n,const uint32 frustum_mask) const
  { 
    CS_ASSERT_MSG("Invalid AABB-tree", n->IsLeaf ());
    const int num_objects=n->GetObjectCount();
    const NodeLeafData* nld = n->GetLeafData(0);

    if (nld->mesh && nld->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
    {
      return true; // node invisible, but continue traversal
    }

    const csBox3& obj_bbox = nld->GetBBox ();
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


struct TraverseFunctor : Common
{
  TraverseFunctor()
  {
  }
  TraverseFunctor(const FrustTest_Front2BackData *data, const csVector3& dir)
  {
    f2bData=data;
    direction=dir;
  }

  csVector3 direction;

  void DrawBBoxQuery(const NodePtr n) const
  {
    n->GetGraphics3D()->OQBeginQuery(n->GetQueryID());
    n->GetGraphics3D()->DrawSimpleMesh(n->srmSimpRendMesh);
    n->GetGraphics3D()->OQEndQuery();
    /*csBox3 box=n->GetLeafData(0)->GetBBox();
    if(n->GetGraphics3D()->OQIsVisible(n->GetQueryID(),0))
    {
      csPrintf("-BB Visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      csPrintf("BB Visible\n");
    }
    else
    {
      csPrintf("-BB Not visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      csPrintf("BB Not visible\n");
    }*/
  }

  void DrawLeafQuery(const NodePtr n,const csSectorVisibleRenderMeshes* meshList,const int numMeshes) const
  {
    n->GetGraphics3D()->OQBeginQuery(n->GetLeafData(0)->GetQueryLeafID());
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

  bool IsQueryFinished(const NodePtr n,const unsigned int oqID) const
  {
    return n->GetGraphics3D()->OQueryFinished(oqID);
  }

  bool CheckOQ(const NodePtr n,const unsigned int oqID) const
  {
    if(oqID)
    {
      return n->GetGraphics3D()->OQIsVisible(oqID,0);
    }
    return true;
  }

  inline bool inner(const NodePtr n, uint32 &frustum_mask) const
  {
    CS_ASSERT_MSG("Invalid AABB-tree", !n->IsLeaf ());
    csBox3 node_bbox = n->GetBBox();
    node_bbox *= f2bData->global_bbox;

    if (node_bbox.Contains (f2bData->pos))
    {
      return true; // node completely in frustum
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

    return true; // node visible
  }

  void LeafNodeVisible(const NodePtr n,const uint32 frustum_mask) const
  {
    csBox3 box=n->GetBBox();
    /*csPrintf("Leaf BB (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());*/
    iMeshWrapper* const mw=n->GetLeafData(0)->mesh;
    csSectorVisibleRenderMeshes* meshList;
    const uint32 frust_mask=f2bData->rview->GetRenderContext ()->clip_planes_mask;
    const int numMeshes = f2bData->viscallback->GetVisibleMeshes(mw,frustum_mask,meshList);
    if(numMeshes > 0 )
    {
      const bool isp=IsRMPortal(n,meshList,numMeshes); // see if it's a portal
      const unsigned int oqID=n->GetLeafData(0)->GetQueryLeafID();
      {
        if(isp)
        {
          f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
        }
        else if(IsQueryFinished(n,oqID))
        {
          if(CheckOQ(n,oqID))
            f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
          DrawLeafQuery(n,meshList,numMeshes);
        }
        else
          f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
      }
    }
  }

  inline bool leaf(const NodePtr n,const uint32 frustum_mask) const
  { 
    CS_ASSERT_MSG("Invalid AABB-tree", n->IsLeaf ());
    const int num_objects=n->GetObjectCount();
    const NodeLeafData* nld = n->GetLeafData(0);

    if (nld->mesh && nld->mesh->GetFlags ().Check (CS_ENTITY_INVISIBLEMESH))
    {
      return false; // node invisible
    }

    const csBox3& obj_bbox = nld->GetBBox ();
    if (obj_bbox.Contains (f2bData->pos))
    {
      return true;
    }
  
    uint32 new_mask;
    if (!csIntersect3::BoxFrustum (obj_bbox, f2bData->frustum, frustum_mask, new_mask))
    {
      return false; // node invisible
    }

    return true;
  }

  inline bool ResultAvailable(const NodePtr n) const
  {
    switch(n->IsLeaf())
    {
      case true:
        return n->GetLeafData(0)->LeafQueryFinished();
      default:
        return n->InnerQueryFinished();
    }
  }

  inline bool InsideViewFrustum(const NodePtr n, uint32 &frustum_mask) const
  {
    switch(n->IsLeaf())
    {
      case true:
        return leaf(n, frustum_mask);
      default:
        return inner(n, frustum_mask);
    }
  }

  inline void TraverseInner(const NodePtr n, const csVector3& direction, std::stack<const NodePtr>& Q) const
  {
    const csVector3 centerDiff = n->GetChild2 ()->GetBBox ().GetCenter () -
       n->GetChild1 ()->GetBBox ().GetCenter ();
    const size_t firstIdx = (centerDiff * direction > 0) ? 0 : 1;

    Q.push(n->GetChild (1-firstIdx));
    Q.push(n->GetChild (firstIdx));
  }

  bool operator() (const NodePtr rootNode,uint32 frustum_mask) const
  {
    bool ret = true;
    if (!rootNode) 
      return ret;
    std::stack<const NodePtr> Q;
    std::queue<const NodePtr> OccQueries;
    Q.push(rootNode);
    while(!Q.empty() || !OccQueries.empty())
    {
      while(!OccQueries.empty() && 
            (ResultAvailable(OccQueries.front()) || Q.empty()) )
      {
        const NodePtr n=OccQueries.front();
        OccQueries.pop();
        const unsigned int oqID=n->IsLeaf()?n->GetLeafData(0)->GetQueryLeafID():n->GetQueryID();
        if(CheckOQ(n,oqID))
        {
          PullUpVisibility(n);
          if(n->IsLeaf())
          {
            iMeshWrapper* const mw=n->GetLeafData(0)->mesh;
            csSectorVisibleRenderMeshes* meshList;
            const uint32 frust_mask=f2bData->rview->GetRenderContext ()->clip_planes_mask;
            const int numMeshes = f2bData->viscallback->GetVisibleMeshes(mw,frustum_mask,meshList);
            if(numMeshes > 0 )
            {
              f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
            }
          }
          else
          {
            TraverseInner(n,direction,Q);
          }
        }
      }
      
      while(!Q.empty())
      {
        const NodePtr n=Q.top();
        const bool bIsLeaf=n->IsLeaf();
        Q.pop();
        if(InsideViewFrustum(n,frustum_mask))
        {
          iCamera* cam=f2bData->rview->GetCamera();
          const bool bWasVisible=n->GetVisibilityForCamera(cam) &&
            (n->GetCameraTimestamp(cam)<f2bData->current_timestamp);
          const bool bLeafOrWasInvisible=(!bWasVisible || bIsLeaf);

          n->SetCameraTimestamp(f2bData->rview->GetCamera(),f2bData->current_timestamp);
          n->SetVisibilityForCamera(f2bData->rview->GetCamera(),false);


          if (bIsLeaf)
          {
            iMeshWrapper* const mw=n->GetLeafData(0)->mesh;
            csSectorVisibleRenderMeshes* meshList;
            const uint32 frust_mask=f2bData->rview->GetRenderContext ()->clip_planes_mask;
            const int numMeshes = f2bData->viscallback->GetVisibleMeshes(mw,frust_mask,meshList);
            if(numMeshes > 0 )
            {
              const bool isp=IsRMPortal(n,meshList,numMeshes); // see if it's a portal
              const unsigned int oqID=n->GetLeafData(0)->GetQueryLeafID();
              if(isp)
              {
                f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
              }
              else
              {
                DrawLeafQuery(n,meshList,numMeshes);
                OccQueries.push(n);
                if(bWasVisible)
                {
                  f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
                }
              }
            }
          }
          else
          {
            if(!bWasVisible)
            {
              const csBox3 box=n->GetBBox();
              csPrintf("Issuing BB (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
                box.MinX(),box.MinY(),box.MinZ(),
                box.MaxX(),box.MaxY(),box.MaxZ());
              DrawBBoxQuery(n);
              OccQueries.push(n);
            }
            else
            {
              TraverseInner(n,direction,Q);
            }
          }
        }
      }
    }
    return ret;
  }

  ~TraverseFunctor()
  {
  }
};

#endif
