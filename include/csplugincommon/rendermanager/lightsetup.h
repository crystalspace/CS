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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_LIGHTSETUP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_LIGHTSETUP_H__

/**\file
 * Light selection and setup.
 */

#include "iengine/lightmgr.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "ivideo/shader/shader.h"

#include "csgfx/lightsvcache.h"
#include "csgfx/shadervarblockalloc.h"
#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/rendertree.h"

class csShaderVariable;

namespace CS
{
namespace RenderManager
{
  struct LayerHelperContextData
  {
    csArray<size_t> newLayerIndices;
    csArray<size_t> newLayerCounts;
  };

  struct RenderTreeLightingTraits : public RenderTreeStandardTraits
  {
    struct ContextNodeExtraDataType
    {
      LayerHelperContextData layerHelperData;
    };
  };

  /**
   * Helper to duplicate render layers.
   * Used when meshes have to be rendered multiple times but with the
   * properties of a specific render layer. A duplicate of a layer is called
   * a "subLayer".
   */
  template<typename RenderTree, typename LayerConfigType,
    typename NewLayersType>
  class LayerHelper
  {
  public:
    /**
     * Construct. \a layerConfig is the source layer setup, \a newLayers will
     * be manipulated as layers get duplicated. It needs to provide a method
     * 'InsertLayer (size_t after, size_t oldLayer)' which inserts a copy of
     * layer \c oldLayer after the new layer \a after. \sa PostLightingLayers
     */
    LayerHelper (LayerHelperContextData& contextData, 
      const LayerConfigType& layerConfig,
      NewLayersType& newLayers) : contextData (contextData), 
      layerConfig (layerConfig), newLayers (newLayers)
    {
      if (contextData.newLayerIndices.GetSize() == 0)
      {
	contextData.newLayerIndices.SetSize (layerConfig.GetLayerCount ());
	contextData.newLayerCounts.SetSize (layerConfig.GetLayerCount ());
	for (size_t l = 0; l < layerConfig.GetLayerCount (); l++)
	{
	  contextData.newLayerIndices[l] = l;
	  contextData.newLayerCounts[l] = 1;
	}
      }
    }

    /// Get the 'new' index of \a layer, \a sublayer.
    size_t GetNewLayerIndex (size_t layer, size_t subLayer) const
    {
      return contextData.newLayerIndices[layer] + subLayer;
    }

    /// Get the amount of sublayers \a layer posseses.
    size_t GetSubLayerCount (size_t layer) const
    {
      return contextData.newLayerCounts[layer];
    }

    /**
     * Make sure \a layer has at least \a neededSubLayers sublayers.
     * \a node is needed to duplicate stored per-layer data in the assocuated
     * context.
     */
    void Ensure (size_t layer, size_t neededSubLayers,
                 typename RenderTree::ContextNode& context)
    {
      if (neededSubLayers > contextData.newLayerCounts[layer])
      {
	// We need to insert new layers

	// How many?
	size_t insertLayerNum = neededSubLayers - contextData.newLayerCounts[layer];
	// The actual insertion
	for (size_t n = contextData.newLayerCounts[layer]; n < neededSubLayers; n++)
	{
	  context.InsertLayer (contextData.newLayerIndices[layer] + n - 1);
	  newLayers.InsertLayer (contextData.newLayerIndices[layer] + n - 1, layer);
	}
	// Update indices for in new index table
	for (size_t l = layer+1; l < layerConfig.GetLayerCount (); l++)
	  contextData.newLayerIndices[l] += insertLayerNum;
	contextData.newLayerCounts[layer] += insertLayerNum;
      }
    }
  protected:
    LayerHelperContextData& contextData;
    const LayerConfigType& layerConfig;
    NewLayersType& newLayers;
  };
  
  /**
   * Compatibility light settings: if two lights have equal compatibility 
   * light settings they can be rendered in one pass; if not, they require
   * different passes.
   */
  struct LightSettings
  {
    /// Light type
    csLightType type;
    /// Light flags
    csFlags lightFlags;
    
    /// Test compatibility light settings equality
    bool operator== (const LightSettings& other) const
    { return (type == other.type) && (lightFlags == other.lightFlags); }
    /// Test compatibility light settings inequality
    bool operator!= (const LightSettings& other) const
    { return (type != other.type) || (lightFlags != other.lightFlags); }
  };

