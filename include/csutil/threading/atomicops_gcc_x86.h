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

#ifndef __CS_CSUTIL_ATOMICOPS_GCC_X86_H__
#define __CS_CSUTIL_ATOMICOPS_GCC_X86_H__

#ifndef DOXYGEN_RUN

namespace CS
{
namespace Threading
{
  class AtomicOperationsX86GCC
  {
  public:
    inline static int32 Set (int32* target, int32 value)
    {
      __asm__ __volatile__
      (
      "xchgl %0, %1"
      : "=r" (value)
      : "m" (*target), "0" (value)
      : "memory"
      );
      return value;
    }

    inline static void* Set (void** target, void* value)
    {
#if CS_PROCESSOR_SIZE == 32
      return (void*)Set ((int32*)target, (int32)value);
#elif CS_PROCESSOR_SIZE == 64
      __asm__ __volatile__
      (
      "xchgq %0, %1"
      : "=r" (value)
      : "m" (*target), "0" (value)
      : "memory"
      );
      return value;
#endif
    }

    inline static int32 CompareAndSet (int32* target, int32 value,
      int32 comparand)
    {
      int32 prev;
      __asm__ __volatile__
      (
      "lock; cmpxchgl %1, %2"
      : "=a" (prev)
      : "r" (value), "m" (*target), "0" (comparand)
      : "memory"
      );
      return prev;
    }

    inline static void* CompareAndSet (void** target, void* value,
      void* comparand)
    {
#if CS_PROCESSOR_SIZE == 32
      return (void*)CompareAndSet ((int32*)target, (int32)value, 
        (int32)comparand);
#elif CS_PROCESSOR_SIZE == 64
      void* prev;
      __asm__ __volatile__
      (
      "lock; cmpxchgq %1, %2"
      : "=a" (prev)
      : "r" (value), "m" (*target), "0" (comparand)
      : "memory"
      );
      return prev;
#endif
    }

    inline static int32 Increment (int32* target, int32 incr = 1)
    {
      int32 result;
      __asm__ __volatile__
      (
      "lock; xaddl %0, %1"
      : "=r" (result), "=m" (*target)
      : "0" (incr), "m" (*target)
      : "memory"
      );
      return result + incr;
    }

    inline static int32 Decrement (int32* target)
    {
      return (int32)Increment (target, -1);
    }
  };

}
}

#endif // DOXYGEN_RUN

#endif // __CS_CSUTIL_ATOMICOPS_GCC_X86_H__
