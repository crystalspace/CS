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
    static int32 Read (const int32* target)
    {
      return Impl::CompareAndSet (const_cast<int32*> (target), 0, 0);
    }

    /**
    * Atomically read pointer pointed to by target.
    */
    CS_FORCEINLINE_TEMPLATEMETHOD
    static void* Read (void* const* target)
    {
      return Impl::CompareAndSet (
        const_cast<void**> (target), (void*)0, (void*)0);
    }
  };
}
}

#if defined (CS_PLATFORM_WIN32) && \
    defined (CS_COMPILER_MSVC)

  #include "csutil/threading/atomicops_msvc.h"

  namespace CS { namespace Threading {
    typedef AtomicOperationsBase<AtomicOperationsMSVC> AtomicOperations;
  } }

#elif defined(CS_COMPILER_GCC) && \
      defined(CS_PROCESSOR_X86)
  
  #include "csutil/threading/atomicops_gcc_x86.h"

  namespace CS { namespace Threading {
    typedef AtomicOperationsBase<AtomicOperationsX86GCC> AtomicOperations;
  } }

#elif defined(CS_COMPILER_GCC) && \
      defined(CS_PROCESSOR_POWERPC)
  
  #include "csutil/threading/atomicops_gcc_ppc.h"

  namespace CS { namespace Threading {
    typedef AtomicOperationsBase<AtomicOperationsPPCGCC> AtomicOperations;
  } }

#elif defined(CS_COMPILER_GCC) && \
      defined(CS_PROCESSOR_SPARC)

  #include "csutil/threading/atomicops_sparc.h"

  namespace CS { namespace Threading {
    typedef AtomicOperationsBase<AtomicOperationsSparc> AtomicOperations;
  } }

#elif defined(CS_COMPILER_GCC) && \
      defined(CS_PROCESSOR_ARM)
  #include "csutil/threading/atomicops_gcc_arm.h"

  namespace CS { namespace Threading {
    typedef AtomicOperationsBase<AtomicOperationsArmGCC> AtomicOperations;
  } }
#else
#error "No atomic operations defined for your platform!"
#endif

#endif