  /**
   * Lighting sorter. Sorts lights in a way that lights with compatible
   * settings appear after one another.
   */
  class CS_CRYSTALSPACE_EXPORT LightingSorter
  {
    size_t lightLimit;
  public:
    /// Information associated with a light
    struct LightInfo
    {
      /// Pointer to light
      iLight* light;
      /// Whether light is static
      bool isStatic;
      /**
       * Number of required sublights ("virtual" lights when, for technical
       * purposes, a light can not be rendered completely at once).
       */
      uint numSubLights;
      /// Sublight IDs
      uint* subLights;
      /// Compatibility light settings
      LightSettings settings;
    };
    
    /**
     * Data used by the helper that needs to persist over multiple frames.
     * Users of LightingSorter must store an instance of this class and provide
     * it to the helper upon instantiation. 
     */
    struct PersistentData
    {
      csArray<LightInfo> lightTypeScratch;
      csArray<LightInfo> putBackLights;
      csMemoryPool sublightNumMem;
      
      /**
       * Do per-frame house keeping - \b MUST be called every frame/
       * RenderView() execution.
       */
      void UpdateNewFrame()
      {
        lightTypeScratch.DeleteAll();
      }
    };

    /**
     * Construct. \a numLights is a hint about the expected number of lighta 
     * added.
     */
    LightingSorter (PersistentData& persist, size_t numLights);
    
    /**
     * Add a light to the sorter. \a influence specifies information about
     * the light to add, \a numSubLights specifies as how many "virtual" lights
     * a light should be treated, \a lightFlagsMask specifies a mask that is
     * applied to the light flags before comparing them for light compatibility
     * purposes.
     */
    void AddLight (const csLightInfluence& influence,
      uint numSubLights, const csFlags& lightFlagsMask);

    /// Query how many lights are in this sorter.
    size_t GetSize ()
    {
      csArray<LightInfo>& putBackLights = persist.putBackLights;
      return lightLimit + putBackLights.GetSize();
    }

    /// Set the expected number of lights to be added.
    void SetNumLights (size_t numLights);

    /// Set the maximum number of lights to keep in the sorter.
    void SetLightsLimit (size_t limit)
    {
      lightLimit = csMin (persist.lightTypeScratch.GetSize(), limit);
    }

    /// Get the next light, optionally skipping static lights.
    bool GetNextLight (LightInfo& out);
    /**
     * Get the next light if compatible to \a settings, optionally skipping 
     * static lights.
     */
    bool GetNextLight (const LightSettings& settings, LightInfo& out);
    
    /// Put earlier fetched lights back into the list
    void PutInFront (LightInfo* lights, size_t num);
  protected:
    PersistentData& persist;
  };

  /**
   * Helper class to deal with shader variables setup for lighting
   */
  class CS_CRYSTALSPACE_EXPORT LightingVariablesHelper
  {
  public:
    /**
     * Data used by the helper that needs to persist over multiple frames.
     * Users of LightingVariablesHelper must store an instance of this class 
     * and provide it to the helper upon instantiation. 
     */
    struct PersistentData
    {
      csShaderVarBlockAlloc<csBlockAllocatorDisposeLeaky<csShaderVariable> >
	  svAlloc;
      /* A number of SVs have to be kept alive even past the expiration
      * of the actual step */
      csRefArray<csShaderVariable> svKeeper;
      
      PersistentData() : svAlloc (32*1024) {}
      
      /**
       * Do per-frame house keeping - \b MUST be called every frame/
       * RenderView() execution.
       */
      void UpdateNewFrame ()
      {
	svKeeper.Empty();
      }
    };
    
    /// Construct
    LightingVariablesHelper (PersistentData& persist) : persist (persist) {}
    
    /**
     * Merge a shader variable into a stack as an item of a shader variable.
     * The variable in the destination stack with the name of \a sv is an
     * array variable or does not exist gets the array item with the index
     * \a index set to \a sv.
     * \return Whether the destination stack was large enough to contain
     *   \a sv.
     */
    bool MergeAsArrayItem (csShaderVariableStack& dst, 
      csShaderVariable* sv, size_t index);

    /**
     * Merge an array of shader variables into a stack as items of a shader
     * variables.
     * \sa MergeAsArrayItem
     */
    void MergeAsArrayItems (csShaderVariableStack& dst, 
      const csRefArray<csShaderVariable>& allVars, size_t index);

    /// Create a shader variable which is only valid for this frame.
    csShaderVariable* CreateTempSV (CS::ShaderVarStringID name =
      CS::InvalidShaderVarStringID);

