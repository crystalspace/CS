/*
Copyright (C) 2009 by Trond Varslot <i.am.faramann@gmail.com>

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

// FixMe: create a check for existence of sys/sysctl.h
//#if defined(CS_HAVE_SYSCTLBYNAME)
#include <sys/sysctl.h>
//#endif

unsigned query_sysctl()
{
  int     cnt(0);
  // FixMe: create a check for existence of sys/sysctl.h
  //#if defined(CS_HAVE_SYSCTLBYNAME)
  size_t  size ( sizeof(cnt) );
  sysctlbyname("hw.ncpu",&cnt,&size,NULL,0);
  //#endif
  return cnt;
}


namespace CS {
  namespace Platform {
    namespace Implementation {

      unsigned GetProcessorCount()
      {
	unsigned s;
        s = query_sysctl();
        return s;
      }
    } // End namespace Implementation
  } // End namespace Platform
} // End namespace CS

