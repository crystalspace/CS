/* -*- Mode: C++; c-basic-offset: 2 -*-
   Copyright (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_UTIL_PO_H__
#define __CS_UTIL_PO_H__

/**\file
 * Generic finite partial order
 */

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/comparator.h"
#include "csutil/hash.h"
#include "csutil/list.h"
#include "csutil/ref.h"
#include "csutil/util.h"

#ifdef ADB_DEBUG
#include "csutil/eventhandlers.h"
#include <iostream>
#endif /* ADB_DEBUG */

/**
 * A generic finite partial order class.
 * A finite partial order is a graph with the following properties:
 * - A finite number of nodes (of type T).
 * - A finite number of reflexive tuple relations T1 <= T2.
 * - An absense of any non-trivial cycles, e.g., T1 < T2 < T1 where T1!=T2.
 *
 * An insert of an edge which violates the third constraint will fail
 * (return false and have no effect).
 * <p>
 * There must be a csHashComputer for type T.
 */
template <class T>
class csPartialOrder
{
protected:
  class Node
  {
  public:
    Node(const Node &other) 
      : self(other.self), output(other.output),
	marked(other.marked),
	pre(other.pre), post(other.post)
    { }
    Node(const T &id)
      : self(id), output(false), marked(false),
	pre(), post()
    { }
    const T self;
    bool output;
    bool marked;
    csArray<size_t> pre;
    csArray<size_t> post;
  };
  csArray<Node> Nodes;
  csHash<size_t,const T> NodeMap;
	
public:
  /// Create a partial order graph
  csPartialOrder ()
    : Nodes (), NodeMap()
  {
    return;
  }

  /// Copy constructor
  csPartialOrder(const csPartialOrder &other)
    : Nodes (other.Nodes), NodeMap (other.NodeMap)
  {
    return;
  }

#ifdef ADB_DEBUG
  // This code is particular to the csHandlerID scheduler.
  void Dump (iObjectRegistry *objreg)
  {
    std::cerr << "Dumping PO Graph..." << std::endl;
    for (size_t i=0 ; i<Nodes.GetSize () ; i++)
    {
      std::cerr << "  NODE: " << csEventHandlerRegistry::GetRegistry(objreg)->GetString(Nodes[i].self) << std::endl;
      std::cerr << "    pre: ";
      for (size_t j=0 ; j<Nodes[i].pre.GetSize () ; j++) 
      {
	std::cerr << csEventHandlerRegistry::GetRegistry(objreg)->GetString(Nodes[Nodes[i].pre[j]].self) << " ";
      }
      std::cerr << std::endl << "    post: ";
      for (size_t j=0 ; j<Nodes[i].post.GetSize () ; j++)
      {
	std::cerr << csEventHandlerRegistry::GetRegistry(objreg)->GetString(Nodes[Nodes[i].post[j]].self) << " ";
      }
      std::cerr << std::endl;
    }
    std::cerr << "End PO Graph, printing a solution..." << std::endl;
    csList<const T> Solution;
    Solve (Solution);
    while (!Solution.IsEmpty ())
    {
      std::cerr << csEventHandlerRegistry::GetRegistry(objreg)->GetString(Solution.Front()) << " ";
      Solution.PopFront();
    }
    std::cerr << std::endl << "Done." << std::endl << std::endl;
  }
#endif /* ADB_DEBUG */

  /// Copy constructor
  csPartialOrder(const csPartialOrder *other)
    : Nodes (other->Nodes), NodeMap (other->NodeMap)
  {
    return;
  }
  
  /// Add a node.  If the node is already present, has no effect.
  void Add (const T& node)
  {
    SanityCheck();
    if (NodeMap.Get(node, csArrayItemNotFound) == csArrayItemNotFound) 
    {
      NodeMap.PutUnique(node, Nodes.Push(node));
    }
    SanityCheck();
  }

  /// Query a node's presence
  bool Contains (const T& node)
  {
    return (NodeMap.Get(node, csArrayItemNotFound) != csArrayItemNotFound);
  }

  /// Query an edge's presence.  Does not check for transitive connectivity.
  bool Contains (const T& pre, const T& post)
  {
    if (!Contains (pre) || !Contains (post))
      return false;
    size_t PreIdx = NodeMap.Get (pre, csArrayItemNotFound);
    size_t PostIdx = NodeMap.Get (post, csArrayItemNotFound);
    for (size_t i=0 ; i<Nodes[PreIdx].post.GetSize () ; i++)
    {
      if (Nodes[PreIdx].post[i] == PostIdx)
      {
	return true;
      }
    }
    return false;
  }
  
