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

/**\file
 * Render manager visibility culling
 */

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "iengine/viscull.h"

namespace CS
{
namespace RenderManager
{
  // Forward declare the functions

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
        const CS::Utility::MeshFilter* filter)
        : scfImplementation1<ViscullCallback, iVisibilityCullerListener> (this), 
        context (context), currentRenderView (currentRenderView),
	sector (currentRenderView->GetThisSector()), filter (filter)
      {}


      virtual void ObjectVisible (iVisibilityObject *visobject, 
        iMeshWrapper *imesh, uint32 frustum_mask)
	  {
        if (!(filter && filter->IsMeshFiltered (imesh)))
        {
          // Get the meshes
          int numMeshes;
          csSectorVisibleRenderMeshes* meshList = sector->GetVisibleRenderMeshes (
	            numMeshes,
	            imesh, currentRenderView, frustum_mask);

          for (int m = 0; m < numMeshes; ++m)
          {
	          // Todo: Handle static lod & draw distance
	          csZBufMode zmode = meshList[m].imesh->GetZBufMode ();
	          CS::Graphics::RenderPriority renderPrio =
	            meshList[m].imesh->GetRenderPriority ();
  
        #ifdef CS_DEBUG
	          const char* const db_mesh_name = meshList[m].imesh->QueryObject()->GetName();
        #endif
	          typename RenderTreeType::MeshNode::SingleMesh sm;
	          sm.meshWrapper = meshList[m].imesh;
	          sm.meshObjSVs = meshList[m].imesh->GetSVContext();
	          sm.zmode = zmode;
	          sm.meshFlags = meshList[m].imesh->GetFlags();
	    
	          // Add it to the appropriate meshnode
	          for (int i = 0; i < meshList[m].num; ++i)
	          {
	            csRenderMesh* rm = meshList[m].rmeshes[i];
  
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
		            context.AddRenderMesh (rm, renderPrio, sm);
	            }
	          }
	        }
	      }
      } // end objectvisible

    virtual int MarkVisible (
        iMeshWrapper *imesh, uint32 frustum_mask,csSectorVisibleRenderMeshes *& meshList)
	  {
        if (!(filter && filter->IsMeshFiltered (imesh)))
        {
          // Get the meshes
          int numMeshes;
          meshList = sector->GetVisibleRenderMeshes (
	            numMeshes,
	            imesh, currentRenderView, frustum_mask);
          for (int m = 0; m < numMeshes; ++m)
          {
	          // Todo: Handle static lod & draw distance
	          csZBufMode zmode = meshList[m].imesh->GetZBufMode ();
	          CS::Graphics::RenderPriority renderPrio =
	            meshList[m].imesh->GetRenderPriority ();
  
        #ifdef CS_DEBUG
	          const char* const db_mesh_name = meshList[m].imesh->QueryObject()->GetName();
        #endif
	          typename RenderTreeType::MeshNode::SingleMesh sm;
	          sm.meshWrapper = meshList[m].imesh;
	          sm.meshObjSVs = meshList[m].imesh->GetSVContext();
	          sm.zmode = zmode;
	          sm.meshFlags = meshList[m].imesh->GetFlags();
	    
	          // Add it to the appropriate meshnode
	          for (int i = 0; i < meshList[m].num; ++i)
	          {
	            csRenderMesh* rm = meshList[m].rmeshes[i];
  
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
		            context.AddRenderMesh (rm, renderPrio, sm);
	            }
	          }
	        }
          return numMeshes;
        }
        else
          return 0;
      } // end objectvisible

    private:      
      ContextNodeType& context;
      RenderView* currentRenderView;
      iSector* sector;
      const CS::Utility::MeshFilter* filter;
    };
  }
  
  /**
   * Use the given renderview and visibility culler to perform a culling step,
   * add all found meshes to the given context.
   *
   * Usually the first thing done with a view and context. Example:
   * \code
   * // Renderview+sector setup
   * sector->PrepareDraw (renderView);
   * // Make sure the clip-planes are ok
   * CS::RenderViewClipper::SetupClipPlanes (renderView->GetRenderContext ());
   * 
   * // Do the culling
   * iVisibilityCuller* culler = sector->GetVisibilityCuller ();
   * Viscull<RenderTree> (context, renderView, culler);
   * \endcode
   */
  template<typename RenderTree>
  bool Viscull (typename RenderTree::ContextNode& context, RenderView* rw, 
    iVisibilityCuller* culler)
  {
    const CS::Utility::MeshFilter* filter = &rw->GetMeshFilter();
    CS::RenderManager::Implementation::ViscullCallback<RenderTree> cb (context, rw, filter);

    int renderW = 0, renderH = 0;
    context.GetTargetDimensions (renderW, renderH);
    culler->VisTest (rw, &cb, renderW, renderH);

    return true;
  }
 
}
}



#endif
