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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
  SCF_CONSTRUCT_IBASE (NULL);
  csVfsCacheManager::object_reg = object_reg;
  csVfsCacheManager::vfsdir = csStrNew (vfsdir);
  vfs = NULL;
  current_type = NULL;
  current_scope = NULL;
}

csVfsCacheManager::~csVfsCacheManager ()
{
  delete[] vfsdir;
  delete[] current_type;
  delete[] current_scope;
  if (vfs) vfs->DecRef ();
}

iVFS* csVfsCacheManager::GetVFS ()
{
  if (!vfs)
  {
    vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  }
  return vfs;
}

void csVfsCacheManager::CacheName (char* buf, const char* type,
	const char* scope, uint32 id)
{
  sprintf (buf, "%s/%s/%ld", type, scope, id);
}

void csVfsCacheManager::SetCurrentType (const char* type)
{
  delete[] current_type;
  if (type)
    current_type = csStrNew (type);
  else
    current_type = NULL;
}

void csVfsCacheManager::SetCurrentScope (const char* scope)
{
  delete[] current_scope;
  if (scope)
    current_scope = csStrNew (scope);
  else
    current_scope = NULL;
}

bool csVfsCacheManager::CacheData (void* data, uint32 size,
  	const char* type, const char* scope, uint32 id)
{
  char buf[512];
  GetVFS ()->PushDir ();
  GetVFS ()->ChDir (vfsdir);
  CacheName (buf, type ? type : current_type,
  	scope ? scope : current_scope, id);
  iFile *cf = GetVFS ()->Open (buf, VFS_FILE_WRITE);
  GetVFS ()->PopDir ();

  if (!cf)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.vfscachemgr.createfile",
	"Could not create file '%s' in VFS dir '%s'\n", buf, vfsdir);
    return false;
  }

  uint32 ws = cf->Write ((const char*)data, size);
  cf->DecRef ();
  if (ws != size)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.vfscachemgr.writefile",
	"Could not write file '%s' in VFS dir '%s'\n", buf, vfsdir);
    return false;
  }

  return true;
}

iDataBuffer* csVfsCacheManager::ReadCache (
  	const char* type, const char* scope, uint32 id)
{
  char buf[512];
  GetVFS ()->PushDir ();
  GetVFS ()->ChDir (vfsdir);
  CacheName (buf, type ? type : current_type,
  	scope ? scope : current_scope, id);
  iDataBuffer* data = GetVFS ()->ReadFile (buf);
  GetVFS ()->PopDir ();

  if (!data)
  {
    // This is not an error. It is possible that the item
    // simply hasn't been cached.
    return NULL;
  }

  return data;
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

