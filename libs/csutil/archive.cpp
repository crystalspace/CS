/*
    ZIP archive support for Crystal Space 3D library
    Copyright (C) 1998 FRIENDS software
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

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SYSDEF_PATH
#define SYSDEF_TEMP
#define SYSDEF_MKDIR
#define SYSDEF_DIR
#define SYSDEF_UNLINK
#define SYSDEF_ACCESS
#include "sysdef.h"
#include "cssys/common/csendian.h"
#include "csutil/archive.h"

// Name for ZIP central directory header file (for readonly archives)
#ifndef ZIP_HEADER_FILENAME
#  define ZIP_HEADER_FILENAME "-dir-.zip"
#endif

// Default compression method to use when adding entries (there is no choice for now)
#ifndef DEFAULT_COMPRESSION_METHOD
#  define DEFAULT_COMPRESSION_METHOD ZIP_DEFLATE
#endif

// Default compression level: 1..9 (maxspeed..minsize)
#ifndef DEFAULT_COMPRESSION_LEVEL
#  define DEFAULT_COMPRESSION_LEVEL Z_DEFAULT_COMPRESSION
#endif

// This value is used as compression_method to indicate deleted file
#define FILE_DELETED 0xffff
#define FILE_ONDISK  0xfffe

//---------------------------------------------------------------------------

char Archive::hdr_central[4] = {'P', 'K', CENTRAL_HDR_SIG};
char Archive::hdr_local[4] = {'P', 'K', LOCAL_HDR_SIG};
char Archive::hdr_endcentral[4] = {'P', 'K', END_CENTRAL_SIG};
char Archive::hdr_extlocal[4] = {'P', 'K', EXTD_LOCAL_SIG};

//-- Endianess handling -----------------------------------------------------

#define BUFF_GET_SHORT(ofs)     get_le_short ((UByte *)&buff[ofs])
#define BUFF_GET_LONG(ofs)      get_le_long  ((UByte *)&buff[ofs])
#define BUFF_SET_SHORT(ofs,val) set_le_short ((UByte *)&buff[ofs], val)
#define BUFF_SET_LONG(ofs,val)  set_le_long  ((UByte *)&buff[ofs], val)

//-- Archive class implementation -------------------------------------------

Archive::Archive (const char *filename, bool read_only, const char *base_path)
{
  first = NULL;
  lazyops = NULL;
  comment = NULL;
  comment_length = 0;
  readonly = read_only;
  CHK (Archive::filename = new char[strlen (filename) + 1]);
  strcpy (Archive::filename, filename);

  if (readonly)
  {
    // Transform basepath into workdir
    char wd[MAXPATHLEN];
    int wd_len = strlen (base_path);
    int tmp;

    strcpy (wd, base_path);
    APPEND_SLASH (wd, wd_len);

    // Append archive name to workdir and remove extension, if any
    strcat (wd, filename);
    tmp = strlen (wd);
    while (--tmp > wd_len)
    {
      if (wd[tmp] == '.')
      {
        wd[tmp] = 0;
        break;
      }
    } /* endwhile */

    wd_len = strlen (wd);
    APPEND_SLASH (wd, wd_len);

    // And finally, copy this into workdir variable
    CHK (workdir = new char[wd_len + 1]);
    strcpy (workdir, wd);
  } else
    workdir = NULL;

  file = fopen (filename, "rb");
  if (!file && !readonly)       /* Create new archive file */
    file = fopen (filename, "wb");
  else
    read_directory ();
}

Archive::~Archive ()
{
  ArchiveEntry *cur = first;

  while (cur)
  {
    ArchiveEntry *prev = cur;

    cur = cur->next;
    CHK (delete prev);
  } /* endwhile */
  if (filename)
    CHKB (delete [] filename);
  if (comment)
    CHKB (delete [] comment);
  if (file)
    fclose (file);
  if (workdir)
    CHKB (delete [] workdir);
}

void Archive::read_directory ()
{
  if (first)
    return;                     /* Directory already read */

  if (workdir)
  {
    FILE *temp;
    char fname[MAXPATHLEN];

    strcpy (fname, workdir);
    strcat (fname, ZIP_HEADER_FILENAME);
    temp = fopen (fname, "rb");
    if (temp)
    {
      read_zip_directory (temp);
      fclose (temp);
    } else
    {
      read_zip_directory (file);
      read_disk_directory (workdir);
    }
  } else
    read_zip_directory (file);
}

