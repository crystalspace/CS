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
  size_t totalAllocCount;
  size_t totalDeallocCount;
  void Init (const char* filename)
  {
    file = (char*)cs_malloc (strlen (filename)+1);
    strcpy (file, filename);
    max_alloc = 0;
    current_alloc = 0;
    max_count = 0;
    current_count = 0;
    totalAllocCount = 0;
    totalDeallocCount = 0;
  }
};

/// 'info' can be filename or some other information to recognize allocation.
CS_DEPRECATED_METHOD
CS_CRYSTALSPACE_EXPORT csMemTrackerInfo* mtiRegisterAlloc(size_t, 
  const char* info);
CS_DEPRECATED_METHOD
CS_CRYSTALSPACE_EXPORT csMemTrackerInfo* mtiRegister (const char* info);
CS_DEPRECATED_METHOD
CS_CRYSTALSPACE_EXPORT void mtiRegisterModule (const char*);
CS_DEPRECATED_METHOD
CS_CRYSTALSPACE_EXPORT void mtiRegisterFree(csMemTrackerInfo* mti, size_t s);
CS_DEPRECATED_METHOD
CS_CRYSTALSPACE_EXPORT void mtiUpdateAmount(csMemTrackerInfo* mti, int dcount,
					    int dsize);

class csMemTrackerModule;

namespace CS
{
  namespace Debug
  {
    namespace MemTracker
    {
      namespace Impl
      {
	CS_CRYSTALSPACE_EXPORT void RegisterAlloc (csMemTrackerModule* m, 
	  void* p, size_t s, const char* info);
	CS_CRYSTALSPACE_EXPORT void RegisterModule (csMemTrackerModule*& m, const char*);
	CS_CRYSTALSPACE_EXPORT void RegisterFree (csMemTrackerModule* m, void* p);
	CS_CRYSTALSPACE_EXPORT void UpdateSize (csMemTrackerModule* m, void* p,
	  void* newP, size_t newSize);
	CS_CRYSTALSPACE_EXPORT void UpdateAmount (csMemTrackerModule* m, const char* info,
	  int dcount, int dsize);
	CS_CRYSTALSPACE_EXPORT const char* GetInfo (csMemTrackerModule* m, void* p);
	  
	extern csMemTrackerModule* thisModule;
      } // namespace Impl
    
      inline void RegisterAlloc (void* p, size_t s, const char* info)
      { Impl::RegisterAlloc (Impl::thisModule, p, s, info); }
      inline void RegisterModule (const char* s)
      { Impl::RegisterModule (Impl::thisModule, s); }
      inline void RegisterFree (void* p)
      { Impl::RegisterFree (Impl::thisModule, p); }
      inline void UpdateSize (void* p, void* newP, size_t newSize)
      { Impl::UpdateSize (Impl::thisModule, p, newP, newSize); }
      inline void UpdateAmount (const char* info, int dcount, int dsize)
      { Impl::UpdateAmount (Impl::thisModule, info, dcount, dsize); }
      inline const char* GetInfo (void* p)
      { return Impl::GetInfo  (Impl::thisModule, p); }
    } // namespace MemTracker
  } // namespace Debug
} // namespace CS

#endif // CS_MEMORY_TRACKER

#endif // __CS_MEMDEBUG_H__
