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

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include <stdio.h>
#include <time.h>
#include "csutil/zip.h"
#include "csutil/csstrvec.h"

// Ugly Win32 hack
#ifdef DeleteFile
#  undef DeleteFile
#endif

/**
 * This class can be used to work with standard ZIP archives.
 * Constructor accepts a file name - if such a file is not found, it is
 * created. After this you can examine archive directory, read files,
 * delete or write files in archive.
 * <p>
 * Operations which changes archive file will be deferred until Flush()
 * method is called. Before calling Flush() you can do any number of
 * deletions and writes, but read operations will not be affected by
 * these until Flush() will be called.
 * <p>
 * Known quirks:
 * <ul>
 * <li>No CRC check is done on reading, although ZIP file format allows it.
 *     This design 'flaw' was allowed to achieve maximal speed. However, when
 *     a file is added to archive, its CRC is computed and updated correctly.
 * <li>Several methods of the csArchive class requires approximatively 20K of
 *     stack space when invoked.
 * </ul>
 */
class csArchive
{
public:
  static char hdr_central[4];
  static char hdr_local[4];
  static char hdr_endcentral[4];
  static char hdr_extlocal[4];

private:
  /// csArchive entry class
  class ArchiveEntry
  {
  public:
    char *filename;
    ZIP_central_directory_file_header info;
    char *buffer;
    size_t buffer_pos;
    void *extrafield, *comment;

    ArchiveEntry (const char *name, ZIP_central_directory_file_header &cdfh);
    ~ArchiveEntry ();
    bool Append (const void *data, size_t size);
    bool WriteLFH (FILE *file);
    bool WriteCDFH (FILE *file);
    bool ReadExtraField (FILE *file, size_t extra_field_length);
    bool ReadFileComment (FILE *file, size_t file_comment_length);
    bool WriteFile (FILE *file);
    void FreeBuffer ();
  };
  friend class ArchiveEntry;

  /// A vector of ArchiveEntries
  class ArchiveEntryVector : public csVector
  {
  public:
    ArchiveEntryVector () : csVector (256, 256) {}
    virtual ~ArchiveEntryVector () { DeleteAll (); }
    virtual bool FreeItem (csSome Item)
    { CHK (delete (ArchiveEntry *)Item); return true; }
    virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
    { return strcmp (((ArchiveEntry *)Item1)->filename, ((ArchiveEntry *)Item2)->filename); }
    virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
    { return strcmp (((ArchiveEntry *)Item)->filename, (char *)Key); }
    ArchiveEntry *Get (int n) const
    { return (ArchiveEntry *)csVector::Get (n); }
  };

  ArchiveEntryVector dir;	// Archive directory: chain head (sorted)
  csStrVector del;		// The array of files that should be deleted (sorted)
  ArchiveEntryVector lazy;	// The array of lazy operations (unsorted)

  char *filename;		// Archive file name
  FILE *file;			// Archive file pointer.

  size_t comment_length;	// Archive comment length
  char *comment;		// Archive comment

  void ReadDirectory ();
  bool IsDeleted (const char *name) const;
  void UnpackTime (ush zdate, ush ztime, tm &rtime) const;
  void PackTime (tm &ztime, ush &rdate, ush &rtime) const;
  bool ReadArchiveComment (FILE *file, size_t zipfile_comment_length);
  void LoadECDR (ZIP_end_central_dir_record &ecdr, char *buff);
  bool ReadCDFH (ZIP_central_directory_file_header &cdfh, FILE *file);
  bool ReadLFH (ZIP_local_file_header &lfh, FILE *file);
  bool WriteECDR (ZIP_end_central_dir_record &ecdr, FILE *file);
  bool WriteZipArchive ();
  bool WriteCentralDirectory (FILE *temp);
  void UpdateDirectory ();
  void ReadZipDirectory (FILE *infile);
  ArchiveEntry *InsertEntry (const char *name, ZIP_central_directory_file_header &cdfh);
  void ReadZipEntries (FILE *infile);
  char *ReadEntry (FILE *infile, ArchiveEntry *f);

public:
  /// Open the archive.
  csArchive (const char *filename);
  /// Close the archive.
  ~csArchive ();

  /// Type a directory listing of the archive to the console.
  void Dir () const;

  /**
   * Create a new file in the archive. If the file already exists
   * it will be overwritten. Calling NewFile twice with same filename
   * without calling Flush() inbetween will cause unpredictable results.
   * <p>
   * Returns NULL if not succesful. Otherwise it returns a pointer
   * that can be passed to 'Write' routine. You won't see any changes
   * to archive until 'Flush' will be called.
   * <p>
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
   * Return true if a file exists. Also return the
   * size of the file if needed.
   */
  bool FileExists (const char *name, size_t *size = NULL) const;

  /**
   * Read a file completely. After finishing with the returned
   * data you need to 'delete[]' it. If the file does not exists
   * this function returns NULL. If "size" is not null, it is set
   * to unpacked size of the file.
   */
  char *Read (const char *name, size_t *size = NULL);

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

  /// Get Nth file in archive or NULL
  void *GetFile (int no)
  { return dir.Get (no); }

  /// Find a file in archive; returns a handle or NULL
  void *FindName (const char *name) const;
  /// Query name from handle
  char *GetFileName (void *entry) const
  { return ((ArchiveEntry*)entry)->filename; }
  /// Query file size from handle
  size_t GetFileSize (void *entry) const
  { return ((ArchiveEntry*)entry)->info.ucsize; }
  /// Query filetime from handle
  void GetFileTime (void *entry, tm &ztime) const;
  /// Set filetime for handle
  void SetFileTime (void *entry, tm &ztime);

  /// Query archive filename
  char *GetName () const
  { return filename; }
  /// Query archive comment
  char *GetComment () const
  { return comment; }
};

inline void csArchive::GetFileTime (void *entry, tm &ztime) const
{
  if (entry)
  {
    UnpackTime (((ArchiveEntry*)entry)->info.last_mod_file_date,
                ((ArchiveEntry*)entry)->info.last_mod_file_time,
                ztime);
  } /* endif */
}

inline void csArchive::SetFileTime (void *entry, tm &ztime)
{
  if (entry)
  {
    PackTime (ztime,
              ((ArchiveEntry*)entry)->info.last_mod_file_date,
              ((ArchiveEntry*)entry)->info.last_mod_file_time);
  } /* endif */
}

#endif // __ARCHIVE_H__
