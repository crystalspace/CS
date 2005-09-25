/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_MEMDEBUG_H__
#define __CS_MEMDEBUG_H__

/**\file
 * Memory debugging support
 */

#if defined(CS_MEMORY_TRACKER) || defined(CS_MEMORY_TRACKER_IMPLEMENT)

#include "csextern.h"

/**
 * This structure is used per file to keep track of allocations.
 * csMemTrackerModule maintains an array of them per module.
 */
struct CS_CRYSTALSPACE_EXPORT csMemTrackerInfo
{
  char* file;
  size_t max_alloc;
  size_t current_alloc;
  int max_count;
  int current_count;
  void Init (char* filename)
  {
    file = (char*)malloc (strlen (filename)+1);
    strcpy (file, filename);
    max_alloc = 0;
    current_alloc = 0;
    max_count = 0;
    current_count = 0;
  }
};

/// 'info' can be filename or some other information to recognize allocation.
CS_CRYSTALSPACE_EXPORT csMemTrackerInfo* mtiRegisterAlloc(size_t, void* info);
CS_CRYSTALSPACE_EXPORT void mtiRegisterFree(csMemTrackerInfo* mti, size_t s);
CS_CRYSTALSPACE_EXPORT void mtiUpdateAmount(csMemTrackerInfo* mti, int dcount,
					    int dsize);

#endif // CS_MEMORY_TRACKER

#endif // __CS_MEMDEBUG_H__
