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
#include "filesystem.h"
#include "csutil/databuf.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfstringarray.h"
#include "csutil/csstring.h"
#include "csutil/syspath.h"
#include "iutil/objreg.h"

#define NEW_CONFIG_SCANNING

CS_IMPLEMENT_PLUGIN


/*********************************************
  TODO : Sort out circular symbolic links !!
**************************************************/

CS_PLUGIN_NAMESPACE_BEGIN(vfs)
{

// Extract a filename from a full path
csString ExtractFileName(const char *FullPath)
{
  csString strPath = FullPath;

    // Remove trailing slash
  if (strPath.GetAt(strPath.Length() - 1) == VFS_PATH_SEPARATOR)
    strPath.Truncate(strPath.Length() - 1);

  csString strFileName;

  size_t newlen = strPath.FindLast(VFS_PATH_SEPARATOR);
  strPath.SubString(strFileName, newlen+1);

  return strFileName;
}

// ----------------------------------------------------------- VfsNode --- //

// Structure to hold data about VfsNode paths
struct MappedPathData
{
  // The name of the path
  csString name;

  // Index into csVfs fsPlugins array
  size_t pluginIndex;

  // The priority of the path
  int priority;

  // Is this a symbolic link
  bool symlink;
};

int CompareMappedPaths(const MappedPathData &a, const MappedPathData &b)
{
  return (a.priority - b.priority);
}

/**
 * TODO: write class description
 * TODO: Implement protection against circular symlinks
 */
class VfsNode
{
public:
  // Constructor
  VfsNode(const char *virtualPathname, const csVFS *parentVFS, VfsNode* parentNode);

  ~VfsNode();
  // A pointer to the parent of this node
  VfsNode *ParentNode;

  // The name of this node in the VFS tree
  csString VirtualPathname;

  // The real paths and symbolic links mapped to this node
  csArray<MappedPathData> MappedPaths;

  // List the files in this directory tree
  void FindFiles(const char *Mask, iStringArray *FileList);

  // Return a pointer to a node representing a virtual path
  VfsNode *FindNode(const char *VirtualPath);

  // Get all the current mounted directories
  void GetMounts(scfStringArray *MountArray);

  // Open a file
  iFile* Open (const char *FileName, int Mode);

  // Delete a file
  bool Delete(const char *FileName);

  // A vector of pointers to the subdirectories of this node
  VfsVector SubDirectories;

  // Does this node contain a file
  bool ContainsFile(const char *FileName) const;

  bool GetFileTime (const char *FileName, csFileTime &oTime) const;

  bool SetFileTime (const char *FileName, const csFileTime &iTime);

  bool GetFileSize (const char *FileName, size_t &oSize);

  bool GetRealPath(const char *FileName, csString &path) const;

  const char * GetName() const { return ((const char*) VirtualPathname); }

  // Add a subdirectory to this node
  bool AddSubDirectory(VfsNode *SubDir);

  // Add a subdirectory to this node
  bool RemoveSubDirectory(VfsNode *SubDir);

  // Allow VFS to access node internals
  const csVFS *ParentVFS;
};

VfsNode::VfsNode(const char *virtualPathname, const csVFS *parentVFS, VfsNode* parentNode)
: ParentVFS(parentVFS), 
  ParentNode(parentNode)
{
	VirtualPathname = virtualPathname;

  if (parentNode)
  {
    parentNode->AddSubDirectory(this);
  }
}

VfsNode::~VfsNode()
{
  SubDirectories.DeleteAll();
}

bool VfsNode::AddSubDirectory(VfsNode *SubDir)
{
    size_t m = 0, l = 0, r = SubDirectories.Length ();
    while (l < r)
    {
      m = (l + r) / 2;
      int cmp = strcmp(SubDirectories[m]->GetName(), SubDir->GetName());

      if (cmp == 0)
      {
        // Duplicate Entry
        return false;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m;
    }
    if ((m + 1) == r)
      m++;

    SubDirectories.Insert(m, SubDir);
    return true;
}

bool VfsNode::RemoveSubDirectory(VfsNode *SubDir)
{
    size_t m = 0, l = 0, r = SubDirectories.Length ();
    while (l < r)
    {
      m = (l + r) / 2;
      int cmp = strcmp(SubDirectories[m]->GetName(), SubDir->GetName());

      if (cmp == 0)
      {
        SubDirectories.DeleteIndex(m);
        return true;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m;
    }
    // Not found
    return false;
}


VfsNode * VfsNode::FindNode(const char *VirtualPath)
{
  if (strlen(VirtualPath) < VirtualPathname.Length())
    return 0;

  if (VirtualPathname.Compare(VirtualPath))
    return this;

  size_t low = -1;
  size_t high = SubDirectories.Length();
  int cmp = 0;
  size_t i = 0;
  
  // Use a binary search to find subdirectory
  while (high - low > 1)
  {
    i = (low + high) >> 1;

    cmp = strcmp(SubDirectories[i]->GetName(), VirtualPath);

	  if (cmp == 0)
	    return SubDirectories[i]->FindNode(VirtualPath);

    if (cmp < 0)
	    low = i;	

    else
      high = i;
  }

  return 0;
}

void VfsNode::GetMounts(scfStringArray * MountArray)
{
  // Save each node path into the array
  MountArray->Push((const char *) VirtualPathname);
  for (size_t i =0; i < SubDirectories.Length(); i++)
  {
	  SubDirectories[i]->GetMounts(MountArray);  
  }
}

iFile* VfsNode::Open (const char *FileName, int Mode)
{
  if (!FileName)
    return false;

  csString FileToFind;
  iFile* file = 0;
  for (size_t i = 0; i < MappedPaths.Length(); i++)
  {
    if (MappedPaths[i].symlink)
    {
      file = ParentVFS->GetDirectoryNode(MappedPaths[i].name)->Open(FileName, Mode);
      if (file)
        return file;
    }
    else
    {
      FileToFind.Clear();
      FileToFind = VirtualPathname;
      FileToFind.Append(VFS_PATH_SEPARATOR);
      FileToFind.Append(FileName);
      iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[i].pluginIndex);

      file = fs->Open((const char *) FileToFind, Mode);

      if (file)
        return file;
    }
  }

  // File was not found
  return 0;
}

bool VfsNode::ContainsFile(const char *FileName) const
{
  if (!FileName)
    return false;

  csString FileToFind;
  for (size_t i = 0; i < MappedPaths.Length(); i++)
  {
    if (MappedPaths[i].symlink)
    {
       if (ParentVFS->GetDirectoryNode(MappedPaths[i].name)->ContainsFile(FileName))
         return true;
    }
    else
    {
       FileToFind.Clear();
       FileToFind = VirtualPathname;
       FileToFind.Append(VFS_PATH_SEPARATOR);
       FileToFind.Append(FileName);
       iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[i].pluginIndex);
       if (fs->Exists((const char *) FileToFind) != fkDoesNotExist)
         return true;
    }
  }

  // File is not within this node
  return false;
}

bool VfsNode::GetRealPath(const char *FileName, csString &path) const
{
  if (!FileName)
  {
    path.Clear();
    return false;
  }

  csString FileToFind;

  for (size_t i = 0; i < MappedPaths.Length(); i++)
  {
    if (MappedPaths[i].symlink)
    {
       if (ParentVFS->GetDirectoryNode(MappedPaths[i].name)->GetRealPath(FileName, path))
       {
          return true;
       }
    }
    else
    {
       FileToFind.Clear();
       FileToFind = VirtualPathname;
       FileToFind.Append(VFS_PATH_SEPARATOR);
       FileToFind.Append(FileName);
       iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[i].pluginIndex);
       if (fs->Exists((const char *) FileToFind) != fkDoesNotExist)
       {
         path = FileToFind;
         return true;
       }
    }
  }

  // File is not within this node
  return false;
}

