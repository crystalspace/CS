/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005-2010 by Frank Richter
              (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_UTIL_REDBLACKTREE_H__
#define __CS_UTIL_REDBLACKTREE_H__

#include "csutil/blockallocator.h"

// hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

/**\file
 * Implementation of a red-black-tree.
 */

/**\addtogroup util_containers
 * @{ */

template <typename K, typename Allocator,
  template<typename K, typename K2> class Ordering>
class csRedBlackTree;

namespace CS
{
  namespace Container
  {
    /**\internal
    * A node in a red black tree
    */
    template <typename K>
    class RedBlackTreeNode
    {
    protected:
      enum Color { Black = 0, Red = 1 };

      template <typename _T, typename _A,
	template<typename _K, typename _K2> class _O>
      friend class csRedBlackTree;

      RedBlackTreeNode* left;
      RedBlackTreeNode* right;
      uint8 key[sizeof(K)];

      RedBlackTreeNode() : parent(0) {}
      ~RedBlackTreeNode() { GetKey().~K(); }
      inline RedBlackTreeNode* GetParent() const
      { return (RedBlackTreeNode*)((uintptr_t)parent & (uintptr_t)~1); }
      void SetParent(RedBlackTreeNode* p)
      { parent = (RedBlackTreeNode*)(((uintptr_t)p & (uintptr_t)~1) | (uint)GetColor()); }
      Color GetColor() const
      { // Expression split over two statements to pacify some broken gcc's which
	// barf saying "can't convert Node* to NodeColor".
	uintptr_t const v = ((uintptr_t)parent & 1);
	return (Color)v;
      }
      void SetColor (Color color)
      { parent = (RedBlackTreeNode*)(((uintptr_t)parent & (uintptr_t)~1) | (uint)color); }
    private:
      /// Pointer to parent, but also stores color in LSB.
      RedBlackTreeNode* parent;
    public:
      //@{
      /// Accessor for node field.
      inline RedBlackTreeNode* GetLeft() const { return left; }
      inline RedBlackTreeNode* GetRight() const { return right; }
      inline K& GetKey ()
      {
	/* Cast through 'void*' to avoid strict aliasing warning.
	  'key' is not ever actually accessed through an uint8*, so the strict
	  aliasing assumption practically still holds. */
	void* p = &key;
	return *(reinterpret_cast<K*> (p));
      }
      inline const K& GetKey () const
      {
	// See above
	const void* p = &key;
	return *(reinterpret_cast<const K*> (p));
      }
      //@}
    };
    
    /// Default allocator for red black trees.
    template <typename K>
    struct DefaultRedBlackTreeAllocator :
      public csFixedSizeAllocator<sizeof(RedBlackTreeNode<K>)>
    {
    };
    
    /// Total ordering for red-black-trees.
    template <typename K, typename K2>
    class RedBlackTreeOrderingTotal
    {
      const K& a;
      const K2& b;
      bool lt;
    public:
      RedBlackTreeOrderingTotal (const K& a, const K2& b)
       : a (a), b (b), lt (a <= b) {}
      
      bool AeqB () const { return a == b; }
      bool AleB () const { return lt; }
      /* Note: For a total ordering, at least either AleB or BleA is always 
       * true. BleA is only used by csRedBlackTree if AeqB and AleB are false;
       * that the comparision here is actually 'b < a' instead of 'b <= a'
       * - which the name suggests - has thus no errorneous consequences.
       */
      bool BleA () const { return !lt; }
    };
    
    /// Partial ordering for red-black-trees.
    template <typename K, typename K2>
    class RedBlackTreeOrderingPartial
    {
      const K& a;
      const K2& b;
    public:
      RedBlackTreeOrderingPartial (const K& a, const K2& b)
       : a (a), b (b) {}
      
      bool AeqB () const { return a == b; }
      bool AleB () const { return a <= b; }
      bool BleA () const { return b <= a; }
    };
  } // namespace Container
} // namespace CS

/**
 * A red-black-tree.
 * \par Uniqueness of keys
 * Keys do <em>not</em> need to be unique. However, searching for a non-unique
 * key Find() will return an <em>equivalent</em> key, not necessarily the
 * <em>identical</em> key.
 *
 * \par Ordering
 * By default, a <em>totally ordered</em> (http://en.wikipedia.org/wiki/Total_ordering)
 * key type is assumed. However, it is possible to use a key type that is only
 * <em>partially ordered</em> (http://en.wikipedia.org/wiki/Partial_order) -
 * however, this will change the runtime characteristics. In the worst case -
 * no key comparable to any other key - operations may take linear time
 * instead of logarithmic time as operations degenerate into an
 * exhaustive search. 
 *
 * \par
 * The ordering of the key is controlled by changing the \a Ordering template
 * parameter. The default, total ordering is selected by
 * CS::Container::RedBlackTreeOrderingTotal. Partial ordering is selected
 * by CS::Container::RedBlackTreeOrderingPartial.
 *
 * \par Operators, key types
 * As a minimum, \a K must have <tt>operator==()</tt> and <tt>operator<=()</tt>.
 *
 * \par
 * The various "Find" methods can take keys of an alternative type (designated
 * \a K2). Operators available must cover the comparisons <tt>K == K2</tt>,
 * <tt>K <= K2</tt> and <tt>K2 <= K</tt>. The ordering of \a K2 should, sensibly,
 * be the same as that of \a K.
 *
 * \remark Only stores keys. If you need a key-value-map, look at
 *  csRedBlackTreeMap.
 * \remark The allocator has to return memory blocks at least aligned to
 *  two byte boundaries. (This is usually already the case.)
 * \remark Allocation requests to the allocator have a size of
 *  <tt>csRedBlackTree<K>::allocationUnitSize</tt>.
 */
template <typename K,
          typename Allocator =
	    CS::Container::DefaultRedBlackTreeAllocator<K>,
	  template<typename K, typename K2> class Ordering =
	    CS::Container::RedBlackTreeOrderingTotal>
class csRedBlackTree
{
protected:
  typedef CS::Container::RedBlackTreeNode<K> Node;
public:
  enum { allocationUnitSize = sizeof (Node) };
protected:
  CS::Memory::AllocatorPointerWrapper<Node, Allocator> root;

  Node* AllocNode ()
  {
    Node* p = (Node*)root.Alloc (allocationUnitSize);
    CS_ASSERT_MSG("Allocator returned block that is not 2-byte-aligned",
      (uintptr_t(p) & 1) == 0);
    new (p) Node;
    return p;
  }

  void FreeNode (Node* p)
  {
    p->~Node();
    root.Free (p);
  }

  /// Create a new tree node
  Node* CreateNode (Node* parent, const K& key)
  {
    Node* node;
    node = AllocNode();
    node->SetParent (parent);
    node->left = node->right = 0;
    new ((K*)&node->key) K (key);
    node->SetColor (Node::Red);
    return node;
  }

  struct InsertCandidate
  {
    Node* parent;
    Node** node;
    uint depth;
    
    InsertCandidate() : parent (0), node (0), depth ((uint)~0) {}
  };
  /// Locate the place where a new node needs to be inserted.
  Node* RecursiveFindInsertCandidate (Node* parent, Node*& node, 
				      const K& key, uint depth,
				      InsertCandidate& candidate)
  {
    if (node == 0)
    {
      if (depth < candidate.depth)
      {
	candidate.parent = parent;
	candidate.node = &node;
	candidate.depth = depth;
      }
      return 0;
    }
    
    Ordering<K, K> _o (node->GetKey(), key);
    if (_o.AleB())
    {
      // New node _must_ be inserted somewhere in the left tree
      InsertCandidate newCandidate;
      Node* n = RecursiveFindInsertCandidate (node, node->left, key, depth+1,
					      newCandidate);
      if (n == 0)
      {
	n = *newCandidate.node = CreateNode (newCandidate.parent, key);
      }
      return n;
    }
    
    if (_o.BleA())
    {
      // New node _must_ be inserted somewhere in the right tree
      InsertCandidate newCandidate;
      Node* n = RecursiveFindInsertCandidate (node, node->right, key, depth+1,
					      newCandidate);
      if (n == 0)
      {
	n = *newCandidate.node = CreateNode (newCandidate.parent, key);
      }
      return n;
    }
    /* Left or right tree, doesn't matter. Try to find a place with a 
       small depth to keep the tree as balanced as possible. */
    Node* n = RecursiveFindInsertCandidate (node, node->left, key, depth+1,
					    candidate);
    if (n == 0)
      n = RecursiveFindInsertCandidate (node, node->right, key, depth+1,
					candidate);
    if (n == 0)
    {
      n = *candidate.node = CreateNode (candidate.parent, key);
    }
    return n;
  }
  /// Left-rotate subtree around 'pivot'.
  void RotateLeft (Node* pivot)
  {
    Node* pivotReplace = pivot->right;
    pivot->right = pivotReplace->left;
    if (pivotReplace->left != 0)
      pivotReplace->left->SetParent (pivot);
    pivotReplace->SetParent (pivot->GetParent());
    if (pivot->GetParent() == 0)
      root.p = pivotReplace;
    else
    {
      if (pivot == pivot->GetParent()->left)
	pivot->GetParent()->left = pivotReplace;
      else
	pivot->GetParent()->right = pivotReplace;
    }
    pivotReplace->left = pivot;
    pivot->SetParent (pivotReplace);
  }
  /// Right-rotate subtree around 'pivot'.
  void RotateRight (Node* pivot)
  {
    Node* pivotReplace = pivot->left;
    pivot->left = pivotReplace->right;
    if (pivotReplace->right != 0)
      pivotReplace->right->SetParent (pivot);
    pivotReplace->SetParent (pivot->GetParent());
    if (pivot->GetParent() == 0)
      root.p = pivotReplace;
    else
    {
      if (pivot == pivot->GetParent()->left)
	pivot->GetParent()->left = pivotReplace;
      else
	pivot->GetParent()->right = pivotReplace;
    }
    pivotReplace->right = pivot;
    pivot->SetParent (pivotReplace);
  }
  /// Check whether a node is black. Note that 0 nodes are by definition black.
  bool IsBlack (Node* node) const
  { return (node == 0) || (node->GetColor() == Node::Black); }
  /// Check whether a node is red.
  bool IsRed (Node* node) const
  { return (node != 0) && (node->GetColor() == Node::Red); }
  /// Fix up the RB tree after an insert.
  void InsertFixup (Node* node)
  {
    Node* p;
    while (((p = node->GetParent()) != 0) && IsRed (p))
    {
      Node* pp = p->GetParent();
      if (pp == 0) break;
      if (p == pp->left)
      {
	Node* y = pp->right;
	if (IsRed (y))
	{
	  // Uncle of 'node' is red
	  p->SetColor (Node::Black);
	  y->SetColor (Node::Black);
	  pp->SetColor (Node::Red);
	  node = pp;
	}
	else
	{
	  if (node == p->right)
	  {
	    // Uncle of 'node' is black, node is right child
	    node = p;
	    RotateLeft (node);
	    p = node->GetParent ();
	    pp = p->GetParent();
	  }
	  // Uncle of 'node' is black, node is left child
	  p->SetColor (Node::Black);
	  pp->SetColor (Node::Red);
	  RotateRight (pp);
	}
      }
      else
      {
	Node* y = pp->left;
	if (IsRed (y))
	{
	  // Uncle of 'node' is red
	  p->SetColor (Node::Black);
	  y->SetColor (Node::Black);
	  pp->SetColor (Node::Red);
	  node = pp;
	}
	else
	{
	  if (node == p->left)
	  {
	    // Uncle of 'node' is black, node is left child
	    node = p;
	    RotateRight (node);
	    p = node->GetParent ();
	    pp = p->GetParent();
	  }
	  // Uncle of 'node' is black, node is right child
	  p->SetColor (Node::Black);
	  pp->SetColor (Node::Red);
	  RotateLeft (pp);
	}
      }
    }
    root.p->SetColor (Node::Black);
  }

  /// Delete a node from the tree.
  void DeleteNode (Node* node)
  {
    Node* y; // Node we will replace 'node' with
    if ((node->left == 0) || (node->right == 0))
      y = node;
    else
      y = Predecessor (node);
    Node* x;
    if (y->left != 0)
      x = y->left;
    else
      x = y->right;
    Node* nilParent = 0;
    if (x != 0)
      x->SetParent (y->GetParent());
    else
      nilParent = y->GetParent();
    if (y->GetParent() == 0)
      root.p = x;
    else
    {
      if (y == y->GetParent()->left)
	y->GetParent()->left = x;
      else
	y->GetParent()->right = x;
    }
    if (y != node)
    {
      // Copy key
      node->GetKey() = y->GetKey();
    }
    if (y->GetColor() == Node::Black)
      DeleteFixup (x, nilParent);
    FreeNode (y);
  }
  /// Fix up the RB tree after a deletion.
  void DeleteFixup (Node* node, Node* nilParent)
  {
    while ((node != root.p) && IsBlack (node))
    {
      Node* p = node ? node->GetParent() : nilParent;
      if (node == p->left)
      {
	Node* w = p->right;
	if (IsRed (w))
	{
	  w->SetColor (Node::Black);
	  p->SetColor (Node::Red);
	  RotateLeft (p);
	  p = node ? node->GetParent() : nilParent;
	  w = p->right;
	}
	if (IsBlack (w->left) && IsBlack (w->right))
	{
	  w->SetColor (Node::Red);
	  node = p;
	}
	else
	{
	  if (IsBlack (w->right))
	  {
	    w->left->SetColor (Node::Red);
	    w->SetColor (Node::Red);
	    RotateRight (w);
	    p = node ? node->GetParent() : nilParent;
	    w = p->right;
	  }
	  w->SetColor (p->GetColor ());
	  p->SetColor (Node::Black);
	  w->right->SetColor (Node::Black);
	  RotateLeft (p);
	  node = root.p;
	}
      }
      else
      {
	Node* w = p->left;
	if (IsRed (w))
	{
	  w->SetColor (Node::Black);
	  p->SetColor (Node::Red);
	  RotateRight (p);
	  p = node ? node->GetParent() : nilParent;
	  w = p->left;
	}
	if (IsBlack (w->left) && IsBlack (w->right))
	{
	  w->SetColor (Node::Red);
	  node = p;
	}
	else
	{
	  if (IsBlack (w->left))
	  {
	    w->right->SetColor (Node::Red);
	    w->SetColor (Node::Red);
	    RotateLeft (w);
	    p = node ? node->GetParent() : nilParent;
	    w = p->left;
	  }
	  w->SetColor (p->GetColor ());
	  p->SetColor (Node::Black);
	  w->left->SetColor (Node::Black);
	  RotateRight (p);
	  node = root.p;
	}
      }
    }
    if (node != 0) node->SetColor (Node::Black);
  }
  /// Find a node for a key.
  template<typename K2>
  Node* LocateNode (Node* node, const K2& other) const
  {
    if (node == 0) return 0;

    Ordering<K, K2> _o (node->GetKey(), other);
    if (_o.AeqB())
      return node;
    Node* n = 0;
    // key <= other, node with 'other' must be in left subtree
    bool leftTree = _o.AleB();
    // other <= key, node with 'other' must be in right subtree
    bool rightTree = _o.BleA();
    // (!leftTree && !rightTree) => node can be in either subtree
    if (leftTree || (!leftTree && !rightTree))
      n = LocateNode (node->left, other);
    if ((n == 0) && (rightTree || (!leftTree && !rightTree)))
      n = LocateNode (node->right, other);
    return n;
  }
  /// Find the node which has a given instance of a key
  Node* LocateNodeExact (Node* node, const K* key) const
  {
    if (node == 0) return 0;

    if (&(node->GetKey()) == key) return node;

    Ordering<K, K> _o (node->GetKey(), *key);
    Node* n = 0;
    // key <= other, node with 'other' must be in left subtree
    bool leftTree = _o.AleB();
    // other <= key, node with 'other' must be in right subtree
    bool rightTree = _o.BleA();
    // (!leftTree && !rightTree) => node can be in either subtree
    if (leftTree || (!leftTree && !rightTree))
      n = LocateNodeExact (node->left, key);
    /* @@@ Go down right node if key equals node value as sometimes
       the node we look for ended up in the right subtree. */
    if ((n == 0) && (rightTree || (!leftTree && !rightTree) || _o.AeqB()))
      n = LocateNodeExact (node->right, key);
    return n;
  }
  /// Return smallest node with a key greater than 'node's.
  static Node* Successor (const Node* node)
  {
    Node* succ;
    if (node->right != 0)
    {
      succ = node->right;
      while (succ->left != 0) succ = succ->left;
      return succ;
    }
    Node* y = node->GetParent();
    while ((y != 0) && (node == y->right))
    {
      node = y;
      y = y->GetParent();
    }
    return y;
  }
  /// Return largest node with a key smaller than 'node's.
  static Node* Predecessor (const Node* node)
  {
    Node* pred;
    if (node->left != 0)
    {
      pred = node->left;
      while (pred->right != 0) pred = pred->right;
      return pred;
    }
    Node* y = node->GetParent();
    while ((y != 0) && (node == y->left))
    {
      node = y;
      y = y->GetParent();
    }
    return y;
  }

  //@{
  /// Locate greatest key that is smaller or equal to 'other'.
  template<typename K2>
  Node* RecursiveFindGSE (Node* node, const K2& other) const
  {
    if (node == 0) return 0;
    Ordering<K, K2> _o (node->GetKey (), other);
    if (_o.AeqB())
      return node;
    Node* n = 0;
    // key <= other, node with 'other' must be in left subtree
    bool leftTree = _o.AleB();
    // other <= key, node with 'other' must be in right subtree
    bool rightTree = _o.BleA();
    // (!leftTree && !rightTree) => node can be in either subtree
    if (leftTree || (!leftTree && !rightTree))
    {
      n = RecursiveFindGSE<K2> (node->left, other);
      /* This node is currently the smallest known greater or equal to
       * 'other', so return that if a search in the left subtree did
       * not give a result */
      if (leftTree && (n == 0)) n = node;
    }
    if (n == 0)
    {
      n = RecursiveFindGSE<K2> (node->right, other);
    }
    return n;
  }
  //@}

  //@{
  /// Locate smallest key that is greater or equal to 'other'.
  template<typename K2>
  Node* RecursiveFindSGE (Node* node, const K2& other) const
  {
    if (node == 0) return 0;
    Ordering<K, K2> _o (node->GetKey (), other);
    if (_o.AeqB())
      return node;
    Node* n = 0;
    // key <= other, node with 'other' must be in left subtree
    bool leftTree = _o.AleB();
    // other <= key, node with 'other' must be in right subtree
    bool rightTree = _o.BleA();
    // (!leftTree && !rightTree) => node can be in either subtree
    if (rightTree || (!leftTree && !rightTree))
    {
      n = RecursiveFindSGE<K2> (node->right, other);
      /* This node is currently the smallest known greater or equal to
       * 'other', so return that if a search in the right subtree did
       * not give a result */
      if (rightTree && (n == 0)) n = node;
    }
    if (n == 0)
    {
      n = RecursiveFindSGE<K2> (node->left, other);
    }
    return n;
  }
  //@}

  /// Traverse tree.
  template <typename CB>
  void RecursiveTraverseInOrder (Node* node, CB& callback) const
  {
    if (node->right != 0) RecursiveTraverseInOrder (node->right, callback);
    callback (node->GetKey());
    if (node->left != 0) RecursiveTraverseInOrder (node->left, callback);
  }

  /// Duplicate a subtree
  void RecursiveCopy (Node*& to, Node* parent, const Node* from)
  {
    if (from == 0)
    {
      to = 0;
      return;
    }
    to = AllocNode ();
    to->SetParent (parent);
    to->SetColor (from->GetColor());
    new ((K*)&to->key) K (from->GetKey());
    RecursiveCopy (to->left, to, from->left);
    RecursiveCopy (to->right, to, from->right);
  }

  /// Recursively delete a subtree
  void RecursiveDelete (Node* node)
  {
    if (node == 0) return;
    RecursiveDelete (node->left);
    RecursiveDelete (node->right);
    FreeNode (node);
  }
public:
  /**
   * Construct a new tree.
   * \param allocatorBlockSize Block size in bytes used by the internal block
   *  allocator for nodes.
   */
  csRedBlackTree (const Allocator& alloc = Allocator()) : root(alloc, 0) { }
  csRedBlackTree (const csRedBlackTree& other) : root (other.root, 0)
  {
    RecursiveCopy (root.p, 0, other.root.p);
  }
  /// Destroy the tree
  ~csRedBlackTree() { RecursiveDelete (root.p); }

  /**
   * Insert a key.
   * \return A pointer to the copy of the key stored in the tree.
   */
  const K* Insert (const K& key)
  {
    InsertCandidate newCandidate;
    Node* n = RecursiveFindInsertCandidate (0, root.p, key, 0, newCandidate);
    if (n == 0)
      n = *newCandidate.node = CreateNode (newCandidate.parent, key);
    InsertFixup (n);
    return &(n->GetKey());
  }
  /**
   * Delete a key.
   * \return Whether the deletion was successful. Fails if the key is not
   *  in the tree.
   */
  bool Delete (const K& key)
  {
    Node* n = LocateNode (root.p, key);
    if (n == 0) return false;
    DeleteNode (n);
    return true;
  }
  /**
   * Delete a specific instance of a key.
   * \return Whether the deletion was successful. Fails if the key is not
   *  in the tree.
   */
  bool DeleteExact (const K* key)
  {
    Node* n = LocateNodeExact (root.p, key);
    if (n == 0) return false;
    DeleteNode (n);
    return true;
  }
  /// Check whether a key is in the tree.
  bool In (const K& key) const
  {
    return (LocateNode (root.p, key) != 0);
  }
  /**
   * Check whether a key is in the tree.
   * \remarks This is rigidly equivalent to Contains(key), but may be
   *   considered more idiomatic by some.
   */
  bool Contains (const K& key) const { return In (key); }

  //@{
  /// Locate key that is equal to \a other
  template<typename K2>
  const K* Find (const K2& other) const
  {
    Node* n = LocateNode<K2> (root.p, other);
    return n ? &(n->GetKey()) : 0;
  }
  template<typename K2>
  const K& Find (const K2& other, const K& fallback) const
  {
    Node* n = LocateNode<K2> (root.p, other);
    return n ? n->GetKey() : fallback;
  }
  //@}

  //@{
  /// Locate smallest key greater or equal to \a other
  template<typename K2>
  const K* FindSmallestGreaterEqual (const K2& other) const
  {
    Node* n = RecursiveFindSGE<K2> (root.p, other);
    return n ? &(n->GetKey()) : 0;
  }
  template<typename K2>
  const K& FindSmallestGreaterEqual (const K2& other,
                                     const K& fallback) const
  {
    Node* n = RecursiveFindSGE<K2> (root.p, other);
    return n ? n->GetKey() : fallback;
  }
  //@}

  //@{
  /// Locate greatest key smaller or equal to \a other
  template<typename K2>
  const K* FindGreatestSmallerEqual (const K2& other) const
  {
    Node* n = RecursiveFindGSE<K2> (root.p, other);
    return n ? &(n->GetKey()) : 0;
  }
  template<typename K2>
  const K& FindGreatestSmallerEqual (const K2& other,
                                     const K& fallback) const
  {
    Node* n = RecursiveFindGSE<K2> (root.p, other);
    return n ? n->GetKey() : fallback;
  }
  //@}

  /// Delete all keys.
  void DeleteAll()
  {
    RecursiveDelete (root.p);
    root.p = 0;
  }
  /// Delete all the keys. (Idiomatic alias for DeleteAll().)
  void Empty() { DeleteAll(); }
  /// Returns whether this tree has no nodes.
  bool IsEmpty() const { return (root.p == 0); }

  //@{
  /// Traverse tree.
  template <typename CB>
  void TraverseInOrder (CB& callback) const
  {
    if (root.p != 0) RecursiveTraverseInOrder (root.p, callback);
  }
  //@}

  //@{
  /// Const iterator for tree
  class ConstIterator
  {
  public:
    /// Returns a boolean indicating whether or not the tree has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value.
    const K& Next ()
    {
      const K& ret = currentNode->GetKey();
      currentNode = Predecessor (currentNode);
      return ret;
    }

  protected:
    friend class csRedBlackTree;
    ConstIterator (const csRedBlackTree<K, Allocator, Ordering>* tree)
      : currentNode (tree->root.p)
    {
      while (currentNode && currentNode->right != 0)
        currentNode = currentNode->right;
    }

  private:
    const typename csRedBlackTree<K, Allocator, Ordering>::Node *currentNode;
  };
  friend class ConstIterator;

  /// Const reverse iterator for tree
  class ConstReverseIterator
  {
  public:
    /// Returns a boolean indicating whether or not the tree has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value.
    const K& Next ()
    {
      const K& ret = currentNode->GetKey();
      currentNode = Successor (currentNode);
      return ret;
    }

  protected:
    friend class csRedBlackTree;
    ConstReverseIterator (const csRedBlackTree<K, Allocator, Ordering>* tree)
      : currentNode (tree->root.p)
    {
      while (currentNode && currentNode->left != 0)
        currentNode = currentNode->left;
    }

  private:
    const typename csRedBlackTree<K, Allocator, Ordering>::Node *currentNode;
  };
  friend class ConstReverseIterator;

  /**
   * Get an iterator for iterating over the entire tree
   */
  ConstIterator GetIterator () const
  {
    return ConstIterator (this);
  }

  /**
   * Get an iterator for iterating over the entire tree
   */
  ConstReverseIterator GetReverseIterator () const
  {
    return ConstReverseIterator (this);
  }

  //@{
  /// Iterator for tree
  class Iterator
  {
  public:
    /// Returns a boolean indicating whether or not the tree has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value without advancing the iterator.
    K& PeekNext ()
    {
      K& ret = currentNode->GetKey();
      return ret;
    }

    /// Get the next element's value.
    K& Next ()
    {
      K& ret = currentNode->GetKey();
      currentNode = Predecessor (currentNode);
      return ret;
    }

  protected:
    friend class csRedBlackTree;
    Iterator (csRedBlackTree<K, Allocator, Ordering>* tree) : currentNode (tree->root.p)
    {
      while (currentNode && currentNode->GetRight() != 0)
        currentNode = currentNode->GetRight();
    }

  private:
    typename csRedBlackTree<K, Allocator, Ordering>::Node *currentNode;
  };
  friend class Iterator;

  /**
   * Get an iterator for iterating over the entire tree
   */
  Iterator GetIterator ()
  {
    return Iterator (this);
  }

  /**
   * Delete the 'next' element pointed at by the iterator.
   * \remarks Will repoint the iterator to the following element.
   */
  void Delete (Iterator& it)
  {
    Node* n = it.currentNode;
    if (n == 0) return;
    Node* nSucc = Successor (n);
    DeleteNode (n);
    Node* newNode;
    if (nSucc == 0)
    {
      newNode = root.p;
      if (newNode != 0)
      {
        while (newNode->GetRight() != 0)
	  newNode = newNode->GetRight();
      }
    }
    else
      newNode = Predecessor (nSucc);
    it.currentNode = newNode;
  }

  //@}
  
  /// Reverse iterator for tree
  class ReverseIterator
  {
  public:
    /// Returns a boolean indicating whether or not the tree has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value without advancing the iterator.
    K& PeekNext ()
    {
      K& ret = currentNode->GetKey();
      return ret;
    }

    /// Get the next element's value.
    K& Next ()
    {
      K& ret = currentNode->GetKey();
      currentNode = Successor (currentNode);
      return ret;
    }

  protected:
    friend class csRedBlackTree;
    ReverseIterator (csRedBlackTree<K, Allocator, Ordering>* tree) : currentNode (tree->root.p)
    {
      while (currentNode && currentNode->left != 0)
        currentNode = currentNode->left;
    }

  private:
    typename csRedBlackTree<K, Allocator, Ordering>::Node *currentNode;
  };
  friend class ReverseIterator;

  /**
   * Get an iterator for iterating over the entire tree
   */
  ReverseIterator GetReverseIterator ()
  {
    return ReverseIterator (this);
  }
};

/**
 * Helper template to allow storage of a payload together with a key in a
 * csRedBlackTree.
 */
template <typename K, typename T>
class csRedBlackTreePayload
{
  K key;
  T value;
public:
  csRedBlackTreePayload (const K& k, const T& v) : key(k), value(v) {}
  csRedBlackTreePayload (const csRedBlackTreePayload& other) :
    key(other.key), value(other.value) {}

  const K& GetKey() const { return key; }
  const T& GetValue() const { return value; }
  T& GetValue() { return value; }
  bool operator == (const csRedBlackTreePayload& other) const
  {
    return key == other.key;
  }
  bool operator == (const K& other) const
  {
    return key == other;
  }
  friend bool operator== (const K& a, const csRedBlackTreePayload& b)
  {
    return a == b.key;
  }
  bool operator <= (const csRedBlackTreePayload& other) const
  {
    return key <= other.key;
  }
  bool operator <= (const K& other) const
  {
    return key <= other;
  }
  friend bool operator <= (const K& k1, const csRedBlackTreePayload& k2)
  {
    return k1 <= k2.key;
  }
  operator const T&() const { return value; }
  operator T&() { return value; }
};

/**
 * Key-value-map, backed by csRedBlackTree.
 * \remark As with csRedBlackTree, every key must be unique.
 */
template <typename K, typename T,
          typename Allocator =
            csFixedSizeAllocator<
              sizeof(CS::Container::RedBlackTreeNode<
                      csRedBlackTreePayload<K, T> >)> >
class csRedBlackTreeMap : protected csRedBlackTree<csRedBlackTreePayload<K, T>, Allocator>
{
  typedef csRedBlackTree<csRedBlackTreePayload<K, T>, Allocator > supahclass;

  template<typename CB>
  class TraverseCB
  {
    CB callback;
  public:
    TraverseCB (const CB& callback) : callback(callback) {}
    void operator() (csRedBlackTreePayload<K, T>& value)
    {
      callback (value.GetKey(), value.GetValue());
    }
  };
public:
  enum { allocationUnitSize = supahclass::allocationUnitSize };

  csRedBlackTreeMap (const Allocator& alloc = Allocator())
   : supahclass (alloc) {}

  /**
   * Add element to map,
   * \return A pointer to the copy of the value stored in the tree, or 0 if the
   *  key already exists.
   */
  T* Put (const K& key, const T &value)
  {
    csRedBlackTreePayload<K, T>* payload = (csRedBlackTreePayload<K, T>*)
      Insert (csRedBlackTreePayload<K, T>(key, value));
    return (payload != 0) ? &payload->GetValue() :  0;
  }
  /**
   * Delete element from map,
   * \return Whether the deletion was successful. Fails if the key is not
   *  in the tree.
   */
  bool Delete (const K& key)
  {
    csRedBlackTreePayload<K, T>* payload = Find (key);
    if (payload == 0) return false;
    return supahclass::Delete (*payload);
  }
  //@{
  /**
   * Get a pointer to the element matching the given key, or 0 if there is
   * none.
   */
  const T* GetElementPointer (const K& key) const
  {
    csRedBlackTreePayload<K, T>* payload = Find (key);
    if (payload == 0) return 0;
    return &payload->GetValue();
  }
  T* GetElementPointer (const K& key)
  {
    csRedBlackTreePayload<K, T>* payload = Find (key);
    if (payload == 0) return 0;
    return &payload->GetValue();
  }
  //@}

  //@{
  /**
   * Get the element matching the given key, or \p fallback if there is none.
   */
  const T& Get (const K& key, const T& fallback) const
  {
    const csRedBlackTreePayload<K, T>* payload = Find (key);
    if (payload == 0) return fallback;
    return payload->GetValue();
  }
  T& Get (const K& key, T& fallback)
  {
    csRedBlackTreePayload<K, T>* payload = Find (key);
    if (payload == 0) return fallback;
    return payload->GetValue();
  }
  //@}
  /// Delete all keys.
  void DeleteAll() { supahclass::Empty(); }
  /// Delete all the keys. (Idiomatic alias for DeleteAll().)
  void Empty() { DeleteAll(); }
  /// Returns whether this map has no nodes.
  bool IsEmpty() const { return supahclass::IsEmpty(); }

  //@{
  /// Traverse tree.
  template <typename CB>
  void TraverseInOrder (CB& callback) const
  {
    TraverseCB<CB> traverser (callback);
    supahclass::TraverseInOrder (traverser);
  }
  //@}

  //@{
  /// Const iterator for map
  class ConstIterator
  {
  public:
    /// Returns a boolean indicating whether or not the map has more elements.
    bool HasNext () const
    {
      return treeIter.HasNext();
    }

    /// Get the next element's value.
    const T& Next (K& key)
    {
      const csRedBlackTreePayload<K, T>& d = treeIter.Next();
      key = d.GetKey ();
      return d.GetValue ();
    }

    /// Get the next element's value.
    const T& Next ()
    {
      const csRedBlackTreePayload<K, T>& d = treeIter.Next();
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    typename supahclass::ConstIterator treeIter;
    ConstIterator (const csRedBlackTreeMap* map)
      : treeIter (static_cast<const supahclass*> (map)->GetIterator())
    {
    }
  };
  friend class ConstIterator;

  /// Iterator for map
  class Iterator
  {
  public:
    /// Returns a boolean indicating whether or not the map has more elements.
    bool HasNext () const
    {
      return treeIter.HasNext();
    }

    /// Get the next element's value.
    T& Next (K& key)
    {
      csRedBlackTreePayload<K, T>& d = treeIter.Next();
      key = d.GetKey ();
      return d.GetValue ();
    }

    T& Next ()
    {
      csRedBlackTreePayload<K, T>& d = treeIter.Next();
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    typename supahclass::Iterator treeIter;
    Iterator (csRedBlackTreeMap* map)
      : treeIter (static_cast<supahclass*> (map)->GetIterator())
    {
    }
  };
  friend class Iterator;

  /// Const reverse iterator for map
  class ConstReverseIterator
  {
  public:
    /// Returns a boolean indicating whether or not the map has more elements.
    bool HasNext () const
    {
      return treeIter.HasNext();
    }

    /// Get the next element's value.
    const T& Next (K& key)
    {
      const csRedBlackTreePayload<K, T>& d = treeIter.Next();;
      key = d.GetKey ();
      return d.GetValue ();
    }

    /// Get the next element's value.
    const T& Next ()
    {
      const csRedBlackTreePayload<K, T>& d = treeIter.Next();;
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    typename supahclass::ConstReverseIterator treeIter;
    ConstReverseIterator (const csRedBlackTreeMap* map)
      : treeIter (static_cast<const supahclass*> (map)->GetReverseIterator())
    {
    }
  };
  friend class ConstReverseIterator;

  /// Reverse iterator for map
  class ReverseIterator
  {
  public:
    /// Returns a boolean indicating whether or not the map has more elements.
    bool HasNext () const
    {
      return treeIter.HasNext();
    }

    /// Get the next element's value.
    T& Next (K& key)
    {
      csRedBlackTreePayload<K, T>& d = treeIter.Next();;
      key = d.GetKey ();
      return d.GetValue ();
    }

    T& Next ()
    {
      csRedBlackTreePayload<K, T>& d = treeIter.Next();;
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    typename supahclass::ReverseIterator treeIter;
    ReverseIterator (csRedBlackTreeMap* map)
      : treeIter (static_cast<supahclass*> (map)->GetReverseIterator())
    {
    }
  };
  friend class ReverseIterator;

  /**
   * Get an iterator for iterating over the entire map
   */
  ConstIterator GetIterator () const
  {
    return ConstIterator (this);
  }

  /**
   * Get an iterator for iterating over the entire map
   */
  Iterator GetIterator ()
  {
    return Iterator (this);
  }

  /**
   * Get an iterator for iterating over the entire map
   */
  ConstReverseIterator GetReverseIterator () const
  {
    return ConstReverseIterator (this);
  }

  /**
   * Get an iterator for iterating over the entire map
   */
  ReverseIterator GetReverseIterator ()
  {
    return ReverseIterator (this);
  }
  //@}
};

/** @} */

#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# define new CS_EXTENSIVE_MEMDEBUG_NEW
#endif

#endif // __CS_UTIL_REDBLACKTREE_H__
