/*
    ZIP archive support for Crystal Space 3D library
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

#ifndef __CS_ARCHIVE_H__
#define __CS_ARCHIVE_H__

/**\file
 * ZIP archive support
 */

#include "csextern.h"

#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/parray.h"
#include "csutil/ref.h"
#include "csutil/stringarray.h"
#include "csutil/zip.h"

struct csFileTime;

/**
 * This class can be used to work with standard ZIP archives.
 * Constructor accepts a file name - if such a file is not found, it is
 * created. After this you can examine archive directory, read files,
 * delete or write files in archive.
 * 
 * Operations which changes archive file will be deferred until Flush()
 * method is called. Before calling Flush() you can do any number of
 * deletions and writes, but read operations will not be affected by
 * these until Flush() will be called.
 * 
 * Known quirks:
 * - No CRC check is done on reading, although ZIP file format allows it.
 *   This design 'flaw' was allowed to achieve maximal speed. However, when
 *   a file is added to archive, its CRC is computed and updated correctly.
 * - Several methods of the csArchive class requires approximatively 20K of
 *   stack space when invoked.
 * - Doesn't like files >4GB.
 */
class CS_CRYSTALSPACE_EXPORT csArchive
{
public:
  static const char hdr_central[4];
  static const char hdr_local[4];
  static const char hdr_endcentral[4];
  static const char hdr_extlocal[4];

private:
  /// csArchive entry class
  class ArchiveEntry
  {
  public:
    char *filename;
    ZIP_central_directory_file_header info;
    char *buffer;
    size_t buffer_pos;
    size_t buffer_size;
    char *extrafield, *comment;
    bool faked;

    ArchiveEntry (const char *name, ZIP_central_directory_file_header &cdfh);
    ~ArchiveEntry ();
    bool Append (const void *data, size_t size);
    bool WriteLFH (iFile* file);
    bool WriteCDFH (iFile* file);
    bool ReadExtraField (iFile* file, size_t extra_field_length);
    bool ReadFileComment (iFile* file, size_t file_comment_length);
    bool WriteFile (iFile* file);
    void FreeBuffer ();
  };
  friend class ArchiveEntry;

  /// A vector of ArchiveEntries
  class CS_CRYSTALSPACE_EXPORT ArchiveEntryVector
  	: public csPDelArray<ArchiveEntry, CS::Container::ArrayAllocDefault,
  	                     csArrayCapacityFixedGrow<256> >
  {
  public:
    ArchiveEntryVector () : csPDelArray<ArchiveEntry, 
      CS::Container::ArrayAllocDefault, csArrayCapacityFixedGrow<256> > (256) {}
    static int Compare (ArchiveEntry* const& Item1, ArchiveEntry* const& Item2)
    { return strcmp (Item1->filename, Item2->filename); }
    static int CompareKey (ArchiveEntry* const& Item, char const* const& Key)
    { return strcmp (Item->filename, Key); }
  };

  ArchiveEntryVector dir;	// Archive directory: chain head (sorted)
  csStringArray del;		// Files that should be deleted (sorted)
  csArray<ArchiveEntry*> lazy;	// Lazy operations (unsorted)

  char *filename;		// Archive file name
  // Archive file pointer.
  csRef<iFile> file;

  size_t comment_length;	// Archive comment length
  char *comment;		// Archive comment

  void ReadDirectory ();
  bool IsDeleted (const char *name) const;
  void UnpackTime (ush zdate, ush ztime, csFileTime &rtime) const;
  void PackTime (const csFileTime &ztime, ush &rdate, ush &rtime) const;
  bool ReadArchiveComment (iFile* file, size_t zipfile_comment_length);
  void LoadECDR (ZIP_end_central_dir_record &ecdr, char *buff);
  bool ReadCDFH (ZIP_central_directory_file_header &cdfh, iFile* file);
  bool ReadLFH (ZIP_local_file_header &lfh, iFile* file);
  bool WriteECDR (ZIP_end_central_dir_record &ecdr, iFile* file);
  bool WriteZipArchive ();
  bool WriteCentralDirectory (iFile* temp);
  void UpdateDirectory ();
  void ReadZipDirectory (iFile *infile);
  ArchiveEntry *InsertEntry (const char *name,
    ZIP_central_directory_file_header &cdfh);
  void ReadZipEntries (iFile* infile);
  bool ReadEntry (iFile* infile, ArchiveEntry *f, char* buf);
  ArchiveEntry *CreateArchiveEntry (const char *name,
    size_t size = 0, bool pack = true);
  void ResetArchiveEntry (ArchiveEntry *f, size_t size, bool pack);

public:
  /// Open the archive.
  csArchive (const char *filename);
  /// Close the archive.
  ~csArchive ();

