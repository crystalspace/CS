/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifdef MEM_CHECK

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include "sysdef.h"

#ifdef MEM_CHECK_EXTENSIVE
#  undef MEM_CHECK_FILL
#  define MEM_CHECK_FILL
#endif

#define WALL_SIZE 16
#define BEFOREWALL_STR	"SabcSdefSghiSjkl"
#define AFTERWALL_STR	"EabcEdefEghiEjkl"
#define MEMFILL_STR	"* this is garbage for filling memory *"

// Simple memory dump
void dump_memory (const char* p, size_t len)
{
  int i;
  const char* op = p;
  for (i = 0 ; i < (int)len ; i++)
  {
    printf ("%c", isprint (*p) ? *p : '.');
    p++;
  }
  p = op;
  printf ("    ");
  for (i = 0 ; i < (int)len ; i++)
  {
    printf ("%02x ", *p);
    p++;
  }
  printf ("\n");
}

// This structure is used to keep the list of all allocated memory blocks
struct csMemListEntry
{
  csMemListEntry* next, * prev;		// memory blocks chained together
  void* p;				// the pointer
  size_t size;				// size
  char* file;				// source file
  int line;				// lineno
  bool freed;				// freed already?
  char* free_file;			// where was freed
  int free_line;			// at which line
};

csMemListEntry *memlist = NULL;
int num_memlist = 0;
// rudimentary thread-safe lock (works not quite well)
static int mem_lock = 0;

// Remember the location of a new or delete in a static stack
#define LOC_STACK_SIZE	64
static struct
{
  char *file;
  int line;
} locstack [LOC_STACK_SIZE];
static int locstackptr = 0;

void PushLoc (char *iFileName, int iLineNo)
{
  if (locstackptr >= LOC_STACK_SIZE)
  {
    printf ("WARNING: MEMORY DEBUGGER LOCATION STACK EXCEEDED!\n");
    return;
  }
  size_t sl = strlen (iFileName) + 1;
  locstack [locstackptr].file = (char *)malloc (sl);
  memcpy (locstack [locstackptr].file, iFileName, sl);
  locstack [locstackptr].line = iLineNo;
  locstackptr++;
}

bool GetLoc (char **oFileName, int *oLineNo)
{
  if (!locstackptr || !locstack [locstackptr - 1].file)
  {
    *oFileName = NULL;
    *oLineNo = 0;
    return false;
  }
  *oFileName = locstack [locstackptr - 1].file;
  *oLineNo = locstack [locstackptr - 1].line;
  locstack [locstackptr - 1].file = NULL;
  return true;
}

void PopLoc ()
{
  if (!locstackptr)
  {
    printf ("WARNING: POPLOC() ON EMPTY LOCATION STACK!\n");
    return;
  }
  locstackptr--;
}

static void *__new (size_t s)
{
  while (mem_lock) ;
  mem_lock++;

  char *p = (char *)malloc (s + WALL_SIZE * 2);
  memcpy (p, BEFOREWALL_STR, WALL_SIZE);
  memcpy (p + s + WALL_SIZE, AFTERWALL_STR, WALL_SIZE);
  memset (p + WALL_SIZE, 'X', s);

  csMemListEntry* me = (csMemListEntry*)malloc (sizeof (csMemListEntry));
  me->p = (void*)(p+WALL_SIZE);
  me->size = s;
  me->next = memlist;
  me->prev = NULL;
  me->freed = false;
  if (!GetLoc (&me->file, &me->line))
    printf ("WARNING! Memory was allocated without CHK (%lu bytes)!\n", s);
  if (memlist)
    memlist->prev = me;
  memlist = me;
  num_memlist++;

  mem_lock--;
  return p + WALL_SIZE;
}

