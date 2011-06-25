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
#include "crystalspace.h"

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
        float* splitDists;
        CS::RenderManager::RenderView* rview;

        SingleRenderLayer depthRenderLayer;

        ViewSetup (PersistentData& persist, CS::RenderManager::RenderView* rview)
          : persist (persist), rview (rview),
          depthRenderLayer (persist.settings.shadowShaderType, 
          persist.settings.shadowDefaultShader)
        {
          numParts = persist.numSplits + 1;
	        splitDists = new float[numParts];

          // linear interpolation

          splitDists[0] = 11;
          splitDists[1] = 13.5;
          splitDists[2] = 14.5;
          splitDists[3] = 16;
        }

        ~ViewSetup() { delete[] splitDists; }
      };

      struct CachedLightData :
        public CS::Memory::CustomAllocated
      {
        uint lastSetupFrame;
        // Transform light space to post-project light space
        CS::Math::Matrix4 lightProject;

        CachedLightData() : lastSetupFrame (~0) {}
        // Transform light space to post-project light space
        struct SuperFrustum : public CS::Utility::FastRefCount<SuperFrustum>
        {
          int actualNumParts;
          // Transform world space to light space
          csReversibleTransform world2light_rotated;

          csRef<csShaderVariable> farZSV;
          csRef<csShaderVariable> numSplitsSV;

          csBox3 castingObjectsBBoxPP;

          struct Frustum
          {
            csRef<csShaderVariable> shadowMapProjectSV;
            csRef<csShaderVariable> splitDistsSV;
            csRef<csShaderVariable> textureSVs[rtaNumAttachments];

            csBox3 receivingObjectsBBoxPP;
          };
          Frustum* frustums;

          ~SuperFrustum() { delete[] frustums; }
        };
        typedef csRefArray<SuperFrustum> LightFrustumsArray;
        struct LightFrustums
        {
          uint setupFrame;
          LightFrustumsArray frustums;

          LightFrustums() : setupFrame (~0) {}
        };
        csHash<LightFrustums, csRef<iCamera> > lightFrustumsHash;

        uint GetSublightNum() const { return (uint)1; }

        void SetupFrame (RenderTree& tree, ShadowOSM& shadows, iLight* light)
        {
          if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

          ViewSetup& viewSetup = shadows.viewSetup;
          uint currentFrame = viewSetup.rview->GetCurrentFrameNumber();
          if (lastSetupFrame != currentFrame)
          {
            lightFrustumsHash.DeleteAll();
            lastSetupFrame = currentFrame;
          }          
          csRef<iCamera> camera (viewSetup.rview->GetCamera());

          LightFrustums& lightFrustumsSettings =
            lightFrustumsHash.GetOrCreate (
            camera, LightFrustums());

          float lightNear = SMALL_Z;
          float lightCutoff = light->GetCutoffDistance();

          CS::Math::Matrix4 lightProject = CS::Math::Projections::Ortho (
            lightCutoff, -lightCutoff, lightCutoff, -lightCutoff,
            -lightCutoff, -lightNear);

          this->lightProject = lightProject;

          LightFrustumsArray& lightFrustums =
            lightFrustumsSettings.frustums;

          LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);

          csRef<SuperFrustum> newFrust;
          newFrust.AttachNew (new SuperFrustum);
          SuperFrustum& superFrustum = *(lightFrustums[lightFrustums.Push (
            newFrust)]);
          
          superFrustum.actualNumParts = viewSetup.numParts;
          superFrustum.frustums =
            new typename SuperFrustum::Frustum[superFrustum.actualNumParts];

          const csReversibleTransform& world2light_base (
            light->GetMovable()->GetFullTransform());

          superFrustum.world2light_rotated = world2light_base;
          superFrustum.world2light_rotated.SetO2T (
            superFrustum.world2light_rotated.GetO2T());

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

            lightFrustum.splitDistsSV = lightVarsHelper.CreateTempSV(
              viewSetup.persist.splitDistsSVName);

            superFrustum.farZSV = lightVarsHelper.CreateTempSV (
              viewSetup.persist.farZSVName);
            superFrustum.numSplitsSV = lightVarsHelper.CreateTempSV (
              viewSetup.persist.numSplitsSVName);

            size_t numTex = viewSetup.persist.settings.targets.GetSize();
            for (size_t t = 0; t < numTex; t++)
            {
              const ShadowSettings::Target* target =
                viewSetup.persist.settings.targets[t];
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

          uint currentFrame = viewSetup.rview->GetCurrentFrameNumber();
          if (lightFrustums.setupFrame
            == currentFrame)
            return;
          lightFrustums.setupFrame = currentFrame;

          typename RenderTree::ContextNode& context = meshNode->GetOwner();
          CS::RenderManager::RenderView* rview = context.renderView;

          float _near = FLT_MAX;
          float _far = FLT_MIN;

          typename RenderTree::ContextNode::TreeType::MeshNodeTreeIteratorType it = 
            context.meshNodes.GetIterator ();

          while (it.HasNext ())
          {
            typename RenderTree::ContextNode::TreeType::MeshNode *node = it.Next ();
            CS_ASSERT_MSG("Null node encountered, should not be possible", node);

            // setup split dists
            for (int i = 0 ; i < node->meshes.GetSize(); i ++)
            {
              typename RenderTree::ContextNode::TreeType::MeshNode::SingleMesh mesh = node->meshes.Get(i);
              
              csVector3 meshPosition = 
                mesh.meshWrapper->GetMovable()->GetPosition();
              csReversibleTransform meshTransform = 
                mesh.meshWrapper->GetMovable()->GetTransform();
              csRef<csRenderBufferHolder> buffers = 
                mesh.renderMesh->buffers;
              iRenderBuffer* positions = buffers->GetRenderBuffer (CS_BUFFER_POSITION);
              csVertexListWalker<float, csVector3> positionWalker (positions);
              csVector3 lightPosition = light->GetMovable()->GetPosition();
              csReversibleTransform lightTransform = 
                light->GetMovable()->GetTransform();    

//               csPrintf("%f %f %f\n", lightTransform.GetO2T().Col1().x, 
//                 lightTransform.GetO2T().Col1().y, 
//                 lightTransform.GetO2T().Col1().z);
//               csPrintf("%f %f %f\n", lightTransform.GetO2T().Col2().x, 
//                 lightTransform.GetO2T().Col2().y, 
//                 lightTransform.GetO2T().Col2().z);
//               csPrintf("%f %f %f\n\n", lightTransform.GetO2T().Col3().x, 
//                 lightTransform.GetO2T().Col3().y, 
//                 lightTransform.GetO2T().Col3().z);

//               csPrintf("%f %f %f\n", lightPosition.x, lightPosition.y, 
//                 lightPosition.z);

              // only take into account translucent objects
              if ( mesh.meshWrapper->GetRenderPriority() != 
                rview->GetEngine()->GetRenderPriority("alpha"))
                continue;

              // Iterate on all vertices
              for (size_t i = 0; i < positionWalker.GetSize (); i++)
              {
                float distance = (lightPosition - 
                  meshTransform.This2Other(*positionWalker)).Norm ();

                if (distance < _near)
                  _near = distance;

                if (distance > _far)
                  _far = distance;

                ++positionWalker;
              }
            }
          }

          // here should be lightFrustums.frustums.GetSize()
//           csPrintf("%d\n", lightFrustums.frustums.GetSize());
          for (size_t l = 0; l < 1; l++)
          {
            const SuperFrustum& superFrust = *(lightFrustums.frustums[l]);
            
            for (int frustNum = 0 ; frustNum < superFrust.actualNumParts ; frustNum ++)
            {

              viewSetup.splitDists[frustNum] = _near + (_far - _near) * 
                ((float)frustNum / (superFrust.actualNumParts - 1));

              const typename SuperFrustum::Frustum& lightFrust = 
                superFrust.frustums[frustNum];
              csRef<CS::RenderManager::RenderView> newRenderView;
              newRenderView = renderTree.GetPersistentData().renderViews.CreateRenderView ();
              newRenderView->SetEngine (rview->GetEngine ());
              newRenderView->SetThisSector (rview->GetThisSector ());

              csBox3 castersBox = superFrust.castingObjectsBBoxPP;
              csBox3 receiversBox = lightFrust.receivingObjectsBBoxPP;
              // set up projection matrix
              const float focusMinX = csMax (receiversBox.MinX(), castersBox.MinX());
              const float focusMinY = csMax (receiversBox.MinY(), castersBox.MinY());
              const float focusMaxX = csMin (receiversBox.MaxX(), castersBox.MaxX());
              const float focusMaxY = csMin (receiversBox.MaxY(), castersBox.MaxY());
              const float frustW = focusMaxX - focusMinX;
              const float frustH = focusMaxY - focusMinY;
              const float cropScaleX = 2.0f/frustW;
              const float cropScaleY = 2.0f/frustH;
              const float cropShiftX =
                (-1.0f * (focusMaxX + focusMinX))/frustW;
              const float cropShiftY =
                (-1.0f * (focusMaxY + focusMinY))/frustH;
              CS::Math::Matrix4 crop = CS::Math::Matrix4 (
                cropScaleX, 0, 0, cropShiftX,
                0, cropScaleY, 0, cropShiftY,
                0, 0, 1, 0,
                0, 0, 0, 1);

              CS::Math::Matrix4 matrix = rview->GetCamera()->GetProjectionMatrix();

              CS::Math::Matrix4 Mortho = CS::Math::Projections::Ortho (-1, 1, 1, -1, 
                1, -1);

              float lightCutoff = light->GetCutoffDistance();
              float lightNear = SMALL_Z;

              lightProject = CS::Math::Projections::Ortho (lightCutoff, 
                -lightCutoff, lightCutoff, -lightCutoff, 
                -viewSetup.splitDists[frustNum], -lightNear);

              matrix = (Mortho * crop * lightProject);

//               lightCutoff = viewSetup.splitDists[frustNum];
//               lightNear = SMALL_Z;
// 
//               matrix = CS::Math::Projections::Ortho (lightCutoff, 
//                 -lightCutoff, lightCutoff, -lightCutoff, -lightCutoff, -lightNear);

              for (int i = 0; i < 4; i++)
              {
                csShaderVariable* item = lightFrust.shadowMapProjectSV->GetArrayElement (i);
                item->SetValue (matrix.Row (i));
              }

              lightFrust.splitDistsSV->SetValue(viewSetup.splitDists[frustNum]);
//               csPrintf("%f\n", viewSetup.splitDists[frustNum]);

              int shadowMapSize = viewSetup.persist.shadowMapRes;

              csRef<iCustomMatrixCamera> shadowViewCam =
                newRenderView->GetEngine()->CreateCustomMatrixCamera();
              newRenderView->SetCamera (shadowViewCam->GetCamera());

//               csPlane3 farplane(0,0,-1,viewSetup.splitDists[frustNum]);
//               shadowViewCam->GetCamera()->SetFarPlane(&farplane);

              shadowViewCam->SetProjectionMatrix (matrix);
              shadowViewCam->GetCamera()->SetTransform (light->GetMovable()->GetTransform());

              typename RenderTree::ContextNode* shadowMapCtx = 
                renderTree.CreateContext (newRenderView);

              for (size_t t = 0; t < persist.settings.targets.GetSize(); t++)
              {
                ShadowSettings::Target* target =
                  viewSetup.persist.settings.targets[t];
                iTextureHandle* tex = target->texCache.QueryUnusedTexture (
                  shadowMapSize, shadowMapSize);
                // register SVs
                lightFrust.textureSVs[target->attachment]->SetValue (tex);
                renderTree.AddDebugTexture (tex);

                shadowMapCtx->renderTargets[target->attachment].texHandle = tex;
                shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
              }

              csBox2 clipBox (0, 0, shadowMapSize, shadowMapSize);
              csRef<iClipper2D> newView;
              newView.AttachNew (new csBoxClipper (clipBox));
              newRenderView->SetClipper (newView);

              // Setup the new context
              ShadowmapContextSetup contextFunction (layerConfig,
                persist.shaderManager, viewSetup, false);
              contextFunction (*shadowMapCtx);
            }
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
//             StandardMeshSorter<RenderTree> mySorter (rview->GetEngine ());
//             mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
//             ForEachMeshNode (context, mySorter);
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
        LightingVariablesHelper::PersistentData lightVarsPersist;
        iShaderManager* shaderManager;
        iGraphics3D* g3d;

        int numSplits;
        float farZ;
        int shadowMapRes;
        csString configPrefix;
        ShadowSettings settings;
        CS::ShaderVarStringID farZSVName;
        CS::ShaderVarStringID numSplitsSVName;
        CS::ShaderVarStringID splitDistsSVName;

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

          farZSVName = strings->Request ("light farZ");
          numSplitsSVName = strings->Request ("light numSplits");
          splitDistsSVName = strings->Request ("light splitDists");

          csConfigAccess cfg (objectReg);
          if (configPrefix.IsEmpty())
          {
            settings.ReadSettings (objectReg, "Alpha");
          }
          else
          {
            settings.ReadSettings (objectReg, 
              cfg->GetStr (
              csString().Format ("%s.ShadowsType", configPrefix.GetData()), "Alpha"));
            numSplits = cfg->GetInt (
              csString().Format ("%s.NumSplits", configPrefix.GetData()), 2);
            farZ = cfg->GetFloat (
              csString().Format ("%s.FarZ", configPrefix.GetData()), 100);
            shadowMapRes = cfg->GetInt (
              csString().Format ("%s.ShadowMapResolution", configPrefix.GetData()), 512);
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

//         csPrintf("sub %d\n", subLightNum);

        csVector3 vLight;
        csBox3 meshBboxWorld (singleMesh.renderMesh->object2world.This2Other (
          singleMesh.renderMesh->bbox));
        vLight = superFrust.world2light_rotated.Other2This (
          meshBboxWorld.GetCorner (0));
        csBox3 meshBboxLightPP;
        csVector4 vLightPP;
        vLightPP = lightData.lightProject * csVector4 (vLight);
        //vLightPP /= vLightPP.w;
        meshBboxLightPP.StartBoundingBox (csVector3 (vLightPP.x,
          vLightPP.y, vLightPP.z));
        for (int c = 1; c < 8; c++)
        {
          vLight = superFrust.world2light_rotated.Other2This (
            meshBboxWorld.GetCorner (c));
          vLightPP = lightData.lightProject * csVector4 (vLight);
          //vLightPP /= vLightPP.w;
          meshBboxLightPP.AddBoundingVertexSmart (csVector3 (vLightPP.x,
            vLightPP.y, vLightPP.z));
        }

        // Mesh casts shadow
        superFrust.castingObjectsBBoxPP += meshBboxLightPP;

        uint spreadFlags = 0;
        int s = 0;

        for (int f = 0; f < superFrust.actualNumParts; f++)
        {
          typename CachedLightData::SuperFrustum::Frustum& lightFrustum =
            superFrust.frustums[f];

          lightFrustum.receivingObjectsBBoxPP += meshBboxLightPP;

          lightVarsHelper.MergeAsArrayItem (lightStacks[lightNum],
            lightFrustum.shadowMapProjectSV, s);

          lightVarsHelper.MergeAsArrayItem(lightStacks[lightNum],
            lightFrustum.splitDistsSV, s);

          for (size_t t = 0; t < persist.settings.targets.GetSize(); t++)
          {
            const ShadowSettings::Target* target =
              viewSetup.persist.settings.targets[t];
            lightVarsHelper.MergeAsArrayItem (lightStacks[lightNum], 
              lightFrustum.textureSVs[target->attachment], s);
          }
          spreadFlags |= (1 << s);
          s++;        
        }

        CS::ShaderVarStringID name = superFrust.numSplitsSV->GetName();
        superFrust.numSplitsSV = lightVarsHelper.CreateVarOnStack(name, 
          lightStacks[lightNum]);
        superFrust.numSplitsSV->SetValue(viewSetup.persist.numSplits);

        name = superFrust.farZSV->GetName();
        superFrust.farZSV = lightVarsHelper.CreateVarOnStack(name, 
          lightStacks[lightNum]);
        superFrust.farZSV->SetValue(viewSetup.persist.farZ);

        // here might be number of lights
        return 1;
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
