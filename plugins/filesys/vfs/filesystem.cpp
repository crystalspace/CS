/*
    Crystal Space File System Plugin classes
    Copyright (C) 2006 by Brandon Hamilton <brandon.hamilton@gmail.com>

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
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "filesystem.h"
#include "csutil/scfstringarray.h"
#include "iutil/objreg.h"

CS_PLUGIN_NAMESPACE_BEGIN(vfs)
{
// ----------------------------------------------------- csFile ---------- //


// ------------------------------------------------------ csFileSystem --- //
csFileSystem::csFileSystem(iBase *iParent) :
  scfImplementationType(this, iParent),
  object_reg(0)
{
 
}

csFileSystem::~csFileSystem ()
{
 
}

bool csFileSystem::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  return true;
}

// ------------------------------------------------ csNativeFileSystem --- //
SCF_IMPLEMENT_FACTORY (csNativeFileSystem)

csNativeFileSystem::csNativeFileSystem(iBase *iParent) :
	scfImplementationExt0(this, iParent)
{

}

csNativeFileSystem::~csNativeFileSystem()
{

}

csPtr<iFile> csNativeFileSystem::Open(const char * FileName, int mode)
{
	return 0;
}

csPtr<iDataBuffer> csNativeFileSystem::ReadFile(const char * FileName)
{
	return 0;
}

bool csNativeFileSystem::WriteFile(const char * Name, const char * Data,
								   size_t Size)
{
	return true;
}

bool csNativeFileSystem::DeleteFile(const char * FileName)
{
	return true;
}

csVFSFileKind csNativeFileSystem::Exists(const char * FileName)
{
	return fkDoesNotExist;
}

// ----------------------------------------------- csArchiveFileSystem --- //
SCF_IMPLEMENT_FACTORY (csArchiveFileSystem)

csArchiveFileSystem::csArchiveFileSystem(iBase *iParent) :
	scfImplementationExt0(this, iParent)
{

}

csArchiveFileSystem::~csArchiveFileSystem()
{

}

csPtr<iFile> csArchiveFileSystem::Open(const char * FileName, int mode)
{
	return 0;
}

csPtr<iDataBuffer> csArchiveFileSystem::ReadFile(const char * FileName)
{
	return 0;
}

bool csArchiveFileSystem::WriteFile(const char * Name, const char * Data, 
									size_t Size)
{
	return true;
}

bool csArchiveFileSystem::DeleteFile(const char * FileName)
{
	return true;
}

csVFSFileKind csArchiveFileSystem::Exists(const char * FileName)
{
	return fkDoesNotExist;
}

} CS_PLUGIN_NAMESPACE_END(vfs)