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

#ifndef __CS_LIBS_CSUTIL_MEMUTIL_H__
#define __CS_LIBS_CSUTIL_MEMUTIL_H__

#include "cssysdef.h"

namespace CS {
  namespace Platform {
    namespace Implementation {

     /**
      * @brief Implementation-dependant memory retreival function.
      *
      * Used by CS::Platform::GetPhysicalMemorySize().  Do not call this
      * function directly, use CS::Platform::GetPhysicalMemorySize().
      *
      * @returns Physical system memory (in kB) on success, or 0 on failure.
      *
      * @sa CS::Platform::GetPhysicalMemorySize()
      */
    size_t GetPhysicalMemorySize();

   /**
    * Implementation-dependant virtual address size retrieval function.
    *
    * Used by CS::Platform::GetMaxVirtualSize().  Do not call this
    * function directly, use CS::Platform::GetMaxVirtualSize().
    *
    * \sa CS::Platform::GetMaxVirtualSize()
    */
   size_t GetMaxVirtualSize();

    } // End namespace Implementation
  } // End namespace Platform
} // End namespace CS

#endif
