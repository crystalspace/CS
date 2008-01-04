/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDER_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDER_H__

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/svarrayholder.h"


namespace CS
{
namespace RenderManager
{
  /**
   * Scope wrapper for setting BeginDraw flags and calling FinishDraw on scope
   * end.
   */
  class BeginFinishDrawScope
  {
  public:
    BeginFinishDrawScope (iGraphics3D* g3d, int drawFlags, bool finishOnEnd = true)
      : g3d (g3d), finishOnEnd (finishOnEnd)
    {
      g3d->BeginDraw (drawFlags);
    }

    ~BeginFinishDrawScope ()
    {
      if (finishOnEnd)
        g3d->FinishDraw ();
    }

  private:
    iGraphics3D* g3d;
    bool finishOnEnd;
  };


  /**
   * Scope wrapper for setting render target from context settings.
   */
  template<typename ContextType>
  class ContextTargetSetup
  {
  public:
    ContextTargetSetup (iGraphics3D* g3d)
      : g3d (g3d)
    {
    }

    ~ContextTargetSetup ()
    {
      g3d->FinishDraw ();
    }

    void operator() (const ContextType& context)
    {
      g3d->FinishDraw ();
      g3d->SetRenderTarget (context.renderTarget, false, context.subtexture);
    }

  private:
    iGraphics3D* g3d;
  };

  /**
   * Render mesh nodes within one context.
   * Does not handle setting render targets or camera transform.
   */
  template<typename RenderTree, typename LayerConfigType>
  class SimpleContextRender
  {
  public:
    SimpleContextRender (iGraphics3D* g3d, iShaderManager* shaderMgr)
      : g3d (g3d), shaderMgr (shaderMgr), currentLayer (0)
    {}
    
    void SetLayer (size_t layer)
    {
      currentLayer = layer;
    }

    void operator() (typename RenderTree::MeshNode* node)
    {
      typename RenderTree::ContextNode& context = node->owner;
      const size_t layerOffset = context.totalRenderMeshes*currentLayer;

      iShader* lastShader = 0;
      size_t lastTicket = ~0;
      size_t lastRenderedMesh = 0;

      for (size_t m = 0; m < node->meshes.GetSize (); ++m)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes.Get (m);
        iShader* shader = context.shaderArray[mesh.contextLocalId+layerOffset];
        
        // Skip meshes without shader (for current layer)
        if (!shader)
          continue;

        size_t ticket = context.ticketArray[mesh.contextLocalId+layerOffset];

        if (shader != lastShader || ticket != lastTicket)
        {
          // Render the latest batch of meshes
          RenderMeshes (node, lastShader, lastTicket, lastRenderedMesh, m);
          lastRenderedMesh = m;

          lastShader = shader;
          lastTicket = ticket;
        }
      }

      RenderMeshes (node, lastShader, lastTicket, lastRenderedMesh, node->meshes.GetSize ());
    }

  private:
    void RenderMeshes (typename RenderTree::MeshNode* node, 
      iShader* shader, size_t ticket,
      size_t firstMesh, size_t lastMesh)
    {
      if (firstMesh == lastMesh)
        return;
      
      if (!shader)
        return;

      typename RenderTree::ContextNode& context = node->owner;
      csShaderVariableStack& svStack = shaderMgr->GetShaderVariableStack ();

      const size_t numPasses = shader->GetNumberOfPasses (ticket);

      for (size_t p = 0; p < numPasses; ++p)
      {
        shader->ActivatePass (ticket, p);

        for (size_t m = firstMesh; m < lastMesh; ++m)
        {
          typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes.Get (m);
          context.svArrays.SetupSVStack (svStack, currentLayer, mesh.contextLocalId);

          csRenderMeshModes modes (*mesh.renderMesh);
          shader->SetupPass (ticket, mesh.renderMesh, modes, svStack);
          modes.z_buf_mode = mesh.zmode;

          g3d->DrawMesh (mesh.renderMesh, modes, svStack);

          shader->TeardownPass (ticket);
        }
        shader->DeactivatePass (ticket);
      }
    }

