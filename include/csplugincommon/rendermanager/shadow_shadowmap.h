/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_SHADOWMAP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_SHADOWMAP_H__

#include "ivideo/shader/shader.h"

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/viscull.h"

#include "csgeom/matrix4.h"

class csShaderVariable;

namespace CS
{
namespace RenderManager
{

  template<typename RenderTree, typename LayerConfigType>
  class ShadowShadowmap
  {
    class ShadowMapData
    {
      csShaderVariable* shadowMapSV;
      csShaderVariable* shadowMapProjectSV;
      csRef<CS::RenderManager::RenderView> newRenderView;
    public:
      csShaderVariable* GetMapShaderVar() const
      { return shadowMapSV; }
      csShaderVariable* GetProjectionShaderVar() const
      { return shadowMapProjectSV; }
      CS::RenderManager::RenderView* GetNewRenderView() const
      { return newRenderView; }

      void Setup (RenderView::Pool& renderViewPool,
                  CS::RenderManager::RenderView* rview,
                  iLight* light, iTextureHandle* shadowTex, 
                  csShaderVariable* shadowMapSV,
                  csShaderVariable* shadowMapProjectSV,
                  LightingVariablesHelper& lightVarsHelper)
      {
        this->shadowMapSV = shadowMapSV;
        this->shadowMapProjectSV = shadowMapProjectSV;

        // Set up a shadow map view for light
#include "csutil/custom_new_disable.h"
	newRenderView.AttachNew (
	  new (renderViewPool) RenderView (*rview));
#include "csutil/custom_new_enable.h"
	newRenderView->SetEngine (rview->GetEngine ());
	newRenderView->SetThisSector (rview->GetThisSector ());
	iCamera* shadowViewCam = newRenderView->CreateNewCamera();
	shadowViewCam->SetTransform (light->GetMovable()->GetFullTransform ());
	shadowViewCam->SetPerspectiveCenter (128, 128);
	shadowViewCam->SetFOVAngle (shadowViewCam->GetFOVAngle(), 256);
	csBox2 clipBox (0, 0, 256, 256);
	csRef<iClipper2D> newView;
	newView.AttachNew (new csBoxClipper (clipBox));
	newRenderView->SetClipper (newView);

	shadowMapSV->SetValue (shadowTex);

	{
	  // Construct the projection matrix the same way gl3d does
	  CS::Math::Matrix4 Mortho (
	    2.0f/256,         0,       0,           -1.0f,
		  0,    -2.0f/256,       0,           1.0f,
		  0,         0, -2.0f/11.0f, -9.0f/11.0f,
		  0,         0,       0,               1);

	  CS::Math::Matrix4 Mtranslate (
	    1, 0, 0, 128,
	    0, 1, 0, 128,
	    0, 0, 1,    0,
	    0, 0, 0,    1);

	  float invAspect = 1.0f/shadowViewCam->GetFOV();
	  CS::Math::Matrix4 Mprojection (
	    1, 0, 0, 0,
	    0, 1, 0, 0,
	    0, 0, 0, -invAspect,
	    0, 0, invAspect, 0);

	  CS::Math::Matrix4 Mfinal =
	    (Mortho * Mtranslate) * Mprojection;

	  shadowMapProjectSV->SetArraySize (4);
	  for (int i = 0; i < 4; i++)
	  {
	    csShaderVariable* item = lightVarsHelper.CreateTempSV (
	      CS::InvalidShaderVarStringID);
	    item->SetValue (Mfinal.Row (i));
	    shadowMapProjectSV->SetArrayElement (i, item);
	  }
        }
      }
    };

    struct CachedLightData
    {
      const csRefArray<csShaderVariable>* shaderVars;

      ShadowMapData shadowMapData;
      uint shadowMapCreateFrame;

      CachedLightData() : shadowMapCreateFrame (~0) {}
    };

    class ShadowmapContextSetup
    {
    public:
      ShadowmapContextSetup (const LayerConfigType& layerConfig,
        iShaderManager* shaderManager)
	: layerConfig (layerConfig), shaderManager (shaderManager)
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
    
        // TODO: portals
	
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
	  StandardSVSetup<RenderTree, MultipleRenderLayer> svSetup (
	    context.svArrays, layerConfig);
    
	  ForEachMeshNode (context, svSetup);
	}
    
	SetupStandardShader (context, shaderManager, layerConfig);
    
	// Setup shaders and tickets
	SetupStandardTicket (context, shaderManager, layerConfig);
      }
    
    
    private:
      const LayerConfigType& layerConfig;
      iShaderManager* shaderManager;
    };
  public:

