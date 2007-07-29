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

namespace CS
{
namespace RenderManager
{

  template<typename Tree>
  class ShaderSetup : public NumberedMeshTraverser<Tree, typename ShaderSetup<Tree> >
  {
  public:
    typedef csArray<iShader*> ShaderArrayType;

    ShaderSetup (ShaderArrayType& shaderArray, csStringID shaderName, 
      iShader* defaultShader = 0)
      : NumberedMeshTraverserType (*this), shaderArray (shaderArray), shaderName (shaderName),
      defaultShader (defaultShader)
    {
    }


    // Need to unhide this one
    using NumberedMeshTraverserType::operator();

    void operator() (const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      // Get the shader
      csRenderMesh* rm = mesh.renderMesh;
      iShader* shader = rm->material->GetMaterial ()->GetShader (shaderName);
      shaderArray[index] = shader ? shader : defaultShader;
    }

  private:
    ShaderArrayType& shaderArray;
    csStringID shaderName;
    iShader* defaultShader;
  };

  template<typename Tree>
  class TicketSetup : public NumberedMeshTraverser<Tree, typename TicketSetup<Tree> >
  {
  public:
    typedef csArray<size_t> TicketArrayType;
    typedef csArray<iShader*> ShaderArrayType;

    TicketSetup (SVArrayHolder& svArrays, csShaderVariableStack& varStack, 
      const ShaderArrayType& shaderArray, TicketArrayType& tickets)
      : NumberedMeshTraverserType (*this), svArrays (svArrays), varStack (varStack),
      shaderArray (shaderArray), ticketArray (tickets)
    {
    }


    // Need to unhide this one
    using NumberedMeshTraverserType::operator();

    void operator() (const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      // Setup the shader array
      svArrays.SetupSVStck (varStack, index);

      // Get the ticket
      iShader* shader = shaderArray[index];
      ticketArray[index] = shader ? shader->GetTicket (*mesh.renderMesh, varStack) : ~0;
    }

  private:
    SVArrayHolder& svArrays;
    csShaderVariableStack& varStack;
    const ShaderArrayType& shaderArray;
    TicketArrayType& ticketArray;
  };

}
}

#endif
