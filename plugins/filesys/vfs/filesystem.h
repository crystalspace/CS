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

#ifndef __CS_FILESYSTEM_H__
#define __CS_FILESYSTEM_H__

#include "csutil/scf_implementation.h"
#include "csutil/archive.h"
#include "csutil/physfile.h"
#include "csutil/scopedmutexlock.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(vfs)
{

class csVFS;
class VfsNode;

/**
 * TODO: write class description
 */
class csFileSystem 
: public scfImplementation2<csFileSystem, iFileSystem, iComponent>
{
public:

  /**
   * TODO: write class description
   */
  class csFile : public scfImplementation1<csFile, iFile>
  {
    protected:
	  // File size (initialized in constructor)
      size_t Size;
      // Filename in VFS
      char *Name;
      // Error status
      int Error;
      // The constructor for csFile
      csFile (const char *Name);
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
  };

  /// Vritual Destructor
  virtual ~csFileSystem ();

  /// Initialize the File System Plugin
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// Replacement for standard fopen()
  virtual iFile* Open(const char * FileName, int mode) = 0;

  /// Delete a file on VFS
  virtual bool Delete(const char * FileName) = 0;

  // Check if must be abstract
  virtual csVFSFileKind Exists(const char * FileName)  = 0;

  // Can this plugin handle a mount
  virtual bool CanHandleMount(const char *FileName) = 0;

  // Get names of files within the path
  virtual void GetFilenames(const char *Path, const char *Mask, 
    iStringArray *Names) = 0;

  // Query file date/time.
  virtual bool GetFileTime (const char *FileName, 
    csFileTime &oTime) const = 0;
  
  // Set file date/time.
  virtual bool SetFileTime (const char *FileName, 
    const csFileTime &iTime) = 0;

  // Query file size (without opening it).
  virtual bool GetFileSize (const char *FileName, size_t &oSize) = 0;

  // Sync
  virtual bool Sync () = 0;

protected:
  /// The constructor for csFileSystem
  csFileSystem(iBase *iParent = 0);

  // Reference to the object registry.
  iObjectRegistry *object_reg;

  friend class csVFS;
};

/**
 * TODO: write class description
 * The native file system plugin will use csPhysicalFile for 
 * access to the physical file system
 */
class csNativeFileSystem 
	: public scfImplementationExt0<csNativeFileSystem, csFileSystem>
{
public:
  /// The constructor for csFileSystem
  csNativeFileSystem(iBase *iParent = 0);

  /// Virtual destructor
  virtual ~csNativeFileSystem();

	/// Replacement for standard fopen()
  virtual iFile* Open(const char * FileName, int mode);

  /// Delete a file on VFS
  virtual bool Delete(const char * FileName);

  // Check if must be abstract
  virtual csVFSFileKind Exists(const char * FileName);

  // Can this plugin handle a mount
  virtual bool CanHandleMount(const char *FileName);

  // Get names of files within the path
  virtual void GetFilenames(const char *Path, const char *Mask, 
    iStringArray *Names);

  // Query file date/time.
  virtual bool GetFileTime (const char *FileName, csFileTime &oTime) const;
  
  // Set file date/time.
  virtual bool SetFileTime (const char *FileName, const csFileTime &iTime);

  // Query file size (without opening it).
  virtual bool GetFileSize (const char *FileName, size_t &oSize);

  // Sync
  virtual bool Sync ();
};

// Minimal time (msec) that an unused archive will be kept unclosed
#define VFS_KEEP_UNUSED_ARCHIVE_TIME	10000

/**
 * TODO: write class description
 */
class csArchiveFileSystem 
	: public scfImplementationExt0<csArchiveFileSystem, csFileSystem>
{
public:

  class VfsArchive;

  /**
   * TODO: write class description
   */
  class csArchiveFile : public scfImplementationExt0<csArchiveFile, csFile>
  {
    private:
      // parent archive
      VfsArchive *Archive;

      // The file handle
      void *FileHandle;

      // file data (for read mode)
      char *FileData;

      // buffer, where read mode data is contained
      csRef<iDataBuffer> DataBuffer;

      // current data pointer
      size_t fpos;

      // constructor
      csArchiveFile(int Mode, const char *Name, VfsArchive *ParentArchive);

      friend class csArchiveFileSystem;
    public:
      // destructor
      virtual ~csArchiveFile();

      // read a block of data
      virtual size_t Read (char *Data, size_t DataSize);

      // write a block of data
      virtual size_t Write (const char *Data, size_t DataSize);

      // check for EOF
      virtual bool AtEOF ();

      /// flush stream
      virtual void Flush ();

      /// Query current file pointer
      virtual size_t GetPos ();

      /// Get all the data at once
      virtual csPtr<iDataBuffer> GetAllData (bool nullterm = false);

      /// Set current file pointer
      virtual bool SetPos (size_t newpos);
  };

  /**
   * TODO: write class description
   */
  class VfsArchive : public csArchive
  {
    public:
      /// Mutex to make VFS thread-safe.
      csRef<csMutex> archive_mutex;

      // Last time this archive was used
      long LastUseTime;

      // Number of references (open files) to this archive
      int RefCount;

      // number of open for writing files in this archive
      int Writing;

      // The system driver
      iObjectRegistry *object_reg;
    
      // Constructor
      VfsArchive (const char *filename);

      // Destructor
      virtual ~VfsArchive ();

      // Update archive time
      void UpdateTime ();

      // Reference counting
      void IncRef ();

      // Reference counting
      void DecRef ();


      bool CheckUp ();
    };

  /// The constructor for csFileSystem
  csArchiveFileSystem(iBase *iParent = 0);

  /// Virtual destructor
  virtual ~csArchiveFileSystem();

	/// Replacement for standard fopen()
  virtual iFile* Open(const char * FileName, int mode);

  /// Delete a file on VFS
  virtual bool Delete(const char * FileName);

  // Check if must be abstract
  virtual csVFSFileKind Exists(const char * FileName);

  // Can this plugin handle a mount
  virtual bool CanHandleMount(const char *FileName);

  // Get names of files within the path
  virtual void GetFilenames(const char *Path, const char *Mask, 
    iStringArray *Names);

    // Query file date/time.
  virtual bool GetFileTime (const char *FileName, csFileTime &oTime) const;
  
  // Set file date/time.
  virtual bool SetFileTime (const char *FileName, 
    const csFileTime &iTime);

  // Query file size (without opening it).
  virtual bool GetFileSize (const char *FileName, size_t &oSize);
  
  // Sync
  virtual bool Sync ();

private:

  // Extract the name of an archive from the path
  csString ExtractArchiveName(const char *path) const;

  // Extract the name of a file within an archive from the path
  csString ExtractFileName(const char *path) const;

  // Return the archive that contains a file
  VfsArchive *FindFile(const char *FileName) const;
};

} CS_PLUGIN_NAMESPACE_END(vfs)

#endif  // __CS_FILESYSTEM_H__

