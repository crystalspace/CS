/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_OPERATIONS_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_OPERATIONS_H__

/**\file
 * Generic operations to combine actions with iterations over sets of some
 * data.
 */

#include "csplugincommon/rendermanager/rendertree.h"
#include "csutil/set.h"
#include "csutil/compositefunctor.h"

namespace CS
{
namespace RenderManager
{
  //-- Tags for information about operations
  /// No forced order of operation. Single execution.
  struct OperationUnordered {};
  /// Give each operation a number passed along to functor. Single execution.
  struct OperationNumbered {};
  /// No forced order of operation. Possible parallel execution.
  struct OperationUnorderedParallel {};
  /// Give each operation a number passed along to functor. Possible parallel execution.
  struct OperationNumberedParallel {};
  

  namespace Implementation
  {
    /**
     * Object blocker for iterator methods below.
     * Blocks nothing.
     */
    template<typename IterationObject>
    struct NoOperationBlock
    {
      void Reset () const
      {}

      bool operator() (IterationObject) const
      {
        return false;
      }
    };

    /**
     * Object blocker for iterator methods below.
     * Blocks already seen objects by storing them in a set. 
     */
    template<typename IterationObject>
    struct OnceOperationBlock
    {     
      void Reset ()
      {
        objectSet.Empty ();
      }

      bool operator() (IterationObject obj)
      {
        if (objectSet.In (obj))
          return true;

        objectSet.AddNoTest (obj);
        return false;
      }

      csSet<IterationObject> objectSet;
    };

    /**
     * Object blocker for iterator methods below.
     * Blocks already seen objects by storing them in a set. 
     */
    template<typename IterationObject>
    struct OnceOperationBlockRef
    {     
      OnceOperationBlockRef (csSet<IterationObject>& set)
        : objectSet (set)
      {}

      void Reset ()
      {
        objectSet.Empty ();
      }

      bool operator() (IterationObject obj)
      {
        if (objectSet.In (obj))
          return true;

        objectSet.AddNoTest (obj);
        return false;
      }

      csSet<IterationObject>& objectSet;
    };

    /**
     * Helper for dispatching the actual function call in ForEach methods below.
     * Split into two-level hierarchy to avoid partial specialization when that is needed     
     */
#if defined (CS_COMPILER_MSVC) && _MSC_VER < 1310
    template<typename Fn, typename OperationBlock>
    struct OperationCallerWorkAround
    {

      /**
       * Empty, not doing anything with unrecognized tag...
       */
      template<typename Type>
      struct OperationCaller {};

      /**
       * Executor for unordered calls
       */
      template<>
      struct OperationCaller<OperationUnordered>
      {
        OperationCaller (Fn& fn, OperationBlock& block)
          : function (fn), block (block)
        {}

        template<typename ObjectType>
        void operator() (const ObjectType& context)
        {
          if (!block (context))
            function (context);
        }

        Fn& function;
        OperationBlock block;
      };

      /**
       * Executor for numbered calls
       */
      template<>
      struct OperationCaller<OperationNumbered>
      {
        OperationCaller (Fn& fn, OperationBlock& block)
          : function (fn), block (block), index (0)
        {}

        template<typename ObjectType>
        void operator() (const ObjectType& context)
        {          
          if (!block (context))
            function (index, context);
          index++;
        }

        Fn& function;
        OperationBlock block;
        size_t index;
      };



      /**
       * Executor for unordered calls
       */
      template<>
      struct OperationCaller<OperationUnorderedParallel>
      {
        OperationCaller (Fn& fn, OperationBlock& block)
          : function (fn), block (block)
        {}

        template<typename ObjectType>
        void operator() (const ObjectType& context)
        {
          if (!block (context))
            function (context);
        }

        Fn& function;
        OperationBlock block;
      };

      /**
       * Executor for numbered calls
       */
      template<>
      struct OperationCaller<OperationNumberedParallel>
      {
        OperationCaller (Fn& fn, OperationBlock& block)
          : function (fn), block (block), index (0)
        {}

        template<typename ObjectType>
        void operator() (const ObjectType& context)
        {          
          if (!block (context))
            function (index, context);
          index++;
        }

