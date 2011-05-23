/*
    Copyright (C) 2010 by Joe Forte

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DEFERREDSHADERSETUP_H__
#define __DEFERREDSHADERSETUP_H__

#include "cssysdef.h"

#include "csplugincommon/rendermanager/shadersetup.h"

#include "deferredoperations.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{
  /**
   * Sets up shaders for objects that can choose to use deferred or forward shading. 
   */
  template<typename RenderTree, typename LayerConfigType>
  class DeferredShaderSetup
  {
  public:
    DeferredShaderSetup(CS::RenderManager::ShaderArrayType &shaderArray, 
                        const LayerConfigType &layerConfig,
                        int deferredLayer,
                        int zonlyLayer)
      : 
    shaderArray(shaderArray), 
    layerConfig(layerConfig), 
    deferredLayer(deferredLayer),
    zonlyLayer(zonlyLayer)
    {}

    void operator() (typename RenderTree::MeshNode *node)
    {
      if (node->useForwardRendering)
        ForwardSetup (node);
      else
        DeferredSetup (node);
    }

    /**
     * Sets up a mesh node to not use the deferred layer.
     */
    void ForwardSetup(typename RenderTree::MeshNode *node)
    {
      // Get the shader
      const size_t totalMeshes = node->GetOwner().totalRenderMeshes;

      const size_t meshCount = node->meshes.GetSize ();
      for (size_t i = 0; i < meshCount; i++)
      {
        typename RenderTree::MeshNode::SingleMesh &mesh = node->meshes[i];
        csRenderMesh *rm = mesh.renderMesh;

        // Setup the deferred layer.
        size_t layerOffset = deferredLayer * totalMeshes;
        shaderArray[mesh.contextLocalId + layerOffset] = nullptr;

        // Setup the forward rendering layers.
        const size_t layerCount = layerConfig.GetLayerCount ();
        for (size_t layer = 0; layer < layerCount; layer++)
        {
          if ((int)layer == deferredLayer)
            continue;

          iShader *shader = nullptr;
          if (rm->material)
          {
            size_t layerShaderNum;
            const csStringID *layerShaders = layerConfig.GetShaderTypes (layer, layerShaderNum);

            shader = rm->material->GetMaterial ()->GetFirstShader (layerShaders, layerShaderNum);
          }
          
          size_t layerOffset = layer * totalMeshes;
          if (shader)
            shaderArray[mesh.contextLocalId + layerOffset] = shader;
          else
            shaderArray[mesh.contextLocalId + layerOffset] = layerConfig.GetDefaultShader (layer);
        }
      }
    }

    /**
     * Sets up a mesh node to only use the deferred layer.
     */
    void DeferredSetup(typename RenderTree::MeshNode *node)
    {
      // Get the shader
      const size_t totalMeshes = node->GetOwner().totalRenderMeshes;

      const size_t meshCount = node->meshes.GetSize ();
      for (size_t i = 0; i < meshCount; i++)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];
        csRenderMesh *rm = mesh.renderMesh;

        // Setup the deferred and zonly layers.
        size_t layers[2] = { deferredLayer, zonlyLayer };
        const size_t count = sizeof(layers) / sizeof(size_t);
        for (size_t k = 0; k < count; k++)
        {
          size_t layer = layers[k];

          iShader *shader = nullptr;
          if (rm->material)
          {
            size_t layerShaderNum;
            const csStringID* layerShaders = layerConfig.GetShaderTypes (layer, layerShaderNum);

            shader = rm->material->GetMaterial ()->GetFirstShader (layerShaders, layerShaderNum);
          }

          size_t layerOffset = layer * totalMeshes;
          if (shader)
            shaderArray[mesh.contextLocalId + layerOffset] = shader;
          else
            shaderArray[mesh.contextLocalId + layerOffset] = layerConfig.GetDefaultShader (layer);
        }

        // Setup the forward rendering layers.
        const size_t layerCount = layerConfig.GetLayerCount ();
        for (size_t layer = 0; layer < layerCount; layer++)
        {
          if ((int)layer == deferredLayer || (int)layer == zonlyLayer)
            continue;

          size_t layerOffset = layer * totalMeshes;
          shaderArray[mesh.contextLocalId + layerOffset] = nullptr;
        }
      }
    }

  private:
    CS::RenderManager::ShaderArrayType &shaderArray;
    const LayerConfigType &layerConfig;
    int deferredLayer;
    int zonlyLayer;
  };

  /**
   * Iterates through the mesh nodes executing the DeferredShaderSetup functor.
   */
  template<typename ContextNodeType, typename LayerConfigType>
  void DeferredSetupShader(ContextNodeType &context, 
                           iShaderManager *shaderManager,
                           const LayerConfigType &layerConfig,
                           int deferredLayer,
                           int zonlyLayer)
  {
    context.shaderArray.SetSize (context.totalRenderMeshes * layerConfig.GetLayerCount ());

    // Shader setup
    typedef typename ContextNodeType::TreeType Tree;

    DeferredShaderSetup<Tree, LayerConfigType>
      shaderSetup (context.shaderArray, layerConfig, deferredLayer, zonlyLayer);

    ForEachMeshNode (context, shaderSetup);
  }
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERREDSHADERSETUP_H__
