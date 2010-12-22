/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#include <stdarg.h>

// Include csplatform.h here. Otherwise we don't get the CS_EXTENSIVE_MEMDEBUG
// or CS_MEMORY_TRACKER definitions.
#include "csplatform.h"

#ifdef CS_EXTENSIVE_MEMDEBUG
// in cssysdef.h is a "#define new" which affects the operator
// implementations as well
#  define CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
#  undef CS_EXTENSIVE_MEMDEBUG
#endif

#ifdef CS_MEMORY_TRACKER
#  define CS_MEMORY_TRACKER_IMPLEMENT
#  undef CS_MEMORY_TRACKER
#endif

// it's important that cssysdef.h is included AFTER the above #ifdef
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/scf_implementation.h"

#ifdef CS_MEMORY_TRACKER_IMPLEMENT
#  define CS_MEMORY_TRACKER
#endif

#ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
// Select the type of memory debugger to use:
//#  define MEMDEBUG_EXTENSIVE
//#  define MEMDEBUG_CHECKALLOC
//#  define MEMDEBUG_DUMPALLOC
#else
#  define MEMDEBUG_MEMORY_TRACKER
#endif

#if defined(CS_COMPILER_MSVC)
//========================================================================
// Branch: For VC
//========================================================================

// The VC runtime has it's own memory debugging facility, always
// enabled when using the debug runtime.
// So not much to do here.

#ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
// use the CRT's built-in memory debugging aids
#include <crtdbg.h>

void* operator new (size_t s, void* filename, int line)
{
  return (void*)_malloc_dbg (s, _NORMAL_BLOCK, (char*)filename, line);
}

void* operator new[] (size_t s, void* filename, int line)
{
  return (void*)_malloc_dbg (s, _NORMAL_BLOCK, (char*)filename, line);
}

void operator delete (void* p)
{
  if (p) _free_dbg (p, _NORMAL_BLOCK);
}
void operator delete[] (void* p)
{
  if (p) _free_dbg (p, _NORMAL_BLOCK);
}
#endif  // CS_EXTENSIVE_MEMDEBUG_IMPLEMENT

#elif defined(MEMDEBUG_EXTENSIVE) // CS_COMPILER_MSVC
//========================================================================
// Extensive memory debugger.
//========================================================================

// Configuration:

// If the following define is 1 we will use a table and then we're able
// to do much more extensive (but lots slower) memory debugging.
#define DETECT_USE_TABLE 0

// If this define is 1 then freed memory will not be freed but
// instead kept for some time (depending on age). This is of course very
// memory expensive.
#define DETECT_KEEP_FREE_MEMORY 0
//========================================================================

// Size of the wall in front and after all memory allocations.
#define DETECT_WALL 20
#define DETECT_WALL_SAME 8

#define DETECT      "ABCDabcd01234567890+"
#define DETECTAR    "ABCDabcd+09876543210"      // 8 first bytes same as DETECT
#define DETECTFREE  "FreeFreeFreeFreeFree"
#define DETECT_NEW  0xda
#define DETECT_FREE 0x9d

struct csMemTrackerEntry
{
  char* start;          // 0 if not used or else pointer to memory.
  size_t size;
  bool freed;           // If true then this memory entry is freed.
  unsigned long age;    // Time when 'free' was done.
  char* alloc_file;     // Filename at allocation time.
  int alloc_line;       // Line number.
};

// This is the size of the table with all memory allocations
// that are kept in memory.
#define DETECT_TABLE_SIZE 100000
// When the table is full use the following define to delete older
// entries. The define is the number of age cycles to go back in time.
#define DETECT_TABLE_OLDER 10000

// This define indicates how often we will check memory.
#define DETECT_CHECK_MEMORY 1000

// If this define is 1 then new allocated memory will be filled
// with DETECT_NEW bytes.
#define DETECT_GARBLE_NEW 1

// If this define is 1 then freed memory will be filled
// with DETECT_FREE bytes.
#define DETECT_GARBLE_FREE 1

#if DETECT_USE_TABLE
static int first_free_idx = -1;
static unsigned long global_age = 0;
static csMemTrackerEntry mem_table[DETECT_TABLE_SIZE];

//=======================================================
// If the table is not initialized, initialize it here.
//=======================================================
static void InitFreeMemEntries ()
{
  if (first_free_idx >= 0) return;
  int i;
  for (i = 0 ; i < DETECT_TABLE_SIZE ; i++)
  {
    mem_table[i].start = 0;
    mem_table[i].size = 0;
    mem_table[i].freed = true;
  }
  first_free_idx = 0;
}

