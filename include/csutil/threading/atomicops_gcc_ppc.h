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

#ifndef __CS_CSUTIL_ATOMICOPS_GCC_PPC_H__
#define __CS_CSUTIL_ATOMICOPS_GCC_PPC_H__

#ifndef DOXYGEN_RUN

namespace CS
{
namespace Threading
{
  class AtomicOperationsPPCGCC
  {
  public:
    inline static int32 Set (int32* target, int32 value)
    {
      __asm__ __volatile__
        (
        "       lwsync \n"
        "1:     lwarx   %0,0,%2 \n"
        "       dcbt     0,%2 \n"
        "       stwcx.  %3,0,%2 \n"
        "       bne-    1b\n"
        "       isync \n"
        : "=&r" (value), "=m" (*(unsigned int *)target)
        : "r" (target), "r" (value), "m" (*(unsigned int *)target)
        : "cc", "memory"
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
        "       lwsync \n"
        "1:     ldarx   %0,0,%2 \n"
        "       dcbt     0,%2 \n"
        "       stdcx.  %3,0,%2 \n"
        "       bne-    1b\n"
        "       isync \n"
        : "=&r" (value), "=m" (*(unsigned int *)target)
        : "r" (target), "r" (value), "m" (*(unsigned int *)target)
        : "cc", "memory"
        );
      return value;
#endif
    }

    inline static int32 CompareAndSet (int32* target, int32 value,
      int32 comparand)
    {
      int32 prev;

      __asm__ __volatile__ (
      "       lwsync \n"
      "1:     lwarx   %0,0,%2\n"
      "       cmpw    0,%0,%3\n"
      "       bne-    2f\n"
      "       dcbt     0,%2 \n"
      "       stwcx.  %4,0,%2\n"
      "       bne-    1b\n"
      "       isync     \n"
      "2:"
      : "=&r" (prev), "=m" (*target)
      : "r" (target), "r" (comparand), "r" (value), "m" (*target)
      : "cc", "memory");
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

      __asm__ __volatile__ (
        "       lwsync \n"
        "1:     ldarx   %0,0,%2\n"
        "       cmpd    0,%0,%3\n"
        "       bne-    2f\n"
        "       dcbt     0,%2 \n"
        "       stdcx.  %4,0,%2\n"
        "       bne-    1b\n"
        "       isync     \n"
        "2:"
        : "=&r" (prev), "=m" (*target)
        : "r" (target), "r" (comparand), "r" (value), "m" (*target)
        : "cc", "memory");
      return prev;
#endif
    }

    inline static int32 Increment (int32* target, int32 incr = 1)
    {
      int32 prev;

      __asm__ __volatile__ (
        "       lwsync \n"
        "1:     lwarx   %0,0,%1\n"
        "       addc    %0,%0,%2\n"
        "       stwcx.  %0,0,%1\n"
        "       bne-    1b\n"
        "       isync     \n"        
        : "=&r" (prev)
        : "r" (target), "r" (incr)
        : "cc", "memory");
      return prev;
    }

    inline static int32 Decrement (int32* target)
    {
      return (int32)Increment (target, -1);
    }
  };
}
}

#endif // DOXYGEN_RUN

#endif // __CS_CSUTIL_ATOMICOPS_GCC_PPC_H__
