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

#ifndef __CSUTIL_PLATFORM_H__
#define __CSUTIL_PLATFORM_H__

#include "cssysdef.h"
#include "csextern.h"

namespace CS {
  namespace Platform {

    /**
     * Retrieve the number of KiB of physical system memory.
     *
     * @returns Physical system memory (in KiB) on success, or 0 on failure.
     */
   CS_CRYSTALSPACE_EXPORT size_t GetPhysicalMemorySize();

  /**
   * Retrieve the maximum number of KiB of virtual address space available to the
   * process.
   * \returns Virtual address space size (in KiB) on success, or 0 on failure.
   */
  CS_CRYSTALSPACE_EXPORT size_t GetMaxVirtualSize();

   /**
    * Retrieve the number of processors in the system.
    * \returns Number of processors, or 0 on failure.
    */
   CS_CRYSTALSPACE_EXPORT uint GetProcessorCount();

  } // End namespace Platform
} // End namespace CS

#endif // __CSUTIL_PLATFORM_H__
