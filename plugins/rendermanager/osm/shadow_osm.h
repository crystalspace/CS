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

#include "csutil/cfgacc.h"
#include "csutil/scfstr.h"
#include "csplugincommon/rendermanager/shadow_common.h"

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
      iShaderManager* shaderManager;
      iGraphics3D* g3d;

      csString configPrefix;
      ShadowSettings settings;

      size_t maps;
      size_t shadowMapSize;

//       CS::ShaderVarStringID svName;
      csLightShaderVarCache svNames;
      LightingVariablesHelper::PersistentData lightVarsPersist;

      /// Set the prefix for configuration settings
      void SetConfigPrefix (const char* configPrefix)
      {
        this->configPrefix = configPrefix;
      }

      void Initialize (iObjectRegistry* objectReg,
        RenderTreeBase::DebugPersistent& dbgPersist)
      {
        csRef<iShaderManager> shaderManager =
          csQueryRegistry<iShaderManager> (objectReg);
        csRef<iGraphics3D> g3d =
          csQueryRegistry<iGraphics3D> (objectReg);

        this->shaderManager = shaderManager;
        this->g3d = g3d;

        iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
        svNames.SetStrings (strings);

//         const char* svNameStr = "light shadow map";
//         svName = strings->Request (svNameStr);

        csConfigAccess cfg (objectReg);
        if (configPrefix.IsEmpty())
        {
          settings.ReadSettings (objectReg, "Depth");
        }
        else
        {
          settings.ReadSettings (objectReg, 
            cfg->GetStr (
            csString().Format ("%s.ShadowsType", configPrefix.GetData()), "Depth"));
          maps = cfg->GetInt (
            csString().Format ("%s.Maps", configPrefix.GetData()), 1);
          shadowMapSize = cfg->GetInt (
            csString().Format ("%s.ShadowMapResolution", configPrefix.GetData()), 1024);
        }
      }
      void UpdateNewFrame ()
      {
        csTicks time = csGetTicks ();
        settings.AdvanceFrame (time);
//         lightVarsPersist.UpdateNewFrame();
      }
    };

    class ViewSetup
    {
    public:
      PersistentData& persist;
      CS::RenderManager::RenderView* rview;
      SingleRenderLayer depthRenderLayer;
//       csRefArray<iTextureHandle> renderTargets;

      ViewSetup (PersistentData& persist, CS::RenderManager::RenderView* rview)
        : persist (persist), rview (rview),
        depthRenderLayer (persist.settings.shadowShaderType, 
        persist.settings.shadowDefaultShader)
      {
        // Creates the accumulation buffer.
//         int flags = CS_TEXTURE_3D | CS_TEXTURE_CLAMP  | CS_TEXTURE_NOMIPMAPS;
//         iGraphics2D *g2d = persist.g3d->GetDriver2D ();
// 
//         scfString errStr;

//         for (size_t i = 0 ; i < persist.maps ; i ++)
//         {
//           csRef<iTextureHandle> renderTarget = 
//             persist.g3d->GetTextureManager ()->CreateTexture (g2d->GetWidth (),
//             g2d->GetHeight (),
//             csimg2D,
//             "d32",
//             flags,
//             &errStr);
// 
//           if (!renderTarget)
//             csPrintf("Error initializing renderTargets!\n");        
// 
//           renderTargets.Push(renderTarget);
//         }
      }
    };

    class ShadowmapContextSetup
    {
    public:
      ShadowmapContextSetup (const SingleRenderLayer& layerConfig,
        iShaderManager* shaderManager, ViewSetup& viewSetup )
        : layerConfig (layerConfig), shaderManager (shaderManager),
        viewSetup (viewSetup)
      {

      }

      void operator() (typename RenderTree::ContextNode& context)
      {
        CS::RenderManager::RenderView* rview = context.renderView;
        iSector* sector = rview->GetThisSector ();

        // @@@ This is somewhat "boilerplate" sector/rview setup.
        rview->SetThisSector (sector);
        sector->CallSectorCallbacks (rview);
        // Make sure the clip-planes are ok
        CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());

        // Do the culling
        iVisibilityCuller* culler = sector->GetVisibilityCuller ();
        Viscull<RenderTree> (context, rview, culler);

        // Sort the mesh lists  
        {
          StandardMeshSorter<RenderTree> mySorter (rview->GetEngine ());
          mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
          ForEachMeshNode (context, mySorter);
        }

        // After sorting, assign in-context per-mesh indices
        {
          SingleMeshContextNumbering<RenderTree> numbering;
          ForEachMeshNode (context, numbering);
        }

        // Setup the SV arrays
        // Push the default stuff
        SetupStandardSVs (context, layerConfig, shaderManager, sector);

        // Setup the material&mesh SVs
        {
          StandardSVSetup<RenderTree, SingleRenderLayer> svSetup (
            context.svArrays, layerConfig);

          ForEachMeshNode (context, svSetup);
        }

        SetupStandardShader (context, shaderManager, layerConfig);

        // Setup shaders and tickets
        SetupStandardTicket (context, shaderManager, layerConfig);
      }


    private:
      const SingleRenderLayer& layerConfig;
      iShaderManager* shaderManager;
      ViewSetup& viewSetup;
    };

    struct CachedLightData
    {
      csRef<csShaderVariable> shadowMapProjectSV;
      csRef<csShaderVariable> textureSVs[rtaNumAttachments];

      void SetupFrame (RenderTree& tree, ShadowOSM& shadows, iLight* light)
      {
        ViewSetup& viewSetup = shadows.viewSetup;

        LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);

        shadowMapProjectSV = lightVarsHelper.CreateTempSV (
          viewSetup.persist.svNames.GetLightSVId (
          csLightShaderVarCache::lightShadowMapProjection));

        shadowMapProjectSV->SetArraySize (4);
        for (int j = 0; j < 4; j++)
        {
          csShaderVariable* item = lightVarsHelper.CreateTempSV (
            CS::InvalidShaderVarStringID);
          shadowMapProjectSV->SetArrayElement (j, item);
        }

        const ShadowSettings::Target* target =
          viewSetup.persist.settings.targets[0];
        textureSVs[rtaDepth] =
          lightVarsHelper.CreateTempSV (target->svName);

      }

      uint GetSublightNum() const { return (uint)1; }

      void ClearFrameData(){}

      void AddShadowMapTarget (typename RenderTree::MeshNode* meshNode,
        PersistentData& persist, const SingleRenderLayer& layerConfig,
        RenderTree& renderTree, iLight* light, ViewSetup& viewSetup)
      {
        if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;
        typename RenderTree::ContextNode& context = meshNode->GetOwner();

        float distance = 30;
        for (size_t i = 0 ; i < persist.maps ; i ++, distance += 10)
        {
          CS::RenderManager::RenderView* rview = context.renderView;
          csRef<CS::RenderManager::RenderView> newRenderView;
          newRenderView = renderTree.GetPersistentData().renderViews.CreateRenderView ();
          newRenderView->SetEngine (rview->GetEngine ());
          newRenderView->SetThisSector (rview->GetThisSector ());

          CS::Math::Matrix4 matrix = rview->GetCamera()->GetProjectionMatrix();

          // this
          for (int i = 0; i < 4; i++)
          {
            csShaderVariable* item = 
              shadowMapProjectSV->GetArrayElement (i);
            item->SetValue (matrix.Row (i));
          }

          csRef<iCustomMatrixCamera> shadowViewCam =
            newRenderView->GetEngine()->CreateCustomMatrixCamera();
          newRenderView->SetCamera (shadowViewCam->GetCamera());
          shadowViewCam->SetProjectionMatrix (matrix);
          shadowViewCam->GetCamera()->SetTransform (light->GetMovable()->GetTransform());
          csPlane3 farplane(0,0,-1,distance);
          shadowViewCam->GetCamera()->SetFarPlane(&farplane);

          ShadowSettings::Target* target =
            viewSetup.persist.settings.targets[0];
          iTextureHandle* tex = target->texCache.QueryUnusedTexture (
            persist.shadowMapSize, persist.shadowMapSize);          

          textureSVs[rtaDepth]->SetValue (tex);

          renderTree.AddDebugTexture (tex);

          csBox2 clipBox (0, 0, persist.shadowMapSize, persist.shadowMapSize);
          csRef<iClipper2D> newView;
          newView.AttachNew (new csBoxClipper (clipBox));
          newRenderView->SetClipper (newView);

          typename RenderTree::ContextNode* shadowMapCtx = 
            renderTree.CreateContext (newRenderView);

          shadowMapCtx->renderTargets[rtaDepth].texHandle = tex;

          shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;

          // Setup the new context
          ShadowmapContextSetup contextFunction (layerConfig,
            persist.shaderManager, viewSetup);
          contextFunction (*shadowMapCtx);

//         csPrintf("New Target %u!\n", currentFrame);
        }
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
      uint lightNum, uint subLightNum) 
    { 
      LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);

      // Add shadow map SVs
      lightVarsHelper.MergeAsArrayItem (lightStacks[0], 
        lightData.shadowMapProjectSV, lightNum);

      lightVarsHelper.MergeAsArrayItem (lightStacks[0], 
        lightData.textureSVs[rtaDepth], lightNum);
        
      return 1; 
    }

  protected:
    PersistentData& persist;
    RenderTree& renderTree;
    typename RenderTree::MeshNode* meshNode;
    ViewSetup& viewSetup;
  };
}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__
