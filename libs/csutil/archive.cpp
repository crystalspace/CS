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
#include "cssysdef.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "csutil/archive.h"
#include "csutil/csendian.h"
#include "csutil/csstring.h"
#include "csutil/physfile.h"
#include "csutil/set.h"
#include "csutil/snprintf.h"
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/util.h"
#include "iutil/vfs.h"	// For csFileTime

// Default compression method to use when adding entries (there is no choice for now)
#ifndef DEFAULT_COMPRESSION_METHOD
#  define DEFAULT_COMPRESSION_METHOD ZIP_DEFLATE
#endif

// Default compression level: 1..9 (maxspeed..minsize)
#ifndef DEFAULT_COMPRESSION_LEVEL
#  define DEFAULT_COMPRESSION_LEVEL Z_DEFAULT_COMPRESSION
#endif

// helper macro: appends a slash to the string if no slash is present
#  define APPEND_SLASH(str,len)                 \
     if ((len)                                  \
      && (str[len - 1] != '/')                  \
      && (str[len - 1] != CS_PATH_SEPARATOR))   \
     {                                          \
       str[len++] = CS_PATH_SEPARATOR;          \
       str[len] = 0;                            \
     } /* endif */

//---------------------------------------------------------------------------

const char csArchive::hdr_central[4] = {'P', 'K', CENTRAL_HDR_SIG};
const char csArchive::hdr_local[4] = {'P', 'K', LOCAL_HDR_SIG};
const char csArchive::hdr_endcentral[4] = {'P', 'K', END_CENTRAL_SIG};
const char csArchive::hdr_extlocal[4] = {'P', 'K', EXTD_LOCAL_SIG};

//-- Endianess handling -----------------------------------------------------

#define BUFF_GET_(ofs, Type) \
  csLittleEndian::Type (csGetFromAddress::Type ((uint8 *)&buff[ofs]))
#define BUFF_GET_SHORT(ofs)     BUFF_GET_(ofs, UInt16)
#define BUFF_GET_LONG(ofs)      BUFF_GET_(ofs, UInt32)

#define BUFF_SET_(ofs, val, Type, Type2)      \
  csSetToAddress::Type ((uint8 *)&buff[ofs],  \
    csLittleEndian::Convert ((Type2)val))
#define BUFF_SET_SHORT(ofs,val) BUFF_SET_(ofs, val, UInt16, uint16)
#define BUFF_SET_LONG(ofs,val)  BUFF_SET_(ofs, val, UInt32, uint32)

//-- Archive class implementation -------------------------------------------

csArchive::csArchive (const char *filename)
{
  comment = 0;
  comment_length = 0;
  csArchive::filename = CS::StrDup (filename);

  file.AttachNew (new csPhysicalFile (filename, "rb"));
  if (file->GetStatus() != VFS_STATUS_OK)	/* Create new archive file */
    file.AttachNew (new csPhysicalFile (filename, "wb"));
  else
    ReadDirectory ();
}

csArchive::~csArchive ()
{
  cs_free (filename);
  cs_free (comment);
  file.Invalidate();

  size_t i;
  for (i = 0; i < lazy.GetSize (); i++)
  {
    ArchiveEntry* e = lazy[i];
    delete e;
  }
}

void csArchive::ResetArchiveEntry (ArchiveEntry *f, size_t size, bool pack)
{
  f->info.compression_method = (pack) ?
    DEFAULT_COMPRESSION_METHOD : ZIP_STORE;
  f->info.ucsize = (u32)size;
  f->buffer_pos = 0;

  time_t curtime = time (0);
  csFileTime ft = *localtime (&curtime);
  SetFileTime ((void *)f, ft);
}

csArchive::ArchiveEntry *csArchive::CreateArchiveEntry (const char *name, size_t size, bool pack)
{
  ZIP_central_directory_file_header cdfh;

  memset (&cdfh, 0, sizeof (cdfh));
  cdfh.version_made_by[0] = 0x16;		/* Zip version 2.2 rev 6 ??? */
  cdfh.version_made_by[1] = 0x06;
  cdfh.version_needed_to_extract[0] = 20;       /* Unzip version 2.0 rev 0 */
  cdfh.version_needed_to_extract[1] = 00;

  ArchiveEntry *f = new ArchiveEntry (name, cdfh);
  ResetArchiveEntry (f, size, pack);

  return f;
}