    /**
     * Create a temporary shader variable (using CreateTempSV) and put it onto
     * \a stack.
     */
    csShaderVariable* CreateVarOnStack (CS::ShaderVarStringID name,
      csShaderVariableStack& stack);
  protected:
    PersistentData& persist;
  };

  /// Shadow handler for "no" shadows
  template<typename RenderTree, typename LayerConfigType>
  class ShadowNone
  {
  public:
    /// Shadowing-specific data persistently stored for every light
    struct CachedLightData
    {
      /// Number of virtual lights needed for this light
      uint GetSublightNum() const { return 1; }
      /// Once per frame setup for light
      void SetupFrame (RenderTree&, ShadowNone&, iLight*) {}
      /// Clear data needed for one frame after rendering
      void ClearFrameData() {}
    };
    /**
     * Data used by the shadow handler that needs to persist over multiple frames.
     */
    struct PersistentData
    {
      /**
       * Called every frame/RenderView() execution, use for housekeeping.
       */
      void UpdateNewFrame () {}
      /// Called upon plugin initialization
      void Initialize (iObjectRegistry*,
        RenderTreeBase::DebugPersistent&) {}
    };
    /// Shadow method specific parameters
    struct ShadowParameters {};

    ShadowNone() {}
    ShadowNone (PersistentData& persist,
      const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode* node, 
      ShadowParameters&) { }

    /// Set up shadowing for a mesh/light combination
    template<typename _CachedLightData>
    uint HandleOneLight (typename RenderTree::MeshNode::SingleMesh& singleMesh,
                         iLight* light, _CachedLightData& lightData,
                         csShaderVariableStack* lightStacks,
                         uint lightNum, uint sublight)
    { return 1; }
    
    /**
     * Return whether, at the end of light setup, there should be another pass
     * over every light
     */
    static bool NeedFinalHandleLight() { return false; }
    /// Final pass, called for each lights
    void FinalHandleLight (iLight*, CachedLightData&) { }
    
    /**
     * Return which light flags should be masked out before comparing flags
     * of two lights.
     */
    csFlags GetLightFlagsMask () const { return csFlags (CS_LIGHT_NOSHADOWS); }
    
    /// Return up to how many layers shadows for a light may need.
    size_t GetLightLayerSpread() const { return 1; }
  };

  /**
   * For each mesh determine the array of affecting lights and generate shader
   * vars for it.
   *
   * Usage: together with iteration over each mesh node.
   * Must be done after shader and shader SV (usually SetupStandardShader())
   * and before ticket setup.
   * Example:
   * \code
   * RenderManager::LightSetupType::ShadowParamType shadowParam;
   * RenderManager::LightSetupType lightSetup (
   *   rmanager->lightPersistent, rmanager->lightManager,
   *   context.svArrays, layerConfig, shadowParam);
   * ForEachMeshNode (context, lightSetup);
   * \endcode
   *
   * The template parameter \a RenderTree gives the render tree type.
   * The parameter \a LayerConfigType gives a class that is used for providing
   * the rendering layer setup. \a ShadowHandler is an optional class that
   * provides shadow handling for each mesh/light combination (default to
   * no shadows).
   *
   */
  template<typename RenderTree, typename LayerConfigType,
    typename ShadowHandler = ShadowNone<RenderTree, LayerConfigType> >
  class LightSetup
  {
  public:
    class PostLightingLayers;
  
  protected:
    /// Data persistently stored for every light
    struct CachedLightData : public ShadowHandler::CachedLightData 
    {
      const csRefArray<csShaderVariable>* shaderVars;
    };
    
