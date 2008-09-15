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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SVSETUP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SVSETUP_H__

/**\file
 * Standard shader variable setup
 */

#include "ivideo/material.h"

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/operations.h"

class csShaderVariable;

namespace CS
{
namespace RenderManager
{

  /**
   * Standard shader variable stack setup functor.
   * Sets up mesh-specific SVs for the object to world and inverse transform,
   * SVs from the layer, material, rendermesh and mesh wrapper.
   * Assumes that the contextLocalId in each mesh is set.
   *
   * Usage: with iteration over each mesh. Usually after SetupStandardSVs. 
   * Example:
   * \code
   * {
   *   StandardSVSetup<RenderTree, RenderLayers> svSetup (
   *     context.svArrays, layerConfig);
   *   ForEachMeshNode (context, svSetup);
   * }
   * \endcode
   */
  template<typename RenderTree, typename LayerConfigType>
  class StandardSVSetup
  {
  public:

    StandardSVSetup (SVArrayHolder& svArrays, 
      const LayerConfigType& layerConfig) 
      : svArrays (svArrays), layerConfig (layerConfig)
    {
    }  

    /// Operator doing main work
    void operator() (typename RenderTree::MeshNode* node)
    {
      csShaderVariableStack localStack;

      // @@TODO: keep the sv-name around in a better way
      const size_t svO2wName = node->owner.owner.GetPersistentData().svObjectToWorldName;
      const size_t svO2wIName = node->owner.owner.GetPersistentData().svObjectToWorldInvName;

      for (size_t i = 0; i < node->meshes.GetSize (); ++i)
      {
        typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];

        csRenderMesh* rm = mesh.renderMesh;
        for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
        {
          svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);

          // Push all contexts here 
          // @@TODO: get more of them        
          localStack[svO2wName] = mesh.svObjectToWorld;
          localStack[svO2wIName] = mesh.svObjectToWorldInv;
          iShaderVariableContext* layerContext =
            layerConfig.GetSVContext (layer);
          if (layerContext) layerContext->PushVariables (localStack);
          if (rm->material)
            rm->material->GetMaterial ()->PushVariables (localStack);
          if (rm->variablecontext)
            rm->variablecontext->PushVariables (localStack);
          if (mesh.meshObjSVs)
            mesh.meshObjSVs->PushVariables (localStack);
        }
      }
      
    }

  private:
    SVArrayHolder& svArrays;
    const LayerConfigType& layerConfig;
  };  

  template<typename RenderTree, typename LayerConfigType>
  struct OperationTraits<StandardSVSetup<RenderTree, LayerConfigType> >
  {
    typedef OperationUnordered Ordering;
  };

  
  /**
   * Standard shader variable stack setup functor for setting up shader variables
   * from given shader and ticket arrays.
   * Assumes that the contextLocalId in each mesh is set.
   * Usually done through SetupStandardTicket().
   */
  template<typename RenderTree, typename LayerConfigType>
  class ShaderSVSetup
  {
  public:    
    typedef csArray<iShader*> ShaderArrayType;
    typedef csArray<size_t> TicketArrayType;

    ShaderSVSetup (SVArrayHolder& svArrays, const ShaderArrayType& shaderArray,
      const TicketArrayType& tickets, const LayerConfigType& layerConfig)
      : svArrays (svArrays), shaderArray (shaderArray),
      ticketArray (tickets), layerConfig (layerConfig)
    {
      tempStack.Setup (svArrays.GetNumSVNames ());
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
    
          tempStack.Clear ();

          iShader* shader = shaderArray[mesh.contextLocalId+layerOffset];
          if (shader) 
          {
            shader->PushShaderVariables (tempStack,
              ticketArray[mesh.contextLocalId+layerOffset]);
          
            // Back-merge it onto the real one
            csShaderVariableStack localStack;
            svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);
            localStack.MergeFront (tempStack);
          }
        }
      }
    }

  private:
    SVArrayHolder& svArrays; 
    const ShaderArrayType& shaderArray;
    const TicketArrayType& ticketArray;
    csShaderVariableStack tempStack;
    const LayerConfigType& layerConfig;
  };

  template<typename RenderTree, typename LayerConfigType>
  struct OperationTraits<ShaderSVSetup<RenderTree, LayerConfigType> >
  {
    typedef OperationUnordered Ordering;
  };


  /**
   * Setup standard shader variables.
   * Allocates space in the context's SV array holder and prefills that with
   * the shader variables from the shader manager, the context and the sector.
   *
   * Usage: has to be done after mesh numbering. Usually done before mesh
   * SV setup. Example:
   * \code
   * SetupStandardSVs (context, layerConfig, shaderManager, sector);
   * \endcode
   */
  template<typename ContextNode, typename LayerConfigType>
  void SetupStandardSVs (ContextNode& context, LayerConfigType& layerConfig,
    iShaderManager* shaderManager, iSector* sector)
  {
    if (context.totalRenderMeshes == 0) return;

    // Setup SV arrays
    context.svArrays.Setup (
      layerConfig.GetLayerCount(), 
      shaderManager->GetSVNameStringset ()->GetSize (),
      context.totalRenderMeshes);

    // Push the default stuff
    csShaderVariableStack& svStack = shaderManager->GetShaderVariableStack ();

    {
      context.svArrays.SetupSVStack (svStack, 0, 0);

      shaderManager->PushVariables (svStack);
      if (context.shadervars.IsValid())
        context.shadervars->PushVariables (svStack);
      sector->GetSVContext ()->PushVariables (svStack);

      // Replicate
      context.svArrays.ReplicateSet (0, 0, 1);
      context.svArrays.ReplicateLayerZero ();
    }
  }
  
}
}

#endif