void csArchive::ReadDirectory ()
{
  if (dir.GetSize ())
    return;                     /* Directory already read */

  ReadZipDirectory (file);

  //// After reading, ensure that a node entry exists in memory for every
  //// directory component even if the physical archive file does not contain
  //// the corresponding directory entries (which are optional in zip files).

  // First, make a list of all possible directory components.
  csString filename, slice;
  csSet<csString> dset;
  for (size_t i = 0, n = dir.GetSize (); i < n; i++)
  {
    ArchiveEntry const* e = dir.Get (i);
    filename = e->filename;
    size_t sep = 0;
    while (sep < filename.Length ())
    {
      size_t slash = filename.FindFirst ('/', sep);
      if (slash == (size_t)-1)
	slash = filename.FindFirst (CS_PATH_SEPARATOR, sep);
      sep = slash;
      if (sep != (size_t)-1)
      {
	filename.SubString (slice, 0, ++sep);
        if (!dset.In (slice.GetData()))
          dset.AddNoTest (slice); 
      }
    }
  }

  // Now, iterate over `dset' and create fake directory components.
  csSet<csString>::GlobalIterator it = dset.GetIterator();
  while (it.HasNext())
  {
    csString dname (it.Next());
    if (!FileExists (dname))
    {
      ArchiveEntry* f = CreateArchiveEntry(dname, 0, 0);
      f->faked = true;
      dir.InsertSorted (f, ArchiveEntryVector::Compare);
    }
  }
}

void csArchive::ReadZipDirectory (iFile* infile)
{
  ZIP_end_central_dir_record ecdr;
  ZIP_central_directory_file_header cdfh;
  char buff[1024];              /* Read ZIPfile from end in 1K chunks */
  size_t cur_offs, min_offs, central_directory_offset;
  size_t step = ZIP_END_CENTRAL_DIR_RECORD_SIZE + sizeof (hdr_endcentral);

  if (!infile)
    return;                     /* File not open */
  if (!infile->SetPos (infile->GetSize()))
    return;                     /* Seek error */
  if ((cur_offs = infile->GetPos()) == (size_t)-1)
    return;                     /* Can't get file position */

  if (cur_offs >= (65535 + ZIP_END_CENTRAL_DIR_RECORD_SIZE +
  	sizeof (hdr_endcentral)))
    min_offs = cur_offs - (65535 + ZIP_END_CENTRAL_DIR_RECORD_SIZE +
    	sizeof (hdr_endcentral));
  else
    min_offs = 0;

  /* Try to find ZIPfile central directory structure */
  /* For this we have to search from end of file the signature "PK" */
  /* after which follows a two-byte END_CENTRAL_SIG */
  while (cur_offs > min_offs)
  {
    size_t search_pos;

    if (cur_offs >= sizeof (buff) - step)
      cur_offs -= sizeof (buff) - step;
    else
      cur_offs = 0;

    infile->SetPos (cur_offs);
    search_pos = infile->Read (buff, sizeof (buff));
    if (search_pos >= step)
    {
      register char *search_ptr;

      for (search_ptr = &buff[search_pos - step]; search_ptr > buff; search_ptr--)
        if ((*search_ptr == 'P') &&
            (memcmp (search_ptr, hdr_endcentral, sizeof (hdr_endcentral)) == 0))
        {
          /* Central directory structure found */
          central_directory_offset = cur_offs + (size_t)(search_ptr - buff);
          LoadECDR (ecdr, &search_ptr[sizeof (hdr_endcentral)]);
          if (!infile->SetPos ((central_directory_offset + sizeof (hdr_endcentral)
              + ZIP_END_CENTRAL_DIR_RECORD_SIZE))
           || !ReadArchiveComment (infile, ecdr.zipfile_comment_length)
           || !infile->SetPos (ecdr.offset_start_central_directory))
            goto rebuild_cdr;   /* Broken central directory */

          /* Now read central directory structure */
          for (;;)
          {
            if ((infile->Read (buff, sizeof (hdr_central)) <
	    	sizeof (hdr_central)) ||
		(memcmp (buff, hdr_central, sizeof (hdr_central)) != 0))
            {
              if (dir.GetSize ())
                return;         /* Finished reading central directory */
              else
                goto rebuild_cdr;       /* Broken central directory */
            }
            if ((!ReadCDFH (cdfh, infile))
             || (cdfh.filename_length > sizeof (buff))
             || (infile->Read (buff, cdfh.filename_length) <
	     	cdfh.filename_length))
              return;           /* Broken zipfile? */
            buff[cdfh.filename_length] = 0;

            ArchiveEntry *curentry = InsertEntry (buff, cdfh);
            if (!curentry->ReadExtraField (infile, cdfh.extra_field_length)
              || !curentry->ReadFileComment (infile, cdfh.file_comment_length))
              return;         /* Broken zipfile? */
          } /* endfor */
        } /* endif */
    } /* endif */
  } /* endwhile */

rebuild_cdr:
  /* If we are here, we did not succeeded to read central directory */
  /* If so, we have to rebuild it by reading each ZIPfile member separately */
  if (!infile->SetPos (0))
    return;
  ReadZipEntries (infile);
}