        Fn& function;
        OperationBlock block;
        size_t index;
      };


    };

    template<typename Fn, typename OperationBlock, typename Type>
    struct OperationCaller : 
      public OperationCallerWorkAround<Fn, OperationBlock>::OperationCaller<Type>
    {
      OperationCaller (Fn& fn, OperationBlock& block)
        : OperationCallerWorkAround<Fn, OperationBlock>::OperationCaller<Type> (fn, block)
      {}
    };
#else
    /**
     * Empty, not doing anything with unrecognized tag...
     */
    template<typename Fn, typename OperationBlock, typename Type>
    struct OperationCaller
    {};

    /**
     * Executor for unordered calls
     */
    template<typename Fn, typename OperationBlock>
    struct OperationCaller<Fn, OperationBlock, OperationUnordered>
    {
      OperationCaller (Fn& fn, OperationBlock& block)
        : function (fn), block (block)
      {}

      template<typename ObjectType>
      void operator() (const ObjectType& context)
      {
        if (!block (context))
          function (context);
      }

      Fn& function;
      OperationBlock block;
    };

    /**
     * Executor for numbered calls
     */
    template<typename Fn, typename OperationBlock>
    struct OperationCaller<Fn, OperationBlock, OperationNumbered>
    {
      OperationCaller (Fn& fn, OperationBlock& block)
        : function (fn), block (block), index (0)
      {}

      template<typename ObjectType>
      void operator() (const ObjectType& context)
      {          
        if (!block (context))
          function (index, context);
        index++;
      }

      Fn& function;
      OperationBlock block;
      size_t index;
    };

    /**
     * Executor for unordered calls
     */
    template<typename Fn, typename OperationBlock>
    struct OperationCaller<Fn, OperationBlock, OperationUnorderedParallel>
    {
      OperationCaller (Fn& fn, OperationBlock& block)
        : function (fn), block (block)
      {}

      template<typename ObjectType>
      void operator() (const ObjectType& context)
      {
        if (!block (context))
          function (context);
      }

      Fn& function;
      OperationBlock block;
    };

    /**
     * Executor for numbered calls
     */
    template<typename Fn, typename OperationBlock>
    struct OperationCaller<Fn, OperationBlock, OperationNumberedParallel>
    {
      OperationCaller (Fn& fn, OperationBlock& block)
        : function (fn), block (block), index (0)
      {}

      template<typename ObjectType>
      void operator() (const ObjectType& context)
      {          
        if (!block (context))
          function (index, context);
        index++;
      }

      Fn& function;
      OperationBlock block;
      size_t index;
    };

#endif
    
    // Helper lookups.. oh god save me from this code...
    template<typename Traits1, typename Traits2>
    struct OperationTraitsCombiner
    {
      // Invalid combination
    };

    template<>
    struct OperationTraitsCombiner<OperationUnordered, OperationUnordered>
    {
      typedef OperationUnordered Result;
    };

    template<>
    struct OperationTraitsCombiner<OperationNumbered, OperationNumbered>
    {
      typedef OperationNumbered Result;
    };

    template<>
    struct OperationTraitsCombiner<OperationUnorderedParallel, OperationUnorderedParallel>
    {
      typedef OperationUnorderedParallel Result;
    };

    template<>
    struct OperationTraitsCombiner<OperationNumberedParallel, OperationNumberedParallel>
    {
      typedef OperationNumberedParallel Result;
    };

    template<>
    struct OperationTraitsCombiner<OperationUnorderedParallel, OperationUnordered>
    {
      typedef OperationUnordered Result;
    };
    template<>
    struct OperationTraitsCombiner<OperationUnordered, OperationUnorderedParallel>
    {
      typedef OperationUnordered Result;
    };

    template<>
    struct OperationTraitsCombiner<OperationNumberedParallel, OperationNumbered>
    {
      typedef OperationNumbered Result;
    };
    template<>
    struct OperationTraitsCombiner<OperationNumbered, OperationNumberedParallel>
    {
      typedef OperationNumbered Result;
    };    
  }
  
  /**
   * Unspecialized traits class for operations defining their ordering
   * and parallellization.
   */
  template<typename T>
  struct OperationTraits
  {
    typedef OperationUnordered Ordering;
  };