static void __delete (void* p)
{
  char *fname;
  int lineno;
  if (!GetLoc (&fname, &lineno))
    printf ("WARNING! Memory was freed without CHK!\n");

  if (!p) return;
  while (mem_lock) ;
  mem_lock++;

  char* pp;

  csMemListEntry* e = memlist;
  while (e)
  {
    if (e->p == p) break;
    e = e->next;
  }

#ifdef MEM_CHECK_EXTENSIVE
  if (e && e->freed)
  {
    printf ("ERROR! Block was already freed earlier!\n");
    printf ("       Error discovered while freeing memory at %s:%d.\n",
    	fname, lineno);
    printf ("       Block was originally allocated at %s:%d.\n",
    	e->line != -1 ? e->file : "", e->line);
    printf ("       Block was first freed at %s:%d.\n",
    	e->free_line != -1 ? e->free_file : "", e->free_line);
    mem_lock--;
    return;
  }
#endif

  if (e)
  {
    pp = ((char*)p)-WALL_SIZE;
    if (strncmp (pp, BEFOREWALL_STR, WALL_SIZE))
    {
      printf ("ERROR! Memory was overwritten before block allocated at %s:%d with size %lu!\n",
      	e->file ? e->file : "", e->line, e->size);
      printf ("       Overwritten with: ");
      dump_memory (pp, WALL_SIZE);
      printf ("       Should be       : ");
      dump_memory (BEFOREWALL_STR, WALL_SIZE);
      printf ("       Error discovered while freeing memory at %s:%d.\n",
    	fname, lineno);
    }
    if (strncmp (pp+e->size+WALL_SIZE, AFTERWALL_STR, WALL_SIZE))
    {
      printf ("ERROR! Memory was overwritten after block allocated at %s:%d with size %lu!\n",
      	e->file ? e->file : "", e->line, e->size);
      printf ("       Overwritten with: ");
      dump_memory (pp+e->size+WALL_SIZE, WALL_SIZE);
      printf ("       Should be       : ");
      dump_memory (AFTERWALL_STR, WALL_SIZE);
      printf ("       Error discovered while freeing memory at %s:%d.\n",
    	fname, lineno);
    }

#ifndef MEM_CHECK_EXTENSIVE
    free (pp);
    if (e->next) e->next->prev = e->prev;
    if (e->prev) e->prev->next = e->next;
    else memlist = e->next;
    if (e->file) free (e->file);
    free (e);
    num_memlist--;
#else
    e->freed = true;
    e->free_file = fname;
    e->free_line = lineno;

    // fill memory block with a test pattern
    char *fill = ((char *)e->p) - WALL_SIZE;
    size_t fill_count = e->size + WALL_SIZE * 2;
    while (fill_count)
    {
      const char *fill_str = MEMFILL_STR;
      int how_many = MIN (fill_count, strlen (fill_str));
      memmove (fill, fill_str, how_many);
      fill += how_many;
      fill_count -= how_many;
    } /* endwhile */
#endif
  }
  else
  {
    printf ("ERROR! Trying to free something that is not allocated!\n");
    printf ("       Error discovered while freeing memory at %s:%d.\n",
      fname, lineno);
  }
  mem_lock--;
}

void *operator new (size_t s)
{
  return __new (s);
}

void operator delete (void* p)
{
  __delete (p);
}

void *operator new [] (size_t s)
{
  return __new (s);
}

void operator delete [] (void* p)
{
  __delete (p);
}

#ifdef MEM_CHECK_EXTENSIVE

void check_mem ()
{
  int num = 0;
  csMemListEntry* e = memlist;
  while (e)
  {
    if (e->freed)
    {
      char *fill = ((char *)e->p) - WALL_SIZE;
      size_t fill_count = e->size + WALL_SIZE * 2;
      while (fill_count)
      {
        const char *fill_str = MEMFILL_STR;
        int how_many = MIN (fill_count, strlen (fill_str));
        if (memcmp (fill, fill_str, how_many))
        {
          printf ("ERROR! Memory was overwritten after block allocated at %s:%d with size %lu was freed!\n",
      	    e->file ? e->file : "", e->line, e->size);
          printf ("       Overwritten with: ");
          dump_memory (fill, 16);
          printf ("       Should have been: ");
          dump_memory (fill_str, 16);
          printf ("       Block was first freed at %s:%d.\n",
    	    e->free_line != -1 ? e->free_file : "", e->free_line);
	  break;
        } /* endif */
        fill += how_many;
        fill_count -= how_many;
      } /* endwhile */
    }
    else
      num++;
    e = e->next;
  }
  fflush (stdout);
}

class X { public: ~X () { dump_mem_list (); check_mem (); } } x;

#endif // MEM_CHECK_EXTENSIVE

void dump_mem_list ()
{
  if (locstackptr)
    printf ("WARNING: LOCATION STACK IS NOT EMPTY (%d deep)\n", locstackptr);
  printf ("------------------- memory block list -------------------\n");
  csMemListEntry *cur = memlist;
  while (cur)
  {
    if (!cur->freed)
      printf ("%08lX (%ld)  %s:%d\n", (unsigned long)cur->p, cur->size, cur->file, cur->line);
    cur = cur->next;
  } /* endwhile */
  printf ("---------------- end of memory block list ---------------\n");
  fflush (stdout);
}

#endif // MEM_CHECK
