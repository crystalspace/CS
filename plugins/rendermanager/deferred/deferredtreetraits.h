/*
    Copyright (C) 2007 by Marten Svanfeldt
    Copyright (C) 2010 by Joe Forte

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

#include "iengine/mesh.h"
#include "ivaria/view.h"
#include "ivideo/rendermesh.h"
#include "csutil/comparator.h"
#include "csutil/compileassert.h"
#include "csutil/stringarray.h"
#include "csutil/cfgacc.h"
#include "csplugincommon/rendermanager/lightsetup.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "csplugincommon/rendermanager/svarrayholder.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{
  /**
   * Render tree traits used by the deferred render manager.
   */
  class RenderTreeDeferredTraits
  {
  public:
    /**\name Standard types
     * @{ */
    /// Any extra data that should be defined for each mesh node
    struct MeshNodeExtraDataType
    {
      CS::Graphics::RenderPriority priority;
      bool useForwardRendering;
    };

    /// Any extra data that should be defined for each context node
    struct ContextNodeExtraDataType 
      : public CS::RenderManager::RenderTreeLightingTraits::ContextNodeExtraDataType
    {
    };
    
    /// Any extra data per mesh in a single mesh 
    struct MeshExtraDataType
    {
    };

    /// Any extra data that needs to persist between frames
    struct PersistentDataExtraDataType
    {
      csBitArray forwardPriorities;
    };

    /**
     * The data type to use as node key for mesh nodes.
     * The type must implement operator==() and operator<=().
     */
    struct MeshNodeKeyType
    {
      uint16 priority            : 12;
      uint16 isPortal            : 1;
      uint16 useForwardRendering : 1;
      uint16 meshSorting         : 2;
      
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

    /// Initializes the extra persistent data.
    static void Initialize(PersistentDataExtraDataType &data, iObjectRegistry *registry)
    {
      const char *messageID = "crystalspace.rendermanager.deferred.treetraits";

      csRef<iEngine> engine = csQueryRegistry<iEngine> (registry);
      csConfigAccess cfg (registry);
      
      const char *str = cfg->GetStr ("RenderManager.Deferred.ForwardPriorities", "alpha,transp,portal");
      csStringArray strArray;
      strArray.SplitString (str, ",", csStringArray::delimIgnore);

      for (size_t i = 0; i < strArray.GetSize (); i++)
      {
        CS::Graphics::RenderPriority p = engine->GetRenderPriority (strArray[i]);
        if (p <= 0)
        {
          csReport (registry, CS_REPORTER_SEVERITY_WARNING,
            messageID, "Unknown render priority '%s' specified.", strArray[i]);
        }
        else
        {
          if (data.forwardPriorities.GetSize() <= p)
            data.forwardPriorities.SetSize (p + 1);
          data.forwardPriorities.SetBit (p);
        }
      }
    }

    /// Returns true if a mesh in the given priority is considered transparent.
    static bool UseForwardRendering(CS::Graphics::RenderPriority priority,
                                    const PersistentDataExtraDataType &data)
    {
      if (priority < data.forwardPriorities.GetSize ())
        return data.forwardPriorities.IsBitSet (priority);
      return false;
    }

    /**\name Standard functions
     * @{ */
    /// Given a iMeshWrapper and a csRenderMesh, get the correct mesh node index
    static CS_FORCEINLINE 
    MeshNodeKeyType GetMeshNodeKey (CS::Graphics::RenderPriority defaultPriority, 
                                    const csRenderMesh& rendermesh,
                                    const PersistentDataExtraDataType& data)
    {
      MeshNodeKeyType result = {0};

      if (rendermesh.renderPrio.IsValid())
        result.priority = rendermesh.renderPrio;
      else
        result.priority = defaultPriority;
      result.isPortal = rendermesh.portal != 0;
      result.useForwardRendering = UseForwardRendering (
	CS::Graphics::RenderPriority (uint (result.priority)), data);

      return result;
    }

    /// Setup a new mesh node from the first iMeshWrapper and csRenderMesh
    template<typename T>
    static CS_FORCEINLINE_TEMPLATEMETHOD 
    void SetupMeshNode (T& meshNode, CS::Graphics::RenderPriority defaultPriority, 
                        const csRenderMesh& rendermesh,
                        const PersistentDataExtraDataType& data)
    {
      if (rendermesh.renderPrio.IsValid())
        meshNode.priority = rendermesh.renderPrio;
      else
        meshNode.priority = defaultPriority;
      meshNode.useForwardRendering = UseForwardRendering (
	CS::Graphics::RenderPriority (meshNode.priority), data);
    }
    /** @} */
  };
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

// Make sure the size matches so that we can use the comparison hack below
CS_COMPILE_ASSERT(sizeof(CS::Plugin::RMDeferred::RenderTreeDeferredTraits::MeshNodeKeyType) == sizeof(int16));

template<>
class csComparator<CS::Plugin::RMDeferred::RenderTreeDeferredTraits::MeshNodeKeyType>
{
public:
  static int Compare (CS::Plugin::RMDeferred::RenderTreeDeferredTraits::MeshNodeKeyType const& mk1, 
    CS::Plugin::RMDeferred::RenderTreeDeferredTraits::MeshNodeKeyType const& mk2)
  {
    //BIG HACK
    return (int) (reinterpret_cast<const int16&> (mk1) - reinterpret_cast<const int16&> (mk2));
  }
};

#endif // __CS_DEFERREDTREETRAITS_H__