    /** 
     * Set up lighting for a mesh/light combination
     * Given the lights affecting the mesh it sets up the light shader vars
     * in the render layers according to the layers configuration.
     */
    template<typename _ShadowHandler>
    size_t HandleLights (_ShadowHandler& shadows,
      size_t overallPass, LightingSorter& sortedLights,
      size_t layer, LayerHelper<RenderTree, LayerConfigType,
        PostLightingLayers>& layers, const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode::SingleMesh& mesh,
      typename RenderTree::MeshNode* node)
    {
      /* Get the shader since the number of passes for that layer depend
	* on it */
      iShader* shaderToUse =
	node->GetOwner().shaderArray[layers.GetNewLayerIndex (layer, 0) 
          * node->GetOwner().totalRenderMeshes + mesh.contextLocalId];
      if (!shaderToUse) return 0;

      UpdateMetadata (layer, shaderToUse);
      const csShaderMetadata& lastMetadata = metadataCache[layer].metadata;
      if ((lastMetadata.numberOfLights == 0) 
        && !layerConfig.IsAmbientLayer (layer)) return 0;

      LightingVariablesHelper lightVarsHelper (persist.varsHelperPersist);

      size_t layerLights = csMin (sortedLights.GetSize (),
        layerConfig.GetMaxLightNum (layer));
      if (lastMetadata.numberOfLights == 0)
        layerLights = 0;
      if (layerLights == 0)
      {
        if (!layerConfig.IsAmbientLayer (layer)) return 0;
        
        // Render 1 layer only, no lights
        layers.Ensure (layer, 1, node->GetOwner());
        
        csShaderVariableStack localStack;
	node->GetOwner().svArrays.SetupSVStack (localStack, 
	  layers.GetNewLayerIndex (layer, 0),
	  mesh.contextLocalId);

	csShaderVariable* lightNum = lightVarsHelper.CreateVarOnStack (
	  persist.svNames.GetDefaultSVId (
	    csLightShaderVarCache::varLightCount), localStack);
	lightNum->SetValue ((int)0);

	csShaderVariable* passNum = lightVarsHelper.CreateVarOnStack (
	  persist.svPassNum, localStack);
	passNum->SetValue ((int)0);
	
	return 0;
      }
      else
      {
	bool meshIsStaticLit = mesh.meshFlags.Check (CS_ENTITY_STATICLIT);
	  
        // Assume at least 1 light
	CS_ALLOC_STACK_ARRAY(LightingSorter::LightInfo, renderLights, layerLights);
	
	/*
	  Applied limitations:
	  - Sublights are not considered for the 'maximum lights' limit.
	  - Sublights *are* considered for the 'maximum passes' limit.
	  - If not all sublights of a light can be drawn in the limits,
	    skip the light.
	 */
	  
	// First, select iLights, limited by the maximum lights layer limit
        size_t remainingLights = layerLights;
        size_t totalWithSublights = 0;
	{
	  size_t firstLight = 0;
	  //size_t totalLayers = 0;
	  while (firstLight < layerLights)
	  {
	    if (!sortedLights.GetNextLight (renderLights[firstLight]))
	      break;
	    size_t realNum = 1;
	    LightSettings lightSettings;
	    lightSettings = renderLights[firstLight].settings;
	    totalWithSublights += renderLights[firstLight].numSubLights;
	    size_t maxPassLights = lastMetadata.numberOfLights * 
	      layerConfig.GetMaxLightPasses (layer);
	    maxPassLights = csMin (maxPassLights, remainingLights);
	    
	    for (; realNum < maxPassLights; realNum++)
	    {
	      // Note that GetNextLight already does a selection on light type
	      if (!sortedLights.GetNextLight (lightSettings, 
		  renderLights[firstLight + realNum]))
		break;
	     totalWithSublights += renderLights[firstLight + realNum].numSubLights;
	    }
	    
	    firstLight += realNum;
	    remainingLights -= realNum;
	  }
	  remainingLights = firstLight;
	}
	
	// "Expand" iLights into sublights
	CS_ALLOC_STACK_ARRAY(LightingSorter::LightInfo*, renderSublights,
	  totalWithSublights);
	CS_ALLOC_STACK_ARRAY(uint, renderSublightNums, totalWithSublights);
	{
	  size_t i = 0;
	  for (size_t l = 0; l < remainingLights; l++)
	  {
	    LightingSorter::LightInfo& li = renderLights[l];
	    for (uint s = 0; s < li.numSubLights; s++)
	    {
	      renderSublights[i] = &li;
	      renderSublightNums[i] = li.subLights[s];
	      i++;
	    }
	  }
	}
	
	// Below this point "lights" are actually sublights!
	
	// Set up layers.
	size_t firstLight = 0;
	remainingLights = totalWithSublights;
	size_t totalLayers = 0;
	while (firstLight < totalWithSublights)
	{
	  if (totalLayers >= layerConfig.GetMaxLightPasses (layer))
	    break;
	
	  // We can draw up to maxPassLights lights in this layer
	  size_t maxPassLights = lastMetadata.numberOfLights * 
	    layerConfig.GetMaxLightPasses (layer);
	  maxPassLights = csMin (maxPassLights, remainingLights);
	
	  // Find out number of consecutive lights in the layer
	  size_t num = 1;
	  LightSettings lightSettings = renderSublights[firstLight]->settings;
	  for (; num < maxPassLights; num++)
	  {
	    if (renderSublights[firstLight + num]->settings != lightSettings)
	      break;
	  }
	  size_t thisPassLayers;
	  thisPassLayers = (num + lastMetadata.numberOfLights - 1)
	    / lastMetadata.numberOfLights;
	  thisPassLayers = csMin (totalLayers + thisPassLayers,
	    layerConfig.GetMaxLightPasses (layer)) - totalLayers;
	  if (thisPassLayers == 0)
	    // Reached layer pass limit
	    break;
	
	  firstLight += num;
	  remainingLights -= num;
	  totalLayers += thisPassLayers * shadows.GetLightLayerSpread();
	}
	layers.Ensure (layer, totalLayers, node->GetOwner());
	if (remainingLights > 0)
	{
	  renderSublights[firstLight]->subLights += renderSublightNums[firstLight];
	  renderSublights[firstLight]->numSubLights -= renderSublightNums[firstLight];
	  sortedLights.PutInFront (*(renderSublights + firstLight), 1);
	  size_t l = firstLight + renderSublights[firstLight]->numSubLights;
	  while (l < firstLight + remainingLights)
	  {
	    sortedLights.PutInFront (*(renderSublights + l), 1);
	    l += renderSublights[l]->numSubLights;
	  }
	}
	
	csShaderVariableStack* localStacks =
	  new csShaderVariableStack[shadows.GetLightLayerSpread()];
  
	// Now render lights for each light type
	remainingLights = firstLight;
	firstLight = 0;
	totalLayers = 0;
	while (firstLight < layerLights)
	{
	  if (totalLayers >= layerConfig.GetMaxLightPasses (layer))
	    break;
	
	  LightSettings lightSettings = renderSublights[firstLight]->settings;
	  size_t num = 1;
	  for (; num < remainingLights; num++)
	  {
	    if (renderSublights[firstLight+num]->settings != lightSettings)
	      break;
	  }
	  /* We have a subset of the lights that are of the same type.
	   * Check the size of it against the shader limit */
	  size_t thisPassLayers;
	  thisPassLayers = (num + lastMetadata.numberOfLights - 1)
	    / lastMetadata.numberOfLights;
	  thisPassLayers = csMin (totalLayers + thisPassLayers,
	    layerConfig.GetMaxLightPasses (layer)) - totalLayers;
	  if (thisPassLayers == 0)
	    // Reached layer pass limit
	    break;
	  size_t neededLayers = totalLayers
	    + thisPassLayers*shadows.GetLightLayerSpread();
  
	  //csShaderVariableStack localStack;
	  for (size_t n = 0; n < thisPassLayers; n++)
	  {
	    if ((totalLayers != 0) || (n > 0))
	    {
	      /* The first layer will have the shader to use set;
	       * subsequent ones don't */
	      node->GetOwner().CopyLayerShader (mesh.contextLocalId,
		layers.GetNewLayerIndex (layer, 0),
		layers.GetNewLayerIndex (layer, n*shadows.GetLightLayerSpread() + totalLayers));
	    }
	    
	    size_t thisNum = csMin (num,
	      layerConfig.GetMaxLightNum (layer));
	    thisNum = csMin (thisNum, (size_t)lastMetadata.numberOfLights);
	    
	    for (size_t s = 0; s < shadows.GetLightLayerSpread(); s++)
	    {
	      node->GetOwner().svArrays.SetupSVStack (localStacks[s], 
		layers.GetNewLayerIndex (layer,
		  n*shadows.GetLightLayerSpread() + totalLayers) + s,
		mesh.contextLocalId);
    
	      csShaderVariable* lightNum = lightVarsHelper.CreateVarOnStack (
		persist.svNames.GetDefaultSVId (
		  csLightShaderVarCache::varLightCount), localStacks[s]);
	      lightNum->SetValue ((int)thisNum);
    
	      csShaderVariable* passNum = lightVarsHelper.CreateVarOnStack (
		persist.svPassNum, localStacks[s]);
	      passNum->SetValue ((int)(n + totalLayers));
      
	      csShaderVariable* lightTypeSV = lightVarsHelper.CreateVarOnStack (
		persist.svNames.GetLightSVId (
		  csLightShaderVarCache::lightType), localStacks[s]);
	      lightTypeSV->SetValue ((int)(lightSettings.type));
	    }
    
            CachedLightData* thisLightSVs;
	    iLight* light = 0;
	    for (uint l = 0; l < thisNum; l++)
	    {
	      bool isStaticLight = renderSublights[firstLight + l]->isStatic;
	      light = renderSublights[firstLight + l]->light;
	      thisLightSVs = persist.lightDataCache.GetElementPointer (light);
	      
	      uint lSpread = shadows.HandleOneLight (mesh, light, *thisLightSVs, 
	        localStacks, l, renderSublightNums[firstLight + l]);
	      if (lSpread == 0) continue;
    
	      uint initialActualSpread = 0;
	      if (node->sorting == CS_RENDPRI_SORT_BACK2FRONT)
	      {
		/* Hack: to make objects with alpha work w/ shadow_pssm -
		   if they're drawn with the first 'spread' they can draw over
		   a split and make that visible (ugly). Drawing them with the
		   last 'spread' makes sure all splits should be drawn already.
		 */
		initialActualSpread = (uint)shadows.GetLightLayerSpread()
		  - CS::Utility::BitOps::ComputeBitsSet (lSpread);
	      }
              uint actualSpread = initialActualSpread;
	      for (size_t s = 0; s < shadows.GetLightLayerSpread(); s++)
	      {
	        if (!(lSpread & (1 << s))) continue;
	        if (actualSpread > 0)
	        {
		  node->GetOwner().CopyLayerShader (mesh.contextLocalId,
		    layers.GetNewLayerIndex (layer, 0),
		    layers.GetNewLayerIndex (layer,
		      n*shadows.GetLightLayerSpread() + actualSpread + totalLayers));
	        }
	      
		lightVarsHelper.MergeAsArrayItems (localStacks[actualSpread],
		  *(thisLightSVs->shaderVars), l);
		if (isStaticLight && meshIsStaticLit
		    && layerConfig.GetStaticLightsSettings ().specularOnly)
		{
		  lightVarsHelper.MergeAsArrayItem (localStacks[actualSpread],
		    persist.diffuseBlack, l);
		}
		actualSpread++;
	      }
	      if (initialActualSpread != 0)
	      {
		node->GetOwner().shaderArray[layers.GetNewLayerIndex (layer,
		    n*shadows.GetLightLayerSpread() + totalLayers)
		  * node->GetOwner().totalRenderMeshes + mesh.contextLocalId] = 0;
	      }
	    }
	    firstLight += thisNum;
	    num -= thisNum;
            remainingLights -= thisNum;
	  }
  
	  totalLayers = neededLayers;
	}
	
	delete[] localStacks;
	
	return firstLight;
      }
    }
    
