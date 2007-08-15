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
  class ShaderSetup : public NumberedMeshTraverser<Tree, ShaderSetup<Tree> >
  {
  public:
    typedef csArray<iShader*> ShaderArrayType;
    typedef NumberedMeshTraverser<Tree, ShaderSetup<Tree> > BaseType;

    ShaderSetup (ShaderArrayType& shaderArray, csStringID shaderName, 
      iShader* defaultShader = 0)
      : BaseType (*this), shaderArray (shaderArray), shaderName (shaderName),
      defaultShader (defaultShader)
    {
    }


    // Need to unhide this one
    using BaseType::operator();

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

  typedef csArray<size_t> TicketArrayType;
  typedef csArray<iShader*> ShaderArrayType;

  template<typename Tree>
  class TicketSetup : public NumberedMeshTraverser<Tree, TicketSetup<Tree> >
  {
  public:
    typedef NumberedMeshTraverser<Tree, TicketSetup<Tree> > BaseType;

    TicketSetup (SVArrayHolder& svArrays, csShaderVariableStack& varStack, 
      const ShaderArrayType& shaderArray, TicketArrayType& tickets)
      : BaseType (*this), svArrays (svArrays), varStack (varStack),
      shaderArray (shaderArray), ticketArray (tickets)
    {
    }


    // Need to unhide this one
    using BaseType::operator();

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


  // Setup the standard shader, shader SV and ticket
  template<typename Tree>
  static void SetupStandarShaderAndTicket (Tree& tree, typename Tree::ContextNode& context, 
    iShaderManager* shaderManager,
    csStringID defaultShaderType, iShader* defaultShader = 0)
  {
    context.shaderArray.SetSize (context.totalRenderMeshes);
    context.ticketArray.SetSize (context.totalRenderMeshes);

    // Shader, sv and ticket setup
    {
      ShaderSetup<Tree> shaderSetup (context.shaderArray, defaultShaderType, defaultShader);
      ShaderSVSetup<Tree> shaderSVSetup (context.svArrays, context.shaderArray);
      TicketSetup<Tree> ticketSetup (context.svArrays, shaderManager->GetShaderVariableStack (),
        context.shaderArray, context.ticketArray);

      tree.TraverseMeshNodes (
        CS::Meta::CompositeFunctor(
          CS::Meta::CompositeFunctor(shaderSetup, shaderSVSetup), 
          ticketSetup), &context);
    }
  }

  typedef csDirtyAccessArray<csStringID> ShaderVariableNameArray;

  static void GetUsedShaderVars (const ShaderArrayType& shaderArray,
    const TicketArrayType& ticketArray, 
    ShaderVariableNameArray& names)
  {
    size_t nameArrayOfs = 0;
    for (size_t i = 0; i < shaderArray.GetSize(); i++)
    {
      size_t returnedNames = 0;
      iShader* shader = shaderArray[i];
      if (shader == 0) continue;
      shader->GetUsedShaderVars (ticketArray[i],
        names.GetArray() + nameArrayOfs, 
        names.GetSize() - nameArrayOfs, returnedNames);
      nameArrayOfs += returnedNames;
    }
    names.SetSize (nameArrayOfs);
  }

  template<typename Tree>
  static void GetAllUsedShaderVars (const typename Tree::ContextNode& context, 
    iShaderManager* shaderManager, ShaderVariableNameArray& allNames)
  {
    ShaderVariableNameArray names;
    size_t numSVs = shaderManager->GetSVNameStringset()->GetSize();
    names.SetSize (numSVs * context.shaderArray.GetSize());
    GetUsedShaderVars (context.shaderArray, context.ticketArray,
      names);
    allNames.Empty ();
    for (size_t i = 0; i < names.GetSize(); i++)
    {
      if (allNames.FindSortedKey (
        csArrayCmp<csStringID, csStringID> (names[i])) == csArrayItemNotFound)
        allNames.InsertSorted (names[i]);
    }
  }

}
}

#endif
