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

#include "cssysdef.h"
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vfs.h"
#include "csutil/archive.h"
#include "csutil/cmdline.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/parray.h"
#include "csutil/scfstringarray.h"
#include "csutil/strset.h"
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/util.h"
#include "csutil/vfsplat.h"
#include "iutil/objreg.h"
#include "iutil/verbositymanager.h"

CS_IMPLEMENT_PLUGIN

// Characters ignored in VFS paths (except in middle)
#define CS_VFSSPACE		" \t"

//***********************************************************
// NOTE on naming convention: public classes begin with "cs"
// while private (local) classes do not.
//***********************************************************

// Minimal time (msec) that an unused archive will be kept unclosed
#define VFS_KEEP_UNUSED_ARCHIVE_TIME	10000

// This is a version of csFile which "lives" on plain filesystem
class DiskFile : public csFile
{
  friend class VfsNode;

  // The file
  FILE *file;
  // Contains the complete file, if GetAllData() was called
  csRef<iDataBuffer> alldata;
  // constructor
  DiskFile(int Mode, VfsNode* ParentNode, size_t RIndex,
  	const char* NameSuffix);
  // set Error according to errno
  void CheckError ();
  // whether this file was opened for writing or reading
  bool writemode;
  // 'real-world' path of this file
  char *fName;
  // position in the data buffer
  size_t fpos;
  // whether alldata is null-terminated
  bool buffernt;

  // attempt to create a file mapping buffer from this file
  iDataBuffer* TryCreateMapping ();
public:
  SCF_DECLARE_IBASE;

  // destructor
  virtual ~DiskFile ();
  // read a block of data
  virtual size_t Read (char *Data, size_t DataSize);
  // write a block of data
  virtual size_t Write (const char *Data, size_t DataSize);
  /// flush stream
  virtual void Flush ();
  // check for EOF
  virtual bool AtEOF ();
  /// Query current file pointer
  virtual size_t GetPos ();
  /// Clear file error after queriyng status
  virtual int GetStatus ();
  /// Set file position
  virtual bool SetPos (size_t newpos);
  /// Get all data
  virtual csPtr<iDataBuffer> GetAllData (bool nullterm = false);
private:
  // Create a directory or a series of directories starting from PathBase
  void MakeDir (const char *PathBase, const char *PathSuffix);
};

// used by ArchiveFile
class VfsArchive;

// This is a version of csFile which "lives" in archives
class ArchiveFile : public csFile
{
private:
  friend class VfsNode;

  // parent archive
  VfsArchive *Archive;
  // The file handle
  void *fh;
  // file data (for read mode)
  char *data;
  // buffer, where read mode data is contained
  csRef<iDataBuffer> databuf;
  // current data pointer
  size_t fpos;
  // constructor
  ArchiveFile (int Mode, VfsNode *ParentNode, size_t RIndex,
    const char *NameSuffix, VfsArchive *ParentArchive);

public:
  SCF_DECLARE_IBASE;

  // destructor
  virtual ~ArchiveFile ();
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

  void UpdateTime ()
  {
    LastUseTime = csGetTicks ();
  }
  void IncRef ()
  {
    RefCount++;
    UpdateTime ();
  }
  void DecRef ()
  {
    if (RefCount)
      RefCount--;
    UpdateTime ();
  }
  bool CheckUp ()
  {
    return (RefCount == 0) &&
      (csGetTicks () - LastUseTime > VFS_KEEP_UNUSED_ARCHIVE_TIME);
  }
  VfsArchive (const char *filename, iObjectRegistry *object_reg)
  	: csArchive (filename)
  {
    RefCount = 0;
    Writing = 0;
    VfsArchive::object_reg = object_reg;
    UpdateTime (); // OpenStep compiler requires having seen this already.
#ifdef VFS_DEBUG
    printf ("VFS: opening archive \"%s\"\n", filename);
#endif
    // We need a recursive mutex.
    archive_mutex = csMutex::Create (true);
  }
  virtual ~VfsArchive ()
  {
    CS_ASSERT (RefCount == 0);
#ifdef VFS_DEBUG
    printf ("VFS: archive \"%s\" closing (writing=%d)\n", GetName (), Writing);
#endif
    Flush ();
#ifdef VFS_DEBUG
    printf ("VFS: archive \"%s\" closed\n", GetName ());
#endif
  }
};

/// This class is thread-safe because it is global.
class VfsArchiveCache
{
private:
  csPDelArray<VfsArchive> array;

public:
  VfsArchiveCache () : array (8, 8)
  {
  }
  virtual ~VfsArchiveCache ()
  {
    array.DeleteAll ();
  }

  /// Find a given archive file.
  size_t FindKey (const char* Key) const
  {
    size_t i;
    for (i = 0; i < array.Length (); i++)
      if (strcmp (array[i]->GetName (), Key) == 0)
        return i;
    return (size_t)-1;
  }

  VfsArchive *Get (size_t iIndex)
  {
    return array.Get (iIndex);
  }

  size_t Length () const
  {
    return array.Length ();
  }

  void Push (VfsArchive* ar)
  {
    array.Push (ar);
  }

  void DeleteAll ()
  {
    array.DeleteAll ();
  }

  void FlushAll ()
  {
    size_t i = 0;
    while (i < array.Length ())
    {
      array[i]->Flush ();
      if (array[i]->RefCount == 0)
      {
	array.DeleteIndex (i);
      }
      else
      {
	i++;
      }
    }
  }

  void CheckUp ()
  {
    size_t i = array.Length ();
    while (i > 0)
    {
      i--;
      VfsArchive *a = array.Get (i);
      if (a->CheckUp ())
        array.DeleteIndex (i);
    }
  }
};

// Private structure used to keep a "node" in virtual filesystem tree.
// The program can be made even fancier if we use a object for each
// "real" path (i.e. each VfsNode will contain an array of real-world
// nodes - both "directory" and "archive" types) but since we have to
// balance between pretty understandable code and effective code, this
// time we choose effectivity - the cost can become very big in this case.
class VfsNode
{
public:
  // The virtual path
  char *VPath;
  // Configuration section key
  char *ConfigKey;
  // The array of real paths/archives bound to this virtual path
  csStringArray RPathV;
  // The array of unexpanded real paths
  csStringArray UPathV;
  // The object registry.
  iObjectRegistry *object_reg;

  // Initialize the object
  VfsNode (char *iPath, const char *iConfigKey, iObjectRegistry *object_reg);
  // Destroy the object
  virtual ~VfsNode ();

  // Parse a directory link directive and fill RPathV
  bool AddRPath (const char *RealPath, csVFS *Parent);
  // Remove a real-world path
  bool RemoveRPath (const char *RealPath);
  // Find all files in a subpath
  void FindFiles(const char *Suffix, const char *Mask, iStringArray *FileList);
  // Find a file and return the appropiate csFile object
  iFile *Open (int Mode, const char *Suffix);
  // Delete a file
  bool Delete (const char *Suffix);
  // Does file exists?
  bool Exists (const char *Suffix);
  // Query date/time
  bool GetFileTime (const char *Suffix, csFileTime &oTime) const;
  // Set date/time
  bool SetFileTime (const char *Suffix, const csFileTime &iTime);
  // Get file size
  bool GetFileSize (const char *Suffix, size_t &oSize);
private:
  // Get value of a variable
  const char *GetValue (csVFS *Parent, const char *VarName);
  // Copy a string from src to dst and expand all variables
  csString Expand (csVFS *Parent, char const *src);
  // Find a file either on disk or in archive - in this node only
  bool FindFile (const char *Suffix, char *RealPath, csArchive *&) const;
};

// The global archive cache
static VfsArchiveCache *ArchiveCache = 0;

