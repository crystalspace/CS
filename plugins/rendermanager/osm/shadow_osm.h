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

#include "ivideo/shader/shader.h"

#include "csutil/cfgacc.h"

#include "cstool/meshfilter.h"

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadow_common.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/viscull.h"

#include "csgeom/matrix4.h"
#include "csgeom/projections.h"

#include "csutil/scfstr.h"

class csShaderVariable;

namespace CS
{
  namespace RenderManager
  {
    template<typename RenderTree, typename LayerConfigType>
    class ShadowOSM
    {
    public:
      struct PersistentData;

      class ViewSetup
      {
      public:
        int numParts;
        PersistentData& persist;

        CS::RenderManager::RenderView* rview;

        SingleRenderLayer depthRenderLayer;

        ViewSetup (PersistentData& persist, CS::RenderManager::RenderView* rview)
          : persist (persist), rview (rview),
          depthRenderLayer (persist.settings.shadowShaderType, 
          persist.settings.shadowDefaultShader)
        {
          numParts = 2;
        }

        ~ViewSetup() {}
      };

      struct CachedLightData
      {
        // Transform light space to post-project light space
        struct SuperFrustum : public CS::Utility::FastRefCount<SuperFrustum>
        {
          int actualNumParts;
          // Transform world space to light space

          struct Frustum
          {
            csRef<csShaderVariable> shadowMapProjectSV;
            csRef<csShaderVariable> textureSVs[rtaNumAttachments];
          };
          Frustum* frustums;

          ~SuperFrustum() { delete[] frustums; }
        };
        typedef csRefArray<SuperFrustum> LightFrustumsArray;
        struct LightFrustums
        {
          LightFrustumsArray frustums;
        };
        csHash<LightFrustums, csRef<iCamera> > lightFrustumsHash;

        uint GetSublightNum() const { return (uint)1; }

        void SetupFrame (RenderTree& tree, ShadowOSM& shadows, iLight* light)
        {
          if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

          ViewSetup& viewSetup = shadows.viewSetup;

          csRef<iCamera> camera (viewSetup.rview->GetCamera());

          LightFrustums& lightFrustumsSettings =
            lightFrustumsHash.GetOrCreate (
            camera, LightFrustums());

          LightFrustumsArray& lightFrustums =
            lightFrustumsSettings.frustums;

          LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);

          for (uint f = 0; f < 1; f++)
          {
            csRef<SuperFrustum> newFrust;
            newFrust.AttachNew (new SuperFrustum);
            SuperFrustum& superFrustum = *(lightFrustums[lightFrustums.Push (
              newFrust)]);
            
            superFrustum.actualNumParts = viewSetup.numParts;
            superFrustum.frustums =
              new typename SuperFrustum::Frustum[superFrustum.actualNumParts];

            for (int i = 0; i < superFrustum.actualNumParts; i++)
            {
              typename SuperFrustum::Frustum& lightFrustum =
                superFrustum.frustums[i];
              lightFrustum.shadowMapProjectSV = lightVarsHelper.CreateTempSV (
                viewSetup.persist.svNames.GetLightSVId (
                csLightShaderVarCache::lightShadowMapProjection));
              lightFrustum.shadowMapProjectSV->SetArraySize (4);

              for (int j = 0; j < 4; j++)
              {
                csShaderVariable* item = lightVarsHelper.CreateTempSV (
                  CS::InvalidShaderVarStringID);
                lightFrustum.shadowMapProjectSV->SetArrayElement (j, item);
              }

              const ShadowSettings::Target* target =
                viewSetup.persist.settings.targets[0];
              lightFrustum.textureSVs[target->attachment] =
                lightVarsHelper.CreateTempSV (target->svName);
            }
          }
        }

