/*
    ZIP archive support for Crystal Space 3D library
    Copyright (C) 1998 FRIENDS software
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

/**
 * Archive class that can be used to work with standard .ZIP format files.
 * Constructor accepts a file name - if such a file is not found, it is
 * created. After this you can examine archive directory, read files,
 * delete or write files in archive.<p>
 * Operation which changes archive file will be postponed until write_archive()
 * will be called. Before calling write_archive() you can do any number of
 * deletions and writes, but read operations will not be affected by
 * these until write_archive() will be called.<p>
 *
 * Two access modes are allowed:<p>
 *
 * 1) Read/write mode. This is selected by setting "readonly" parameter
 * at object creation to false (this is default if you don't explicitly
 * set it). In this mode any deletions/additions will be made to the
 * original archive file.<p>
 *
 * 2) Readonly mode. This is selected by setting "readonly" constructor
 * parameter to true and possibly defining a base path on a writeable media
 * which will be used for reflecting the changes to the archive.<p>
 * When you make any changes to the archive, on call to write_archive ()
 * a subdirectory is created in directory you specified at object construction
 * time with same name as archive but without extension. In this directory
 * a file is created called $(ZIP_HEADER_FILENAME), which contains the updated
 * ZIP archive directory. All files that were added to the ZIP file will be
 * also written into that directory, or in even deeper subdirectories if
 * they were added into archive with subdirectories. For example, if you
 * will open archive TEST.ZIP in readonly mode and will add file abcde.doc
 * and text/abcde.txt, at write_archive () time they will be compressed and
 * written to disk as TEST/abcde.doc and TEST/text/abcde.txt.<p>
 * If you want to make some additions/changes to the archive without changing
 * the archive itself you can do any of the following:<p>
 *
 * a) Create a directory called <archivename> and put all replacement files
 * in uncompressed form there.<p>
 *
 * b) Create a directory called <archivename> and put all replacement files
 * in compressed form there. Files should have absolutely the same name as
 * original, i.e. for example if you want to replace file texture.gif in a
 * readonly archive with a newer version, you should pack texture.gif into
 * an ZIP archive, then rename that archive into texture.gif.<p>
 *
 * c) Create a directory called <archivename> and put a number of arbitrary
 * named ZIP files into that directory. In this case at object creation time
 * (but obly in readonly mode) the object will scan replacement directory
 * for files and overlay directories of any archives that will be found there
 * onto the directory of main archive file. These archives can contain multiple
 * files, not neccessarily only one file. Keep in mind, however, that such an
 * compound archive should be used in readonly mode. Any writes to such an
 * archive will invalidate replacement files.<p>
 *
 * Known quirks:<p>
 *
 * No CRC check is done on reading, although ZIP file format allows it.
 * This design 'flaw' was allowed to achieve maximal speed. However, when
 * a file is added to archive, its CRC is computed and updated correctly.<p>
 *
 * Several methods of the Archive class requires approximatively 20K of
 * stack space when invoked.<p>
 */
class Archive
{
public:
  static char hdr_central[4];
  static char hdr_local[4];
  static char hdr_endcentral[4];
  static char hdr_extlocal[4];

private:
  /// Archive entry class
  class ArchiveEntry
  {
  public:
    char *filename;
    char *archive;
    ZIP_central_directory_file_header info;
    ArchiveEntry *next;
    char *buffer;
    size_t buffer_pos;
    void *extrafield, *comment;

    ArchiveEntry (const char *name,
      ZIP_central_directory_file_header &cdfh, const char *source);
    ~ArchiveEntry ();
    bool append (const void *data, size_t size);
    bool write_lfh (FILE *file);
    bool write_cdfh (FILE *file);
    bool read_extra_field (FILE *file, size_t extra_field_length);
    bool read_file_comment (FILE *file, size_t file_comment_length);
    bool write_file (FILE *file);
    void free_buffer ();
  };
  friend class ArchiveEntry;

  ArchiveEntry *first;           // Archive directory: chain head
  ArchiveEntry *lazyops;         // Chain head of lazy operations

  char *filename;                // Archive file name
  FILE *file;		         // Archive file pointer.

  UInt comment_length;           // Archive comment length
  char *comment;                 // Archive comment
  bool readonly;                 // Archive opened in readonly mode
  char *workdir;                 // The path for alternative storage