void csArchive::ReadZipEntries (iFile* infile)
{
  size_t cur_offs, new_offs;
  char buff[1024];
  ZIP_central_directory_file_header cdfh;
  ZIP_local_file_header lfh;

  cur_offs = 0;
  while ((infile->Read (buff, sizeof (hdr_local)) >= sizeof (hdr_local))
         && (memcmp (buff, hdr_local, sizeof (hdr_local)) == 0)
         && (ReadLFH (lfh, infile)))
  {
    new_offs = cur_offs + sizeof (hdr_local) + ZIP_LOCAL_FILE_HEADER_SIZE +
      lfh.filename_length + lfh.extra_field_length + lfh.csize;
    if ((lfh.filename_length > sizeof (buff))
        || (infile->Read (buff, lfh.filename_length) < lfh.filename_length))
      return;                   /* Broken zipfile? */
    buff[lfh.filename_length] = 0;

    if ((buff[lfh.filename_length - 1] != '/')
        && (buff[lfh.filename_length - 1] != CS_PATH_SEPARATOR))
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
      cdfh.relative_offset_local_header = (u32)cur_offs;

      ArchiveEntry *curentry = InsertEntry (buff, cdfh);

      if (!curentry->ReadExtraField (infile, lfh.extra_field_length))
        return;                 /* Broken zipfile */
    } /* endif */
    cur_offs = new_offs;
    if (!infile->SetPos (new_offs))
      return;                   /* Broken zipfile */
  } /* endwhile */
}

csArchive::ArchiveEntry *csArchive::InsertEntry (const char *name,
  ZIP_central_directory_file_header &cdfh)
{
  size_t dupentry;
  ArchiveEntry *e = new ArchiveEntry (name, cdfh);
  dir.InsertSorted (e, ArchiveEntryVector::Compare, &dupentry);
  if (dupentry != (size_t)-1)
    dir.DeleteIndex (dupentry);
  return e;
}

bool csArchive::ReadArchiveComment (iFile* infile, size_t zipfile_comment_length)
{
  if (comment && (comment_length != zipfile_comment_length))
  {
    cs_free (comment);
    comment = 0;
  }

  comment_length = zipfile_comment_length;
  if (!comment_length)
    return true;

  if (!comment)
    comment = (char*)cs_malloc (zipfile_comment_length);
  return (infile->Read (comment, zipfile_comment_length) ==
  	zipfile_comment_length);
}

void csArchive::Dir () const
{
  csPrintf (" Comp |Uncomp| File |CheckSum| File\n");
  csPrintf (" size | size |offset| (CRC32)| name\n");
  csPrintf ("------+------+------+--------+------\n");
  size_t fn;
  for (fn = 0; fn < dir.GetSize (); fn++)
  {
    ArchiveEntry *e = dir.Get (fn);
    csPrintf ("%6" PRIu32 "|%6" PRIu32 "|%6" PRIu32 "|%08" PRIx32 "|%s\n",
	  e->info.csize, e->info.ucsize,
      e->info.relative_offset_local_header, e->info.crc32, e->filename);
  }
}

void *csArchive::FindName (const char *name) const
{
  size_t idx = dir.FindSortedKey(csArrayCmp<ArchiveEntry*,char const*>(name,
    ArchiveEntryVector::CompareKey));
  if (idx == (size_t)-1)
    return 0;
  return dir.Get (idx);
}

char *csArchive::Read (const char *name, size_t *size)
{
  ArchiveEntry *f = (ArchiveEntry *) FindName (name);

  if (!f)
    return 0;
  char* out_buff = new char[f->info.ucsize+1];
  if (size)
    *size = f->info.ucsize;

  if (!ReadEntry (file, f, out_buff))
  {
    delete[] out_buff;
    return 0;
  }
  out_buff [f->info.ucsize] = 0;
  return out_buff;
}