// -------------------------------------------------------------- csFile --- //

csFile::csFile (int Mode, VfsNode *ParentNode, size_t RIndex,
  const char *NameSuffix)
{
  (void)Mode;
  Node = ParentNode;
  Index = RIndex;
  Size = 0;
  Error = VFS_STATUS_OK;

  size_t vpl = strlen (Node->VPath);
  size_t nsl = strlen (NameSuffix);
  Name = new char [vpl + nsl + 1];
  memcpy (Name, Node->VPath, vpl);
  memcpy (Name + vpl, NameSuffix, nsl + 1);
}

csFile::~csFile ()
{
  delete [] Name;
  ArchiveCache->CheckUp ();
}

int csFile::GetStatus ()
{
  int rc = Error;
  Error = VFS_STATUS_OK;
  return rc;
}

// ------------------------------------------------------------ DiskFile --- //

#ifdef CS_HAVE_MEMORY_MAPPED_IO
bool csMemoryMapFile(csMemMapInfo*, char const* filename);
void csUnMemoryMapFile(csMemMapInfo*);

class csMMapDataBuffer : public iDataBuffer
{
  csMemMapInfo mapping;
  bool status;
public:
  SCF_DECLARE_IBASE;

  csMMapDataBuffer (const char* filename);
  virtual ~csMMapDataBuffer ();

  bool GetStatus() { return status; }

  virtual size_t GetSize () const { return mapping.file_size; };
  virtual char* GetData () const { return (char*)mapping.data; };
};

SCF_IMPLEMENT_IBASE (csMMapDataBuffer)
  SCF_IMPLEMENTS_INTERFACE (iDataBuffer);
SCF_IMPLEMENT_IBASE_END

csMMapDataBuffer::csMMapDataBuffer (const char* filename)
{
  SCF_CONSTRUCT_IBASE (0);

  status = csMemoryMapFile (&mapping, filename);
  if (!status)
  {
    mapping.data = 0; 
    mapping.file_size = 0;
  }
}

csMMapDataBuffer::~csMMapDataBuffer ()
{
  if (status)
  {
    csUnMemoryMapFile (&mapping);
  }
  SCF_DESTRUCT_IBASE();
}

#endif

SCF_IMPLEMENT_IBASE (DiskFile)
  SCF_IMPLEMENTS_INTERFACE (iFile)
SCF_IMPLEMENT_IBASE_END

#ifndef O_BINARY
#  define O_BINARY 0
#endif

#define VFS_READ_MODE	(O_RDONLY | O_BINARY)
#define VFS_WRITE_MODE	(O_CREAT | O_TRUNC | O_WRONLY | O_BINARY)

// files above this size are attempted to be mapped into memory, 
// instead of accessed via 'normal' file operations
#define VFS_DISKFILE_MAPPING_THRESHOLD_MIN	    0
// same as above, but upper size limit
#define VFS_DISKFILE_MAPPING_THRESHOLD_MAX	    256*1024*1024
// disabled for now.
// #define VFS_DISKFILE_MAPPING

DiskFile::DiskFile (int Mode, VfsNode *ParentNode, size_t RIndex,
  const char *NameSuffix) : csFile (Mode, ParentNode, RIndex, NameSuffix)
{
  SCF_CONSTRUCT_IBASE (0);
  char *rp = (char *)Node->RPathV [Index];
  size_t rpl = strlen (rp);
  size_t nsl = strlen (NameSuffix);
  fName = new char [rpl + nsl + 1];
  memcpy (fName, rp, rpl);
  memcpy (fName + rpl, NameSuffix, nsl + 1);

  // Convert all VFS_PATH_SEPARATOR's in filename into CS_PATH_SEPARATOR's
  size_t n;
  for (n = 0; n < nsl; n++)
    if (fName [rpl + n] == VFS_PATH_SEPARATOR)
      fName [rpl + n] = CS_PATH_SEPARATOR;

  writemode = (Mode & VFS_FILE_MODE) != VFS_FILE_READ;

  int t;
  for (t = 1; t <= 2; t++)
  {
#ifdef VFS_DEBUG
    printf ("VFS: Trying to open disk file \"%s\"\n", fName);
#endif
    if ((Mode & VFS_FILE_MODE) == VFS_FILE_WRITE)
        file = fopen (fName, "wb");
    else if ((Mode & VFS_FILE_MODE) == VFS_FILE_APPEND)
        file = fopen (fName, "ab");
    else
        file = fopen (fName, "rb");

    if (file || (t != 1))
      break;

    // we don't need to create a directory if we only want to read
    if ((Mode & VFS_FILE_MODE) == VFS_FILE_READ)
      break;
    
    char *lastps = (char*)strrchr (NameSuffix, VFS_PATH_SEPARATOR);
    if (!lastps)
      break;

    *lastps = 0;
    MakeDir (rp, NameSuffix);
    *lastps = VFS_PATH_SEPARATOR;
  }

  if (!file)
    CheckError ();
  if (Error == VFS_STATUS_OK)
  {
    if (fseek (file, 0, SEEK_END))
      CheckError ();
    Size = ftell (file);
    if (Size == (size_t)-1)
    {
      Size = 0;
      CheckError ();
    }
    if ((Mode & VFS_FILE_MODE) != VFS_FILE_APPEND)
    {
      if (fseek (file, 0, SEEK_SET))
        CheckError ();
    }
  }
#ifdef VFS_DEBUG
  if (file)
    printf ("VFS: Successfully opened, handle = %d\n", fileno (file));
#endif

#if defined(VFS_DISKFILE_MAPPING) && defined(CS_HAVE_MEMORY_MAPPED_IO)
  if ((Error == VFS_STATUS_OK) && (!writemode) && 
    (Size >= VFS_DISKFILE_MAPPING_THRESHOLD_MIN) &&
    (Size <= VFS_DISKFILE_MAPPING_THRESHOLD_MAX))
  {
    alldata = csPtr<iDataBuffer> (TryCreateMapping ());
    if (alldata)
    {
      fclose (file);
      file = 0;
      SetPos (0);
      buffernt = false;
    }
  }
#endif
}

DiskFile::~DiskFile ()
{
#ifdef VFS_DEBUG
  if (file)
    printf ("VFS: Closing some file with handle = %d\n", fileno (file));
  else
    printf ("VFS: Deleting an unsuccessfully opened file\n");
#endif

  if (file)
    fclose (file);
  delete [] fName;

  SCF_DESTRUCT_IBASE();
}

void DiskFile::MakeDir (const char *PathBase, const char *PathSuffix)
{
  size_t pbl = strlen (PathBase);
  size_t pl = pbl + strlen (PathSuffix) + 1;
  char *path = new char [pl];
  char *cur = path + pbl;
  char *prev = 0;

  strcpy (path, PathBase);
  strcpy (cur, PathSuffix);

  // Convert all VFS_PATH_SEPARATOR's in path into CS_PATH_SEPARATOR's
  for (size_t n = 0; n < pl; n++)
    if (path [n] == VFS_PATH_SEPARATOR)
      path [n] = CS_PATH_SEPARATOR;

  while (cur != prev)
  {
    prev = cur;

    char oldchar = *cur;
    *cur = 0;
#ifdef VFS_DEBUG
    printf ("VFS: Trying to create directory \"%s\"\n", path);
#endif
    CS_MKDIR (path);
    *cur = oldchar;
    if (*cur)
      cur++;

    while (*cur && (*cur != CS_PATH_SEPARATOR))
      cur++;
  }
  delete [] path;
}

int DiskFile::GetStatus ()
{
  if (file != 0)
    clearerr (file);
  return csFile::GetStatus ();
}