void Archive::read_zip_directory (FILE *infile)
{
  ZIP_end_central_dir_record ecdr;
  ZIP_central_directory_file_header cdfh;
  char buff[1024];              /* Read ZIPfile from end in 1K chunks */
  size_t cur_offs, min_offs, central_directory_offset;
  size_t step = ZIP_END_CENTRAL_DIR_RECORD_SIZE + sizeof (hdr_endcentral);

  if (!infile)
    return;                     /* File not open */
  if (fseek (infile, 0, SEEK_END))
    return;                     /* Seek error */
  if ((long) (cur_offs = ftell (infile)) == -1)
    return;                     /* Can't get file position */

  if (cur_offs >= (65535 + ZIP_END_CENTRAL_DIR_RECORD_SIZE + sizeof (hdr_endcentral)))
    min_offs = cur_offs - (65535 + ZIP_END_CENTRAL_DIR_RECORD_SIZE + sizeof (hdr_endcentral));
  else
    min_offs = 0;

  /* Try to find ZIPfile central directory structure */
  /* For this we have to search from end of file the signature "PK" */
  /* after which follows a two-byte END_CENTRAL_SIG */
  while (cur_offs > min_offs)
  {
    UInt search_pos;

    if (cur_offs >= sizeof (buff) - step)
      cur_offs -= sizeof (buff) - step;
    else
      cur_offs = 0;

    fseek (infile, cur_offs, SEEK_SET);
    search_pos = fread (buff, 1, sizeof (buff), infile);
    if (search_pos >= step)
    {
      register char *search_ptr;

      for (search_ptr = &buff[search_pos - step]; search_ptr > buff; search_ptr--)
        if ((*search_ptr == 'P') &&
            (memcmp (search_ptr, hdr_endcentral, sizeof (hdr_endcentral)) == 0))
        {
          /* Central directory structure found */
          central_directory_offset = cur_offs + (ULong) search_ptr - (ULong)buff;
          load_ecdr (ecdr, &search_ptr[sizeof (hdr_endcentral)]);
          if (fseek (infile, central_directory_offset + sizeof (hdr_endcentral) + ZIP_END_CENTRAL_DIR_RECORD_SIZE, SEEK_SET)
              || !read_archive_comment (infile, ecdr.zipfile_comment_length)
              || fseek (infile, ecdr.offset_start_central_directory, SEEK_SET))
            goto rebuild_cdr;   /* Broken central directory */

          /* Now read central directory structure */
          for (;;)
          {
            if ((fread (buff, 1, sizeof (hdr_central), infile) < sizeof (hdr_central))
                || (memcmp (buff, hdr_central, sizeof (hdr_central)) != 0))
            {
              if (first)
                return;         /* Finished reading central directory */
              else
                goto rebuild_cdr;       /* Broken central directory */
            }
            if ((!read_cdfh (cdfh, infile))
                || (cdfh.filename_length > sizeof (buff))
                || (fread (buff, 1, cdfh.filename_length, infile) < cdfh.filename_length))
              return;           /* Broken zipfile? */
            buff[cdfh.filename_length] = 0;

            if ((buff[cdfh.filename_length - 1] != '/')
                && (buff[cdfh.filename_length - 1] != PATH_SEPARATOR))
            {
              ArchiveEntry *curentry;

              if (infile == file)
                curentry = insert_entry (buff, cdfh, NULL);
              else
              {
                char fname[MAXPATHLEN];

                strcpy (fname, workdir);
                strcat (fname, buff);
                FILE *temp = fopen (fname, "rb");

                if (temp)
                {
                  read_zip_entries (temp, fname);
                  fclose (temp);
              skipfile:
                  if (fseek (infile, cdfh.extra_field_length + cdfh.file_comment_length, SEEK_CUR))
                    return;     /* Broken zipfile? */
                  continue;
                } else if (cdfh.compression_method != FILE_ONDISK)
                  curentry = insert_entry (buff, cdfh, NULL);
                else
                  goto skipfile;
              } /* endif */
              if (!curentry->read_extra_field (infile, cdfh.extra_field_length)
                  || !curentry->read_file_comment (infile, cdfh.file_comment_length))
                return;         /* Broken zipfile? */
            } else
            {
              if (fseek (infile, cdfh.extra_field_length + cdfh.file_comment_length, SEEK_CUR))
                return;         /* Broken zipfile? */
            } /* endif */
          } /* endfor */
        } /* endif */
    } /* endif */
  } /* endwhile */

rebuild_cdr:
  /* If we are here, we did not succeeded to read central directory */
  /* If so, we have to rebuild it by reading each ZIPfile member separately */
  if (fseek (infile, 0, SEEK_SET))
    return;
  read_zip_entries (infile, NULL);
}

void Archive::read_disk_directory (const char *path)
{
  char dirname[MAXPATHLEN];
  size_t pathlen = strlen (path);
  size_t basepathlen = strlen (workdir);

  if ((workdir[basepathlen - 1] != '/')
      && (workdir[basepathlen - 1] != PATH_SEPARATOR))
    basepathlen++;
  strcpy (dirname, path);
  if ((dirname[pathlen - 1] == '/')
      || (dirname[pathlen - 1] == PATH_SEPARATOR))
    dirname[--pathlen] = 0;

  DIR *dh;
  struct dirent *de;

  if ((dh = opendir (dirname)) == NULL)
    return;
  while ((de = readdir (dh)) != NULL)
  {
    if ((strcmp (de->d_name, ".") == 0)
     || (strcmp (de->d_name, "..") == 0))
      continue;
    size_t tmplen = pathlen;

    APPEND_SLASH (dirname, tmplen);
    strcat (dirname, de->d_name);

    if (isdir (path, de))
      read_disk_directory (dirname);
    else
    {
      FILE *temp = fopen (dirname, "rb");

      if (temp)
      {
        read_zip_entries (temp, dirname);
        fclose (temp);
      } /* endif */
    } /* endif */
    dirname[pathlen] = 0;
  } /* endwhile */
  closedir (dh);
}

void Archive::read_zip_entries (FILE *infile, const char *source)
{
  size_t cur_offs, new_offs;
  char buff[1024];
  ZIP_central_directory_file_header cdfh;
  ZIP_local_file_header lfh;

  cur_offs = 0;
  while ((fread (buff, 1, sizeof (hdr_local), infile) >= sizeof (hdr_local))
         && (memcmp (buff, hdr_local, sizeof (hdr_local)) == 0)
         && (read_lfh (lfh, infile)))
  {
    new_offs = cur_offs + sizeof (hdr_local) + ZIP_LOCAL_FILE_HEADER_SIZE +
      lfh.filename_length + lfh.extra_field_length + lfh.csize;
    if ((lfh.filename_length > sizeof (buff))
        || (fread (buff, 1, lfh.filename_length, infile) < lfh.filename_length))
      return;                   /* Broken zipfile? */
    buff[lfh.filename_length] = 0;

    if ((buff[lfh.filename_length - 1] != '/')
        && (buff[lfh.filename_length - 1] != PATH_SEPARATOR))
    {
      /* Partialy convert lfh to cdfh */
      memset (&cdfh, 0, sizeof (cdfh));
      cdfh.version_needed_to_extract[0] = lfh.version_needed_to_extract[0];
      cdfh.version_needed_to_extract[1] = lfh.version_needed_to_extract[1];
      cdfh.general_purpose_bit_flag = lfh.general_purpose_bit_flag;
      cdfh.compression_method = lfh.compression_method;
      cdfh.last_mod_file_time = lfh.last_mod_file_time;
      cdfh.last_mod_file_date = lfh.last_mod_file_date;
      cdfh.crc32 = lfh.crc32;
      cdfh.csize = lfh.csize;
      cdfh.ucsize = lfh.ucsize;
      cdfh.relative_offset_local_header = cur_offs;

      ArchiveEntry *curentry = insert_entry (buff, cdfh, source);

      if (!curentry->read_extra_field (infile, lfh.extra_field_length))
        return;                 /* Broken zipfile */
    } /* endif */
    if (fseek (infile, cur_offs = new_offs, SEEK_SET))
      return;                   /* Broken zipfile */
  } /* endwhile */

  if ((infile != file) && (cur_offs == 0))      /* No headers found */
  {
    /* Treat this file as a plain uncompressed file */
    struct stat st;

    memset (&cdfh, 0, sizeof (cdfh));
    cdfh.version_made_by[0] = 0x16;     /* Zip version 2.2 rev 6 ??? */
    cdfh.version_made_by[1] = 0x06;
    cdfh.version_needed_to_extract[0] = 20;     /* Unzip version 2.0 rev 0 */
    cdfh.version_needed_to_extract[1] = 00;
    cdfh.compression_method = FILE_ONDISK;

    if (fstat (fileno (infile), &st) == 0)
    {
      time_t const mtime = st.st_mtime;
      struct tm *curtm = localtime (&mtime);

      cdfh.csize = st.st_size;
      cdfh.ucsize = st.st_size;
      cdfh.relative_offset_local_header = 0;
      strcpy (buff, (char *) &source[strlen (workdir)]);
      set_file_time (insert_entry (buff, cdfh, source), *curtm);
    }
  } /* endif */
}