    iGraphics3D* g3d;
    iShaderManager* shaderMgr;

    size_t currentLayer;
  };

  template<typename RenderTree, typename LayerConfigType>
  struct OperationTraits<SimpleContextRender<RenderTree, LayerConfigType> >
  {
    typedef OperationUnordered Ordering;
  };

  /**
   * Renderer for multiple contexts, grouping them by render target and
   * rendering all layers of each context to same target.
   */
  template<typename RenderTree, typename LayerConfigType>
  class SimpleTreeRenderer
  {
  public:
    SimpleTreeRenderer (iGraphics3D* g3di, iShaderManager* shaderMgri, 
      const LayerConfigType& layers)
      : targetSetup (g3di), meshRender (g3di, shaderMgri),
      g3d (g3di), shaderMgr (shaderMgri), layers (layers),
      lastTarget (0), lastSubtexture (0)
    {}   

    ~SimpleTreeRenderer ()
    {
      // Render any remaining contexts
      RenderContextStack ();
    }

    void operator() (size_t i, typename RenderTree::ContextNode* context)
    {
      // Make sure right target is set
      if (IsNew (*context))
      {
        // New context, render out the old ones
        RenderContextStack ();
        lastTarget = context->renderTarget;
        lastSubtexture = context->subtexture;
      }

      // Push the context
      contextStack.Push (context);      
    }
   
  private:

    /**
     * Render all contexts currently in stack.
     * Assumes that all contexts within the stack have same target,
     * camera (except transform) and clipper.
     */
    void RenderContextStack ()
    {
      if (contextStack.IsEmpty ())
        return;

      // Setup context from first entry
      typename RenderTree::ContextNode* context = contextStack.Get (0);

      targetSetup (*context);

      RenderView* rview = context->renderView;

      int drawFlags = rview->GetEngine ()->GetBeginDrawFlags ();
      drawFlags |= CSDRAW_3DGRAPHICS /*| CSDRAW_CLEARSCREEN*/;

      iCamera* cam = rview->GetCamera ();
      iClipper2D* clipper = rview->GetClipper ();

      // Setup the camera etc.. @@should be delayed as well
      g3d->SetPerspectiveCenter (int (cam->GetShiftX ()), 
        int (cam->GetShiftY ()));
      g3d->SetPerspectiveAspect (cam->GetFOV ());
      g3d->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

      BeginFinishDrawScope bd (g3d, drawFlags);      

      // Render all mesh nodes in context
      for (size_t layer = 0; layer < layers.GetLayerCount (); ++layer)
      {
        meshRender.SetLayer (layer);

        for (size_t i = 0; i < contextStack.GetSize (); ++i)
        {
          context = contextStack.Get (i);

          g3d->SetWorldToCamera (context->cameraTransform.GetInverse ());
          ForEachMeshNode (*context, meshRender);
        }
      }

      contextStack.Empty ();
    }


    bool IsNew (const typename RenderTree::ContextNode& context)
    {
      return lastTarget != context.renderTarget ||
        lastSubtexture != context.subtexture;
    }

    ContextTargetSetup<typename RenderTree::ContextNode> targetSetup;
    SimpleContextRender<RenderTree, LayerConfigType> meshRender;

    iGraphics3D* g3d;
    iShaderManager* shaderMgr;
    const LayerConfigType& layers;

    iTextureHandle* lastTarget;
    int lastSubtexture;

    csArray<typename RenderTree::ContextNode*> contextStack;
  };

  template<typename RenderTree, typename LayerConfigType>
  struct OperationTraits<SimpleTreeRenderer<RenderTree, LayerConfigType> >
  {
    typedef OperationNumbered Ordering;
  };



}
}

#endif
