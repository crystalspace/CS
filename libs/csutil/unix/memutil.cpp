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
#include "csutil/sysfunc.h"
#include "../memutil.h"
#include <sys/sysinfo.h>


namespace CS {
  namespace Platform {
    namespace Implementation {

      size_t GetPhysicalMemorySize()
      {
        int error = 0;
#if defined(CS_HAVE_SYSINFO)
        struct sysinfo s_info;
        error = sysinfo(&s_info);

        if (error == 0)
        {
          return (s_info.totalram/1024);
        }
#elif 0 
        // this isn't working, simply because /proc/meminfo isn't a normal file
        // I think perhaps csPhysicalFile just doesn't like it.  :)
        //defined (CS_HAVE_PROC_MEMINFO)
        // no sysinfo(), but we have /proc/meminfo
        // open /proc/meminfo
        csRef<iFile> procMeminfo;
        procMeminfo.AttachNew(new csPhysicalFile("/proc/meminfo", "rb"));
        csRef<iDataBuffer> allData = procMeminfo->GetAllData();
        csStringReader lineParser(allData->GetData());

        // while we haven't found the line we are looking for
        size_t totalMem = 0;
        while (lineParser.HasMoreLines())
        {
          // grab the next line
          csString nextLine;
          lineParser.GetLine(nextLine);
          size_t pos = nextLine.Find(":");
          csString sub1 = nextLine.Slice(0, pos);
          csString sub2 = nextLine.Slice(pos+1, nextLine.Length() - 1);

          // determine if the first token is MemTotal
          if (sub1.Compare("MemTotal:"))
          {
            // if it is, then the next token is what we want
            totalMem = (size_t)atol(sub2.GetData());
            break;
          }
        }

        // divide the amount we have to get it in kB
        totalMem /= 1024;
        
        // return the scaled amount
        return totalMem;
#endif
        // everything seems to have failed, so output some information
        // and return 0
        csPrintfErr("ERROR: GetPhysicalMemorySize(): Unable to determine mechanism for finding physical memory size.\n");
        return 0;
      }
    } // End namespace Implementation
  } // End namespace Platform
} // End namespace CS