        void AddShadowMapTarget (typename RenderTree::MeshNode* meshNode,
          PersistentData& persist, const SingleRenderLayer& layerConfig,
          RenderTree& renderTree, iLight* light, ViewSetup& viewSetup)
        {
          if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

          LightFrustums* lightFrustumsPtr =
            lightFrustumsHash.GetElementPointer (
            viewSetup.rview->GetCamera());

          LightFrustums& lightFrustums = *lightFrustumsPtr;
          float distance = 10;

          typename RenderTree::ContextNode& context = meshNode->GetOwner();

          const SuperFrustum& superFrust = *(lightFrustums.frustums[0]);
          
          for (int frustNum = 0 ; frustNum < superFrust.actualNumParts ; frustNum ++, distance += 10)
          {
            const typename SuperFrustum::Frustum& lightFrust = 
              superFrust.frustums[frustNum];
            CS::RenderManager::RenderView* rview = context.renderView;
            csRef<CS::RenderManager::RenderView> newRenderView;
            newRenderView = renderTree.GetPersistentData().renderViews.CreateRenderView ();
            newRenderView->SetEngine (rview->GetEngine ());
            newRenderView->SetThisSector (rview->GetThisSector ());

            CS::Math::Matrix4 matrix = rview->GetCamera()->GetProjectionMatrix();

            for (int i = 0; i < 4; i++)
            {
              csShaderVariable* item = lightFrust.shadowMapProjectSV->GetArrayElement (i);
              item->SetValue (matrix.Row (i));
            }

            int shadowMapSize = 1024;

            csRef<iCustomMatrixCamera> shadowViewCam =
              newRenderView->GetEngine()->CreateCustomMatrixCamera();
            newRenderView->SetCamera (shadowViewCam->GetCamera());

            csPlane3 farplane(0,0,-1,distance);
            shadowViewCam->GetCamera()->SetFarPlane(&farplane);

            shadowViewCam->SetProjectionMatrix (matrix);
            shadowViewCam->GetCamera()->SetTransform (light->GetMovable()->GetTransform());

            ShadowSettings::Target* target =
              viewSetup.persist.settings.targets[0];
            iTextureHandle* tex = target->texCache.QueryUnusedTexture (
              shadowMapSize, shadowMapSize);
            // and also this
            lightFrust.textureSVs[target->attachment]->SetValue (tex);
            renderTree.AddDebugTexture (tex);

            csBox2 clipBox (0, 0, shadowMapSize, shadowMapSize);
            csRef<iClipper2D> newView;
            newView.AttachNew (new csBoxClipper (clipBox));
            newRenderView->SetClipper (newView);

            typename RenderTree::ContextNode* shadowMapCtx = 
              renderTree.CreateContext (newRenderView);
            shadowMapCtx->renderTargets[rtaDepth].texHandle = tex;
            shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;

            // Setup the new context
            ShadowmapContextSetup contextFunction (layerConfig,
              persist.shaderManager, viewSetup, false);
            contextFunction (*shadowMapCtx);
          }
        }

        void ClearFrameData(){}

      };
    private:
      class ShadowmapContextSetup
      {
      public:
        ShadowmapContextSetup (const SingleRenderLayer& layerConfig,
          iShaderManager* shaderManager, ViewSetup& viewSetup,
          bool doIDTexture)
          : layerConfig (layerConfig), shaderManager (shaderManager),
          viewSetup (viewSetup), doIDTexture (doIDTexture)
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
        bool doIDTexture;
      };
    public:

      /**
      * Data used by the shadow handler that needs to persist over multiple frames.
      * Generally stored inside the light setup's persistent data.
      */
      struct PersistentData
      {
        csLightShaderVarCache svNames;
        LightingSorter::PersistentData lightSorterPersist;
        LightingVariablesHelper::PersistentData lightVarsPersist;
        iShaderManager* shaderManager;
        csRefArray<iTextureHandle> emptySMs;
        iGraphics3D* g3d;

        csString configPrefix;
        ShadowSettings settings;

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
          }
        }
        void UpdateNewFrame ()
        {
          csTicks time = csGetTicks ();
          settings.AdvanceFrame (time);
        }
      };

      typedef ViewSetup ShadowParameters;

      ShadowOSM (PersistentData& persist,
        const LayerConfigType& layerConfig,
        typename RenderTree::MeshNode* node, 
        ViewSetup& viewSetup) 
        : persist (persist), layerConfig (layerConfig), 
        renderTree (node->GetOwner().owner), meshNode (node),
        viewSetup (viewSetup)
      { }

      uint HandleOneLight (typename RenderTree::MeshNode::SingleMesh& singleMesh,
        iLight* light, CachedLightData& lightData,
        csShaderVariableStack* lightStacks,
        uint lightNum, uint subLightNum)
      {
        LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);

        typename CachedLightData::LightFrustumsArray& lightFrustums =
          lightData.lightFrustumsHash.GetElementPointer (
          viewSetup.rview->GetCamera())->frustums;
        typename CachedLightData::SuperFrustum& superFrust =
          *(lightFrustums[subLightNum]);

        uint spreadFlags = 0;
        int s = 0;

        for (int f = 0; f < superFrust.actualNumParts; f++)
        {
          typename CachedLightData::SuperFrustum::Frustum& lightFrustum =
            superFrust.frustums[f];
          lightVarsHelper.MergeAsArrayItem (lightStacks[0],
            lightFrustum.shadowMapProjectSV, s);

          const ShadowSettings::Target* target =
            viewSetup.persist.settings.targets[0];
          lightVarsHelper.MergeAsArrayItem (lightStacks[0], 
            lightFrustum.textureSVs[target->attachment], s);

          spreadFlags |= (1 << s);
          s++;        
        }

        return spreadFlags;
      }

      static bool NeedFinalHandleLight() { return true; }
      void FinalHandleLight (iLight* light, CachedLightData& lightData)
      {
        lightData.AddShadowMapTarget (meshNode, persist,
          viewSetup.depthRenderLayer, renderTree, light, viewSetup);

        lightData.ClearFrameData();
      }

      csFlags GetLightFlagsMask () const { return csFlags (0); }

      size_t GetLightLayerSpread() const { return 1; }
    protected:
      PersistentData& persist;
      const LayerConfigType& layerConfig;
      RenderTree& renderTree;
      typename RenderTree::MeshNode* meshNode;
      ViewSetup& viewSetup;
    };

  }
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_OSM_H__