void DiskFile::CheckError ()
{
  // The first error usually is the main cause, so we won't
  // overwrite it until user reads it with Status ()
  if (Error != VFS_STATUS_OK)
    return;

  // If file descriptor is invalid, that's really bad
  if (!file)
  {
    Error = VFS_STATUS_OTHER;
    return;
  }

  if (!ferror (file))
    return;

  // note: if some OS does not have a specific errno value,
  // DON'T remove it from switch statement. Instead, take it in a
  // #ifdef ... #endif brace. Look at ETXTBSY for a example.
  switch (errno)
  {
    case 0:
      Error = VFS_STATUS_OK;
      break;
#ifdef ENOSPC
    case ENOSPC:
      Error = VFS_STATUS_NOSPACE;
      break;
#endif
#ifdef EMFILE
    case EMFILE:
#endif
#ifdef ENFILE
    case ENFILE:
#endif
#ifdef ENOMEM
    case ENOMEM:
#endif
#if defined( EMFILE ) || defined( ENFILE ) || defined( ENOMEM )
      Error = VFS_STATUS_RESOURCES;
      break;
#endif
#ifdef ETXTBSY
    case ETXTBSY:
#endif
#ifdef EROFS
    case EROFS:
#endif
#ifdef EPERM
   case EPERM:
#endif
#ifdef EACCES
   case EACCES:
#endif
#if defined( ETXTBSY ) || defined( EROFS ) || defined( EPERM ) || \
    defined( EACCES )
      Error = VFS_STATUS_ACCESSDENIED;
      break;
#endif
#ifdef EIO
    case EIO:
      Error = VFS_STATUS_IOERROR;
      break;
#endif
    default:
      Error = VFS_STATUS_OTHER;
      break;
  }
}

size_t DiskFile::Read (char *Data, size_t DataSize)
{
  if (writemode)
  {
    Error = VFS_STATUS_ACCESSDENIED;
    return 0;
  }
  else
  {
    if (file)
    {
      size_t rc = fread (Data, 1, DataSize, file);
      if (rc < DataSize)
	CheckError ();
      return rc;
    }
    else
    {
      size_t rc = MIN (DataSize, Size - fpos);
      memcpy (Data, (void*)(alldata->GetData() + fpos), rc);
      fpos += rc;
      return rc;
    }
  }
}

size_t DiskFile::Write (const char *Data, size_t DataSize)
{
  if (!writemode)
  {
    Error = VFS_STATUS_ACCESSDENIED;
    return 0;
  }
  else
  {
    size_t rc = fwrite (Data, 1, DataSize, file);
    if (rc < DataSize)
      CheckError ();
    return rc;
  }
}

void DiskFile::Flush ()
{
  if (file)
    fflush (file);
}

bool DiskFile::AtEOF ()
{
  if (file)
  {
    return feof (file);
  }
  else
  {
    return (fpos >= Size);
  }
}

size_t DiskFile::GetPos ()
{
  if (file)
  {
    return ftell (file);
  }
  else
  {
    return fpos;
  }
}

bool DiskFile::SetPos (size_t newpos)
{
  if (file)
  {
    return (fseek (file, newpos, SEEK_SET) == 0);
  }
  else
  {
    fpos = (newpos > Size) ? Size : newpos;
    return true;
  }
}

csPtr<iDataBuffer> DiskFile::GetAllData (bool nullterm)
{
// retrieve file contents

  // refuse to work when writing
  if (!writemode)
  {
    // do we already have everything?
    if (!alldata)
    {
      iDataBuffer* newbuf = 0;
      // attempt to create file mapping
      size_t oldpos = GetPos();
      if (!nullterm)
      {
	newbuf = TryCreateMapping ();
      }
      // didn't succeed or not supported -
      // old style readin'
      if (!newbuf)
      {
	SetPos (0);

	char* data = new char [Size + 1];
	csDataBuffer* dbuf = new csDataBuffer (data, Size);
	Read (data, Size);
	*((char*)data + Size) = 0;

	newbuf = dbuf;
      }
      // close file, set correct pos
      fclose (file);
      file = 0;
      SetPos (oldpos);
      // setup buffer.
      alldata = csPtr<iDataBuffer> (newbuf);
      buffernt = nullterm;
    }
    else
    {
      // The data was already read.
      if (nullterm && !buffernt)
      {
	// However, a null-terminated buffer is requested,
	// but this one isn't yet - copy data, append null
	alldata = csPtr<iDataBuffer> 
	  (new csDataBuffer (alldata));

        buffernt = nullterm;
      }
    }
    return csPtr<iDataBuffer> (alldata);
  }
  else
  {
    return 0;
  }
}

iDataBuffer* DiskFile::TryCreateMapping ()
{
#ifdef CS_HAVE_MEMORY_MAPPED_IO  
  csMMapDataBuffer* buf = new csMMapDataBuffer (fName);
  if (buf->GetStatus())
  {
    return buf;
  }
  else
  {
    delete buf;
  }
#endif
  return 0;
}

// --------------------------------------------------------- ArchiveFile --- //

SCF_IMPLEMENT_IBASE (ArchiveFile)
  SCF_IMPLEMENTS_INTERFACE (iFile)
SCF_IMPLEMENT_IBASE_END

ArchiveFile::ArchiveFile (int Mode, VfsNode *ParentNode, size_t RIndex,
  const char *NameSuffix, VfsArchive *ParentArchive) :
  csFile (Mode, ParentNode, RIndex, NameSuffix)
{
  SCF_CONSTRUCT_IBASE (0);
  Archive = ParentArchive;
  Error = VFS_STATUS_OTHER;
  Size = 0;
  fh = 0;
  data = 0;
  fpos = 0;

  csScopedMutexLock lock (Archive->archive_mutex);
  Archive->UpdateTime ();
  ArchiveCache->CheckUp ();

#ifdef VFS_DEBUG
  printf ("VFS: Trying to open file \"%s\" from archive \"%s\"\n",
    NameSuffix, Archive->GetName ());
#endif

  if ((Mode & VFS_FILE_MODE) == VFS_FILE_READ)
  {
    // If reading a file, flush all pending operations
    if (Archive->Writing == 0)
      Archive->Flush ();
    if ((data = Archive->Read (NameSuffix, &Size)))
    {
      Error = VFS_STATUS_OK;
      databuf = csPtr<iDataBuffer> (new csDataBuffer (data, Size));
    }
  }
  else if ((Mode & VFS_FILE_MODE) == VFS_FILE_WRITE)
  {
    if ((fh = Archive->NewFile(NameSuffix,0,!(Mode & VFS_FILE_UNCOMPRESSED))))
    {
      Error = VFS_STATUS_OK;
      Archive->Writing++;
    }
  }
  Archive->IncRef ();
}

ArchiveFile::~ArchiveFile ()
{
#ifdef VFS_DEBUG
  printf ("VFS: Closing some file from archive \"%s\"\n", Archive->GetName ());
#endif

  csScopedMutexLock lock (Archive->archive_mutex);
  if (fh)
    Archive->Writing--;
  Archive->DecRef ();

  SCF_DESTRUCT_IBASE();
}

size_t ArchiveFile::Read (char *Data, size_t DataSize)
{
  if (data)
  {
    size_t sz = DataSize;
    if (fpos + sz > Size)
      sz = Size - fpos;
    memcpy (Data, data + fpos, sz);
    fpos += sz;
    return sz;
  }
  else
  {
    Error = VFS_STATUS_ACCESSDENIED;
    return 0;
  }
}

size_t ArchiveFile::Write (const char *Data, size_t DataSize)
{
  if (data)
  {
    Error = VFS_STATUS_ACCESSDENIED;
    return 0;
  }
  csScopedMutexLock lock (Archive->archive_mutex);
  if (!Archive->Write (fh, Data, DataSize))
  {
    Error = VFS_STATUS_NOSPACE;
    return 0;
  }
  return DataSize;
}

