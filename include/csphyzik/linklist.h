/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert

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

#ifndef __CT_LINKLIST_H__
#define __CT_LINKLIST_H__

#include <stdlib.h>

template <class T>
class llLink
{
public:
  T* contents;
  llLink<T>* next;

  llLink ()
  { contents = 0; next = 0; }

  llLink ( T* c )
  { contents = c; next = 0; }

  llLink ( T* c, llLink<T>* n )
  { contents = c; next = n; }

  ~llLink(){}

  void delete_contents ()
  { delete contents; }
};

/**
 * Singly-link list. Uses a sentinel at head.
 * Caches link immediately before most recent access for ease of deletion.
 * Caches more recent link for ease of iteration.
 */
template <class T>
class ctLinkList
{
protected:
  llLink<T>* head;
  llLink<T>* prev;  // cached the last link accessed
  llLink<T>* current;  // cached the last link accessed
  long size;

public:
  /// Reset list
  void reset()
  { prev = 0; current = head; }

  /// Get first item. set current item to first item
  T* get_first()
  {  if ( head->next != 0 )
     {
       prev = head;
       current = head->next;
       return head->next->contents;
     }
     else return 0;
  }

  /// Get next to current item.  set current item to next item
  T* get_next()
  {
    if ( current->next )
    {
      prev = current;
      current = current->next;
      return current->contents;
    }
    else return 0;
  }

    /// Get first item. don't set current item to first item
  T* peek_first()
  {  return head->next->contents; }

  /// Add link to front
  void add_link ( T* c )
  {
    head->next = new llLink<T>( c, head->next );
    current = head->next;
    prev = head;
    size++;
  }

  /// Add link to front
  void push( T* c )
  {
    head->next = new llLink<T>( c, head->next );
    current = head->next;
    prev = head;
    size++;
  }

  /// Pop first element off list
  T* pop ()
  {
    T* ret = head->next->contents;
    prev = head;
    current = head->next;
    head->next = current->next;
    delete current;
    current = head->next;
    size--;
    return ret;
  }

  /// Remove and delete link, delete contents
  void delete_link ( T* c )
  {
    if ( c == 0 ) return;
    if ( prev && prev->next && c == prev->next->contents )
    {
      current = prev->next->next;
      delete prev->next;
      size--;
      prev->next = current;
    }
    else
    {
      llLink<T>* ll = head;
      while ( ll->next )
      {
        if ( ll->next->contents == c )
	{
          prev = ll;
          current = ll->next->next;
          ll->next->delete_contents();
          delete ll->next;
          size--;
          prev->next = current;
          return;
        }
        ll = ll->next;
      }
    }
  }

  /// Remove and delete link, DOESN'T delete contents
  void remove_link ( T* c )
  {
    if ( c == 0 ) return;
    if ( prev && prev->next && c == prev->next->contents )
    {
      current = prev->next->next;
      delete prev->next;
      size--;
      prev->next = current;
    }
    else
    {
      llLink<T> *ll = head;
      while ( ll->next )
      {
        if( ll->next->contents == c ){
          prev = ll;
          current = ll->next->next;
          delete ll->next;
          size--;
          prev->next = current;
          return;
        }
        ll = ll->next;
      }
    }
  }

  /// Remove all nodes, doesn't delete contents of nodes
  void remove_all ()
  {
    if ( head == 0 )
      return;
    prev = head->next;
    while ( prev )
    {
      current = prev->next;
      delete prev;
      prev = current;
    }
    size = 0;
    head->next = prev = 0;
    current = head;
  }

  /// Remove all nodes, delete contents of nodes
  void delete_all ()
  {
    if ( head == 0 )
      return;
    prev = head->next;
    while ( prev )
    {
      current = prev->next;
      prev->delete_contents ();
      delete prev;
      prev = current;
    }
    size = 0;
    head->next = prev = 0;
    current = head;
  }

  /// Get number of items in list
  long get_size ()
  { return size; }

  /// Constructor
  ctLinkList ()
  { size = 0; head = new llLink<T>(); prev = 0; current = head; }

  /// Destructor.  Clean up nodes.  Does NOT delete contents of nodes
  ~ctLinkList()
  {  remove_all(); delete head; }
};

#endif // __CT_LINKLIST_H__