Archive::ArchiveEntry *Archive::insert_entry (const char *name,
  ZIP_central_directory_file_header &cdfh, const char *source)
{
  ArchiveEntry *cur, *prev;

  cur = first;
  prev = NULL;
  while (cur)
  {
    if (strcmp (cur->filename, name) == 0)
    {
      if (prev)
      {
        CHK (prev->next = new ArchiveEntry (name, cdfh, source));
        prev->next->next = cur->next;
        prev = prev->next;
      } else
      {
        CHK (first = new ArchiveEntry (name, cdfh, source));
        first->next = cur->next;
        prev = first;
      }
      CHK (delete cur);
      return prev;
    }
    prev = cur;
    cur = cur->next;
  } /* endwhile */

  if (first)
  {
    cur = first;
    while (cur->next)
      cur = cur->next;
    CHK (cur->next = new ArchiveEntry (name, cdfh, source));
    return (cur->next);
  } else
  {
    CHK (first = new ArchiveEntry (name, cdfh, source));
    return first;
  } /* endif */
}

bool Archive::read_archive_comment (FILE *infile, size_t zipfile_comment_length)
{
  if (comment && (comment_length != zipfile_comment_length))
  {
    CHK (delete [] comment);
    comment = NULL;
  }
  comment_length = zipfile_comment_length;
  if (!comment)
    CHKB (comment = new char [zipfile_comment_length]);
  return (fread (comment, 1, zipfile_comment_length, infile) == zipfile_comment_length);
}

void Archive::dir () const
{
  ArchiveEntry *f = first;

  printf (" Comp |Uncomp| File |CheckSum| File\n");
  printf (" size | size |offset| (CRC32)| name\n");
  printf ("------+------+------+--------+------\n");
  while (f)
  {
    printf ("%6ld|%6ld|%6ld|%08x|%s\n", f->info.csize, f->info.ucsize,
            f->info.relative_offset_local_header, (UInt) f->info.crc32, f->filename);
    f = f->next;
  }
}

void *Archive::find_name (const char *name) const
{
  ArchiveEntry *f = first;

  while (f)
  {
    if (!strcmp (f->filename, name))
      return (void *) f;
    f = f->next;
  }
  return NULL;
}

char *Archive::read (const char *name, size_t * size)
{
  ArchiveEntry *f = (ArchiveEntry *) find_name (name);

  if (!f)
    return NULL;
  if (size)
    *size = f->info.ucsize;

  if (f->archive)
  {
    FILE *temp = fopen (f->archive, "rb");

    if (temp)
    {
      char *data = read_entry (temp, f);

      fclose (temp);
      return data;
    } else
      return NULL;
  }
  else
    return read_entry (file, f);
}

char *Archive::read_entry (FILE *infile, ArchiveEntry * f)
{
  // This routine allocates one byte more than is actually needed
  // and fills it with zero. This can be used when reading text files

  size_t bytes_left;
  char buff[1024];
  char *out_buff;
  int err;
  ZIP_local_file_header lfh;

  CHK (out_buff = new char[f->info.ucsize + 1]);
  if (!out_buff)
    return NULL;
  out_buff [f->info.ucsize] = 0;

  if (f->info.compression_method == FILE_ONDISK)
  {
    if (fread (out_buff, 1, f->info.ucsize, infile) < f->info.ucsize)
    {
      CHK (delete[] out_buff);
      return NULL;
    } /* endif */
    return out_buff;
  }
  if ((fseek (infile, f->info.relative_offset_local_header, SEEK_SET))
      || (fread (buff, 1, sizeof (hdr_local), infile) < sizeof (hdr_local))
      || (memcmp (buff, hdr_local, sizeof (hdr_local)) != 0)
      || (!read_lfh (lfh, infile))
      || (fseek (infile, lfh.filename_length + lfh.extra_field_length, SEEK_CUR)))
  {
    CHK (delete[] out_buff);
    return NULL;
  }
  switch (f->info.compression_method)
  {
    case ZIP_STORE:
      {
        if (fread (out_buff, 1, f->info.csize, infile) < f->info.csize)
        {
          CHK (delete[] out_buff);
          return NULL;
        } /* endif */
        break;
      }
    case ZIP_DEFLATE:
      {
        z_stream zs;

        bytes_left = f->info.csize;
        zs.next_out = (z_Byte *) out_buff;
        zs.avail_out = f->info.ucsize;
        zs.zalloc = (alloc_func) 0;
        zs.zfree = (free_func) 0;

        /* Undocumented: if wbits is negative, zlib skips header check */
        err = inflateInit2 (&zs, -DEF_WBITS);
        if (err != Z_OK)
        {
          CHK (delete[] out_buff);
          return NULL;
        }
        while (bytes_left)
        {
          size_t size;

          zs.next_in = (z_Byte *)buff;
          if (bytes_left > sizeof (buff))
            size = sizeof (buff);
          else
            size = bytes_left;
          zs.avail_in = fread (buff, 1, size, infile);

          err = inflate (&zs, bytes_left > size ? Z_PARTIAL_FLUSH : Z_FINISH);
          bytes_left -= size;
        } /* endwhile */
        inflateEnd (&zs);

        /* discard dynamically allocated buffers on error */
        // Kludge warning: I've encountered a file where zlib 1.1.1 returned
        // Z_BUF_ERROR although everything was ok (a slightly compressed PNG file)
        if ((err != Z_STREAM_END)
            && ((err != Z_BUF_ERROR) || bytes_left || zs.avail_out))
        {
          //CHK (delete[] out_buff);
          //return NULL;
        }
        break;
      }
    default:
      {
        /* Can't handle this compression algorythm */
        CHK (delete[] out_buff);
        return NULL;
      }
  } /* endswitch */
  return out_buff;
}

