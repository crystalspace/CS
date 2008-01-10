/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_VISCULL_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_VISCULL_H__

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "iengine/viscull.h"

namespace CS
{
namespace RenderManager
{
  // Forward declare the functions

  template<typename RenderTree>
  void AddRenderMeshToContext (typename RenderTree::ContextNode& context, 
    csRenderMesh* rm, CS::Graphics::RenderPriority renderPrio,
    typename RenderTree::MeshNode::SingleMesh& singleMeshTemplate);

  template<typename RenderTree>
  bool Viscull (typename RenderTree::ContextNode& context, RenderView* rw, 
    iVisibilityCuller* culler);

  namespace Implementation
  {
    /**
     * Helper class implementing the visculler callback functionality
     * and adds rendermeshes via AddRenderMeshToContext function
     */
    template<typename RenderTree>
    class ViscullCallback : 
      public scfImplementation1<ViscullCallback<RenderTree>, iVisibilityCullerListener>
    {
    public:
      //---- Types
      typedef RenderTree RenderTreeType;
      typedef typename RenderTreeType::ContextNode ContextNodeType;

      ViscullCallback (ContextNodeType& context, RenderView* currentRenderView, 
        CS::Utility::MeshFilter* filter)
        : scfImplementation1<ViscullCallback, iVisibilityCullerListener> (this), 
        context (context), currentRenderView (currentRenderView),
        filter (filter)
      {}


      virtual void ObjectVisible (iVisibilityObject *visobject, 
        iMeshWrapper *imesh, uint32 frustum_mask)
      {        
        if (!(filter && filter->IsMeshFiltered (imesh)))
        {
          // Todo: Handle static lod & draw distance
          csZBufMode zmode = imesh->GetZBufMode ();
          CS::Graphics::RenderPriority renderPrio = imesh->GetRenderPriority ();

          // Get the meshes
          int numMeshes;
          csRenderMesh** meshList = imesh->GetRenderMeshes (numMeshes, currentRenderView, frustum_mask);
#ifdef CS_DEBUG
          const char* const db_mesh_name = imesh->QueryObject()->GetName();
#endif

          typename RenderTreeType::MeshNode::SingleMesh sm;
          sm.meshObjSVs = imesh->GetSVContext();
          sm.zmode = zmode;

          // Add it to the appropriate meshnode
          for (int i = 0; i < numMeshes; ++i)
          {
            csRenderMesh* rm = meshList[i];
#ifdef CS_DEBUG
            rm->db_mesh_name = db_mesh_name;
#endif

            if (rm->portal)
            {
#ifdef CS_DEBUG
              typename ContextNodeType::PortalHolder h = {db_mesh_name, rm->portal, imesh};
#else
              typename ContextNodeType::PortalHolder h = {rm->portal, imesh};
#endif
              context.allPortals.Push (h);              
            }
            else
            {
              AddRenderMeshToContext<RenderTreeType> (context, rm, renderPrio, sm);
            }            
          }
        }
      }

    private:      
      ContextNodeType& context;
      RenderView* currentRenderView;
      CS::Utility::MeshFilter* filter;
    };
  }
  
  /**
   * Use the given renderview and visibility culler to perform a culling step,
   * add all found meshes to the given context.
   */
  template<typename RenderTree>
  bool Viscull (typename RenderTree::ContextNode& context, RenderView* rw, 
    iVisibilityCuller* culler)
  {
    CS::Utility::MeshFilter* filter = &rw->GetMeshFilter();
    CS::RenderManager::Implementation::ViscullCallback<RenderTree> cb (context, rw, filter);

    culler->VisTest (rw, &cb);

    return true;
  }

  /**
   * Add a rendermesh to context, putting it in the right meshnode etc.
   */
  template<typename RenderTree>
  void AddRenderMeshToContext (typename RenderTree::ContextNode& context, 
    csRenderMesh* rm, CS::Graphics::RenderPriority renderPrio,
    typename RenderTree::MeshNode::SingleMesh& singleMeshTemplate)
  {
    typename RenderTree::TreeTraitsType::MeshNodeKeyType meshKey = 
      RenderTree::TreeTraitsType::GetMeshNodeKey (renderPrio, *rm);
    
    RenderTree& tree = context.owner;

    // Get the mesh node
    typename RenderTree::MeshNode* meshNode = context.meshNodes.Get (meshKey, 0);
    if (!meshNode)
    {
      // Get a new one
      meshNode = tree.CreateMeshNode (context, meshKey);

      RenderTree::TreeTraitsType::SetupMeshNode(*meshNode, renderPrio, *rm);
      context.meshNodes.Put (meshKey, meshNode);
    }

    csRef<csShaderVariable> svObjectToWorld;
    svObjectToWorld.AttachNew (new csShaderVariable (
      tree.GetPersistentData ().svObjectToWorldName));
    svObjectToWorld->SetValue (rm->object2world);

    typename RenderTree::MeshNode::SingleMesh sm (singleMeshTemplate);
    sm.renderMesh = rm;
    sm.svObjectToWorld = svObjectToWorld;
    if (rm->z_buf_mode != (csZBufMode)~0) 
      sm.zmode = rm->z_buf_mode;

    meshNode->meshes.Push (sm);
    context.totalRenderMeshes++;
  }
 
}
}



#endif
