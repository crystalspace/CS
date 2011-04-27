/*
    Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SVTRAVERSE_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SVTRAVERSE_H__

/**\file
 * Traversal of shader variables used in a render tree
 */

#include "csplugincommon/rendermanager/operations.h"
#include "csutil/bitarray.h"

namespace CS
{
namespace RenderManager
{
  /**
   * Traverser for all meshes in a tree, returning the set of used
   * shader variables for each mesh.
   *
   * Usage: together with ForEachMeshNode(). A functor must be provided
   * in \a Fn which implements void operator() (RenderTree::MeshNode* node,
   * size_t layer, RenderTree::MeshNode::SingleMesh& mesh, csBitArray names).
   * The \a names bit array will have a bit set for each shader variable name
   * used by the given mesh.
   */
  template<typename RenderTree, typename Fn>
  class TraverseUsedSVSets
  {
  public:
    TraverseUsedSVSets (Fn& fn, size_t maxNumSVs, uint svUsers = iShader::svuAll)
      : fn (fn), svUsers (svUsers)
    {
      names.SetSize (maxNumSVs);
    }

    void operator() (typename RenderTree::MeshNode* node)
    {
      typename RenderTree::ContextNode& context = node->GetOwner();
      

      iShader* lastShader = 0;
      size_t lastTicket = ~0;

      for (size_t layer = 0; layer < context.svArrays.GetNumLayers(); ++layer)
      {
        const size_t layerOffset = context.totalRenderMeshes*layer;
        
        for (size_t m = 0; m < node->meshes.GetSize (); ++m)
        {
          typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes.Get (m);
          iShader* shader = context.shaderArray[mesh.contextLocalId+layerOffset];

          // Skip meshes without shader (for current layer)
          if (!shader)
            continue;
          size_t ticket = context.ticketArray[mesh.contextLocalId+layerOffset];

          if (shader != lastShader || 
              ticket != lastTicket)
          {
            names.Clear();
            shader->GetUsedShaderVars (ticket, names, svUsers);
            lastShader = shader;
          }
          
          fn (node, layer, mesh, names);
        }
      }
    }

  private:
    Fn& fn;
    csBitArray names;
    uint svUsers;
  };

  /**
   * Traverser for all meshes in a tree, calling the callback for each SV
   * used by each mesh.
   *
   * Usage: together with ForEachMeshNode(). A functor must be provided
   * in \a Fn which implements void operator() (CS::ShaderVarStringID varName,
   * csShaderVariable* sv). The functor will be called for each shader
   * variable used in the traversed context.
   */
  template<typename RenderTree, typename Fn>
  class TraverseUsedSVs
  {
  public:
    TraverseUsedSVs (Fn& fn, size_t maxNumSVs, uint svUsers = iShader::svuAll)
      : proxy (fn), traverseSets (proxy, maxNumSVs, svUsers)
    {
    }

    void operator() (typename RenderTree::MeshNode* node)
    {
      traverseSets.operator() (node);
    }

  private:
    struct TraverseSVsProxy
    {
      Fn& fn;
      
      TraverseSVsProxy (Fn& fn) : fn (fn) { }
  
      void operator() (const typename RenderTree::MeshNode* node,
                       size_t layer,
                       const typename RenderTree::MeshNode::SingleMesh& mesh,
                       const csBitArray& names)
      {
	// No names to check anyway, so leave right away ...
	if (names.AllBitsFalse()) return;
	
	typename RenderTree::ContextNode& context = node->GetOwner();

	csShaderVariableStack varStack;
	context.svArrays.SetupSVStack (varStack, layer, mesh.contextLocalId);
  
	size_t lastName = csMin (names.GetSize(), varStack.GetSize());
  
	csBitArray::SetBitIterator it = names.GetSetBitIterator ();
	while (it.HasNext ())
	{
	  size_t name = it.Next ();
	  if (name >= lastName)
	    break;
  
	  CS::ShaderVarStringID varName ((CS::StringIDValue)name);
  
	  csShaderVariable* sv = varStack[name];
	  if (sv != 0) 
	    fn (varName, sv);
	}
      }
    };
    TraverseSVsProxy proxy;
    TraverseUsedSVSets<RenderTree, TraverseSVsProxy> traverseSets;
  };

}
}

#endif
