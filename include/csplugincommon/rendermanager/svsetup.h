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

  template<typename Tree>
  class StandardSVSetup : public NumberedMeshTraverser<Tree, StandardSVSetup<Tree> >
  {
  public:
    typedef NumberedMeshTraverser<Tree, StandardSVSetup<Tree> > BaseType;

    StandardSVSetup (SVArrayHolder& svArrays)
      : BaseType (*this), svArrays (svArrays)
    {
    }

    // Need to unhide this one
    using BaseType::operator();

    void operator() (const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      csRenderMesh* rm = mesh.renderMesh;

      csShaderVariableStack localStack;
      svArrays.SetupSVStck (localStack, index);

      // Push all contexts here @@TODO: get more of them
      rm->material->GetMaterial ()->PushVariables (localStack);
      if (rm->variablecontext)
        rm->variablecontext->PushVariables (localStack);
    }

  private:
    SVArrayHolder& svArrays;  
  };

  template<typename Tree>
  class ShaderSVSetup : public NumberedMeshTraverser<Tree, ShaderSVSetup<Tree> >
  {
  public:
    typedef NumberedMeshTraverser<Tree, ShaderSVSetup<Tree> > BaseType;
    typedef csArray<iShader*> ShaderArrayType;

    ShaderSVSetup (SVArrayHolder& svArrays, const ShaderArrayType& shaderArray)
      : BaseType (*this), svArrays (svArrays), shaderArray (shaderArray)
    {
      tempStack.Setup (svArrays.GetNumSVNames ());
    }

    // Need to unhide this one
    using BaseType::operator();

    void operator() (const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      tempStack.Clear ();

      iShader* shader = shaderArray[index];
      shader->PushVariables (tempStack);
      
      // Back-merge it onto the real one
      csShaderVariableStack localStack;
      svArrays.SetupSVStck (localStack, index);
      localStack.MergeFront (tempStack);
    }

  private:
    SVArrayHolder& svArrays; 
    const ShaderArrayType& shaderArray;
    csShaderVariableStack tempStack;
  };

  template<typename Tree>
  void SetupStandardSVs (typename Tree::ContextNode& context, 
    iShaderManager* shaderManager, iSector* sector)
  {
    // Setup SV arrays
    context.svArrays.Setup (shaderManager->GetSVNameStringset ()->GetSize (), 
      context.totalRenderMeshes);

    // Push the default stuff
    csShaderVariableStack& svStack = shaderManager->GetShaderVariableStack ();

    {
      context.svArrays.SetupSVStck (svStack, 0);

      shaderManager->PushVariables (svStack);
      sector->GetSVContext ()->PushVariables (svStack);

      // Replicate
      context.svArrays.ReplicateSet (0, 1);
    }
  }
  
}
}

#endif
