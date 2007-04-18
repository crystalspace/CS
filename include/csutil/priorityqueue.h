/*
    Copyright (C) 2007 by Frank Richter

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

#ifndef __CS_CSUTIL_PRIORTITYQUEUE_H__
#define __CS_CSUTIL_PRIORTITYQUEUE_H__

/**\file
 * Priority queue implementation
 */
 
#include "array.h"

namespace CS
{
  namespace Utility
  {
    
    /**
     * An implementation of a priority queue.
     * Items with a \em higher priority value are returned first.
     */
    template<typename T, typename Tpriority = int>
    class PriorityQueue
    {
      struct QueueItem
      {
	Tpriority p;
	T val;

        QueueItem (const Tpriority& p, const T& val) : p (p), val (val) {}
      };
      // Play safe so e.g. csWeakRef<> works
      csSafeCopyArray<QueueItem> items;

      inline static size_t Parent (size_t n) { return (n-1)/2; }
      inline static size_t Left (size_t n) { return 2*n+1; }
      inline static size_t Right (size_t n) { return 2*n+2; }

      void SwapItems (size_t a, size_t b)
      {
        QueueItem tmp (items[a]);
        items[a] = items[b];
        items[b] = tmp;
      }

      /// Ensure heap order on 'items'.
      void HeapifyUp (size_t n)
      {
        size_t current = n;
        while (current > 0)
        {
          size_t parent = Parent (current);
          size_t larger = current;
          if (((current ^ 1) < items.GetSize())
            && (items[current ^ 1].p > items[larger].p))
          {
            larger = current ^ 1;
          }
          if (items[larger].p > items[parent].p)
            SwapItems (larger, parent);
          else
            return;
          current = parent;
        }
      }
      /// Ensure heap order on 'items'.
      void HeapifyDown (size_t n)
      {
        size_t current = n;
        do
        {
          size_t l = Left (current);
          size_t r = Right (current);
          size_t larger = current;
          if ((l < items.GetSize())
            && (items[l].p > items[larger].p))
          {
            larger = l;
          }
          if ((r < items.GetSize())
            && (items[r].p > items[larger].p))
          {
            larger = r;
          }
          if (larger == current) return;
          SwapItems (larger, current);
          current = larger;
        }
        while (current < items.GetSize ());
      }
    public:
      typedef Tpriority PriorityType;
    
      /// Insert an item with a given priority.
      void Insert (const Tpriority& prio, const T& what)
      {
        size_t n = items.Push (QueueItem (prio, what));
        HeapifyUp (n);
      }
    
      //@{
      /// Return and remove the item with the \em highest priority.
      T Pop (Tpriority& prio)
      {
        prio = items[0].p;
        T val = items[0].val;
        items.DeleteIndexFast (0);
        HeapifyDown (0);
        return val;
      }
      T Pop ()
      {
        T val = items[0].val;
        items.DeleteIndexFast (0);
        HeapifyDown (0);
        return val;
      }
      //@}
    
      //@{
      /// Return, but don't remove the item with the \em highest priority.
      const T& Top (Tpriority& prio) const
      {
        prio = items[0].p;
        return items[0].val;
      }
      const T& Top () const
      {
        return items[0].val;
      }
      //@}

      /**
       * Remove \a item with highest attached priority from the queue.
       * \returns Whether \a item was found and deleted.
       * \remarks Does a linear search - slow.
       */
      bool Delete (const T& item)
      {
        for (size_t n = 0; n < items.GetSize(); n++)
        {
          if (csComparator<T, T>::Compare (items[n].val, item) == 0)
          {
            items.DeleteIndexFast (n);
            HeapifyDown (n);
            return true;
          }
        }
        return false;
      }
    
      /// Return whether items are still in the queue.
      bool IsEmpty() const { return items.IsEmpty(); }
    };
    
  } // namespace Utility
} // namespace CS

#endif // __CS_CSUTIL_PRIORTITYQUEUE_H__
