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
 * Template which describs the data stored in the linked list
 * For example a list of ints uses csListElement<int>
 */
template <class Type> struct csListElement
{
  /// default constructor
  csListElement() :next(0), prev(0), data(0) {}

  /// Use specified data
  csListElement(Type d) : next(0), prev(0), data(d) {}

  /// Next element in list. If this is the last one, then next is NULL
  csListElement<Type>* next;

  /// Previous element in list. If this is the first one, prev is NULL
  csListElement<Type>* prev;

  /// Accual data
  Type data;
};

/*
 * A lightweight template double-linked list.
 * This is VERY lightweight and not very tested, as it was writen for
 * one single purpose, and have not been extended to be more general
 * The best solution would be to replace it with STL, or a STL-api
 * compatible linked-list
 */
template <class T> class csList
{
public:
  /// Default constructor
  csList() : head(0), tail(0) {}

  /// Copyconstructor
  csList(const csList &other);

  /// Destructor
  ~csList() { empty(); }

  /// Assignment, swallow copy
  csList &operator=(const csList& other);

  /// Add an item first in list. Copy T into the listdata
  csListElement<T>* prepend(const T item);
  
  /// Add an item last in list. Copy T into the listdata
  csListElement<T>* append(const T item);

  /// Remove specific item
  void remove(csListElement<T> *element) {delete detach(element);}

  /// Detach element from list
  csListElement<T>* detach(csListElement<T> *element);

  /// Empty an list
  void empty();

  /// Iterator for the double-linked lsit
  class Iterator
  {
  public:
    Iterator() : ptr(0) {}
    Iterator( const csList<T> &list, bool reverse = false ) 
      {
        if(reverse) ptr = list.tail;
        else ptr = list.head;
      }
  
    Iterator &operator=( const csList<T> &list) { ptr = list.head; return *this; }

    bool HasNext() const  { return (ptr) && (ptr->next != 0); }
    bool HasPrevious() const { return (ptr) && (ptr->prev != 0); }
    bool IsFirst() const  { return ptr && ptr->prev == 0; }
    bool IsLast() const   { return ptr && ptr->next == 0; }

    operator T*() const   { return &ptr->data; }
    T &operator *() const { return ptr->data; }
    T *operator->() const { return &ptr->data; }

    inline T operator++(int)  { ptr = ptr->next; return ( ptr == 0 ? 0 : ptr->data); }
    inline T operator--(int)  { ptr = ptr->prev; return ( ptr == 0 ? 0 : ptr->data); }

    csListElement<T>* GetRawPointer() {return ptr;}

  private:
    csListElement<T>* ptr;
  };

private:
  friend class Iterator;
  csListElement<T> *head, *tail;

  /// Add one element before another
  void addBefore(csListElement<T> *nextelement, csListElement<T> *newelement);

  /// Add one element to the list after another
  void addAfter(csListElement<T> *previouselement, csListElement<T> *newelement);
};

/// Deep copy of list
template <class T> csList<T>::csList(const csList<T> &other) : head(0), tail(0)
{
  csListElement<T>* e = other.head;
  while( e != 0)
  {
    append(e->data);
    e = e->next;
  }
}

/// assignment, eraes and deep-copy
template <class T> csList<T>& csList<T>::operator =(const csList<T> &other)
{
  empty();

  csListElement<T>* e = other.head;
  while( e != 0)
  {
    append(e->data);
    e = e->next;
  }

  return *this;
}

/// delete all elements, do not touche the raw data
template <class T> void csList<T>::empty()
{
  csListElement<T> *cur = head, *next = 0;
  
  while(cur != 0)
  {
    next = cur->next;
    delete cur;
    cur = next;
  }
  head = tail = 0;
}

/// add one item last in the list
template <class T> csListElement<T>* csList<T>::append(const T item)
{
  csListElement<T>* el = new csListElement<T>(item);
  addAfter(tail, el );
  return el;
}

/// add one item first in the list
template <class T> csListElement<T>* csList<T>::prepend(const T item)
{
  csListElement<T>* el = new csListElement<T>(item);
  addBefore(head, el );
  return el;
}

template <class T> void csList<T>::addAfter(csListElement<T>* previouselement,
                                            csListElement<T>* newelement)
{
  //set previous pointer
  newelement->prev = previouselement;

  //set forward pointer
  if(previouselement == 0)
  {
    //no previous element, insert at head
    newelement->next = head;
    head = newelement;
  }
  else
  {
    newelement->next = previouselement->next;
    previouselement->next = newelement;
  }

  //set reverse pointers
  if(newelement->next == 0)
  {
    //no nextelement
    tail = newelement;
  }
  else
  {
    newelement->next->prev = newelement;
  }
}

template <class T> void csList<T>::addBefore(csListElement<T>* nextelement,
                                             csListElement<T>* newelement)
{
  //set next pointer
  newelement->next = nextelement;

  //set reverse pointers
  if (nextelement == 0)
  {
    //no next element, insert at tail
    newelement->prev = tail;
    tail = newelement;
  }
  else
  {
    newelement->prev = nextelement->prev;
    nextelement->prev = newelement;
  }

  //set forward pointer
  if (newelement->prev == 0)
  {
    //no previous element, set at head
    head = newelement;
  }
  else
  {
    newelement->prev->next = newelement;
  }
}

template <class T> csListElement<T>* csList<T>::detach(csListElement<T>* el)
{

  //set forward pointer to skip element
  if (el->prev == 0)
  {
    head = el->next;
  }
  else
  {
    el->prev->next = el->next;
  }

  //set reverse pointer to skip element
  if (el->next == 0)
  {
    tail = el->prev;
  }
  else
  {
    el->next->prev = el->prev;
  }

  return el;
}

#endif //__CS_UTIL_LIST_H__