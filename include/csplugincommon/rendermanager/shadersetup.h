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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADERSETUP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADERSETUP_H__

#include "csutil/compositefunctor.h"

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/svsetup.h"

namespace CS
{
namespace RenderManager
{

  typedef csArray<size_t> TicketArrayType;
  typedef csArray<iShader*> ShaderArrayType;

  /**
   * Default shader setup functor.
   * Setup per mesh & layer shader arrays for each node.
   * Assumes that the contextLocalId in each mesh is set.
   */
  template<typename RenderTree, typename LayerConfigType>
  class ShaderSetup
  {
  public:
    ShaderSetup (ShaderArrayType& shaderArray, const LayerConfigType& layerConfig)
      : shaderArray (shaderArray), layerConfig (layerConfig)
    {
    }

    void operator() (typename RenderTree::MeshNode* node)
    {
      // Get the shader
      const size_t totalMeshes = node->owner.totalRenderMeshes;

      for (size_t i = 0; i < node->meshes.GetSize (); ++i)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];
        
        csRenderMesh* rm = mesh.renderMesh;
  
        for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
        {
          size_t layerOffset = layer*totalMeshes;

          iShader* shader = 0;
          if (rm->material)
          {
            size_t layerShaderNum;
            const csStringID* layerShaders = layerConfig.GetShaderTypes (layer, layerShaderNum);

            shader = rm->material->GetMaterial ()->GetFirstShader (layerShaders,
              layerShaderNum);
          }
          shaderArray[mesh.contextLocalId+layerOffset] = shader ? shader : layerConfig.GetDefaultShader (layer);
        }
      }
    }

  private:
    ShaderArrayType& shaderArray;
    const LayerConfigType& layerConfig;
  };

  template<typename RenderTree, typename LayerConfigType>
  struct OperationTraits<ShaderSetup<RenderTree, LayerConfigType> >
  {
    typedef OperationUnorderedParallel Ordering;
  };

  

  /**
   * Default shader ticket setup.
   * Assumes that the contextLocalId in each mesh is set.
   */
  template<typename RenderTree, typename LayerConfigType>
  class TicketSetup
  {
  public:    

    TicketSetup (SVArrayHolder& svArrays, csShaderVariableStack& varStack,       
      const ShaderArrayType& shaderArray, TicketArrayType& tickets, 
      const LayerConfigType& layerConfig)
      : svArrays (svArrays), varStack (varStack), 
      shaderArray (shaderArray), ticketArray (tickets),
      layerConfig (layerConfig)
    {
    }


    void operator() (typename RenderTree::MeshNode* node)
    {
      const size_t totalMeshes = node->owner.totalRenderMeshes;

      for (size_t i = 0; i < node->meshes.GetSize (); ++i)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];

        for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
        {
          size_t layerOffset = layer*totalMeshes;
          
          // Setup the shader array
          svArrays.SetupSVStack (varStack, layer, mesh.contextLocalId);

          // Get the ticket
          iShader* shader = shaderArray[mesh.contextLocalId+layerOffset];
          ticketArray[mesh.contextLocalId+layerOffset] = shader ? shader->GetTicket (*mesh.renderMesh, varStack) : ~0;
        }
      }
    }

  private:
    SVArrayHolder& svArrays;
    csShaderVariableStack& varStack;
    const ShaderArrayType& shaderArray;
    TicketArrayType& ticketArray;
    const LayerConfigType& layerConfig;
  };

  template<typename RenderTree, typename LayerConfigType>
  struct OperationTraits<TicketSetup<RenderTree, LayerConfigType> >
  {
    typedef OperationUnordered Ordering;
  };

  // Some helper for below
  template<typename RenderTree, typename LayerConfigType>
  struct StandardSATSetupTypes
  {
    typedef ShaderSetup<RenderTree, LayerConfigType> ShaderSetupType;
    typedef ShaderSVSetup<RenderTree, LayerConfigType> ShaderSVSetupType;
    typedef TicketSetup<RenderTree, LayerConfigType> TicketSetupType;

    typedef CS::Meta::CompositeFunctorType3<ShaderSetupType, ShaderSVSetupType, 
      TicketSetupType> CombinedFunctorType;
  };

  /**
   * Setup the standard shader, shader SV and ticket
   */
  template<typename ContextNodeType, typename LayerConfigType>
  void SetupStandarShaderAndTicket (ContextNodeType& context, 
    iShaderManager* shaderManager,
    const LayerConfigType& layerConfig)
  {
    context.shaderArray.SetSize (context.totalRenderMeshes*layerConfig.GetLayerCount ());
    context.ticketArray.SetSize (context.totalRenderMeshes*layerConfig.GetLayerCount ());

    // Shader, sv and ticket setup
    typedef typename ContextNodeType::TreeType Tree;
    typedef StandardSATSetupTypes<Tree, LayerConfigType> TypeHelper;
    
    typename TypeHelper::ShaderSetupType 
      shaderSetup (context.shaderArray, layerConfig);

    typename TypeHelper::ShaderSVSetupType 
      shaderSVSetup (context.svArrays, context.shaderArray, layerConfig);
         
    typename TypeHelper::TicketSetupType 
      ticketSetup (context.svArrays, shaderManager->GetShaderVariableStack (),
        context.shaderArray, context.ticketArray, layerConfig);

    // Do the three operations sequentially after each other
    typename TypeHelper::CombinedFunctorType 
      combFunctor (CS::Meta::CompositeFunctor (shaderSetup, shaderSVSetup, ticketSetup));

    ForEachMeshNode (context, combFunctor);
  }

  typedef csDirtyAccessArray<csStringID> ShaderVariableNameArray;
}
}

#endif
