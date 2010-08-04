/*
    Copyright (C) 2010 by Joe Forte

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DEFERREDOPERATIONS_H__
#define __DEFERREDOPERATIONS_H__

#include "cssysdef.h"

#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/operations.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

  /**
   * Iterate over all lights within a context, call functor for each one.
   * Does not use any blocking.
   */
  template<typename ContextType, typename Fn>
  void ForEachLight(ContextType &context, Fn &fn)
  {
    iLightList *list = context.sector->GetLights ();

    const int count = list->GetCount ();
    for (int i = 0; i < count; i++)
    {
      fn (list->Get (i));
    }
  }

  /**
   * Iterate over all mesh nodes within context, call functor for each one
   * not marked to use forward rendering. Does not use any blocking.
   */
  template<typename ContextType, typename Fn>
  void ForEachDeferredMeshNode(ContextType &context, Fn &fn)
  {
    typedef CS::RenderManager::Implementation::NoOperationBlock<
      typename ContextType::TreeType::MeshNode*
    > noBlockType;

    typename ContextType::TreeType::MeshNodeTreeIteratorType it = 
      context.meshNodes.GetIterator ();

    noBlockType noBlock;
    // Helper object for calling function
    CS::RenderManager::Implementation::OperationCaller<
      Fn, 
      noBlockType,
      typename CS::RenderManager::OperationTraits<Fn>::Ordering
    > caller (fn, noBlock);

    while (it.HasNext ())
    {
      typename ContextType::TreeType::MeshNode *node = it.Next ();
      CS_ASSERT_MSG("Null node encountered, should not be possible", node);

      if (!node->useForwardRendering)
        caller (node);
    }
  }

  /**
   * Iterate over all mesh nodes within context, call functor for each one 
   * marked to use forward rendering. Does not use any blocking.
   */
  template<typename ContextType, typename Fn>
  void ForEachForwardMeshNode(ContextType &context, Fn &fn)
  {
    typedef CS::RenderManager::Implementation::NoOperationBlock<
      typename ContextType::TreeType::MeshNode*
    > noBlockType;

    typename ContextType::TreeType::MeshNodeTreeIteratorType it = 
      context.meshNodes.GetIterator ();

    noBlockType noBlock;
    // Helper object for calling function
    CS::RenderManager::Implementation::OperationCaller<
      Fn, 
      noBlockType,
      typename CS::RenderManager::OperationTraits<Fn>::Ordering
    > caller (fn, noBlock);

    while (it.HasNext ())
    {
      typename ContextType::TreeType::MeshNode *node = it.Next ();
      CS_ASSERT_MSG("Null node encountered, should not be possible", node);

      if (node->useForwardRendering)
        caller (node);
    }
  }

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERREDOPERATIONS_H__
