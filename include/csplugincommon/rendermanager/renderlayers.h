/*
    Copyright (C) 2007 by Marten Svanfeldt

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

  class SingleRenderLayer
  {
  public:
    /// Create a single render layer with a no shader type.
    SingleRenderLayer (iShader* defaultShader = 0)
      : defaultShader (defaultShader)
    { }
    /// Create a single render layer with a single shader type.
    SingleRenderLayer (const csStringID shaderType, iShader* defaultShader = 0)
      : defaultShader (defaultShader)
    {
      shaderTypes.Push (shaderType);
    }
    /// Create a single render layer with a multiply shader type.
    SingleRenderLayer (const csStringID* shaderTypes, size_t numTypes,
      iShader* defaultShader = 0)
      : defaultShader (defaultShader)
    {
      this->shaderTypes.SetSize (numTypes);
      memcpy (this->shaderTypes.GetArray(), shaderTypes,
	numTypes * sizeof (csStringID));
    }
    
    void AddShaderType (csStringID shaderType)
    {
      shaderTypes.Push (shaderType);
    }

    size_t GetLayerCount () const
    {
      return 1;
    }

    const csStringID* GetShaderTypes (size_t layer, size_t& num) const
    {
      CS_ASSERT(layer == 0);
      
      num = shaderTypes.GetSize ();
      return shaderTypes.GetArray();
    }

    iShader* GetDefaultShader (size_t layer) const
    {
      CS_ASSERT(layer == 0);

      return defaultShader;
    }

  private:
    csDirtyAccessArray<csStringID,
      csArrayElementHandler<csStringID>,
      CS::Memory::AllocatorMalloc,
      csArrayCapacityFixedGrow<1> > shaderTypes;
    iShader* defaultShader;
  };

  class MultipleRenderLayer
  {
  public:
    MultipleRenderLayer () {}
    MultipleRenderLayer (size_t numLayers, const csStringID* shaderTypes, 
      iShader** defaultShader)
    {
      layerTypes.SetSize (numLayers);
      memcpy (layerTypes.GetArray(), shaderTypes, numLayers);
      for (size_t l = 0; l < numLayers; l++)
      {
	Layer newLayer;
	newLayer.defaultShader = defaultShader[l];
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
	newLayer.firstType = layerTypes.GetSize ();
	const csStringID* copyTypes = layers.GetShaderTypes (l,
	  newLayer.numTypes);
	layerTypes.SetSize (newLayer.firstType + newLayer.numTypes);
	memcpy (layerTypes.GetArray() + newLayer.firstType, copyTypes,
	  newLayer.numTypes * sizeof (csStringID));
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
      return layerTypes.GetArray() + layers[layer].firstType;
    }

    iShader* GetDefaultShader (size_t layer) const
    {
      return layers[layer].defaultShader;
    }

  private:
    struct Layer
    {
      iShader* defaultShader;
      size_t firstType;
      size_t numTypes;
    };
    csArray<Layer> layers;
    csDirtyAccessArray<csStringID> layerTypes;
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
  
  /*
  bool AddLayersFromNode (iObjectRegistry* objectReg,
    MultipleRenderLayer& layers, iDocumentNode* node);
  */
}
}

#endif
