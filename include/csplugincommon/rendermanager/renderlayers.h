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

  /**
   * Render layer implementation for a single render layer.
   */
  class SingleRenderLayer
  {
  public:
    /// Create a single render layer with a no shader type.
    SingleRenderLayer (iShader* defaultShader = 0,
      size_t maxLightPasses = 1, size_t maxLights = ~0)
      : defaultShader (defaultShader), maxLightPasses (maxLightPasses), 
        maxLights (maxLights)
    { }
    /// Create a single render layer with a single shader type.
    SingleRenderLayer (const csStringID shaderType, iShader* defaultShader = 0,
        size_t maxLightPasses = 1, size_t maxLights = ~0)
      : defaultShader (defaultShader), maxLightPasses (maxLightPasses),
        maxLights (maxLights)
    {
      shaderTypes.Push (shaderType);
    }
    /// Create a single render layer with a multiply shader type.
    SingleRenderLayer (const csStringID* shaderTypes, size_t numTypes,
      iShader* defaultShader = 0,
      size_t maxLightPasses = 1, size_t maxLights = ~0)
      : defaultShader (defaultShader), maxLightPasses (maxLightPasses), 
        maxLights (maxLights)
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

  private:
    /*csDirtyAccessArray<StringIDValue,
      csArrayElementHandler<StringIDValue>,
      CS::Memory::AllocatorMalloc,
      csArrayCapacityFixedGrow<1> > shaderTypes;*/
    csDirtyAccessArray<StringIDValue> shaderTypes;
    csRef<iShader> defaultShader;
    size_t maxLightPasses;
    size_t maxLights;
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
      CS_ASSERT(layer == 0);

      return layers[layer].maxLights;
    }

    size_t GetMaxLightPasses (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return layers[layer].maxLightPasses;
    }

  private:
    struct Layer
    {
      csRef<iShader> defaultShader;
      size_t maxLightPasses;
      size_t maxLights;
      size_t firstType;
      size_t numTypes;
    };
    csArray<Layer> layers;
    csDirtyAccessArray<StringIDValue> layerTypes;
  };

  enum
  {
    defaultlayerNoTerrain = 1
    //defaultlayerNoLighting = 2
  };
  
  /* @@@ TODO: Perhaps revisit naming after light support was added,
      see how many seperate layer objects needed for base vs. lighting
      (I[res] could imagine that light iteration needs another layer
      object). */
  void CS_CRYSTALSPACE_EXPORT AddDefaultBaseLayers (iObjectRegistry* objectReg,
    MultipleRenderLayer& layers, uint flags = 0);
  
}
}

#endif
