/*
Copyright (C) 2010 by Frank Richter

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

#include "../memutil.h"

namespace CS {
  namespace Platform {
    namespace Implementation {

      size_t GetMaxVirtualSize()
      {
	// Guess available virtual address space ...
      #if CS_PROCESSOR_SIZE == 32
	// 32-bit: 2GiB virtual address space
	return 2 * 1024 * 1024;
      #else
	// 64-bit: 8TiB virtual address space
	return 8 * size_t (1024 * 1024 * 1024);
      #endif
      }

    } // End namespace Implementation
  } // End namespace Platform
} // End namespace CS