void *Archive::new_file (const char *name, size_t size, bool pack)
{
  delete_file (name);

  ZIP_central_directory_file_header cdfh;

  memset (&cdfh, 0, sizeof (cdfh));
  cdfh.version_made_by[0] = 0x16;       /* Zip version 2.2 rev 6 ??? */
  cdfh.version_made_by[1] = 0x06;
  cdfh.version_needed_to_extract[0] = 20;       /* Unzip version 2.0 rev 0 */
  cdfh.version_needed_to_extract[1] = 00;
  if (pack)
    cdfh.compression_method = DEFAULT_COMPRESSION_METHOD;
  else
    cdfh.compression_method = ZIP_STORE;
  cdfh.ucsize = size;

  ArchiveEntry *f;

  if (readonly)
  {
    char source[MAXPATHLEN];

    strcpy (source, workdir);
    strcat (source, name);
    CHK (f = new ArchiveEntry (name, cdfh, source));
  } else
    CHKB (f = new ArchiveEntry (name, cdfh, NULL));

  time_t curtime = time (NULL);
  struct tm *curtm = localtime (&curtime);

  set_file_time ((void *) f, *curtm);

  if (lazyops)
  {
    ArchiveEntry *last = lazyops;

    while (last->next)
      last = last->next;
    last->next = f;
  } else
    lazyops = f;

  return (void *) f;
}

void Archive::delete_file (const char *name)
{
  if (!file_exists (name))
    return;

  ZIP_central_directory_file_header cdfh;

  memset (&cdfh, 0, sizeof (cdfh));
  cdfh.compression_method = FILE_DELETED;
  CHK (ArchiveEntry * f = new ArchiveEntry (name, cdfh, NULL));

  if (lazyops)
  {
    ArchiveEntry *last = lazyops;

    while (last->next)
      last = last->next;
    last->next = f;
  } else
    lazyops = f;
}

bool Archive::file_exists (const char *name, size_t * size) const
{
  ArchiveEntry *f = (ArchiveEntry *) find_name (name);

  if (!f)
    return false;
  if (size)
    *size = f->info.ucsize;
  return true;
}

bool Archive::write (void *entry, const char *data, size_t len)
{
  if (entry)
  {
    ((ArchiveEntry *) entry)->buffer_pos = 0;
    return (((ArchiveEntry *) entry)->append (data, len));
  } else
    return false;
}

bool Archive::append (void *entry, const char *data, size_t len)
{
  if (entry)
    return (((ArchiveEntry *) entry)->append (data, len));
  else
    return false;
}

// Flush all pending operations (if any)
bool Archive::write_archive ()
{
  if (!lazyops)
    return true;                /* Nothing to do */
  if (readonly)
    return write_workdir_archive ();
  else
    return write_zip_archive ();
}

