/*
    Copyright (C) 2003 by Marten Svanfeldt
    influenced by Aapl by Adrian Thurston <adriant@ragel.ca>

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

#ifndef __CS_UTIL_LIST_H__
#define __CS_UTIL_LIST_H__

/**\file
 * Double-linked list
 */

#include "csextern.h"

#include "csutil/allocator.h"

/**
 * A lightweight double-linked list template.  Copies the elements into the
 * list for storages.  Assumes that type T supports copy construction.
 */
template <class T, class MemoryAllocator = CS::Memory::AllocatorMalloc>
class csList
{
protected:
  /**
   * Template which describes the data stored in the linked list
   * For example a list of ints uses ListElement<int>.
   */
  struct ListElement
  {
    /// Use specified data
    ListElement(const T& d, ListElement* newnext, ListElement* newprev) :
      next(newnext), prev(newprev), data(d) {}

    /// Next element in list. If this is the last one, then next is 0
    ListElement* next;

    /// Previous element in list. If this is the first one, prev is 0
    ListElement* prev;

    /// Stored data
    T data;
  };

  /// Remove specific item by explicit ref
  void Delete (ListElement *el);

  void* AllocElement ()
  {
    return head.Alloc (sizeof (ListElement));
  }
  void FreeElement (ListElement* el)
  {
    el->~ListElement();
    head.Free (el);
  }
public:
  /**
   * This is the size of the memory block the wrapper list uses to
   * store the actual data. It is published to make fixed-size allocators
   * possible.
   */
  static const size_t allocSize = sizeof (ListElement);

  /// Default constructor
  csList() : head ((ListElement*)0), tail(0) {}

  /// Construct with allocator setup
  csList (const MemoryAllocator& alloc) : head (alloc, (ListElement*)0), 
    tail(0) {}

  /// Copy constructor
  csList(const csList<T, MemoryAllocator> &other);

  /// Destructor
  ~csList()
  { DeleteAll (); }

  /// Iterator for the list
  class Iterator
  {
  public:
    /// Constructor
    Iterator() : ptr(0), visited(false), reversed(false)
    { }
    /// Copy constructor
    Iterator(const Iterator& r)
    { ptr = r.ptr; visited = r.visited; reversed = r.reversed; }
    /// Constructor.
    Iterator(const csList<T, MemoryAllocator> &list, bool reverse = false) :
      visited(false), reversed(reverse)
    {
      if (reverse) ptr = list.tail;
      else ptr = list.head.p;
    }
    /// Assignment operator
    Iterator& operator= (const Iterator& r)
    { ptr = r.ptr; visited = r.visited; reversed = r.reversed; return *this; }
    /// Test if the Iterator is set to a valid element.
    bool HasCurrent() const
    { return visited && ptr != 0; }
    /// Test if there is a next element.
    bool HasNext() const
    { return ptr && (ptr->next || !visited); }
    /// Test if there is a previous element.
    bool HasPrevious() const
    { return ptr && (ptr->prev || !visited); }
    /// Test if the Iterator is set to the first element.
    bool IsFirst() const
    { return ptr && ptr->prev == 0; }
    /// Test if the Iterator is set to the last element.
    bool IsLast() const
    { return ptr && ptr->next == 0; }
    /// Test if the iterator is reversed.
    bool IsReverse() const
    { return reversed; }

    /// Cast operator.
    operator T*() const
    { return visited && ptr ? &ptr->data : 0; }
    /// Dereference operator (*).
    T& operator *() const
    { CS_ASSERT(ptr != 0); return ptr->data; }
    /// Dereference operator (->).
    T* operator->() const
    { return visited && ptr ? &ptr->data : 0; }

    /// Set iterator to non-existent element. HasCurrent() will return false.
    void Clear ()
    {
      ptr = 0;
      visited = true;
    }
    /// Advance to next element and return it.
    T& Next ()
    {
      if (visited && ptr != 0)
        ptr = ptr->next;
      visited = true;
      CS_ASSERT(ptr != 0);
      return ptr->data;
    }
    /// Backup to previous element and return it.
    T& Previous()
    {
      if (visited && ptr != 0)
        ptr = ptr->prev;
      visited = true;
      CS_ASSERT(ptr != 0);
      return ptr->data;
    }
    T& Prev() { return Previous(); } // Backward compatibility.

    /// Advance to next element and return it.
    Iterator& operator++()
    {
      if (visited && ptr != 0)
        ptr = ptr->next;
      visited = true;
      return *this;
    }
    /// Backup to previous element and return it.
    Iterator& operator--()
    {
      if (visited && ptr != 0)
        ptr = ptr->prev;
      visited = true;
      return *this;
    }