bool VfsNode::Delete(const char *FileName)
{
  if (!FileName)
    return false;

  csString FileToFind;
  for (size_t i = 0; i < MappedPaths.Length(); i++)
  {
    if (MappedPaths[i].symlink)
    {
       if (ParentVFS->GetDirectoryNode(MappedPaths[i].name)->Delete(FileName))
         return true;
    }
    else
    {
       FileToFind.Clear();
       FileToFind = VirtualPathname;
       FileToFind.Append(VFS_PATH_SEPARATOR);
       FileToFind.Append(FileName);
       iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[i].pluginIndex);
       if (fs->Delete((const char *) FileToFind))
         return true;
    }
  }

  // File was not found
  return false;
}

void VfsNode::FindFiles(const char *Mask, iStringArray *FileList)
{
  printf("Calling FindFiles(%s) from %s\n", Mask, (const char *) VirtualPathname);
  // First add names of all subdirectories
  for (size_t i = 0; i < SubDirectories.Length(); i++)
  {
    if (!FileList->Contains(ExtractFileName(SubDirectories[i]->VirtualPathname), true))
       FileList->Push(ExtractFileName(SubDirectories[i]->VirtualPathname));
  }

  // Add names from the paths
  for (size_t j = 0; j < MappedPaths.Length(); j++)
  {
    // Add names in symbolic links
    if (MappedPaths[j].symlink)
    {
      ParentVFS->GetDirectoryNode(MappedPaths[j].name)->FindFiles(Mask, FileList);
    }
    // Add names from FileSystems
    else
    {
      iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[j].pluginIndex);
      fs->GetFilenames(MappedPaths[j].name, Mask, FileList);
    }
  }
}