// Write pending operations into ZIP archive
bool Archive::write_zip_archive ()
{
  char temp_file[MAXPATHLEN];
  FILE *temp;
  char buff[1024];
  bool success = false;

  // Step one: Copy archive file into a temporary file,
  // skipping entries marked as 'deleted'
  strcpy (temp_file, TEMP_DIR);
  int tmplen = strlen (temp_file);

  APPEND_SLASH (temp_file, tmplen);
  sprintf (&temp_file[tmplen], TEMP_FILE);
  if ((temp = fopen (temp_file, "w+b")) == NULL)
    return false;               /* Cannot create temporary file */
  fseek (file, 0, SEEK_SET);

  while (fread (buff, 1, sizeof (hdr_local), file) == sizeof (hdr_local))
  {
    size_t bytes_to_copy, bytes_to_skip;
    ArchiveEntry *this_file = NULL;

    if (memcmp (buff, hdr_local, sizeof (hdr_local)) == 0)
    {                           //----------------------------------local header
      ZIP_local_file_header lfh;

      if (!read_lfh (lfh, file))
        goto temp_failed;

      CHK (char *this_name = new char[lfh.filename_length + 1]);
      if (fread (this_name, 1, lfh.filename_length, file) < lfh.filename_length)
      {
        CHK (delete[] this_name);
        goto temp_failed;
      }
      this_name[lfh.filename_length] = 0;

      if (is_deleted (this_name))
      {
    skip_file:
        bytes_to_skip = lfh.extra_field_length + lfh.csize;
        bytes_to_copy = 0;
        CHK (delete[] this_name);
      } else
      {
        this_file = (ArchiveEntry *) find_name (this_name);
        if (this_file)
        {
          CHK (delete[] this_name);
          if (this_file->info.csize != lfh.csize)
            goto temp_failed;   /* Broken archive */
          this_file->read_extra_field (file, lfh.extra_field_length);
          bytes_to_skip = 0;
          bytes_to_copy = lfh.csize;
          if (!this_file->write_lfh (temp))
            goto temp_failed;   /* Write error */
        } else
          goto skip_file;       /* hmm... strange. */
      }
    } else if (memcmp (buff, hdr_central, sizeof (hdr_central)) == 0)
    {                           //----------------------------------central directory header
      ZIP_central_directory_file_header cdfh;

      if (!read_cdfh (cdfh, file))
        goto temp_failed;

      bytes_to_copy = 0;
      bytes_to_skip = cdfh.filename_length + cdfh.extra_field_length + cdfh.file_comment_length;
    } else if (memcmp (buff, hdr_endcentral, sizeof (hdr_endcentral)) == 0)
    {                           //----------------------------------end-of-central-directory header
      ZIP_end_central_dir_record ecdr;
      char buff[ZIP_END_CENTRAL_DIR_RECORD_SIZE];

      if (fread (buff, 1, ZIP_END_CENTRAL_DIR_RECORD_SIZE, file) < ZIP_END_CENTRAL_DIR_RECORD_SIZE)
        goto temp_failed;
      load_ecdr (ecdr, buff);

      bytes_to_copy = 0;
      bytes_to_skip = ecdr.zipfile_comment_length;
    } else
    {
      /* Unknown chunk type */
      goto temp_failed;
    } /* endif */

    if (bytes_to_skip)
      fseek (file, bytes_to_skip, SEEK_CUR);
    while (bytes_to_copy)
    {
      size_t size;

      if (bytes_to_copy > sizeof (buff))
        size = sizeof (buff);
      else
        size = bytes_to_copy;

      if ((fread (buff, 1, size, file) < size)
          || (fwrite (buff, 1, size, temp) < size))
        goto temp_failed;
      bytes_to_copy -= size;
    }
  } /* endwhile */

  /* Now we have to append all files that were added to archive */
  {
    ArchiveEntry *cur = lazyops;

    while (cur)
    {
      if ((cur->info.compression_method != FILE_DELETED)
          && !cur->write_file (temp))
        goto temp_failed;       /* Write error */
      cur = cur->next;
    } /* endwhile */
  }

  /* And finaly write central directory structure */
  if (!write_central_directory (temp))
    goto temp_failed;

  /* Now copy temporary file into archive. If we'll get a error in process, */
  /* we're lost! I don't know for a good solution for this without wasting */
  /* disk space for yet another copy of the archive :-( */
  {
    fseek (temp, 0, SEEK_END);
    size_t fsize = ftell (temp);

    fseek (temp, 0, SEEK_SET);
    fclose (file);

    if ((file = fopen (filename, "wb")) == NULL)
    {
      file = fopen (filename, "rb");
      goto temp_failed;
    }
    while (fsize)
    {
      char buff[16384];
      size_t bytes_read = fread (buff, 1, sizeof (buff), temp);

      if (fwrite (buff, 1, bytes_read, file) < bytes_read)
      {                         /* Yuck! Keep at least temporary file */
        fclose (temp);
        fclose (file);
        file = fopen (filename, "rb");
        return false;
      }
      fsize -= bytes_read;
    }
    /* Hurray! We're done */
    fclose (file);
    file = fopen (filename, "rb");
  }

  /* Now if we are here, all operations have been successful */
  update_directory ();

  success = true;

temp_failed:
  fclose (temp);
  unlink (temp_file);
  return success;
}

// Write pending operations to disk into workdir
bool Archive::write_workdir_archive ()
{
  // Create the work directory if it doesn't exist
  {
    char fname[MAXPATHLEN];
    size_t wd_len = strlen (workdir);

    strcpy (fname, workdir);
    if ((fname[wd_len - 1] == '/') || (fname[wd_len - 1] == PATH_SEPARATOR))
      fname[wd_len - 1] = 0;
    MKDIR (fname);
  }

  /* Write all files that were added to archive */
  {
    ArchiveEntry *cur = lazyops;

    while (cur)
    {
      size_t i = 0, wd_len = strlen (workdir), fnlen = strlen (cur->filename);
      char fname[MAXPATHLEN];

      strcpy (fname, workdir);
      while (i < fnlen)
      {
        fname[wd_len++] = cur->filename[i++];

        // If this is a subdirectory in archive, try to reproduce
        // same directory on disk
        if ((cur->filename[i] == '/') || (cur->filename[i] == PATH_SEPARATOR))
        {
          fname[wd_len] = 0;
          MKDIR (fname);
        } /* endif */
      } /* endwhile */
      fname[wd_len] = 0;

      if (cur->info.compression_method != FILE_DELETED)
      {
        FILE *temp = fopen (fname, "wb");

        if (!temp)
          return false;         /* File creation error */
        if (!cur->write_file (temp))
        {
          fclose (temp);
          unlink (fname);
          return false;         /* Write error */
        }
        fclose (temp);
      } else
        unlink (fname);

      cur = cur->next;
    } /* endwhile */
  }

  {
    char fname[MAXPATHLEN];

    strcpy (fname, workdir);
    strcat (fname, ZIP_HEADER_FILENAME);

    FILE *temp = fopen (fname, "wb");

    if (!temp)
      return false;             /* ZIPfile directory creation error */

    if (!write_central_directory (temp))
    {
      fclose (temp);
      unlink (fname);
      return false;
    }
    fclose (temp);
  }

  update_directory ();
  return true;
}

bool Archive::write_central_directory (FILE *temp)
{
  ArchiveEntry *cur = first;
  int count = 0;
  size_t cdroffs = ftell (temp);

  while (cur)
  {
    if (!is_deleted (cur->filename))
    {
      if (!cur->write_cdfh (temp))
        return false;
      count++;
    } /* endif */
    cur = cur->next;
  } /* endwhile */

  cur = lazyops;
  while (cur)
  {
    if (cur->info.compression_method != FILE_DELETED)
      if (cur->write_cdfh (temp))
        count++;
      else
        return false;
    cur = cur->next;
  } /* endwhile */

  /* Write end-of-central-directory record */
  ZIP_end_central_dir_record ecdr;

  memset (&ecdr, 0, sizeof (ecdr));
  ecdr.num_entries_centrl_dir_ths_disk = count;
  ecdr.total_entries_central_dir = count;
  ecdr.size_central_directory = ftell (temp) - cdroffs;
  ecdr.offset_start_central_directory = cdroffs;
  ecdr.zipfile_comment_length = comment_length;
  if (!write_ecdr (ecdr, temp))
    return false;
  return true;
}