  /// Delete a node and all edges connected to it.
  void Delete (const T& node)
  {
    SanityCheck();
    size_t p = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(p!=csArrayItemNotFound);
    CS_ASSERT(p < Nodes.GetSize ());
    // delete all posts pointing to node
    for (size_t iter=0 ; iter<Nodes[p].pre.GetSize () ; iter++) 
    {
      Nodes[Nodes[p].pre[iter]].post.Delete(p);
    }
    // delete node's pre's
    Nodes[p].pre.DeleteAll();
    // delete all pres pointing to node
    for (size_t iter=0 ; iter<Nodes[p].post.GetSize () ; iter++) 
    {
      Nodes[Nodes[p].post[iter]].pre.Delete(p);
    }
    // delete node's post's
    Nodes[p].post.DeleteAll();
    // delete node p, move the node at the end of the csArray into its place...
    Nodes.DeleteIndexFast(p);

    // update NodeMap accordingly by killing the lookup for the deleted node
    // and updating the lookup for the node that moved into its place...
    NodeMap.Delete(node, p);
    if (Nodes.GetSize () > p)
    {
      // who got moved into "p"?
      size_t moved = NodeMap.Get(Nodes[p].self, csArrayItemNotFound);
      CS_ASSERT (moved != csArrayItemNotFound);

      // change references to "moved" to reference "p"
      for (size_t iter=0 ; iter<Nodes.GetSize () ; iter++)
      {
	for (size_t iter2=0 ; iter2<Nodes[iter].pre.GetSize () ; iter2++)
	{
	  if (Nodes[iter].pre[iter2] == moved)
	    Nodes[iter].pre[iter2] = p;
	}
	for (size_t iter2=0 ; iter2<Nodes[iter].post.GetSize () ; iter2++)
	{
	  if (Nodes[iter].post[iter2] == moved)
	    Nodes[iter].post[iter2] = p;
	}
      }
      NodeMap.Delete(Nodes[p].self, moved);
      NodeMap.PutUnique(Nodes[p].self, p);
    }

    SanityCheck();
  }
  
  /**
   * Add an ordering constraint (node1 precedes node2).
   * Edge addition is not idempotent, i.e., if you
   * add an edge three times, it must be removed three times
   * before it will disappear from the graph.
   */
  bool AddOrder (const T& node1, const T& node2)
  {
    SanityCheck();
    size_t n1 = NodeMap.Get(node1, csArrayItemNotFound);
    size_t n2 = NodeMap.Get(node2, csArrayItemNotFound);
    CS_ASSERT(n1 != csArrayItemNotFound);
    CS_ASSERT(n2 != csArrayItemNotFound);
    
    /* make the forward link "n1->n2" */
    Nodes[n1].post.Push(n2);
    
    if (InternalCycleTest(n1)) 
    {
      /* undo our mischief and report failure */
      Nodes[n1].post.Pop();
      return false;
    } 
    else 
    {
      /* make the paired pre link "n2<-n1" */
      Nodes[n2].pre.Push(n1);
      return true;
    }
    SanityCheck();
  }
  
  /// Remove an ordering constraint (node1 precedes node2)
  void DeleteOrder (const T& node1, const T& node2)
  {
    SanityCheck();
    size_t n1 = NodeMap.Get(node1, csArrayItemNotFound);
    size_t n2 = NodeMap.Get(node2, csArrayItemNotFound);
    CS_ASSERT(n1 != csArrayItemNotFound);
    CS_ASSERT(n2 != csArrayItemNotFound);
    Nodes[n2].pre.DeleteFast(n1);
    Nodes[n1].post.DeleteFast(n2);
    SanityCheck();
  }

  /// Number of nodes in the graph
  size_t Size ()
  {
    return Nodes.GetSize ();
  }

  /// Return a node with a given index (0 through Size()-1)
  const T GetByIndex(size_t i)
  {
    return Nodes[i].self;
  }
  
  /**
   * Produce a valid "solution" to the partial order graph, 
   * i.e., a sequence of nodes that violates no constraints.
   */
  void Solve (csList<const T> & result)
  {
    SanityCheck();
    for (size_t iter=0 ; iter<Nodes.GetSize () ; iter++) 
    {
      Nodes[iter].output = false;
    }
    result.DeleteAll();
    bool done;
    do 
    {
      done = true;
      for (size_t iter=0 ; iter<Nodes.GetSize () ; iter++) 
      {
	if (Nodes[iter].output == false) 
	{
	  int canoutput = true;
	  for (size_t i2=0 ; i2<Nodes[iter].pre.GetSize () ; i2++) 
	  {
	    if (!Nodes[Nodes[iter].pre[i2]].output) 
	    {
	      canoutput = false;
	      break;
	    }
	  }
	  if (canoutput) 
	  {
	    result.PushBack(Nodes[iter].self);
	    Nodes[iter].output = true;
	  } 
	  else 
	  {
	    done = false;
	  }
	}
      }
    } 
    while (!done);
    SanityCheck();
  }
  
  /**
   * Set the "marked" flag for a given node.
   * This is useful for implementing your own graph iterators.
   */
  void Mark (const T& node)
  {
    size_t i = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(i != csArrayItemNotFound);
    CS_ASSERT(i < Nodes.GetSize ());
    Nodes[i].marked = true;
  }
  
  /**
   * Query whether a given node is "marked".
   */
  bool IsMarked (const T& node)
  {
    size_t i = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(i != csArrayItemNotFound);
    CS_ASSERT(i < Nodes.GetSize ());
    return Nodes[i].marked;
  }
  