//=======================================================
// Compact the table by removing all entries that are 0
// and shifting the others upwards.
//=======================================================
static void CompactMemEntries ()
{
  csPrintf ("Basic compact of memory table!\n"); fflush (stdout);
  InitFreeMemEntries ();
  // Compact the table.
  int i, j;
  for (i = j = 0 ; i < DETECT_TABLE_SIZE ; i++)
  {
    if (mem_table[i].start != 0)
    {
      if (j != i) mem_table[j] = mem_table[i];
      j++;
    }
  }
  for (i = j ; i < DETECT_TABLE_SIZE ; i++)
    mem_table[i].start = 0;
  first_free_idx = j;
}

//=======================================================
// Compact the table by removing all freed memory entries
// that are older that the given older_age.
//=======================================================
static void CompactMemEntries (unsigned long older_age)
{
  csPrintf ("Extended compact of memory table!\n"); fflush (stdout);
  InitFreeMemEntries ();
  // Compact the table.
  int i, j;
  for (i = j = 0 ; i < DETECT_TABLE_SIZE ; i++)
  {
    if (mem_table[i].start != 0)
    {
      if (mem_table[i].freed && mem_table[i].age < older_age)
      {
        cs_free (mem_table[i].start-DETECT_WALL-4);
      }
      else
      {
        if (j != i) mem_table[j] = mem_table[i];
        j++;
      }
    }
  }
  for (i = j ; i < DETECT_TABLE_SIZE ; i++)
    mem_table[i].start = 0;
  first_free_idx = j;
}

//=======================================================
// Find a free memory entry.
// If needed the table will be compacted.
//=======================================================
static csMemTrackerEntry& FindFreeMemEntry ()
{
  InitFreeMemEntries ();
  if (first_free_idx >= DETECT_TABLE_SIZE)
  {
    CompactMemEntries ();
    if (first_free_idx >= DETECT_TABLE_SIZE-20)
    {
      // There is still too little space left so we
      // compact again.
      unsigned long older_age = global_age;
      if (older_age >= DETECT_TABLE_OLDER)
        older_age -= DETECT_TABLE_OLDER;
      else older_age = 0;
      for (;;)
      {
        CompactMemEntries (older_age);
        if (first_free_idx >= DETECT_TABLE_SIZE-5)
        {
          older_age += DETECT_TABLE_OLDER/10;
          if (older_age > global_age-10)
          {
            csPrintf ("Increase DETECT_TABLE_SIZE!\n");
            fflush (stdout);
            exit (0);
          }
        }
        else
        {
          break;
        }
      }
    }
  }
  first_free_idx++;
  return mem_table[first_free_idx-1];
}

//=======================================================
// Find the memory entry for the given memory.
//=======================================================
static csMemTrackerEntry* FindMemEntry (char* mem)
{
  int i;
  for (i = 0 ; i < first_free_idx ; i++)
  {
    if (mem == mem_table[i].start) return mem_table+i;
  }
  return 0;
}

//=======================================================
// Show block info when a crash occurs.
//=======================================================
static void ShowBlockInfo (csMemTrackerEntry& me)
{
  csPrintf ("BLOCK: start=%08p size=%zu freed=%d\n", me.start,
        me.size, (int)me.freed);
# ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
  csPrintf ("       alloced at %s %d\n", CS::Quote::Single (me.alloc_file), me.alloc_line);
# endif
}

