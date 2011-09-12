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

/**\file
 * OSM shadow handler
 */

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
    /**
     * OSM shadow handler.
     * Usage: in the \c ShadowHandler argument of the LightSetup class.
     * In addition, on initialization the SetConfigPrefix() method of the shadow
     * handler persistent data (accessible through the light setup persistent
     * data) must be called, before the light setup persistent data is
     * initialized:
     * \code
     * // First, set prefix for configuration settings
     * lightPersistent.shadowPersist.SetConfigPrefix ("RenderManager.ShadowPSSM");
     * // Then, light setup (and shadow handler) persistent data is initialized
     * lightPersistent.Initialize (objectReg, treePersistent.debugPersist);
     * \endcode
     */
    template<typename RenderTree, typename LayerConfigType>
    class ShadowOSM
    {
    public:
      struct PersistentData;

      /**
       * Shadow per-view specific data.
       * An instance of this class needs to be created when rendering a view and
       * passed the light setup.
       * Example:
       * \code
       * // ... basic setup ...
       *
       * // Setup shadow handler per-view data
       * ShadowType::ViewSetup shadowViewSetup (
       *   rednerManager->lightPersistent.shadowPersist, renderView);
       *
       * // ... perform various tasks ...
       *
       * // Setup lighting for meshes
       * LightSetupType lightSetup (
       *  renderManager->lightPersistent, renderManager->lightManager,
       *  context.svArrays, layerConfig, shadowViewSetup);
       * \endcode
       * \sa LightSetup
       */
      class ViewSetup
      {
      public:
        PersistentData& persist;
        CS::RenderManager::RenderView* rview;

        SingleRenderLayer depthRenderLayer;

        ViewSetup (PersistentData& persist, 
          CS::RenderManager::RenderView* rview) : persist (persist), 
          rview (rview), depthRenderLayer (persist.osmShaderType, 
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
        struct SuperFrustum : 
          public CS::Utility::FastRefCount<SuperFrustum>
        {
          int actualNumParts;
          csRef<csShaderVariable> numSplitsSV;
          csRef<csShaderVariable> shadowMapResSV;
	        CS::Utility::MeshFilter meshFilter;

          csRef<csShaderVariable> depthStartSV;
          csRef<csShaderVariable> depthEndSV;
          csRef<csShaderVariable> splitSV;
          csRef<csShaderVariable> textureSV[rtaNumAttachments];
          csRef<csShaderVariable> shadowMapProjectSV;
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
          // No shadows return
          if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

          ViewSetup& viewSetup = shadows.viewSetup;

          PersistentData& persist = viewSetup.persist;
          // Recompute splitting ratio
          if( persist.dbgPersist->IsDebugFlagEnabled(persist.dbgChooseSplit) )
          {
            persist.splitRatio = -1;
            persist.bestSplitRatioCorrelation = FLT_MAX;
            persist.SetHybridSplit(0);
            persist.dbgPersist->EnableDebugFlag(persist.dbgChooseSplit,false);
          }

          uint currentFrame = viewSetup.rview->GetCurrentFrameNumber();
          // Avoid two updates on the same frame
          if (lastSetupFrame != currentFrame)
          {
            lightFrustumsHash.DeleteAll();
            lastSetupFrame = currentFrame;
          }          
          csRef<iCamera> camera (viewSetup.rview->GetCamera());

          LightFrustums& lightFrustumsSettings =
            lightFrustumsHash.GetOrCreate (
            camera, LightFrustums());

          // Light updates only once
          if (lightFrustumsSettings.frustumsSetupFrame != currentFrame)
          {
            float lightNear = SMALL_Z;
            float lightCutoff = light->GetCutoffDistance();

            //Find light projection
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

            // Configure current light frustum
	          superFrustum.meshFilter.SetFilterMode 
              (CS::Utility::MESH_FILTER_INCLUDE);
            superFrustum.actualNumParts = viewSetup.persist.numSplits;

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

            superFrustum.shadowMapResSV = lightVarsHelper.CreateTempSV (
              viewSetup.persist.shadowMapResSVName);

            superFrustum.depthStartSV =
              lightVarsHelper.CreateTempSV(viewSetup.persist.depthStartSVName);

            superFrustum.depthEndSV =
              lightVarsHelper.CreateTempSV(viewSetup.persist.depthEndSVName);

            superFrustum.splitSV =
              lightVarsHelper.CreateTempSV(viewSetup.persist.splitSVName);

            for (int i = 0; i < superFrustum.actualNumParts; i++)
            {
              if (i % 4 == 3)
                superFrustum.textureSV[rtaColor0 + i / 4] =
                  lightVarsHelper.CreateTempSV (viewSetup.persist.osmSVName);
            }

            lightFrustumsSettings.frustumsSetupFrame = currentFrame;
          }
        }

        // Compute objects limits 
        void ProcessGeometry(SuperFrustum* superFrust, 
          typename RenderTree::ContextNode& context, iLight* light, 
          CS::RenderManager::RenderView* rview, float& _near, float& _far, 
          csBox3& castingObjects, csBox3& receivingObjects, 
          bool showOpaqueObjects)
        {
          typename RenderTree::ContextNode::TreeType::MeshNodeTreeIteratorType 
            it = context.meshNodes.GetIterator ();

          // Iterate through all meshes
          while (it.HasNext ())
          {
            typename RenderTree::ContextNode::TreeType::MeshNode *node = 
              it.Next();
            CS_ASSERT_MSG
              ("Null node encountered, should not be possible", node);

            for (size_t i = 0 ; i < node->meshes.GetSize(); i ++)
            {
              typename RenderTree::ContextNode::TreeType::MeshNode::SingleMesh 
                mesh = node->meshes.Get(i);
              
              csReversibleTransform meshTransform = 
                mesh.meshWrapper->GetMovable()->GetTransform();
              csVector3 lightPosition = light->GetMovable()->GetPosition();

              // Add / Remove opaque objects from the crop matrix
              if (!showOpaqueObjects &&
                mesh.meshWrapper->GetRenderPriority() != 
                rview->GetEngine()->GetRenderPriority("alpha"))
                  continue;

              // Add to mesh filter
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
            }
          }
        }

        // Compute crop matrix
        CS::Math::Matrix4 FindOrthCropMatrix
          (CS::RenderManager::RenderView* rview,
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

        // Render pass for computing opacity maps
        void AddOSMTarget (typename RenderTree::MeshNode* meshNode,
          RenderTree& renderTree, iLight* light, ViewSetup& viewSetup)
        {
          if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;

          LightFrustums* lightFrustumsPtr =
            lightFrustumsHash.GetElementPointer (
            viewSetup.rview->GetCamera());

          LightFrustums& lightFrustums = *lightFrustumsPtr;

          uint currentFrame = viewSetup.rview->GetCurrentFrameNumber();

          // Fixes a memory leak
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
  
          // Find object limits
          ProcessGeometry(lightFrustums.frustums[0], context, light, rview, 
            _near, _far, castingObjects, receivingObjects, 
            persist.dbgPersist->
              IsDebugFlagEnabled(persist.dbgShowOpaqueObjects));

          // Then compute crop matrix
          CS::Math::Matrix4 proj = FindOrthCropMatrix(rview, light, 
            castingObjects, receivingObjects);

          float lightCutoff = light->GetCutoffDistance();
          float lightNear = SMALL_Z;

          // And finally compute projection matrix
          lightProject = CS::Math::Projections::Ortho (lightCutoff, 
            -lightCutoff, lightCutoff, -lightCutoff, 
            -lightNear, -_far);

          CS::Math::Matrix4 matrix = proj * lightProject;

          // Get end depth map
          AddShadowMapTarget(meshNode, renderTree, light, 
            viewSetup, matrix, persist.depthEnd);

          lightProject = CS::Math::Projections::Ortho (lightCutoff, 
            -lightCutoff, lightCutoff, -lightCutoff, 
            -_far, -lightNear);

          matrix = proj * lightProject;

          // Get start depth map
          AddShadowMapTarget(meshNode, renderTree, light, 
            viewSetup, matrix, persist.depthStart);

          int shadowMapSize = persist.shadowMapRes;

          // @@@ FIXME: Here should be lightFrustums.frustums.GetSize()
          for (size_t l = 0; l < 1; l++)
          {
            const SuperFrustum& superFrust = *(lightFrustums.frustums[l]);            

            for (int attachments = 0 ; attachments < persist.mrt ; attachments ++)
            {
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

            // We only need one shadow map project SV per light
            for (int i = 0; i < 4; i++)
            {
              csShaderVariable* item = 
                superFrust.shadowMapProjectSV->GetArrayElement (i);
              item->SetValue (matrix.Row (i));
            }

            // Pass the matrix to the shader that generates OSMs as well
            persist.osmShader->AddVariable(superFrust.shadowMapProjectSV);

            // Create render target
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
              if(persist.dbgPersist->
                IsDebugFlagEnabled(persist.dbgShowRenderTextures))
                  renderTree.AddDebugTexture (persist.texs[t]);
              // Register SVs
              shadowMapCtx->renderTargets[rtaColor0 + t].texHandle = 
                persist.texs[t];
              shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
            }

            ShadowmapContextSetup contextFunction (layerConfig,
              persist.shaderManager, viewSetup);
            contextFunction (*shadowMapCtx);            
          }

          ChooseSplitFunction(persist);
        }

        // Recompute the splitting function, when necessarily 
        void ChooseSplitFunction(PersistentData& persist)
        {
          if (persist.splitRatio >= -0.05 && persist.splitRatio <= 1.05)
          {
            csRef<iDataBuffer> databuf;
            uint8** data = new uint8*[persist.mrt];
            CS::StructuredTextureFormat readbackFmt 
              (CS::TextureFormatStrings::ConvertStructured ("abgr8"));

            // Get data from MRTs
            for (int i = 0 ; i < persist.mrt ; i ++)
            {
              databuf = persist.texs[i]->Readback(readbackFmt);
              data[i] = databuf->GetUint8();
            }

            csPrintf("split_ratio %lf \n", persist.splitRatio);

            // Compute correlation coefficient:
            double *means = new double[4 * persist.mrt];
            double *squareSum = new double[4 * persist.mrt];
            double *xy = new double[4 * persist.mrt];
            double *correlations = new double[4 * persist.mrt];

            for (int i = 0 ; i < 4 * persist.mrt ; i ++)
            {
              means[i] = 0;
              squareSum[i] = 0;
              xy[i] = 0;
            }

            // Use only one for to speed things up
            for (int layer = 0 ; layer < persist.mrt ; layer ++)
              for (int i = 0 ; i < persist.shadowMapRes ; i ++)
                for (int j = 0 ; j < persist.shadowMapRes ; j ++)
                  for (int k = 0 ; k < 4 ; k ++)
                  {
                    uint8 x = 
                      data[layer][4 * (i + j * persist.shadowMapRes) + k];
                    
                    means[4 * layer + k] += x;
                    squareSum[4 * layer + k] += (x * x);

                    if (4 * layer + k < 4 * persist.mrt - 1)
                    {
                      uint8 y = data[layer + (k + 1) / 4]
                        [4 * (i + j * persist.shadowMapRes) + (k + 1) % 4];
                      xy[4 * layer + k] += (x * y);
                    }
                  }

            int n = persist.shadowMapRes * persist.shadowMapRes;
            for (int i = 0 ; i < 4 * persist.mrt ; i ++)
              means[i] /= n;

            // Correlations between consecutive images
            for (int i = 0 ; i < 4 * persist.mrt - 1 ; i ++)
            {
              correlations[i] = (xy[i] - means[i] * means[i + 1] * n) /
                sqrt( (squareSum[i] - means[i] * means[i] * n) * 
                (squareSum[i + 1] - means[i + 1] * means[i + 1] * n));
            }

            // The sum of correlation coefficients
            double sum = 0;
            for (int i = 0 ; i < 4 * persist.mrt - 1 ; i ++)
              sum += correlations[i];

            csPrintf("Correlation coefficient: ");
            csPrintf("%lf \n", sum);

            // Choose the smallest sum, i.e. the biggest difference
            if (sum < persist.bestSplitRatioCorrelation)
            {
              persist.bestSplitRatioCorrelation = sum;
              persist.bestSplitRatio = persist.splitRatio;
            }

            if (persist.splitRatio < 0.95)
              persist.SetHybridSplit(persist.splitRatio + 0.1);
            else
            {
              persist.SetHybridSplit(persist.bestSplitRatio);
              csPrintf("Best Split is: %lf\n", persist.bestSplitRatio);
            }

            delete [] means;
            delete [] squareSum;
            delete [] xy;
            delete [] correlations;
            delete [] data;
          }

          persist.splitRatio += 0.1;
        }

        // Render pass to get a depth / shadow map
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

          // @@@ FIXME: here should be lightFrustums.frustums.GetSize()
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

            // Create new render pass
            newRenderView->SetViewDimensions (shadowMapSize, shadowMapSize);
            csBox2 clipBox (0, 0, shadowMapSize, shadowMapSize);
            csRef<iClipper2D> newView;
            newView.AttachNew (new csBoxClipper (clipBox));
            newRenderView->SetClipper (newView);
            newRenderView->SetMeshFilter(superFrust.meshFilter);

            typename RenderTree::ContextNode* shadowMapCtx = 
              renderTree.CreateContext (newRenderView);
            
            if(persist.dbgPersist->
              IsDebugFlagEnabled(persist.dbgShowRenderTextures))
                renderTree.AddDebugTexture (renderTexture);
            // Register SVs
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

          // Sort the mesh lists - not needed for OSM
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

        int mrt;
        int numSplits;
        int shadowMapRes;
        int splitRes;
        csString configPrefix;
        CS::ShaderVarStringID numSplitsSVName;
        CS::ShaderVarStringID shadowMapResSVName;
        CS::ShaderVarStringID mrtSVName;
        CS::ShaderVarStringID osmSVName;
        CS::ShaderVarStringID depthStartSVName;
        CS::ShaderVarStringID depthEndSVName;
        CS::ShaderVarStringID splitSVName;
        // Shader for rendering to OSM
        csRef<iShader> osmShader;
        // Shader type for rendering to OSM
        csStringID osmShaderType;
        // Shader for rendering to shadow map
        csRef<iShader> shadowShader;
        // Shader type for rendering to shadow map
        csStringID shadowShaderType;

        csRefArray<iTextureHandle> texs;
        csRef<iTextureHandle> depthStart;
        csRef<iTextureHandle> depthEnd;
        csRef<iTextureHandle> split;

        double splitRatio;
        double bestSplitRatio;
        double bestSplitRatioCorrelation;

        uint dbgChooseSplit;
        uint dbgShowRenderTextures;
        uint dbgShowOpaqueObjects;
        RenderTreeBase::DebugPersistent *dbgPersist;

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

          // Communication between the render manager and application level
          this->dbgPersist = &dbgPersist;
          dbgChooseSplit = 
            dbgPersist.RegisterDebugFlag ("draw.osm.choose.split");
          dbgShowRenderTextures =
            dbgPersist.RegisterDebugFlag ("draw.osm.render.textures");
          dbgPersist.EnableDebugFlag(dbgShowRenderTextures, true);
          dbgShowOpaqueObjects =
            dbgPersist.RegisterDebugFlag ("draw.osm.opaque.objects");
          dbgPersist.EnableDebugFlag(dbgShowOpaqueObjects, true);

          this->shaderManager = shaderManager;
          mrt = 0;

          iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
          svNames.SetStrings (strings);

          // Set up SV names
          numSplitsSVName = strings->Request ("light numSplits");
          shadowMapResSVName = strings->Request ("light shadow map size");
          mrtSVName = strings->Request ("mrt");
          osmSVName = strings->Request("light osm");
          depthStartSVName = strings->Request("light shadow map start");
          depthEndSVName = strings->Request("light shadow map end");
          splitSVName = strings->Request("split function");

          // Read settings from file (engine.cfg)
          csConfigAccess cfg (objectReg);
          shadowMapRes = 512;
          if (!configPrefix.IsEmpty())
          {
            mrt = cfg->GetInt ( csString().Format (
              "%s.ForceTextureNumber", configPrefix.GetData()), 0);
            shadowMapRes = cfg->GetInt ( csString().Format (
              "%s.ShadowMapResolution", configPrefix.GetData()), 512);
          }
          
          // Get the number of supported MRTs
          const csGraphics3DCaps *caps = g3d->GetCaps();
          if (mrt > 0)  
            mrt = csMin(caps->MaxRTColorAttachments, mrt);
          else
            mrt = caps->MaxRTColorAttachments;

          // Load shaders
          csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));
          osmShader = loader->LoadShader ("/shader/shadow/shadow_osm.xml");
          osmShaderType = StringIDValue(strings->Request ("osm"));
          shadowShader = loader->LoadShader ("/shader/shadow/shadow_vsm.xml");
          shadowShaderType = StringIDValue(strings->Request ("shadow"));

          numSplits = 4 * mrt;
          
          // Create textures
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

          splitRatio = 2;
          bestSplitRatio = 0;
          bestSplitRatioCorrelation = FLT_MAX;
          // Set linear or logarithmic split
          SetHybridSplit(1);

          osmShader->GetVariableAdd(numSplitsSVName)->SetValue(numSplits);
          osmShader->GetVariableAdd(splitSVName)->SetValue(split);
          osmShader->GetVariableAdd(depthStartSVName)->SetValue(depthStart);
          osmShader->GetVariableAdd(depthEndSVName)->SetValue(depthEnd);

          // Pass mrt number to shader, doesn't change w/ time
          csShaderVariable* mrtVar = new csShaderVariable(mrtSVName); 
          mrtVar->SetValue(mrt);
          shaderManager->AddVariable(mrtVar);
        }
        void UpdateNewFrame ()
        {
          lightVarsPersist.UpdateNewFrame();
        }

        // Set the ratio between linear and logarithmic split
        void SetHybridSplit(float logValue)
        {
          const int &textureSize = splitRes;
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

          // Write to texture
          data[0] = 0;
          for (int i = 4 ; i < 4 * textureSize ; i += 4)
          {
            data[i] = (unsigned char)(csMin( (1 - logValue) * i / 4 + logValue *
              ( (log(pow(2.0, start) * (i / 4.0)) / log(2.0) - start) 
                * end / range ) , end) * 255 / end);
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

        // Create shader variables arrays on stack
        for (int f = 0; f < superFrust.actualNumParts; f++)
        {

          if (f % 4 == 3)    
            lightVarsHelper.MergeAsArrayItem (lightStacks[0], 
              superFrust.textureSV[rtaColor0 + f / 4], 8 * lightNum + f / 4);
        }

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.depthStartSV, lightNum);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.depthEndSV, lightNum);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.splitSV, lightNum);

        superFrust.numSplitsSV->SetValue(viewSetup.persist.numSplits);
        superFrust.shadowMapResSV->SetValue(viewSetup.persist.shadowMapRes);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.shadowMapProjectSV, lightNum);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.numSplitsSV, lightNum);

        lightVarsHelper.MergeAsArrayItem (lightStacks[0],
          superFrust.shadowMapResSV, lightNum);

        // @@@ FIXME: here might be number of lights
        return 1;
      }

      static bool NeedFinalHandleLight() { return true; }
      void FinalHandleLight (iLight* light, CachedLightData& lightData)
      {    
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