    // Simple cache
    struct CachedShaderMetadata
    {
      iShader* shader;
      csShaderMetadata metadata;
      
      CachedShaderMetadata() : shader (0) {}
    };
    csArray<CachedShaderMetadata> metadataCache;
    
    inline void UpdateMetadata (size_t layer, iShader* shaderToUse)
    {
      if (shaderToUse != metadataCache[layer].shader)
      {
	metadataCache[layer].metadata = shaderToUse->GetMetadata();
	metadataCache[layer].shader = shaderToUse;
      }
    }
  public:
    struct PersistentData;
    typedef csArray<iShader*> ShaderArrayType;
    typedef ShadowHandler ShadowHandlerType;
    typedef typename ShadowHandler::ShadowParameters ShadowParamType;

    /**
     * Construct.
     */
    LightSetup (PersistentData& persist, iLightManager* lightmgr,
      SVArrayHolder& svArrays, const LayerConfigType& layerConfig,
      ShadowParamType& shadowParam)
      : persist (persist), lightmgr (lightmgr),
        svArrays (svArrays), allMaxLights (0), newLayers (layerConfig),
        shadowParam (shadowParam)
    {
      // Sum up the number of lights we can possibly handle
      for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
      {
	const size_t layerMax = layerConfig.GetMaxLightNum (layer);
	// Max lights can be ~0, so need to avoid overflow
	allMaxLights += csMin (layerMax, ((size_t)~0) - allMaxLights);
      }
    }