void Archive::update_directory ()
{
  /* Update archive directory: remove deleted entries first */
  {
    ArchiveEntry *cur = first, *last = first;
    ArchiveEntry **prev = &first;

    while (cur)
    {
      if (is_deleted (cur->filename))
      {
        *prev = cur->next;
	if (cur == last)
	  last = cur->next;
        CHK (delete cur);
        cur = *prev;
      } else
      {
        prev = &cur->next;
        cur = cur->next;
      } /* endif */
    } /* endwhile */

    /* Now add 'new_file'd entries to archive directory */
    if (last)
      while (last->next)
        last = last->next;
    while (lazyops)
    {
      cur = lazyops->next;
      if (lazyops->info.compression_method != FILE_DELETED)
      {
        lazyops->free_buffer ();
        lazyops->next = NULL;

        if (lazyops->archive)
          CHKB (delete[] lazyops->archive);
        lazyops->archive = NULL;
        if (readonly)
        {
          char fname[MAXPATHLEN];

          strcpy (fname, workdir);
          strcat (fname, lazyops->filename);
          if (access (fname, F_OK) == 0)
          {
            CHK (lazyops->archive = new char[strlen (fname) + 1]);
            strcpy (lazyops->archive, fname);
          } /* endif */
        } /* endif */
        if (last)
          last->next = lazyops;
        last = lazyops;
        if (!first)
          first = lazyops;
      } else
        CHKB (delete lazyops);
      lazyops = cur;
    } /* endwhile */
  }
}

bool Archive::is_deleted (const char *name) const
{
  ArchiveEntry *current = lazyops;

  while (current)
  {
    if ((current->info.compression_method == FILE_DELETED)
        && (strcmp (name, current->filename) == 0))
      return true;
    current = current->next;
  } /* endwhile */
  return false;
}

void Archive::unpack_time (ush zdate, ush ztime, tm & rtime) const
{
  rtime.tm_year = (((zdate >> 9) & 0x7f) + 80);
  rtime.tm_mon = ((zdate >> 5) & 0x0f);
  rtime.tm_mday = (zdate & 0x1f);

  rtime.tm_hour = ((ztime >> 11) & 0x1f);
  rtime.tm_min = ((ztime >> 5) & 0x3f);
  rtime.tm_sec = ((ztime & 0x1f) * 2);
}

void Archive::pack_time (tm & ztime, ush & rdate, ush & rtime) const
{
  rdate = (((ztime.tm_year - 80) & 0x7f) << 9)
        | ((ztime.tm_mon & 0x0f) << 5)
        | (ztime.tm_mday & 0x1f);
  rtime = ((ztime.tm_hour & 0x1f) << 11)
        | ((ztime.tm_min & 0x3f) << 5)
        | ((ztime.tm_sec / 2) & 0x1f);
}

void Archive::load_ecdr (ZIP_end_central_dir_record & ecdr, char *buff)
{
  ecdr.number_this_disk = BUFF_GET_SHORT (E_NUMBER_THIS_DISK);
  ecdr.num_disk_start_cdir = BUFF_GET_SHORT (E_NUM_DISK_WITH_START_CENTRAL_DIR);
  ecdr.num_entries_centrl_dir_ths_disk = BUFF_GET_SHORT (E_NUM_ENTRIES_CENTRL_DIR_THS_DISK);
  ecdr.total_entries_central_dir = BUFF_GET_SHORT (E_TOTAL_ENTRIES_CENTRAL_DIR);
  ecdr.size_central_directory = BUFF_GET_LONG (E_SIZE_CENTRAL_DIRECTORY);
  ecdr.offset_start_central_directory = BUFF_GET_LONG (E_OFFSET_START_CENTRAL_DIRECTORY);
  ecdr.zipfile_comment_length = BUFF_GET_SHORT (E_ZIPFILE_COMMENT_LENGTH);
}

bool Archive::read_cdfh (ZIP_central_directory_file_header & cdfh, FILE *infile)
{
  char buff[ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE];

  if (fread (buff, 1, ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE, infile) < ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE)
    return false;

  cdfh.version_made_by[0] = buff[C_VERSION_MADE_BY_0];
  cdfh.version_made_by[1] = buff[C_VERSION_MADE_BY_1];
  cdfh.version_needed_to_extract[0] = buff[C_VERSION_NEEDED_TO_EXTRACT_0];
  cdfh.version_needed_to_extract[1] = buff[C_VERSION_NEEDED_TO_EXTRACT_1];
  cdfh.general_purpose_bit_flag = BUFF_GET_SHORT (C_GENERAL_PURPOSE_BIT_FLAG);
  cdfh.compression_method = BUFF_GET_SHORT (C_COMPRESSION_METHOD);
  cdfh.last_mod_file_time = BUFF_GET_SHORT (C_LAST_MOD_FILE_TIME);
  cdfh.last_mod_file_date = BUFF_GET_SHORT (C_LAST_MOD_FILE_DATE);
  cdfh.crc32 = BUFF_GET_LONG (C_CRC32);
  cdfh.csize = BUFF_GET_LONG (C_COMPRESSED_SIZE);
  cdfh.ucsize = BUFF_GET_LONG (C_UNCOMPRESSED_SIZE);
  cdfh.filename_length = BUFF_GET_SHORT (C_FILENAME_LENGTH);
  cdfh.extra_field_length = BUFF_GET_SHORT (C_EXTRA_FIELD_LENGTH);
  cdfh.file_comment_length = BUFF_GET_SHORT (C_FILE_COMMENT_LENGTH);
  cdfh.disk_number_start = BUFF_GET_SHORT (C_DISK_NUMBER_START);
  cdfh.internal_file_attributes = BUFF_GET_SHORT (C_INTERNAL_FILE_ATTRIBUTES);
  cdfh.external_file_attributes = BUFF_GET_LONG (C_EXTERNAL_FILE_ATTRIBUTES);
  cdfh.relative_offset_local_header = BUFF_GET_LONG (C_RELATIVE_OFFSET_LOCAL_HEADER);

  return true;
}

