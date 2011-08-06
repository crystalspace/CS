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
        PersistentData& persist;
        CS::RenderManager::RenderView* rview;

        SingleRenderLayer depthRenderLayer;

        ViewSetup (PersistentData& persist, CS::RenderManager::RenderView* rview)
          : persist (persist), rview (rview),
          depthRenderLayer (persist.osmShaderType, 
          persist.osmShader) { }

        ~ViewSetup() { }
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
          csRef<csShaderVariable> numSplitsSV;
	        CS::Utility::MeshFilter meshFilter;

          struct Frustum
          {
            float splitDists;
            csRef<csShaderVariable> splitDistsSV;
          };

          csRef<csShaderVariable> depthStartSV;
          csRef<csShaderVariable> depthEndSV;
          csRef<csShaderVariable> splitSV;
          csRef<csShaderVariable> textureSV[rtaNumAttachments];
          csRef<csShaderVariable> shadowMapProjectSV;
          Frustum* frustums;

          ~SuperFrustum() { delete[] frustums; }
        };
        typedef csRefArray<SuperFrustum> LightFrustumsArray;
        struct LightFrustums
        {
          uint frustumsSetupFrame;
          uint setupFrame;
          LightFrustumsArray frustums;

          LightFrustums() :  frustumsSetupFrame (~0), setupFrame (~0) {}
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

          if (lightFrustumsSettings.frustumsSetupFrame != currentFrame)
          {
            float lightNear = SMALL_Z;
            float lightCutoff = light->GetCutoffDistance();

            CS::Math::Matrix4 lightProject = CS::Math::Projections::Ortho (
              lightCutoff, -lightCutoff, lightCutoff, -lightCutoff,
              -lightCutoff, -lightNear);

            this->lightProject = lightProject;

            LightFrustumsArray& lightFrustums =
              lightFrustumsSettings.frustums;

            LightingVariablesHelper lightVarsHelper
              (viewSetup.persist.lightVarsPersist);

            csRef<SuperFrustum> newFrust;
            newFrust.AttachNew (new SuperFrustum);
            SuperFrustum& superFrustum = *(lightFrustums[lightFrustums.Push (
              newFrust)]);

	          superFrustum.meshFilter.SetFilterMode (CS::Utility::MESH_FILTER_INCLUDE);
            superFrustum.actualNumParts = viewSetup.persist.numSplits;
            superFrustum.frustums =
              new typename SuperFrustum::Frustum[superFrustum.actualNumParts];

            superFrustum.shadowMapProjectSV = lightVarsHelper.CreateTempSV (
              viewSetup.persist.svNames.GetLightSVId (
              csLightShaderVarCache::lightShadowMapProjection));
            superFrustum.shadowMapProjectSV->SetArraySize (4);

            for (int j = 0; j < 4; j++)
            {
              csShaderVariable* item = lightVarsHelper.CreateTempSV (
                CS::InvalidShaderVarStringID);
              superFrustum.shadowMapProjectSV->SetArrayElement (j, item);
            }

            superFrustum.numSplitsSV = lightVarsHelper.CreateTempSV (
              viewSetup.persist.numSplitsSVName);

            superFrustum.depthStartSV =
              lightVarsHelper.CreateTempSV(viewSetup.persist.depthStartSVName);

            superFrustum.depthEndSV =
              lightVarsHelper.CreateTempSV(viewSetup.persist.depthEndSVName);

            superFrustum.splitSV =
              lightVarsHelper.CreateTempSV(viewSetup.persist.splitSVName);

            for (int i = 0; i < superFrustum.actualNumParts; i++)
            {
              typename SuperFrustum::Frustum& lightFrustum =
                superFrustum.frustums[i];
              lightFrustum.splitDistsSV = lightVarsHelper.CreateTempSV(
                viewSetup.persist.splitDistsSVName);

              if (i % 4 == 3)
                superFrustum.textureSV[rtaColor0 + i / 4] =
                  lightVarsHelper.CreateTempSV (viewSetup.persist.osmSVName);
            }

            lightFrustumsSettings.frustumsSetupFrame = currentFrame;
          }
        }

        void ProcessGeometry(SuperFrustum* superFrust, 
          typename RenderTree::ContextNode& context, iLight* light, 
          CS::RenderManager::RenderView* rview, float& _near, float& _far, 
          csBox3& castingObjects, csBox3& receivingObjects)
        {
          typename RenderTree::ContextNode::TreeType::MeshNodeTreeIteratorType 
            it = context.meshNodes.GetIterator ();

          while (it.HasNext ())
          {
            typename RenderTree::ContextNode::TreeType::MeshNode *node = it.Next();
            CS_ASSERT_MSG("Null node encountered, should not be possible", node);

            // setup split dists
            for (size_t i = 0 ; i < node->meshes.GetSize(); i ++)
            {
              typename RenderTree::ContextNode::TreeType::MeshNode::SingleMesh 
                mesh = node->meshes.Get(i);
              
              csReversibleTransform meshTransform = 
                mesh.meshWrapper->GetMovable()->GetTransform();
//               csRef<csRenderBufferHolder> buffers = 
//                 mesh.renderMesh->buffers;
//               iRenderBuffer* positions = 
//                 buffers->GetRenderBuffer (CS_BUFFER_POSITION);
//               csVertexListWalker<float, csVector3> positionWalker (positions);
              csVector3 lightPosition = light->GetMovable()->GetPosition();

              // only take into account translucent objects
              if ( mesh.meshWrapper->GetRenderPriority() != 
                rview->GetEngine()->GetRenderPriority("alpha"))
                continue;

              // add to mesh filter include
	            superFrust->meshFilter.AddFilterMesh (mesh.meshWrapper);

              csReversibleTransform world2light = 
                light->GetMovable()->GetFullTransform();

              csVector3 vLight;
              csBox3 meshBboxWorld (mesh.renderMesh->object2world.This2Other (
                mesh.renderMesh->bbox));
              vLight = world2light.Other2This ( meshBboxWorld.GetCorner (0));
              csBox3 meshBboxLightPP;
              csVector4 vLightPP;
              vLightPP = lightProject * csVector4 (vLight);
              //vLightPP /= vLightPP.w;
              meshBboxLightPP.StartBoundingBox (csVector3 (vLightPP.x,
                vLightPP.y, vLightPP.z));
              for (int c = 1; c < 8; c++)
              {
                vLight = world2light.Other2This ( meshBboxWorld.GetCorner (c));
                vLightPP = lightProject * csVector4 (vLight);
                //vLightPP /= vLightPP.w;
                meshBboxLightPP.AddBoundingVertexSmart (csVector3 (vLightPP.x,
                  vLightPP.y, vLightPP.z));

                float distance = (lightPosition - 
                  meshBboxWorld.GetCorner (c)).Norm ();

                if (distance < _near)
                  _near = distance;

                if (distance > _far)
                  _far = distance;
              }

              castingObjects += meshBboxLightPP;
              receivingObjects += meshBboxLightPP;
                  
              // Iterate on all vertices
//               for (size_t i = 0; i < positionWalker.GetSize (); i++)
//               {
//                 float distance = (lightPosition - 
//                   meshTransform.This2Other(*positionWalker)).Norm ();
// 
//                 if (distance < _near)
//                   _near = distance;
// 
//                 if (distance > _far)
//                   _far = distance;
// 
//                 ++positionWalker;
//               }
            }
          }
        }

        CS::Math::Matrix4 FindOrthCropMatrix(CS::RenderManager::RenderView* rview,
          iLight* light, const csBox3& castingObjects, 
          const csBox3& receivingObjects)
        {
          csBox3 castersBox = castingObjects;
          csBox3 receiversBox = receivingObjects;
          // set up projection matrix
          const float focusMinX = csMax(receiversBox.MinX(), castersBox.MinX());
          const float focusMinY = csMax(receiversBox.MinY(), castersBox.MinY());
          const float focusMaxX = csMin(receiversBox.MaxX(), castersBox.MaxX());
          const float focusMaxY = csMin(receiversBox.MaxY(), castersBox.MaxY());
          const float frustW = focusMaxX - focusMinX;
          const float frustH = focusMaxY - focusMinY;
          const float cropScaleX = 2.0f / frustW;
          const float cropScaleY = 2.0f / frustH;
          const float cropShiftX =
            (-1.0f * (focusMaxX + focusMinX) ) / frustW;
          const float cropShiftY =
            (-1.0f * (focusMaxY + focusMinY) ) / frustH;
          CS::Math::Matrix4 crop = CS::Math::Matrix4 (
            cropScaleX, 0, 0, cropShiftX,
            0, cropScaleY, 0, cropShiftY,
            0, 0, 1, 0,
            0, 0, 0, 1);

          CS::Math::Matrix4 matrix = rview->GetCamera()->GetProjectionMatrix();

          CS::Math::Matrix4 Mortho = 
            CS::Math::Projections::Ortho (-1, 1, 1, -1, 1, -1);

          return Mortho * crop;
        }

        void AddOSMTarget (typename RenderTree::MeshNode* meshNode,
          RenderTree& renderTree, iLight* light, ViewSetup& viewSetup)
        {
          if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

          LightFrustums* lightFrustumsPtr =
            lightFrustumsHash.GetElementPointer (
            viewSetup.rview->GetCamera());

          LightFrustums& lightFrustums = *lightFrustumsPtr;

          uint currentFrame = viewSetup.rview->GetCurrentFrameNumber();

          // fixes a memory leak
          if (lightFrustums.setupFrame == currentFrame ||
            lightFrustums.setupFrame != (uint)-1)
            return;

          lightFrustums.setupFrame = currentFrame;

          typename RenderTree::ContextNode& context = meshNode->GetOwner();
          CS::RenderManager::RenderView* rview = context.renderView;
          PersistentData& persist = viewSetup.persist;
          SingleRenderLayer& layerConfig = viewSetup.depthRenderLayer;

          float _near = FLT_MAX;
          float _far = FLT_MIN;

          csBox3 castingObjects;
          csBox3 receivingObjects;
  
          ProcessGeometry(lightFrustums.frustums[0], context, light, rview, 
            _near, _far, castingObjects, receivingObjects);

          CS::Math::Matrix4 proj = FindOrthCropMatrix(rview, light, 
            castingObjects, receivingObjects);

          float lightCutoff = light->GetCutoffDistance();
          float lightNear = SMALL_Z;

          lightProject = CS::Math::Projections::Ortho (lightCutoff, 
            -lightCutoff, lightCutoff, -lightCutoff, 
            -lightNear, -_far);

          CS::Math::Matrix4 matrix = proj * lightProject;

          AddShadowMapTarget(meshNode, renderTree, light, 
            viewSetup, matrix, persist.depthEnd);

          lightProject = CS::Math::Projections::Ortho (lightCutoff, 
            -lightCutoff, lightCutoff, -lightCutoff, 
            -_far, -lightNear);

          matrix = proj * lightProject;

          AddShadowMapTarget(meshNode, renderTree, light, 
            viewSetup, matrix, persist.depthStart);

          int shadowMapSize = persist.shadowMapRes;

          // here should be lightFrustums.frustums.GetSize()
//           csPrintf("%d\n", lightFrustums.frustums.GetSize());
          for (size_t l = 0; l < 1; l++)
          {
            const SuperFrustum& superFrust = *(lightFrustums.frustums[l]);            

            for (int attachments = 0 ; attachments < persist.mrt ; attachments ++)
            {
              for (int channels = 0 ; channels < 4 ; channels ++)
              {
                int frustNum = 4 * attachments + channels;
                typename SuperFrustum::Frustum& lightFrust = 
                  superFrust.frustums[frustNum];
                lightFrust.splitDists = _near + (_far - _near) * 
                  ((float)frustNum / (superFrust.actualNumParts - 1));

                lightFrust.splitDistsSV->SetValue(lightFrust.splitDists);

                // fill in split dists for osm
                csRef<csShaderVariable> passColorSV = persist.osmShader->
                  GetVariableAdd(persist.passColorSVName);

                passColorSV->SetArrayElement(frustNum, lightFrust.splitDistsSV);
              }

              iTextureHandle *tex= persist.texs[attachments];
              superFrust.textureSV[rtaColor0 + attachments]->SetValue (tex);
            }

            superFrust.depthStartSV->SetValue(persist.depthStart);
            superFrust.depthEndSV->SetValue(persist.depthEnd);
            superFrust.splitSV->SetValue(persist.split);

            csRef<CS::RenderManager::RenderView> newRenderView;
            newRenderView = 
              renderTree.GetPersistentData().renderViews.CreateRenderView ();
            newRenderView->SetEngine (rview->GetEngine ());
            newRenderView->SetThisSector (rview->GetThisSector ());

            // we only need one shadow map project sv per light
            for (int i = 0; i < 4; i++)
            {
              csShaderVariable* item = 
                superFrust.shadowMapProjectSV->GetArrayElement (i);
              item->SetValue (matrix.Row (i));
            }

            // pass the matrix to the shader that generates OSMs
            persist.osmShader->AddVariable(superFrust.shadowMapProjectSV);

            csRef<iCustomMatrixCamera> shadowViewCam =
              newRenderView->GetEngine()->CreateCustomMatrixCamera();
            newRenderView->SetCamera (shadowViewCam->GetCamera());

            shadowViewCam->SetProjectionMatrix (matrix);
            shadowViewCam->GetCamera()->SetTransform 
              (light->GetMovable()->GetTransform());

            newRenderView->SetViewDimensions (shadowMapSize, shadowMapSize);
            csBox2 clipBox (0, 0, shadowMapSize, shadowMapSize);
            csRef<iClipper2D> newView;
            newView.AttachNew (new csBoxClipper (clipBox));
            newRenderView->SetClipper (newView);
            newRenderView->SetMeshFilter(superFrust.meshFilter);

            typename RenderTree::ContextNode* shadowMapCtx = 
              renderTree.CreateContext (newRenderView);

            for (int t = 0; t < persist.mrt; t++)
            {
              renderTree.AddDebugTexture (persist.texs[t]);
              // register SVs
              shadowMapCtx->renderTargets[rtaColor0 + t].texHandle = 
                persist.texs[t];
              shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
            }

            ShadowmapContextSetup contextFunction (layerConfig,
              persist.shaderManager, viewSetup);
            contextFunction (*shadowMapCtx);            
          }
        }

        void AddShadowMapTarget (typename RenderTree::MeshNode* meshNode,
          RenderTree& renderTree, iLight* light, ViewSetup& viewSetup, 
          const CS::Math::Matrix4& matrix, iTextureHandle* renderTexture)
        {
          if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

          LightFrustums* lightFrustumsPtr =
            lightFrustumsHash.GetElementPointer (
            viewSetup.rview->GetCamera());

          LightFrustums& lightFrustums = *lightFrustumsPtr;

          typename RenderTree::ContextNode& context = meshNode->GetOwner();
          CS::RenderManager::RenderView* rview = context.renderView;

          PersistentData& persist = viewSetup.persist;
          int shadowMapSize = persist.shadowMapRes;

          // here should be lightFrustums.frustums.GetSize()
//           csPrintf("%d\n", lightFrustums.frustums.GetSize());
          for (size_t l = 0; l < 1; l++)
          {
            const SuperFrustum& superFrust = *(lightFrustums.frustums[l]);            

            csRef<CS::RenderManager::RenderView> newRenderView;
            newRenderView = 
              renderTree.GetPersistentData().renderViews.CreateRenderView ();
            newRenderView->SetEngine (rview->GetEngine ());
            newRenderView->SetThisSector (rview->GetThisSector ());

            csRef<iCustomMatrixCamera> shadowViewCam =
              newRenderView->GetEngine()->CreateCustomMatrixCamera();
            newRenderView->SetCamera (shadowViewCam->GetCamera());

            shadowViewCam->SetProjectionMatrix (matrix);
            shadowViewCam->GetCamera()->SetTransform 
              (light->GetMovable()->GetTransform());

            newRenderView->SetViewDimensions (shadowMapSize, shadowMapSize);
            csBox2 clipBox (0, 0, shadowMapSize, shadowMapSize);
            csRef<iClipper2D> newView;
            newView.AttachNew (new csBoxClipper (clipBox));
            newRenderView->SetClipper (newView);
            newRenderView->SetMeshFilter(superFrust.meshFilter);

            typename RenderTree::ContextNode* shadowMapCtx = 
              renderTree.CreateContext (newRenderView);

            renderTree.AddDebugTexture (renderTexture);
            // register SVs
            shadowMapCtx->renderTargets[rtaColor0].texHandle = renderTexture;
            shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;

            SingleRenderLayer layerConfig(persist.shadowShaderType, 
              persist.shadowShader);

            ShadowmapContextSetup contextFunction (layerConfig,
              persist.shaderManager, viewSetup);
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
          iShaderManager* shaderManager, ViewSetup& viewSetup)
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
//             StandardMeshSorter<RenderTree> mySorter (rview->GetEngine ());
//             mySorter.SetupCameraLocation 
//               (rview->GetCamera ()->GetTransform ().GetOrigin ());
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

        int mrt;
        int numSplits;
        int shadowMapRes;
        int splitRes;
        csString configPrefix;
        CS::ShaderVarStringID numSplitsSVName;
        CS::ShaderVarStringID splitDistsSVName;
        CS::ShaderVarStringID passColorSVName;
        CS::ShaderVarStringID mrtSVName;
        CS::ShaderVarStringID osmSVName;
        CS::ShaderVarStringID depthStartSVName;
        CS::ShaderVarStringID depthEndSVName;
        CS::ShaderVarStringID splitSVName;
        /// Shader for rendering to OSM
        csRef<iShader> osmShader;
        /// Shader type for rendering to OSM
        csStringID osmShaderType;
        /// Shader for rendering to shadow map
        csRef<iShader> shadowShader;
        /// Shader type for rendering to shadow map
        csStringID shadowShaderType;

        csRefArray<iTextureHandle> texs;
        csRef<iTextureHandle> depthStart;
        csRef<iTextureHandle> depthEnd;
        csRef<iTextureHandle> split;

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
          mrt = 0;

          iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
          svNames.SetStrings (strings);

          numSplitsSVName = strings->Request ("light numSplits");
          splitDistsSVName = strings->Request ("light splitDists");
          passColorSVName = strings->Request ("pass color");
          mrtSVName = strings->Request ("mrt");
          osmSVName = strings->Request("light osm");
          depthStartSVName = strings->Request("light shadow map start");
          depthEndSVName = strings->Request("light shadow map end");
          splitSVName = strings->Request("split function");

          csConfigAccess cfg (objectReg);
          shadowMapRes = 512;
          if (!configPrefix.IsEmpty())
          {
            mrt = cfg->GetInt ( csString().Format (
              "%s.ForceTextureNumber", configPrefix.GetData()), 0);
            shadowMapRes = cfg->GetInt ( csString().Format (
              "%s.ShadowMapResolution", configPrefix.GetData()), 512);
          }
          
          const csGraphics3DCaps *caps = g3d->GetCaps();
          
          if (mrt > 0)  
            mrt = csMin(caps->MaxRTColorAttachments, mrt);
          else
            mrt = caps->MaxRTColorAttachments;

          csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));

          osmShader = loader->LoadShader ("/shader/shadow/shadow_osm.xml");
          osmShaderType = StringIDValue(strings->Request ("osm"));
          shadowShader = loader->LoadShader ("/shader/shadow/shadow_vsm.xml");
          shadowShaderType = StringIDValue(strings->Request ("shadow"));

          numSplits = 4 * mrt;
          
          depthStart = g3d->GetTextureManager()-> CreateTexture(shadowMapRes, 
            shadowMapRes, csimg2D, "abgr32_f", CS_TEXTURE_3D | CS_TEXTURE_CLAMP);

          depthEnd = g3d->GetTextureManager()-> CreateTexture(shadowMapRes, 
            shadowMapRes, csimg2D, "abgr32_f", CS_TEXTURE_3D | CS_TEXTURE_CLAMP);

          for (int i = 0 ; i < mrt ; i ++)
          {
            csRef<iTextureHandle> tex = g3d->GetTextureManager()->
              CreateTexture(shadowMapRes, shadowMapRes, csimg2D, "abgr32_f", 
              CS_TEXTURE_3D | CS_TEXTURE_CLAMP);
            texs.Push(tex);
          }

          splitRes = 512;
          // Create a default texture
          split = g3d->GetTextureManager()->CreateTexture(splitRes, 1, csimg2D, 
            "abgr8", CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP);

          // Set linear or logarithmic split
          SetHybridSplit(0.75, splitRes);

          osmShader->GetVariableAdd(numSplitsSVName)->SetValue(numSplits);
          osmShader->GetVariableAdd(splitSVName)->SetValue(split);
          osmShader->GetVariableAdd(depthStartSVName)->SetValue(depthStart);
          osmShader->GetVariableAdd(depthEndSVName)->SetValue(depthEnd);

          // pass mrt number to shader
          csShaderVariable* mrtVar = new csShaderVariable(mrtSVName); 
          mrtVar->SetValue(mrt);
          shaderManager->AddVariable(mrtVar);
        }
        void UpdateNewFrame ()
        {
          lightVarsPersist.UpdateNewFrame();
        }

      private:       
        void SetHybridSplit(float logValue, int textureSize)
        {
          CS::StructuredTextureFormat readbackFmt 
            (CS::TextureFormatStrings::ConvertStructured ("abgr8"));
          csRef<iDataBuffer> databuf = split->Readback(readbackFmt);

          if (!databuf)
          {
            csPrintfErr("Error reading back from texture!\n");
            return;
          }

          uint8* data = databuf->GetUint8();

          if (!data)
          {
            csPrintfErr("Error converting data buffer!\n");
            return;
          }

          double end = textureSize - 1;
          double range = (int)(log(end - 1.0)/log(2.0));
          double start = end - range;

//           csPrintf("%lf %lf %lf\n", start, end, range);
          data[0] = 0;
          for (int i = 4 ; i < 4 * textureSize ; i += 4)
          {
            data[i] = (unsigned char)(csMin( (1 - logValue) * i / 4 + logValue *
              ( (log(pow(2.0, start) * (i / 4.0)) / log(2.0) - start) 
                * end / range ) , end) * 255 / end);
//             csPrintf("%d ", data[i] );
          }

          split->Blit(0, 0, textureSize, 1, data);  
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
        LightingVariablesHelper lightVarsHelper 
          (viewSetup.persist.lightVarsPersist);

        typename CachedLightData::LightFrustumsArray& lightFrustums =
          lightData.lightFrustumsHash.GetElementPointer (
          viewSetup.rview->GetCamera())->frustums;
        typename CachedLightData::SuperFrustum& superFrust =
          *(lightFrustums[subLightNum]);

        for (int f = 0; f < superFrust.actualNumParts; f++)
        {
          typename CachedLightData::SuperFrustum::Frustum& lightFrustum =
            superFrust.frustums[f];
          
          if (f % 4 == 3)    
            lightVarsHelper.MergeAsArrayItem (lightStacks[0], 
              superFrust.textureSV[rtaColor0 + f / 4], 8 * lightNum + f / 4);

          lightVarsHelper.MergeAsArrayItem(lightStacks[0],
            lightFrustum.splitDistsSV, 8 * lightNum + f);
        }

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.depthStartSV, lightNum);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.depthEndSV, lightNum);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.splitSV, lightNum);

        superFrust.numSplitsSV->SetValue(viewSetup.persist.numSplits);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.shadowMapProjectSV, lightNum);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.numSplitsSV, lightNum);

        // here might be number of lights
        return 1;
      }

      static bool NeedFinalHandleLight() { return true; }
      void FinalHandleLight (iLight* light, CachedLightData& lightData)
      {    
//         lightData.AddShadowMapTarget (meshNode, persist, renderTree, light, 
//           viewSetup);

        lightData.AddOSMTarget (meshNode, renderTree, light, viewSetup);

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