//=======================================================
// Do a sanity check on all entries in the memory table.
// Check if freed memory is still containing the contents
// we put there. Check if the walls are intact.
//=======================================================
static void MemoryCheck ()
{
  csPrintf ("Checking memory (age=%lu)...\n", global_age); fflush (stdout);
  int i;
  for (i = 0 ; i < first_free_idx ; i++)
  {
    csMemTrackerEntry& me = mem_table[i];
    if (me.start != 0)
    {
      char* rc = me.start - DETECT_WALL - 4;
      size_t s;
      memcpy (&s, rc+DETECT_WALL, 4);
      if (s != me.size)
      {
        ShowBlockInfo (me);
        csPrintf("CHK: Size in table does not correspond to size in block!\n");
        fflush (stdout);
        CS_DEBUG_BREAK;
      }
      if (me.freed)
      {
        if (strncmp (rc, DETECTFREE, DETECT_WALL) != 0)
        {
          ShowBlockInfo (me);
          csPrintf ("CHK: Bad start of block for freed block!\n");
          fflush (stdout);
          CS_DEBUG_BREAK;
        }
        if (strncmp (rc+4+DETECT_WALL+s, DETECTFREE, DETECT_WALL) != 0)
        {
          ShowBlockInfo (me);
          csPrintf ("CHK: Bad end of block for freed block!\n");
          fflush (stdout);
          CS_DEBUG_BREAK;
        }
#if DETECT_KEEP_FREE_MEMORY
        unsigned int j;
        for (j = 0 ; j < s ; j++)
        {
          if (me.start[j] != (char)DETECT_FREE)
          {
            ShowBlockInfo (me);
            csPrintf ("CHK: Freed memory is used at offset (%u)!\n", j);
            fflush (stdout);
            CS_DEBUG_BREAK;
          }
        }
#endif
      }
      else
      {
        if (strncmp (rc, DETECT, DETECT_WALL_SAME) != 0)
        {
          ShowBlockInfo (me);
          csPrintf ("CHK: Bad start of block!\n");
          fflush (stdout);
          CS_DEBUG_BREAK;
        }
        if (strncmp (rc+4+DETECT_WALL+s, DETECT, DETECT_WALL_SAME) != 0)
        {
          ShowBlockInfo (me);
          csPrintf ("CHK: Bad end of block!\n");
          fflush (stdout);
          CS_DEBUG_BREAK;
        }
      }
    }
  }
}
#endif // DETECT_USE_TABLE

//=======================================================
// Dump error
//=======================================================
static void DumpError (const char* msg, int info, char* rc)
{
  bool do_crash = true;
#if DETECT_USE_TABLE
  if (rc)
  {
    csMemTrackerEntry* me = FindMemEntry (rc);
    if (me)
      ShowBlockInfo (*me);
    else
    {
      csPrintf ("Memory block not allocated in this module!\n");
      do_crash = false;
    }
  }
  else
  {
    csPrintf ("No block info!\n");
  }
#endif
  csPrintf (msg, info);
  fflush (stdout);
  if (do_crash)
  {
    CS_DEBUG_BREAK;
  }
}

#ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
#undef new
void* operator new (size_t s, void* filename, int line)
#else
void* operator new (size_t s)
#endif
{
#if DETECT_USE_TABLE
  global_age++;
  if (global_age % DETECT_CHECK_MEMORY == 0) MemoryCheck ();
#endif
  if (s <= 0) DumpError ("BAD SIZE in new %zu\n", s, 0);
  char* rc = (char*)cs_malloc (s+4+DETECT_WALL+DETECT_WALL);
  memcpy (rc, DETECT, DETECT_WALL);
  memcpy (rc+DETECT_WALL, &s, 4);
  memcpy (rc+DETECT_WALL+4+s, DETECT, DETECT_WALL);
#if DETECT_GARBLE_NEW
  memset ((void*)(rc+4+DETECT_WALL), DETECT_NEW, s);
#endif
#if DETECT_USE_TABLE
  csMemTrackerEntry& me = FindFreeMemEntry ();
  me.start = rc+4+DETECT_WALL;
  me.size = s;
  me.freed = false;
# ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
  me.alloc_file = (char*)filename;
  me.alloc_line = line;
# endif
#endif
  return (void*)(rc+4+DETECT_WALL);
}

#ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
void* operator new[] (size_t s, void* filename, int line)
#else
void* operator new[] (size_t s)
#endif
{
#if DETECT_USE_TABLE
  global_age++;
  if (global_age % DETECT_CHECK_MEMORY == 0) MemoryCheck ();
#endif
  if (s <= 0) DumpError ("BAD SIZE in new[] %zu\n", s, 0);
  char* rc = (char*)cs_malloc (s+4+DETECT_WALL+DETECT_WALL);
  memcpy (rc, DETECTAR, DETECT_WALL);
  memcpy (rc+DETECT_WALL, &s, 4);
  memcpy (rc+DETECT_WALL+4+s, DETECTAR, DETECT_WALL);
#if DETECT_GARBLE_NEW
  memset ((void*)(rc+4+DETECT_WALL), DETECT_NEW, s);
#endif
#if DETECT_USE_TABLE
  csMemTrackerEntry& me = FindFreeMemEntry ();
  me.start = rc+4+DETECT_WALL;
  me.size = s;
  me.freed = false;
# ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
  me.alloc_file = (char*)filename;
  me.alloc_line = line;
# endif
#endif
  return (void*)(rc+4+DETECT_WALL);
}

