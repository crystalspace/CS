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

/**\file
 * Context rendering
 */

#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/svarrayholder.h"

#include "ivideo/graph2d.h"

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
      for (int a = 0; a < rtaNumAttachments; a++)
      {
	/* @@@ Checking for rtaColor0 is ugly. Eventually, PostEffects
	    should act on multiple targets at the same time */
        if ((csRenderTargetAttachment (a) == rtaColor0)
            && (context.postEffects.IsValid()))
        {
          context.postEffects->SetEffectsOutputTarget (
            context.renderTargets[a].texHandle);
		  g3d->SetRenderTarget (
			context.postEffects->GetScreenTarget (), false,
			context.renderTargets[a].subtexture, 
			csRenderTargetAttachment (a));
        }
        else
		  g3d->SetRenderTarget (context.renderTargets[a].texHandle, false,
			context.renderTargets[a].subtexture, csRenderTargetAttachment (a));
      }
    }

  private:
    iGraphics3D* g3d;
  };

  /**
   * Render mesh nodes within one context.
   * Does not handle setting render targets or camera transform.
   */
  template<typename RenderTree, bool occlusionQueries = false>
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
        
        size_t ticket = context.ticketArray[mesh.contextLocalId+layerOffset];

        if (shader != lastShader || ticket != lastTicket
            || (mesh.preCopyNum != 0))
        {
          // Render the latest batch of meshes
          RenderMeshes (node, lastShader, lastTicket, lastRenderedMesh, m);
          lastRenderedMesh = m;

          lastShader = shader;
          lastTicket = ticket;
        }
        
        if (mesh.preCopyNum != 0)
        {
          g3d->CopyFromRenderTargets (mesh.preCopyNum,
            mesh.preCopyAttachments, mesh.preCopyTextures);
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
      
      // Skip meshes without shader (for current layer)
      if (!shader)
        return;

      typename RenderTree::ContextNode& context = node->owner;
      csShaderVariableStack& svStack = shaderMgr->GetShaderVariableStack ();

      const size_t numPasses = shader->GetNumberOfPasses (ticket);

      for (size_t p = 0; p < numPasses; ++p)
      {
        if (!shader->ActivatePass (ticket, p)) continue;

        for (size_t m = firstMesh; m < lastMesh; ++m)
        {
          typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes.Get (m);
          context.svArrays.SetupSVStack (svStack, currentLayer, mesh.contextLocalId);

          csRenderMeshModes modes (*mesh.renderMesh);
          if (!shader->SetupPass (ticket, mesh.renderMesh, modes, svStack)) continue;
          modes.z_buf_mode = mesh.zmode;

		  if (occlusionQueries)
          {
            g3d->BeginOcclusionQuery(*mesh.occlusionQuery);
          }

          g3d->DrawMesh (mesh.renderMesh, modes, svStack);

		  if (occlusionQueries)
          {
            g3d->EndOcclusionQuery();
          }

          shader->TeardownPass (ticket);
        }
        shader->DeactivatePass (ticket);
      }
    }

    iGraphics3D* g3d;
    iShaderManager* shaderMgr;

    size_t currentLayer;
  };

  template<typename RenderTree>
  struct OperationTraits<SimpleContextRender<RenderTree> >
  {
    typedef OperationUnordered Ordering;
  };

  /**
   * Renderer for multiple contexts, grouping them by render target and
   * rendering all layers of each context to same target.
   *
   * Usage: with reverse iteration over all contexts.
   * Usually used in the final step before post processing effects are
   * applied. 
   *
   * Example:
   * \code
   * // ... contexts setup etc. ...
   *
   * {
   *   SimpleTreeRenderer<RenderTree> render (renderView->GetGraphics3D (),
   *     shaderManager);
   *   ForEachContextReverse (renderTree, render);
   * }
   *
   * // ... apply post processing ...
   * \endcode
   */
  template<typename RenderTree>
  class SimpleTreeRenderer
  {
  public:
    SimpleTreeRenderer (iGraphics3D* g3di, iShaderManager* shaderMgri)
      : targetSetup (g3di), meshRender (g3di, shaderMgri),
      g3d (g3di), shaderMgr (shaderMgri), lastRenderView (0)
    {
      memset (lastTarget, 0, sizeof (lastTarget));
      memset (lastSubtexture, 0, sizeof (lastSubtexture));
    }

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
        for (int a = 0; a < rtaNumAttachments; a++)
        {
          lastTarget[a] = context->renderTargets[a].texHandle;
          lastSubtexture[a] = context->renderTargets[a].subtexture;
        }
		lastRenderView = context->renderView;
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
      drawFlags |= context->drawFlags;

      iCamera* cam = rview->GetCamera ();
      iClipper2D* clipper = rview->GetClipper ();

      if (context->owner.IsDebugClearEnabled())
      {
        iGraphics2D* G2D = g3d->GetDriver2D();
        g3d->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARZBUFFER);
		int bgcolor_clear = G2D->FindRGB (0, 255, 255);
		G2D->Clear (bgcolor_clear);
      }
      
      // Setup the camera etc.. @@should be delayed as well
      g3d->SetProjectionMatrix (
	context->perspectiveFixup * cam->GetProjectionMatrix ());
      g3d->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

      BeginFinishDrawScope bd (g3d, drawFlags);

      /* Different contexts may have different numbers of layers,
       * so determine the upper layer number */
      size_t maxLayer = 0;
      for (size_t i = 0; i < contextStack.GetSize (); ++i)
      {
        maxLayer = csMax (maxLayer,
          contextStack[i]->svArrays.GetNumLayers());
      }

      // Render all mesh nodes in context
      for (size_t layer = 0; layer < maxLayer; ++layer)
      {
        meshRender.SetLayer (layer);

        for (size_t i = 0; i < contextStack.GetSize (); ++i)
        {
          context = contextStack.Get (i);
          /* Bail out if layer index is above the actual layer count in the
           * context */
          if (layer >= context->svArrays.GetNumLayers()) continue;

          g3d->SetWorldToCamera (context->cameraTransform.GetInverse ());
          ForEachMeshNode (*context, meshRender);
        }
      }
      /* @@@ FIXME: When switching from RT to screen with a clipper set
         the clip rect gets wrong (stays at RT size). This workaround ensures
         that no "old" clip rect is stored which is restored later.
         Should really be fixed in the renderer. */
      g3d->SetClipper (0, CS_CLIPPER_TOPLEVEL);

      contextStack.Empty ();
      
      if (context->postEffects.IsValid())
        context->postEffects->DrawPostEffects (context->owner);
    }


    bool IsNew (const typename RenderTree::ContextNode& context)
    {
      for (int a = 0; a < rtaNumAttachments; a++)
      {
        if ((lastTarget[a] != context.renderTargets[a].texHandle)
	    || (lastSubtexture[a] != context.renderTargets[a].subtexture))
          return true;
      }
      return context.renderView != lastRenderView;
    }

    ContextTargetSetup<typename RenderTree::ContextNode> targetSetup;
    SimpleContextRender<RenderTree> meshRender;

    iGraphics3D* g3d;
    iShaderManager* shaderMgr;

    iTextureHandle* lastTarget[rtaNumAttachments];
    int lastSubtexture[rtaNumAttachments];
    RenderView* lastRenderView;

    csArray<typename RenderTree::ContextNode*> contextStack;
  };

  template<typename RenderTree>
  struct OperationTraits<SimpleTreeRenderer<RenderTree> >
  {
    typedef OperationNumbered Ordering;
  };



}
}

#endif
