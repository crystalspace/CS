/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csutil/util.h"
#include "csutil/vfscache.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csVfsCacheManager)
  SCF_IMPLEMENTS_INTERFACE (iCacheManager)
SCF_IMPLEMENT_IBASE_END

csVfsCacheManager::csVfsCacheManager (iObjectRegistry* object_reg,
	const char* vfsdir)
{
  SCF_CONSTRUCT_IBASE (0);
  csVfsCacheManager::object_reg = object_reg;
  csVfsCacheManager::vfsdir = csStrNew (vfsdir);
  vfs = 0;
  current_type = 0;
  current_scope = 0;
}

csVfsCacheManager::~csVfsCacheManager ()
{
  delete[] vfsdir;
  delete[] current_type;
  delete[] current_scope;
  SCF_DESTRUCT_IBASE ();
}

iVFS* csVfsCacheManager::GetVFS ()
{
  if (!vfs)
    vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  return vfs;
}

void csVfsCacheManager::CacheName (char* buf, const char* type,
	const char* scope, uint32 id)
{
  if (id == (uint32)~0)
  {
    if (scope == 0)
      sprintf (buf, "%s", type);
    else
      sprintf (buf, "%s/%s", type, scope);
  }
  else
    sprintf (buf, "%s/%s/%" PRIu32 , type, scope, id);
}

void csVfsCacheManager::SetCurrentType (const char* type)
{
  delete[] current_type;
  if (type)
    current_type = csStrNew (type);
  else
    current_type = 0;
}

void csVfsCacheManager::SetCurrentScope (const char* scope)
{
  delete[] current_scope;
  if (scope)
    current_scope = csStrNew (scope);
  else
    current_scope = 0;
}

bool csVfsCacheManager::CacheData (void* data, size_t size,
  	const char* type, const char* scope, uint32 id)
{
  char buf[512];
  GetVFS ()->PushDir ();
  GetVFS ()->ChDir (vfsdir);
  CacheName (buf, type ? type : current_type,
  	scope ? scope : current_scope, id);
  csRef<iFile> cf = GetVFS ()->Open (buf, VFS_FILE_WRITE);
  GetVFS ()->PopDir ();

  if (!cf)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.vfscachemgr.createfile",
	"Could not create file '%s' in VFS dir '%s'\n", buf, vfsdir);
    return false;
  }

  size_t ws = cf->Write ((const char*)data, size);
  if (ws != size)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.vfscachemgr.writefile",
	"Could not write file '%s' in VFS dir '%s'\n", buf, vfsdir);
    return false;
  }

  return true;
}

csPtr<iDataBuffer> csVfsCacheManager::ReadCache (
  	const char* type, const char* scope, uint32 id)
{
  char buf[512];
  GetVFS ()->PushDir ();
  GetVFS ()->ChDir (vfsdir);
  CacheName (buf, type ? type : current_type,
  	scope ? scope : current_scope, id);
  csRef<iDataBuffer> data (GetVFS ()->ReadFile (buf, false));
  GetVFS ()->PopDir ();

  if (!data)
  {
    // This is not an error. It is possible that the item
    // simply hasn't been cached.
    return 0;
  }

  return csPtr<iDataBuffer> (data);
}

bool csVfsCacheManager::ClearCache (const char* type, const char* scope,
  	const uint32* id)
{
  // @@@ Not implemented yet.
  (void)type;
  (void)scope;
  (void)id;
  return false;
}

void csVfsCacheManager::Flush ()
{
  GetVFS ()->Sync ();
}

