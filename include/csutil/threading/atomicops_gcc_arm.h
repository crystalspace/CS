/*
  Copyright (C) 2010 by Stefano Angeleri
                2006 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_ATOMICOPS_GCC_ARM_H__
#define __CS_CSUTIL_ATOMICOPS_GCC_ARM_H__

#ifndef DOXYGEN_RUN

namespace CS
{
namespace Threading
{
  class CS_CRYSTALSPACE_EXPORT AtomicOperationsArmGCC
  {

    private:

    //NOTE: This could be a big problem with static linking but 
    //      I couldn't find a better way to implement it.
    //      (Each plugin will have his copy making the lock useless)
    static char AtomicLock;

    inline static char Swpb(volatile char *target, char value)
    {
       register char ret;
       asm volatile("swpb %0 , %2, [%3]"
                    : "=&r"(ret), "=m" (*target)
                    : "r"(value), "r"(target)
                    : "cc", "memory");
       return ret;
    }

    public:

    inline static int32 Set (int32* target, int32 value)
    {
       while(Swpb(&AtomicLock, ~0) != 0);
       *target = value;
       Swpb(&AtomicLock, 0);
       return value;
    }

    inline static void* Set (void** target, void* value)
    {
       return (void*)Set ((int32*)target, (int32)value);
    }

    inline static int32 CompareAndSet (int32* target, int32 value,
      int32 comparand)
    {

       while(Swpb(&AtomicLock, ~0) != 0);
       int32 oldvalue = *target;
       if(*target == comparand)
         *target = value;
       Swpb(&AtomicLock, 0);
       return oldvalue;
    }

    inline static void* CompareAndSet (void** target, void* value,
      void* comparand)
    {
       return (void*)CompareAndSet ((int32*)target, (int32)value,
        (int32)comparand);
    }

    inline static int32 Increment (int32* target, register int32 incr = 1)
    {
       while(Swpb(&AtomicLock, ~0) != 0);
       *target += incr;
       int32 ret = *target;
       Swpb(&AtomicLock, 0);
       return ret;
    }

    inline static int32 Decrement (int32* target)
    {
       return (int32)Increment (target, -1);
    }
  };
}
}

#endif // DOXYGEN_RUN

#endif // __CS_CSUTIL_ATOMICOPS_GCC_ARM_H__
