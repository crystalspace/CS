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

#define CHK(x) PushLoc (__FILE__, __LINE__); x; PopLoc ()
#define CHKB(x) { CHK(x); }

extern void PushLoc (char *iFileName, int iLineNo);
extern void PopLoc ();
extern void dump_mem_list ();

#else /*MEM_CHECK*/

#define CHK(x) x
#define CHKB(x) { CHK(x); }

#endif /*MEM_CHECK*/

#endif /*MEMORY_H*/