void operator delete (void* p)
{
  if (!p) return;
#if DETECT_USE_TABLE
  global_age++;
  if (global_age % DETECT_CHECK_MEMORY == 0) MemoryCheck ();
#endif
  char* rc = (char*)p;
  rc -= 4+DETECT_WALL;
  size_t s;
  memcpy (&s, rc+DETECT_WALL, 4);
  if (strncmp (rc, DETECT, DETECT_WALL) != 0)
    DumpError ("operator delete: BAD START!\n", 0, (char*)p);
  if (strncmp (rc+4+DETECT_WALL+s, DETECT, DETECT_WALL) != 0)
    DumpError ("operator delete: BAD END!\n", 0, (char*)p);
  memcpy (rc, DETECTFREE, DETECT_WALL);
  memcpy (rc+4+s+DETECT_WALL, DETECTFREE, DETECT_WALL);

#if DETECT_GARBLE_FREE
  memset ((void*)(rc+4+DETECT_WALL), DETECT_FREE, s);
#endif

#if DETECT_USE_TABLE
  csMemTrackerEntry* me = FindMemEntry (rc+4+DETECT_WALL);
  if (!me)
    DumpError ("ERROR! Can't find memory entry for this block!\n", 0,
      (char*)p);
  if (me->size != s)
    DumpError("ERROR! Size in table does not correspond with size in block!\n",
        0, (char*)p);
  if (me->freed)
    DumpError ("ERROR! According to table memory is already freed!\n", 0,
      (char*)p);
# if DETECT_KEEP_FREE_MEMORY
  me->freed = true;
  me->age = global_age;
# else
  me->start = 0;
  cs_free (rc);
# endif
#else
  cs_free (rc);
#endif
}

void operator delete[] (void* p)
{
  if (!p) return;
#if DETECT_USE_TABLE
  global_age++;
  if (global_age % DETECT_CHECK_MEMORY == 0) MemoryCheck ();
#endif
  char* rc = (char*)p;
  rc -= 4+DETECT_WALL;
  size_t s;
  memcpy (&s, rc+DETECT_WALL, 4);
  if (strncmp (rc, DETECTAR, DETECT_WALL) != 0)
    DumpError ("operator delete[]: BAD START!\n", 0, (char*)p);
  if (strncmp (rc+4+DETECT_WALL+s, DETECTAR, DETECT_WALL) != 0)
    DumpError ("operator delete[]: BAD END!\n", 0, (char*)p);
  memcpy (rc, DETECTFREE, DETECT_WALL);
  memcpy (rc+4+s+DETECT_WALL, DETECTFREE, DETECT_WALL);
#if DETECT_GARBLE_FREE
  memset ((void*)(rc+4+DETECT_WALL), DETECT_FREE, s);
#endif

#if DETECT_USE_TABLE
  csMemTrackerEntry* me = FindMemEntry (rc+4+DETECT_WALL);
  if (!me)
    DumpError ("ERROR! Can't find memory entry for this block!\n", 0,
      (char*)p);
  if (me->size != s)
    DumpError("ERROR! Size in table does not correspond with size in block!\n",
        0, (char*)p);
  if (me->freed)
    DumpError ("ERROR! According to table memory is already freed!\n", 0,
        (char*)p);
# if DETECT_KEEP_FREE_MEMORY
  me->freed = true;
  me->age = global_age;
# else
  me->start = 0;
  cs_free (rc);
# endif
#else
  cs_free (rc);
#endif
}

#elif defined(MEMDEBUG_CHECKALLOC)      // CS_COMPILER_MSVC
//========================================================================
// This alternative branch allows for checking allocated memory amounts.
//========================================================================

#ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
#undef new
static size_t alloc_total = 0;
static size_t alloc_cnt = 0;
void* operator new (size_t s, void* filename, int line)
{
  alloc_total += s;
  alloc_cnt++;
  uint32* rc = (uint32*)cs_malloc (s+8);
  *rc++ = 0xdeadbeef;
  *rc++ = s;
  csPrintf ("+ %p %zu %zu %s\n",
            &alloc_total, alloc_total, alloc_cnt, filename);
  fflush (stdout);
  return (void*)rc;
}
void* operator new[] (size_t s, void* filename, int line)
{
  alloc_total += s;
  alloc_cnt++;
  uint32* rc = (uint32*)cs_malloc (s+8);
  *rc++ = 0xdeadbeef;
  *rc++ = s;
  csPrintf ("+ %p %zu %zu %s\n",
            &alloc_total, alloc_total, alloc_cnt, filename);
  fflush (stdout);
  return (void*)rc;
}
void operator delete (void* p)
{
  if (p)
  {
    uint32* rc = ((uint32*)p)-2;
    CS_ASSERT (*rc == 0xdeadbeef);
    alloc_total -= rc[1];
    alloc_cnt--;
    cs_free ((void*)rc);
    csPrintf ("- %p %zu %zu\n", &alloc_total, alloc_total, alloc_cnt);
  }
}
void operator delete[] (void* p)
{
  if (p)
  {
    uint32* rc = ((uint32*)p)-2;
    CS_ASSERT (*rc == 0xdeadbeef);
    alloc_total -= rc[1];
    alloc_cnt--;
    cs_free ((void*)rc);
    csPrintf ("- %p %zu %zu\n", &alloc_total, alloc_total, alloc_cnt);
  }
}
#endif  // CS_EXTENSIVE_MEMDEBUG_IMPLEMENT