  //@{
  /**
   * Specialized traits class for composite functor
   */
  template<typename Fn1, typename Fn2>
  struct OperationTraits<CS::Meta::CompositeFunctorType2<Fn1, Fn2> >
  {
    typedef typename Implementation::OperationTraitsCombiner<
      typename OperationTraits<Fn1>::Ordering, typename OperationTraits<Fn2>::Ordering
    >::Result Ordering;
  };

  /**
   * Specialized traits class for composite functor 
   */
  template<typename Fn1, typename Fn2, typename Fn3>
  struct OperationTraits<CS::Meta::CompositeFunctorType3<Fn1, Fn2, Fn3> >
  {
    typedef typename Implementation::OperationTraitsCombiner<
      typename OperationTraits<Fn1>::Ordering, typename OperationTraits<Fn2>::Ordering
    >::Result Ordering1;

    typedef typename Implementation::OperationTraitsCombiner<
      Ordering1, typename OperationTraits<Fn3>::Ordering
    >::Result Ordering;
  };
  //@}


  //@{

  /**
   * Iterate over all contexts within render tree, call functor for each one.
   * Does not use any blocking at all.
   */
  template<typename RenderTree, typename Fn>
  void ForEachContext (RenderTree& tree, Fn& fn)
  {
    // Iterate over all contexts, calling the functor for each one
    typename RenderTree::ContextNodeArrayIteratorType it = tree.GetContextIterator ();

    Implementation::NoOperationBlock<typename RenderTree::ContextNode*> noBlock;
    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Implementation::NoOperationBlock<typename RenderTree::ContextNode*>,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, noBlock);

    while (it.HasNext ())
    {
      typename RenderTree::ContextNode* context = it.Next ();
      CS_ASSERT_MSG("Null context encountered, should not be possible", context);      

      caller (context);
    }
  }

  /**
   * Iterate over all contexts within render tree, call functor for each one.
   * Does use a user-supplied blocker for ignoring certain contexts.
   */
  template<typename RenderTree, typename Fn, typename Blocker>
  void ForEachContext (RenderTree& tree, Fn& fn, Blocker& block)
  {
    // Iterate over all contexts, calling the functor for each one
    typename RenderTree::ContextNodeArrayIteratorType it = tree.GetContextIterator ();

    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Blocker,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, block);