  void read_directory ();
  bool is_deleted (const char *name) const;
  void unpack_time (ush zdate, ush ztime, tm &rtime) const;
  void pack_time (tm &ztime, ush &rdate, ush &rtime) const;
  bool read_archive_comment (FILE *file, size_t zipfile_comment_length);
  void load_ecdr (ZIP_end_central_dir_record &ecdr, char *buff);
  bool read_cdfh (ZIP_central_directory_file_header &cdfh, FILE *file);
  bool read_lfh (ZIP_local_file_header &lfh, FILE *file);
  bool write_ecdr (ZIP_end_central_dir_record &ecdr, FILE *file);
  bool write_zip_archive ();
  bool write_workdir_archive ();
  bool write_central_directory (FILE *temp);
  void update_directory ();
  void read_zip_directory (FILE *infile);
  void read_disk_directory (const char *path);
  ArchiveEntry *insert_entry (const char *name, ZIP_central_directory_file_header &cdfh, const char *source);
  void read_zip_entries (FILE *infile, const char *source);
  char *read_entry (FILE *infile, ArchiveEntry *f);

public:
  /// Open the archive.
  Archive (const char *filename, bool read_only = false, const char *base_path = "");
  /// Close the archive.
  ~Archive ();

  /// Show a directory listing of the archive.
  void dir () const;

  /**
   * Create a new file in the archive. If the file already exists
   * it will be overwritten. Calling new_file twice with same filename
   * without calling write_archive() between will cause unpredictable results.
   *
   * Returns NULL if not succesful. Otherwise it returns a pointer
   * that can be given to 'write' and 'append'. You won't see any changes
   * to archive until 'write_archive' will be called.
   */
  void *new_file (const char *name, size_t size, bool pack = true);

  /**
   * Delete a file from the archive. You won't see any changes
   * to archive until 'write_archive' will be called.
   */
  void delete_file (const char *name);

  /**
   * Return true if a file exists. Also return the
   * size of the file if needed.
   */
  bool file_exists (const char *name, size_t *size = NULL) const;

  /**
   * Read a file completely. After finishing with the returned
   * data you need to 'delete[]' it. If the file does not exists
   * this function returns NULL. If "size" is not null, it is set
   * to unpacked size of the file.
   */
  char *read (const char *name, size_t *size = NULL);

  /**
   * Write data to a file. Note that 'size' need not be
   * the total maximum size if this was given in 'new_file'.
   */
  bool write (void *entry, const char *data, size_t size);

  /**
   * Append data to a file. This function will fail if the
   * total size exceeds the maximum size that was given in
   * 'new_file'.
   */
  bool append (void *entry, const char *data, size_t size);

  /**
   * Execute all pending operations involving writes to archive
   * Neither delete_file or new_file will have effect until this
   * function will be called. Returns false if operation failed.
   * If operation failed, postponed operations remains in the
   * same state as before calling write_archive(), i.e. for example
   * user can be prompted to free some space on drive then retry
   * write_archive().
   */
  bool write_archive ();

  /// Iterator.
  void *first_file () const
  { return (void*)first; }
  void *next_file (void *entry) const
  { return (void*)(((ArchiveEntry*)entry)->next); }

  /// Find a file in archive; returns a handle or NULL
  void *find_name (const char *name) const;
  /// Query name from handle
  char *get_file_name (void *entry) const
  { return ((ArchiveEntry*)entry)->filename; }
  /// Query file size from handle
  size_t get_file_size (void *entry) const
  { return ((ArchiveEntry*)entry)->info.ucsize; }
  /// Query filetime from handle
  void get_file_time (void *entry, tm &ztime) const;
  /// Set filetime for handle
  void set_file_time (void *entry, tm &ztime);

  /// Query archive filename
  char *GetFilename () const
  { return filename; }
  /// Query archive comment
  char *GetComment () const
  { return comment; }
};

inline void Archive::get_file_time (void *entry, tm &ztime) const
{
  if (entry)
  {
    unpack_time (((ArchiveEntry*)entry)->info.last_mod_file_date,
                 ((ArchiveEntry*)entry)->info.last_mod_file_time,
                 ztime);
  } /* endif */
}

inline void Archive::set_file_time (void *entry, tm &ztime)
{
  if (entry)
  {
    pack_time (ztime,
               ((ArchiveEntry*)entry)->info.last_mod_file_date,
               ((ArchiveEntry*)entry)->info.last_mod_file_time);
  } /* endif */
}

#endif // __ARCHIVE_H__
