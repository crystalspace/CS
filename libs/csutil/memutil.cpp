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

#include "csutil/memutil.h"

namespace CS {
  namespace Memory {
    
    long GetPhysicalMemory()
    {
      #if defined(CS_PLATFORM_WIN32)
        const int kbConversion = 1024;
        MEMORYSTATUS memAmount;
        GlobalMemoryStatus(&memAmount);
        return (memAmount.dwTotalPhys/kbConversion);
      #endif

      #if defined(CS_PLATFORM_WIN64)
        const int kbConversion = 1024;
        MEMORYSTATUSEX memAmount;
        GlobalMemoryStatusEx(&memAmount);
        return (memAmount.ullTotalPhys/kbConversion);
      #endif
      
      #if defined(CS_PLATFORM_UNIX)
        ifstream memInfo("/proc/meminfo");

        if (memInfo.fail())
        {
          cout << "Unable to get memory information!";
          return 0;
        }

        string info, mem;
        memInfo >> info >> mem;
        long totalMemInKB = 0;

        while (! (info.compare("MemTotal:") == 0) && !memInfo.eof())
        {
          memInfo >> info >> mem;
        }

        totalMemInKB = atol(mem.c_str());
        memInfo.close();
        return totalMemInKB;
      #endif
      
      return 0;
    }

  } // End namespace Memory
} // End namespace CS