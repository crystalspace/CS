/*
    Copyright (C) 2002 by Frank Richter

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
#include "csutil/nulcache.h"

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csNullCacheManager)
  SCF_IMPLEMENTS_INTERFACE (iCacheManager)
SCF_IMPLEMENT_IBASE_END

csNullCacheManager::csNullCacheManager ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csNullCacheManager::~csNullCacheManager ()
{
}

void csNullCacheManager::SetCurrentType (const char*)
{
}

void csNullCacheManager::SetCurrentScope (const char*)
{
}

bool csNullCacheManager::CacheData (void*, uint32,
  	const char*, const char*, uint32)
{
  return false;
}

csPtr<iDataBuffer> csNullCacheManager::ReadCache (
  	const char* /*type*/, const char* /*scope*/, uint32 /*id*/)
{
  return csPtr<iDataBuffer> (NULL);
}

bool csNullCacheManager::ClearCache (const char*, const char*,
  	const uint32*)
{
  return false;
}

