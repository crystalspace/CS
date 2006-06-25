/*
    Crystal Space Virtual File System Plugin classes
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
csFileSystem::csFile::csFile (const char *Name) :
  scfImplementationType(this, 0)
{
  Size = 0;
  Error = VFS_STATUS_OK;
}

csFileSystem::csFile::~csFile ()
{
  delete [] Name;
}

int csFileSystem::csFile::GetStatus ()
{
  int rc = Error;
  Error = VFS_STATUS_OK;
  return rc;
}
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

iFile* csNativeFileSystem::Open(const char * FileName, int mode)
{
  csPhysicalFile* file;

  if ((mode & VFS_FILE_MODE) == VFS_FILE_WRITE)
        file = new csPhysicalFile(FileName, "wb");
  else if ((mode & VFS_FILE_MODE) == VFS_FILE_APPEND)
        file = new csPhysicalFile(FileName, "ab");
  else
        file = new csPhysicalFile(FileName, "rb");

	return file;
}

bool csNativeFileSystem::Delete(const char * FileName)
{
  return (unlink (FileName) == 0);
}

csVFSFileKind csNativeFileSystem::Exists(const char * FileName)
{
  struct stat stats;
  if (stat (FileName, &stats) == 0)
    return fkDoesNotExist;


  if ((stats.st_mode & _S_IFDIR) != 0)
    return fkFile;
  else
    return fkDirectory;
}

bool csNativeFileSystem::CanHandleMount(const char *FileName)
{
  struct stat stats;
  if (stat (FileName, &stats) == 0)
    return false;

  return true;
}

void csNativeFileSystem::GetFilenames(const char *Path, const char *Mask,
                                      iStringArray *Names)
{
  csString vpath;
  DIR *dh;
  struct dirent *de;

  dh = opendir (Path);
  if (dh == 0)
    return;
  while ((de = readdir (dh)) != 0)
  {
    if ((strcmp (de->d_name, ".") == 0) || (strcmp (de->d_name, "..") == 0))
          continue;
    if (!csGlobMatches (de->d_name, Mask))
          continue;

    Names->Push(de->d_name);
  }
  closedir (dh);
}

// Query file date/time.
bool csNativeFileSystem::GetFileTime (const char *FileName, 
                                      csFileTime &oTime) const
{
  struct stat st;

  if (stat (FileName, &st))
    return false;

  const time_t mtime = st.st_mtime;
  struct tm *curtm = localtime (&mtime);

  ASSIGN_FILETIME (oTime, *curtm);

  return true;
}
  
// Set file date/time.
bool csNativeFileSystem::SetFileTime (const char *FileName, 
                                      const csFileTime &iTime)
{
  // Cannot do this in platform independant way
  return true;
}

// Query file size (without opening it).
bool csNativeFileSystem::GetFileSize (const char *FileName, size_t &oSize)
{
  struct stat st;
  if (stat (FileName, &st))
      return false;
  oSize = st.st_size;
  return true;
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

iFile* csArchiveFileSystem::Open(const char * FileName, int mode)
{
	return 0;
}

bool csArchiveFileSystem::Delete(const char * FileName)
{
	return true;
}

csVFSFileKind csArchiveFileSystem::Exists(const char * FileName)
{
	return fkDoesNotExist;
}

bool csArchiveFileSystem::CanHandleMount(const char *FileName)
{
  FILE* f = fopen (FileName, "rb");
  if (!f) return false;

  char header[4];
  bool ret = ((fread (header, sizeof(header), 1, f) == 1)
    && (header[0] == 'P') && (header[1] == 'K')
    && (header[2] ==   3) && (header[3] ==   4));
  fclose (f);

  return ret;
}

void csArchiveFileSystem::GetFilenames(const char *Path, const char *Mask, iStringArray *Names)
{

}

// Query file date/time.
bool csArchiveFileSystem::GetFileTime (const char *FileName, csFileTime &oTime) const
{
  return true;
}
  
// Set file date/time.
bool csArchiveFileSystem::SetFileTime (const char *FileName, const csFileTime &iTime)
{
  return true;
}

// Query file size (without opening it).
bool csArchiveFileSystem::GetFileSize (const char *FileName, size_t &oSize)
{
  return true;
}
} CS_PLUGIN_NAMESPACE_END(vfs)