bool VfsNode::GetFileTime (const char *FileName, csFileTime &oTime) const
{
  csString FileToFind;
  for (size_t i = 0; i < MappedPaths.Length(); i++)
  {
    if (MappedPaths[i].symlink)
    {
       if (ParentVFS->GetDirectoryNode(MappedPaths[i].name)->GetFileTime(FileName, oTime))
         return true;
    }
    else
    {
       FileToFind.Clear();
       FileToFind = VirtualPathname;
       FileToFind.Append(VFS_PATH_SEPARATOR);
       FileToFind.Append(FileName);
       iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[i].pluginIndex);
       if (fs->GetFileTime((const char *) FileToFind, oTime))
         return true;
    }
  }

  // File was not found
  return false;
}		

bool VfsNode::SetFileTime (const char *FileName, const csFileTime &iTime)
{
  csString FileToFind;
  for (size_t i = 0; i < MappedPaths.Length(); i++)
  {
    if (MappedPaths[i].symlink)
    {
       if (ParentVFS->GetDirectoryNode(MappedPaths[i].name)->SetFileTime(FileName, iTime))
         return true;
    }
    else
    {
       FileToFind.Clear();
       FileToFind = VirtualPathname;
       FileToFind.Append(VFS_PATH_SEPARATOR);
       FileToFind.Append(FileName);
       iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[i].pluginIndex);
       if (fs->SetFileTime((const char *) FileToFind, iTime))
         return true;
    }
  }

  // File was not found
  return false;
}

bool VfsNode::GetFileSize (const char *FileName, size_t &oSize)
{
  csString FileToFind;
  for (size_t i = 0; i < MappedPaths.Length(); i++)
  {
    if (MappedPaths[i].symlink)
    {
       if (ParentVFS->GetDirectoryNode(MappedPaths[i].name)->GetFileSize(FileName, oSize))
         return true;
    }
    else
    {
       FileToFind.Clear();
       FileToFind = VirtualPathname;
       FileToFind.Append(VFS_PATH_SEPARATOR);
       FileToFind.Append(FileName);
       iFileSystem *fs = ParentVFS->GetPlugin(MappedPaths[i].pluginIndex);
       if (fs->GetFileSize((const char *) FileToFind, oSize))
         return true;
    }
  }

  // File was not found
  return false;
}

