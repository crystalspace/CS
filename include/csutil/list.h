/*
    Copyright (C) 2003 by Mårten Svanfeldt
    influenced by Aapl by Adrian Thurston <adriant@ragel.ca>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_UTIL_LIST_H__
#define __CS_UTIL_LIST_H__

/*
 * A lightweight template double-linked list.
 * This is VERY lightweight and not very tested, as it was writen for
 * one single purpose, and have not been extended to be more general
 */
template <class T> class csList
{
protected:
  /* 
   * Template which describs the data stored in the linked list
   * For example a list of ints uses csListElement<int>
   */
  struct csListElement
  {
    /// Use specified data
    csListElement(const T& d, csListElement* newnext,
			      csListElement* newprev)
      : next(newnext), prev(newprev), data(d)
    {}
  
    /// Next element in list. If this is the last one, then next is NULL
    csListElement* next;
  
    /// Previous element in list. If this is the first one, prev is NULL
    csListElement* prev;
  
    /// Accual data
    T data;         
  };
  
public:
  /// Default constructor
  csList()
    : head(0), tail(0)
  {}

  /// Copyconstructor
  csList(const csList &other);

  /// Destructor
  ~csList()
  { DeleteAll (); }

  /// Iterator for the list
  class Iterator
  {
  public:
    Iterator()
      : ptr(0)
    { }
    Iterator(const Iterator& other)
    { ptr = other.ptr; }
    Iterator(const csList<T> &list, bool reverse = false) 
    {
      if(reverse) ptr = list.tail;
      else ptr = list.head;
    }
    const Iterator& operator= (const Iterator& other)
    { ptr = other.ptr; return *this; }

    bool HasNext() const
    { return ptr != 0; }
    bool HasPrevious() const
    { return ptr != 0; }
    bool IsFirst() const
    { return ptr && ptr->prev == 0; }
    bool IsLast() const
    { return ptr && ptr->next == 0; }

    operator T*() const
    { return &ptr->data; }
    T &operator *() const
    { return ptr->data; }
    T *operator->() const
    { return &ptr->data; }

    inline Iterator operator++(int)
    { 
      ptr = ptr->next;
      return Iterator(ptr);
    }
    inline Iterator operator--(int)
    { 
      ptr = ptr->prev;
      return Iterator(ptr);
    }
  protected:
    friend class csList;
    Iterator (csListElement* element)
      : ptr(element)
    { }

  private:
    csListElement* ptr;
  };

  /// Assignment, swallow copy
  csList &operator=(const csList& other);

  /// Add an item first in list. Copy T into the listdata
  Iterator PushFront (const T & item);
  
  /// Add an item last in list. Copy T into the listdata
  Iterator PushBack (const T & item);

  /// Remove specific item
  void Delete (Iterator element);

  /// Empty an list
  void DeleteAll();

  /// Return first element of the list
  const T& Front () const
  { return head->data; }
  /// Return last element of the list
  const T& Last () const
  { return tail->data; }

  /// Deletes the first element of the list
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

private:
  friend class Iterator;
  csListElement *head, *tail;
};

/// Deep copy of list
template <class T> csList<T>::csList(const csList<T> &other)
  : head(0), tail(0)
{
  csListElement* e = other.head;
  while( e != 0)
  {
    PushBack (e->data);
    e = e->next;
  }
}

/// assignment, eraes and deep-copy
template <class T> csList<T>& csList<T>::operator =(const csList<T> &other)
{
  DeleteAll ();

  csListElement* e = other.head;
  while( e != 0)
  {
    PushBack (e->data);
    e = e->next;
  }

  return *this;
}

/// delete all elements, do not touche the raw data
template <class T> void csList<T>::DeleteAll ()
{
  csListElement *cur = head, *next = 0;
  
  while(cur != 0)
  {
    next = cur->next;
    delete cur;
    cur = next;
  }
  head = tail = 0;
}

/// add one item last in the list
template <class T> typename csList<T>::Iterator csList<T>::PushBack (const T& item)
{
  csListElement* el = new csListElement (item, NULL, tail);
  if (tail)
    tail->next = el;
  else
    head = el;
  tail = el;
  
  return Iterator(el);
}

/// add one item first in the list
template <class T> typename csList<T>::Iterator csList<T>::PushFront (const T& item)
{
  csListElement* el = new csListElement (item, head, NULL);
  if (head)
    head->prev = el;
  else
    tail = el;
  head = el;
  
  return Iterator (el);
}

template <class T> void csList<T>::Delete (Iterator it)
{
  csListElement* el = it.ptr;

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
