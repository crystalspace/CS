/*
    Copyright (C) 2010 by Joe Forte

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DEFERREDVISCULL_H__
#define __DEFERREDVISCULL_H__

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "iengine/viscull.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

  /**
   * Returns true of the given mesh uses alpha blending (is transparent).
   */
  bool IsAlphaBlended(csRenderMesh *rm)
  {
    // TODO: Make more thorough.
    return rm->mixmode != CS_FX_COPY;
  }

  /**
   * Helper class implementing the visculler callback functionality
   * for the deferred render manager.
   */
  template<typename RenderTree>
  class DeferredViscullCallback : 
    public scfImplementation1<DeferredViscullCallback<RenderTree>, iVisibilityCullerListener>
  {
  public:
    //---- Types
    typedef RenderTree RenderTreeType;
    typedef typename RenderTreeType::ContextNode ContextNodeType;

    DeferredViscullCallback(ContextNodeType &context, 
                            CS::RenderManager::RenderView *currentRenderView, 
                            const CS::Utility::MeshFilter *filter)
      : 
    scfImplementation1<DeferredViscullCallback, iVisibilityCullerListener>(this), 
    context(context), 
    currentRenderView(currentRenderView),
    sector(currentRenderView->GetThisSector ()), 
    filter(filter)
    {}


    virtual void ObjectVisible(iVisibilityObject *visobject, 
                               iMeshWrapper *imesh, 
                               uint32 frustum_mask)
    {
      if (!(filter && filter->IsMeshFiltered (imesh)))
      {
        // Get the meshes
        int numMeshes;
        csSectorVisibleRenderMeshes *meshList = sector->GetVisibleRenderMeshes (
	      numMeshes, imesh, currentRenderView, frustum_mask);

        for (int m = 0; m < numMeshes; ++m)
        {
          // Todo: Handle static lod & draw distance
          csZBufMode zmode = meshList[m].imesh->GetZBufMode ();
          CS::Graphics::RenderPriority renderPrio = 
            meshList[m].imesh->GetRenderPriority ();

#ifdef CS_DEBUG
          const char *db_mesh_name = meshList[m].imesh->QueryObject()->GetName();
#endif
          typename RenderTreeType::MeshNode::SingleMesh sm;
          sm.meshWrapper = meshList[m].imesh;
          sm.meshObjSVs = meshList[m].imesh->GetSVContext();
          sm.zmode = zmode;
          sm.meshFlags = meshList[m].imesh->GetFlags();
	    
          // Add it to the appropriate meshnode
          for (int i = 0; i < meshList[m].num; ++i)
          {
            csRenderMesh *rm = meshList[m].rmeshes[i];

            if (rm->portal)
            {
#ifdef CS_DEBUG
              typename ContextNodeType::PortalHolder h = { db_mesh_name, rm->portal, imesh };
#else
              typename ContextNodeType::PortalHolder h = { rm->portal, imesh };
#endif
              context.allPortals.Push (h);              
            }
            else if (!IsAlphaBlended (rm))
            {
              context.AddRenderMesh (rm, renderPrio, sm);
            }
          }
        }
      }
    }

  private:

    ContextNodeType &context;
    CS::RenderManager::RenderView *currentRenderView;
    iSector *sector;
    const CS::Utility::MeshFilter *filter;
  };
  
  /**
   * Use the given renderview, visibility culler, and viscull callback 
   * to perform a culling step and add all found meshes to the given context.
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
   * ViscullCallback<RenderTree> callback (...);
   * CustomCallbackViscull<RenderTree> (context, renderView, culler, &callback);
   * \endcode
   */
  template<typename RenderTree>
  bool CustomCallbackViscull(typename RenderTree::ContextNode &context,
                             CS::RenderManager::RenderView *rw, 
                             iVisibilityCuller *culler, 
                             iVisibilityCullerListener *callback)
  {
    const CS::Utility::MeshFilter *filter = &rw->GetMeshFilter();

    int renderW = 0, renderH = 0;
    context.GetTargetDimensions (renderW, renderH);
    culler->VisTest (rw, callback, renderW, renderH);

    return true;
  }
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERREDVISCULL_H__
