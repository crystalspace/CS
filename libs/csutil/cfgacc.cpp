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
#include "isys/plugin.h"
#include "isys/vfs.h"
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
  iConfigManager* cfgmgr = CS_QUERY_REGISTRY (System->GetObjectRegistry (),
      iConfigManager);
  for (long i=0; i<ConfigFiles.Length(); i++)
    cfgmgr->RemoveDomain ((iConfigFile*)ConfigFiles.Get(i));
}

void csConfigAccess::AddConfig(iSystem *sys, const char *fname,
  bool vfs, int priority)
{
  System = sys;
  iConfigManager* cfgmgr = CS_QUERY_REGISTRY (System->GetObjectRegistry (),
      iConfigManager);
  iVFS* VFS = NULL;
  if (vfs)
  {
    // @@@ We cannot use CS_QUERY_REGISTRY here to get the pointer
    // to iVFS since this function is called very early even before
    // VFS is added to the object registry. In the future we have
    // to see if we cannot avoid this.
    iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (
	System->GetObjectRegistry (), iPluginManager);
    VFS = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VFS, iVFS);
    //CS_ASSERT (VFS != NULL);
  }
  ConfigFiles.Push(cfgmgr->AddDomain (fname, VFS, priority));
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
