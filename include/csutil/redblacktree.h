/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter
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
#include "csutil/comparator.h"

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

/**
 * A red-black-tree.
 * \remark Does not allow duplicate keys.
 * \remark Uses csComparator<> for key comparisons.
 * \remark Only stores keys. If you need a key-value-map, look at 
 *  csRedBlackTreeMap.
 */
template <typename K>
class csRedBlackTree
{
protected:
  enum NodeColor { Black = 0, Red = 1 };
  /// A node in the tree
  struct Node
  {
  private:
    /// Pointer to parent, but also stores color in LSB.
    Node* parent;
  public:
    Node* left;
    Node* right;
    uint8 key[sizeof(K)];
    
    Node() : parent(0) {}
    ~Node() { ((K*)&key)->~K(); }
    inline Node* GetParent() const
    { return (Node*)((uintptr_t)parent & (uintptr_t)~1); }
    void SetParent(Node* p)
    { parent = (Node*)(((uintptr_t)p & (uintptr_t)~1) | (uint)GetColor()); }
    NodeColor GetColor() const
    { // Expression split over two statements to pacify some broken gcc's which
      // barf saying "can't convert Node* to NodeColor".
      uintptr_t const v = ((uintptr_t)parent & 1); 
      return (NodeColor)v;
    }
    void SetColor (NodeColor color)
    { parent = (Node*)(((uintptr_t)parent & (uintptr_t)~1) | (uint)color); }
  };
  csBlockAllocator<Node, CS::Memory::AllocatorAlign<2> > nodeAlloc;
  
  Node* root;
  