bool csArchive::ReadEntry (iFile* infile, ArchiveEntry * f, char* out_buff)
{
  // This routine allocates one byte more than is actually needed
  // and fills it with zero. This can be used when reading text files

  size_t bytes_left;
  char buff[1024];
  int err;
  ZIP_local_file_header lfh;

  if (!out_buff)
    return false;

  if ((!infile->SetPos (f->info.relative_offset_local_header))
      || (infile->Read (buff, sizeof (hdr_local)) < sizeof (hdr_local))
      || (memcmp (buff, hdr_local, sizeof (hdr_local)) != 0)
      || (!ReadLFH (lfh, infile))
      || (!infile->SetPos (infile->GetPos() + lfh.filename_length + lfh.extra_field_length)))
  {
    return false;
  }
  switch (f->info.compression_method)
  {
    case ZIP_STORE:
      {
        if (infile->Read  (out_buff, f->info.csize) < f->info.csize)
        {
          return false;
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
          return false;
        }
        while (bytes_left)
        {
          size_t size;

          zs.next_in = (z_Byte *)buff;
          if (bytes_left > sizeof (buff))
            size = sizeof (buff);
          else
            size = bytes_left;
	  zs.avail_in = (uInt)infile->Read (buff, size);

          err = inflate (&zs, bytes_left > size ? Z_PARTIAL_FLUSH : Z_FINISH);
          bytes_left -= size;
        } /* endwhile */
        inflateEnd (&zs);

        /* discard dynamically allocated buffers on error */
        // Kludge warning: I've encountered a file where zlib 1.1.1 returned
        // Z_BUF_ERROR although everything was ok (a slightly compressed PNG
	// file)
        if ((err != Z_STREAM_END)
         && ((err != Z_BUF_ERROR) || bytes_left || zs.avail_out))
        {
          //delete [] out_buff;
          //return 0;
        }
        break;
      }
    default:
      {
        /* Can't handle this compression algorithm */
        return false;
      }
  } /* endswitch */
  return true;
}

void *csArchive::NewFile (const char *name, size_t size, bool pack)
{
  DeleteFile (name);
  size_t idx = lazy.FindKey (csArrayCmp<ArchiveEntry*,char const*>(name,
    ArchiveEntryVector::CompareKey));
  ArchiveEntry *f;
  if (idx != csArrayItemNotFound)
  {
    // If already added without flushing, reuse the previously added
    // entry and just reset the write pointer and compression method.
    f = lazy.Get(idx);
    ResetArchiveEntry (f, size, pack);
  }
  else
  {
    f = CreateArchiveEntry(name, size, pack);
    lazy.Push (f);
  }
  return (void *)f;
}

bool csArchive::DeleteFile (const char *name)
{
  if (!FileExists (name))
    return false;

  del.InsertSorted (name);
  return true;
}

bool csArchive::FileExists (const char *name, size_t * size) const
{
  ArchiveEntry *f = (ArchiveEntry *) FindName (name);

  if (!f)
    return false;
  if (size)
    *size = f->info.ucsize;
  return true;
}

bool csArchive::Write (void *entry, const char *data, size_t len)
{
  if (entry)
    return (((ArchiveEntry *) entry)->Append (data, len));
  else
    return false;
}

// Flush all pending operations (if any)
bool csArchive::Flush ()
{
  if (!lazy.GetSize () && !del.GetSize ())
    return true;                /* Nothing to do */
  return WriteZipArchive ();
}

