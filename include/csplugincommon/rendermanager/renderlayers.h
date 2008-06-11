/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERLAYERS_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERLAYERS_H__

#include "csutil/dirtyaccessarray.h"
#include "csutil/strset.h"

struct iShader;

namespace CS
{
namespace RenderManager
{
  /*\file
   *
   * Provide render layer implementations.
   * All implementations must at least supply:
   *  - size_t GetLayerCount () const
   *  - const csStringID* GetShaderTypes (size_t layer, size_t& num) const
   *  - iShader* GetDefaultShader (size_t layer) const
   */
   
  struct StaticLightsSettings
  {
    /// Don't render static lights.
    bool nodraw;
    
    StaticLightsSettings() : nodraw (false) {}
  };

  /**
   * Render layer implementation for a single render layer.
   */
  class SingleRenderLayer
  {
  public:
    /// Create a single render layer with a no shader type.
    SingleRenderLayer (iShader* defaultShader = 0,
      size_t maxLightPasses = 3, size_t maxLights = ~0)
      : defaultShader (defaultShader), maxLightPasses (maxLightPasses), 
        maxLights (maxLights), isAmbient (false)
    { }
    /// Create a single render layer with a single shader type.
    SingleRenderLayer (const csStringID shaderType, iShader* defaultShader = 0,
        size_t maxLightPasses = 3, size_t maxLights = ~0)
      : defaultShader (defaultShader), maxLightPasses (maxLightPasses),
        maxLights (maxLights), isAmbient (false)
    {
      shaderTypes.Push (shaderType);
    }
    /// Create a single render layer with a multiply shader type.
    SingleRenderLayer (const csStringID* shaderTypes, size_t numTypes,
      iShader* defaultShader = 0,
      size_t maxLightPasses = 3, size_t maxLights = ~0)
      : defaultShader (defaultShader), maxLightPasses (maxLightPasses), 
        maxLights (maxLights), isAmbient (false)
    {
      this->shaderTypes.SetSize (numTypes);
      for (size_t i = 0; i < numTypes; ++i)
        this->shaderTypes[i] = shaderTypes[i];
    }
    
    void AddShaderType (csStringID shaderType)
    {
      shaderTypes.Push (static_cast<StringIDValue> (shaderType));
    }

    size_t GetLayerCount () const
    {
      return 1;
    }

    const csStringID* GetShaderTypes (size_t layer, size_t& num) const
    {
      CS_ASSERT(layer == 0);
      
      num = shaderTypes.GetSize ();
      return reinterpret_cast<const csStringID*> (shaderTypes.GetArray());
    }

    iShader* GetDefaultShader (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return defaultShader;
    }
    
    size_t GetMaxLightNum (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return maxLights;
    }

    size_t GetMaxLightPasses (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return maxLightPasses;
    }
    bool IsAmbientLayer (size_t layer) const
    {
      CS_ASSERT(layer == 0);
      return isAmbient;
    }
    iShaderVariableContext* GetSVContext (size_t layer) const
    {
      CS_ASSERT(layer == 0);
      return svContext;
    }
    StaticLightsSettings& GetStaticLightsSettings (size_t layer)
    {
      CS_ASSERT(layer == 0);
      return staticLights;
    }
    const StaticLightsSettings& GetStaticLightsSettings (size_t layer) const
    {
      CS_ASSERT(layer == 0);
      return staticLights;
    }

    void SetShaderTypes (const csStringID* shaderTypes, size_t numTypes)
    {
      this->shaderTypes.SetSize (numTypes);
      for (size_t i = 0; i < numTypes; ++i)
        this->shaderTypes[i] = shaderTypes[i];
    }
    void SetDefaultShader (iShader* defaultShader)
    {
      this->defaultShader = defaultShader;
    }
    void SetMaxLightPasses (size_t maxLightPasses)
    {
      this->maxLightPasses = maxLightPasses;
    }
    void SetMaxLights (size_t maxLights)
    {
      this->maxLights = maxLights;
    }
    void SetAmbient (bool isAmbient)
    {
      this->isAmbient = isAmbient;
    }
    void SetSVContext (iShaderVariableContext* svContext)
    {
      this->svContext = svContext;
    }
  private:
    /*csDirtyAccessArray<StringIDValue,
      csArrayElementHandler<StringIDValue>,
      CS::Memory::AllocatorMalloc,
      csArrayCapacityFixedGrow<1> > shaderTypes;*/
    csDirtyAccessArray<StringIDValue> shaderTypes;
    csRef<iShader> defaultShader;
    size_t maxLightPasses;
    size_t maxLights;
    bool isAmbient;
    csRef<iShaderVariableContext> svContext;
    StaticLightsSettings staticLights;
  };