    /**
     * Return current element.
     * Warning! Assumes there is a current element!
     */
    T& FetchCurrent () const
    {
      CS_ASSERT(visited && ptr != 0);
      return ptr->data;
    }
    /**
     * Return next element but don't modify iterator.
     * Warning! Assumes there is a next element!
     */
    T& FetchNext () const
    {
      CS_ASSERT(ptr != 0);
      return visited ? ptr->next->data : ptr->data;
    }
    /**
     * Return previous element but don't modify iterator.
     * Warning! Assumes there is a previous element!
     */
    T& FetchPrevious () const
    {
      CS_ASSERT(ptr != 0);
      return visited ? ptr->prev->data : ptr->data;
    }
    T& FetchPrev () const { return FetchPrevious(); } // Backward compat.

  protected:
    friend class csList<T, MemoryAllocator>;
    Iterator (ListElement* element, bool visit = true, bool rev = false) :
      ptr(element), visited(visit), reversed(rev)
    {}

  private:
    ListElement* ptr;
    bool visited;
    bool reversed;
  };

  /// Assignment, deep-copy
  csList& operator=(const csList<T, MemoryAllocator>& other);

  /// Add an item first in list. Copy T into the listdata.
  Iterator PushFront (const T& item);

  /// Add an item last in list. Copy T into the listdata.
  Iterator PushBack (const T& item);

  /// Insert an item before the item the iterator is set to.
  void InsertBefore(Iterator& it, const T& item);

  /// Insert an item after the item the iterator is set to.
  void InsertAfter(Iterator& it, const T& item);

  /**
   * Move an item (as iterator \a item) before the item the iterator \a it is 
   * set to.
   */
  void MoveBefore(const Iterator& it, const Iterator& item);
  /// Move an item (as iterator \a item) to the front of the list.
  void MoveToFront (const Iterator& item);

  /** 
   * Move an item (as iterator \a item ) after the item the iterator \a it is 
   * set to.
   */
  void MoveAfter(const Iterator& it, const Iterator& item);
  /// Move an item (as iterator \a item) to the front of the list.
  void MoveToBack (const Iterator& item);

  /// Remove specific item by iterator.
  void Delete (Iterator& it);
  
  /**
   * Remove specified item.
   * \remarks Slow!
   */
  bool Delete (const T& item)
  {
    ListElement* e = head.p;
    while (e != 0)
    {
      if (e->data == item)
      {
	Delete (e);
	return true;
      }
      e = e->next;
    }
    return false;
  }

  /// Empty an list.
  void DeleteAll();

  /// Return first element of the list.
  T& Front () const
  { return head.p->data; }
  /// Return last element of the list.
  T& Last () const
  { return tail->data; }

  /// Deletes the first element of the list.
  bool PopFront ()
  {
    if (!head.p)
      return false;
    Delete (head.p);
    return true;
  }

  /// Deletes the last element of the list
  bool PopBack ()
  {
    if (!tail)
      return false;
    Delete (tail);
    return true;
  }
  
  bool IsEmpty () const
  {
    CS_ASSERT((head.p == 0 && tail == 0) || (head.p !=0 && tail != 0));
    return head.p == 0;
  }

private:
  friend class Iterator;
  CS::Memory::AllocatorPointerWrapper<ListElement, MemoryAllocator> head;
  ListElement* tail;
};

/// Deep copy of list
template <class T, class MemoryAllocator>
inline csList<T, MemoryAllocator>::csList(
  const csList<T, MemoryAllocator> &other) : head((ListElement*)0), tail(0)
{
  ListElement* e = other.head.p;
  while (e != 0)
  {
    PushBack (e->data);
    e = e->next;
  }
}

/// Assignment, deep-copy
template <class T, class MemoryAllocator>
inline csList<T, MemoryAllocator>& csList<T, MemoryAllocator>::operator= (
  const csList<T, MemoryAllocator> &other)
{
  DeleteAll ();
  ListElement* e = other.head.p;
  while (e != 0)
  {
    PushBack (e->data);
    e = e->next;
  }
  return *this;
}

/// Delete all elements
template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::DeleteAll ()
{
  ListElement *cur = head.p, *next = 0;
  while (cur != 0)
  {
    next = cur->next;
    FreeElement (cur);
    cur = next;
  }
  head.p = tail = 0;
}

#include "csutil/custom_new_disable.h"

/// Add one item last in the list
template <class T, class MemoryAllocator>
inline typename csList<T, MemoryAllocator>::Iterator 
  csList<T, MemoryAllocator>::PushBack (const T& e)
{
  ListElement* el = new (AllocElement()) ListElement (e, 0, tail);
  if (tail)
    tail->next = el;
  else
    head.p = el;
  tail = el;
  return Iterator(el);
}