    struct PersistentData
    {
      uint frameNum;
      csLightShaderVarCache svNames;
      CS::ShaderVarStringID svPassNum;
      LightingSorter::PersistentData lightSorterPersist;
      LightingVariablesHelper::PersistentData lightVarsPersist;
      csHash<CachedLightData, csPtrKey<iLight> > lightDataCache;
      iShaderManager* shaderManager;

      TextureCache texCache;

      PersistentData() : frameNum (0),
        texCache (csimg2D, "d32", 
          CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP,
          "shadowmap", TextureCache::tcachePowerOfTwo)
      {
      }

      ~PersistentData()
      {
        if (lcb.IsValid()) lcb->parent = 0;
      }
      
      void Initialize (iShaderManager* shaderManager)
      {
        this->shaderManager = shaderManager;
        iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	svNames.SetStrings (strings);
        svPassNum = strings->Request ("pass number");
      }
      void UpdateNewFrame ()
      {
        frameNum++;
        csTicks time = csGetTicks ();
        texCache.AdvanceFrame (time);
      }

      iLightCallback* GetLightCallback()
      {
        if (!lcb.IsValid()) lcb.AttachNew (new LightCallback (this));
        return lcb;
      }
    protected:
      class LightCallback : public scfImplementation1<LightCallback, 
                                                      iLightCallback>
      {
      public:
        PersistentData* parent;

        LightCallback (PersistentData* parent)
          : scfImplementation1<LightCallback, iLightCallback> (this),
            parent (parent) {}

	void OnColorChange (iLight* light, const csColor& newcolor) { }
	void OnPositionChange (iLight* light, const csVector3& newpos) { }
	void OnSectorChange (iLight* light, iSector* newsector) { }
	void OnRadiusChange (iLight* light, float newradius) { }
	void OnDestroy (iLight* light)
        {
          if (parent != 0)
          {
            parent->lightDataCache.DeleteAll (light);
          }
        }
	void OnAttenuationChange (iLight* light, int newatt) { }
      };
      csRef<LightCallback> lcb;
    };

    ShadowShadowmap (PersistentData& persist,
      const LayerConfigType& layerConfig,
      RenderTree& tree) 
      : persist (persist), layerConfig (layerConfig), renderTree (tree),
        lastShader (0) { }

