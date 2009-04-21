/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_ATOMICOPS_SPARC_H__
#define __CS_CSUTIL_ATOMICOPS_SPARC_H__

#ifndef DOXYGEN_RUN

namespace CS
{
namespace Threading
{
  class AtomicOperationsSparc
  {
  public:
    inline static int32 Set (int32* target, int32 value)
    {
      *const_cast<volatile int32*> (target) = value;
      __asm__ __volatile__
        (
        "membar #StoreStore | #StoreLoad"
        : : : "memory"
        );
      return value;
    }

    inline static void* Set (void** target, void* value)
    {
      return (void*)Set ((int32*)target, (int32)value);
    }

    inline static int32 CompareAndSet (int32* target, int32 value,
      int32 comparand)
    {
      int32 prev;

      __asm__ __volatile__ 
        (
        "cas [%1],%2,%0"
        : "+r" (value)
        : "r" (target), "r" (comparand)
        );

      return prev;
    }

    inline static void* CompareAndSet (void** target, void* value,
      void* comparand)
    {
      return (void*)CompareAndSet ((int32*)target, (int32)value, 
        (int32)comparand);
    }

    inline static int32 Increment (int32* target, int32 incr = 1)
    {
      //@@Potentially dangerous code, needs to be revisited
      int32 prevValue, currValue, nextValue;
      do 
      {
        __asm__ __volatile__
          (
          "membar #StoreLoad | #LoadLoad"
          : : : "memory"
          );

        currValue = *reinterpret_cast<volatile int32*> (target);
        nextValue = currValue + incr;
        prevValue = CompareAndSet (target, nextValue, currValue);
      } while(prevValue == currValue);
      return nextValue;
    }

    inline static int32 Decrement (int32* target)
    {
      return (int32)Increment (target, -1);
    }
  };
}
}

#endif // DOXYGEN_RUN

#endif // __CS_CSUTIL_ATOMICOPS_SPARC_H__
