/*
    Copyright (C) 2000 by David Durant

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

#ifndef __CS_CSDLLIST_H__
#define __CS_CSDLLIST_H__

#include "cstypes.h"

/**
 * This structure should not need to be accessed directly.
 * It is used exclusively by the csDLinkList class.
 */
struct csDLListItem
{
  csDLListItem * prevItem;   // ptr to previous node
  csDLListItem * nextItem;   // ptr to next node
  void *         theObject;  // ptr to the object
};


/**
 * This class implements a doubly-linked list. Note that this implementation
 * builds a circular list, that means to next pointer of the last element
 * points to the first element.
 * <p>
 * Nothing in this code affects the objects in the list.
 * They do not get deleted by any of these function calls.
 */
class csDLinkList
{
private:
  /// Pointer to first csDLListItem node
  csDLListItem* firstItem;
  /// Pointer to current csDLListItem node
  csDLListItem* currentItem;
  /// Private search method
  csDLListItem* FindListItem (void *anObj);

public:
  /// Constructor.
  csDLinkList ();

  /**
   * Deletes just the csDLListItems in the list, not the objects they
   * point to.
   */
  ~csDLinkList ();

  /**
   * Add an item to the list.  Does not affect the currentItem unless
   * this is the first item added to the list, in which case this becomes
   * the currentItem, as well as the firstItem.
   */
  bool AddItem (void *theObj);

  /**
   * Add an item to the list, immediately after the current item.  Then
   * set the current item to be the one just added.
   */
  bool AddCurrentItem (void *theObj);

  /// Remove the currentItem from the list.
  void RemoveItem ();
  /// Remove the specified item from the list.
  void RemoveItem (void *theObj);

  /**
   * Return the first item in the list, and set the currentItem to
   * the first item.
   */
  void* GetFirstItem ();

  /**
   * Return the first item in the list, but do not set the currentItem
   * to the first item.
   */
  void* PeekFirstItem ();

  /**
   * Return the previous item in the list, and set the
   * currentItem to that item.
   */
  void* GetPrevItem ();

  /**
   * Return the next item in the list, and set the currentItem
   * to that item.
   * <p>
   * Note that the last list element doesn't point to NULL but to the
   * first item again. So to traverse a list do something like the following:
   * <code>
   * csDLinkList list;
   * type* first = (type*)list.GetFirstItem ();
   * type* p = first;
   * do
   * {
   *   ...
   *   p = (type*) list.GetNextItem ();
   * } while (p != first);
   * </code>
   */
  void* GetNextItem ();

  /// Return the current item in the list.
  void* GetCurrentItem ();
  /// Set the specified item as the currentItem.
  void* SetCurrentItem (void *theObj);
};

#endif // __CS_CSDLLIST_H__

