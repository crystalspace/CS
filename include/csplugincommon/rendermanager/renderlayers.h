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

/**\file
 * Render layers
 */

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
  
  /// Layer settings for handling static lights
  struct StaticLightsSettings
  {
    /// Don't render static lights.
    bool nodraw;
    /// Render, but only specular (forces diffuse color to black)
    bool specularOnly;
    
    StaticLightsSettings() : nodraw (false), specularOnly (false) {}
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
    /// Create a single render layer with multiple shader types.
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
    
    /// Add a shader type to the list of types this layer handles.
    void AddShaderType (csStringID shaderType)
    {
      shaderTypes.Push (static_cast<StringIDValue> (shaderType));
    }

    /// Get number of render layers (always 1 in this implementation)
    size_t GetLayerCount () const
    {
      return 1;
    }

    /// Get the list of shader types a layer handles.
    const csStringID* GetShaderTypes (size_t layer, size_t& num) const
    {
      CS_ASSERT(layer == 0);
      
      num = shaderTypes.GetSize ();
      return reinterpret_cast<const csStringID*> (shaderTypes.GetArray());
    }

    /// Get the default fallback shader for a layer
    iShader* GetDefaultShader (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return defaultShader;
    }
    
    /**
     * Get maximum number of lights to be rendered on that layer.
     * If a shader supports less than this amount of lights, multiple passes
     * are rendered, until one of the light maximum or the pass maximum
     * is reached.
     */
    size_t GetMaxLightNum (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return maxLights;
    }

    /**
     * Get maximum number of passes to render lights with.
     * \sa GetMaxLightNum
     */
    size_t GetMaxLightPasses (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return maxLightPasses;
    }
    /**
     * Whether a layer is an ambient layer (means that it's rendered even when 
     * no lights are selected/the shader does not support lighting)
     */
    bool IsAmbientLayer (size_t layer) const
    {
      CS_ASSERT(layer == 0);
      return isAmbient;
    }
    /// Get per-layer shader variables
    iShaderVariableContext* GetSVContext (size_t layer) const
    {
      CS_ASSERT(layer == 0);
      return svContext;
    }
    //@{
    /// Get settings for handling static lights
    StaticLightsSettings& GetStaticLightsSettings ()
    {
      return staticLights;
    }
    const StaticLightsSettings& GetStaticLightsSettings () const
    {
      return staticLights;
    }
    //@}

    /// Set the list of shader types this layer handles.
    void SetShaderTypes (const csStringID* shaderTypes, size_t numTypes)
    {
      this->shaderTypes.SetSize (numTypes);
      for (size_t i = 0; i < numTypes; ++i)
        this->shaderTypes[i] = shaderTypes[i];
    }
    /// Set the default fallback shader
    void SetDefaultShader (iShader* defaultShader)
    {
      this->defaultShader = defaultShader;
    }
    /**
     * Set maximum number of passes to render lights with.
     * \sa GetMaxLightNum
     */
    void SetMaxLightPasses (size_t maxLightPasses)
    {
      this->maxLightPasses = maxLightPasses;
    }
    /**
     * Get maximum number of lights to be rendered on that layer.
     * \sa GetMaxLightNum
     */
    void SetMaxLights (size_t maxLights)
    {
      this->maxLights = maxLights;
    }
    /**
     * Set whether this is an ambient layer.
     * \sa IsAmbientLayer
     */
    void SetAmbient (bool isAmbient)
    {
      this->isAmbient = isAmbient;
    }
    /// Set per-layer shader variables
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
    /// Create with no layers
    MultipleRenderLayer () {}
    /**
     * Create with \a numLayers layers, initialized with a shader type from
     * \a shaderTypes, default shader from \a defaultShader, maximum light
     * passes from \a maxLightPasses and maximum lights from \a maxLights.
     */
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
    
    /// Add all layers of the render layers container \a layers to this one.
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

    /// Get number of render layers
    size_t GetLayerCount () const
    {
      return layers.GetSize();
    }

    /// Get the list of shader types a layer handles.
    const csStringID* GetShaderTypes (size_t layer, size_t& num) const
    {
      num = layers[layer].numTypes;
      return reinterpret_cast<const csStringID*> (
        layerTypes.GetArray() + layers[layer].firstType);
    }

    /// Get the default fallback shader for a layer
    iShader* GetDefaultShader (size_t layer) const
    {
      return layers[layer].defaultShader;
    }
    
    /**
     * Get maximum number of lights to be rendered on that layer.
     * If a shader supports less than this amount of lights, multiple passes
     * are rendered, until one of the light maximum or the pass maximum
     * is reached.
     */
    size_t GetMaxLightNum (size_t layer) const
    {
      return layers[layer].maxLights;
    }

    /**
     * Get maximum number of passes to render lights with.
     * \sa GetMaxLightNum
     */
    size_t GetMaxLightPasses (size_t layer) const
    {
      return layers[layer].maxLightPasses;
    }
    /**
     * Whether a layer is an ambient layer (means that it's rendered even when 
     * no lights are selected/the shader does not support lighting)
     */
    bool IsAmbientLayer (size_t layer) const
    {
      return layers[layer].isAmbient;
    }
    /// Get per-layer shader variables
    iShaderVariableContext* GetSVContext (size_t layer) const
    {
      return layers[layer].svContext;
    }
    //@{
    /// Get settings for handling static lights
    StaticLightsSettings& GetStaticLightsSettings ()
    {
      return staticLights;
    }
    const StaticLightsSettings& GetStaticLightsSettings () const
    {
      return staticLights;
    }
    //@}

    /// Remove all layers
    void Clear()
    {
      layers.DeleteAll ();
      layerTypes.DeleteAll ();
    }
  private:
    StaticLightsSettings staticLights;
    struct Layer
    {
      csRef<iShader> defaultShader;
      size_t maxLightPasses;
      size_t maxLights;
      size_t firstType;
      size_t numTypes;
      bool isAmbient;
      csRef<iShaderVariableContext> svContext;
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
   * Read layers setup from an XML file.
   */
  bool CS_CRYSTALSPACE_EXPORT AddLayersFromFile (
    iObjectRegistry* objectReg, const char* fileName,
    MultipleRenderLayer& layers);
}
}

#endif