#elif defined(MEMDEBUG_DUMPALLOC)       // CS_COMPILER_MSVC
//========================================================================
// This alternative branch allows for dumping all memory allocations.
//========================================================================

#ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
#undef new
static size_t alloc_total = 0;
static size_t alloc_cnt = 0;
void* operator new (size_t s, void* filename, int line)
{
  alloc_total += s;
  alloc_cnt++;
  if (s > 1000) { csPrintf ("new s=%zu tot=%zu/%zu file=%s line=%d\n",
        s, alloc_total, alloc_cnt, filename, line); fflush (stdout); }
  return (void*)cs_malloc (s);
}
void* operator new[] (size_t s, void* filename, int line)
{
  alloc_total += s;
  alloc_cnt++;
  if (s > 1000)
    csPrintf ("new[] s=%zu tot=%zu/%zu file=%s line=%d\n",
              s, alloc_total, alloc_cnt, filename, line); fflush (stdout);
  return (void*)cs_malloc (s);
}
void operator delete (void* p)
{
  if (p) cs_free (p);
}
void operator delete[] (void* p)
{
  if (p) cs_free (p);
}
#endif  // CS_EXTENSIVE_MEMDEBUG_IMPLEMENT

#else   // CS_COMPILER_MSVC
//========================================================================
// If CS_EXTENSIVE_MEMDEBUG is defined we still have to provide
// the correct overloaded operators even if we don't do debugging.
//========================================================================

#ifdef CS_EXTENSIVE_MEMDEBUG_IMPLEMENT
#undef new
CS_CRYSTALSPACE_EXPORT void* operator new (size_t s, void* f, int l)
{
#ifdef CS_CHECKING_ALLOCATIONS
  return ptmalloc_checking (s);
#else
  return ptmalloc_located (s);
#endif
}
CS_CRYSTALSPACE_EXPORT void* operator new[] (size_t s, void* f, int l)
{
#ifdef CS_CHECKING_ALLOCATIONS
  return ptmalloc_checking (s);
#else
  return ptmalloc_located (s);
#endif
}
CS_CRYSTALSPACE_EXPORT void operator delete (void* p)
{
#ifdef CS_CHECKING_ALLOCATIONS
  return ptfree_checking (p);
#else
  return ptfree_located (p);
#endif
}
CS_CRYSTALSPACE_EXPORT void operator delete[] (void* p)
{
#ifdef CS_CHECKING_ALLOCATIONS
  return ptfree_checking (p);
#else
  return ptfree_located (p);
#endif
}
#endif  // CS_EXTENSIVE_MEMDEBUG_IMPLEMENT

#endif  // CS_COMPILER_MSVC

//#elif defined(MEMDEBUG_MEMORY_TRACKER)
//#endif  // CS_COMPILER_MSVC

#if defined(MEMDEBUG_MEMORY_TRACKER)

#ifdef CS_MEMORY_TRACKER_IMPLEMENT

#define CS_MEMORY_TRACKER // to get iSCF::object_reg
#include "csutil/scf.h"
#undef CS_MEMORY_TRACKER

#include "iutil/objreg.h"
#include "csutil/hash.h"
#include "csutil/memdebug.h"
#include "iutil/memdebug.h"

#undef new

// This class is the memory tracker per module or application.
// csMemTrackerRegistry maintains a list of them.
class csMemTrackerModule
{
public:
  const char* Class;          // Name of class or 0 for application level.
  csArray<csMemTrackerInfo*, csArrayElementHandler<csMemTrackerInfo*>,
    CS::Memory::AllocatorMallocPlatform> mti_table;
  struct BlockInfo
  {
    size_t size;
    const char* info;
  };
  csHash<BlockInfo, void*, CS::Memory::AllocatorMallocPlatform> blockSizes;

