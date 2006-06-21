/*
    Crystal Space Virtual File System class
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>
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

#include "vfs.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfstringarray.h"
#include "iutil/objreg.h"

#define NEW_CONFIG_SCANNING

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(vfs)
{

// ----------------------------------------------------------- VfsNode --- //
class VfsNode
{
public:
  // Constructor
  VfsNode(char *virtualPathname, csVFS *parentVFS, VfsNode* parentNode, 
	  int pluginIndex);

  ~VfsNode();
  // A pointer to the parent of this node
  VfsNode *ParentNode;

  // The name of this node in the VFS tree
  char *VirtualPathname;

  // The real paths mapped to this node
  csStringArray RealPaths;

  // The virtual paths mapped to this node (symbolic links)
  csStringArray SymbolicLinks;

  // List the files in this directory tree
  csPtr<iStringArray> FindFiles(const char *Suffix, const char *Mask);

  // Return a pointer to a node representing a virtual path
  VfsNode *FindNode(const char *VirtualPath);

  int FileSystem() { return FileSystemPlugin; }

  // Index into the filesystem plugins associated with this node
  int FileSystemPlugin;

  // A vector of pointers to the subdirectories of this node
  VfsVector SubDirectories;

  // Allow VFS to access node internals
  csVFS *ParentVFS;
};


VfsNode::VfsNode(char *virtualPathname, csVFS *parentVFS, VfsNode* parentNode, 
				 int pluginIndex)
: ParentVFS(parentVFS), 
  ParentNode(parentNode), FileSystemPlugin(pluginIndex)
{
	VirtualPathname = virtualPathname;
}

VfsNode::~VfsNode()
{
  SubDirectories.DeleteAll();
}

VfsNode * VfsNode::FindNode(const char *VirtualPath)
{
  printf("  -> Called FindNode('%s') from '%s'\n", VirtualPath, VirtualPathname);

  if (strlen(VirtualPath) < strlen(VirtualPathname))
    return 0;

  if (strcmp(VirtualPath, VirtualPathname) == 0)
    return this;

  printf("    -> Searching subdirectories...\n");
  size_t low = -1;
  size_t high = SubDirectories.Length();
  int cmp = 0;
  size_t i = 0;

  while (high - low > 1)
  {
    i = (low + high) >> 1;
	printf("      -> Low = %d, High = %d, mid = %d\n", low, high, i);
	printf("        -> Comparing %d characters of '%s' with '%s'....", strlen(SubDirectories.Get(i)->VirtualPathname), SubDirectories.Get(i)->VirtualPathname, VirtualPath);
    cmp = strncmp(SubDirectories.Get(i)->VirtualPathname, VirtualPath, strlen(SubDirectories.Get(i)->VirtualPathname));
	printf("%d\n", cmp);
	if (cmp == 0)
	  return SubDirectories.Get(i)->FindNode(VirtualPath);
    if (cmp < 0)
	  low = i;	
    else
      high = i;
  }

  printf("    -> Finished Searching\n");
  printf("  -> '%s' Not Found !\n", VirtualPath);
  return 0;
}

// --------------------------------------------------------- VfsVector --- //
int VfsVector::Compare (VfsNode* const& Item1, VfsNode* const& Item2)
{
  return strcmp (Item1->VirtualPathname, Item2->VirtualPathname);
}


// ------------------------------------------------------------- csVFS --- //

SCF_IMPLEMENT_FACTORY (csVFS)

// csVFS contructor
csVFS::csVFS (iBase *iParent) :
  scfImplementationType(this, iParent),
  RootNode(0),
  CwdNode(0),
  object_reg(0)
{
  mutex = csMutex::Create (true); // We need a recursive mutex.
  RootNode = new VfsNode("/", this, 0, 0);
  RootNode->SubDirectories.Push(new VfsNode("/aswell" , this, RootNode, 0));
  RootNode->SubDirectories.Push(new VfsNode("/cool" , this, RootNode, 0));
  RootNode->SubDirectories.Push(new VfsNode("/doingit" , this, RootNode, 0));
  RootNode->SubDirectories.Push(new VfsNode("/mnt" , this, RootNode, 0));
  RootNode->SubDirectories.Get(3)->SubDirectories.Push(new VfsNode("/mnt/alternate" , this, RootNode->SubDirectories.Get(0), 0));
  RootNode->SubDirectories.Get(3)->SubDirectories.Push(new VfsNode("/mnt/dumbass" , this, RootNode->SubDirectories.Get(0), 0));
  RootNode->SubDirectories.Push(new VfsNode("/other" , this, RootNode, 0));
  RootNode->SubDirectories.Push(new VfsNode("/things" , this, RootNode, 0));
  CwdNode = RootNode;
}

// csVFS destructor
csVFS::~csVFS ()
{
  // Free memory used by VFS Nodes
  delete RootNode;
}

bool csVFS::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  return true;
}

bool csVFS::ChDir (const char *Path)
{
  if (strlen(Path) == 0)
    return false;

  VfsNode *newCwd;
  // Absolute Path
  if (Path[0] == VFS_PATH_SEPARATOR)
  {
    newCwd = RootNode->FindNode(Path);
  }
  // Relative Path
  else
  {
	size_t cwdLen = strlen(CwdNode->VirtualPathname);

    char *path = new char[cwdLen + strlen(Path) + 1];
	
	strcpy (path, CwdNode->VirtualPathname);
	path[cwdLen] = VFS_PATH_SEPARATOR;
	strcpy (path + cwdLen + 1, Path);

    newCwd = CwdNode->FindNode(path);
  }

  // Assign new Cwd
  if (newCwd)
  {
	CwdNode = newCwd;
	return true;
  }
  return false;
}

void csVFS::PushDir (char const* Path)
{
	// Push the current directory onto the stack
	DirectoryStack.Push(CwdNode);

	// Change the current directory
    ChDir(Path);
}

bool csVFS::PopDir ()
{
	// Check that the stack is not empty
	if (DirectoryStack.Length() < 1)
		return false;

	// Pop the stack and change the cwd
	CwdNode = DirectoryStack.Pop();
	return true;
}

csPtr<iDataBuffer> csVFS::ExpandPath (const char *Path, bool IsDir) const
{
	return NULL;
}

bool csVFS::Exists (const char *Path) const
{
  if (strlen(Path) == 0)
    return false;

  // Find the parent directory of the files we are checking
  VfsNode *ParentDirectoryNode;
	
  const char *Suffix = strrchr(Path, VFS_PATH_SEPARATOR);
  char *Directory = new char[Suffix - Path + 1];
  strncpy(Directory, Path, Suffix - Path);

  // Absolute Path
  if (Directory[0] == VFS_PATH_SEPARATOR)
  {
    ParentDirectoryNode = RootNode->FindNode(Directory);
  }
  // Relative Path
  else
  {
	size_t cwdLen = strlen(CwdNode->VirtualPathname);

    char *path = new char[cwdLen + strlen(Directory) + 1];
	
	strcpy (path, CwdNode->VirtualPathname);
	path[cwdLen] = VFS_PATH_SEPARATOR;
	strcpy (path + cwdLen + 1, Directory);

    ParentDirectoryNode = CwdNode->FindNode(path);
  }
 
  return (fsPlugins.Get(ParentDirectoryNode->FileSystem())->Exists(Path) != fkDoesNotExist);
}

csPtr<iStringArray> csVFS::FindFiles (const char *Path) const
{
	return 0;	
}

csPtr<iFile> csVFS::Open (const char *FileName, int Mode)
{
	return 0;	
}
  
csPtr<iDataBuffer> csVFS::ReadFile (const char *FileName, bool nullterm)
{
	return 0;	
}

bool csVFS::WriteFile (const char *FileName, const char *Data, size_t Size)
{
		return true;	
}

bool csVFS::DeleteFile (const char *FileName)
{
		return true;	
}

bool csVFS::Sync ()
{
		return true;	
}

bool csVFS::Mount (const char *VirtualPath, const char *RealPath)
{
		return true;	
}

bool csVFS::Unmount (const char *VirtualPath, const char *RealPath)
{
		return true;	
}
  
csRef<iStringArray> csVFS::MountRoot (const char *VirtualPath)
{
		return NULL;
}

bool csVFS::SaveMounts (const char *FileName)
{
		return true;
}

bool csVFS::LoadMountsFromFile (iConfigFile* file)
{
		return true;	
}

bool csVFS::ChDirAuto (const char* path, const csStringArray* paths,
					   const char* vfspath, const char* filename)
{
  return true;
}

bool csVFS::GetFileTime (const char *FileName, csFileTime &oTime) const
{
			return true;
}		

bool csVFS::SetFileTime (const char *FileName, const csFileTime &iTime)
{
			return true;
}

bool csVFS::GetFileSize (const char *FileName, size_t &oSize)
{
			return true;
}

csPtr<iDataBuffer> csVFS::GetRealPath (const char *FileName)
{
		return 0;
}

csRef<iStringArray> csVFS::GetMounts ()
{
		return 0;
}

csRef<iStringArray> csVFS::GetRealMountPaths (const char *VirtualPath)
{
		return 0;
}

// Register a filesystem plugin
size_t csVFS::RegisterPlugin(csRef<iFileSystem> FileSystem)
{
	// Add the plugin
	return fsPlugins.PushSmart(FileSystem);
}

// Create or add a symbolic link
bool csVFS::SymbolicLink(const char *Target, const char *Link, bool Overwrite)
{
	return true;
}

} CS_PLUGIN_NAMESPACE_END(vfs)
