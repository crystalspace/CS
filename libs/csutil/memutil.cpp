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

#if 0 // Marten temporarily disabled this until it has been sorted out

namespace CS {
  namespace Memory {
    
	static size_t cachedMemory;
	static bool memoryCached;
	
    size_t GetPhysicalMemory()
    {
      // determine if the amount of physical memory has been cached
	  if (memoryCached)
	  {
	  	// if so, return it
		return cachedMemory;
	  }
	  
	  // otherwise, use implementation-dependant function to get memory
	  size_t currentMem = CS::Memory::Implementation::GetPhysicalMemory();
	  
	  // cache it
	  cachedMemory = currentMem;
	  memoryCached = true;
	  
	  // and return it 
      return cachedMemory;
    }

  } // End namespace Memory
} // End namespace CS

#endif
