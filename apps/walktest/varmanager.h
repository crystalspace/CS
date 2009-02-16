/*
    Copyright (C) 2008 by Jorrit Tyberghein

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

#ifndef __WALKTEST_VARMANAGER_H__
#define __WALKTEST_VARMANAGER_H__

class WalkTest;

class WalkTestVarManager
{
public:
  static bool SetVariableColor (WalkTest* Sys, const char* arg);
  static bool SetVariableVector (WalkTest* Sys, const char* arg);
  static bool SetVariable (WalkTest* Sys, const char* arg);
  static bool ShowVariable (WalkTest* Sys, const char* arg);
  static bool ListVariables (WalkTest* Sys, const char* arg);
};

#endif // __WALKTEST_VARMANAGER_H__

