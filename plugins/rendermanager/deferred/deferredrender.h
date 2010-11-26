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

#ifndef __DEFERREDRENDER_H__
#define __DEFERREDRENDER_H__

#include "cssysdef.h"

#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/rendertree.h"

#include "deferredlightrender.h"
#include "deferredoperations.h"
#include "gbuffer.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{
  /**
   * Renderer for multiple contexts where all forward rendering objects are drawn.
   */
  template<typename RenderTree>
  class ForwardMeshTreeRenderer
  {
  public:

    ForwardMeshTreeRenderer(iGraphics3D* g3d, iShaderManager *shaderMgr, int deferredLayer)
      : 
    meshRender(g3d, shaderMgr),
    graphics3D(g3d),
    shaderMgr(shaderMgr),
    deferredLayer(deferredLayer)
    {}

    ~ForwardMeshTreeRenderer() {}

    /**
     * Render all contexts.
     */
    void operator()(typename RenderTree::ContextNode *context)
    {
      CS::RenderManager::RenderView *rview = context->renderView;
      
      iCamera *cam = rview->GetCamera ();
      iClipper2D *clipper = rview->GetClipper ();
      
      // Setup the camera etc.. @@should be delayed as well
      graphics3D->SetProjectionMatrix (context->perspectiveFixup * cam->GetProjectionMatrix ());
      graphics3D->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

      graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

      size_t layerCount = context->svArrays.GetNumLayers ();
      for (size_t layer = 0; layer < layerCount; ++layer)
      {
        if ((int)layer == deferredLayer)
          continue;

        meshRender.SetLayer (layer);
        ForEachForwardMeshNode (*context, meshRender);
      }
    }
   
  private:

    CS::RenderManager::SimpleContextRender<RenderTree> meshRender;

    iGraphics3D *graphics3D;
    iShaderManager *shaderMgr;

    int deferredLayer;
  };

  /**
   * Deferred renderer for multiple contexts.
   *
   * Example:
   * \code
   * // ... contexts setup etc. ...
   *
   * {
   *   DeferredTreeRenderer<RenderTree> 
   *     render (graphics3D, shaderManager, stringSet, 
   *             gbuffer, lightRenderPersistent, deferredLayer, 
   *             zonlyLayer, drawLightVolumes);
   *
   *   ForEachContextReverse (renderTree, render);
   * }
   *
   * // ... apply post processing ...
   * \endcode
   */
  template<typename RenderTree>
  class DeferredTreeRenderer
  {
  public:

    DeferredTreeRenderer(iGraphics3D *g3d, 
                         iShaderManager *shaderMgr,
                         iStringSet *stringSet,
                         GBuffer &gbuffer,
                         DeferredLightRenderer::PersistentData &lightRenderPersistent,
                         int deferredLayer,
                         int zonlyLayer,
                         bool drawLightVolumes)
      : 
    meshRender(g3d, shaderMgr),
    graphics3D(g3d),
    shaderMgr(shaderMgr),
    stringSet(stringSet),
    gbuffer(gbuffer),
    lightRenderPersistent(lightRenderPersistent),
    deferredLayer(deferredLayer),
    zonlyLayer(zonlyLayer),
    lastAccumBuf(nullptr),
    lastSubTex(-1),
    lastRenderView(nullptr),
    drawLightVolumes(drawLightVolumes)
    {}

    ~DeferredTreeRenderer() 
    {
      RenderContextStack ();
    }

    /**
     * Render all contexts.
     */
    void operator()(typename RenderTree::ContextNode *context)
    {
      if (IsNew (*context))
      {
        // New context, render out the old ones
        RenderContextStack ();

        lastAccumBuf = context->renderTargets[rtaColor0].texHandle;
        lastSubTex = context->renderTargets[rtaColor0].subtexture;
        lastRenderView = context->renderView;
      }
      
      contextStack.Push (context);
    }

  protected:

    void RenderContextStack()
    {
      const size_t ctxCount = contextStack.GetSize ();

      if (ctxCount == 0)
        return;
      
      typename RenderTree::ContextNode *context = contextStack[0];

      CS::RenderManager::RenderView *rview = context->renderView;

      iEngine *engine = rview->GetEngine ();
      iCamera *cam = rview->GetCamera ();
      iClipper2D *clipper = rview->GetClipper ();

      // Create the light renderer here so we do not needlessly recreate it latter.
      DeferredLightRenderer lightRender (graphics3D,
                                         shaderMgr,
                                         stringSet,
                                         rview,
                                         gbuffer,
                                         lightRenderPersistent);

      // Fill the gbuffer
      gbuffer.Attach ();
      {
        graphics3D->SetZMode (CS_ZBUF_MESH);

        // Setup the camera etc.
        graphics3D->SetProjectionMatrix (context->perspectiveFixup * cam->GetProjectionMatrix ());
        graphics3D->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

        int drawFlags = CSDRAW_3DGRAPHICS | context->drawFlags;
        drawFlags |= CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
        graphics3D->BeginDraw (drawFlags);
        graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

        meshRender.SetLayer (deferredLayer);

        for (size_t i = 0; i < ctxCount; i++)
        {
          typename RenderTree::ContextNode *context = contextStack[i];
          
          ForEachDeferredMeshNode (*context, meshRender);
        }
        
        graphics3D->FinishDraw ();
      }
      gbuffer.Detach ();

      // Fills the accumulation buffer
      AttachAccumBuffer (context, false);
      {
        graphics3D->SetZMode (CS_ZBUF_MESH);

        int drawFlags = CSDRAW_3DGRAPHICS | context->drawFlags;
        drawFlags |= CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;

        graphics3D->BeginDraw (drawFlags);
        graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());
        
        lightRender.OutputAmbientLight ();

        // Iterate through lights adding results into accumulation buffer.
        for (size_t i = 0; i < ctxCount; i++)
        {
          typename RenderTree::ContextNode *context = contextStack[i];

          ForEachLight (*context, lightRender);
        }
      }
      DetachAccumBuffer ();

      // Draws the forward shaded objects.
      AttachAccumBuffer (context, true);
      {
        graphics3D->SetZMode (CS_ZBUF_MESH);

        ForwardMeshTreeRenderer<RenderTree> render (graphics3D, shaderMgr, deferredLayer);
        LightVolumeRenderer lightVolumeRender (lightRender, true, 0.2f);

        for (size_t i = 0; i < ctxCount; i++)
        {
          typename RenderTree::ContextNode *context = contextStack[i];

          render (context);

          // Output light volumes.
          if (drawLightVolumes)
            ForEachLight (*context, lightVolumeRender);
        }

        graphics3D->FinishDraw ();
      }
      DetachAccumBuffer ();

      contextStack.Empty ();
    }

    /**
     * Attaches the accumulation buffer stored with the given context.
     */
    bool AttachAccumBuffer(typename RenderTree::ContextNode *context, bool useGbufferDepth)
    {
      int subTex;
      iTextureHandle *buf = GetAccumBuffer (context, subTex);
      
      CS_ASSERT (buf);

      return AttachAccumBuffer (buf, subTex, useGbufferDepth);
    }

    /**
     * Attaches the given accumulation buffer and uses the gbuffers depth if requested.
     */
    bool AttachAccumBuffer(iTextureHandle *buf, int subTex, bool useGbufferDepth)
    {
      CS_ASSERT (buf);

      // Assume that the accumulation texture matches the gbuffer dimensions.
  #if CS_DEBUG
      int w, h;
      int bufW, bufH;
      gbuffer.GetDimensions (w, h);
      buf->GetRendererDimensions (bufW, bufH);

      CS_ASSERT (w == bufW && h == bufH);
  #endif

      if (!graphics3D->SetRenderTarget (buf, false, subTex, rtaColor0))
        return false;

      if (useGbufferDepth && gbuffer.HasDepthBuffer ())
      {
        if (!graphics3D->SetRenderTarget (gbuffer.GetDepthBuffer (), false, 0, rtaDepth))
          return false; 
      }

      if (!graphics3D->ValidateRenderTargets ())
        return false;

      return true;
    }

    /**
     * Detaches the accumulation buffer.
     */
    void DetachAccumBuffer()
    {
      graphics3D->UnsetRenderTargets ();
    }

     /**
      * Returns the contexts accumulation buffer or NULL if no such buffer exists.
      */
    iTextureHandle *GetAccumBuffer(typename RenderTree::ContextNode *context, int &subTex)
    {
      subTex = context->renderTargets[rtaColor0].subtexture;
      return context->renderTargets[rtaColor0].texHandle;
    }

    /**
     * Returns true if the given context has a valid accumulation buffer.
     */
    bool HasAccumBuffer(typename RenderTree::ContextNode *context)
    {
      int subTex;
      iTextureHandle *buf = GetAccumBuffer (context, subTex);
      return buf != (iTextureHandle*) nullptr; 
    }

    /**
     * Returns true if the given context has the same accumulation buffer 
     * as the last context.
     */
    bool HasSameAccumBuffer(typename RenderTree::ContextNode &context)
    {
      int subTex;
      iTextureHandle *buf = GetAccumBuffer (&context, subTex);

      return buf == lastAccumBuf && subTex == lastSubTex;
    }

    /**
     * Returns true if the given context has the same render view as the 
     * last context.
     */
    bool HasSameRenderView(typename RenderTree::ContextNode &context)
    {
      return context.renderView == lastRenderView;
    }

    /**
     * Returns true if the given context is different from the last context.
     */
    bool IsNew(typename RenderTree::ContextNode &context)
    {
      return !HasSameAccumBuffer (context) || !HasSameRenderView (context);
    }

  private:

    CS::RenderManager::SimpleContextRender<RenderTree> meshRender;

    iGraphics3D *graphics3D;
    iShaderManager *shaderMgr;
    iStringSet *stringSet;

    GBuffer &gbuffer;
    DeferredLightRenderer::PersistentData &lightRenderPersistent;

    csArray<typename RenderTree::ContextNode*> contextStack;

    int deferredLayer;
    int zonlyLayer;

    iTextureHandle *lastAccumBuf;
    int lastSubTex;
    CS::RenderManager::RenderView *lastRenderView;

    bool drawLightVolumes;
  };

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERREDRENDER_H__
