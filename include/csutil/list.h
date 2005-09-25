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

/**
 * A lightweight double-linked list template.  Copies the elements into the
 * list for storages.  Assumes that type T supports copy construction.
 */
template <class T>
class csList
{
protected:
  /**
   * Template which describes the data stored in the linked list
   * For example a list of ints uses csListElement<int>.
   */
  struct csListElement
  {
    /// Use specified data
    csListElement(const T& d, csListElement* newnext, csListElement* newprev) :
      next(newnext), prev(newprev), data(d) {}

    /// Next element in list. If this is the last one, then next is 0
    csListElement* next;

    /// Previous element in list. If this is the first one, prev is 0
    csListElement* prev;

    /// Stored data
    T data;
  };

  /// Remove specific item by explicit ref
  void Delete (csListElement *el);

public:
  /// Default constructor
  csList() : head(0), tail(0) {}

  /// Copy constructor
  csList(const csList<T> &other);

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
    Iterator(const csList<T> &list, bool reverse = false) :
      visited(false), reversed(reverse)
    {
      if (reverse) ptr = list.tail;
      else ptr = list.head;
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
    friend class csList<T>;
    Iterator (csListElement* element, bool visit = true, bool rev = false) :
      ptr(element), visited(visit), reversed(rev)
    {}

  private:
    csListElement* ptr;
    bool visited;
    bool reversed;
  };

  /// Assignment, shallow copy.
  csList& operator=(const csList<T>& other);

  /// Add an item first in list. Copy T into the listdata.
  Iterator PushFront (const T& item);

  /// Add an item last in list. Copy T into the listdata.
  Iterator PushBack (const T& item);

  /// Insert an item before the item the iterator is set to.
  void InsertBefore(Iterator& it, const T& item);

  /// Insert an item after the item the iterator is set to.
  void InsertAfter(Iterator& it, const T& item);

  /// Move an item (as iterator) before the item the iterator is set to.
  void MoveBefore(const Iterator& it, const Iterator& item);

  /// Move an item (as iterator) after the item the iterator is set to.
  void MoveAfter(const Iterator& it, const Iterator& item);

  /// Remove specific item by iterator.
  void Delete (Iterator& it);

  /// Empty an list.
  void DeleteAll();

  /// Return first element of the list.
  T& Front () const
  { return head->data; }
  /// Return last element of the list.
  T& Last () const
  { return tail->data; }

  /// Deletes the first element of the list.
  bool PopFront ()
  {
    if (!head)
      return false;
    Delete (head);
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
    CS_ASSERT((head == 0 && tail == 0) || (head !=0 && tail != 0));
    return head == 0;
  }

private:
  friend class Iterator;
  csListElement *head, *tail;
};

/// Deep copy of list
template <class T>
inline csList<T>::csList(const csList<T> &other) : head(0), tail(0)
{
  csListElement* e = other.head;
  while (e != 0)
  {
    PushBack (e->data);
    e = e->next;
  }
}

/// Assignment, deep-copy
template <class T>
inline csList<T>& csList<T>::operator= (const csList<T> &other)
{
  DeleteAll ();
  csListElement* e = other.head;
  while (e != 0)
  {
    PushBack (e->data);
    e = e->next;
  }
  return *this;
}

/// Delete all elements
template <class T>
inline void csList<T>::DeleteAll ()
{
  csListElement *cur = head, *next = 0;
  while (cur != 0)
  {
    next = cur->next;
    delete cur;
    cur = next;
  }
  head = tail = 0;
}

/// Add one item last in the list
template <class T>
inline typename_qualifier csList<T>::Iterator csList<T>::PushBack (const T& e)
{
  csListElement* el = new csListElement (e, 0, tail);
  if (tail)
    tail->next = el;
  else
    head = el;
  tail = el;
  return Iterator(el);
}

/// Add one item first in the list
template <class T>
inline typename_qualifier csList<T>::Iterator csList<T>::PushFront (const T& e)
{
  csListElement* el = new csListElement (e, head, 0);
  if (head)
    head->prev = el;
  else
    tail = el;
  head = el;
  return Iterator (el);
}

template <class T>
inline void csList<T>::InsertAfter (Iterator &it, const T& item)
{
  CS_ASSERT(it.HasCurrent());
  csListElement* el = it.ptr;
  csListElement* next = el->next;
  csListElement* prev = el;
  csListElement* newEl = new csListElement (item, next, prev);
  if (!next) // this is the last element
    tail = newEl;
  else
    el->next->prev = newEl;
  el->next = newEl;
}

template <class T>
inline void csList<T>::InsertBefore (Iterator &it, const T& item)
{
  CS_ASSERT(it.HasCurrent());
  csListElement* el = it.ptr;
  csListElement* next = el;
  csListElement* prev = el->prev;
  csListElement* newEl = new csListElement (item, next, prev);
  if (!prev) // this is the first element
    head = newEl;
  else
    el->prev->next = newEl;
  el->prev = newEl;
}

template <class T>
inline void csList<T>::MoveAfter (const Iterator &it, const Iterator &item)
{
  CS_ASSERT(item.HasCurrent());
  csListElement* el_item = item.ptr;

  // Unlink the item.
  if (el_item->prev)
    el_item->prev->next = el_item->next;
  else
    head = el_item->next;
  if (el_item->next)
    el_item->next->prev = el_item->prev;
  else
    tail = el_item->prev;

  CS_ASSERT(it.HasCurrent());
  csListElement* el = it.ptr;
  csListElement* next = el->next;
  csListElement* prev = el;

  el_item->next = next;
  el_item->prev = prev;
  if (!next) // this is the last element
    tail = el_item;
  else
    el->next->prev = el_item;
  el->next = el_item;
}

template <class T>
inline void csList<T>::MoveBefore (const Iterator &it, const Iterator &item)
{
  CS_ASSERT(item.HasCurrent());
  csListElement* el_item = item.ptr;

  // Unlink the item.
  if (el_item->prev)
    el_item->prev->next = el_item->next;
  else
    head = el_item->next;
  if (el_item->next)
    el_item->next->prev = el_item->prev;
  else
    tail = el_item->prev;

  CS_ASSERT(it.HasCurrent());
  csListElement* el = it.ptr;
  csListElement* next = el;
  csListElement* prev = el->prev;

  el_item->next = next;
  el_item->prev = prev;
  if (!prev) // this is the first element
    head = el_item;
  else
    el->prev->next = el_item;
  el->prev = el_item;
}

template <class T>
inline void csList<T>::Delete (Iterator &it)
{
  CS_ASSERT(it.HasCurrent());
  csListElement* el = it.ptr;

  // Advance the iterator so we can delete the data it's using
  if (it.IsReverse())
    --it;
  else
    ++it;

  Delete(el);
}

template <class T>
inline void csList<T>::Delete (csListElement *el)
{
  CS_ASSERT(el != 0);

  // Fix the pointers of the 2 surrounding elements
  if (el->prev)
    el->prev->next = el->next;
  else
    head = el->next;

  if (el->next)
    el->next->prev = el->prev;
  else
    tail = el->prev;

  delete el;
}

#endif //__CS_UTIL_LIST_H__