    template<typename LayerHelper>
    size_t HandleLights (csLightInfluence* influenceLights, size_t numLights,
      size_t layer, LayerHelper& layers,
      typename RenderTree::MeshNode::SingleMesh& mesh,
      typename RenderTree::MeshNode* node)
    {
      persist.texCache.SetG3D (node->owner.renderView->GetGraphics3D()); // @@@ Not here.

      LightingSorter sortedLights (persist.lightSorterPersist, influenceLights,
        numLights);
      LightingVariablesHelper lightVarsHelper (persist.lightVarsPersist);

      /* Get the shader since the number of passes for that layer depend
	* on it */
      iShader* shaderToUse =
	node->owner.shaderArray[layers.GetNewLayerIndex (layer, 0) 
          * node->owner.totalRenderMeshes + mesh.contextLocalId];
      if (!shaderToUse) return 0;

      UpdateMetadata (shaderToUse);
      if (lastMetadata.numberOfLights == 0) return 0;

      const size_t layerLights = sortedLights.GetSize();

      // Set up layers
      size_t firstLight = 0;
      size_t remainingLights = layerLights;
      size_t totalLayers = 0;
      while (firstLight < layerLights)
      {
	csLightType lightType = sortedLights.GetLightType (firstLight);
	size_t num = 1;
	for (; num < remainingLights; num++)
	{
	  if (sortedLights.GetLightType (firstLight + num) != lightType)
	    break;
	}
	/* We have a subset of the lights that are of the same type.
	 * Check the size of it against the shader limit */
	size_t thisPassLayers = (num 
	  + lastMetadata.numberOfLights - 1) / lastMetadata.numberOfLights;
	thisPassLayers = csMin (thisPassLayers, layerConfig.GetMaxLightPasses (layer));
	size_t neededLayers = totalLayers + thisPassLayers;

        layers.Ensure (layer, neededLayers, node);

	firstLight += num;
	remainingLights -= num;
	totalLayers = neededLayers;
      }

      // Now render lights for each light type
      firstLight = 0;
      remainingLights = layerLights;
      totalLayers = 0;
      while (firstLight < layerLights)
      {
	csLightType lightType = sortedLights.GetLightType (firstLight);
	size_t num = 1;
	for (; num < remainingLights; num++)
	{
	  if (sortedLights.GetLightType (firstLight + num) != lightType)
	    break;
	}
	/* We have a subset of the lights that are of the same type.
	  * Check the size of it against the shader limit */
	size_t thisPassLayers = (num 
	  + lastMetadata.numberOfLights - 1) / lastMetadata.numberOfLights;
	thisPassLayers = csMin (thisPassLayers, layerConfig.GetMaxLightPasses (layer));
	size_t neededLayers = totalLayers + thisPassLayers;

	csShaderVariableStack localStack;
	for (size_t n = 0; n < thisPassLayers; n++)
	{
	  if ((totalLayers != 0) || (n > 0))
	  {
	    /* The first layer will have the shader to use set;
	      * subsequent ones don't */
	    node->owner.CopyLayerShader (mesh.contextLocalId,
              layers.GetNewLayerIndex (layer, 0),
              layers.GetNewLayerIndex (layer, n + totalLayers));
	  }
	  node->owner.svArrays.SetupSVStack (localStack, 
            layers.GetNewLayerIndex (layer, n + totalLayers),
	    mesh.contextLocalId);
  
	  size_t thisNum = csMin (num,
	    layerConfig.GetMaxLightNum (layer));
	  csShaderVariable* lightNum = lightVarsHelper.CreateVarOnStack (
	    persist.svNames.GetDefaultSVId (
	      csLightShaderVarCache::varLightCount), localStack);
	  lightNum->SetValue ((int)thisNum);

	  csShaderVariable* passNum = lightVarsHelper.CreateVarOnStack (
	    persist.svPassNum, localStack);
	  passNum->SetValue ((int)(n + totalLayers));
  
	  csShaderVariable* lightTypeSV = lightVarsHelper.CreateVarOnStack (
	    persist.svNames.GetLightSVId (
	      csLightShaderVarCache::lightType), localStack);
	  lightTypeSV->SetValue ((int)(lightType));

	  for (size_t l = thisNum; l-- > 0; )
	  {
	    iLight* light = sortedLights.GetLight (firstLight + l);
	    CachedLightData* thisLightCacheData =
	      persist.lightDataCache.GetElementPointer (light);
	    if (thisLightCacheData == 0)
	    {
              CachedLightData newCacheData;
              newCacheData.shaderVars =
                &(light->GetSVContext()->GetShaderVariables());
	      thisLightCacheData = &persist.lightDataCache.Put (
		light, newCacheData);
	    }
	    lightVarsHelper.MergeAsArrayItems (localStack, 
              *(thisLightCacheData->shaderVars), l);
  
            csShaderVariable* lightTransInvSV = lightVarsHelper.CreateTempSV (
              persist.svNames.GetLightSVId (
	        csLightShaderVarCache::lightTransformWorldInverse));
            lightTransInvSV->SetValue (light->GetMovable()->GetFullTransform ().GetInverse());
            lightVarsHelper.MergeAsArrayItem (localStack, lightTransInvSV, l);

            // Add shadow map texture SV
            if (thisLightCacheData->shadowMapCreateFrame != persist.frameNum)
            {
              iTextureHandle* shadowTex =
                persist.texCache.QueryUnusedTexture (256, 256, 0);
              thisLightCacheData->shadowMapData.Setup (
                renderTree.GetPersistentData().renderViewPool,
                node->owner.renderView, light, shadowTex,
                lightVarsHelper.CreateTempSV (persist.svNames.GetLightSVId (
                  csLightShaderVarCache::lightShadowMap)),
                lightVarsHelper.CreateTempSV (persist.svNames.GetLightSVId (
	          csLightShaderVarCache::lightShadowMapProjection)),
                lightVarsHelper);
              thisLightCacheData->shadowMapCreateFrame = persist.frameNum;

	      // Create a new context for shadow map w/ computed view
	      typename RenderTree::ContextNode* shadowMapCtx = 
		renderTree.CreateContext (
                  thisLightCacheData->shadowMapData.GetNewRenderView());
	      shadowMapCtx->renderTargets[rtaDepth].texHandle = shadowTex;
    
	      // Setup the new context
	      ShadowmapContextSetup contextFunction (layerConfig,
		persist.shaderManager);
	      contextFunction (*shadowMapCtx);
  
            }
	    lightVarsHelper.MergeAsArrayItem (localStack, 
	      thisLightCacheData->shadowMapData.GetMapShaderVar(), l);
	    lightVarsHelper.MergeAsArrayItem (localStack,
	      thisLightCacheData->shadowMapData.GetProjectionShaderVar(), l);

	  }
	  num -= thisNum;
	  firstLight += thisNum;
	  remainingLights -= thisNum;
	}

	totalLayers = neededLayers;
      }
      return firstLight;
    }
  protected:
    PersistentData& persist;
    const LayerConfigType& layerConfig;
    RenderTree& renderTree;

    // Simple cache
    iShader* lastShader;
    csShaderMetadata lastMetadata;
    
    inline void UpdateMetadata (iShader* shaderToUse)
    {
      if (shaderToUse != lastShader)
      {
	lastMetadata = shaderToUse->GetMetadata();
	lastShader = shaderToUse;
      }
    }
  };

}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_SHADOWMAP_H__