// Write pending operations into ZIP archive
bool csArchive::WriteZipArchive ()
{
  //char temp_file[CS_MAXPATHLEN];
  csString temp_file ((size_t)CS_MAXPATHLEN);
  csRef<iFile> temp;
  char buff [16 * 1024];
  bool success = false;
  size_t n = 0;

  // Check if file is opened for reading first
  if (!file) return false;

  // Step one: Copy archive file into a temporary file,
  // skipping entries marked as 'deleted'
  temp_file = CS::Platform::GetTempDirectory ();
  temp_file << CS_PATH_SEPARATOR;
  temp_file += CS::Platform::GetTempFilename (temp_file.GetData ());

  temp.AttachNew (new csPhysicalFile (temp_file.GetDataSafe (), "w+b"));
  if (temp->GetStatus() != VFS_STATUS_OK)
    return false;               /* Cannot create temporary file */
  file->SetPos (0);

  while (file->Read (buff, sizeof (hdr_local)) == sizeof (hdr_local))
  {
    size_t bytes_to_copy, bytes_to_skip;
    ArchiveEntry *this_file = 0;

    if (memcmp (buff, hdr_local, sizeof (hdr_local)) == 0)
    {
      // local header
      ZIP_local_file_header lfh;

      if (!ReadLFH (lfh, file))
        goto temp_failed;

      char *this_name = (char*)cs_malloc (lfh.filename_length + 1);
      if (file->Read (this_name, lfh.filename_length) < lfh.filename_length)
      {
        cs_free (this_name);
        goto temp_failed;
      }
      this_name[lfh.filename_length] = 0;

      if (IsDeleted (this_name))
      {
skip_entry:
        bytes_to_skip = lfh.extra_field_length + lfh.csize;
        bytes_to_copy = 0;
        cs_free (this_name);
      }
      else
      {
        this_file = (ArchiveEntry *) FindName (this_name);

        if (!this_file)
          /* This means we found a entry in archive which is not
           * present in our `dir' array: this means either the ZIP
           * file has changed after we read the ZIP directory,
           * or this is a `pure directory' entry (which we ignore
           * during reading). In any case, just copy it unchanged
           * into the output file.
           */
          goto skip_entry;

        cs_free (this_name);
        if (this_file->info.csize != lfh.csize)
          goto temp_failed;   /* Broken archive */
        this_file->ReadExtraField (file, lfh.extra_field_length);
        bytes_to_skip = 0;
        bytes_to_copy = lfh.csize;
        if (!this_file->WriteLFH (temp))
          goto temp_failed;   /* Write error */
      }
    }
    else if (memcmp (buff, hdr_central, sizeof (hdr_central)) == 0)
    {
      // central directory header
      ZIP_central_directory_file_header cdfh;

      if (!ReadCDFH (cdfh, file))
        goto temp_failed;

      bytes_to_copy = 0;
      bytes_to_skip = cdfh.filename_length + cdfh.extra_field_length +
      	cdfh.file_comment_length;
    }
    else if (memcmp (buff, hdr_endcentral, sizeof (hdr_endcentral)) == 0)
    {
      // end-of-central-directory header
      ZIP_end_central_dir_record ecdr;
      char buff [ZIP_END_CENTRAL_DIR_RECORD_SIZE];

      if (file->Read (buff, ZIP_END_CENTRAL_DIR_RECORD_SIZE) <
      	ZIP_END_CENTRAL_DIR_RECORD_SIZE)
        goto temp_failed;
      LoadECDR (ecdr, buff);

      bytes_to_copy = 0;
      bytes_to_skip = ecdr.zipfile_comment_length;
    }
    else
    {
      // Unknown chunk type
      goto temp_failed;
    } /* endif */

    file->SetPos (file->GetPos() + bytes_to_skip);
    while (bytes_to_copy)
    {
      size_t size;

      if (bytes_to_copy > sizeof (buff))
        size = sizeof (buff);
      else
        size = bytes_to_copy;

      if ((file->Read (buff, size) < size)
          || (temp->Write (buff, size) < size))
        goto temp_failed;
      bytes_to_copy -= size;
    }
  } /* endwhile */

  /* Now we have to append all files that were added to archive */
  for (n = 0; n < lazy.GetSize (); n++)
  {
    ArchiveEntry *f = lazy.Get (n);
    if (!f->WriteFile (temp))
      goto temp_failed;			/* Write error */
  } /* endfor */

  /* And finaly write central directory structure */
  if (!WriteCentralDirectory (temp))
    goto temp_failed;

  /* Now copy temporary file into archive. If we'll get a error in process, */
  /* we're lost! I don't know for a good solution for this without wasting */
  /* disk space for yet another copy of the archive :-( */
  {
    size_t fsize = temp->GetSize();

    temp->SetPos (0);

    file.AttachNew (new csPhysicalFile (filename, "wb"));
    if (file->GetStatus() != VFS_STATUS_OK)
    {
      file.AttachNew (new csPhysicalFile (filename, "rb"));
      goto temp_failed;
    }
    while (fsize)
    {
      size_t bytes_read = temp->Read (buff, sizeof (buff));

      if (file->Write (buff, bytes_read) < bytes_read)
      {
        /* Yuck! Keep at least temporary file */
        file.AttachNew (new csPhysicalFile (filename, "rb"));
        return false;
      }
      fsize -= bytes_read;
    }
    /* Hurray! We're done */
    file.AttachNew (new csPhysicalFile (filename, "rb"));
  }

  /* Now if we are here, all operations have been successful */
  UpdateDirectory ();

  success = true;

temp_failed:
  temp.Invalidate();
  unlink (temp_file);
  return success;
}

bool csArchive::WriteCentralDirectory (iFile* temp)
{
  size_t n, count = 0;
  size_t cdroffs = temp->GetPos();

  for (n = 0; n < dir.GetSize (); n++)
  {
    ArchiveEntry *f = dir.Get (n);
    // Don't write deleted or faked entries
    if (IsDeleted (f->filename) || f->faked)
      continue;
    if (!f->WriteCDFH (temp))
      return false;
    count++;
  } /* endwhile */

  for (n = 0; n < lazy.GetSize (); n++)
  {
    ArchiveEntry *f = lazy.Get (n);
    if (!f->WriteCDFH (temp))
      return false;
    count++;
  } /* endwhile */

  /* Write end-of-central-directory record */
  ZIP_end_central_dir_record ecdr;

  memset (&ecdr, 0, sizeof (ecdr));
  ecdr.num_entries_centrl_dir_ths_disk = (ush)count;
  ecdr.total_entries_central_dir = (ush)count;
  ecdr.size_central_directory = (u32)(temp->GetPos() - cdroffs);
  ecdr.offset_start_central_directory = (u32)cdroffs;
  ecdr.zipfile_comment_length = (ush)comment_length;
  if (!WriteECDR (ecdr, temp))
    return false;
  return true;
}