void ArchiveFile::Flush ()
{
  if (Archive)
  {
    csScopedMutexLock lock (Archive->archive_mutex);
    Archive->Flush ();
  }
}

bool ArchiveFile::AtEOF ()
{
  if (data)
    return fpos + 1 >= Size;
  else
    return true;
}

size_t ArchiveFile::GetPos ()
{
  return fpos;
}

bool ArchiveFile::SetPos (size_t newpos)
{
  if (data)
  {
    fpos = (newpos > Size) ? Size : newpos;
    return true;
  }
  else
  {
    return false;
  }
}

csPtr<iDataBuffer> ArchiveFile::GetAllData (bool nullterm)
{
  if (data)
  {
    return csPtr<iDataBuffer> (databuf);
  }
  else
  {
    return 0;
  }
}

// ------------------------------------------------------------- VfsNode --- //

VfsNode::VfsNode (char *iPath, const char *iConfigKey,
	iObjectRegistry *object_reg)
{
  VPath = iPath;
  ConfigKey = csStrNew (iConfigKey);
  VfsNode::object_reg = object_reg;
}

VfsNode::~VfsNode ()
{
  delete [] ConfigKey;
  delete [] VPath;
}

bool VfsNode::AddRPath (const char *RealPath, csVFS *Parent)
{
  bool rc = false;
  csString const expanded_path = Expand(Parent, RealPath);
  // Split rpath into several, separated by commas
  size_t rpl = expanded_path.Length();
  char *cur, *src;
  char *oldsrc = src = csStrNew (expanded_path);
  for (cur = src, rpl++; rpl-- > 0; cur++)
  {
    if ((rpl == 0) || (*cur == VFS_PATH_DIVIDER))
    {
      *cur = 0;
      src += strspn (src, CS_VFSSPACE);
      size_t cl = strlen (src);
      while (cl && strchr (CS_VFSSPACE, src [cl - 1]))
        cl--;
      if (cl == 0)
      {
        src = cur;
        continue;
      } /* endif */
      src [cl] = 0;

      rc = true;
      UPathV.Push (src);

      // Now parse this path
      char rpath [CS_MAXPATHLEN + 1];
      strncpy(rpath, src, CS_MAXPATHLEN);
      rpath[CS_MAXPATHLEN] = '\0';
      RPathV.Push (rpath);
      src = cur + 1;
    } /* endif */
  } /* for */

  delete [] oldsrc;
  return rc;
}

bool VfsNode::RemoveRPath (const char *RealPath)
{
  if (!RealPath)
  {
    RPathV.DeleteAll ();
    UPathV.DeleteAll ();
    return true;
  } /* endif */

  size_t i;
  for (i = 0; i < RPathV.Length (); i++)
    if (strcmp ((char *)RPathV.Get (i), RealPath) == 0)
    {
      RPathV.DeleteIndex (i);
      UPathV.DeleteIndex (i);
      return true;
    } /* endif */

  return false;
}

csString VfsNode::Expand (csVFS *Parent, char const *source)
{
  csString dst;
  char *src_start = csStrNew(source);
  char *src = src_start;
  while (*src != '\0')
  {
    // Is this a variable reference?
    if (*src == '$')
    {
      // Parse the name of variable
      src++;
      char *var = src;
      char one_letter_varname [2];
      if (*src == '(' || *src == '{')
      {
        // Parse until the end of variable, skipping pairs of '(' and ')'
        int level = 1;
        src++; var++;
        while (level > 0 && *src != '\0')
        {
          if (*src == '(' || *src == '{')
	  {
            level++;
	  }
          else if (*src == ')' || *src == '}')
	  {
            level--;
	  }
	  if (level > 0)
	    src++; // don't skip over the last parenthesis
        } /* endwhile */
        // Replace closing parenthesis with \0
        *src++ = '\0';
      }
      else
      {
        var = one_letter_varname;
        var [0] = *src++;
        var [1] = 0;
      }

      char *alternative = strchr (var, ':');
      if (alternative)
        *alternative++ = '\0';
      else
        alternative = strchr (var, '\0');

      const char *value = GetValue (Parent, var);
      if (!value)
      {
        if (*alternative)
          dst << Expand (Parent, alternative);
      }
      else
      {
	// @@@ FIXME: protect against circular references
        dst << Expand (Parent, value);
      }
    } /* endif */
    else
      dst << *src++;
  } /* endif */
  delete[] src_start;
  return dst;
}

const char *VfsNode::GetValue (csVFS *Parent, const char *VarName)
{
  // Look in environment first
  const char *value = getenv (VarName);
  if (value)
    return value;

  iConfigFile *Config = &(Parent->config);

  // Now look in "VFS.Unix" section, for example
  csString Keyname;
  Keyname << "VFS." CS_PLATFORM_NAME "." << VarName;
  value = Config->GetStr (Keyname, 0);
  if (value)
    return value;

  // Now look in "VFS.Alias" section for alias section name
  const char *alias = Config->GetStr ("VFS.Alias." CS_PLATFORM_NAME, 0);
  // If there is one, look into that section too
  if (alias)
  {
    Keyname.Clear();
    Keyname << alias << '.' << VarName;
    value = Config->GetStr (Keyname, 0);
  }
  if (value)
    return value;

  // Handle predefined variables here so that user
  // can override them in config file or environment

  // check for OS-specific predefined variables
  value = csCheckPlatformVFSVar(VarName);
  if (value)
    return value;

  static char path_separator [] = {VFS_PATH_SEPARATOR, 0};
  if (strcmp (VarName, path_separator) == 0)	// Path separator variable?
  {
    static char path_sep [] = {CS_PATH_SEPARATOR, 0};
    return path_sep;
  }

  if (strcmp (VarName, "*") == 0) // Resource directory?
    return Parent->resdir;
    
  if (strcmp (VarName, "^") == 0) // Application or Cocoa wrapper directory?
    return Parent->appdir;
    
  if (strcmp (VarName, "@") == 0) // Installation directory?
    return Parent->basedir;

  return 0;
}

