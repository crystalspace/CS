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

#ifndef __CS_DEFERREDTREETRAITS_H__
#define __CS_DEFERREDTREETRAITS_H__

/**\file
 * Standard render tree traits
 */

#include "iengine/mesh.h"
#include "ivaria/view.h"
#include "ivideo/rendermesh.h"
#include "csutil/comparator.h"
#include "csutil/compileassert.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "csplugincommon/rendermanager/svarrayholder.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{
  /**
   * Standard traits for customizing the deferred render tree class.
   *
   * Render tree traits specify additional data stored with
   * meshes, contexts and others in a render tree. 
   * To provide custom traits, create a class and either provide a new, custom
   * type for a trait or typedef in the respective type from
   * RenderTreeStandardTraits.
   */
  class DeferredRenderTreeTraits
  {
  public:
    /**\name Standard types
     * @{ */
    /// Any extra data that should be defined for each mesh node
    struct MeshNodeExtraDataType
    {
      int   priority;
      bool  isTransparent;
    };

    /// Any extra data that should be defined for each context node
    struct ContextNodeExtraDataType
    {
    };
    
    /// Any extra data per mesh in a single mesh 
    struct MeshExtraDataType
    {
    };

    /**
     * The data type to use as node key for mesh nodes.
     * The type must implement operator==() and operator<=().
     */
    struct MeshNodeKeyType
    {
      uint16 priority      : 12;
      uint16 isPortal      : 1;
      uint16 isTransparent : 1;
      uint16 meshSorting   : 2;
      
      bool operator== (const MeshNodeKeyType& other) const
      {
	//BIG HACK
	return (reinterpret_cast<const int16&> (*this) == reinterpret_cast<const int16&> (other));
      }
      bool operator<= (const MeshNodeKeyType& other) const
      {
	//BIG HACK
	return (reinterpret_cast<const int16&> (*this) <= reinterpret_cast<const int16&> (other));
      }
    };
    /** @} */

    // Enable/disables

    /**\name Standard functions
     * @{ */
    /// Given a iMeshWrapper and a csRenderMesh, get the correct mesh node index
    static CS_FORCEINLINE 
    MeshNodeKeyType GetMeshNodeKey (CS::Graphics::RenderPriority defaultPriority, 
				    const csRenderMesh& rendermesh,
                                    bool isTrasparent)
    {
      MeshNodeKeyType result = {0};

      if (rendermesh.renderPrio >= 0)
        result.priority = rendermesh.renderPrio;
      else
        result.priority = defaultPriority;
      result.isPortal = rendermesh.portal != 0;
      result.isTransparent = isTrasparent;

      return result;
    }

    /// Setup a new mesh node from the first iMeshWrapper and csRenderMesh
    template<typename T>
    static CS_FORCEINLINE_TEMPLATEMETHOD 
    void SetupMeshNode (T& meshNode, CS::Graphics::RenderPriority defaultPriority, 
                        const csRenderMesh& rendermesh,
                        bool isTrasparent)
    {
      if (rendermesh.renderPrio >= 0)
        meshNode.priority = rendermesh.renderPrio;
      else
        meshNode.priority = defaultPriority;
      meshNode.isTransparent = isTrasparent;
    }
    /** @} */
  };
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

// Make sure the size matches so that we can use the comparison hack below
CS_COMPILE_ASSERT(sizeof(CS::Plugin::RMDeferred::DeferredRenderTreeTraits::MeshNodeKeyType) == sizeof(int16));

template<>
class csComparator<CS::Plugin::RMDeferred::DeferredRenderTreeTraits::MeshNodeKeyType>
{
public:
  static int Compare (CS::Plugin::RMDeferred::DeferredRenderTreeTraits::MeshNodeKeyType const& mk1, 
    CS::Plugin::RMDeferred::DeferredRenderTreeTraits::MeshNodeKeyType const& mk2)
  {
    //BIG HACK
    return (int) (reinterpret_cast<const int16&> (mk1) - reinterpret_cast<const int16&> (mk2));
  }
};

#endif // __CS_DEFERREDTREETRAITS_H__
