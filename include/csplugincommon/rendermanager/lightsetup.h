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
  template<typename RenderTree, typename LayerConfigType,
    typename NewLayersType>
  class LayerHelper
  {
  public:
    struct PersistentData
    {
      csArray<size_t> newLayerIndices;
      csArray<size_t> newLayerCounts;
      
      void UpdateNewFrame()
      {
        newLayerIndices.DeleteAll();
        newLayerCounts.DeleteAll();
      }
    };

    LayerHelper (PersistentData& persist, 
      const LayerConfigType& layerConfig,
      NewLayersType& newLayers) : persist (persist), 
      layerConfig (layerConfig), newLayers (newLayers)
    {
      persist.newLayerIndices.SetSize (layerConfig.GetLayerCount ());
      persist.newLayerCounts.SetSize (layerConfig.GetLayerCount ());
      for (size_t l = 0; l < layerConfig.GetLayerCount (); l++)
      {
        persist.newLayerIndices[l] = l;
        persist.newLayerCounts[l] = 1;
      }
    }

    size_t GetNewLayerIndex (size_t layer, size_t subLayer) const
    {
      return persist.newLayerIndices[layer] + subLayer;
    }

    size_t GetSubLayerCount (size_t layer) const
    {
      return persist.newLayerCounts[layer];
    }

    void Ensure (size_t layer, size_t neededSubLayers,
                 typename RenderTree::MeshNode* node)
    {
      if (neededSubLayers > persist.newLayerCounts[layer])
      {
	// We need to insert new layers

	// How many?
	size_t insertLayerNum = neededSubLayers - persist.newLayerCounts[layer];
	// The actual insertion
	for (size_t n = persist.newLayerCounts[layer]; n < neededSubLayers; n++)
	{
	  node->owner.InsertLayer (persist.newLayerIndices[layer] + n - 1);
	  newLayers.InsertLayer (persist.newLayerIndices[layer] + n - 1, layer);
	}
	// Update indices for in new index table
	for (size_t l = layer+1; l < layerConfig.GetLayerCount (); l++)
	  persist.newLayerIndices[l] += insertLayerNum;
	persist.newLayerCounts[layer] += insertLayerNum;
      }
    }
  protected:
    PersistentData& persist;
    const LayerConfigType& layerConfig;
    NewLayersType& newLayers;
  };
  
  struct LightSettings
  {
    csLightType type;
    csFlags lightFlags;
    
    bool operator== (const LightSettings& other) const
    { return (type == other.type) && (lightFlags == other.lightFlags); }
    bool operator!= (const LightSettings& other) const
    { return (type != other.type) || (lightFlags != other.lightFlags); }
  };

  class CS_CRYSTALSPACE_EXPORT LightingSorter
  {
    size_t lightLimit;
  public:
    struct LightInfo
    {
      iLight* light;
      bool isStatic;
      uint numSubLights;
      LightSettings settings;
    };
    
    struct PersistentData
    {
      csArray<LightInfo> lightTypeScratch;
      csArray<LightInfo> putBackLights;
      
      void UpdateNewFrame()
      {
        lightTypeScratch.DeleteAll();
      }
    };

    LightingSorter (PersistentData& persist, size_t numLights);
    
    void AddLight (const csLightInfluence& influence,
      uint numSubLights, const csFlags& lightFlagsMask);

    size_t GetSize (bool skipStatic)
    {
      csArray<LightInfo>& putBackLights = persist.putBackLights;
    
      size_t n = 0;
      while (n < lightLimit + putBackLights.GetSize())
      {
        if (n < putBackLights.GetSize())
        {
          size_t j = putBackLights.GetSize()-1-n;
	  if (!skipStatic || !putBackLights[j].isStatic)
	    n++;
	  else
	    putBackLights.DeleteIndex (j);
        }
        else
        {
          size_t i = n - putBackLights.GetSize();
	  if (!skipStatic || !persist.lightTypeScratch[i].isStatic)
	    n++;
	  else
	  {
	    persist.lightTypeScratch.DeleteIndex (i);
	    lightLimit = csMin (persist.lightTypeScratch.GetSize(), lightLimit);
	  }
	}
      }
      return n;
    }
    void SetLightsLimit (size_t limit)
    {
      lightLimit = csMin (persist.lightTypeScratch.GetSize(), limit);
    }

    bool GetNextLight (bool skipStatic, LightInfo& out);
    bool GetNextLight (const LightSettings& settings, bool skipStatic,
      LightInfo& out);
      
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
    struct PersistentData
    {
      csShaderVarBlockAlloc<csBlockAllocatorDisposeLeaky<csShaderVariable> >
	  svAlloc;
      /* A number of SVs have to be kept alive even past the expiration
      * of the actual step */
      csRefArray<csShaderVariable> svKeeper;
      
      void UpdateNewFrame ()
      {
	svKeeper.Empty();
      }
    };
    
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

  template<typename RenderTree, typename LayerConfigType>
  class ShadowNone
  {
  public:
    struct CachedLightData
    {
      uint GetSublightNum() const { return 1; }
      void SetupFrame (RenderTree&, ShadowNone&, iLight*) {}
      void ClearFrameData() {}
    };
    struct PersistentData
    {
      void UpdateNewFrame () {}
      void Initialize (iObjectRegistry*) {}
    };
    struct ShadowParameters {};

    ShadowNone() {}
    ShadowNone (PersistentData& persist,
      const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode* node, 
      ShadowParameters&) { }

    template<typename _CachedLightData>
    uint HandleOneLight (typename RenderTree::MeshNode::SingleMesh& singleMesh,
                         iLight* light, _CachedLightData& lightData,
                         csShaderVariableStack* lightStacks,
                         uint lightNum, uint sublight)
    { return 0; }
    
    static bool NeedFinalHandleLight() { return false; }
    void FinalHandleLight (iLight*, CachedLightData&) { }
    
    /**
     * Return which light flags should be masked out before comparing flags
     * of two lights.
     */
    csFlags GetLightFlagsMask () const { return csFlags (CS_LIGHT_NOSHADOWS); }
    
    size_t GetLightLayerSpread() const { return 1; }
  };

  /**
   * For each mesh determine the array of affecting lights and generate shader
   * vars for it.
   * Must be done after shader and shader SV (usually SetupStandardShader())
   * and before ticket setup.
   */
  template<typename RenderTree, typename LayerConfigType,
    typename ShadowHandler = ShadowNone<RenderTree, LayerConfigType> >
  class LightSetup
  {
  public:
    class PostLightingLayers;
  
  protected:
    struct CachedLightData : public ShadowHandler::CachedLightData 
    {
      const csRefArray<csShaderVariable>* shaderVars;
    };
    
    template<typename _ShadowHandler>
    size_t HandleLights (_ShadowHandler& shadows,
      LightingSorter& sortedLights,
      size_t layer, LayerHelper<RenderTree, LayerConfigType,
        PostLightingLayers>& layers, const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode::SingleMesh& mesh,
      typename RenderTree::MeshNode* node)
    {
      LightingVariablesHelper lightVarsHelper (persist.varsHelperPersist);

      /* Get the shader since the number of passes for that layer depend
	* on it */
      iShader* shaderToUse =
	node->owner.shaderArray[layers.GetNewLayerIndex (layer, 0) 
          * node->owner.totalRenderMeshes + mesh.contextLocalId];
      if (!shaderToUse) return 0;

      UpdateMetadata (shaderToUse);
      if ((lastMetadata.numberOfLights == 0) 
        && !layerConfig.IsAmbientLayer (layer)) return 0;

      bool skipStatic = mesh.meshFlags.Check (CS_ENTITY_STATICLIT)
	&& layerConfig.GetStaticLightsSettings (layer).nodraw;

      size_t layerLights = csMin (sortedLights.GetSize (skipStatic),
        layerConfig.GetMaxLightNum (layer));
      if (lastMetadata.numberOfLights == 0)
        layerLights = 0;
      if (layerLights == 0)
      {
        if (!layerConfig.IsAmbientLayer (layer)) return 0;
        
        // Render 1 layer only, no lights
        layers.Ensure (layer, 1, node);
        
        csShaderVariableStack localStack;
	node->owner.svArrays.SetupSVStack (localStack, 
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
        // Assume at least 1 light
	CS_ALLOC_STACK_ARRAY(LightingSorter::LightInfo, renderLights, layerLights);
	
	/*
	  Applied limitations:
	  - Sublights are not considered for the 'maximum lights' limit.
	  - Sublights *are* considered for the 'maximum passes' limit.
	  - If not all sublights of a light can't be drawn in the limits,
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
	    sortedLights.GetNextLight (skipStatic, renderLights[firstLight]);
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
	      if (!sortedLights.GetNextLight (lightSettings, skipStatic, 
		  renderLights[firstLight + realNum]))
		break;
	     totalWithSublights += renderLights[firstLight + realNum].numSubLights;
	    }
	    
	    firstLight += realNum;
	    remainingLights -= realNum;
	  }
	  sortedLights.PutInFront (renderLights + firstLight, remainingLights);
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
	      renderSublightNums[i] = s;
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
	layers.Ensure (layer, totalLayers, node);
	
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
	      node->owner.CopyLayerShader (mesh.contextLocalId,
		layers.GetNewLayerIndex (layer, 0),
		layers.GetNewLayerIndex (layer, n*shadows.GetLightLayerSpread() + totalLayers));
	    }
	    
	    size_t thisNum = csMin (num,
	      layerConfig.GetMaxLightNum (layer));
	    thisNum = csMin (thisNum, (size_t)lastMetadata.numberOfLights);
	    
	    for (size_t s = 0; s < shadows.GetLightLayerSpread(); s++)
	    {
	      node->owner.svArrays.SetupSVStack (localStacks[s], 
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
	    for (size_t l = 0; l < thisNum; l++)
	    {
	      light = renderSublights[firstLight + l]->light;
	      thisLightSVs = persist.lightDataCache.GetElementPointer (light);
	      
	      uint lSpread = shadows.HandleOneLight (mesh, light, *thisLightSVs, 
	        localStacks, l, renderSublightNums[firstLight + l]);
    
	      for (size_t s = 0; s < shadows.GetLightLayerSpread(); s++)
	      {
	        if ((s > 0) && (lSpread & (1 << s)))
	        {
		  node->owner.CopyLayerShader (mesh.contextLocalId,
		    layers.GetNewLayerIndex (layer, 0),
		    layers.GetNewLayerIndex (layer, n*shadows.GetLightLayerSpread() + s + totalLayers));
	        }
	      
		lightVarsHelper.MergeAsArrayItems (localStacks[s],
		  *(thisLightSVs->shaderVars), l);
	      }
	    }
	    firstLight += thisNum;
	    remainingLights -= thisNum;
	  }
  
	  totalLayers = neededLayers;
	}
	
	delete[] localStacks;
	
	return firstLight;
      }
    }
    
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
  public:
    struct PersistentData;
    typedef csArray<iShader*> ShaderArrayType;
    typedef typename ShadowHandler::ShadowParameters ShadowParamType;

    LightSetup (PersistentData& persist, iLightManager* lightmgr,
      SVArrayHolder& svArrays, const LayerConfigType& layerConfig,
      ShadowParamType& shadowParam)
      : persist (persist), lightmgr (lightmgr), svArrays (svArrays),
        allMaxLights (0), newLayers (layerConfig), shadowParam (shadowParam)
    {
      // Sum up the number of lights we can possibly handle
      for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
      {
	const size_t layerMax = layerConfig.GetMaxLightNum (layer);
	// Max lights can be ~0, so need to avoid overflow
	allMaxLights += csMin (layerMax, ((size_t)~0) - allMaxLights);
      }
    }

    void operator() (typename RenderTree::MeshNode* node)
    {
      // The original layers
      const LayerConfigType& layerConfig = newLayers.GetOriginalLayers();

      /* This step will insert layers, keep track of the new indices of
       * the original layer as well as how often a layer has been
       * duplicated */
      LayerHelper<RenderTree, LayerConfigType,
        PostLightingLayers> layerHelper (persist.layerPersist, layerConfig,
        newLayers);
      ShadowHandler shadows (persist.shadowPersist, layerConfig,
        node, shadowParam);
      ShadowNone<RenderTree, LayerConfigType> noShadows;

      for (size_t i = 0; i < node->meshes.GetSize (); ++i)
      {
      
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];

        size_t numLights;
        csLightInfluence* influences;
	lightmgr->GetRelevantLights (node->owner.sector,
	  mesh.bbox, influences, numLights, allMaxLights);
	
	LightingSorter sortedLights (persist.lightSorterPersist,  numLights);
	for (size_t l = 0; l < numLights; l++)
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
          thisLightSVs->SetupFrame (node->owner.owner, shadows, light);
          csFlags lightFlagsMask;
          if (mesh.meshFlags.Check (CS_ENTITY_NOSHADOWS))
            lightFlagsMask = noShadows.GetLightFlagsMask();
          else
            lightFlagsMask = shadows.GetLightFlagsMask();
	  sortedLights.AddLight (influences[l], thisLightSVs->GetSublightNum(),
	    ~lightFlagsMask);
	}
        
        size_t lightOffset = 0;
	for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
        {
          // Get the subset of lights for this layer
          size_t layerLights;
          if (mesh.meshFlags.Check (CS_ENTITY_NOLIGHTING))
            layerLights = 0;
          else
            layerLights = csMin (layerConfig.GetMaxLightNum (layer),
              numLights - lightOffset);
          if ((layerLights == 0)
            && (!layerConfig.IsAmbientLayer (layer)))
          {
            /* Layer has no lights and is no ambient layer - prevent it from
             * being drawn completely */
            node->owner.shaderArray[layerHelper.GetNewLayerIndex (layer, 0) 
              * node->owner.totalRenderMeshes + mesh.contextLocalId] = 0;
            continue;
          }

          sortedLights.SetLightsLimit (layerLights);
          size_t handledLights;
          if (mesh.meshFlags.CheckAll (CS_ENTITY_NOSHADOWCAST
              | CS_ENTITY_NOSHADOWRECEIVE))
	    handledLights = HandleLights (noShadows, sortedLights,
	      layer, layerHelper, layerConfig, mesh, node);
	  else
	    handledLights = HandleLights (shadows, sortedLights,
	      layer, layerHelper, layerConfig, mesh, node);
          if ((handledLights == 0)
            && (!layerConfig.IsAmbientLayer (layer)))
          {
            /* No lights have been set up, so don't draw either */
            node->owner.shaderArray[layerHelper.GetNewLayerIndex (layer, 0) 
              * node->owner.totalRenderMeshes + mesh.contextLocalId] = 0;
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
    
    const PostLightingLayers& GetPostLightingLayers () const
    {
      return newLayers;
    }

    struct PersistentData
    {
      typename ShadowHandler::PersistentData shadowPersist;
      typename LayerHelper<RenderTree, LayerConfigType,
        PostLightingLayers>::PersistentData layerPersist;
      LightingSorter::PersistentData lightSorterPersist;
      csLightShaderVarCache svNames;
      CS::ShaderVarStringID svPassNum;
      LightingVariablesHelper::PersistentData varsHelperPersist;
      typedef csHash<CachedLightData, csPtrKey<iLight> > LightDataCache;
      LightDataCache lightDataCache;

      ~PersistentData()
      {
        if (lcb.IsValid()) lcb->parent = 0;
      }
      
      void Initialize (iObjectRegistry* objReg)
      {
        csRef<iShaderManager> shaderManager =
          csQueryRegistry<iShaderManager> (objReg);
        
	iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	svNames.SetStrings (strings);
        svPassNum = strings->Request ("pass number");
	shadowPersist.Initialize (objReg);
      }
      void UpdateNewFrame ()
      {
        shadowPersist.UpdateNewFrame();
        layerPersist.UpdateNewFrame();
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