  /// Type a directory listing of the archive to the console.
  void Dir () const;

  /**
   * Create a new file in the archive. If the file already exists
   * it will be overwritten.
   * 
   * Returns 0 if not succesful. Otherwise it returns a pointer
   * that can be passed to 'Write' routine. You won't see any changes
   * to archive until 'Flush' will be called.
   * 
   * 'size' is the _advisory_ file size. There is no problem if you will
   * write more or less bytes, its just a matter of performance - if you
   * set the right size, archive manager will have to allocate memory
   * only once; however if you set size to zero and then write all the
   * data in one call, it will have same performance.
   */
  void *NewFile (const char *name, size_t size = 0, bool pack = true);

  /**
   * Delete a file from the archive. You won't see any changes
   * to archive until 'Flush' will be called.
   */
  bool DeleteFile (const char *name);

  /**
   * Return true if a path exists. Also return the
   * size of the file if needed.
   */
  bool FileExists (const char *name, size_t *size = 0) const;

  /**
   * Read a file completely. After finishing with the returned
   * data you need to 'delete[]' it. If the file does not exists
   * this function returns 0. If "size" is not null, it is set
   * to unpacked size of the file.
   */
  char *Read (const char *name, size_t *size = 0);

  /**
   * Read a file completely into a buffer allocated with the given
   * allocator. If the file does not exists this function returns 0. 
   */
  template<typename Allocator>
  csPtr<iDataBuffer> Read (const char *name, Allocator& alloc)
  {
    ArchiveEntry *f = (ArchiveEntry *) FindName (name);
  
    if (!f)
      return 0;

    csRef<iDataBuffer> buf;
    buf.AttachNew (new CS::DataBuffer<Allocator> (f->info.ucsize, alloc));
    if (!ReadEntry (file, f, buf->GetData()))
      return 0;
    return csPtr<iDataBuffer> (buf);
  }

  /**
   * Read a file completely. If the file does not exists
   * this function returns 0. 
   */
  csPtr<iDataBuffer> Read (const char *name)
  {
    CS::Memory::AllocatorMalloc alloc;
    return Read (name, alloc);
  }

  /**
   * Write data to a file. Note that 'size' need not be
   * the overall file size if this was given in 'NewFile',
   * but this function will fail if the total size of written
   * data exceeds the maximum size given to 'NewFile'.
   */
  bool Write (void *entry, const char *data, size_t size);

  /**
   * Execute all pending operations involving writes to archive
   * Neither DeleteFile or NewFile will have effect until this
   * function will be called. Returns false if operation failed.
   * If operation failed, postponed operations remains in the
   * same state as before calling Flush(), i.e. for example
   * user can be prompted to free some space on drive then retry
   * Flush().
   */
  bool Flush ();

  /// Get Nth file in archive or 0
  void *GetFile (size_t no)
  { return (no < dir.GetSize ()) ? dir.Get (no) : 0; }

  /// Find a file in archive; returns a handle or 0
  void *FindName (const char *name) const;
  /// Query name from handle
  char *GetFileName (void *entry) const
  { return ((ArchiveEntry*)entry)->filename; }
  /// Query file size from handle
  size_t GetFileSize (void *entry) const
  { return ((ArchiveEntry*)entry)->info.ucsize; }
  /// Query filetime from handle
  void GetFileTime (void *entry, csFileTime &ztime) const;
  /// Set filetime for handle
  void SetFileTime (void *entry, const csFileTime &ztime);

  /// Query archive filename
  char *GetName () const
  { return filename; }
  /// Query archive comment
  char *GetComment () const
  { return comment; }
};

inline void csArchive::GetFileTime (void *entry, csFileTime &ztime) const
{
  if (entry)
  {
    UnpackTime (((ArchiveEntry*)entry)->info.last_mod_file_date,
                ((ArchiveEntry*)entry)->info.last_mod_file_time,
                ztime);
  }
}

inline void csArchive::SetFileTime (void *entry, const csFileTime &ztime)
{
  if (entry)
  {
    PackTime (ztime,
              ((ArchiveEntry*)entry)->info.last_mod_file_date,
              ((ArchiveEntry*)entry)->info.last_mod_file_time);
  }
}

#endif // __CS_ARCHIVE_H__