  /**
   * Clear the "marked" flag for a given node.
   */
  void ClearMark (const T& node)
  {
    size_t i = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(i != csArrayItemNotFound);
    CS_ASSERT(i < Nodes.GetSize ());
    Nodes[i].marked = false;
  }
  
  /**
   * Clear all "marked" flags.
   */
  void ClearMark ()
  {
    for (size_t i=0 ; i<Nodes.GetSize () ; i++) 
    {
      Nodes[i].marked = false;
    }
  }
  
  /**
   * Return true if all of the node's (zero or more) predecessors 
   * have been marked and the node itself has not.
   */
  bool IsEnabled(const T& node)
  {
    size_t i = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(i != csArrayItemNotFound);
    return InternalIsEnabled(i);
  }
  
  /**
   * Return true if any node is enabled.
   */
  bool HasEnabled()
  {
    for (size_t i=0 ; i<Nodes.GetSize () ; i++) 
    {
      if (InternalIsEnabled(i))
	return true;
    }
    return false;
  }
  
  /**
   * Return an enabled node
   */
  const T GetEnabled(T fail)
  {
    for (size_t i=0 ; i<Nodes.GetSize () ; i++) 
    {
      if (InternalIsEnabled(i))
	return Nodes[i].self;
    }
    return fail;
  }
  
protected:
  void SanityCheck ()
  {
#ifdef CS_DEBUG
    CS_ASSERT_MSG ("NodeMap has different size from Node list", 
		   NodeMap.GetSize() == Nodes.GetSize ());
    for (size_t i1=0; i1<Nodes.GetSize () ; i1++)
    {
      CS_ASSERT_MSG ("NodeMap names wrong location for node",
		     NodeMap.Get(Nodes[i1].self, csArrayItemNotFound) == i1);
      for (size_t i2=0 ; i2<Nodes[i1].pre.GetSize () ; i2++)
      {
	CS_ASSERT_MSG ("Node prefix index less than zero", 
		       Nodes[i1].pre[i2] >= 0);
	CS_ASSERT_MSG ("Node prefix index larger than Nodes list",
		       Nodes[i1].pre[i2] < Nodes.GetSize ());
	bool reciprocal_post_exists = false;
	for (size_t i3=0 ; i3<Nodes[Nodes[i1].pre[i2]].post.GetSize () ; i3++)
	{
	  if (Nodes[Nodes[i1].pre[i2]].post[i3] == i1)
          {
	    reciprocal_post_exists = true;
            break;
          }
	}
	CS_ASSERT_MSG ("Node prefix does not have reciprocal postfix", 
		       reciprocal_post_exists);
      }
      for (size_t i2=0 ; i2<Nodes[i1].post.GetSize () ; i2++)
      {
	CS_ASSERT_MSG ("Node postfix index less than zero",
		       Nodes[i1].post[i2] >= 0);
	CS_ASSERT_MSG ("Node postfix index larger than Nodes list",
		       Nodes[i1].post[i2] < Nodes.GetSize ());
	bool reciprocal_pre_exists = false;
	for (size_t i3=0 ; i3<Nodes[Nodes[i1].post[i2]].pre.GetSize () ; i3++)
	{
	  if (Nodes[Nodes[i1].post[i2]].pre[i3] == i1)
          {
	    reciprocal_pre_exists = true;
            break;
          }
	}
	CS_ASSERT_MSG ("Node postfix does not have reciprocal prefix",
		       reciprocal_pre_exists);
      }
    }
    typename csHash<size_t,const T>::GlobalIterator iter = 
      NodeMap.GetIterator ();
    while (iter.HasNext())
    {
      size_t idx = iter.Next();
      CS_ASSERT_MSG ("NodeMap contains an index larger than Nodes list",
		     idx < Nodes.GetSize ());
    }
#endif /* CS_DEBUG */
  }

  bool CycleTest(const T& node)
  {
    size_t n = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(n != csArrayItemNotFound);
    return InternalCycleTest(n);
  }
  
  bool InternalIsEnabled(size_t i)
  {
    if (Nodes[i].marked)
      return false;
    for (size_t j=0 ; j<Nodes[i].pre.GetSize () ; j++) 
    {
      if (!Nodes[Nodes[i].pre[j]].marked)
	return false;
    }
    return true;
  }
  
  bool InternalCycleTest(size_t n1, size_t n2)
  {
    // n1 is the inserted node, see if n2 hits n1 again...
    if (n1==n2)
      return true;
    for (size_t i=0 ; i<Nodes[n2].post.GetSize () ; i++) 
    {
      if (InternalCycleTest(n1, Nodes[n2].post[i]))
	return true;
    }
    return false;
  }
  bool InternalCycleTest(size_t n)
  {
    for (size_t i=0 ; i<Nodes[n].post.GetSize () ; i++) 
    {
      if (InternalCycleTest(n, Nodes[n].post[i]))
	return true;
    }
    return false;
  }
};

#endif // __CS_UTIL_PO_H__
