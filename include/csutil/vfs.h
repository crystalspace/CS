/*
    Crystal Space Virtual File System class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __VFS_H__
#define __VFS_H__

#include "def.h"
#include "csstrvec.h"

// Composite path divider
#define VFS_PATH_DIVIDER        ','
// The "virtual" path separator
#define VFS_PATH_SEPARATOR      '/'
// The maximal "virtual" path+filename length
#define VFS_MAX_PATH_LEN        256

class csFile;
class VfsNode;
class csStrVector;
class csIniFile;

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
class csVFS : public csBase
{
  // A vector of VFS nodes
  class VfsVector : public csVector
  {
  public:
    VfsVector ();
    virtual ~VfsVector ();
    virtual bool FreeItem (csSome Item);
    virtual int Compare (csSome Item1, csSome Item2, int Mode) const;
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;
  } NodeList;

  // Current working directory (in fact, the automaticaly-added prefix path)
  // NOTE: cwd ALWAYS ends in '/'!
  char *cwd;
  // Current node
  const VfsNode *cnode;
  // The current directory minus current node (cnode suffix)
  char cnsufx [VFS_MAX_PATH_LEN + 1];
  // The initialization file
  csIniFile *config;
  // Directory stack (used in PushDir () and PopDir ())
  csStrVector dirstack;

public:
  /// Initialize VFS by reading contents of given INI file
  csVFS (csIniFile *Config);
  /// Virtual File System destructor
  virtual ~csVFS ();

  /// Add a virtual link: real path can contain $(...) macros
  bool AddLink (const char *VirtualPath, const char *RealPath);

  /// Set current working directory
  bool ChDir (const char *Path);
  /// Get current working directory
  const char *GetCwd () const
  { return cwd; }

  /// Push current directory
  void PushDir ();
  /// Pop current directory
  bool PopDir ();

  /**
   * Expand given virtual path, interpret all "." and ".."'s relative to
   * 'currend virtual directory'. Return a (new char [...])'ed string.
   * If IsDir is true, expanded path ends in an '/', otherwise no.
   */
  char *ExpandPath (const char *Path, bool IsDir = false) const;

  /// Check whenever a file exists
  bool Exists (const char *Path) const;

  /// Find all files in a virtual directory and return an array with their names
  csStrVector *FindFiles (const char *Path) const;
  /// Replacement for standard fopen()
  csFile *Open (const char *FileName, int Mode);
  /**
   * Get an entire file at once. You should delete[] returned data
   * after usage. This is more effective than opening files and reading
   * the file in blocks.
   */
  char *ReadFile (const char *FileName, size_t &Size);
  /// Write an entire file in one pass.
  bool WriteFile (const char *FileName, const char *Data, size_t Size);

  /// Delete a file on VFS
  bool DeleteFile (const char *FileName);

  /// Close all opened archives, free temporary storage etc.
  bool Sync ();

  /// Set an entry in configuration file (like "CD = /proc/cd")
  bool SetConfig (char *VarName, char *Value);
  /// Mount an VFS path on a "real-world-filesystem" path
  bool Mount (const char *VirtualPath, const char *RealPath);
  /// Unmount an VFS path; if RealPath is NULL, entire VirtualPath is unmounted
  bool Unmount (const char *VirtualPath, const char *RealPath);

  /// Save current configuration back into configuration file
  bool SaveMounts (const char *FileName);

private:
  // Callback function used to read ini file
  static bool EnumConfig (csSome Parm, char *Name, size_t DataSize, csSome Data);
  // Find the VFS node corresponding to given virtual path
  VfsNode *GetNode (const char *Path, char *NodePrefix, size_t NodePrefixSize) const;
  // Common routine for many functions
  bool PreparePath (const char *Path, bool IsDir, VfsNode *&Node,
    char *Suffix, size_t SuffixSize) const;
};

/// File open mode mask
#define VFS_FILE_MODE		0x0000000f
/// Open file for reading
#define VFS_FILE_READ		0x00000000
/// Open file for writing
#define VFS_FILE_WRITE		0x00000001
/// Store file uncompressed (no gain possible)
#define VFS_FILE_UNCOMPRESSED	0x80000000

/// File status ok
#define	VFS_STATUS_OK		0
/// Unclassified error
#define VFS_STATUS_OTHER	1
/// Device has no more space for file data
#define	VFS_STATUS_NOSPC	2
/// Not enough system resources
#define VFS_STATUS_RESOURCES	3
/// Access denied: either you have no write access, or readonly filesystem
#define VFS_STATUS_ACCESSDENIED	4
/// An error occured during reading or writing data
#define VFS_STATUS_IOERROR	5

/// A replacement for FILE type in the virtual file space
class csFile
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
  csFile (int Mode, VfsNode *ParentNode, int RIndex, const char *NameSuffix);

public:
  /// Instead of fclose() do "delete file"
  virtual ~csFile ();

  /// Query file name (in VFS)
  const char *GetName () { return Name; }
  /// Query file size
  size_t GetSize () { return Size; }
  /// Check (and clear) file last error status
  virtual int GetStatus ();

  /// Replacement for standard fread()
  virtual size_t Read (char *Data, size_t DataSize) = 0;
  /// Replacement for standard fwrite()
  virtual size_t Write (const char *Data, size_t DataSize) = 0;
  /// Replacement for standard feof()
  virtual bool AtEOF () = 0;
  /// Query current file pointer
  virtual size_t GetPos () = 0;

protected:
  friend class csVFS;
  // Get entire file data at once, if possible, or NULL
  virtual char *GetAllData ();
};

#endif // __VFS_H__
