/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
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

#include "cssysdef.h"
#include "csutil/scf.h"

extern int csMain (int argc, char* argv[]);
extern int ApplicationShow;
HINSTANCE ModuleHandle;	

#undef main
// The main entry for console applications
int main (int argc, char* argv[])
{
  // hInstance is really the handle of module
  ModuleHandle = GetModuleHandle (NULL);
  ApplicationShow = SW_SHOWNORMAL;
  return csMain (argc, argv);
}

// The main entry for GUI applications
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  ModuleHandle = hInstance;
  ApplicationShow = nCmdShow;
  (void)lpCmdLine;
  (void)hPrevInstance;

  return
#ifdef COMP_BC
    csMain ( _argc,  _argv);
#else
    csMain (__argc, __argv);
#endif
}
