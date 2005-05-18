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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/cfgmgr.h"
#include "iutil/objreg.h"

csConfigAccess::csConfigAccess()
{
  object_reg = 0;
}

csConfigAccess::csConfigAccess(iObjectRegistry *object_reg)
{
  csConfigAccess::object_reg = object_reg;
}

csConfigAccess::csConfigAccess(iObjectRegistry *object_reg, const char *fname,
  bool vfs, int priority)
{
  AddConfig (object_reg, fname, vfs, priority);
}

csConfigAccess::csConfigAccess(iObjectRegistry *object_reg, iConfigFile* cfg,
  int priority)
{
  AddConfig (object_reg, cfg, priority);
}

csConfigAccess::~csConfigAccess()
{
  if (object_reg)
  {
    csRef<iConfigManager> cfgmgr (
    	CS_QUERY_REGISTRY (object_reg, iConfigManager));
    if (cfgmgr)
    {
      size_t i;
      for (i = 0; i < ConfigFiles.Length (); i++)
	cfgmgr->RemoveDomain (ConfigFiles[i]);
    }
  }
}

void csConfigAccess::AddConfig (iObjectRegistry *object_reg, const char *fname,
  bool vfs, int priority)
{
  csConfigAccess::object_reg = object_reg;
  csRef<iConfigManager> cfgmgr (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  csRef<iVFS> VFS;
  if (vfs)
  {
    VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
    //CS_ASSERT (VFS != 0);
  }
  ConfigFiles.Push (cfgmgr->AddDomain (fname, VFS, priority));
}

void csConfigAccess::AddConfig (iObjectRegistry *object_reg, iConfigFile* cfg,
  int priority)
{
  csConfigAccess::object_reg = object_reg;
  csRef<iConfigManager> cfgmgr (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  cfgmgr->AddDomain (cfg, priority);
  ConfigFiles.Push (cfg);
}

iConfigFile *csConfigAccess::operator->()
{
  csRef<iConfigManager> cfg (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  return (iConfigFile*)cfg;	// This will decref cfg but that's ok in this case.
}

csConfigAccess::operator iConfigFile* ()
{
  csRef<iConfigManager> cfg (CS_QUERY_REGISTRY (object_reg, iConfigManager));
  return (iConfigFile*)cfg;	// This will decref cfg but that's ok in this case.
}