  /// Locate the place where a new node needs to be inserted.
  Node* RecursiveInsert (Node* parent, Node*& node, const K& key)
  {
    if (node == 0)
    {
      node = nodeAlloc.Alloc();
      node->SetParent (parent);
      node->left = node->right = 0;
      new ((K*)&node->key) K (key);
      node->SetColor (Red);
      return node;
    }
    else
    {
      int r = csComparator<K, K>::Compare (key, *((K*)&node->key));
      if (r < 0)
	return RecursiveInsert (node, node->left, key);
      else
	return RecursiveInsert (node, node->right, key);
    }
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
      root = pivotReplace;
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
      root = pivotReplace;
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
  { return (node == 0) || (node->GetColor() == Black); }
  /// Check whether a node is red.
  bool IsRed (Node* node) const
  { return (node != 0) && (node->GetColor() == Red); }
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
	  p->SetColor (Black);
	  y->SetColor (Black);
	  pp->SetColor (Red);
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
	  p->SetColor (Black);
	  pp->SetColor (Red);
	  RotateRight (pp);
	}
      }
      else
      {
	Node* y = pp->left;
	if (IsRed (y))
	{
	  // Uncle of 'node' is red
	  p->SetColor (Black);
	  y->SetColor (Black);
	  pp->SetColor (Red);
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
	  p->SetColor (Black);
	  pp->SetColor (Red);
	  RotateLeft (pp);
	}
      }
    }
    root->SetColor (Black);
  }
  
  /// Delete a node from the tree.
  void DeleteNode (Node* node)
  {
    Node* y; // Node we will replace 'node' with
    if ((node->left == 0) || (node->right == 0))
      y = node;
    else
      y = Successor (node);
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
      root = x;
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
      *((K*)&node->key) = *((K*)&y->key);
    }
    if (y->GetColor() == Black)
      DeleteFixup (x, nilParent);
    nodeAlloc.Free (y);
  }
  /// Fix up the RB tree after a deletion.
  void DeleteFixup (Node* node, Node* nilParent)
  {
    while ((node != root) && IsBlack (node))
    {
      Node* p = node ? node->GetParent() : nilParent;
      if (node == p->left)
      {
	Node* w = p->right;
	if (IsRed (w))
	{
	  w->SetColor (Black);
	  p->SetColor (Red);
	  RotateLeft (p);
	  p = node ? node->GetParent() : nilParent;
	  w = p->right;
	}
	if (IsBlack (w->left) && IsBlack (w->right))
	{
	  w->SetColor (Red);
	  node = p;
	}
	else
	{
	  if (IsBlack (w->right))
	  {
	    w->left->SetColor (Red);
	    w->SetColor (Red);
	    RotateRight (w);
	    p = node ? node->GetParent() : nilParent;
	    w = p->right;
	  }
	  w->SetColor (p->GetColor ());
	  p->SetColor (Black);
	  w->right->SetColor (Black);
	  RotateLeft (p);
	  node = root;
	}
      }
      else
      {
	Node* w = p->left;
	if (IsRed (w))
	{
	  w->SetColor (Black);
	  p->SetColor (Red);
	  RotateRight (p);
	  p = node ? node->GetParent() : nilParent;
	  w = p->left;
	}
	if (IsBlack (w->left) && IsBlack (w->right))
	{
	  w->SetColor (Red);
	  node = p;
	}
	else
	{
	  if (IsBlack (w->left))
	  {
	    w->right->SetColor (Red);
	    w->SetColor (Red);
	    RotateLeft (w);
	    p = node ? node->GetParent() : nilParent;
	    w = p->left;
	  }
	  w->SetColor (p->GetColor ());
	  p->SetColor (Black);
	  w->left->SetColor (Black);
	  RotateRight (p);
	  node = root;
	}
      }
    }
    if (node != 0) node->SetColor (Black);
  }
  /// Find the node for a key.
  Node* LocateNode (Node* node, const K& key) const
  {
    if (node == 0) return 0;
      
    int r = csComparator<K, K>::Compare (key, *((K*)&node->key));
    if (r == 0) 
      return node;
    else if (r < 0)
      return LocateNode (node->left, key);
    else
      return LocateNode (node->right, key);
  }
  /// Find the node which has a given instance of a key
  Node* LocateNodeExact (Node* node, const K* key) const
  {
    if (node == 0) return 0;
      
    if (key == (K*)&node->key) return node;
    int r = csComparator<K, K>::Compare (*key, *((K*)&node->key));
    if (r == 0)
    {
      // @@@ Should that be really necessary?
      Node* n = LocateNodeExact (node->left, key);
      if (n != 0) return n;
      return LocateNodeExact (node->right, key);
    }
    else if (r < 0)
      return LocateNodeExact (node->left, key);
    else
      return LocateNodeExact (node->right, key);
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
  /// Locate key that is equal to 'other'.
  template<typename K2>
  const K* RecursiveFind (Node* node, const K2& other) const
  {
    if (node == 0) return 0;
    int r = csComparator<K2, K>::Compare (other, *((K*)&node->key));
    if (r == 0)
      return ((K*)&node->key);
    else if (r < 0)
      return RecursiveFind<K2> (node->left, other);
    else
      return RecursiveFind<K2> (node->right, other);
  }
  template<typename K2>
  K* RecursiveFind (Node* node, const K2& other)
  {
    if (node == 0) return 0;
    int r = csComparator<K2, K>::Compare (other, *((K*)&node->key));
    if (r == 0)
      return ((K*)&node->key);
    else if (r < 0)
      return RecursiveFind<K2> (node->left, other);
    else
      return RecursiveFind<K2> (node->right, other);
  }
  template<typename K2>
  const K& RecursiveFind (Node* node, const K2& other, const K& fallback) const
  {
    if (node == 0) return fallback;
    int r = csComparator<K2, K>::Compare (other, *((K*)&node->key));
    if (r == 0)
      return *((K*)&node->key);
    else if (r < 0)
      return RecursiveFind<K2> (node->left, other, fallback);
    else
      return RecursiveFind<K2> (node->right, other, fallback);
  }
  template<typename K2>
  K& RecursiveFind (Node* node, const K2& other, K& fallback)
  {
    if (node == 0) return fallback;
    int r = csComparator<K2, K>::Compare (other, *((K*)&node->key));
    if (r == 0)
      return *((K*)&node->key);
    else if (r < 0)
      return RecursiveFind (node->left, other, fallback);
    else
      return RecursiveFind (node->right, other, fallback);
  }
  //@}
  
  //@{
  /// Locate key that is equal to 'other'.
  template<typename K2>
  const K* RecursiveFindSGE (Node* node, const K2& other) const
  {
    if (node == 0) return 0;
    int r = csComparator<K2, K>::Compare (other, *((K*)&node->key));
    if (r == 0)
      return ((K*)&node->key);
    if (r < 0)
    {
      const K* x = RecursiveFindSGE<K2> (node->left, other);
      /* This node is currently the smallest known greater or equal to
       * 'other', so return that if a search in the left subtree does
       * not give a result */
      return x ? x : ((K*)&node->key);
    }
    else
      return RecursiveFindSGE<K2> (node->right, other);
  }
  template<typename K2>
  const K& RecursiveFindSGE (Node* node, const K2& other, const K& fallback) const
  {
    if (node == 0) return 0;
    int r = csComparator<K2, K>::Compare (other, *((K*)&node->key));
    if (r == 0)
      return *((K*)&node->key);
    if (r < 0)
    {
      const K& x = RecursiveFindSGE<K2> (node->left, other);
      /* This node is currently the smallest known greater or equal to
       * 'other', so return that if a search in the left subtree does
       * not give a result */
      return x ? x : *((K*)&node->key);
    }
    else
      return RecursiveFindSGE<K2> (node->right, other);
  }
  //@}
  
  /// Traverse tree.
  template <typename CB>
  void RecursiveTraverseInOrder (Node* node, CB& callback) const
  {
    if (node->left != 0) RecursiveTraverseInOrder (node->left, callback);
    callback.Process (*((K*)&node->key));
    if (node->right != 0) RecursiveTraverseInOrder (node->right, callback);
  }

  //@{
  /// Locate key that is equal to 'other'
  template<typename K2>
  K* Find (const K2& other)
  {
    return RecursiveFind<K2> (root, other);
  }
  template<typename K2>
  K& Find (const K2& other, K& fallback)
  {
    return RecursiveFind<K2> (root, other, fallback);
  }
  //@}

  /// Duplicate a subtree
  void RecursiveCopy (Node*& to, Node* parent, const Node* from)
  {
    if (from == 0)
    {
      to = 0; 
      return;
    }
    to = nodeAlloc.Alloc();
    to->SetParent (parent);
    to->SetColor (from->GetColor());
    new ((K*)&to->key) K (*((K*)&from->key));
    RecursiveCopy (to->left, to, from->left);
    RecursiveCopy (to->right, to, from->right);
  }
public:
  /**
   * Construct a new tree.
   * \param allocatorBlockSize Block size in bytes used by the internal block
   *  allocator for nodes.
   */
  csRedBlackTree (size_t allocatorBlockSize = 4096) : 
    nodeAlloc (allocatorBlockSize / sizeof(Node)), root(0) { }
  csRedBlackTree (const csRedBlackTree& other) : 
    nodeAlloc (other.nodeAlloc.GetBlockElements())
  {
    RecursiveCopy (root, 0, other.root);
  }

  /**
   * Insert a key.
   * \return A pointer to the copy of the key stored in the tree, or 0 if the
   *  key already exists.
   */
  const K* Insert (const K& key)
  {
    Node* n = RecursiveInsert (0, root, key);
    if (n == 0) return 0;
    InsertFixup (n);
    return (K*)&n->key;
  }
  /**
   * Delete a key.
   * \return Whether the deletion was successful. Fails if the key is not 
   *  in the tree.
   */
  bool Delete (const K& key)
  {
    Node* n = LocateNode (root, key);
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
    Node* n = LocateNodeExact (root, key);
    if (n == 0) return false;
    DeleteNode (n);
    return true;
  }
  /// Check whether a key is in the tree.
  bool In (const K& key) const
  {
    return (LocateNode (root, key) != 0);
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
    return RecursiveFind<K2> (root, other);
  }
  template<typename K2>
  const K& Find (const K2& other, const K& fallback) const
  {
    return RecursiveFind<K2>  (root, other, fallback);
  }
  //@}
  
  //@{
  /// Locate smallest key greater or equal to \a other
  template<typename K2>
  const K* FindSmallestGreaterEqual (const K2& other) const
  {
    return RecursiveFindSGE<K2> (root, other);
  }
  template<typename K2>
  const K& FindSmallestGreaterEqual (const K2& other, 
                                     const K& fallback) const
  {
    return RecursiveFindSGE<K2>  (root, other, fallback);
  }
  //@}
  
  /// Delete all keys.
  void DeleteAll()
  {
    nodeAlloc.Empty();
    root = 0;
  }
  /// Delete all the keys. (Idiomatic alias for DeleteAll().)
  void Empty() { DeleteAll(); }
  /// Returns whether this tree has no nodes.
  bool IsEmpty() const { return (root == 0); }

  //@{
  /// Traverse tree.
  template <typename CB>
  void TraverseInOrder (CB& callback) const
  {
    if (root != 0) RecursiveTraverseInOrder (root, callback);
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
      const K& ret = *((const K*)&currentNode->key);
      currentNode = Successor (currentNode);
      return ret;
    }

  protected:
    friend class csRedBlackTree;
    ConstIterator (const csRedBlackTree<K>* tree)
      : currentNode (tree->root)
    {
      while (currentNode && currentNode->left != 0)
        currentNode = currentNode->left;
    }

  private:
    const typename csRedBlackTree<K>::Node *currentNode;
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
      const K& ret = *((const K*)&currentNode->key);
      currentNode = Predecessor (currentNode);
      return ret;
    }

  protected:
    friend class csRedBlackTree;
    ConstReverseIterator (const csRedBlackTree<K>* tree)
      : currentNode (tree->root)
    {
      while (currentNode && currentNode->right != 0)
        currentNode = currentNode->right;
    }

  private:
    const typename csRedBlackTree<K>::Node *currentNode;
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
  ConstReverseIterator GetReverseIterator ()
  {
    return ConstReverseIterator (this);
  }

  //@{
  /// Const iterator for tree
  class Iterator
  {
  public:
    /// Returns a boolean indicating whether or not the tree has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value without advancing the iterator.
    const K& PeekNext ()
    {
      const K& ret = *((const K*)&currentNode->key);
      return ret;
    }

    /// Get the next element's value.
    const K& Next ()
    {
      const K& ret = *((const K*)&currentNode->key);
      currentNode = Successor (currentNode);
      return ret;
    }

  protected:
    friend class csRedBlackTree;
    Iterator (csRedBlackTree<K>* tree) : currentNode (tree->root)
    {
      while (currentNode && currentNode->left != 0)
        currentNode = currentNode->left;
    }

  private:
    typename csRedBlackTree<K>::Node *currentNode;
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
    Node* nPred = Predecessor (n);
    DeleteNode (n);
    Node* newNode;
    if (nPred == 0)
    {
      newNode = root;
      if (newNode != 0)
      {
        while (newNode->left != 0) newNode = newNode->left;
      }
    }
    else
      newNode = Successor (nPred);
    it.currentNode = newNode;
  }

  //@}
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
  bool operator < (const csRedBlackTreePayload& other) const
  {
    return (csComparator<K, K>::Compare (key, other.key) < 0);
  }
  bool operator < (const K& other) const
  {
    return (csComparator<K, K>::Compare (key, other) < 0);
  }
  friend bool operator < (const K& k1, const csRedBlackTreePayload& k2)
  {
    return (csComparator<K, K>::Compare (k1, k2.key) < 0);
  }
  operator const T&() const { return value; }
  operator T&() { return value; }
};

