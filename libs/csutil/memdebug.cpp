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
#include "cssysdef.h"

#if 0

//========================================================================
// CONFIGURATION OF THE MEMORY DEBUGGER:

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
#define DETECTAR    "ABCDabcd+09876543210"	// 8 first bytes same as DETECT
#define DETECTFREE  "FreeFreeFreeFreeFree"
#define DETECT_NEW  0xda
#define DETECT_FREE 0x9d

struct MemEntry
{
  char* start;		// NULL if not used or else pointer to memory.
  size_t size;
  bool freed;		// If true then this memory entry is freed.
  unsigned long age;	// Time when 'free' was done.
  char* alloc_file;	// Filename at allocation time.
  int alloc_line;	// Line number.
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
static MemEntry mem_table[DETECT_TABLE_SIZE];

//=======================================================
// If the table is not initialized, initialize it here.
//=======================================================
static void InitFreeMemEntries ()
{
  if (first_free_idx >= 0) return;
  int i;
  for (i = 0 ; i < DETECT_TABLE_SIZE ; i++)
  {
    mem_table[i].start = NULL;
    mem_table[i].size = 0;
    mem_table[i].freed = true;
  }
  first_free_idx = 0;
}

//=======================================================
// Compact the table by removing all entries that are NULL
// and shifting the others upwards.
//=======================================================
static void CompactMemEntries ()
{
  printf ("Basic compact of memory table!\n"); fflush (stdout);
  InitFreeMemEntries ();
  // Compact the table.
  int i, j;
  for (i = j = 0 ; i < DETECT_TABLE_SIZE ; i++)
  {
    if (mem_table[i].start != NULL)
    {
      if (j != i) mem_table[j] = mem_table[i];
      j++;
    }
  }
  for (i = j ; i < DETECT_TABLE_SIZE ; i++)
    mem_table[i].start = NULL;
  first_free_idx = j;
}

//=======================================================
// Compact the table by removing all freed memory entries
// that are older that the given older_age.
//=======================================================
static void CompactMemEntries (unsigned long older_age)
{
  printf ("Extended compact of memory table!\n"); fflush (stdout);
  InitFreeMemEntries ();
  // Compact the table.
  int i, j;
  for (i = j = 0 ; i < DETECT_TABLE_SIZE ; i++)
  {
    if (mem_table[i].start != NULL)
    {
      if (mem_table[i].freed && mem_table[i].age < older_age)
      {
        free (mem_table[i].start-DETECT_WALL-4);
      }
      else
      {
        if (j != i) mem_table[j] = mem_table[i];
        j++;
      }
    }
  }
  for (i = j ; i < DETECT_TABLE_SIZE ; i++)
    mem_table[i].start = NULL;
  first_free_idx = j;
}

//=======================================================
// Find a free memory entry.
// If needed the table will be compacted.
//=======================================================
static MemEntry& FindFreeMemEntry ()
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
	    printf ("Increase DETECT_TABLE_SIZE!\n");
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
static MemEntry* FindMemEntry (char* mem)
{
  int i;
  for (i = 0 ; i < first_free_idx ; i++)
  {
    if (mem == mem_table[i].start) return mem_table+i;
  }
  return NULL;
}

//=======================================================
// Show block info when a crash occurs.
//=======================================================
static void ShowBlockInfo (MemEntry& me)
{
  printf ("BLOCK: start=%08lx size=%d freed=%d\n", (long)me.start,
  	me.size, me.freed);
# if CS_EXTENSIVE_MEMDEBUG
  printf ("       alloced at '%s' %d\n", me.alloc_file, me.alloc_line);
# endif
}

//=======================================================
// Do a sanity check on all entries in the memory table.
// Check if freed memory is still containing the contents
// we put there. Check if the walls are intact.
//=======================================================
static void MemoryCheck ()
{
  printf ("Checking memory (age=%ld)...\n", global_age); fflush (stdout);
  int i;
  for (i = 0 ; i < first_free_idx ; i++)
  {
    MemEntry& me = mem_table[i];
    if (me.start != NULL)
    {
      char* rc = me.start - DETECT_WALL - 4;
      size_t s;
      memcpy (&s, rc+DETECT_WALL, 4);
      if (s != me.size)
      {
	ShowBlockInfo (me);
        printf ("CHK: Size in table doesn't correspond with size in block!\n");
	fflush (stdout);
	CRASH;
      }
      if (me.freed)
      {
	if (strncmp (rc, DETECTFREE, DETECT_WALL) != 0)
	{
	  ShowBlockInfo (me);
	  printf ("CHK: Bad start of block for freed block!\n");
	  fflush (stdout);
	  CRASH;
	}
	if (strncmp (rc+4+DETECT_WALL+s, DETECTFREE, DETECT_WALL) != 0)
	{
	  ShowBlockInfo (me);
	  printf ("CHK: Bad end of block for freed block!\n");
	  fflush (stdout);
	  CRASH;
	}
#       if DETECT_KEEP_FREE_MEMORY
        unsigned int j;
	for (j = 0 ; j < s ; j++)
	{
	  if (me.start[j] != (char)DETECT_FREE)
	  {
	    ShowBlockInfo (me);
	    printf ("CHK: Freed memory is used at offset (%d)!\n", j);
	    fflush (stdout);
	    CRASH;
	  }
	}
#       endif
      }
      else
      {
	if (strncmp (rc, DETECT, DETECT_WALL_SAME) != 0)
	{
	  ShowBlockInfo (me);
	  printf ("CHK: Bad start of block!\n");
	  fflush (stdout);
	  CRASH;
	}
	if (strncmp (rc+4+DETECT_WALL+s, DETECT, DETECT_WALL_SAME) != 0)
	{
	  ShowBlockInfo (me);
	  printf ("CHK: Bad end of block!\n");
	  fflush (stdout);
	  CRASH;
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
    MemEntry* me = FindMemEntry (rc);
    if (me)
      ShowBlockInfo (*me);
    else
    {
      printf ("Memory block not allocated in this module!\n");
      do_crash = false;
    }
  }
  else
  {
    printf ("No block info!\n");
  }
#endif
  printf (msg, info);
  fflush (stdout);
  if (do_crash)
  {
    CRASH;
  }
}

#if CS_EXTENSIVE_MEMDEBUG
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
  if (s <= 0) DumpError ("BAD SIZE in new %d\n", s, NULL);
  char* rc = (char*)malloc (s+4+DETECT_WALL+DETECT_WALL);
  memcpy (rc, DETECT, DETECT_WALL);
  memcpy (rc+DETECT_WALL, &s, 4);
  memcpy (rc+DETECT_WALL+4+s, DETECT, DETECT_WALL);
#if DETECT_GARBLE_NEW
  memset ((void*)(rc+4+DETECT_WALL), DETECT_NEW, s);
#endif
#if DETECT_USE_TABLE
  MemEntry& me = FindFreeMemEntry ();
  me.start = rc+4+DETECT_WALL;
  me.size = s;
  me.freed = false;
# if CS_EXTENSIVE_MEMDEBUG
  me.alloc_file = (char*)filename;
  me.alloc_line = line;
# endif
#endif
  return (void*)(rc+4+DETECT_WALL);
}

#if CS_EXTENSIVE_MEMDEBUG
void* operator new[] (size_t s, void* filename, int line)
#else
void* operator new[] (size_t s)
#endif
{
#if DETECT_USE_TABLE
  global_age++;
  if (global_age % DETECT_CHECK_MEMORY == 0) MemoryCheck ();
#endif
  if (s <= 0) DumpError ("BAD SIZE in new[] %d\n", s, NULL);
  char* rc = (char*)malloc (s+4+DETECT_WALL+DETECT_WALL);
  memcpy (rc, DETECTAR, DETECT_WALL);
  memcpy (rc+DETECT_WALL, &s, 4);
  memcpy (rc+DETECT_WALL+4+s, DETECTAR, DETECT_WALL);
#if DETECT_GARBLE_NEW
  memset ((void*)(rc+4+DETECT_WALL), DETECT_NEW, s);
#endif
#if DETECT_USE_TABLE
  MemEntry& me = FindFreeMemEntry ();
  me.start = rc+4+DETECT_WALL;
  me.size = s;
  me.freed = false;
# if CS_EXTENSIVE_MEMDEBUG
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
  MemEntry* me = FindMemEntry (rc+4+DETECT_WALL);
  if (!me)
    DumpError ("ERROR! Can't find memory entry for this block!\n", 0,
      (char*)p);
  if (me->size != s)
    DumpError ("ERROR! Size in table does not correspond with size in block!\n",
    	0, (char*)p);
  if (me->freed)
    DumpError ("ERROR! According to table memory is already freed!\n", 0,
      (char*)p);
# if DETECT_KEEP_FREE_MEMORY
  me->freed = true;
  me->age = global_age;
# else
  me->start = NULL;
  free (rc);
# endif
#else
  free (rc);
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
  MemEntry* me = FindMemEntry (rc+4+DETECT_WALL);
  if (!me)
    DumpError ("ERROR! Can't find memory entry for this block!\n", 0,
      (char*)p);
  if (me->size != s)
    DumpError ("ERROR! Size in table does not correspond with size in block!\n",
    	0, (char*)p);
  if (me->freed)
    DumpError ("ERROR! According to table memory is already freed!\n", 0,
    	(char*)p);
# if DETECT_KEEP_FREE_MEMORY
  me->freed = true;
  me->age = global_age;
# else
  me->start = NULL;
  free (rc);
# endif
#else
  free (rc);
#endif
}

#else

// If CS_EXTENSIVE_MEMDEBUG is defined we still have to provide
// the correct overloaded operators even if we don't do debugging.

#if CS_EXTENSIVE_MEMDEBUG
#undef new
void* operator new (size_t s, void*, int)
{
  return (void*)malloc (s);
}
void* operator new[] (size_t s, void*, int)
{
  return (void*)malloc (s);
}
void operator delete (void* p)
{
  if (p) free (p);
}
void operator delete[] (void* p)
{
  if (p) free (p);
}
#endif

#endif