    /** 
     * Set up lighting for a mesh combination
     * Selects the lights affecting the mesh and sets up the light shader vars
     * in the render layers according to the layers configuration.
     */
    void operator() (typename RenderTree::MeshNode* node)
    {
      // The original layers
      const LayerConfigType& layerConfig = newLayers.GetOriginalLayers();
      // Set up metadata cache
      metadataCache.SetSize (layerConfig.GetLayerCount());

      /* This step will insert layers, keep track of the new indices of
      * the original layer as well as how often a layer has been
      * duplicated */
      LayerHelper<RenderTree, LayerConfigType,
        PostLightingLayers> layerHelper (
        static_cast<RenderTreeLightingTraits::ContextNodeExtraDataType&> (
        node->GetOwner()).layerHelperData, layerConfig, newLayers);
      ShadowHandler shadows (persist.shadowPersist, layerConfig,
        node, shadowParam);
      ShadowNone<RenderTree, LayerConfigType> noShadows;

      for (size_t i = 0; i < node->meshes.GetSize (); ++i)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];

        size_t numLights = 0;
        csLightInfluence* influences = 0;
        LightingSorter sortedLights (persist.lightSorterPersist, 0);

        if (!mesh.meshFlags.Check(CS_ENTITY_NOLIGHTING))
        {
	  bool meshIsStaticLit = mesh.meshFlags.Check (CS_ENTITY_STATICLIT);
	  bool skipStatic = meshIsStaticLit
	    && layerConfig.GetStaticLightsSettings ().nodraw;
	    
          uint relevantLightsFlags =
            skipStatic ? ((CS_LIGHTQUERY_GET_ALL & ~CS_LIGHTQUERY_GET_TYPE_ALL)
                            | CS_LIGHTQUERY_GET_TYPE_DYNAMIC)
                       : CS_LIGHTQUERY_GET_ALL;
          
          lightmgr->GetRelevantLightsSorted (node->GetOwner().sector,
            mesh.renderMesh->bbox, influences, numLights, allMaxLights,
            &mesh.renderMesh->object2world,
            relevantLightsFlags);

          sortedLights.SetNumLights (numLights);
          for (size_t l = 0; l < numLights; ++l)
          {
            iLight* light = influences[l].light;
            CachedLightData* thisLightSVs =
              persist.lightDataCache.GetElementPointer (light);
            if (thisLightSVs == 0)
            {
              CachedLightData newCacheData;
              newCacheData.shaderVars =
                &(light->GetSVContext()->GetShaderVariables());
              thisLightSVs = &persist.lightDataCache.Put (
                light, newCacheData);
              light->SetLightCallback (persist.GetLightCallback());
            }
            thisLightSVs->SetupFrame (node->GetOwner().owner, shadows, light);
            csFlags lightFlagsMask;
            if (mesh.meshFlags.Check (CS_ENTITY_NOSHADOWS))
              lightFlagsMask = noShadows.GetLightFlagsMask();
            else
              lightFlagsMask = shadows.GetLightFlagsMask();
            sortedLights.AddLight (influences[l], thisLightSVs->GetSublightNum(),
              ~lightFlagsMask);
          }
        }