// --------------------------------------------------------- VfsVector --- //
int VfsVector::Compare (VfsNode* const& Item1, VfsNode* const& Item2)
{
  return strcmp ((const char *) Item1->VirtualPathname, (const char *) Item2->VirtualPathname);
}


// ------------------------------------------------------------- csVFS --- //

SCF_IMPLEMENT_FACTORY (csVFS)

// csVFS contructor
csVFS::csVFS (iBase *iParent) :
  scfImplementationType(this, iParent),
  RootNode(0),
  CwdNode(0),
  auto_name_counter(0),
  object_reg(0)
{
  mutex = csMutex::Create (true); // We need a recursive mutex.

  // Always register native filesystem plugin at index 0;
  fsPlugins.Push(new csNativeFileSystem(this));

  RootNode = new VfsNode("/", this, 0);
 
  // Testing purposes only

  //--------cut here-------------

  VfsNode *node;

  node = new VfsNode("/adirectory" , this, RootNode);
  node = new VfsNode("/cool" , this, RootNode);
  node = new VfsNode("/done" , this, RootNode);
  node = new VfsNode("/mnt" , this, RootNode);

  new VfsNode("/mnt/alternate" , this, node);
  new VfsNode("/mnt/other" , this, node);

  node = new VfsNode("/things" , this, RootNode);
  node = new VfsNode("/tmp" , this, RootNode);
 
  //--------cut here-------------

  CwdNode = RootNode;
}

// csVFS destructor
csVFS::~csVFS ()
{
  // Free memory used by VFS Nodes
  delete RootNode;
}

const char * csVFS::GetCwd () const
{ 
  return CwdNode->GetName(); 
}

bool csVFS::Initialize (iObjectRegistry* r)
{
  // TODO: complete initialize
  object_reg = r;
  return true;
}

bool csVFS::ChDir (const char *Path)
{
  if (strlen(Path) == 0)
    return false;

  csScopedMutexLock lock (mutex);

  VfsNode *newCwd = GetDirectoryNode(Path);
  
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

  csScopedMutexLock lock (mutex);

	// Pop the stack and change the cwd
	CwdNode = DirectoryStack.Pop();
	return true;
}

csString csVFS::_ExpandPath (const char *Path) const
{
  csString ExpandedPath;

  // Split string up
  csStringArray directories;
  directories.SplitString(Path, "/");

  size_t loop = 1;

  csScopedMutexLock lock (mutex);

  if (Path[0] != VFS_PATH_SEPARATOR)
  {
    ExpandedPath = CwdNode->VirtualPathname;
    loop = 0;
  }

  while(loop < directories.Length() && directories[loop])
  {
    if (strcmp(directories[loop], "..") == 0)
    {
      size_t newlen = ExpandedPath.FindLast(VFS_PATH_SEPARATOR);
      ExpandedPath.Truncate(newlen);
    }
    else if (strcmp(directories[loop], ".") == 0)
    {
      // Do nothing
    }
    else if (strcmp(directories[loop], "~") == 0)
    {
      ExpandedPath = "/~/";
    }
    else
    {
      ExpandedPath.Append(VFS_PATH_SEPARATOR);
      ExpandedPath.Append(directories[loop]);
    }
    loop++;
  }

  return ExpandedPath;
}

csPtr<iDataBuffer> csVFS::ExpandPath (const char *Path, bool IsDir) const
{
  if (!Path)
    return 0;

  csString strXp = _ExpandPath(Path);
  char *xp = new char[strXp.Length() +1];
  strcpy(xp, (const char *) strXp);
  xp[strXp.Length()] = 0;

  return csPtr<iDataBuffer> (new csDataBuffer (xp, strlen (xp) + 1));
}

bool csVFS::Exists (const char *Path) const
{
  if (strlen(Path) == 0)
    return false;

  csScopedMutexLock lock (mutex);

  // Find the parent directory of the files we are checking
  VfsNode *node = GetParentDirectoryNode(Path, false);

  if (!node)
	  return false;
 
  return node->ContainsFile(ExtractFileName(Path));
}

