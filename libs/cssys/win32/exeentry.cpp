/*
    Copyright (C) 1998 by Jorrit Tyberghein
    This is the entry point for console executables
  
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

#include <windows.h>

#include "sysdef.h"
#include "csutil/scf.h"

#undef main
extern int csMain (int argc, char* argv[]);
HINSTANCE ModuleHandle;		// defined in the COM library

// The main entry for console applications
int main (int argc, char* argv[])
{
  // hInstance is really the handle of module
  ModuleHandle = GetModuleHandle (NULL);
  return csMain (argc, argv);
}