        size_t lightOffset = 0;
        size_t overallPass = 0;
        for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
        {
          size_t layerLights = (numLights == 0) ? 0 :
            csMin (layerConfig.GetMaxLightNum (layer), numLights - lightOffset);

          if (layerLights == 0 && !layerConfig.IsAmbientLayer (layer))
          {
            /* Layer has no lights and is no ambient layer - prevent it from
            * being drawn completely */
            node->GetOwner().shaderArray[layerHelper.GetNewLayerIndex (layer, 0) 
              * node->GetOwner().totalRenderMeshes + mesh.contextLocalId] = 0;
            continue;
          }

          sortedLights.SetLightsLimit (layerLights);
          size_t handledLights;
          if (mesh.meshFlags.CheckAll (CS_ENTITY_NOSHADOWCAST
              | CS_ENTITY_NOSHADOWRECEIVE))
            handledLights = HandleLights (noShadows, overallPass, sortedLights,
              layer, layerHelper, layerConfig, mesh, node);
          else
            handledLights = HandleLights (shadows, overallPass, sortedLights,
              layer, layerHelper, layerConfig, mesh, node);
          overallPass++;
          if ((handledLights == 0)
            && (!layerConfig.IsAmbientLayer (layer)))
          {
            /* No lights have been set up, so don't draw either */
            node->GetOwner().shaderArray[layerHelper.GetNewLayerIndex (layer, 0) 
              * node->GetOwner().totalRenderMeshes + mesh.contextLocalId] = 0;
            continue;
          }
          lightOffset += handledLights;
        }

