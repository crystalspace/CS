/*
    Crystal Space Virtual File System class
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include "csutil/cfgfile.h"
#include "csutil/parray.h"
#include "csutil/scopedmutexlock.h"
#include "csutil/stringarray.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"		  

class VfsNode;
class csVFS;
struct iConfigFile;

/// A replacement for standard-C FILE type in the virtual file space
class csFile : public iFile
{
protected:
  // Index into parent node RPath
  int Index;
  // File node
  VfsNode *Node;
  // Filename in VFS
  char *Name;
  // File size (initialized in constructor)
  size_t Size;
  // last error code
  int Error;

  // The constructor for csFile
  csFile (int Mode, VfsNode *ParentNode, int RIndex,
  	const char *NameSuffix);

public:
  /// Instead of fclose() do "delete file" or file->DecRef ()
  virtual ~csFile ();

  /// Query file name (in VFS)
  virtual const char *GetName () { return Name; }
  /// Query file size
  virtual size_t GetSize () { return Size; }
  /// Check (and clear) file last error status
  virtual int GetStatus ();

  /// Replacement for standard fread()
  virtual size_t Read (char *Data, size_t DataSize) = 0;
  /// Replacement for standard fwrite()
  virtual size_t Write (const char *Data, size_t DataSize) = 0;
  /// Flush stream.
  virtual void Flush () = 0;
  /// Replacement for standard feof()
  virtual bool AtEOF () = 0;
  /// Query current file pointer
  virtual size_t GetPos () = 0;
  /// Get entire file data at once, if possible, or 0
  virtual csPtr<iDataBuffer> GetAllData (bool nullterm = false) = 0;
  /// Set new file pointer
  virtual bool SetPos (size_t newpos) = 0;
protected:
  friend class csVFS;
};

/**
 * The Virtual Filesystem Class is intended to be the only way for Crystal
 * Space engine to access the files. This gives unified control over the
 * way how files are found, read and written. VFS gives the following
 * goodies over the standard file i/o functions:
 * <ul>
 * <li>Multiple search directories. Several "real" directories can be
 *     collected together into one "virtual" directory.
 * <li>Directories can be mapped to "real" directories as well as to
 *     archives (.zip files). Files are compressed/decompressed
 *     transparently for clients.
 * <li>The Virtual File System is unique across all operating systems
 *     Crystal Space supports, no matter of features of the underlying OS.
 * </ul>
 * This class has only most basic features of a real filesystem: file
 * reading and writing (no simultaneous read and write mode are allowed
 * because it would be rather complex to implement it for archives).
 * However, most programs don't even need such functionality, and for
 * sure Crystal Space itself doesn't need it. Files open for writing are
 * always truncated. A simple meaning for getting a list of files in a
 * virtual directory is implemented; however the user is presented with
 * only a list of file names; no fancy things like file size, time etc
 * (file size can be determined after opening it for reading).
 */
class csVFS : public iVFS
{
private:
  /// Mutex to make VFS thread-safe.
  csRef<csMutex> mutex;

  friend class VfsNode;

  // A vector of VFS nodes
  class VfsVector : public csPDelArray<VfsNode>
  {
  public:
    static int Compare (VfsNode* const&, VfsNode* const&);
  } NodeList;

  // Current working directory (in fact, the automaticaly-added prefix path)
  // NOTE: cwd ALWAYS ends in '/'!
  char *cwd;
  // The installation directory (the value of $@)
  char *basedir;
  // Full path of application's resource directory (the value of $*)
  char *resdir;
  // Full path of the directory containing the application executable or
  // the Cocoa application wrapper (the value of $^)
  char *appdir;
  // Current node
  const VfsNode *cnode;
  // The current directory minus current node (cnode suffix)
  char cnsufx [VFS_MAX_PATH_LEN + 1];
  // The initialization file
  csConfigFile config;
  // Directory stack (used in PushDir () and PopDir ())
  csStringArray dirstack;
  // The pointer to system driver interface
  iObjectRegistry *object_reg;

public:
  SCF_DECLARE_IBASE;

  /// Initialize VFS by reading contents of given INI file
  csVFS (iBase *iParent);
  /// Virtual File System destructor
  virtual ~csVFS ();

  /// Set current working directory
  virtual bool ChDir (const char *Path);
  /// Get current working directory
  virtual const char *GetCwd () const
  { return cwd; }

  /// Push current directory
  virtual void PushDir (char const* Path = 0);
  /// Pop current directory
  virtual bool PopDir ();

  /**
   * Expand given virtual path, interpret all "." and ".."'s relative to
   * 'currend virtual directory'. Return a new iString object.
   * If IsDir is true, expanded path ends in an '/', otherwise no.
   */
  virtual csPtr<iDataBuffer> ExpandPath (
  	const char *Path, bool IsDir = false) const;

  /// Check whenever a file exists
  virtual bool Exists (const char *Path) const;

  /// Find all files in a virtual directory and return an array of their names
  virtual csPtr<iStringArray> FindFiles (const char *Path) const;
  /// Replacement for standard fopen()
  virtual csPtr<iFile> Open (const char *FileName, int Mode);
  /**
   * Get an entire file at once. You should delete[] returned data
   * after usage. This is more effective than opening files and reading
   * the file in blocks.  Note that the returned buffer is always null-
   * terminated (so that it can be conveniently used with string functions)
   * but the extra null-terminator is not counted as part of the returned size.
   */
  virtual csPtr<iDataBuffer> ReadFile (const char *FileName, bool nullterm);
  /// Write an entire file in one pass.
  virtual bool WriteFile (const char *FileName, const char *Data, size_t Size);

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

  /// Initialize the Virtual File System
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Query file local date/time
  virtual bool GetFileTime (const char *FileName, csFileTime &oTime) const;
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

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csVFS);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

private:
  /// Same as ExpandPath() but with less overhead
  char *_ExpandPath (const char *Path, bool IsDir = false) const;

  /// Read and set the VFS config file
  bool ReadConfig ();

  /// Add a virtual link: real path can contain $(...) macros
  virtual bool AddLink (const char *VirtualPath, const char *RealPath);

  /// Find the VFS node corresponding to given virtual path
  VfsNode *GetNode (const char *Path, char *NodePrefix,
    size_t NodePrefixSize) const;

  /// Common routine for many functions
  bool PreparePath (const char *Path, bool IsDir, VfsNode *&Node,
    char *Suffix, size_t SuffixSize) const;
};

#endif // __CS_VFS_H__
