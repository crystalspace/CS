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

namespace cspluginVFS
{

// --------------------------------------------------------------- VfsNode --- //

// Private structure used to keep a "node" in virtual filesystem tree.
// The program can be made even fancier if we use a object for each
// "real" path (i.e. each VfsNode will contain an array of real-world
// nodes - both "directory" and "archive" types) but since we have to
// balance between pretty understandable code and effective code, this
// time we choose effectivity - the cost can become very big in this case.
class VfsNode
{
public:
  // The virtual path
  char *VPath;

  // Configuration section key
  char *ConfigKey;

  // The array of real paths/archives bound to this virtual path
  csStringArray RPathV;

  // The array of unexpanded real paths
  csStringArray UPathV;

  // The object registry.
  iObjectRegistry *object_reg;

  // Verbosity flags.
  unsigned int verbosity;

  // Initialize the object
  VfsNode (char *iPath, const char *iConfigKey, iObjectRegistry *object_reg, unsigned int verbosity);

  // Destroy the object
  virtual ~VfsNode ();

  // Parse a directory link directive and fill RPathV
  bool AddRPath (const char *RealPath, csVFS *Parent);

  // Remove a real-world path
  bool RemoveRPath (const char *RealPath, csVFS *Parent);

  // Find all files in a subpath
  void FindFiles(const char *Suffix, const char *Mask, iStringArray *FileList);

  // Find a file and return the appropiate csFile object
  iFile *Open (int Mode, const char *Suffix);

  // Delete a file
  bool Delete (const char *Suffix);

  // Does file exists?
  bool Exists (const char *Suffix);

  // Query date/time
  bool GetFileTime (const char *Suffix, csFileTime &oTime) const;

  // Set date/time
  bool SetFileTime (const char *Suffix, const csFileTime &iTime);

  // Get file size
  bool GetFileSize (const char *Suffix, size_t &oSize);

private:
  // Get value of a variable
  const char *GetValue (csVFS *Parent, const char *VarName);

  // Copy a string from src to dst and expand all variables
  csString Expand (csVFS *Parent, char const *src);

  // Find a file either on disk or in archive - in this node only
  //bool FindFile (const char *Suffix, char *RealPath, csArchive *&) const;
};

// --------------------------------------------------------------- csFileSystem --- //
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

// --------------------------------------------------------------- csNativeFileSystem --- //
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
	return NULL;
}

csPtr<iDataBuffer> csNativeFileSystem::ReadFile(const char * FileName, bool nullterm)
{
	return NULL;
}

bool csNativeFileSystem::WriteFile(const char * Name, const char * Data, size_t Size)
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

// --------------------------------------------------------------- csArchiveFileSystem --- //
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
	return NULL;
}

csPtr<iDataBuffer> csArchiveFileSystem::ReadFile(const char * FileName, bool nullterm)
{
	return NULL;
}

bool csArchiveFileSystem::WriteFile(const char * Name, const char * Data, size_t Size)
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

// --------------------------------------------------------------- csNetworkFileSystem --- //
SCF_IMPLEMENT_FACTORY (csNetworkFileSystem)

csNetworkFileSystem::csNetworkFileSystem(iBase *iParent) :
	scfImplementationExt0(this, iParent)
{

}

csNetworkFileSystem::~csNetworkFileSystem()
{

}

csPtr<iFile> csNetworkFileSystem::Open(const char * FileName, int mode)
{
	return NULL;
}

csPtr<iDataBuffer> csNetworkFileSystem::ReadFile(const char * FileName, bool nullterm)
{
	return NULL;
}

bool csNetworkFileSystem::WriteFile(const char * Name, const char * Data, size_t Size)
{
	return true;
}

bool csNetworkFileSystem::DeleteFile(const char * FileName)
{
	return true;
}

csVFSFileKind csNetworkFileSystem::Exists(const char * FileName)
{
	return fkDoesNotExist;
}


} // namespace cspluginVFS
