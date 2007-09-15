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

    SimpleRender (iGraphics3D* g3d, csShaderVariableStack& varStack, size_t meshLayer)
      : BaseType (*this), g3d (g3d), varStack (varStack), meshLayer (meshLayer)
    {
    }

    void operator() (const typename Tree::TreeTraitsType::MeshNodeKeyType& key,
      typename Tree::MeshNode* node, typename Tree::ContextNode& ctxNode, Tree& tree)
    {
      lastShader = 0;
      lastTicket = (size_t)~0;
      firstMeshIndex = 0;
      meshesToRender.Empty ();
      meshesToRender.SetCapacity (ctxNode.totalRenderMeshes);

      BaseType::operator() (key, node, ctxNode, tree);

      RenderMeshes (lastShader, lastTicket, ctxNode);
    }

    void operator() (typename Tree::MeshNode* node,
      const typename Tree::MeshNode::SingleMesh& mesh, size_t index,
      typename Tree::ContextNode& ctxNode, const Tree& tree)
    {
      size_t layerOffset = ctxNode.totalRenderMeshes*meshLayer;

      // Get the shader, ticket and sv array
      iShader* shader = ctxNode.shaderArray[index+layerOffset];
      if (!shader) return;
      size_t ticket = ctxNode.ticketArray[index+layerOffset];

      // Render meshes if shader or ticket differ
      if ((shader != lastShader) || (ticket != lastTicket))
      {
        RenderMeshes (lastShader, lastTicket, ctxNode);
        lastShader = shader;
        lastTicket = ticket;
        firstMeshIndex = index;
      }
      MeshToRender& mtr = meshesToRender.GetExtend (meshesToRender.GetSize ());
      mtr.mesh = mesh.renderMesh;
      mtr.zmode = mesh.zmode;
    }

  private:

    void RenderMeshes (iShader* shader, size_t shaderTicket, 
      typename Tree::ContextNode& ctxNode)
    {
      if (meshesToRender.GetSize() == 0) return;

      if (shader != 0)
      {
	size_t numPasses = shader->GetNumberOfPasses (shaderTicket);
  
	for (size_t i = 0; i < numPasses; ++i)
	{
	  shader->ActivatePass (shaderTicket, i);
  
	  for (size_t m = 0; m < meshesToRender.GetSize(); m++)
	  {
	    ctxNode.svArrays.SetupSVStack (varStack, meshLayer, firstMeshIndex + m);
  
	    csRenderMesh* mesh = meshesToRender[m].mesh;
	    csRenderMeshModes modes (*mesh);
	    shader->SetupPass (shaderTicket, mesh, modes, varStack);
	    modes.z_buf_mode = meshesToRender[m].zmode;
  
	    g3d->DrawMesh (mesh, modes, varStack);
  
	    shader->TeardownPass (shaderTicket);
	  }
	  shader->DeactivatePass (shaderTicket);
	}
      }
      meshesToRender.Empty ();
    }


    iGraphics3D* g3d;
    csShaderVariableStack& varStack;

    size_t meshLayer;

    iShader* lastShader;
    size_t lastTicket;
    size_t firstMeshIndex;
    struct MeshToRender
    {
      csRenderMesh* mesh;
      csZBufMode zmode;
    };
    csArray<MeshToRender> meshesToRender;
  };


}
}

#endif
