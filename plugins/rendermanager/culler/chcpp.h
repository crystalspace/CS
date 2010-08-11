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
public:
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

class LeafTraverse
{
public:
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

/*------------------- Occlusion query based visibility culling ----------------------*/

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
    csBox3 box=n->GetBBox();
    if(box.In(f2bData->pos) || CheckOQ(n))
    {
      n->SetCameraTimestamp(f2bData->rview->GetCamera(),f2bData->current_timestamp);
      n->SetVisibilityForCamera(f2bData->rview->GetCamera(),false);
      DrawQuery(n);
      return true;
    }
    DrawQuery(n);
    return false;
  }

  bool operator() (NodePtr n, uint32 &frustum_mask) const
  {
    CS_ASSERT_MSG("Invalid AABB-tree", !n->IsLeaf ());
    csBox3 node_bbox = n->GetBBox();
    node_bbox *= f2bData->global_bbox;

    if (node_bbox.Contains (f2bData->pos))
    {
      return NodeVisible(n); // node completely visible
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

    /*csBox3 box=n->GetLeafData(0)->GetBBox();
    if(n->GetGraphics3D()->OQIsVisible(n->GetLeafData(0)->GetQueryLeafID(),0))
    {
      csPrintf("+BB Visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      //csPrintf("BB Visible\n");
    }
    else
    {
      csPrintf("+BB Not visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
          box.MinX(),box.MinY(),box.MinZ(),
          box.MaxX(),box.MaxY(),box.MaxZ());
      //csPrintf("BB Not visible\n");
    }*/
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

  bool CheckLeafOQ(const NodePtr n) const
  {
    const unsigned int oqID=n->GetLeafData(0)->GetQueryLeafID();
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
    const uint32 frust_mask=f2bData->rview->GetRenderContext ()->clip_planes_mask;
    csSectorVisibleRenderMeshes* meshList;
    const int numMeshes = f2bData->viscallback->GetVisibleMeshes(mw,frustum_mask,meshList);
    if(numMeshes > 0 )
    {
      const bool isp=IsRMPortal(n,meshList,numMeshes); // see if it's a portal
      //if(isp || CheckOQ(n))
      {
        if(isp || CheckLeafOQ(n))
        {
          f2bData->viscallback->MarkVisible(n->GetLeafData(0)->mesh, numMeshes, meshList);
        }
        if(!isp)
          DrawLeafQuery(n,meshList,numMeshes);
      }
      //else
      {
        //csPrintf("+BB Not visible (%.2f %.2f %.2f) (%.2f %.2f %.2f)\n",
        //  box.MinX(),box.MinY(),box.MinZ(),
        //  box.MaxX(),box.MaxY(),box.MaxZ());
        //printf("BB not visible\n");
      }
      //DrawQuery(n);
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

#endif
