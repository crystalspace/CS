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

#ifndef __CS_VFS_H__
#define __CS_VFS_H__

#include "csutil/refarr.h"
#include "csutil/parray.h"
#include "csutil/scf_implementation.h"
#include "csutil/csstring.h"
#include "csutil/cfgfile.h"
#include "csutil/scopedmutexlock.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iConfigFile;

#define VFS_AUTOCONFIGURE

CS_PLUGIN_NAMESPACE_BEGIN(vfs)
{

class VfsNode;

// Class to automatically configure the csVFS
class VfsAutoConfig
{
  public: 
    // Destructor
    virtual ~VfsAutoConfig() = 0;

    // Configure the file system
    virtual bool Configure(iVFS *vfs, iObjectRegistry *object_reg) = 0;

};

inline VfsAutoConfig::~VfsAutoConfig() { }

/**
 * An array holding pointers to nodes in the VFS, which are compared by name.
 */
class VfsVector : public csPDelArray<VfsNode>
{
   public:
      /// Comparison method
      static int Compare (VfsNode* const&, VfsNode* const&);
};

/**
 * TODO: Rewrite class description.
 */
class csVFS : public scfImplementation2<csVFS, iVFS, iComponent>
{

public:

  /// Initialize VFS by reading contents of given INI file
  csVFS (iBase *iParent);

  /// Virtual File System destructor
  virtual ~csVFS ();

  /// Initialize the Virtual File System
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Set current working directory
  virtual bool ChDir (const char *Path);

  /// Get current working directory
  virtual const char *GetCwd () const; 

  /// Push current directory
  virtual void PushDir (char const* Path = 0);

  /// Pop current directory
  virtual bool PopDir ();

  /**
   * Expand given virtual path, interpret all "." and ".."'s relative to
   * 'current virtual directory'. Return a new iString object.
   * If IsDir is true, expanded path ends in an '/', otherwise no.
   */
  virtual csPtr<iDataBuffer> ExpandPath (const char *Path, 
	  bool IsDir = false) const;

  /// Check whenever a file exists
  virtual bool Exists (const char *Path);

  /// Find all files in a virtual directory and return an array of their 
  /// names
  virtual csPtr<iStringArray> FindFiles (const char *Path);

  /// Replacement for standard fopen()
  virtual csPtr<iFile> Open (const char *FileName, int Mode);
  
  /**
   * Get an entire file at once. You should delete[] returned data
   * after usage. This is more effective than opening files and reading
   * the file in blocks.  Note that the returned buffer is always null-
   * terminated (so that it can be conveniently used with string functions)
   * but the extra null-terminator is not counted as part of the returned 
   * size.
   */
  virtual csPtr<iDataBuffer> ReadFile (const char *FileName, bool nullterm);

  /// Write an entire file in one pass.
  virtual bool WriteFile (const char *FileName, const char *Data, 
	  size_t Size);

  /// Delete a file on VFS
  virtual bool DeleteFile (const char *FileName);

  /// Close all opened archives, free temporary storage etc.
  virtual bool Sync ();

  /// Mount an VFS path on a "real-world-filesystem" path
  virtual bool Mount (const char *VirtualPath, const char *RealPath);
  /// Unmount an VFS path; if RealPath is 0, entire VirtualPath is unmounted
  virtual bool Unmount (const char *VirtualPath, const char *RealPath);
  
  /// Mount the root directory or directories 
  virtual csRef<iStringArray> MountRoot (const char *VirtualPath);

  /// Save current configuration back into configuration file
  virtual bool SaveMounts (const char *FileName);
  
  /// Load a configuration file
  virtual bool LoadMountsFromFile (iConfigFile* file);

  /// Auto-mount ChDir.
  virtual bool ChDirAuto (const char* path, const csStringArray* paths = 0,
	  const char* vfspath = 0, const char* filename = 0);

  /// Query file local date/time
  virtual bool GetFileTime (const char *FileName, csFileTime &oTime);
  /// Set file local date/time
  virtual bool SetFileTime (const char *FileName, const csFileTime &iTime);

  /// Query file size (without opening it)
  virtual bool GetFileSize (const char *FileName, size_t &oSize);

  /**
   * Query real-world path from given VFS path.
   * This will work only for files that are stored on real filesystem,
   * not in archive files. You should expect this function to return
   * 0 in this case.
   */
  virtual csPtr<iDataBuffer> GetRealPath (const char *FileName);

  /// Get all current virtual mount paths
  virtual csRef<iStringArray> GetMounts ();

  /// Get the real paths associated with a mount
  virtual csRef<iStringArray> GetRealMountPaths (const char *VirtualPath);

  /// Register a filesystem plugin
  virtual size_t RegisterPlugin(iFileSystem *FileSystem);

  /// Create or add a symbolic link
  virtual bool SymbolicLink(const char *Target, const char *Link = 0, 
    int priority = 0);

  /// Mount a file
  virtual bool Mount(const char *VirtualPath, const char *RealPath, 
    int priority, size_t plugin = 0);

  /// Return a filesystem plugin
  virtual iFileSystem* GetPlugin(size_t index) const;

  /// Load a VFS configuration File
  bool LoadConfigFile(char const* VirtualPath, bool Mount = true);

  // Clean the VFS
  bool Clean();

protected:

  /// Mutex to make VFS thread-safe.
  csRef<csMutex> mutex;

  /// Currently registered FileSystem plugins
  csRefArray<iFileSystem> fsPlugins;

  /// Reference to the object registry.
  iObjectRegistry *object_reg;

  /// A pointer to the root node of the vfs
  VfsNode *RootNode;

  /// A pointer to the current working directory
  VfsNode *CwdNode;

  /// A pointer to a stale current working directory
  VfsNode *StaleCwd;

  /// A stack to implement directory changes
  VfsVector DirectoryStack;

  /// A counter for ChDirAuto
  int auto_name_counter;

  /// The initialization file
  csConfigFile config;

  /// The install directory
  csString InstallDirectory;

  /// The user directory
  csString UserDirectory;

  /// The application directory
  csString AppDirectory;

  /// The resource directory
  csString ResourceDirectory;

  /// Get the directory node
  VfsNode* GetDirectoryNode(const char *path);

  /// Get the parent directory node from the path
  /// If create is true, the directories along the path that do not 
  // exist will be created.
  VfsNode* GetParentDirectoryNode(const char *path, 
    bool create = false, bool mount = false);

  /// Get the last closest existing ancestor directory node of the path
  VfsNode* csVFS::GetClosestDirectoryNode(const char *path);

  // Check if the path is a valid real directory
  bool isDirectory(const char *path);

  // Expand a Virtual Path
  csString _ExpandPath (const char *Path) const;

  // Expand a Real Path
  csString ExpandRealPath(char const *Path);

  // Expand a Real Path
  csString _ExpandRealPath(char const *Path);

  // Try change directory
  bool TryChDirAuto(const char *Path, const char *FileName);

  // Scan the config file and mount paths
  bool MountConfigFile(csConfigFile* conf = 0);

  // Get the value of a variable
  const char *GetValue(const char *VarName);

  friend class VfsNode;

  // Plugin for Autoconfiguration
  VfsAutoConfig *AutoConfigPluginPtr;

  friend class VfsAutoConfig;

  class AutoConfigPlugin : public VfsAutoConfig
  {
    public: 
    // Destructor
    virtual ~AutoConfigPlugin();

    // Configure the file system
    virtual bool Configure(iVFS *vfs, iObjectRegistry *object_reg);

    friend class csVFS;
  };

};

} CS_PLUGIN_NAMESPACE_END(vfs)

#endif  // __CS_VFS_H__