        lightmgr->FreeInfluenceArray (influences);
      }

      if (shadows.NeedFinalHandleLight())
      {
        typename PersistentData::LightDataCache::GlobalIterator lightDataIt (
          persist.lightDataCache.GetIterator());
        while (lightDataIt.HasNext())
        {
          csPtrKey<iLight> light;
          CachedLightData& data = lightDataIt.Next (light);
          shadows.FinalHandleLight (light, data);
          data.ClearFrameData();
        }
      }
    }

    class PostLightingLayers
    {
      const LayerConfigType& layerConfig;
      csArray<size_t> layerMap;

      friend class LightSetup;
      const LayerConfigType& GetOriginalLayers() const
      {
        return layerConfig;
      }
    public:
      PostLightingLayers (const LayerConfigType& layerConfig)
        : layerConfig (layerConfig)
      {
        layerMap.SetCapacity (layerConfig.GetLayerCount());
        for (size_t l = 0; l < layerConfig.GetLayerCount(); l++)
          layerMap.Push (l);
      }

      size_t GetLayerCount () const
      {
	return layerMap.GetSize();
      }
  
      const csStringID* GetShaderTypes (size_t layer, size_t& num) const
      {
        return layerConfig.GetShaderTypes (layerMap[layer], num);
      }
  
      iShader* GetDefaultShader (size_t layer) const
      {
        return layerConfig.GetDefaultShader (layerMap[layer]);
      }
      
      size_t GetMaxLightNum (size_t layer) const
      {
        return layerConfig.GetMaxLightNum (layerMap[layer]);
      }
  
      size_t GetMaxLightPasses (size_t layer) const
      {
        return layerConfig.GetMaxLightPasses (layerMap[layer]);
      }
      bool IsAmbientLayer (size_t layer) const
      {
        return layerConfig.IsAmbientLayer (layerMap[layer]);
      }

      void InsertLayer (size_t after, size_t oldLayer)
      {
        layerMap.Insert (after+1, oldLayer);
      }
    };
    
    /**
     * Return the render layers, adjusted for possible additional layers needed
     * by lighting/shadowing
     */
    const PostLightingLayers& GetPostLightingLayers () const
    {
      return newLayers;
    }

    /**
     * Data used by the light setup helper that needs to persist over multiple frames.
     * Users of LightSetup must store an instance of this class and provide
     * it to the helper upon instantiation. 
     */
    struct PersistentData
    {
      typename ShadowHandler::PersistentData shadowPersist;
      LightingSorter::PersistentData lightSorterPersist;
      csLightShaderVarCache svNames;
      CS::ShaderVarStringID svPassNum;
      csRef<csShaderVariable> diffuseBlack;
      LightingVariablesHelper::PersistentData varsHelperPersist;
      typedef csHash<CachedLightData, csPtrKey<iLight> > LightDataCache;
      LightDataCache lightDataCache;

      ~PersistentData()
      {
        if (lcb.IsValid()) lcb->parent = 0;
      }
      
        /**
         * Initialize helper. Fetches various required values from objects in
         * the object registry. Must be called when the RenderManager plugin 
         * is initialized.
         */
      void Initialize (iObjectRegistry* objReg,
                       RenderTreeBase::DebugPersistent& dbgPersist)
      {
        csRef<iShaderManager> shaderManager =
          csQueryRegistry<iShaderManager> (objReg);
        
	iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	svNames.SetStrings (strings);
        svPassNum = strings->Request ("pass number");
	shadowPersist.Initialize (objReg, dbgPersist);
	
	diffuseBlack.AttachNew (new csShaderVariable (svNames.GetLightSVId (
	  csLightShaderVarCache::lightDiffuse)));
	diffuseBlack->SetValue (csVector4 (0, 0, 0, 0));
      }
      
      /**
       * Do per-frame house keeping - \b MUST be called every frame/
       * RenderView() execution.
       */
      void UpdateNewFrame ()
      {
        shadowPersist.UpdateNewFrame();
        lightSorterPersist.UpdateNewFrame();
        varsHelperPersist.UpdateNewFrame();
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

  private:
    PersistentData& persist;
    iLightManager* lightmgr;
    SVArrayHolder& svArrays; 
    size_t allMaxLights;
    PostLightingLayers newLayers;
    ShadowParamType& shadowParam;
  };

}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_LIGHTSETUP_H__
