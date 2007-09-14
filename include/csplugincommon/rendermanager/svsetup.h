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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SVSETUP_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SVSETUP_H__

#include "csplugincommon/rendermanager/rendertree.h"

class csShaderVariable;

namespace CS
{
namespace RenderManager
{

  template<typename Tree, typename LayerConfigType>
  class StandardSVSetup : public NumberedMeshTraverser<Tree, StandardSVSetup<Tree, LayerConfigType> >
  {
  public:
    typedef NumberedMeshTraverser<Tree, StandardSVSetup<Tree, LayerConfigType> > BaseType;

    StandardSVSetup (SVArrayHolder& svArrays, const LayerConfigType& layerConfig)
      : BaseType (*this), svArrays (svArrays), layerConfig (layerConfig)
    {
    }

    // Need to unhide this one
    using BaseType::operator();

    void operator() (typename Tree::MeshNode* node,
      const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      csRenderMesh* rm = mesh.renderMesh;

      csShaderVariableStack localStack;

      for (size_t layer = 0; layer < layerConfig.GetLayerCount (); ++layer)
      {
        svArrays.SetupSVStck (localStack, layer, index);

        // Push all contexts here @@TODO: get more of them
        localStack[tree.GetObjectToWorldName()] = mesh.svObjectToWorld;
        rm->material->GetMaterial ()->PushVariables (localStack);
        if (rm->variablecontext)
          rm->variablecontext->PushVariables (localStack);
        mesh.meshObjSVs->PushVariables (localStack);
      }
    }

  private:
    SVArrayHolder& svArrays;
    const LayerConfigType& layerConfig;
  };

  template<typename Tree, typename LayerConfigType>
  class ShaderSVSetup : public NumberedMeshTraverser<Tree, ShaderSVSetup<Tree, LayerConfigType> >
  {
  public:
    typedef NumberedMeshTraverser<Tree, ShaderSVSetup<Tree, LayerConfigType> > BaseType;
    typedef csArray<iShader*> ShaderArrayType;

    ShaderSVSetup (SVArrayHolder& svArrays, const ShaderArrayType& shaderArray,
      const LayerConfigType& layerConfig)
      : BaseType (*this), svArrays (svArrays), shaderArray (shaderArray),
      layerConfig (layerConfig)
    {
      tempStack.Setup (svArrays.GetNumSVNames ());
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
  
        tempStack.Clear ();

        iShader* shader = shaderArray[index+layerOffset];
        if (shader) 
        {
          shader->PushVariables (tempStack);
        
          // Back-merge it onto the real one
          csShaderVariableStack localStack;
          svArrays.SetupSVStck (localStack, layer, index);
          localStack.MergeFront (tempStack);
        }
      }
    }

  private:
    SVArrayHolder& svArrays; 
    const ShaderArrayType& shaderArray;
    csShaderVariableStack tempStack;
    const LayerConfigType& layerConfig;
  };

  template<typename Tree, typename LayerConfigType>
  void SetupStandardSVs (LayerConfigType& layerConfig,
    typename Tree::ContextNode& context, 
    iShaderManager* shaderManager, iSector* sector)
  {
    // Setup SV arrays
    context.svArrays.Setup (layerConfig.GetLayerCount(), 
      shaderManager->GetSVNameStringset ()->GetSize (), context.totalRenderMeshes);

    // Push the default stuff
    csShaderVariableStack& svStack = shaderManager->GetShaderVariableStack ();

    {
      context.svArrays.SetupSVStck (svStack, 0, 0);

      shaderManager->PushVariables (svStack);
      sector->GetSVContext ()->PushVariables (svStack);

      // Replicate
      context.svArrays.ReplicateSet (0, 0, 1);
      context.svArrays.ReplicateLayerZero ();
    }
  }
  
}
}

#endif
