/*
    Copyright (C) 2008 by Scott Johnson <scottj@cs.umn.edu>

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

#include "csutil/platform.h"
#include "memutil.h"

namespace CS {
  namespace Platform {

    size_t GetPhysicalMemorySize()
    {
      static size_t memorySize = 0;
      static bool cacheValid = false;

      if (!cacheValid)
      {
        memorySize = CS::Platform::Implementation::GetPhysicalMemorySize();
        cacheValid = true;
      }

      return memorySize;

    } // End GetPhysicalMemorySize()      

    size_t GetMaxVirtualSize()
    {
      static size_t memorySize = 0;
      static bool cacheValid = false;

      if (!cacheValid)
      {
        memorySize = CS::Platform::Implementation::GetMaxVirtualSize();
        cacheValid = true;
      }

      return memorySize;

    }
  } // End namespace Platform
} // End namespace CS
