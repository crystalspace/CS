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
#include "cssys/common/system.h"

extern csSystemDriver *System;

#ifdef MEM_CHECK_EXTENSIVE
#  undef MEM_CHECK_FILL
#  define MEM_CHECK_FILL
#endif

#define WALL_SIZE 16
#define BEFOREWALL_STR	"SabcSdefSghiSjkl"
#define AFTERWALL_STR	"EabcEdefEghiEjkl"
#define MEMFILL_STR	"* this is garbage for filling memory *"

volatile char MemFile[255];
volatile int MemLine = -1;
MemEntryA* entries = NULL;
int num_entries = 0;
// rudimentary thread-safe lock (works not quite well)
static int mem_lock = 0;

void* operator new (size_t s)
{
  while (mem_lock) ;
  mem_lock++;

  char *p = (char *)malloc (s + WALL_SIZE * 2);
  memcpy (p, BEFOREWALL_STR, WALL_SIZE);
  memcpy (p + s + WALL_SIZE, AFTERWALL_STR, WALL_SIZE);
  memset (p + WALL_SIZE, 'X', s);

  MemEntryA* me = (MemEntryA*)malloc (sizeof (MemEntryA));
  me->p = (void*)(p+WALL_SIZE);
  me->size = s;
  me->next = entries;
  me->prev = NULL;
  me->line = MemLine;
  me->file = NULL;
  me->freed = false;
  if (MemLine != -1)
  {
    me->file = (char*)malloc (strlen ((char *)MemFile)+1);
    strcpy (me->file, (char *)MemFile);
  }
  else
  {
    System->Printf (MSG_STDOUT, "WARNING! Memory was allocated without CHK (%d bytes)!\n", s);
    me->file = NULL;
  }
  if (entries) entries->prev = me;
  entries = me;
  num_entries++;

  mem_lock--;
  return (void *)(p + WALL_SIZE);
}

void dump_memory (const char* p, size_t len)
{
  int i;
  const char* op = p;
  for (i = 0 ; i < (int)len ; i++)
  {
    System->Printf (MSG_STDOUT, "%c", isprint (*p) ? *p : '.');
    p++;
  }
  p = op;
  System->Printf (MSG_STDOUT, "    ");
  for (i = 0 ; i < (int)len ; i++)
  {
    System->Printf (MSG_STDOUT, "%02x ", *p);
    p++;
  }
  System->Printf (MSG_STDOUT, "\n");
}

void operator delete (void* p)
{
  if (!p) return;
  while (mem_lock) ;
  mem_lock++;

  char* pp;

  MemEntryA* e = entries;
  while (e)
  {
    if (e->p == p) break;
    e = e->next;
  }

#ifdef MEM_CHECK_EXTENSIVE
  if (e && e->freed)
  {
    System->Printf (MSG_STDOUT, "ERROR! Block was already freed earlier!\n");
    System->Printf (MSG_STDOUT, "       Error discovered while freeing memory at %s:%d.\n",
    	MemLine != -1 ? MemFile : "", MemLine);
    System->Printf (MSG_STDOUT, "       Block was originally allocated at %s:%d.\n",
    	e->line != -1 ? e->file : "", e->line);
    System->Printf (MSG_STDOUT, "       Block was first freed at %s:%d.\n",
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
      System->Printf (MSG_STDOUT, "ERROR! Memory was overwritten before block allocated at %s:%d with size %d!\n",
      	e->file ? e->file : "", e->line, e->size);
      System->Printf (MSG_STDOUT, "       Overwritten with: ");
      dump_memory (pp, WALL_SIZE);
      System->Printf (MSG_STDOUT, "       Should be       : ");
      dump_memory (BEFOREWALL_STR, WALL_SIZE);
      System->Printf (MSG_STDOUT, "       Error discovered while freeing memory at %s:%d.\n",
    	MemLine != -1 ? MemFile : "", MemLine);
    }
    if (strncmp (pp+e->size+WALL_SIZE, AFTERWALL_STR, WALL_SIZE))
    {
      System->Printf (MSG_STDOUT, "ERROR! Memory was overwritten after block allocated at %s:%d with size %d!\n",
      	e->file ? e->file : "", e->line, e->size);
      System->Printf (MSG_STDOUT, "       Overwritten with: ");
      dump_memory (pp+e->size+WALL_SIZE, WALL_SIZE);
      System->Printf (MSG_STDOUT, "       Should be       : ");
      dump_memory (AFTERWALL_STR, WALL_SIZE);
      System->Printf (MSG_STDOUT, "       Error discovered while freeing memory at %s:%d.\n",
    	MemLine != -1 ? MemFile : "", MemLine);
    }

#ifndef MEM_CHECK_EXTENSIVE
    free (pp);
    if (e->next) e->next->prev = e->prev;
    if (e->prev) e->prev->next = e->next;
    else entries = e->next;
    if (e->file) free (e->file);
    free (e);
    num_entries--;
#else
    e->freed = true;
    e->free_line = MemLine;
    if (MemLine != -1)
    {
      e->free_file = (char*)malloc (strlen ((char *)MemFile)+1);
      strcpy (e->free_file, (char *)MemFile);
    }
    else
      e->free_file = NULL;

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
    System->Printf (MSG_STDOUT, "ERROR! Trying to free something that is not allocated!\n");
    System->Printf (MSG_STDOUT, "       Error discovered while freeing memory at %s:%d.\n",
    	MemLine != -1 ? MemFile : "", MemLine);
  }
  mem_lock--;
}

#ifdef MEM_CHECK_EXTENSIVE

void check_mem ()
{
  int num = 0;
  MemEntryA* e = entries;
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
          System->Printf (MSG_STDOUT, "ERROR! Memory was overwritten after block allocated at %s:%d with size %d was freed!\n",
      	    e->file ? e->file : "", e->line, e->size);
          System->Printf (MSG_STDOUT, "       Overwritten with: ");
          dump_memory (fill, 16);
          System->Printf (MSG_STDOUT, "       Should have been: ");
          dump_memory (fill_str, 16);
          System->Printf (MSG_STDOUT, "       Block was first freed at %s:%d.\n",
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
  System->Printf (MSG_STDOUT, "------------------- memory block list -------------------\n");
  MemEntryA *cur = entries;
  while (cur)
  {
    if (!cur->freed)
      System->Printf (MSG_STDOUT, "%08lX (%ld)  %s:%d\n", (unsigned long)cur->p, cur->size, cur->file, cur->line);
    cur = cur->next;
  } /* endwhile */
  System->Printf (MSG_STDOUT, "---------------- end of memory block list ---------------\n");
  fflush (stdout);
}

#endif // MEM_CHECK