csPtr<iStringArray> csVFS::FindFiles (const char *Path) const
{
//  printf("Called FindFiles('%s')\n", Path); 
  csScopedMutexLock lock (mutex);

  // Get parent node
	VfsNode *node = GetDirectoryNode(Path);

  csString strMask;

	if (!node)
  {
    node = GetParentDirectoryNode(Path, false);
    if (!node)
    {
      return 0;
    }
    strMask = ExtractFileName(Path);
  }

//  printf("Finding files in node '%s'\n", node->GetName()); 

  // Array to store cosntents
  scfStringArray *fl = new scfStringArray;

  if (strMask.Length() == 0)
  {
    strMask = "*";
  }

  // Get file from node
  node->FindFiles((const char *) strMask, fl);

  fl->Sort(true);

  csPtr<iStringArray> v(fl);
  return v;
}

csPtr<iFile> csVFS::Open (const char *FileName, int Mode)
{
  if (!FileName)
    return 0;
  
  csScopedMutexLock lock (mutex);

  // Get the directory node
  VfsNode *node = GetParentDirectoryNode(FileName, false);

  if (!node)
   return 0;

  // Get the file pointer
  iFile *f = node->Open(FileName, Mode);

  // Return the results
  return csPtr<iFile> (f);
}
  
csPtr<iDataBuffer> csVFS::ReadFile (const char *FileName, bool nullterm)
{
  // Lock
  csScopedMutexLock lock (mutex);

  // Open File for reading
  csRef<iFile> F (Open (FileName, VFS_FILE_READ));

  if (!F)
    return 0;

  // Get size
  size_t Size = F->GetSize();

  // Get Data
  csRef<iDataBuffer> data (F->GetAllData(nullterm));
  if (data)
  {
    return csPtr<iDataBuffer> (data);
  }

  char *buff = new char [Size + 1];
  if (!buff)
    return 0;

  // Make the file zero-terminated in the case we'll use it as an ASCIIZ string
  buff [Size] = 0;
  if (F->Read(buff, Size) != Size)
  {
    delete [] buff;
    return 0;
  }

  return csPtr<iDataBuffer> (new csDataBuffer (buff, Size));
}

bool csVFS::WriteFile (const char *FileName, const char *Data, size_t Size)
{
  // Acquire Lock
  csScopedMutexLock lock (mutex);

  // Open File for writing
  csRef<iFile> F (Open (FileName, VFS_FILE_WRITE));

  if (!F)
    return false;

  return (F->Write (Data, Size) == Size);
}

bool csVFS::DeleteFile (const char *FileName)
{
  if (!FileName)
    return false;

  csScopedMutexLock lock (mutex);

  // Get the parent node
  VfsNode *node = GetParentDirectoryNode(FileName, false);

  if (!node)
    return false;

  return node->Delete(FileName);	
}

bool csVFS::Sync ()
{
  // Used with Cached Filesystems
  // TODO: investigate how to handle this
	return true;	
}

bool csVFS::Mount (const char *VirtualPath, const char *RealPath)
{
  // Mount without priority and autodetect plugin
  return Mount(VirtualPath, RealPath, 0, 0);
}