bool Archive::read_lfh (ZIP_local_file_header & lfh, FILE *infile)
{
  char buff[ZIP_LOCAL_FILE_HEADER_SIZE];

  if (fread (buff, 1, ZIP_LOCAL_FILE_HEADER_SIZE, infile) < ZIP_LOCAL_FILE_HEADER_SIZE)
    return false;

  lfh.version_needed_to_extract[0] = buff[L_VERSION_NEEDED_TO_EXTRACT_0];
  lfh.version_needed_to_extract[1] = buff[L_VERSION_NEEDED_TO_EXTRACT_1];
  lfh.general_purpose_bit_flag = BUFF_GET_SHORT (L_GENERAL_PURPOSE_BIT_FLAG);
  lfh.compression_method = BUFF_GET_SHORT (L_COMPRESSION_METHOD);
  lfh.last_mod_file_time = BUFF_GET_SHORT (L_LAST_MOD_FILE_TIME);
  lfh.last_mod_file_date = BUFF_GET_SHORT (L_LAST_MOD_FILE_DATE);
  lfh.crc32 = BUFF_GET_LONG (L_CRC32);
  lfh.csize = BUFF_GET_LONG (L_COMPRESSED_SIZE);
  lfh.ucsize = BUFF_GET_LONG (L_UNCOMPRESSED_SIZE);
  lfh.filename_length = BUFF_GET_SHORT (L_FILENAME_LENGTH);
  lfh.extra_field_length = BUFF_GET_SHORT (L_EXTRA_FIELD_LENGTH);

  return true;
}

bool Archive::write_ecdr (ZIP_end_central_dir_record & ecdr, FILE *outfile)
{
  char buff[ZIP_END_CENTRAL_DIR_RECORD_SIZE];

  if (fwrite (hdr_endcentral, 1, sizeof (hdr_endcentral), outfile) != sizeof (hdr_endcentral))
    return false;

  BUFF_SET_SHORT (E_NUMBER_THIS_DISK, ecdr.number_this_disk);
  BUFF_SET_SHORT (E_NUM_DISK_WITH_START_CENTRAL_DIR, ecdr.num_disk_start_cdir);
  BUFF_SET_SHORT (E_NUM_ENTRIES_CENTRL_DIR_THS_DISK, ecdr.num_entries_centrl_dir_ths_disk);
  BUFF_SET_SHORT (E_TOTAL_ENTRIES_CENTRAL_DIR, ecdr.total_entries_central_dir);
  BUFF_SET_LONG (E_SIZE_CENTRAL_DIRECTORY, ecdr.size_central_directory);
  BUFF_SET_LONG (E_OFFSET_START_CENTRAL_DIRECTORY, ecdr.offset_start_central_directory);
  BUFF_SET_SHORT (E_ZIPFILE_COMMENT_LENGTH, ecdr.zipfile_comment_length);

  if ((fwrite (buff, 1, ZIP_END_CENTRAL_DIR_RECORD_SIZE, outfile) != ZIP_END_CENTRAL_DIR_RECORD_SIZE)
      || (fwrite (comment, 1, comment_length, outfile) != comment_length))
    return false;
  return true;
}

Archive::ArchiveEntry::ArchiveEntry (const char *name,
  ZIP_central_directory_file_header &cdfh, const char *source)
{
  CHK (filename = new char[strlen (name) + 1]);
  strcpy (filename, name);
  info = cdfh;
  next = NULL;
  buffer = NULL;
  extrafield = NULL;
  comment = NULL;
  buffer_pos = 0;
  if (source)
  {
    CHK (archive = new char[strlen (source) + 1]);
    strcpy (archive, source);
  } else
    archive = NULL;
}

Archive::ArchiveEntry::~ArchiveEntry ()
{
  free_buffer ();
  if (archive)
    CHKB (delete [] archive);
  if (comment)
    CHKB (delete [] comment);
  if (extrafield)
    CHKB (delete [] extrafield);
  if (filename)
    CHKB (delete [] filename);
}

void Archive::ArchiveEntry::free_buffer ()
{
  if (buffer)
    CHKB (delete[] buffer);
  buffer = NULL;
  buffer_pos = 0;
}

bool Archive::ArchiveEntry::append (const void *data, size_t size)
{
  if ((info.compression_method == FILE_DELETED) /* Non-writeable file */
      || (buffer_pos + size > info.ucsize))     /* Data exceeds buffer */
    return false;
  if (!buffer)
  {
    CHK (buffer = new char[info.ucsize]);
    if (!buffer)
      return false;             /* Not enough memory */
    memset (buffer, 0, info.ucsize);
  } /* endif */
  memcpy (&buffer[buffer_pos], data, size);
  buffer_pos += size;
  return true;
}

bool Archive::ArchiveEntry::write_lfh (FILE *outfile)
{
  char buff[ZIP_LOCAL_FILE_HEADER_SIZE];
  size_t lfhpos = ftell (outfile);

  buff[L_VERSION_NEEDED_TO_EXTRACT_0] = info.version_needed_to_extract[0];
  buff[L_VERSION_NEEDED_TO_EXTRACT_1] = info.version_needed_to_extract[1];
  BUFF_SET_SHORT (L_GENERAL_PURPOSE_BIT_FLAG, info.general_purpose_bit_flag);
  BUFF_SET_SHORT (L_COMPRESSION_METHOD, info.compression_method);
  BUFF_SET_SHORT (L_LAST_MOD_FILE_TIME, info.last_mod_file_time);
  BUFF_SET_SHORT (L_LAST_MOD_FILE_DATE, info.last_mod_file_date);
  BUFF_SET_LONG (L_CRC32, info.crc32);
  BUFF_SET_LONG (L_COMPRESSED_SIZE, info.csize);
  BUFF_SET_LONG (L_UNCOMPRESSED_SIZE, info.ucsize);
  BUFF_SET_SHORT (L_FILENAME_LENGTH, info.filename_length = strlen (filename));
  BUFF_SET_SHORT (L_EXTRA_FIELD_LENGTH,
                  info.extra_field_length = extrafield ? info.extra_field_length : 0);

  if ((fwrite (hdr_local, 1, sizeof (hdr_local), outfile) < sizeof (hdr_local))
      || (fwrite (buff, 1, ZIP_LOCAL_FILE_HEADER_SIZE, outfile) < ZIP_LOCAL_FILE_HEADER_SIZE)
      || (fwrite (filename, 1, info.filename_length, outfile) < info.filename_length)
      || (fwrite (extrafield, 1, info.extra_field_length, outfile) < info.extra_field_length))
    return false;

  info.relative_offset_local_header = lfhpos;
  return true;
}