void VfsNode::FindFiles (const char *Suffix, const char *Mask,
  iStringArray *FileList)
{
  // Look through all RPathV's for file or directory
  size_t i;
  csString vpath;
  for (i = 0; i < RPathV.Length (); i++)
  {
    char *rpath = (char *)RPathV [i];
    size_t rpl = strlen (rpath);
    if (rpath [rpl - 1] == CS_PATH_SEPARATOR)
    {
      // rpath is a directory
      DIR *dh;
      struct dirent *de;

      char tpath [CS_MAXPATHLEN + 1];
      memcpy (tpath, rpath, rpl);
      strcpy (tpath + rpl, Suffix);
      rpl = strlen (tpath);
      if ((rpl > 1)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
       && ((rpl > 2) || (tpath [1] != ':'))
       && (!((rpl == 3) && (tpath [1] == ':') && (tpath [2] == '\\')))
       // keep trailing backslash for drive letters
#endif
       && ((tpath [rpl - 1] == '/') || (tpath [rpl - 1] == CS_PATH_SEPARATOR)))
        tpath [rpl - 1] = 0;		// remove trailing CS_PATH_SEPARATOR

      if ((dh = opendir (tpath)) == 0)
        continue;
      while ((de = readdir (dh)) != 0)
      {
        if ((strcmp (de->d_name, ".") == 0)
         || (strcmp (de->d_name, "..") == 0))
          continue;

        if (!csGlobMatches (de->d_name, Mask))
          continue;

        bool append_slash = isdir (tpath, de);
	vpath.Clear();
	vpath << VPath << Suffix << de->d_name;
	if (append_slash)
	{
	  vpath << VFS_PATH_SEPARATOR;
	}

	FileList->Push (vpath);
      } /* endwhile */
      closedir (dh);
    }
    else
    {
      // rpath is an archive
      size_t idx = ArchiveCache->FindKey (rpath);
      // archive not in cache?
      if (idx == (size_t)-1)
      {
        // does file rpath exist?
        if (access (rpath, F_OK) != 0)
          continue;

        idx = ArchiveCache->Length ();
        ArchiveCache->Push (new VfsArchive (rpath, object_reg));
      }

      VfsArchive *a = ArchiveCache->Get (idx);
      // Flush all pending operations
      a->UpdateTime ();
      if (a->Writing == 0)
        a->Flush ();
      void *iterator;
      size_t sl = strlen (Suffix);
      int no = 0;
      while ((iterator = a->GetFile (no++)))
      {
        char *fname = a->GetFileName (iterator);
	size_t fnl = strlen (fname);
	if ((fnl >= sl) && (memcmp (fname, Suffix, sl) == 0)
         && csGlobMatches (fname, Mask))
	{
          size_t cur = sl;

          // Do not return an entry for the directory itself.
          if (fname[cur] == 0)
            continue;

	  while (cur < fnl)
	  {
	    if (fname [cur] == VFS_PATH_SEPARATOR)
	      break;
	    cur++;
	  }
	  if (cur < fnl)
	    cur++;
          size_t vpl = strlen (VPath);
	  vpath.Clear();
	  vpath << VPath;
	  vpath << fname;
	  vpath.Truncate (vpl + cur);
	  if (FileList->Find (vpath) == csArrayItemNotFound)
            FileList->Push (vpath);
        }
      }
    }
  }
}

iFile* VfsNode::Open (int Mode, const char *FileName)
{
  csFile *f = 0;

  // Look through all RPathV's for file or directory
  size_t i;
  for (i = 0; i < RPathV.Length (); i++)
  {
    char *rpath = (char *)RPathV [i];
    if (rpath [strlen (rpath) - 1] == CS_PATH_SEPARATOR)
    {
      // rpath is a directory
      f = new DiskFile (Mode, this, i, FileName);
      if (f->GetStatus () == VFS_STATUS_OK)
        break;
      else
      {
        delete f;
	f = 0;
      }
    }
    else
    {
      // rpath is an archive
      size_t idx = ArchiveCache->FindKey (rpath);
      // archive not in cache?
      if (idx == csArrayItemNotFound)
      {
        if ((Mode & VFS_FILE_MODE) != VFS_FILE_WRITE)
	{
          // does file rpath exist?
	  if (access (rpath, F_OK) != 0)
            continue;
	}

        idx = ArchiveCache->Length ();
        ArchiveCache->Push (new VfsArchive (rpath, object_reg));
      }

      f = new ArchiveFile (Mode, this, i, FileName,
      	ArchiveCache->Get (idx));
      if (f->GetStatus () == VFS_STATUS_OK)
        break;
      else
      {
        delete f;
	f = 0;
      }
    }
  }
  return f;
}

bool VfsNode::FindFile (const char *Suffix, char *RealPath,
  csArchive *&Archive) const
{
  // Look through all RPathV's for file or directory
  size_t i;
  for (i = 0; i < RPathV.Length (); i++)
  {
    char *rpath = (char *)RPathV [i];
    if (rpath [strlen (rpath) - 1] == CS_PATH_SEPARATOR)
    {
      // rpath is a directory
      size_t rl = strlen (rpath);
      memcpy (RealPath, rpath, rl);
      strcpy (RealPath + rl, Suffix);
      Archive = 0;
      if (access (RealPath, F_OK) == 0)
        return true;
    }
    else
    {
      // rpath is an archive
      size_t idx = ArchiveCache->FindKey (rpath);
      // archive not in cache?
      if (idx == csArrayItemNotFound)
      {
        // does file rpath exist?
        if (access (rpath, F_OK) != 0)
          continue;

        idx = ArchiveCache->Length ();
        ArchiveCache->Push (new VfsArchive (rpath, object_reg));
      }

      VfsArchive *a = ArchiveCache->Get (idx);
      a->UpdateTime ();
      if (a->FileExists (Suffix, 0))
      {
        Archive = a;
        strcpy (RealPath, Suffix);
        return true;
      }
    }
  }
  return false;
}

bool VfsNode::Delete (const char *Suffix)
{
  char fname [CS_MAXPATHLEN + 1];
  csArchive *a;
  if (!FindFile (Suffix, fname, a))
    return false;

  if (a)
    return a->DeleteFile (fname);
  else
    return (unlink (fname) == 0);
}

bool VfsNode::Exists (const char *Suffix)
{
  char fname [CS_MAXPATHLEN + 1];
  csArchive *a;
  return FindFile (Suffix, fname, a);
}

bool VfsNode::GetFileTime (const char *Suffix, csFileTime &oTime) const
{
  char fname [CS_MAXPATHLEN + 1];
  csArchive *a;
  if (!FindFile (Suffix, fname, a))
    return false;

  if (a)
  {
    void *e = a->FindName (fname);
    if (!e)
      return false;
    a->GetFileTime (e, oTime);
  }
  else
  {
    struct stat st;
    if (stat (fname, &st))
      return false;
    const time_t mtime = st.st_mtime;
    struct tm *curtm = localtime (&mtime);
    ASSIGN_FILETIME (oTime, *curtm);
  }
  return true;
}

bool VfsNode::SetFileTime (const char *Suffix, const csFileTime &iTime)
{
  char fname [CS_MAXPATHLEN + 1];
  csArchive *a;
  if (!FindFile (Suffix, fname, a))
    return false;

  if (a)
  {
    void *e = a->FindName (fname);
    if (!e)
      return false;
    a->SetFileTime (e, iTime);
  }
  else
  {
    // Not supported for now since there's no portable way of doing that - A.Z.
    return false;
  }
  return true;
}

bool VfsNode::GetFileSize (const char *Suffix, size_t &oSize)
{
  char fname [CS_MAXPATHLEN + 1];
  csArchive *a;
  if (!FindFile (Suffix, fname, a))
    return false;

  if (a)
  {
    void *e = a->FindName (fname);
    if (!e)
      return false;
    oSize = a->GetFileSize (e);
  }
  else
  {
    struct stat st;
    if (stat (fname, &st))
      return false;
    oSize = st.st_size;
  }
  return true;
}

// ----------------------------------------------------------- VfsVector --- //

int csVFS::VfsVector::Compare (VfsNode* const& Item1, VfsNode* const& Item2)
{
  return strcmp (Item1->VPath, Item2->VPath);
}

// --------------------------------------------------------------- csVFS --- //

SCF_IMPLEMENT_IBASE (csVFS)
  SCF_IMPLEMENTS_INTERFACE (iVFS)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csVFS::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csVFS)


csVFS::csVFS (iBase *iParent) : dirstack (8, 8)
{
  object_reg = 0;
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  cwd = new char [2];
  cwd [0] = VFS_PATH_SEPARATOR;
  cwd [1] = 0;
  basedir = 0;
  resdir = 0;
  appdir = 0;
  auto_name_counter = 0;
  CS_ASSERT (!ArchiveCache);
  ArchiveCache = new VfsArchiveCache ();
  mutex = csMutex::Create (true); // We need a recursive mutex.
}

