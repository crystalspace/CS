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
 * A generic finite partial order
 */

#include "csextern.h"
#include "array.h"
#include "comparator.h"
#include "ref.h"
#include "util.h"
#include "hash.h"

/**
 * A generic finite partial order class.
 * A finite partial order is a graph with the following properties:
 * <ul>
 * <li>A finite number of nodes (of type T).
 * <li>A finite number of reflexive tuple relations T1 <= T2.
 * <li>An absense of any non-trivial cycles, e.g., T1 < T2 < T1 where T1!=T2.
 * </ul>
 * An insert of an edge which violates the third constraint will fail
 * (return false and have no effect).
 * <p>
 * There must be a csHashComputer for type T.
 */
template <class T>
class CS_CRYSTALSPACE_EXPORT csPartialOrder
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

  /// Copy constructor
  csPartialOrder(const csPartialOrder *other)
    : Nodes (other->Nodes), NodeMap (other->NodeMap)
  {
    return;
  }
  
  /// Add a node.  If the node is already present, has no effect.
  void Add (const T& node)
  {
    if (NodeMap.Get(node, csArrayItemNotFound) == csArrayItemNotFound) 
    {
      NodeMap.PutUnique(node, Nodes.Push(node));
    }
    Dump();
  }

  /// Query a node's presence
  bool Contains (const T& node)
  {
    return (NodeMap.Get(node, csArrayItemNotFound) != csArrayItemNotFound);
  }
  
  /// Delete a node and all edges connected to it.
  void Delete (const T& node)
  {
    size_t p = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(p!=csArrayItemNotFound);
    // delete all posts pointing to node
    for (size_t iter=0 ; iter<Nodes[p].pre.Length() ; iter++) 
    {
      Nodes[Nodes[p].pre[iter]].post.DeleteFast(p);
    }
    // delete node's pre's
    Nodes[p].pre.DeleteAll();
    // delete all pres pointing to node
    for (size_t iter=0 ; iter<Nodes[p].post.Length() ; iter++) 
    {
      Nodes[Nodes[p].post[iter]].pre.DeleteFast(p);
    }
    // delete node's post's
    Nodes[p].post.DeleteAll();
    // delete node
    Nodes.DeleteIndexFast(p);
    NodeMap.Delete(node, p);
    
    Dump();
  }
  
  /**
   * Add an ordering constraint (node1 precedes node2).
   * Edge addition is not idempotent, i.e., if you
   * add an edge three times, it must be removed three times
   * before it will disappear from the graph.
   */
  bool AddOrder (const T& node1, const T& node2)
  {
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
  }
  
  /// Remove an ordering constraint (node1 precedes node2)
  void DeleteOrder (const T& node1, const T& node2)
  {
    size_t n1 = NodeMap.Get(node1, csArrayItemNotFound);
    size_t n2 = NodeMap.Get(node2, csArrayItemNotFound);
    CS_ASSERT(n1 != csArrayItemNotFound);
    CS_ASSERT(n2 != csArrayItemNotFound);
    Nodes[n2].pre.DeleteFast(n1);
    Nodes[n1].post.DeleteFast(n2);
  }
  
  /**
   * Produce a valid "solution" to the partial order graph, 
   * i.e., a sequence of nodes that violates no constraints.
   */
  void Solve (csList<const T> & result) {
    for (size_t iter=0 ; iter<Nodes.Length() ; iter++) 
    {
      Nodes[iter].output = false;
    }
    result.DeleteAll();
    bool done;
    do 
    {
      done = true;
      for (size_t iter=0 ; iter<Nodes.Length() ; iter++) 
      {
	if (Nodes[iter].output == false) 
	{
	  int canoutput = true;
	  for (size_t i2=0 ; i2<Nodes[iter].pre.Length() ; i2++) 
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
    return;
  }
  
  /**
   * Set the "marked" flag for a given node.
   * This is useful for implementing your own graph iterators.
   */
  void Mark (const T& node)
  {
    size_t i = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(i != csArrayItemNotFound);
    Nodes[i].marked = true;
  }
  
  /**
   * Query whether a given node is "marked".
   */
  bool IsMarked (const T& node)
  {
    size_t i = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(i != csArrayItemNotFound);
    return Nodes[i].marked;
  }
  
  /**
   * Clear the "marked" flag for a given node.
   */
  void ClearMark (const T& node)
  {
    size_t i = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(i != csArrayItemNotFound);
    Nodes[i].marked = false;
  }
  
  /**
   * Clear all "marked" flags.
   */
  void ClearMark ()
  {
    for (size_t i=0 ; i<Nodes.Length() ; i++) 
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
    for (size_t i=0 ; i<Nodes.Length() ; i++) 
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
    for (size_t i=0 ; i<Nodes.Length() ; i++) 
    {
      if (InternalIsEnabled(i))
	return Nodes[i].self;
    }
    return fail;
  }
  
private:
  bool CycleTest(const T& node)
  {
    size_t n = NodeMap.Get(node, csArrayItemNotFound);
    CS_ASSERT(n != csArrayItemNotFound);
    return InternalCycleTest(n);
  }
  
  bool InternalIsEnabled(size_t i) {
    if (Nodes[i].marked)
      return false;
    for (size_t j=0 ; j<Nodes[i].pre.Length() ; j++) 
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
    for (size_t i=0 ; i<Nodes[n2].post.Length() ; i++) 
    {
      if (InternalCycleTest(n1, Nodes[n2].post[i]))
	return true;
    }
    return false;
  }
  bool InternalCycleTest(size_t n)
  {
    for (size_t i=0 ; i<Nodes[n].post.Length() ; i++) 
    {
      if (InternalCycleTest(n, Nodes[n].post[i]))
	return true;
    }
    return false;
  }
  
public:
  void Dump() {
#if 0 /* ADB debugging stuff */
    std::cerr << "PARTIAL ORDER---------------------------------------" 
	      << std::endl;
    for (size_t i=0 ; i<Nodes.Length() ; i++) 
    {
      std::cerr << "Node #" << i << " [" << Nodes[i].self 
		<< "]:" << std::endl;
      std::cerr << "  pres:" << std::endl;
      for (size_t j=0 ; j<Nodes[i].pre.Length() ; j++) 
      {
	std::cerr << "    #" << Nodes[i].pre[j] << std::endl;
      }
      std::cerr << "  posts:" << std::endl;
      for (size_t j=0 ; j<Nodes[i].post.Length() ; j++) 
      {
	std::cerr << "    #" << Nodes[i].post[j] << std::endl;
      }
    }
    std::cerr << "PARTIAL ORDER END-----------------------------------" 
	      << std::endl << std::endl;
#endif
  }
};

#endif // __CS_UTIL_PO_H__
