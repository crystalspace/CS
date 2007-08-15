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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDER_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDER_H__

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/svarrayholder.h"


namespace CS
{
namespace RenderManager
{

  class BeginFinishDrawScope
  {
  public:
    BeginFinishDrawScope (iGraphics3D* g3d, int drawFlags, bool finishOnEnd = true)
      : g3d (g3d), finishOnEnd (finishOnEnd)
    {
      g3d->BeginDraw (drawFlags);
    }

    ~BeginFinishDrawScope ()
    {
      if (finishOnEnd)
        g3d->FinishDraw ();
    }

  private:
    iGraphics3D* g3d;
    bool finishOnEnd;
  };



  template<typename Tree>
  class SimpleRender : public NumberedMeshTraverser<Tree, SimpleRender<Tree> >
  {
  public:
    typedef NumberedMeshTraverser<Tree, SimpleRender<Tree> > BaseType;

    SimpleRender (iGraphics3D* g3d, csShaderVariableStack& varStack)
      : BaseType (*this), g3d (g3d), varStack (varStack)
    {
    }

    // Need to unhide this one
    using BaseType::operator();

    void operator() (const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      // Get the shader, ticket and sv array
      iShader* shader = ctxNode.shaderArray[index];
      size_t ticket = ctxNode.ticketArray[index];
      ctxNode.svArrays.SetupSVStck (varStack, index);

      // Render the mesh
      RenderSingleMesh (mesh.renderMesh, shader, ticket,
        mesh.zmode);
    }

  private:

    void RenderSingleMesh (csRenderMesh* mesh, iShader* shader, size_t shaderTicket,
      csZBufMode zmode)
    {
      size_t numPasses = shader->GetNumberOfPasses (shaderTicket);

      for (size_t i = 0; i < numPasses; ++i)
      {
        shader->ActivatePass (shaderTicket, i);

        csRenderMeshModes modes (*mesh);
        shader->SetupPass (shaderTicket, mesh, modes, varStack);
        modes.z_buf_mode = zmode;

        g3d->DrawMesh (mesh, modes, varStack);

        shader->TeardownPass (shaderTicket);
        shader->DeactivatePass (shaderTicket);
      }
    }

    iGraphics3D* g3d;
    csShaderVariableStack& varStack;
  };


}
}

#endif