  csMemTrackerModule ()
  {
  }

  void InsertBefore (int idx, const char* filename)
  {
    csMemTrackerInfo* mti =
      (csMemTrackerInfo*)malloc (sizeof (csMemTrackerInfo));
    mti->Init (filename);
    mti_table.Insert (idx, mti);
  }

  csMemTrackerInfo* FindInsertMtiTableEntry (
        const char* filename, int start, int end)
  {
    // Binary search.
    if (mti_table.GetSize() == 0)
    {
      csMemTrackerInfo* mti =
        (csMemTrackerInfo*)malloc (sizeof (csMemTrackerInfo));
      mti->Init (filename);
      mti_table.Push (mti);
      return mti;
    }

    if (start == end)
    {
      int rc = strcmp (filename, mti_table[start]->file);
      if (rc == 0) return mti_table[start];
      if (rc < 0)
      {
        InsertBefore (start, filename);
        return mti_table[start];
      }
      else
      {
        InsertBefore (start+1, filename);
        return mti_table[start+1];
      }
    }
    else if (start+1 == end)
    {
      int rc1 = strcmp (filename, mti_table[start]->file);
      if (rc1 == 0) return mti_table[start];
      if (rc1 < 0)
      {
        InsertBefore (start, filename);
        return mti_table[start];
      }

      int rc2 = strcmp (filename, mti_table[end]->file);
      if (rc2 == 0) return mti_table[end];
      if (rc2 > 0)
      {
        InsertBefore (end+1, filename);
        return mti_table[end+1];
      }
      InsertBefore (start+1, filename);
      return mti_table[start+1];
    }
    else
    {
      int mid = (start+end)/2;
      int rc = strcmp (filename, mti_table[mid]->file);
      if (rc == 0) return mti_table[mid];
      if (rc < 0) return FindInsertMtiTableEntry (filename, start, mid-1);
      return FindInsertMtiTableEntry (filename, mid+1, end);
    }
  }

  csMemTrackerInfo* FindInsertMtiTableEntry (const char* filename)
  {
    return FindInsertMtiTableEntry (filename, 0, mti_table.GetSize()-1);
  }

  void Dump (bool summary_only)
  {
    size_t i;
    csPrintf ("-----------------------------------------------------\n");
    csPrintf ("Module: %s\n", Class);
    csPrintf ("       bytes   ...max   blocks   ...max #alloc #free  file\n");
    size_t total_current_alloc = 0;
    int total_current_count = 0;
    for (i = 0 ; i < mti_table.GetSize(); i++)
    {
      csMemTrackerInfo* mti = mti_table[i];
      if (!summary_only)
      {
        csPrintf ("    %8zu %8zu %8d %8d %6zu %6zu %s\n", mti->current_alloc,
            mti->max_alloc, mti->current_count, mti->max_count,
            mti->totalAllocCount, mti->totalDeallocCount, mti->file);
      }
      total_current_alloc += mti->current_alloc;
      total_current_count += mti->current_count;
    }
  #define Sigma "\xE2\x88\x91"
    csPrintf (Sigma "bytes=%zu " Sigma "blocks=%d Module=%s\n",
        total_current_alloc, total_current_count, Class);
  #undef Sigma
    fflush (stdout);
  }
};