  /**
   * Render layer implementation providing multiple render layers
   */
  class MultipleRenderLayer
  {
  public:
    MultipleRenderLayer () {}
    MultipleRenderLayer (size_t numLayers, const csStringID* shaderTypes, 
      iShader** defaultShader, 
      size_t* maxLightPasses = 0, size_t* maxLights = 0)
    {
      layerTypes.SetSize (numLayers);      
      for (size_t l = 0; l < numLayers; l++)
      {
        layerTypes[l] = shaderTypes[l];

	Layer newLayer;
	newLayer.defaultShader = defaultShader[l];
	newLayer.maxLightPasses = maxLightPasses ? maxLightPasses[l] : 1;
	newLayer.maxLights = maxLights ? maxLights[l] : ~0;
	newLayer.firstType = l;
	newLayer.numTypes = 1;
	newLayer.isAmbient = false;
	layers.Push (newLayer);
      }
      layers.ShrinkBestFit ();
      layerTypes.ShrinkBestFit ();
    }

    ~MultipleRenderLayer ()
    {
    }
    
    template<typename LayerType>
    void AddLayers (const LayerType& layers)
    {
      for (size_t l = 0; l < layers.GetLayerCount(); l++)
      {
	Layer newLayer;
	newLayer.defaultShader = layers.GetDefaultShader (l);
	newLayer.maxLightPasses = layers.GetMaxLightPasses (l);
	newLayer.maxLights = layers.GetMaxLightNum (l);
	newLayer.isAmbient = layers.IsAmbientLayer (l);
	newLayer.svContext = layers.GetSVContext (l);
	newLayer.staticLights = layers.GetStaticLightsSettings (l);
	newLayer.firstType = layerTypes.GetSize ();
	const csStringID* copyTypes = layers.GetShaderTypes (l,
	  newLayer.numTypes);
	layerTypes.SetSize (newLayer.firstType + newLayer.numTypes);

        for (size_t i = 0; i < newLayer.numTypes; ++i)
          layerTypes[i + newLayer.firstType] = copyTypes[i];

	this->layers.Push (newLayer);
      }
      this->layers.ShrinkBestFit ();
      layerTypes.ShrinkBestFit ();
    }

    size_t GetLayerCount () const
    {
      return layers.GetSize();
    }

    const csStringID* GetShaderTypes (size_t layer, size_t& num) const
    {
      num = layers[layer].numTypes;
      return reinterpret_cast<const csStringID*> (
        layerTypes.GetArray() + layers[layer].firstType);
    }

    iShader* GetDefaultShader (size_t layer) const
    {
      return layers[layer].defaultShader;
    }
    
    size_t GetMaxLightNum (size_t layer) const
    {
      return layers[layer].maxLights;
    }

    size_t GetMaxLightPasses (size_t layer) const
    {
      return layers[layer].maxLightPasses;
    }
    bool IsAmbientLayer (size_t layer) const
    {
      return layers[layer].isAmbient;
    }
    iShaderVariableContext* GetSVContext (size_t layer) const
    {
      return layers[layer].svContext;
    }
    StaticLightsSettings& GetStaticLightsSettings (size_t layer)
    {
      return layers[layer].staticLights;
    }
    const StaticLightsSettings& GetStaticLightsSettings (size_t layer) const
    {
      return layers[layer].staticLights;
    }

    /// Remove all layers
    void Clear()
    {
      layers.DeleteAll ();
      layerTypes.DeleteAll ();
    }
  private:
    struct Layer
    {
      csRef<iShader> defaultShader;
      size_t maxLightPasses;
      size_t maxLights;
      size_t firstType;
      size_t numTypes;
      bool isAmbient;
      csRef<iShaderVariableContext> svContext;
      StaticLightsSettings staticLights;
    };
    csArray<Layer> layers;
    csDirtyAccessArray<StringIDValue> layerTypes;
  };

  enum
  {
    defaultlayerNoTerrain = 1,
    defaultlayerNoLighting = 2
  };
  
  /* @@@ TODO: Perhaps revisit naming after light support was added,
      see how many seperate layer objects needed for base vs. lighting
      (I[res] could imagine that light iteration needs another layer
      object). */
  void CS_CRYSTALSPACE_EXPORT AddDefaultBaseLayers (iObjectRegistry* objectReg,
    MultipleRenderLayer& layers, uint flags = 0, iShader* defaultShader = 0);
  
  /**
   * Read layers setup from a document node.
   */
  bool CS_CRYSTALSPACE_EXPORT AddLayersFromDocument (
    iObjectRegistry* objectReg, iDocumentNode* node,
    MultipleRenderLayer& layers);
  
  /**
   * Read layers setup from a file.
   */
  bool CS_CRYSTALSPACE_EXPORT AddLayersFromFile (
    iObjectRegistry* objectReg, const char* fileName,
    MultipleRenderLayer& layers);
}
}

#endif
