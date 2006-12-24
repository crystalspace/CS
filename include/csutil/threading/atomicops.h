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

#ifndef __CS_CSUTIL_ATOMICOPS_H__
#define __CS_CSUTIL_ATOMICOPS_H__

/**\file
 * Basic low-level atomic operations for different processors
 */
namespace CS
{
namespace Threading
{

  /**
   * Define low-level atomic operations.
   * These operations are truly atomic on the processor, and thereby
   * safe to use on shared variables in a multi-threaded environment.
   */
  template<typename Impl>
  class AtomicOperationsBase
  {
  public:
    /**
     * Set value pointed to by target to value and return previous value
     */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static int32 Set (int32* target, int32 value)
    {
      return Impl::Set (target, value);
    }

    /**
     * Set pointer pointed to by target to value and return previous value.
     */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static void* Set ( void** target, void* value)
    {
      return Impl::Set (target, value);
    }

    /**
     * Compare value pointed to by target with comparand, if they are equal
     * set value pointed to by target to value, otherwise do nothing.
     * Returns initial value pointed to by target.
     */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static int32 CompareAndSet (int32* target, int32 value,
      int32 comparand)
    {
      return Impl::CompareAndSet (target, value, comparand);
    }

    /**
     * Compare pointer pointed to by target with comparand, if they are equal
     * set pointer pointed to by target to value, otherwise do nothing.
     * Returns initial pointer pointed to by target.
     */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static void* CompareAndSet (void** target, void* value,
      void* comparand)
    {
      return Impl::CompareAndSet (target, value, comparand);
    }

    /**
     * Atomically increment value pointed to by target.
     * Returns value after increment.
     */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static int32 Increment (int32* target)
    {
      return Impl::Increment (target);
    }

    /**
     * Atomically decrement value pointed to by target.
     * Returns value after decrement.
     */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static int32 Decrement (int32* target)
    {
      return Impl::Decrement (target);
    }

    /**
     * Atomically read value pointed to by target.
     */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static int32 Read (int32* target)
    {
      return Impl::CompareAndSet (target, 0, 0);
    }

    /**
    * Atomically read pointer pointed to by target.
    */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static void* Read (void** target)
    {
      return Impl::CompareAndSet (target, (void*)0, (void*)0);
    }
  };

#if defined (CS_PLATFORM_WIN32) && \
    defined (CS_COMPILER_MSVC)      //Windows and MSVC, use intrinsics

#pragma warning(push)
#pragma warning(disable: 4311)

  class AtomicOperationsMSVC
  {
  public:
    inline static int32 Set (int32* target, int32 value)
    {
      return (int32)_InterlockedExchange ((long*)target, value);
    }

    inline static void* Set (void** target, void* value)
    {
#if CS_PROCESSOR_SIZE == 64
      return (void*)_InterlockedExchange64 ((__int64*)target, (__int64)value);
#else
      return (void*)_InterlockedExchange ((long*)target, (long)value);
#endif
    }

    inline static int32 CompareAndSet (int32* target, int32 value,
                                       int32 comparand)
    {
      return (int32)_InterlockedCompareExchange ((long*)target,
        (long)value, (long)comparand);
    }

    inline static void* CompareAndSet (void** target, void* value,
      void* comparand)
    {
#if CS_PROCESSOR_SIZE == 64
      return (void*)_InterlockedCompareExchange64 ((__int64*)target, 
        (__int64)value, (__int64)comparand);
#else
      return (void*)_InterlockedCompareExchange ((long*)target, 
        (long)value, (long)comparand);
#endif
    }

    inline static int32 Increment (int32* target)
    {
      return (int32)_InterlockedIncrement ((long*)target);
    }

    inline static int32 Decrement (int32* target)
    {
      return (int32)_InterlockedDecrement ((long*)target);
    }
  };
  
  typedef AtomicOperationsBase<AtomicOperationsMSVC> AtomicOperations;
#pragma warning(pop)

#elif defined(CS_COMPILER_GCC) && \
      defined(CS_PROCESSOR_X86)

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

  typedef AtomicOperationsBase<AtomicOperationsX86GCC> AtomicOperations;

#elif defined(CS_COMPILER_GCC) && \
      defined(CS_PROCESSOR_POWERPC)

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
      return (void*)Set ((int32*)target, (int32)value);
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
      return (void*)CompareAndSet ((int32*)target, (int32)value, 
        (int32)comparand);
    }

    inline static int32 Increment (int32* target, int32 incr = 1)
    {
      //@@Potentially dangerous code, needs to be revisited
      int32 prevValue, currValue, nextValue;
      do 
      {
        currValue = *target;
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

  typedef AtomicOperationsBase<AtomicOperationsPPCGCC> AtomicOperations;

#else
#error "No atomic operations defined for your platform!"
#endif

}
}

#endif