bool Archive::ArchiveEntry::write_cdfh (FILE *outfile)
{
  char buff[ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE];

  /* Convert endianess if needed */
  buff[C_VERSION_MADE_BY_0] = info.version_made_by[0];
  buff[C_VERSION_MADE_BY_1] = info.version_made_by[1];
  buff[C_VERSION_NEEDED_TO_EXTRACT_0] = info.version_needed_to_extract[0];
  buff[C_VERSION_NEEDED_TO_EXTRACT_1] = info.version_needed_to_extract[1];

  BUFF_SET_SHORT (C_GENERAL_PURPOSE_BIT_FLAG, info.general_purpose_bit_flag);
  BUFF_SET_SHORT (C_COMPRESSION_METHOD, info.compression_method);
  BUFF_SET_SHORT (C_LAST_MOD_FILE_TIME, info.last_mod_file_time);
  BUFF_SET_SHORT (C_LAST_MOD_FILE_DATE, info.last_mod_file_date);
  BUFF_SET_LONG (C_CRC32, info.crc32);
  BUFF_SET_LONG (C_COMPRESSED_SIZE, info.csize);
  BUFF_SET_LONG (C_UNCOMPRESSED_SIZE, info.ucsize);

  BUFF_SET_SHORT (C_FILENAME_LENGTH, info.filename_length = strlen (filename));
  /* We're ignoring extra field for central directory, although InfoZIP puts there a field containing EF_TIME -
     universal timestamp - but for example DOS pkzip/pkunzip does not put nothing there. */
  BUFF_SET_SHORT (C_EXTRA_FIELD_LENGTH, 0);
  BUFF_SET_SHORT (C_FILE_COMMENT_LENGTH,
                  info.file_comment_length = comment ? info.file_comment_length : 0);
  BUFF_SET_SHORT (C_DISK_NUMBER_START, info.disk_number_start);
  BUFF_SET_SHORT (C_INTERNAL_FILE_ATTRIBUTES, info.internal_file_attributes);
  BUFF_SET_LONG (C_EXTERNAL_FILE_ATTRIBUTES, info.external_file_attributes);
  BUFF_SET_LONG (C_RELATIVE_OFFSET_LOCAL_HEADER, info.relative_offset_local_header);

  if ((fwrite (hdr_central, 1, sizeof (hdr_central), outfile) < sizeof (hdr_central))
      || (fwrite (buff, 1, ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE, outfile) < ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE)
      || (fwrite (filename, 1, info.filename_length, outfile) < info.filename_length)
      || (fwrite (comment, 1, info.file_comment_length, outfile) < info.file_comment_length))
    return false;

  return true;
}

bool Archive::ArchiveEntry::read_extra_field (FILE *infile, size_t extra_field_length)
{
  if (extrafield && (info.extra_field_length != extra_field_length))
  {
    CHK (delete[] extrafield);
    extrafield = NULL;
  }
  info.extra_field_length = extra_field_length;
  if (!extrafield)
    CHKB (extrafield = new char[extra_field_length]);
  return (fread (extrafield, 1, extra_field_length, infile) == extra_field_length);
}

bool Archive::ArchiveEntry::read_file_comment (FILE *infile, size_t file_comment_length)
{
  if (comment && (info.file_comment_length != file_comment_length))
  {
    CHK (delete[] comment);
    comment = NULL;
  }
  info.file_comment_length = file_comment_length;
  if (!comment)
    CHKB (comment = new char[file_comment_length]);
  return (fread (comment, 1, file_comment_length, infile) == file_comment_length);
}

bool Archive::ArchiveEntry::write_file (FILE *outfile)
{
  size_t lfhoffs = ftell (outfile);

  info.crc32 = crc32 (CRCVAL_INITIAL, (z_Byte *) buffer, buffer_pos);
  bool finished = false;

  while (!finished)
  {
    if (fseek (outfile, lfhoffs + sizeof (hdr_local) + ZIP_LOCAL_FILE_HEADER_SIZE +
               strlen (filename) + (extrafield ? info.extra_field_length : 0), SEEK_SET))
      return false;

    switch (info.compression_method)
    {
      case ZIP_STORE:
        {
          if (fwrite (buffer, 1, buffer_pos, outfile) < buffer_pos)
            return false;       /* Write error */
          info.csize = info.ucsize = buffer_pos;
          finished = true;
          break;
        }
      case ZIP_DEFLATE:
        {
          z_stream zs;

          zs.zalloc = (alloc_func) 0;
          zs.zfree = (free_func) 0;
          zs.next_in = (z_Byte *) buffer;
          zs.avail_in = buffer_pos;
          if (deflateInit (&zs, DEFAULT_COMPRESSION_LEVEL) != Z_OK)
            return false;
          info.csize = 0;
          info.ucsize = buffer_pos;

          char buff[16384];
          int buffofs = 2;      /* Skip inflated data header */

          while (1)
          {
            zs.next_out = (z_Byte *)buff;
            zs.avail_out = sizeof (buff);

            int rc = deflate (&zs, Z_FINISH);   /* Do actual compression */
            size_t size = sizeof (buff) - zs.avail_out - buffofs;

            info.csize += size;

            if (fwrite (&buff[buffofs], 1, size, outfile) != size)
            {
              deflateEnd (&zs);
              return false;
            }
            if (rc == Z_STREAM_END)
              break;            /* finished */
            buffofs = 0;
          } /* endwhile */
          deflateEnd (&zs);
          if (info.csize < info.ucsize)
            finished = true;
          else
            info.compression_method = ZIP_STORE;
          break;
        }
      default:
        return false;
    } /* endswitch */
  } /* endwhile */

  fseek (outfile, lfhoffs, SEEK_SET);
  if (!write_lfh (outfile))
    return false;
  fseek (outfile, info.csize, SEEK_CUR);
  return true;
}
