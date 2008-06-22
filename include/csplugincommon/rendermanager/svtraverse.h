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

#include "csplugincommon/rendermanager/operations.h"
#include "csutil/bitarray.h"

namespace CS
{
namespace RenderManager
{

  template<typename RenderTree, typename Fn>
  class TraverseUsedSVs
  {
  public:
    TraverseUsedSVs (Fn& fn, size_t maxNumSVs)
      : fn (fn)
    {
      names.SetSize (maxNumSVs);
    }

    void operator() (const typename RenderTree::MeshNode* node)
    {
      typename RenderTree::ContextNode& context = node->owner;
      

      iShader* lastShader = 0;
      size_t lastTicket = ~0;

      for (size_t layer = 0; layer < context.svArrays.GetNumLayers(); ++layer)
      {
        const size_t layerOffset = context.totalRenderMeshes*layer;
        
        for (size_t m = 0; m < node->meshes.GetSize (); ++m)
        {
          const typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes.Get (m);
          iShader* shader = context.shaderArray[mesh.contextLocalId+layerOffset];

          // Skip meshes without shader (for current layer)
          if (!shader)
            continue;
          size_t ticket = context.ticketArray[mesh.contextLocalId+layerOffset];

          if (shader != lastShader || 
              ticket != lastTicket)
          {
            names.Clear();
            shader->GetUsedShaderVars (ticket, names);
            lastShader = shader;
          }

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
      }
    }

  private:
    Fn& fn;
    csBitArray names;
  };

}
}

#endif