// The following machinery is needed to try to keep track of memory
// allocations in different plugins.
class csMemTrackerRegistry : public scfImplementation1<csMemTrackerRegistry, 
                                                       iMemoryTracker>
{
public:
  static const int max_modules = 500;
  csMemTrackerModule* modules[max_modules];     // @@@ Hardcoded!
  int num_modules;

  csMemTrackerRegistry ()
    : scfImplementationType (this), num_modules (0)
  {
  }

  virtual ~csMemTrackerRegistry ()
  {
  }

  csMemTrackerModule* NewMemTrackerModule (const char* Class)
  {
    csMemTrackerModule* mod = new csMemTrackerModule ();
    mod->Class = Class;
    modules[num_modules++] = mod;
    CS_ASSERT(num_modules <= max_modules);
    return mod;
  }

  virtual void Dump (bool summary_only)
  {
#if defined(CS_PLATFORM_WIN32)
    /* Yes, that's horribly platform-dependent, and this may not be the right
     * place. However, the heap statistics are somewhat useful, since they
     * can e.g. indicate fragmentation. */
    DWORD heapNum = GetProcessHeaps (0, 0);
    CS_ALLOC_STACK_ARRAY(HANDLE, heaps, heapNum);
    GetProcessHeaps (heapNum, heaps);

    for (DWORD i = 0; i < heapNum; i++)
    {
      const HANDLE heap = heaps[i];

      struct RegionInfo
      {
        size_t committedSize;
        size_t uncommittedSize;
        size_t busySize;
        size_t blocks;

        RegionInfo() : committedSize (0), uncommittedSize (0),
          busySize (0), blocks (0) {}
      };
      RegionInfo regions[256];

      PROCESS_HEAP_ENTRY entry;
      entry.lpData = 0;
      bool hasEntry = HeapWalk (heap, &entry) != FALSE;
      while (hasEntry)
      {
        RegionInfo& region = regions[entry.iRegionIndex];
        if (entry.wFlags & PROCESS_HEAP_REGION)
        {
          region.committedSize = entry.Region.dwCommittedSize;
          region.uncommittedSize = entry.Region.dwUnCommittedSize;
        }
        else if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
        {
          region.busySize += entry.cbData;
        }
        if (!(entry.wFlags & (PROCESS_HEAP_REGION | PROCESS_HEAP_UNCOMMITTED_RANGE)))
          region.blocks++;
        hasEntry = HeapWalk (heap, &entry) != FALSE;
      }
      for (int i = 0; i < 256; i++)
      {
        const RegionInfo& region = regions[i];
        if ((region.committedSize + region.uncommittedSize) == 0) continue;
        csPrintf ("heap %p region %d: %9zu/%9zu/%9zu bytes in %6zu blocks\n", (void*)heap, i,
          region.committedSize, region.uncommittedSize, region.busySize, region.blocks);
      }
    }
    fflush (stdout);
#endif

    int i;
    for (i = 0 ; i < num_modules ; i++)
    {
      modules[i]->Dump (summary_only);
    }
  }
};


static csMemTrackerModule* mti_this_module;

void mtiRegisterModule (const char* Class)
{
  CS::Debug::MemTracker::Impl::RegisterModule (mti_this_module,
    Class);
}

csMemTrackerInfo* mtiRegisterAlloc (size_t s, const char* filename)
{
  if (mti_this_module == 0)
    return 0;   // Don't track this alloc yet.

  csMemTrackerInfo* mti = mti_this_module->FindInsertMtiTableEntry (
        filename);
  mti->current_count++;
  mti->current_alloc += s;
  if (mti->current_count > mti->max_count)
    mti->max_count = mti->current_count;
  if (mti->current_alloc > mti->max_alloc)
    mti->max_alloc = mti->current_alloc;
  mti->totalAllocCount++;
  return mti;
}

csMemTrackerInfo* mtiRegister (const char* info)
{
  if (!mti_this_module) return 0;
  csMemTrackerInfo* mti = mti_this_module->FindInsertMtiTableEntry (
        info);
  return mti;
}

void mtiRegisterFree (csMemTrackerInfo* mti, size_t s)
{
  if (mti)
  {
    mti->current_count--;
    mti->current_alloc -= s;
    mti->totalDeallocCount++;
  }
}

void mtiUpdateAmount (csMemTrackerInfo* mti, int dcount, int dsize)
{
  if (mti)
  {
    mti->current_count += dcount;
    mti->current_alloc += dsize;
    if (mti->current_count > mti->max_count)
      mti->max_count = mti->current_count;
    if (mti->current_alloc > mti->max_alloc)
      mti->max_alloc = mti->current_alloc;
    if (dcount > 0)
      mti->totalAllocCount++;
    else if (dcount < 0)
      mti->totalDeallocCount++;
  }
}