csVFS::~csVFS ()
{
  delete [] cwd;
  delete [] basedir;
  delete [] resdir;
  delete [] appdir;
  CS_ASSERT (ArchiveCache);
  delete ArchiveCache;
  ArchiveCache = 0;
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

static void add_final_delimiter(csString& s)
{
  if (!s.IsEmpty() && s[s.Length() - 1] != CS_PATH_SEPARATOR)
    s << CS_PATH_SEPARATOR;
}

static char* alloc_normalized_path(char const* s)
{
  char* t = 0;
  if (s != 0)
  {
    csString c(s);
    add_final_delimiter(c);
    t = csStrNew(c);
  }
  return t;
}

static bool load_vfs_config(csConfigFile& cfg, char const* dir,
  csStringSet& seen, bool verbose)
{
  bool ok = false;
  if (dir != 0)
  {
    csString s(dir);
    add_final_delimiter(s);
    s << "vfs.cfg";
    if (seen.Contains(s))
      ok = true;
    else
    {
      seen.Request(s);
      bool const merge = !cfg.IsEmpty();
      ok = cfg.Load(s, 0, merge, false);
      if (ok && verbose)
      {
	char const* t = merge ? "merged" : "loaded";
	csPrintf("VFS_NOTIFY: %s configuration file: %s\n", t, s.GetData());
      }
    }
  }
  return ok;
}

bool csVFS::Initialize (iObjectRegistry* r)
{
  bool verbose = false;
  object_reg = r;
  basedir = alloc_normalized_path(csGetConfigPath());

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (object_reg, iVerbosityManager));
  if (verbosemgr) 
    verbose = verbosemgr->CheckFlag ("vfs");
  csRef<iCommandLineParser> cmdline =
    CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  if (cmdline)
  {
    resdir = alloc_normalized_path(cmdline->GetResourceDir());
    appdir = alloc_normalized_path(cmdline->GetAppDir());
  }
  
  // Order-sensitive: Mounts in first-loaded configuration file take precedence
  // over conflicting mounts in files loaded later.
  csStringSet seen;
  load_vfs_config(config, resdir,  seen, verbose);
  load_vfs_config(config, appdir,  seen, verbose);
  load_vfs_config(config, basedir, seen, verbose);

  return ReadConfig ();
}

bool csVFS::ReadConfig ()
{
  csRef<iConfigIterator> iterator (config.Enumerate ("VFS.Mount."));
  while (iterator->Next ())
    AddLink (iterator->GetKey (true), iterator->GetStr ());
  NodeList.Sort (NodeList.Compare);
  return true;
}

bool csVFS::AddLink (const char *VirtualPath, const char *RealPath)
{
  char *xp = _ExpandPath (VirtualPath, true);
  VfsNode *e = new VfsNode (xp, VirtualPath, object_reg);
  if (!e->AddRPath (RealPath, this))
  {
    delete e;
    return false;
  }

  NodeList.Push (e);
  return true;
}

char *csVFS::_ExpandPath (const char *Path, bool IsDir) const
{
  char outname [VFS_MAX_PATH_LEN + 1];
  size_t inp = 0, outp = 0, namelen = strlen (Path);

  // Copy 'Path' to 'outname', processing FS macros during the way
  while ((outp < sizeof (outname) - 1) && (inp < namelen))
  {
    // Get next path component
    char tmp [VFS_MAX_PATH_LEN + 1];
    size_t ptmp = 0;
    while ((inp < namelen) && (Path [inp] != VFS_PATH_SEPARATOR))
      tmp [ptmp++] = Path [inp++];
    tmp [ptmp] = 0;

    // If this is the very first component, append it to cwd
    if ((ptmp > 0) && (outp == 0))
    {
      strcpy (outname, GetCwd ());
      outp = strlen (outname);
    } /* endif */

    // Check if path component is ".."
    if (strcmp (tmp, "..") == 0)
    {
      // Skip back all '/' we encounter
      while ((outp > 0) && (outname [outp - 1] == VFS_PATH_SEPARATOR))
        outp--;
      // Skip back until we find another '/'
      while ((outp > 0) && (outname [outp - 1] != VFS_PATH_SEPARATOR))
        outp--;
    }
    // Check if path component is "."
    else if (strcmp (tmp, ".") == 0)
    {
      // do nothing
    }
    // Check if path component is "~"
    else if (strcmp (tmp, "~") == 0)
    {
      // Strip entire output path; start from scratch
      strcpy (outname, "/~/");
      outp = 3;
    }
    else
    {
      size_t sl = strlen (tmp);
      memcpy (&outname [outp], tmp, sl);
      outp += sl;
      if (IsDir || (inp < namelen))
        outname [outp++] = VFS_PATH_SEPARATOR;
    } /* endif */

    // Skip all '/' in source path
    while ((inp < namelen) && (Path [inp] == VFS_PATH_SEPARATOR))
      inp++;
  } /* endwhile */

  // Allocate a new string and return it
  char *ret = new char [outp + 1];
  memcpy (ret, outname, outp);
  ret [outp] = 0;
  return ret;
}

csPtr<iDataBuffer> csVFS::ExpandPath (const char *Path, bool IsDir) const
{
  char *xp = _ExpandPath (Path, IsDir);
  return csPtr<iDataBuffer> (new csDataBuffer (xp, strlen (xp) + 1));
}

VfsNode *csVFS::GetNode (const char *Path, char *NodePrefix,
  size_t NodePrefixSize) const
{
  size_t i, best_i = (size_t)-1;
  size_t best_l = 0, path_l = strlen (Path);
  for (i = 0; i < NodeList.Length (); i++)
  {
    VfsNode *node = (VfsNode *)NodeList [i];
    size_t vpath_l = strlen (node->VPath);
    if ((vpath_l <= path_l) && (strncmp (node->VPath, Path, vpath_l) == 0))
    {
      best_i = i;
      best_l = vpath_l;
      if (vpath_l == path_l)
        break;
    }
  }
  if (best_i != (size_t)-1)
  {
    if (NodePrefix != 0 && NodePrefixSize != 0)
    {
      size_t taillen = path_l - best_l + 1;
      if (taillen > NodePrefixSize)
        taillen = NodePrefixSize;
      memcpy (NodePrefix, Path + best_l, taillen);
      NodePrefix [taillen - 1] = 0;
    }
    return (VfsNode *)NodeList [best_i];
  }
  return 0;
}

bool csVFS::PreparePath (const char *Path, bool IsDir, VfsNode *&Node,
  char *Suffix, size_t SuffixSize) const
{
  Node = 0; *Suffix = 0;
  char *fname = _ExpandPath (Path, IsDir);
  if (!fname)
    return false;

  Node = GetNode (fname, Suffix, SuffixSize);
  delete [] fname;
  return (Node != 0);
}

bool csVFS::CheckIfMounted(char const* virtual_path) const
{
  bool ok = false;
  csScopedMutexLock lock(mutex);
  char* const s = _ExpandPath(virtual_path, true);
  if (s != 0)
  {
    ok = GetNode(s, 0, 0) != 0;
    delete[] s;
  }
  return ok;
}

bool csVFS::ChDir (const char *Path)
{
  csScopedMutexLock lock (mutex);
  // First, transform Path to absolute
  char *newwd = _ExpandPath (Path, true);
  if (!newwd)
    return false;
  delete[] cwd;
  cwd = newwd;
  ArchiveCache->CheckUp ();
  return true;
}

void csVFS::PushDir (char const* Path)
{
  { // Scope.
    csScopedMutexLock lock (mutex);
    dirstack.Push (cwd);
  }
  if (Path != 0)
    ChDir(Path);
}

bool csVFS::PopDir ()
{
  csScopedMutexLock lock (mutex);
  if (!dirstack.Length ())
    return false;
  char *olddir = (char *) dirstack.Pop ();
  bool retcode = ChDir (olddir);
  delete [] olddir;
  return retcode;
}