void csArchive::UpdateDirectory ()
{
  /* Update archive directory: remove deleted entries first */
  size_t n = dir.GetSize ();
  while (n > 0)
  {
    n--;
    ArchiveEntry *e = dir.Get (n);
    if (IsDeleted (e->filename))
      dir.DeleteIndex (n);
  }
  del.DeleteAll ();

  for (n = 0; n < lazy.GetSize (); n++)
  {
    ArchiveEntry *e = lazy.Get (n);
    e->FreeBuffer ();
    dir.InsertSorted (e, ArchiveEntryVector::Compare);
    lazy.Put (n, 0);
  }
  lazy.DeleteAll ();
}

bool csArchive::IsDeleted (const char *name) const
{
  return (del.FindSortedKey (name) != csArrayItemNotFound);
}

void csArchive::UnpackTime (ush zdate, ush ztime, csFileTime & rtime) const
{
  memset (&rtime, 0, sizeof (csFileTime));

  rtime.year = (((zdate >> 9) & 0x7f) + 1980);
  rtime.mon = ((zdate >> 5) & 0x0f) - 1;
  rtime.day = (zdate & 0x1f);

  rtime.hour = ((ztime >> 11) & 0x1f);
  rtime.min = ((ztime >> 5) & 0x3f);
  rtime.sec = ((ztime & 0x1f) * 2);
}

void csArchive::PackTime (const csFileTime & ztime, ush & rdate,
	ush & rtime) const
{
  rdate = (((ztime.year - 1980) & 0x7f) << 9)
        | (((ztime.mon & 0x0f) + 1) << 5)
        | (ztime.day & 0x1f);
  rtime = ((ztime.hour & 0x1f) << 11)
        | ((ztime.min & 0x3f) << 5)
        | ((ztime.sec / 2) & 0x1f);
}

void csArchive::LoadECDR (ZIP_end_central_dir_record & ecdr, char *buff)
{
  ecdr.number_this_disk = BUFF_GET_SHORT (E_NUMBER_THIS_DISK);
  ecdr.num_disk_start_cdir = BUFF_GET_SHORT (E_NUM_DISK_WITH_START_CENTRAL_DIR);
  ecdr.num_entries_centrl_dir_ths_disk = BUFF_GET_SHORT (
  	E_NUM_ENTRIES_CENTRL_DIR_THS_DISK);
  ecdr.total_entries_central_dir = BUFF_GET_SHORT (E_TOTAL_ENTRIES_CENTRAL_DIR);
  ecdr.size_central_directory = BUFF_GET_LONG (E_SIZE_CENTRAL_DIRECTORY);
  ecdr.offset_start_central_directory = BUFF_GET_LONG (
  	E_OFFSET_START_CENTRAL_DIRECTORY);
  ecdr.zipfile_comment_length = BUFF_GET_SHORT (E_ZIPFILE_COMMENT_LENGTH);
}

bool csArchive::ReadCDFH (ZIP_central_directory_file_header & cdfh, iFile* infile)
{
  char buff[ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE];

  if (infile->Read (buff, ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE) <
  	ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE)
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
  cdfh.relative_offset_local_header = BUFF_GET_LONG (
  	C_RELATIVE_OFFSET_LOCAL_HEADER);

  return true;
}