bool csVFS::Mount(const char *VirtualPath, const char *RealPath, int priority, size_t plugin)
{
  if (!VirtualPath || !RealPath)
	  return false;

  printf(" [+] Mounting %s at %s\n", RealPath, VirtualPath);

  // Create the node for the mount
  csString tmp = VirtualPath;
  tmp.Append("/tmp");

  csScopedMutexLock lock (mutex);
  VfsNode *node = GetParentDirectoryNode((const char *) tmp, true);

  if (!node)
  {
    printf(" [!] Could not create node !!\n");
	  return false;
  }
  // Create the data structure
  struct MappedPathData pathData;
  pathData.name = RealPath;
  pathData.priority = priority;
  pathData.pluginIndex = plugin;
  pathData.symlink = false;

  // Check if a plugin index was given
  if (plugin != 0)
  {
    node->MappedPaths.InsertSorted(pathData, CompareMappedPaths);
    return true;
  }

  // Real Path is a directory
  if (isDirectory(RealPath))
  {
    node->MappedPaths.InsertSorted(pathData, CompareMappedPaths);
	  return true;
  }

  // Find the correct plugin for handling the RealPath
  for (size_t i = 1; i < fsPlugins.Length(); i++)
  {
    // Start from 1 because we don't want to check the NativeFileSystem
    if(fsPlugins[i]->CanHandleMount(RealPath))
    {
      pathData.pluginIndex = i;
      node->MappedPaths.InsertSorted(pathData, CompareMappedPaths);
      return true;
    }
  }

  // Finally, check if nativeFileSystem can handle mount
  if(fsPlugins[0]->CanHandleMount(RealPath))
  {
      node->MappedPaths.InsertSorted(pathData, CompareMappedPaths);
      return true;
  }

  printf("[!] Real Path is not valid !!");
  // RealPath is not valid
  return false;	
}

bool csVFS::Unmount (const char *VirtualPath, const char *RealPath)
{
  if (!VirtualPath)
	  return false;

  csScopedMutexLock lock (mutex);

  // Get the appropriate node
  VfsNode *node = GetDirectoryNode(VirtualPath);

  // Node does not exists
  if (!node)
    return false;
  

  if (!RealPath)
  {
    if (node->ParentNode)
      node->ParentNode->RemoveSubDirectory(node);
    return true;
  }

  // Find the path within the node
  for (size_t i = 0; i < node->MappedPaths.Length(); i++)
  {
    // Node must be a real path
    if (!node->MappedPaths[i].symlink)
    {
      if (node->MappedPaths[i].name.Compare(RealPath))
      {
        // Delete the RealPAth if it is found
        return node->MappedPaths.DeleteIndex(i);
      }
    }
  }
  
  // Real Path was not found within the node
  return false;	
}
  
csRef<iStringArray> csVFS::MountRoot (const char *VirtualPath)
{
  scfStringArray* outv = new scfStringArray;

  csScopedMutexLock lock (mutex);

  if (VirtualPath != 0)
  {
    csRef<iStringArray> roots = csInstallationPathsHelper::FindSystemRoots();
    size_t i;
    size_t n = roots->Length ();

    VfsNode *node;
    for (i = 0 ; i < n ; i++)
    {
      char const* t = roots->Get(i);

      csString s(t);
      size_t const slen = s.Length();
      char c = '\0';

      csString vfs_dir;
      vfs_dir << VirtualPath << '/';

      for (size_t j = 0; j < slen; j++)
      {
        c = s.GetAt(j);
        if (c == '_' || c == '-' || isalnum(c))
	      vfs_dir << (char)tolower(c);
      }

      csString real_dir(s);

// Remove trailing slash
#if !(defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32))
      if (slen > 0 && (c = real_dir.GetAt(slen - 1)) == '/' || c == '\\')
        real_dir.Truncate(slen - 1);
#else
      if (slen > 4 && (c = real_dir.GetAt(slen - 1)) == '/' || c == '\\')
        real_dir.Truncate(slen - 1);
#endif

      outv->Push (vfs_dir);

      vfs_dir.Append("/tmp");

      node = GetParentDirectoryNode((const char *) vfs_dir, true);

      if (!node)
      {
        continue;
      }
      // Create the data structure
      struct MappedPathData pathData;
      pathData.name = real_dir;
      pathData.priority = 0;
      pathData.pluginIndex = 0;
      pathData.symlink = false;

      node->MappedPaths.InsertSorted(pathData, CompareMappedPaths);
    }
  }

  csRef<iStringArray> v(outv);
  outv->DecRef ();
  return v;
}

bool csVFS::SaveMounts (const char *FileName)
{
   // TODO: implement
	return true;
}