bool csVFS::Exists (const char *Path) const
{
  if (!Path)
    return false;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];

  csScopedMutexLock lock (mutex);
  PreparePath (Path, false, node, suffix, sizeof (suffix));
  bool exists = (node && (!suffix [0] || node->Exists (suffix)));

  ArchiveCache->CheckUp ();
  return exists;
}

csRef<iStringArray> csVFS::MountRoot (const char *Path)
{
  scfStringArray* outv = new scfStringArray;

  csScopedMutexLock lock (mutex);
  if (Path != 0)
  {
    csRef<iStringArray> roots = csFindSystemRoots();
    size_t i;
    size_t n = roots->Length ();
    for (i = 0 ; i < n ; i++)
    {
      char const* t = roots->Get(i);
      csString s(t);
      size_t const slen = s.Length();
      char c = '\0';

      csString vfs_dir;
      vfs_dir << Path << '/';
      for (size_t j = 0; j < slen; j++)
      {
        c = s.GetAt(j);
        if (c == '_' || c == '-' || isalnum(c))
	  vfs_dir << (char)tolower(c);
      }

      csString real_dir(s);
      if (slen > 0 && (c = real_dir.GetAt(slen - 1)) == '/' || c == '\\')
        real_dir.Truncate(slen - 1);
      real_dir << "$/";

      outv->Push (vfs_dir);
      Mount(vfs_dir, real_dir);
    }
  }

  csRef<iStringArray> v(outv);
  outv->DecRef ();
  return v;
}

csPtr<iStringArray> csVFS::FindFiles (const char *Path) const
{
  csScopedMutexLock lock (mutex);
  scfStringArray *fl = new scfStringArray;		// the output list

  csString news;
  if (Path != 0)
  {
    VfsNode *node;				// the node we are searching
    char suffix [VFS_MAX_PATH_LEN + 1];		// the suffix relative to node
    char mask [VFS_MAX_PATH_LEN + 1];		// the filename mask
    char XPath [VFS_MAX_PATH_LEN + 1];		// the expanded path

    PreparePath (Path, false, node, suffix, sizeof (suffix));

    // Now separate the mask from directory suffix
    size_t dirlen = strlen (suffix);
    while (dirlen && suffix [dirlen - 1] != VFS_PATH_SEPARATOR)
      dirlen--;
    strcpy (mask, suffix + dirlen);
    suffix [dirlen] = 0;
    if (!mask [0])
      strcpy (mask, "*");

    if (node)
    {
      strcpy (XPath, node->VPath);
      strcat (XPath, suffix);
    }
    else
    {
      char *s = _ExpandPath (Path, true);
      strcpy (XPath, s);
      delete [] s;
    }

    // first add all nodes that are located one level deeper
    // these are "directories" and will have a slash appended
    size_t sl = strlen (XPath);
    size_t i;
    for (i = 0; i < NodeList.Length (); i++)
    {
      VfsNode *node = (VfsNode *)NodeList [i];
      if ((memcmp (node->VPath, XPath, sl) == 0) && (node->VPath [sl]))
      {
        const char *pp = node->VPath + sl;
        while (*pp && *pp == VFS_PATH_SEPARATOR)
	  pp++;
        while (*pp && *pp != VFS_PATH_SEPARATOR)
          pp++;
        while (*pp && *pp == VFS_PATH_SEPARATOR)
          pp++;
	news.Clear();
	news.Append (node->VPath);
	news.Truncate (pp - node->VPath);
	if (fl->Find (news) == csArrayItemNotFound)
          fl->Push (news);
      }
    }

    // Now find all files in given directory node
    if (node)
      node->FindFiles (suffix, mask, fl);

    ArchiveCache->CheckUp ();
  }

  csPtr<iStringArray> v(fl);
  return v;
}

csPtr<iFile> csVFS::Open (const char *FileName, int Mode)
{
  if (!FileName)
    return 0;
  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  csScopedMutexLock lock (mutex);
  if (!PreparePath (FileName, false, node, suffix, sizeof (suffix)))
    return 0;

  iFile *f = node->Open (Mode, suffix);

  ArchiveCache->CheckUp ();
  return csPtr<iFile> (f);
}

bool csVFS::Sync ()
{
  csScopedMutexLock lock (mutex);
  ArchiveCache->FlushAll ();
  return true;
  //@@@return (ArchiveCache->Length () == 0);
}

csPtr<iDataBuffer> csVFS::ReadFile (const char *FileName, bool nullterm)
{
  csScopedMutexLock lock (mutex);
  csRef<iFile> F (Open (FileName, VFS_FILE_READ));
  if (!F)
    return 0;

  size_t Size = F->GetSize ();
  csRef<iDataBuffer> data (F->GetAllData (nullterm));
  if (data)
  {
    return csPtr<iDataBuffer> (data);
  }

  char *buff = new char [Size + 1];
  if (!buff)
    return 0;

  // Make the file zero-terminated in the case we'll use it as an ASCIIZ string
  buff [Size] = 0;
  if (F->Read (buff, Size) != Size)
  {
    delete [] buff;
    return 0;
  }

  return csPtr<iDataBuffer> (new csDataBuffer (buff, Size));
}

bool csVFS::WriteFile (const char *FileName, const char *Data, size_t Size)
{
  csScopedMutexLock lock (mutex);
  csRef<iFile> F (Open (FileName, VFS_FILE_WRITE));
  if (!F)
    return false;

  bool success = (F->Write (Data, Size) == Size);
  return success;
}

bool csVFS::DeleteFile (const char *FileName)
{
  if (!FileName)
    return false;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  csScopedMutexLock lock (mutex);
  if (!PreparePath (FileName, false, node, suffix, sizeof (suffix)))
    return false;

  bool rc = node->Delete (suffix);

  ArchiveCache->CheckUp ();
  return rc;
}

bool csVFS::Mount (const char *VirtualPath, const char *RealPath)
{
  csScopedMutexLock lock (mutex);
  ArchiveCache->CheckUp ();

  if (!VirtualPath || !RealPath)
    return false;
#ifdef VFS_DEBUG
  printf("VFS: Mounted dir: Vpath %s, Rpath %s",VirtualPath, RealPath);
#endif
  VfsNode *node;
  char suffix [2];
  if (!PreparePath (VirtualPath, true, node, suffix, sizeof (suffix))
   || suffix [0])
  {
    char *xp = _ExpandPath (VirtualPath, true);
    node = new VfsNode (xp, VirtualPath, object_reg);
    NodeList.Push (node);
  }

  node->AddRPath (RealPath, this);
  if (node->RPathV.Length () == 0)
  {
    size_t idx = NodeList.Find (node);
    if (idx != csArrayItemNotFound)
      NodeList.DeleteIndex (idx);
    return false;
  }

  return true;
}

bool csVFS::Unmount (const char *VirtualPath, const char *RealPath)
{
  csScopedMutexLock lock (mutex);
  ArchiveCache->CheckUp ();

  if (!VirtualPath)
    return false;

  VfsNode *node;
  char suffix [2];
  if (!PreparePath (VirtualPath, true, node, suffix, sizeof (suffix))
   || suffix [0])
    return false;

  if (!node->RemoveRPath (RealPath))
    return false;

  if (node->RPathV.Length () == 0)
  {
    csString s("VFS.Mount.");
    s+=node->ConfigKey;
    config.DeleteKey (s);
    size_t idx = NodeList.Find (node);
    if (idx != csArrayItemNotFound)
      NodeList.DeleteIndex (idx);
  }

  return true;
}