    while (it.HasNext ())
    {
      typename RenderTree::ContextNode* context = it.Next ();
      CS_ASSERT_MSG("Null context encountered, should not be possible", context);      

      caller (context);
    }
  }

  /**
   * Iterate over all contexts within render tree backwards, call functor for each one.
   * Does not use any blocking at all.
   */
  template<typename RenderTree, typename Fn>
  void ForEachContextReverse (RenderTree& tree, Fn& fn)
  {
    // Iterate over all contexts, calling the functor for each one
    typename RenderTree::ContextNodeArrayReverseIteratorType it = tree.GetReverseContextIterator ();

    Implementation::NoOperationBlock<typename RenderTree::ContextNode*> noBlock;
    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Implementation::NoOperationBlock<typename RenderTree::ContextNode*>,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, noBlock);

    while (it.HasNext ())
    {
      typename RenderTree::ContextNode* context = it.Next ();
      CS_ASSERT_MSG("Null context encountered, should not be possible", context);      

      caller (context);
    }
  }

  /**
   * Iterate over all contexts within render tree backwards, call functor for each one.
   * Does use a user-supplied blocker for ignoring certain contexts.
   */
  template<typename RenderTree, typename Fn, typename Blocker>
  void ForEachContextReverse (RenderTree& tree, Fn& fn, Blocker& block)
  {
    // Iterate over all contexts, calling the functor for each one
    typename RenderTree::ContextNodeArrayIteratorType it = tree.GetReverseContextIterator ();

    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Blocker,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, block);

    while (it.HasNext ())
    {
      typename RenderTree::ContextNode* context = it.Next ();
      CS_ASSERT_MSG("Null context encountered, should not be possible", context);      

      caller (context);
    }
  }

  //@}

  //@{
  /**
   * Iterate over all mesh nodes within context, call functor for each one.
   * Does not use any blocking at all.
   */
  template<typename ContextType, typename Fn>
  void ForEachMeshNode (ContextType& context, Fn& fn)
  {
    typename ContextType::TreeType::MeshNodeTreeIteratorType it = context.meshNodes.GetIterator ();

    Implementation::NoOperationBlock<typename ContextType::TreeType::MeshNode*> noBlock;
    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Implementation::NoOperationBlock<typename ContextType::TreeType::MeshNode*>,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, noBlock);

    while (it.HasNext ())
    {
      typename ContextType::TreeType::MeshNode* node = it.Next ();
      CS_ASSERT_MSG("Null node encountered, should not be possible", node);

      caller (node);
    }
  }

  /**
   * Iterate over all mesh nodes within context, call functor for each one.
   * Does use a user-supplied blocker for ignoring certain nodes.
   */
  template<typename ContextType, typename Fn, typename Blocker>
  void ForEachMeshNode (ContextType& context, Fn& fn, Blocker& blocker)
  {
    typename ContextType::TreeType::MeshNodeTreeIteratorType it = context.meshNodes.GetIterator ();

    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Blocker,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, blocker);

    while (it.HasNext ())
    {
      typename ContextType::TreeType::MeshNode* node = it.Next ();
      CS_ASSERT_MSG("Null node encountered, should not be possible", node);

      caller (node);
    }
  }

  /**
   * Iterate over all mesh nodes within context reversed, call functor for each one.
   * Does not use any blocking at all.
   */
  template<typename ContextType, typename Fn>
  void ForEachMeshNodeReverse (ContextType& context, Fn& fn)
  {
    typename ContextType::TreeType::MeshNodeTreeIteratorType it = context.meshNodes.GetReverseIterator ();

    Implementation::NoOperationBlock<typename ContextType::TreeType::MeshNode*> noBlock;
    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Implementation::NoOperationBlock<typename ContextType::TreeType::MeshNode*>,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, noBlock);

    while (it.HasNext ())
    {
      typename ContextType::TreeType::MeshNode* node = it.Next ();
      CS_ASSERT_MSG("Null node encountered, should not be possible", node);

      caller (node);
    }
  }

  /**
   * Iterate over all mesh nodes within context reversed, call functor for each one.
   * Does use a user-supplied blocker for ignoring certain nodes.
   */
  template<typename ContextType, typename Fn, typename Blocker>
  void ForEachMeshNodeReverse (ContextType& context, Fn& fn, Blocker& blocker)
  {
    typename ContextType::TreeType::MeshNodeTreeIteratorType it = context.meshNodes.GetReverseIterator ();

    // Helper object for calling function
    Implementation::OperationCaller<
      Fn, 
      Blocker,
      typename OperationTraits<Fn>::Ordering
    > caller (fn, blocker);

    while (it.HasNext ())
    {
      typename ContextType::TreeType::MeshNode* node = it.Next ();
      CS_ASSERT_MSG("Null node encountered, should not be possible", node);

      caller (node);
    }
  }

  namespace Implementation
  {
    template<typename ContextType, typename MeshFn, typename Blocker>
    struct MeshContextTraverser
    {
      MeshContextTraverser (MeshFn& fn, Blocker& blocker)
        : fn (fn), blocker (blocker)
      {}

      void operator() (ContextType* context)
      {
        ForEachMeshNode (*context, fn, blocker);
      }

      MeshFn& fn;
      Blocker& blocker;
    };
  }

  //@}
 



  //@{
  /**
   * Simple operation to assign sequential IDs to all mesh nodes within a
   * context.
   */
  template<typename RenderTree>
  class SingleMeshContextNumbering
  {
  public:
    SingleMeshContextNumbering ()
      : meshIndex (0)
    {}

    void operator () (size_t, typename RenderTree::MeshNode* node)
    {
      for (size_t i = 0; i < node->meshes.GetSize(); ++i)
      {
        node->meshes[i].contextLocalId = meshIndex++;
      }
    }

  private:
    size_t meshIndex;
  };

  template<typename RenderTree>
  struct OperationTraits<SingleMeshContextNumbering<RenderTree> >
  {
    typedef OperationNumbered Ordering;
  };
  //@}

}
}



#endif
