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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_CONTEXT_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_CONTEXT_H__

#include "iengine/sector.h"

#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"

namespace CS
{
namespace RenderManager
{
  template<typename RenderTreeType>
  class ContextSetup
  {
  public:
    ContextSetup (iShaderManager* shaderManager, iShader* defaultShader, 
      csStringID defaultShaderName)
      : shaderManager (shaderManager), 
        defaultShader (defaultShader), defaultShaderName (defaultShaderName),
        recurseCount (0)
    {
  
    }
  
    void operator() (RenderTreeType& renderTree, 
      typename RenderTreeType::ContextNode* context, 
      typename RenderTreeType::ContextsContainer* container, 
      iSector* sector, CS::RenderManager::RenderView* rview)
    {
      // @@@ FIXME: Of course, don't hardcode.
      if (recurseCount > 5) return;
    
      int numSectorCB = sector->GetSectorCallbackCount ();
      while (numSectorCB-- > 0)
      {
        iSectorCallback* cb = sector->GetSectorCallback (numSectorCB);
        cb->Traverse (sector, rview);
      }

      // Do the culling
      iVisibilityCuller* culler = sector->GetVisibilityCuller ();
      renderTree.Viscull (container, context, rview, culler);
  
      // Sort the mesh lists  
      {
	StandardMeshSorter<RenderTreeType> mySorter (rview->GetEngine (), rview->GetCamera ());
	renderTree.TraverseMeshNodes (mySorter, context);
      }
  
      // Setup the SV arrays
      // Push the default stuff
      SetupStandardSVs<RenderTreeType> (*context, shaderManager, sector);
  
      // Setup the material&mesh SVs
      {
	StandardSVSetup<RenderTreeType> svSetup (context->svArrays);
	renderTree.TraverseMeshNodes (svSetup, context);
      }
  
      // Setup shaders and tickets
      SetupStandarShaderAndTicket (renderTree, *context, shaderManager, 
	defaultShaderName, defaultShader);
  
      csDirtyAccessArray<csVector2> allPortalVerts2d (64);
      csDirtyAccessArray<size_t> allPortalVertsNums;
      // Handle all portals
      for (size_t pc = 0; pc < context->allPortals.GetSize (); ++pc)
      {
	typename RenderTreeType::ContextNode::PortalHolder& holder = context->allPortals[pc];
	const size_t portalCount = holder.portalContainer->GetPortalCount ();
	
	size_t allPortalVertices = holder.portalContainer->GetTotalVertexCount ();
	allPortalVerts2d.SetSize (allPortalVertices * 3);
	allPortalVertsNums.SetSize (portalCount);
	
	holder.portalContainer->ComputeScreenPolygons (rview, 
	  allPortalVerts2d.GetArray(), allPortalVerts2d.GetSize(), 
	  allPortalVertsNums.GetArray());
	csVector2* portalVerts2d = allPortalVerts2d.GetArray();
  
	for (size_t pi = 0; pi < portalCount; ++pi)
	{
	  iPortal* portal = holder.portalContainer->GetPortal (pi);
  
	  if (IsSimplePortal (portal))
	  {
	    // Finish up the sector
	    if (!portal->CompleteSector (rview))
	      continue;
  
            size_t orgCount = allPortalVertsNums[pi];
            if (orgCount == 0) continue;
	    // Setup a bounding box, in screen-space
	    csBox2 screenBox;
	    for (size_t i = 0; i < orgCount; ++i)
	    {
	      screenBox.AddBoundingVertex (portalVerts2d[i]);
	    }
	    
	    size_t count = orgCount;
	    if ((rview->GetClipper ()->ClipInPlace (portalVerts2d, count, screenBox) == CS_CLIP_OUTSIDE) ||
	      count == 0)
	      continue;
  
	    // Setup simple portal
	    rview->CreateRenderContext ();
	    rview->SetLastPortal (portal);
	    rview->SetPreviousSector (sector);
	    csPolygonClipper newView (portalVerts2d, count);
	    rview->SetClipper (&newView);
  
	    typename RenderTreeType::ContextNode* portalCtx = 
              renderTree.CreateContext (container, rview);
  
            recurseCount++;
	    // Setup the new context
	    (*this)(renderTree, portalCtx, container, portal->GetSector (), rview);
	    recurseCount--;
  
	    rview->RestoreRenderContext ();
	    portalVerts2d += orgCount;
	  }
	  else
	  {
	    // Setup heavy portal @@TODO
	  }
	}
      }
    }
  
  
  private:
    bool IsSimplePortal (iPortal* portal)
    {
      return true;
    }
  
    iShaderManager* shaderManager;
    iShader* defaultShader;
    csStringID defaultShaderName;
    int recurseCount;
  };
  
  template<typename RenderTreeType>
  class SetupRenderTarget
  {
  public:
    SetupRenderTarget (typename RenderTreeType::ContextsContainer* contexts,
      iGraphics3D* g3d)
    {
      g3d->SetRenderTarget (contexts->renderTarget, false,
        contexts->subtexture);
    }
  };
    
  template<typename RenderTreeType>
  class ContextRender
  {
  public:
    ContextRender (iShaderManager* shaderManager)
      : shaderManager (shaderManager)
    {
    }
  
    void operator() (typename RenderTreeType::ContextsContainer* contexts, 
      RenderTreeType& tree)
    {
      iView* view = contexts->view;
      iGraphics3D* g3d = view->GetContext ();
      int drawFlags = view->GetEngine ()->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS;

      SetupRenderTarget<RenderTreeType> setupTarget (contexts, g3d);
      iCamera* cam = view->GetCamera();
      g3d->SetPerspectiveCenter (int (cam->GetShiftX ()), 
        int (cam->GetShiftY ()));
      g3d->SetPerspectiveAspect (cam->GetFOV ());
      
      BeginFinishDrawScope bd (g3d, drawFlags);

      g3d->SetWorldToCamera (cam->GetTransform ().GetInverse ());

      ContextCB cb (*this, g3d);
      tree.TraverseContextsReverse (contexts, cb);
    }
  
  private:
    template<typename Fn>
    struct MeshNodeCB
    {
      MeshNodeCB(Fn& meshNodeFunction, typename RenderTreeType::ContextNode* node, RenderTreeType& tree)
	: meshNodeFunction (meshNodeFunction), node (node), tree (tree)
      {}
  
      void operator() (const typename RenderTreeType::TreeTraitsType::MeshNodeKeyType& key, 
	typename RenderTreeType::MeshNode* meshNode)
      {
	meshNodeFunction (key, meshNode, *node, tree);
      }
  
      Fn& meshNodeFunction;
      typename RenderTreeType::ContextNode* node;
      RenderTreeType& tree;
    };

    struct ContextCB
    {
      ContextRender& parent;
      iGraphics3D* g3d;

      ContextCB (ContextRender& parent, iGraphics3D* g3d) : parent (parent),
        g3d (g3d) {}

      void operator() (typename RenderTreeType::ContextNode* node, 
        RenderTreeType& tree)
      {
        SimpleRender<RenderTreeType> render (g3d, 
          parent.shaderManager->GetShaderVariableStack ());
    
        MeshNodeCB<SimpleRender<RenderTreeType> > cb (render, node, tree);
        node->meshNodes.TraverseInOrder (cb);
      }
    };

    iShaderManager* shaderManager;
  };

} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_CONTEXT_H__
