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

#ifndef MEMORY_H
#define MEMORY_H

/*
 * System independent (but slower) memory debugger.
 * This one will not catch all errors but it is general.
 */

#ifdef MEM_CHECK

#include <stdio.h>

extern volatile char MemFile[];
extern volatile int MemLine;

#define CHK(x) strcpy ((char *)MemFile, __FILE__); MemLine = __LINE__; x; MemLine = -1
#define CHKB(x) { CHK(x); }

void* operator new (size_t);
void operator delete (void*);
void dump_mem_list ();

// 'MemEntry' is a system structure on the Amiga, thus the name change.
struct MemEntryA
{
  MemEntryA* next, * prev;
  void* p;
  size_t size;
  char* file;
  int line;
  bool freed;	// If this is a block that is already free.
  char* free_file;
  int free_line;
};

#else /*MEM_CHECK*/

#define CHK(x) x
#define CHKB(x) { CHK(x); }

#endif /*MEM_CHECK*/

#endif /*MEMORY_H*/