bool csArchive::ReadLFH (ZIP_local_file_header & lfh, iFile* infile)
{
  char buff[ZIP_LOCAL_FILE_HEADER_SIZE];

  if (infile->Read (buff, ZIP_LOCAL_FILE_HEADER_SIZE) <
  	ZIP_LOCAL_FILE_HEADER_SIZE)
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

bool csArchive::WriteECDR (ZIP_end_central_dir_record & ecdr, iFile* outfile)
{
  char buff[ZIP_END_CENTRAL_DIR_RECORD_SIZE];

  if (outfile->Write (hdr_endcentral, sizeof (hdr_endcentral)) !=
  	sizeof (hdr_endcentral))
    return false;

  BUFF_SET_SHORT (E_NUMBER_THIS_DISK, ecdr.number_this_disk);
  BUFF_SET_SHORT (E_NUM_DISK_WITH_START_CENTRAL_DIR, ecdr.num_disk_start_cdir);
  BUFF_SET_SHORT (E_NUM_ENTRIES_CENTRL_DIR_THS_DISK,
  	ecdr.num_entries_centrl_dir_ths_disk);
  BUFF_SET_SHORT (E_TOTAL_ENTRIES_CENTRAL_DIR, ecdr.total_entries_central_dir);
  BUFF_SET_LONG (E_SIZE_CENTRAL_DIRECTORY, ecdr.size_central_directory);
  BUFF_SET_LONG (E_OFFSET_START_CENTRAL_DIRECTORY,
  	ecdr.offset_start_central_directory);
  BUFF_SET_SHORT (E_ZIPFILE_COMMENT_LENGTH, ecdr.zipfile_comment_length);

  if ((outfile->Write  (buff, ZIP_END_CENTRAL_DIR_RECORD_SIZE) !=
  	ZIP_END_CENTRAL_DIR_RECORD_SIZE) ||
	(outfile->Write  (comment, comment_length) != comment_length))
    return false;
  return true;
}

csArchive::ArchiveEntry::ArchiveEntry (const char *name,
  ZIP_central_directory_file_header &cdfh)
{
  filename = CS::StrDup (name);
  info = cdfh;
  buffer = 0;
  extrafield = 0;
  comment = 0;
  buffer_pos = 0;
  buffer_size = 0;
  faked = false;
}

csArchive::ArchiveEntry::~ArchiveEntry ()
{
  FreeBuffer ();
  cs_free (comment);
  cs_free (extrafield);
  cs_free (filename);
}

void csArchive::ArchiveEntry::FreeBuffer ()
{
  if (buffer) cs_free (buffer);
  buffer = 0;
  buffer_pos = 0;
  buffer_size = 0;
}

bool csArchive::ArchiveEntry::Append (const void *data, size_t size)
{
  if (!buffer || (buffer_pos + size > buffer_size))
  {
    // Increase buffer size in 1K chunks
    buffer_size += (size + 1023) & ~1023;
    // If the user has defined the uncompressed file size, take it
    if (buffer_size < info.ucsize)
      buffer_size = info.ucsize;
    buffer = (char *)cs_realloc (buffer, buffer_size);
    if (!buffer)
    {
      buffer_pos = buffer_size = info.ucsize = 0;
      return false;				/* Not enough memory */
    }
  } /* endif */

  if (info.ucsize < buffer_pos + size)
    info.ucsize = (u32)(buffer_pos + size);

  memcpy (buffer + buffer_pos, data, size);
  buffer_pos += size;
  return true;
}

bool csArchive::ArchiveEntry::WriteLFH (iFile* outfile)
{
  char buff[ZIP_LOCAL_FILE_HEADER_SIZE];
  size_t lfhpos = outfile->GetPos();

  buff[L_VERSION_NEEDED_TO_EXTRACT_0] = info.version_needed_to_extract[0];
  buff[L_VERSION_NEEDED_TO_EXTRACT_1] = info.version_needed_to_extract[1];
  BUFF_SET_SHORT (L_GENERAL_PURPOSE_BIT_FLAG, info.general_purpose_bit_flag);
  BUFF_SET_SHORT (L_COMPRESSION_METHOD, info.compression_method);
  BUFF_SET_SHORT (L_LAST_MOD_FILE_TIME, info.last_mod_file_time);
  BUFF_SET_SHORT (L_LAST_MOD_FILE_DATE, info.last_mod_file_date);
  BUFF_SET_LONG (L_CRC32, info.crc32);
  BUFF_SET_LONG (L_COMPRESSED_SIZE, info.csize);
  BUFF_SET_LONG (L_UNCOMPRESSED_SIZE, info.ucsize);
  info.filename_length = (ush)strlen (filename);
  BUFF_SET_SHORT (L_FILENAME_LENGTH, info.filename_length);
  info.extra_field_length = extrafield ? info.extra_field_length : 0;
  BUFF_SET_SHORT (L_EXTRA_FIELD_LENGTH,
                  info.extra_field_length);

  if ((outfile->Write (hdr_local, sizeof (hdr_local)) < sizeof (hdr_local))
      || (outfile->Write (buff, ZIP_LOCAL_FILE_HEADER_SIZE) <
          ZIP_LOCAL_FILE_HEADER_SIZE)
      || (outfile->Write (filename, info.filename_length) <
          info.filename_length)
      || (outfile->Write (extrafield, info.extra_field_length) <
          info.extra_field_length))
    return false;

  info.relative_offset_local_header = (u32)lfhpos;
  return true;
}

bool csArchive::ArchiveEntry::WriteCDFH (iFile* outfile)
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

  info.filename_length = (ush)strlen (filename);
  BUFF_SET_SHORT (C_FILENAME_LENGTH, info.filename_length);
  /* We're ignoring extra field for central directory, although InfoZIP puts 
   * there a field containing EF_TIME - universal timestamp - but for example
   * DOS pkzip/pkunzip does not put nothing there. */
  BUFF_SET_SHORT (C_EXTRA_FIELD_LENGTH, 0);
  info.file_comment_length = comment ? info.file_comment_length : 0;
  BUFF_SET_SHORT (C_FILE_COMMENT_LENGTH,
                  info.file_comment_length);
  BUFF_SET_SHORT (C_DISK_NUMBER_START, info.disk_number_start);
  BUFF_SET_SHORT (C_INTERNAL_FILE_ATTRIBUTES, info.internal_file_attributes);
  BUFF_SET_LONG (C_EXTERNAL_FILE_ATTRIBUTES, info.external_file_attributes);
  BUFF_SET_LONG (C_RELATIVE_OFFSET_LOCAL_HEADER,
  	info.relative_offset_local_header);

  if ((outfile->Write (hdr_central, sizeof (hdr_central)) <
  	sizeof (hdr_central))
      || (outfile->Write (buff, ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE) <
      	  ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE)
      || (outfile->Write (filename, info.filename_length) <
      	  info.filename_length)
      || (outfile->Write (comment, info.file_comment_length) <
      	  info.file_comment_length))
    return false;

  return true;
}

