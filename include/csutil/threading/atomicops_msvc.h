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

#ifndef __CS_CSUTIL_ATOMICOPS_MSVC_H__
#define __CS_CSUTIL_ATOMICOPS_MSVC_H__

#ifndef DOXYGEN_RUN

namespace CS
{
namespace Threading
{

#pragma warning(push)
#pragma warning(disable: 4311)

  //Windows and MSVC, use intrinsics
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
#pragma warning(pop)
  
}
}

#endif // DOXYGEN_RUN

#endif // __CS_CSUTIL_ATOMICOPS_MSVC_H__