/// Add one item first in the list
template <class T, class MemoryAllocator>
inline typename csList<T, MemoryAllocator>::Iterator 
  csList<T, MemoryAllocator>::PushFront (const T& e)
{
  ListElement* el = new (AllocElement()) ListElement (e, head.p, 0);
  if (head.p)
    head.p->prev = el;
  else
    tail = el;
  head.p = el;
  return Iterator (el);
}

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::InsertAfter (Iterator &it, 
  const T& item)
{
  CS_ASSERT(it.HasCurrent());
  ListElement* el = it.ptr;
  ListElement* next = el->next;
  ListElement* prev = el;
  ListElement* newEl = new (AllocElement()) ListElement (item, next, 
    prev);
  if (!next) // this is the last element
    tail = newEl;
  else
    el->next->prev = newEl;
  el->next = newEl;
}

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::InsertBefore (Iterator &it, 
  const T& item)
{
  CS_ASSERT(it.HasCurrent());
  ListElement* el = it.ptr;
  ListElement* next = el;
  ListElement* prev = el->prev;
  ListElement* newEl = new (AllocElement()) ListElement (item, next, prev);
  if (!prev) // this is the first element
    head.p = newEl;
  else
    el->prev->next = newEl;
  el->prev = newEl;
}

#include "csutil/custom_new_enable.h"

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::MoveAfter (const Iterator &it, 
  const Iterator &item)
{
  CS_ASSERT(item.HasCurrent());
  ListElement* el_item = item.ptr;

  // Unlink the item.
  if (el_item->prev)
    el_item->prev->next = el_item->next;
  else
    head.p = el_item->next;
  if (el_item->next)
    el_item->next->prev = el_item->prev;
  else
    tail = el_item->prev;

  CS_ASSERT(it.HasCurrent());
  ListElement* el = it.ptr;
  ListElement* next = el->next;
  ListElement* prev = el;

  el_item->next = next;
  el_item->prev = prev;
  if (!next) // this is the last element
    tail = el_item;
  else
    el->next->prev = el_item;
  el->next = el_item;
}

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::MoveToBack (const Iterator &item)
{
  CS_ASSERT(item.HasCurrent());
  ListElement* el_item = item.ptr;

  if (!el_item->next)
    // Already at back.
    return;

  ListElement* el = tail;
  ListElement* prev = el;

  // Unlink the item.
  if (el_item->prev)
    el_item->prev->next = el_item->next;
  else
    head.p = el_item->next;
  el_item->next->prev = el_item->prev;

  el_item->next = 0;
  el_item->prev = prev;
  tail = el_item;
  el->next = el_item;
}

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::MoveBefore (const Iterator &it, 
  const Iterator &item)
{
  CS_ASSERT(item.HasCurrent());
  ListElement* el_item = item.ptr;

  // Unlink the item.
  if (el_item->prev)
    el_item->prev->next = el_item->next;
  else
    head.p = el_item->next;
  if (el_item->next)
    el_item->next->prev = el_item->prev;
  else
    tail = el_item->prev;

  CS_ASSERT(it.HasCurrent());
  ListElement* el = it.ptr;
  ListElement* next = el;
  ListElement* prev = el->prev;

  el_item->next = next;
  el_item->prev = prev;
  if (!prev) // this is the first element
    head.p = el_item;
  else
    el->prev->next = el_item;
  el->prev = el_item;
}

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::MoveToFront (const Iterator &item)
{
  CS_ASSERT(item.HasCurrent());
  ListElement* el_item = item.ptr;

  if (!el_item->prev)
    // Already at front.
    return;

  ListElement* el = head.p;
  ListElement* next = el;

  // Unlink the item.
  el_item->prev->next = el_item->next;
  if (el_item->next)
    el_item->next->prev = el_item->prev;
  else
    tail = el_item->prev;

  el_item->next = next;
  el_item->prev = 0;
  head.p = el_item;
  el->prev = el_item;
}

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::Delete (Iterator &it)
{
  CS_ASSERT(it.HasCurrent());
  ListElement* el = it.ptr;

  if (el->prev == 0)
  {
    // Deleting first element, reset to next
    if (it.IsReverse())
      --it;
    else
      ++it;
    Delete(el);
    it.visited = false;
  }
  else
  {
    /* Make a step back so the current element can be deleted
       and the next element returned is the one after the deleted */
    if (it.IsReverse())
      ++it;
    else
      --it;
    Delete(el);
  }
}

template <class T, class MemoryAllocator>
inline void csList<T, MemoryAllocator>::Delete (ListElement *el)
{
  CS_ASSERT(el != 0);

  // Fix the pointers of the 2 surrounding elements
  if (el->prev)
    el->prev->next = el->next;
  else
    head.p = el->next;

  if (el->next)
    el->next->prev = el->prev;
  else
    tail = el->prev;

  FreeElement (el);
}

#endif //__CS_UTIL_LIST_H__