bool csVFS::LoadMountsFromFile (iConfigFile* file)
{
  // TODO: implement
  return true;	
}

bool csVFS::TryChDirAuto(const char *Path, const char *FileName)
{
  csScopedMutexLock lock (mutex);

  // Check if the VFS Path exists
  VfsNode *node = GetDirectoryNode(Path);

  if (!node)
    return false;


  if (FileName)
  {
     if (node->ContainsFile(FileName))
     {
        CwdNode = node;
        return true;
     }
  }
  else
  {
    CwdNode = node;
    return true;
  }

  return false;
}

bool csVFS::ChDirAuto (const char* path, const csStringArray* paths,
					   const char* vfspath, const char* filename)
{
  // Check if the VFS Path exists
  if (TryChDirAuto(path, filename))
    return true;

  // Check the paths
  if (paths)
  {
    for (size_t i = 0; i < paths->Length(); i++)
    {
      csString testpath = paths->Get(i);
      if (testpath.GetAt(testpath.Length() - 1) != VFS_PATH_SEPARATOR && path[0] != VFS_PATH_SEPARATOR)
        testpath.Append(VFS_PATH_SEPARATOR);
      else if (testpath.GetAt(testpath.Length() - 1) == VFS_PATH_SEPARATOR && path[0] == VFS_PATH_SEPARATOR)
        testpath.Truncate(testpath.Length() -1);

      testpath.Append(path);

      if (TryChDirAuto((const char *) testpath, filename))
        return true;
    }
  }
  
  csString vpath;

  if (vfspath)
    vpath = vfspath;
  else
  {
    vpath.Format("/tmp/__automount%d__", auto_name_counter);
    auto_name_counter++;
  }

  if (!Mount(vpath, path))
    return false;

  return ChDir(vpath);
}

bool csVFS::GetFileTime (const char *FileName, csFileTime &oTime) const
{
  if (!FileName)
    return false;

  csScopedMutexLock lock (mutex);

  VfsNode *node = GetParentDirectoryNode(FileName, false);

  if (!node)
    return false;

  return node->GetFileTime(FileName, oTime);
}		

bool csVFS::SetFileTime (const char *FileName, const csFileTime &iTime)
{
  if (!FileName)
    return false;

  csScopedMutexLock lock (mutex);

  VfsNode *node = GetParentDirectoryNode(FileName, false);

  if (!node)
    return false;

  return node->SetFileTime(FileName, iTime);
}

bool csVFS::GetFileSize (const char *FileName, size_t &oSize)
{
  if (!FileName)
    return false;

  csScopedMutexLock lock (mutex);

  VfsNode *node = GetParentDirectoryNode(FileName, false);

  if (!node)
    return false;

  return node->GetFileSize(FileName, oSize);
}

csPtr<iDataBuffer> csVFS::GetRealPath (const char *FileName)
{
  if (!FileName)
    return 0;

  csScopedMutexLock lock (mutex);
  VfsNode *node = GetParentDirectoryNode(FileName, false);

  if (!node)
    return false;

  csString path;
  
  if (!node->GetRealPath(ExtractFileName(FileName), path))
    return 0;

  return csPtr<iDataBuffer> (new csDataBuffer (csStrNew ((const char *) path), path.Length() + 1));
}

// Get virtual mount paths
csRef<iStringArray> csVFS::GetMounts ()
{
  // An array to hold the mounts
  scfStringArray* mounts = new scfStringArray;

  csScopedMutexLock lock (mutex);

  // Get all mounted nodes
  RootNode->GetMounts(mounts);  

  csRef<iStringArray> m (mounts);
  mounts->DecRef ();

  return m;

}

