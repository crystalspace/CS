/*
    Dummy main function.
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

#include "cssysdef.h"

#if !defined(CS_STATIC_LINKED)
// Dummy main function to satisfy the linker on Unix.
// This is not always needed (for example, not on Solaris)
// but it doesn't hurt. We could also have solved this by
// using -nostdlib but then the constructors for global
// objects are not called.
int main (int argc, char* argv[])
{
  (void)argc; (void)argv;
  return 0;
}
#endif
