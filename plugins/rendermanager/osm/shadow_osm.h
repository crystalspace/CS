/*
  Copyright (C) 2011 Alexandru - Teodor Voicu
      Imperial College London
      http://www3.imperial.ac.uk/

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__

namespace CS
{
namespace RenderManager
{
  template<typename RenderTree, typename LayerConfigType>
  class ShadowOSM
  {
  public:
    struct PersistentData
    {
      csRef<iTextureHandle> accumBuffer;
      csRef<iShaderManager> shaderManager;

      void Initialize (iObjectRegistry* objectReg,
        RenderTreeBase::DebugPersistent& dbgPersist)
      {
	      csConfigAccess cfg (objectReg);
        csRef<iGraphics3D> g3d =
          csQueryRegistry<iGraphics3D> (objectReg);

        // Creates the accumulation buffer.
        int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
        const char *accumFmt = cfg->GetStr ("RenderManager.Deferred.AccumBufferFormat", "rgb16_f");
        iGraphics2D *g2d = g3d->GetDriver2D ();

        scfString errStr;
        accumBuffer = g3d->GetTextureManager ()->CreateTexture (g2d->GetWidth (),
          g2d->GetHeight (),
          csimg2D,
          accumFmt,
          flags,
          &errStr);

        if (!accumBuffer)
          csPrintf("Error initializing accumBuffer!\n");

        shaderManager = csQueryRegistry<iShaderManager> (objectReg);
      }
      
      void UpdateNewFrame ()
      {
      }
    };

    class ViewSetup
    {
    public:
      CS::RenderManager::RenderView* rview;

      PersistentData& persist;
      SingleRenderLayer depthRenderLayer;

      ViewSetup (PersistentData& persist, CS::RenderManager::RenderView* rview)
         : persist (persist), rview(rview)
      {
        
      }
    };

    struct CachedLightData
    {
      void SetupFrame (RenderTree& tree, ShadowOSM& shadows, iLight* light){}

      uint GetSublightNum() const { return (uint)1; }

      void ClearFrameData(){}

      void AddShadowMapTarget (typename RenderTree::MeshNode* meshNode,
        PersistentData& persist, const SingleRenderLayer& layerConfig,
        RenderTree& renderTree, iLight* light, ViewSetup& viewSetup)
      {
        if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

        uint currentFrame = viewSetup.rview->GetCurrentFrameNumber();

//         CS_ALLOC_STACK_ARRAY(iTextureHandle*, texHandles, 1);
        
        typename RenderTree::ContextNode* shadowMapCtx = 
          renderTree.CreateContext (viewSetup.rview);
        shadowMapCtx->drawFlags |= (CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER);

        shadowMapCtx->renderTargets[rtaColor1].texHandle = persist.accumBuffer;        
        shadowMapCtx->renderTargets[rtaColor1].subtexture = 0; 
        renderTree.AddDebugTexture (persist.accumBuffer);

//         csPrintf("New Target %u!\n", currentFrame);
      }
    };

    typedef ViewSetup ShadowParameters;

    ShadowOSM (PersistentData& persist,
      const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode* node, 
      ViewSetup& viewSetup) : persist (persist), 
      renderTree (node->GetOwner().owner), meshNode (node), 
      viewSetup (viewSetup)
    {
    }

    csFlags GetLightFlagsMask () const { return csFlags (0); }

    static bool NeedFinalHandleLight() { return true; }

    void FinalHandleLight (iLight* light, CachedLightData& lightData) 
    { 
      lightData.AddShadowMapTarget (meshNode, persist,
        viewSetup.depthRenderLayer, renderTree, light, viewSetup);

      lightData.ClearFrameData();
    }

    size_t GetLightLayerSpread() const { return 1; }

    uint HandleOneLight (typename RenderTree::MeshNode::SingleMesh& singleMesh,
      iLight* light, CachedLightData& lightData,
      csShaderVariableStack* lightStacks,
      uint lightNum, uint subLightNum) { return 1; }

  protected:
    PersistentData& persist;
    RenderTree& renderTree;
    typename RenderTree::MeshNode* meshNode;
    ViewSetup& viewSetup;
  };
}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__
