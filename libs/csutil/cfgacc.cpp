/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csutil/cfgacc.h"
#include "iutil/cfgmgr.h"
#include "iutil/objreg.h"

csConfigAccess::csConfigAccess()
{
  System = NULL;
}

csConfigAccess::csConfigAccess(iSystem *sys, const char *fname,
  bool vfs, int priority)
{
  AddConfig(sys, fname, vfs, priority);
}

csConfigAccess::~csConfigAccess()
{
  for (long i=0; i<ConfigFiles.Length(); i++)
    System->RemoveConfig((iConfigFile*)ConfigFiles.Get(i));
}

void csConfigAccess::AddConfig(iSystem *sys, const char *fname,
  bool vfs, int priority)
{
  System = sys;
  ConfigFiles.Push(System->AddConfig(fname, vfs, priority));
}

iConfigFile *csConfigAccess::operator->()
{
  iObjectRegistry* object_reg = System->GetObjectRegistry ();
  iConfigFile* cfg = CS_QUERY_REGISTRY (object_reg, iConfigManager);
  return cfg;
}

csConfigAccess::operator iConfigFile* ()
{
  iObjectRegistry* object_reg = System->GetObjectRegistry ();
  iConfigFile* cfg = CS_QUERY_REGISTRY (object_reg, iConfigManager);
  return cfg;
}