csRef<iStringArray> csVFS::GetRealMountPaths (const char *VirtualPath)
{
  if (!VirtualPath)
    return 0;

  csScopedMutexLock lock (mutex);

  // Find the node
  VfsNode *node = GetDirectoryNode(VirtualPath);
  
  if (!node)
    return 0;

  scfStringArray *rmounts = new scfStringArray;

  // Copy the real paths
  for (size_t i = 0; i < node->MappedPaths.Length(); i++)
  {
    if (!node->MappedPaths[i].symlink)
	    rmounts->Push(node->MappedPaths[i].name);
  }

  // return the results
  csRef<iStringArray> r (rmounts);
  rmounts->DecRef();

  return r;
}

// Register a filesystem plugin
size_t csVFS::RegisterPlugin(iFileSystem *FileSystem)
{
  csScopedMutexLock lock (mutex);

	// Add the plugin
	return fsPlugins.PushSmart(FileSystem);
}

// Create or add a symbolic link
bool csVFS::SymbolicLink(const char *Target, const char *Link, int priority)
{
  csScopedMutexLock lock (mutex);

  VfsNode *Directory = GetDirectoryNode(Link);

  // Node does not exist
  if (!Directory)
  {
    // Create the node
	  csString tmp = Link;
	  tmp.Append("/tmp");
	  Directory = GetParentDirectoryNode((const char *) tmp, true);
  }

  struct MappedPathData mp;
  mp.name = Target;
  mp.pluginIndex = 0;
  mp.priority = priority;
  mp.symlink = true;

  // Add the target to the symlinks of the directory
  Directory->MappedPaths.InsertSorted(mp, CompareMappedPaths);

  return true;
}

// Get a node corresponding to a directory
VfsNode* csVFS::GetDirectoryNode(const char *Path) const
{
  csString strPath = Path;

  // Remove trailing slash
  if (strPath.GetAt(strPath.Length() - 1) == VFS_PATH_SEPARATOR)
    strPath.Truncate(strPath.Length() - 1);

  strPath.Append("/tmp");

  return GetParentDirectoryNode((const char *) strPath, false);
}

// Get a node corresponding to a parent directory
VfsNode* csVFS::GetParentDirectoryNode(const char *path, bool create) const
{
  if (strlen(path) == 0)
	  return 0;

  csString strPath = path;

  // Remove trailing slash
  if (strPath.GetAt(strPath.Length() - 1) == VFS_PATH_SEPARATOR)
    strPath.Truncate(strPath.Length() - 1);

  csStringArray directories;
  directories.SplitString((const char *) strPath, "/");

  VfsNode * node;
  VfsNode * tmpNode;
  size_t counter = 0;
  csString currentDir;

  // Absolute Path
  if (path[0] == VFS_PATH_SEPARATOR)
  {
    node = RootNode;
	  counter = 1;
	  currentDir = "";
  }
  // Relative Path
  else
  {
    node = CwdNode;
	  if (strcmp(node->GetName(), "/") != 0)
    {
	    currentDir = node->GetName();
    }
    else
    {
      node = RootNode;
	    currentDir = "";
    }
  }

  while(counter < directories.Length() - 1)
  {
    if (strcmp (directories[counter], "..") == 0)
    {
      if (node->ParentNode)
       node = node->ParentNode;

      counter++;
      continue;
    }
    else if (strcmp (directories[counter], ".") == 0)
    {
      counter++;
      continue;
    }
    currentDir.Append(VFS_PATH_SEPARATOR);
	  currentDir.Append(directories[counter]);
	  tmpNode = node->FindNode((const char *) currentDir);
	  if (!tmpNode)
	  {
	    if (!create)
	    {
          return 0;
	    }
      tmpNode = new VfsNode((const char *)currentDir, this, node);
	  }
	  node = tmpNode;
	  counter++;
  }
  return node;
}

bool csVFS::isDirectory(const char *path)
{
  struct stat stats;
  if (stat (path, &stats) == 0)
    return false;

  // path is a directory
  return ((stats.st_mode & _S_IFDIR) != 0);
}

iFileSystem* csVFS::GetPlugin(size_t index) const
{
  return fsPlugins.Get(index);
}

} CS_PLUGIN_NAMESPACE_END(vfs)