bool csVFS::SaveMounts (const char *FileName)
{
  csScopedMutexLock lock (mutex);
  size_t i;
  for (i = 0; i < NodeList.Length (); i++)
  {
    VfsNode *node = (VfsNode *)NodeList.Get (i);
    size_t j;
    size_t sl = 0;
    for (j = 0; j < node->UPathV.Length (); j++)
      sl += strlen ((char *)node->UPathV.Get (j)) + 1;

    char *tmp = new char[sl + 1];
    sl = 0;
    for (j = 0; j < node->UPathV.Length (); j++)
    {
      char *rp = (char *)node->UPathV.Get (j);
      size_t rpl = strlen (rp);
      memcpy (tmp + sl, rp, rpl);
      if (j < node->UPathV.Length () - 1)
      {
        tmp [sl + rpl] = ',';
        sl++;
        tmp [sl + rpl] = ' ';
      }
      else
        tmp [sl + rpl] = 0;
      sl += rpl + 1;
    }
    csString s("VFS.Mount.");
    s+=node->ConfigKey;
    config.SetStr (s, tmp);
    delete [] tmp;
  }
  return config.Save (FileName);
}

bool csVFS::LoadMountsFromFile (iConfigFile* file)
{
  bool success = true;

  csRef<iConfigIterator> iter = file->Enumerate ("VFS.Mount.");
  while (iter->Next ())
  {
    const char *rpath = iter->GetKey (true);
    const char *vpath = iter->GetStr ();
    if (!Mount (rpath, vpath)) {
      csPrintfErr("VFS_WARNING: cannot mount \"%s\" to \"%s\"\n", rpath,vpath);
      success = false;
    }
  }

  return success;
}

// Transform a path so that every \ or / is replaced with $/.
// If 'add_end' is true there will also be a $/ at the end if there
// is not already one there.
// The result of this function must be deleted with delete[].
static char* TransformPath (const char* path, bool add_end)
{
  // The length we allocate below is enough in all cases.
  char* npath = new char [strlen (path)*2+2+1];
  char* np = npath;
  bool lastispath = false;
  while (*path)
  {
    if (*path == '$' && (*(path+1) == '/' || *(path+1) == '.'))
    {
      *np++ = '$';
      *np++ = *(path+1);
      path++;
      if (*(path+1) == '/')
        lastispath = true;
    }
    else if (*path == '/' || *path=='\\')
    {
      *np++ = '$';
      *np++ = '/';
      lastispath = true;
    }
    else if (*path == '.')
    {
      *np++ = '$';
      *np++ = '.';
      lastispath = false;
    }
    else
    {
      *np++ = *path;
      lastispath = false;
    }
    path++;
  }
  if (add_end && !lastispath)
  {
    *np++ = '$';
    *np++ = '/';
  }
  *np++ = 0;
  return npath;
}

static csString compose_vfs_path(char const* dir, char const* file)
{
  csString path(dir);
  size_t const n = path.Length();
  if (n > 0 && path[n - 1] != VFS_PATH_SEPARATOR)
    path << VFS_PATH_SEPARATOR;
  path << file;
  return path;
}

bool csVFS::TryChDirAuto(char const* dir, char const* filename)
{
  bool ok = false;
  if (CheckIfMounted(dir))
  {
    if (filename == 0)
      ok = true;
    else
    {
      csString testpath = compose_vfs_path(dir, filename);
      ok = Exists(testpath);
    }
  }
  return ok && ChDir(dir);
}

bool csVFS::ChDirAuto (const char* path, const csStringArray* paths,
	const char* vfspath, const char* filename)
{
  // If the VFS path is valid we can use that.
  if (TryChDirAuto(path, filename))
    return true;

  // Now try to see if we can get it from one of the paths.
  if (paths)
  {
    for (size_t i = 0; i < paths->Length(); i++)
    {
      csString testpath = compose_vfs_path(paths->Get(i), path);
      if (TryChDirAuto(testpath, filename))
	return true;
    }
  }

  // First check if it is a zip file.
  size_t pathlen = strlen (path);
  bool is_zip = pathlen >= 5 && !strcasecmp (path+pathlen-4, ".zip");
  char* npath = TransformPath (path, !is_zip);

  // See if we have to generate a unique VFS name.
  csString tryvfspath;
  if (vfspath)
    tryvfspath = vfspath;
  else
  {
    tryvfspath.Format ("/tmp/__automount%d__", auto_name_counter);
    auto_name_counter++;
  }
  
  bool rc = Mount (tryvfspath, npath);
  delete[] npath;
  if (!rc)
    return false;
  return ChDir (tryvfspath);
}

bool csVFS::GetFileTime (const char *FileName, csFileTime &oTime) const
{
  if (!FileName)
    return false;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  csScopedMutexLock lock (mutex);
  PreparePath (FileName, false, node, suffix, sizeof (suffix));

  bool success = node ? node->GetFileTime (suffix, oTime) : false;

  ArchiveCache->CheckUp ();
  return success;
}

bool csVFS::SetFileTime (const char *FileName, const csFileTime &iTime)
{
  if (!FileName)
    return false;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  csScopedMutexLock lock (mutex);
  PreparePath (FileName, false, node, suffix, sizeof (suffix));

  bool success = node ? node->SetFileTime (suffix, iTime) : false;

  ArchiveCache->CheckUp ();
  return success;
}

bool csVFS::GetFileSize (const char *FileName, size_t &oSize)
{
  if (!FileName)
    return false;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  csScopedMutexLock lock (mutex);
  PreparePath (FileName, false, node, suffix, sizeof (suffix));

  bool success = node ? node->GetFileSize (suffix, oSize) : false;

  ArchiveCache->CheckUp ();
  return success;
}

csPtr<iDataBuffer> csVFS::GetRealPath (const char *FileName)
{
  if (!FileName)
    return 0;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  csScopedMutexLock lock (mutex);
  PreparePath (FileName, false, node, suffix, sizeof (suffix));
  if (!node)
    return 0;

  bool ok = false;
  char path [CS_MAXPATHLEN + 1];
  size_t i;
  for (i = 0; !ok && i < node->RPathV.Length (); i++)
  {
    const char *rpath = node->RPathV.Get (i);
    sprintf(path, "%s%s", rpath, suffix);
    strcat (strcpy (path, rpath), suffix);
    ok = access (path, F_OK) == 0;
  }

  if (!ok)
  {
    CS_ASSERT(node->RPathV.Length() != 0);
    char const* defpath = node->RPathV.Get(0);
    CS_ASSERT(defpath != 0);
    size_t const len = strlen(defpath);
    if (len > 0 && defpath[len - 1] != VFS_PATH_SEPARATOR)
      sprintf(path, "%s%c%s", defpath, VFS_PATH_SEPARATOR, suffix);
    else
      sprintf(path, "%s%s", defpath, suffix);
  }

  return csPtr<iDataBuffer> (
  	new csDataBuffer (csStrNew (path), strlen (path) + 1));
}

csRef<iStringArray> csVFS::GetMounts ()
{
  scfStringArray* mounts = new scfStringArray;
  for (size_t i=0; i<NodeList.Length (); i++)
  {
    mounts->Push (NodeList[i]->VPath);
  }
  
  csRef<iStringArray> m (mounts);
  mounts->DecRef ();
  return m;
}

csRef<iStringArray> csVFS::GetRealMountPaths (const char *VirtualPath)
{
  if (!VirtualPath)
    return 0;

  scfStringArray* rmounts = new scfStringArray;

  VfsNode *node;
  char suffix [2];
  if (PreparePath (VirtualPath, true, node, suffix, sizeof (suffix))
    && !suffix [0])
  {
    for (size_t i=0; i<node->RPathV.Length (); i++)
      rmounts->Push (node->RPathV[i]);
  }

  csRef<iStringArray> r (rmounts);
  rmounts->DecRef ();
  return r;
}