bool csArchive::ArchiveEntry::ReadExtraField (iFile* infile,
	size_t extra_field_length)
{
  if (extrafield && (info.extra_field_length != extra_field_length))
  {
    cs_free (extrafield);
    extrafield = 0;
  }
  info.extra_field_length = (ush)extra_field_length;
  if (extra_field_length)
  {
    if (!extrafield)
      extrafield = (char*)cs_malloc (extra_field_length);
    return (infile->Read (extrafield, extra_field_length) ==
    	extra_field_length);
  }
  else return true;
}

bool csArchive::ArchiveEntry::ReadFileComment (iFile* infile,
	size_t file_comment_length)
{
  if (comment && (info.file_comment_length != file_comment_length))
  {
    cs_free (comment);
    comment = 0;
  }
  info.file_comment_length = (ush)file_comment_length;
  if (file_comment_length)
  {
    if (!comment)
      comment = (char*)cs_malloc (file_comment_length);
    return (infile->Read (comment, file_comment_length) ==
    	file_comment_length);
  }
  else return true;
}

bool csArchive::ArchiveEntry::WriteFile (iFile* outfile)
{
  size_t lfhoffs = outfile->GetPos ();

  info.crc32 = crc32 (CRCVAL_INITIAL, (z_Byte *)buffer, (uInt)buffer_pos);
  bool finished = false;

  while (!finished)
  {
    if (!outfile->SetPos (lfhoffs + sizeof (hdr_local) +
    	ZIP_LOCAL_FILE_HEADER_SIZE +
	strlen (filename) + (extrafield ? info.extra_field_length : 0)))
      return false;

    switch (info.compression_method)
    {
      case ZIP_STORE:
        {
          if (outfile->Write (buffer, buffer_pos) < buffer_pos)
            return false;       /* Write error */
          info.csize = info.ucsize = (u32)buffer_pos;
          finished = true;
          break;
        }
      case ZIP_DEFLATE:
        {
          z_stream zs;

          zs.zalloc = (alloc_func) 0;
          zs.zfree = (free_func) 0;
          zs.next_in = (z_Byte *) buffer;
          zs.avail_in = (uInt)buffer_pos;
          if (deflateInit (&zs, DEFAULT_COMPRESSION_LEVEL) != Z_OK)
            return false;
          info.csize = 0;
          info.ucsize = (u32)buffer_pos;

          char buff[16384];
          int buffofs = 2;      /* Skip inflated data header */

          while (1)
          {
            zs.next_out = (z_Byte *)buff;
            zs.avail_out = sizeof (buff);

            int rc = deflate (&zs, Z_FINISH);   /* Do actual compression */
            size_t size = sizeof (buff) - zs.avail_out - buffofs;

            info.csize += (u32)size;

            if (outfile->Write (&buff[buffofs], size) != size)
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

  outfile->SetPos (lfhoffs);
  if (!WriteLFH (outfile))
    return false;
  outfile->SetPos (outfile->GetPos() + info.csize);
  return true;
}
