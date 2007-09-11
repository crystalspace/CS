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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_STANDARDTREETRAITS_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_STANDARDTREETRAITS_H__

#include "iengine/mesh.h"
#include "ivaria/view.h"
#include "ivideo/rendermesh.h"
#include "csutil/comparator.h"
#include "csutil/compileassert.h"
#include "csplugincommon/rendermanager/svarrayholder.h"

namespace CS
{
  //TODO: Move these to general location
  struct TrueType
  {
    static const bool Value = true;
  };

  struct FalseType
  {
    static const bool Value = false;
  };

namespace RenderManager
{
  class RenderTreeStandardTraits
  {
  public:
    //-- Standard types

    // Any extra data that should be defined for each mesh node
    struct MeshNodeExtraDataType
    {
      int   priority;
    };

    // Any extra data that should be defined for each context node
    struct ContextNodeExtraDataType
    {
      // The SVs themselves
      SVArrayHolder svArrays;

      // Arrays of per-mesh shader and ticket info
      csArray<iShader*> shaderArray;
      csArray<size_t> ticketArray;
    };
    
    struct ContextsContainerExtraDataType
    {
      iTextureHandle* renderTarget;
      int subtexture;
      
      ContextsContainerExtraDataType() : renderTarget (0), subtexture (0)
      {}
    };

    // Any extra data per mesh in a mesh node
    struct MeshExtraDataType
    {
      iShaderVariableContext* meshObjSVs;
      csRef<csShaderVariable> svObjectToWorld;
    };

    // The data type to use as node key for mesh nodes
    struct MeshNodeKeyType
    {
      uint8     priority    : 5;
      uint8     isPortal    : 1;
      uint8     meshSorting : 2;
    };

    // Enable/disables

    //-- Standard functions
    // Given a iMeshWrapper and a csRenderMesh, get the correct mesh node index
    static CS_FORCEINLINE 
    MeshNodeKeyType GetMeshNodeKey (iMeshWrapper* imesh, const csRenderMesh& rendermesh)
    {
      MeshNodeKeyType result = {0};

      result.priority = imesh->GetRenderPriority ();
      result.isPortal = rendermesh.portal != 0;
      
      return result;
    }

    // Setup a new mesh node from the first iMeshWrapper and csRenderMesh
    template<typename T>
    static CS_FORCEINLINE_TEMPLATEMETHOD 
    void SetupMeshNode (T& meshNode, iMeshWrapper* imesh, const csRenderMesh& rendermesh)
    {
      meshNode.priority = imesh->GetRenderPriority ();
    }

  private:
  };


 
}
}


CS_COMPILE_ASSERT(sizeof(CS::RenderManager::RenderTreeStandardTraits::MeshNodeKeyType) == sizeof(int8));

template<>
class csComparator<CS::RenderManager::RenderTreeStandardTraits::MeshNodeKeyType>
{
public:
  static int Compare (CS::RenderManager::RenderTreeStandardTraits::MeshNodeKeyType const& mk1, 
    CS::RenderManager::RenderTreeStandardTraits::MeshNodeKeyType const& mk2)
  {
    //BIG HACK
    return (int) (reinterpret_cast<const int8&> (mk1) - reinterpret_cast<const int8&> (mk2));
  }
};



#endif