/**
 * Key-value-map, backed by csRedBlackTree.
 * \remark As with csRedBlackTree, every key must be unique.
 */
template <typename K, typename T>
class csRedBlackTreeMap : protected csRedBlackTree<csRedBlackTreePayload<K, T> >
{
  typedef csRedBlackTree<csRedBlackTreePayload<K, T> > supahclass;

  template<typename CB>
  class TraverseCB
  {
    CB callback;
  public:
    TraverseCB (const CB& callback) : callback(callback) {}
    void Process (csRedBlackTreePayload<K, T>& value)
    {
      callback.Process (value.GetKey(), value.GetValue());
    }
  };
public:
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
      return currentNode != 0;
    }

    /// Get the next element's value.
    const T& Next (K& key)
    {
      const csRedBlackTreePayload<K, T>& d = *((const csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Successor (currentNode);
      key = d.GetKey ();
      return d.GetValue ();
    }

    /// Get the next element's value.
    const T& Next ()
    {
      const csRedBlackTreePayload<K, T>& d = *((const csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Successor (currentNode);      
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    ConstIterator (const csRedBlackTreeMap<K, T>* tree)
      : currentNode (tree->root)
    {
      while (currentNode && currentNode->left != 0)
        currentNode = currentNode->left;
    }

  private:
    const typename csRedBlackTreeMap<K, T>::Node *currentNode;
  };
  friend class ConstIterator;

  /// Iterator for map
  class Iterator
  {
  public:
    /// Returns a boolean indicating whether or not the map has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value.
    T& Next (K& key)
    {
      csRedBlackTreePayload<K, T>& d = *((csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Successor (currentNode);
      key = d.GetKey ();
      return d.GetValue ();
    }

    T& Next ()
    {
      csRedBlackTreePayload<K, T>& d = *((csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Successor (currentNode);
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    Iterator (csRedBlackTreeMap<K, T>* tree)
      : currentNode (tree->root)
    {
      while (currentNode && currentNode->left != 0)
        currentNode = currentNode->left;
    }

  private:
    typename csRedBlackTreeMap<K, T>::Node *currentNode;
  };
  friend class Iterator;

  /// Const reverse iterator for map
  class ConstReverseIterator
  {
  public:
    /// Returns a boolean indicating whether or not the map has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value.
    const T& Next (K& key)
    {
      const csRedBlackTreePayload<K, T>& d = *((const csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Predecessor (currentNode);
      key = d.GetKey ();
      return d.GetValue ();
    }

    /// Get the next element's value.
    const T& Next ()
    {
      const csRedBlackTreePayload<K, T>& d = *((const csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Predecessor (currentNode);      
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    ConstReverseIterator (const csRedBlackTreeMap<K, T>* tree)
      : currentNode (tree->root)
    {
      while (currentNode->right != 0)
        currentNode = currentNode->right;
    }

  private:
    const typename csRedBlackTreeMap<K, T>::Node *currentNode;
  };
  friend class ConstReverseIterator;

  /// Reverse iterator for map
  class ReverseIterator
  {
  public:
    /// Returns a boolean indicating whether or not the map has more elements.
    bool HasNext () const
    {
      return currentNode != 0;
    }

    /// Get the next element's value.
    T& Next (K& key)
    {
      csRedBlackTreePayload<K, T>& d = *((csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Predecessor (currentNode);
      key = d.GetKey ();
      return d.GetValue ();
    }

    T& Next ()
    {
      csRedBlackTreePayload<K, T>& d = *((csRedBlackTreePayload<K, T>*)&currentNode->key);
      currentNode = Predecessor (currentNode);
      return d.GetValue ();
    }

  protected:
    friend class csRedBlackTreeMap;
    ReverseIterator (csRedBlackTreeMap<K, T>* tree)
      : currentNode (tree->root)
    {
      while (currentNode && currentNode->right != 0)
        currentNode = currentNode->right;
    }

  private:
    typename csRedBlackTreeMap<K, T>::Node *currentNode;
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
