/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifndef __CS_UTIL_BINTREE_H__
#define __CS_UTIL_BINTREE_H__

/**\file
 * Generic binary search tree.
 */

#include "blockallocator.h"

/**
 * A generic binary search tree.
 */
template <class T> class csTree
{
private:
  struct Element
  {
    uint32 key;
    T value;
    Element *left, *right;

    inline Element () {}
    inline Element (uint32 key0, const T &value0,
      Element *left0 = 0, Element *right0 = 0)
    : key (key0), value (value0), left (left0), right (right0) {}
    inline Element (const Element &o)
    : key (o.key), value (o.value), left (o.left), right (o.right) {}
  };
  Element *Root;
  csBlockAllocator<Element> Pool;
  int BS;
  int Size;

  inline Element* New (uint32 key,
    Element *left = 0, Element *right = 0)
  {
    Element *e = Pool.Alloc ();
    e->key = key;
    e->left = left;
    e->right = right;
    return e;
  }

  inline Element* New (uint32 key, const T &value,
    Element *left = 0, Element *right = 0)
  {
    Element *e = Pool.Alloc ();
    e->key = key;
    e->value = value;
    e->left = left;
    e->right = right;
    return e;
  }

  void Copy (Element *dest, const Element *src)
  {
    dest->key = src->key;
    dest->value = src->value;
    if (src->left) Copy (dest->left = Pool.Alloc (), src->left);
    else dest->left = 0;
    if (src->right) Copy (dest->right = Pool.Alloc (), src->right);
    else dest->right = 0;
  }

  void DeleteTree (Element *e)
  {
    if (e->left) DeleteTree (e->left);
    if (e->right) DeleteTree (e->right);
    Pool.Free (e);
  }

  Element* Populate (const csArray<uint32> &keys, const csArray<T> &values)
  {
    int len = keys.Length ();
    if (! len) return 0;
    int mid = len / 2;
    return New (keys[mid], values[mid],
      Populate (keys.Section (0, mid-1), values.Section (0, mid-1)),
      Populate (keys.Section (mid+1, len-1), values.Section (mid+1, len-1)));
  }

  /// Pass to Traverse() a pointer to a function of this type.
  typedef void TraverseFunc (uint32 key, const T &value, void *some);

  void Traverse (const Element *e, TraverseFunc *func, void *some) const
  {
    func (e->key, e->value, some);
    if (e->left) Traverse (e->left, func, some);
    if (e->right) Traverse (e->right, func, some);
  }

public:
  /**
   * Construct a tree that allocates memory for nodes
   * in blocks of the given size.
   */
  csTree (int blocksize = 15)
    : Pool (blocksize), BS (blocksize), Size (0), Root (0) {}

  /// Copy constructor.
  csTree (const csTree<T> &other)
    : Pool (other.BS), BS (other.BS), Size (other.Size)
  {
    if (other.Root) Copy (Root = Pool.Alloc (), other.Root);
    else Root = 0;
  }

  /// Populate an empty tree with entries from a sorted array.
  void PopulateWith (const csArray<uint32> &keys, const csArray<T> &values)
  {
    CS_ASSERT (! Root);
    CS_ASSERT (keys.Length () == values.Length ());
    Root = Populate (keys, values);
  }

  /// Call a callback function once for every entry in the tree.
  void Traverse (TraverseFunc *func, void *some) const
  {
    if (Root) Traverse (Root, func, some);
  }

  /// Put an entry into the tree, overwriting if the key already exists.
  void Put (uint32 key, const T &value)
  {
    if (! Root)
    {
      Root = New (key, value);
      Size++;
      return;
    }
    Element *e = Root;
    while (true)
      if (key < e->key)
      {
        if (e->left) e = e->left;
        else
        {
          e->left = New (key, value);
          Size++;
          return;
        }
      }
      else if (e->key < key)
      {
        if (e->right) e = e->right;
        else
        {
          e->right = New (key, value);
          Size++;
          return;
        }
      }
      else
      {
        e->value = value;
        return;
      }
  }

  /// Find the given entry in the tree, creating it if it doesn't exist.
  T& Find (uint32 key)
  {
    if (! Root)
    {
      Root = New (key);
      Size++;
      return Root->value;
    }
    Element *e = Root;
    while (true)
      if (key < e->key)
      {
        if (e->left) e = e->left;
        else
        {
          e->left = New (key);
          Size++;
          return e->left->value;
        }
      }
      else if (e->key < key)
      {
        if (e->right) e = e->right;
        else
        {
          e->right = New (key);
          Size++;
          return e->right->value;
        }
      }
      else return e->value;
  }

  /// Find the given entry in the tree, returning 0 if it doesn't exist,
  const T& Find (uint32 key) const
  {
    static const T zero (0);
    if (! Root) return zero;
    Element *e = Root;
    while (true)
      if (key < e->key)
      {
        if (e->left) e = e->left;
        else return zero;
      }
      else if (e->key < key)
      {
        if (e->right) e = e->right;
        else return zero;
      }
      else return e->value;
  }

  /// Delete the given entry from the tree, returning false if it doesn't exist.
  bool Delete (uint32 key)
  {
    if (! Root) return false;
    if (Root->key == key)
    {
      Pool.Free (Root);
      Root = 0;
      return true;
    }

    bool left;
    Element *old, *e = Root;
    while (true)
      if (key < e->key)
      {
        if (e->left) e = e->left;
        else return false;
        left = true;
        old = e;
      }
      else if (e->key < key)
      {
        if (e->right) e = e->right;
        else return false;
        left = false;
        old = e;
      }
      else
      {
        if (e->left)
        {
          if (e->right)
          {
            Element *promoteold = e, *promote = e->right;
            while (promote->left)
            {
              promoteold = promote;
              promote = promote->left;
            }
            promoteold->left = 0;
            e->key = promote->key;
            e->value = promote->value;
            Pool.Free (promote);
          }
          else
          {
            if (left) old->left = e->left;
            else old->right = e->left;
            Pool.Free (e);
          }
        }
        else
        {
          if (e->right)
          {
            if (left) old->left = e->right;
            else old->right = e->right;
          }
          else
          {
            if (left) old->left = 0;
            else old->right = 0;
          }
          Pool.Free (e);
        }
        Size--;
        return true;
      }
  }

  /// Delete all the entries from the tree.
  bool DeleteAll ()
  {
    if (! Root) return false;
    DeleteTree (Root);
    return true;
  }

  /// Get the number of entries in the tree.
  int GetSize () const
  {
    return Size;
  }
};

#endif
