/*
    Crystal Space Virtual File System SCF interface
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

#ifndef __IVFS_H__
#define __IVFS_H__

#include "csutil/scf.h"
#include "iplugin.h"

// forward declarations
struct iStrVector;
struct tm;

/// Composite path divider
#define VFS_PATH_DIVIDER        ','
/// The "virtual" path separator
#define VFS_PATH_SEPARATOR      '/'
/// The maximal "virtual" path+filename length
#define VFS_MAX_PATH_LEN        256

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

SCF_VERSION (iFile, 0, 0, 1);

/// A replacement for FILE type in the virtual file space
struct iFile : public iBase
{
  /// Query file name (in VFS)
  virtual const char *GetName () = 0;
  /// Query file size
  virtual size_t GetSize () = 0;
  /// Check (and clear) file last error status
  virtual int GetStatus () = 0;

  /// Replacement for standard fread()
  virtual size_t Read (char *Data, size_t DataSize) = 0;
  /// Replacement for standard fwrite()
  virtual size_t Write (const char *Data, size_t DataSize) = 0;
  /// Replacement for standard feof()
  virtual bool AtEOF () = 0;
  /// Query current file pointer
  virtual size_t GetPos () = 0;

  /**
   * This function is intended for VFS internal use only. It is not guaranteed
   * to work for all file types (currently it works only for archive files)
   */
  virtual char *GetAllData () = 0;
};

// Ugly Win32 hack
#ifdef DeleteFile
#  undef DeleteFile
#endif

SCF_VERSION (iVFS, 0, 0, 2);

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
struct iVFS : public iPlugIn
{
  /// Initialize the Virtual File System plugin
  virtual bool Initialize (iSystem *iSys) = 0;

  /// Set current working directory
  virtual bool ChDir (const char *Path) = 0;
  /// Get current working directory
  virtual const char *GetCwd () const = 0;

  /// Push current directory
  virtual void PushDir () = 0;
  /// Pop current directory
  virtual bool PopDir () = 0;

  /**
   * Expand given virtual path, interpret all "." and ".."'s relative to
   * 'currend virtual directory'. Return a (new char [...])'ed string.
   * If IsDir is true, expanded path ends in an '/', otherwise no.
   */
  virtual char *ExpandPath (const char *Path, bool IsDir = false) const = 0;

  /// Check whenever a file exists
  virtual bool Exists (const char *Path) const = 0;

  /// Find all files in a virtual directory and return an array with their names
  virtual iStrVector *FindFiles (const char *Path) const = 0;
  /// Replacement for standard fopen()
  virtual iFile *Open (const char *FileName, int Mode) = 0;
  /**
   * Get an entire file at once. You should delete[] returned data
   * after usage. This is more effective than opening files and reading
   * the file in blocks.
   */
  virtual char *ReadFile (const char *FileName, size_t &Size) = 0;
  /// Write an entire file in one pass.
  virtual bool WriteFile (const char *FileName, const char *Data, size_t Size) = 0;

  /// Delete a file on VFS
  virtual bool DeleteFile (const char *FileName) = 0;

  /// Close all opened archives, free temporary storage etc.
  virtual bool Sync () = 0;

  /// Mount an VFS path on a "real-world-filesystem" path
  virtual bool Mount (const char *VirtualPath, const char *RealPath) = 0;
  /// Unmount an VFS path; if RealPath is NULL, entire VirtualPath is unmounted
  virtual bool Unmount (const char *VirtualPath, const char *RealPath) = 0;

  /// Save current configuration back into configuration file
  virtual bool SaveMounts (const char *FileName) = 0;

  /// Query file date/time
  virtual bool GetFileTime (const char *FileName, tm &ztime) const = 0;
  /// Set file date/time
  virtual bool SetFileTime (const char *FileName, tm &ztime) = 0;
};

#endif // __IVFS_H__
