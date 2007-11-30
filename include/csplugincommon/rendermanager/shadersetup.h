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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADERSETUP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADERSETUP_H__

#include "csutil/compositefunctor.h"

namespace CS
{
namespace RenderManager
{

  template<typename Tree, typename LayerConfigType>
  class ShaderSetup : public NumberedMeshTraverser<Tree, ShaderSetup<Tree, LayerConfigType> >
  {
  public:
    typedef csArray<iShader*> ShaderArrayType;
    typedef NumberedMeshTraverser<Tree, ShaderSetup<Tree, LayerConfigType> > BaseType;

    ShaderSetup (ShaderArrayType& shaderArray, const LayerConfigType& layerConfig)
      : BaseType (*this), shaderArray (shaderArray), layerConfig (layerConfig)

    {
    }

    // Need to unhide this one
    using BaseType::operator();

    void operator() (typename Tree::MeshNode* node,
      const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      // Get the shader
      csRenderMesh* rm = mesh.renderMesh;

      for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
      {
        size_t layerOffset = layer*ctxNode.totalRenderMeshes;

        iShader* shader = 0;
        if (rm->material)
        {
          size_t layerShaderNum;
          const csStringID* layerShaders = 
            layerConfig.GetShaderTypes (layer, layerShaderNum);
          shader = rm->material->GetMaterial ()->GetFirstShader (layerShaders,
            layerShaderNum);
        }
        shaderArray[index+layerOffset] = shader ? shader : layerConfig.GetDefaultShader (layer);
      }
    }

  private:
    ShaderArrayType& shaderArray;
    const LayerConfigType& layerConfig;
  };

  typedef csArray<size_t> TicketArrayType;
  typedef csArray<iShader*> ShaderArrayType;

  template<typename Tree, typename LayerConfigType>
  class TicketSetup : public NumberedMeshTraverser<Tree, TicketSetup<Tree, LayerConfigType> >
  {
  public:
    typedef NumberedMeshTraverser<Tree, TicketSetup<Tree, LayerConfigType> > BaseType;

    TicketSetup (SVArrayHolder& svArrays, csShaderVariableStack& varStack,       
      const ShaderArrayType& shaderArray, TicketArrayType& tickets, 
      const LayerConfigType& layerConfig)
      : BaseType (*this), svArrays (svArrays), varStack (varStack), 
      shaderArray (shaderArray), ticketArray (tickets),
      layerConfig (layerConfig)
    {
    }


    // Need to unhide this one
    using BaseType::operator();

    void operator() (typename Tree::MeshNode* node,
      const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
      {
        size_t layerOffset = layer*ctxNode.totalRenderMeshes;
        
        // Setup the shader array
        svArrays.SetupSVStack (varStack, layer, index);

        // Get the ticket
        iShader* shader = shaderArray[index+layerOffset];
        ticketArray[index+layerOffset] = shader ? shader->GetTicket (*mesh.renderMesh, varStack) : ~0;
      }
    }

  private:
    SVArrayHolder& svArrays;
    csShaderVariableStack& varStack;
    const ShaderArrayType& shaderArray;
    TicketArrayType& ticketArray;
    const LayerConfigType& layerConfig;
  };


  // Setup the standard shader, shader SV and ticket
  template<typename Tree, typename LayerConfigType>
  static void SetupStandarShaderAndTicket (Tree& tree, typename Tree::ContextNode& context, 
    iShaderManager* shaderManager,
    const LayerConfigType& layerConfig)
  {
    context.shaderArray.SetSize (context.totalRenderMeshes*layerConfig.GetLayerCount ());
    context.ticketArray.SetSize (context.totalRenderMeshes*layerConfig.GetLayerCount ());

    // Shader, sv and ticket setup
    typedef ShaderSetup<Tree, LayerConfigType> ShaderSetupType;
    ShaderSetupType shaderSetup (context.shaderArray, layerConfig);

    typedef ShaderSVSetup<Tree, LayerConfigType> ShaderSVSetupType;
    ShaderSVSetupType shaderSVSetup (context.svArrays, context.shaderArray, layerConfig);
      
    typedef TicketSetup<Tree, LayerConfigType> TicketSetupType;
    TicketSetupType ticketSetup (context.svArrays, shaderManager->GetShaderVariableStack (),
      context.shaderArray, context.ticketArray, layerConfig);

    CS::Meta::CompositeFunctorType3<ShaderSetupType, ShaderSVSetupType, 
      TicketSetupType> functor (CS::Meta::CompositeFunctor (shaderSetup, 
        shaderSVSetup, ticketSetup));
    tree.TraverseMeshNodes (functor, &context);
  }

  typedef csDirtyAccessArray<csStringID> ShaderVariableNameArray;
}
}

#endif