namespace CS
{
  namespace Debug
  {
    namespace MemTracker
    {
      namespace Impl
      {
	void RegisterAlloc (csMemTrackerModule* m, void* p, size_t s, const char* info)
	{
	  if (!m) return;
	  csMemTrackerInfo* mti = m->FindInsertMtiTableEntry (info);
	  mti->current_count++;
	  mti->current_alloc += s;
	  if (mti->current_count > mti->max_count)
	    mti->max_count = mti->current_count;
	  if (mti->current_alloc > mti->max_alloc)
	    mti->max_alloc = mti->current_alloc;
	  mti->totalAllocCount++;
	  
	  csMemTrackerModule::BlockInfo bi;
	  bi.size = s;
	  bi.info = info;
	  m->blockSizes.Put (p, bi);
	}
	void RegisterModule (csMemTrackerModule*& m, const char* Class)
	{
	  if (!iSCF::SCF)
	  {
	    csPrintf ("%s for %s: iSCF::SCF not set yet!\n",
	      CS_FUNCTION_NAME, Class);
	    return;
	  }
	
	  if (iSCF::SCF->object_reg)
	  {
	    csRef<iMemoryTracker> mtiTR = csQueryRegistryTagInterface<iMemoryTracker> (
		iSCF::SCF->object_reg, "crystalspace.utilities.memorytracker");
	    if (!mtiTR)
	    {
	      mtiTR.AttachNew (new csMemTrackerRegistry);
	      iSCF::SCF->object_reg->Register (mtiTR,
		"crystalspace.utilities.memorytracker");
	    }
	    m = (static_cast<csMemTrackerRegistry*> (
	      (iMemoryTracker*)mtiTR))->NewMemTrackerModule (Class);
	  }
	  else
	  {
	    csPrintf ("Object Reg not set for %s!!!\n", Class);
	    fflush (stdout);
	  }
	}
	void RegisterFree (csMemTrackerModule* m, void* p)
	{
	  if (!m) return;
	  csMemTrackerModule::BlockInfo* bi =
	    m->blockSizes.GetElementPointer (p);
	  if (!bi) return;
	  size_t s = bi->size;
	  csMemTrackerInfo* mti = m->FindInsertMtiTableEntry (bi->info);
	  mti->current_count--;
	  mti->current_alloc -= s;
	  mti->totalDeallocCount++;
	  m->blockSizes.DeleteAll (p);
	}
	void UpdateSize (csMemTrackerModule* m, void* p,
	  void* newP, size_t newSize)
	{
	  if (!m) return;
	  csMemTrackerModule::BlockInfo* bi =
	    m->blockSizes.GetElementPointer (p);
	  if (!bi) return;
	  size_t s = bi->size;
	  csMemTrackerInfo* mti = m->FindInsertMtiTableEntry (bi->info);
	  mti->current_alloc -= s;
          mti->current_alloc += newSize;
	  if (mti->current_alloc > mti->max_alloc)
	    mti->max_alloc = mti->current_alloc;
          csMemTrackerModule::BlockInfo newBI;
          newBI.size = newSize;
          newBI.info = bi->info;
	  m->blockSizes.DeleteAll (p);
          m->blockSizes.Put (newP, newBI);
	}
	void UpdateAmount (csMemTrackerModule* m, const char* info,
	  int dcount, int dsize)
	{
	  if (!m) return;
	  csMemTrackerInfo* mti = m->FindInsertMtiTableEntry (info);
	  mti->current_count += dcount;
	  mti->current_alloc += dsize;
	  if (mti->current_count > mti->max_count)
	    mti->max_count = mti->current_count;
	  if (mti->current_alloc > mti->max_alloc)
	    mti->max_alloc = mti->current_alloc;
	  if (dcount > 0)
	    mti->totalAllocCount++;
	  else if (dcount < 0)
	    mti->totalDeallocCount++;
	}
	const char* GetInfo (csMemTrackerModule* m, void* p)
	{
	  if (!m) return 0;
	  csMemTrackerModule::BlockInfo* bi =
	    m->blockSizes.GetElementPointer (p);
	  if (!bi) return 0;
	  return bi->info;
	}
      } // namespace Impl
    } // namespace MemTracker
  } // namespace Debug
} // namespace CS


static void* memdebug_malloc (size_t s)
{ return ptmalloc_sentinel (s); }
static void memdebug_free (void* p)
{ return ptfree_sentinel (p); }

CS_CRYSTALSPACE_EXPORT void* operator new (size_t s, void* filename, int /*line*/)
{
  void* rc = memdebug_malloc (s);
  CS::Debug::MemTracker::RegisterAlloc (rc, s, (const char*)filename);
  return rc;
}
CS_CRYSTALSPACE_EXPORT void* operator new[] (size_t s, void* filename, int /*line*/)
{
  void* rc = memdebug_malloc (s);
  CS::Debug::MemTracker::RegisterAlloc (rc, s, (const char*)filename);
  return rc;
}
CS_CRYSTALSPACE_EXPORT void operator delete (void* p)
{
  if (p)
  {
    memdebug_free (p);
    CS::Debug::MemTracker::RegisterFree (p);
  }
}
CS_CRYSTALSPACE_EXPORT void operator delete[] (void* p)
{
  if (p)
  {
    memdebug_free (p);
    CS::Debug::MemTracker::RegisterFree (p);
  }
}

#endif // CS_MEMORY_TRACKER_IMPLEMENT

#endif // defined(MEMDEBUG_MEMORY_TRACKER)
