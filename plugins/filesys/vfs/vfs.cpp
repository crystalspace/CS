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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CS_SYSDEF_PROVIDE_PATH
#define CS_SYSDEF_PROVIDE_DIR
#define CS_SYSDEF_PROVIDE_ACCESS
#define CS_SYSDEF_PROVIDE_MKDIR
#define CS_SYSDEF_PROVIDE_UNLINK
#include "cssysdef.h"
#include "vfs.h"
#include "csutil/archive.h"
#include "csutil/util.h"
#include "csutil/scfstrv.h"
#include "csutil/databuf.h"
#include "csutil/csstring.h"
#include "isys/system.h"

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
  // constructor
  DiskFile(int Mode, VfsNode* ParentNode, int RIndex, const char* NameSuffix);
  // set Error according to errno
  void CheckError ();

public:
  SCF_DECLARE_IBASE;

  // destructor
  virtual ~DiskFile ();
  // read a block of data
  virtual size_t Read (char *Data, size_t DataSize);
  // write a block of data
  virtual size_t Write (const char *Data, size_t DataSize);
  // check for EOF
  virtual bool AtEOF ();
  /// Query current file pointer
  virtual size_t GetPos ();
  /// Clear file error after queriyng status
  virtual int GetStatus ();
private:
  // Create a directory or a series of directories starting from PathBase
  void MakeDir (const char *PathBase, const char *PathSuffix);
};

// used by ArchiveFile
class VfsArchive;

// This is a version of csFile which "lives" in archives
class ArchiveFile : public csFile
{
  friend class VfsNode;

  // parent archive
  VfsArchive *Archive;
  // The file handle
  void *fh;
  // file data (for read mode)
  char *data;
  // current data pointer
  size_t fpos;
  // constructor
  ArchiveFile (int Mode, VfsNode *ParentNode, int RIndex,
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
  /// Query current file pointer
  virtual size_t GetPos ();
  /// Get all the data at once
  virtual iDataBuffer *GetAllData ();
};

class VfsArchive : public csArchive
{
public:
  // Last time this archive was used
  long LastUseTime;
  // Number of references (open files) to this archive
  int RefCount;
  // number of open for writing files in this archive
  int Writing;
  // The system driver
  iSystem *System;

  void UpdateTime ()
  {
    LastUseTime = System->GetTime ();
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
      (System->GetTime () - LastUseTime > VFS_KEEP_UNUSED_ARCHIVE_TIME);
  }
  VfsArchive (const char *filename, iSystem *iSys) : csArchive (filename)
  {
    RefCount = 0;
    Writing = 0;
    System = iSys;
    UpdateTime (); // OpenStep compiler requires having seen this already.
#ifdef VFS_DEBUG
    printf ("VFS: opening archive \"%s\"\n", filename);
#endif
  }
  virtual ~VfsArchive ()
  {
#ifdef VFS_DEBUG
    printf ("VFS: archive \"%s\" closing (writing=%d)\n", GetName (), Writing);
#endif
    Flush ();
#ifdef VFS_DEBUG
    printf ("VFS: archive \"%s\" closed\n", GetName ());
#endif
  }
};

class VfsArchiveCache : public csVector
{
public:
  VfsArchiveCache () : csVector (8, 8)
  {
  }
  virtual ~VfsArchiveCache ()
  {
    DeleteAll ();
  }
  int CompareKey (csSome Item, csConstSome Key, int Mode) const
  {
    (void)Mode;
    return strcmp (((VfsArchive *)Item)->GetName (), (char *)Key);
  }
  virtual bool FreeItem (csSome Item)
  {
    delete (VfsArchive *)Item;
    return true;
  }
  VfsArchive *Get (int iIndex)
  {
    return (VfsArchive *)csVector::Get (iIndex);
  }
  void CheckUp ()
  {
    for (int i = Length () - 1; i >= 0; i--)
    {
      VfsArchive *a = Get (i);
      if (a->CheckUp ())
        Delete (i);
    }
  }
};

// Private structure used to keep a "node" in virtual filesystem tree.
// The program can be made even fancier if we use a object for each
// "real" path (i.e. each VfsNode will contain an csVector of real-world
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
  csStrVector RPathV;
  // The array of unexpanded real paths
  csStrVector UPathV;
  // The system interface
  iSystem *System;

  // Initialize the object
  VfsNode (char *iPath, const char *iConfigKey, iSystem *iSys);
  // Destroy the object
  virtual ~VfsNode ();

  // Parse a directory link directive and fill RPathV
  bool AddRPath (const char *RealPath, csVFS *Parent);
  // Remove a real-world path
  bool RemoveRPath (const char *RealPath);
  // Find all files in a subpath
  void FindFiles (const char *Suffix, const char *Mask, iStrVector *FileList);
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
  int Expand (csVFS *Parent, char *dst, char *src, int size);
  // Find a file either on disk or in archive - in this node only
  bool FindFile (const char *Suffix, char *RealPath, csArchive *&) const;
};

// The global archive cache
static VfsArchiveCache *ArchiveCache = NULL;

// -------------------------------------------------------------- csFile --- //

csFile::csFile (int Mode, VfsNode *ParentNode, int RIndex,
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

iDataBuffer *csFile::GetAllData ()
{
  return NULL;
}

// ------------------------------------------------------------ DiskFile --- //

SCF_IMPLEMENT_IBASE (DiskFile)
  SCF_IMPLEMENTS_INTERFACE (iFile)
SCF_IMPLEMENT_IBASE_END

#ifndef O_BINARY
#  define O_BINARY 0
#endif

#define VFS_READ_MODE	(O_RDONLY | O_BINARY)
#define VFS_WRITE_MODE	(O_CREAT | O_TRUNC | O_WRONLY | O_BINARY)

DiskFile::DiskFile (int Mode, VfsNode *ParentNode, int RIndex,
  const char *NameSuffix) : csFile (Mode, ParentNode, RIndex, NameSuffix)
{
  SCF_CONSTRUCT_IBASE (NULL);
  char *rp = (char *)Node->RPathV [Index];
  size_t rpl = strlen (rp);
  size_t nsl = strlen (NameSuffix);
  char *fName = new char [rpl + nsl + 1];
  memcpy (fName, rp, rpl);
  memcpy (fName + rpl, NameSuffix, nsl + 1);

  // Convert all VFS_PATH_SEPARATOR's in filename into PATH_SEPARATOR's
  for (size_t n = 0; n < nsl; n++)
    if (fName [rpl + n] == VFS_PATH_SEPARATOR)
      fName [rpl + n] = PATH_SEPARATOR;

  for (int t = 1; t <= 2; t++)
  {
#ifdef VFS_DEBUG
    printf ("VFS: Trying to open disk file \"%s\"\n", fName);
#endif
    file = fopen (fName, (Mode & VFS_FILE_MODE) == VFS_FILE_WRITE ? "wb":"rb");
    if (file || (t != 1))
      break;
    char *lastps = strrchr (NameSuffix, VFS_PATH_SEPARATOR);
    if (!lastps)
      break;

    *lastps = 0;
    MakeDir (rp, NameSuffix);
    *lastps = VFS_PATH_SEPARATOR;
  }

  delete [] fName;

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
    if (fseek (file, 0, SEEK_SET))
      CheckError ();
  }
#ifdef VFS_DEBUG
  if (file)
    printf ("VFS: Successfully opened, handle = %d\n", fileno (file));
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
}

void DiskFile::MakeDir (const char *PathBase, const char *PathSuffix)
{
  size_t pbl = strlen (PathBase);
  size_t pl = pbl + strlen (PathSuffix) + 1;
  char *path = new char [pl];
  char *cur = path + pbl;
  char *prev = NULL;

  strcpy (path, PathBase);
  strcpy (cur, PathSuffix);

  // Convert all VFS_PATH_SEPARATOR's in path into PATH_SEPARATOR's
  for (size_t n = 0; n < pl; n++)
    if (path [n] == VFS_PATH_SEPARATOR)
      path [n] = PATH_SEPARATOR;

  while (cur != prev)
  {
    prev = cur;

    char oldchar = *cur;
    *cur = 0;
#ifdef VFS_DEBUG
    printf ("VFS: Trying to create directory \"%s\"\n", path);
#endif
    MKDIR (path);
    *cur = oldchar;
    if (*cur)
      cur++;

    while (*cur && (*cur != PATH_SEPARATOR))
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
  size_t rc = fread (Data, 1, DataSize, file);
  if (rc < DataSize)
    CheckError ();
  return rc;
}

size_t DiskFile::Write (const char *Data, size_t DataSize)
{
  size_t rc = fwrite (Data, 1, DataSize, file);
  if (rc < DataSize)
    CheckError ();
  return rc;
}

bool DiskFile::AtEOF ()
{
  return feof (file);
}

size_t DiskFile::GetPos ()
{
  return ftell (file);
}

// --------------------------------------------------------- ArchiveFile --- //

SCF_IMPLEMENT_IBASE (ArchiveFile)
  SCF_IMPLEMENTS_INTERFACE (iFile)
SCF_IMPLEMENT_IBASE_END

ArchiveFile::ArchiveFile (int Mode, VfsNode *ParentNode, int RIndex,
  const char *NameSuffix, VfsArchive *ParentArchive) :
  csFile (Mode, ParentNode, RIndex, NameSuffix)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Archive = ParentArchive;
  Error = VFS_STATUS_OTHER;
  Size = 0;
  fh = NULL;
  data = NULL;
  fpos = 0;

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
      Error = VFS_STATUS_OK;
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

  delete [] data;
  if (fh)
    Archive->Writing--;
  Archive->DecRef ();
}

size_t ArchiveFile::Read (char *Data, size_t DataSize)
{
  size_t sz = DataSize;
  if (fpos + sz > Size)
    sz = Size - fpos;
  memcpy (Data, data + fpos, sz);
  fpos += sz;
  return sz;
}

size_t ArchiveFile::Write (const char *Data, size_t DataSize)
{
  if (!Archive->Write (fh, Data, DataSize))
  {
    Error = VFS_STATUS_NOSPACE;
    return 0;
  }
  return DataSize;
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

iDataBuffer *ArchiveFile::GetAllData ()
{
  iDataBuffer *db = new csDataBuffer (data, Size);
  data = NULL;
  fpos = 0;
  Size = 0;
  return db;
}

// ------------------------------------------------------------- VfsNode --- //

VfsNode::VfsNode (char *iPath, const char *iConfigKey, iSystem *iSys)
{
  VPath = iPath;
  ConfigKey = csStrNew (iConfigKey);
  System = iSys;
}

VfsNode::~VfsNode ()
{
  delete [] ConfigKey;
  delete [] VPath;
}

bool VfsNode::AddRPath (const char *RealPath, csVFS *Parent)
{
  bool rc = false;
  // Split rpath into several, separated by commas
  int rpl = strlen (RealPath);
  char *cur, *src;
  char *oldsrc = src = csStrNew (RealPath);
  for (cur = src; rpl >= 0; cur++, rpl--)
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
      UPathV.Push (csStrNew (src));

      // Now parse and expand this path
      char rpath [MAXPATHLEN + 1];
      Expand (Parent, rpath, src, MAXPATHLEN);
      RPathV.Push (csStrNew (rpath));
      src = cur + 1;
    } /* endif */

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

  for (int i = 0; i < RPathV.Length (); i++)
    if (strcmp ((char *)RPathV.Get (i), RealPath) == 0)
    {
      RPathV.Delete (i);
      UPathV.Delete (i);
      return true;
    } /* endif */

  return false;
}

int VfsNode::Expand (csVFS *Parent, char *dst, char *src, int size)
{
  char *org = dst;
  while (*src && ((dst - org) < size))
  {
    // Is this a variable reference?
    if (*src == '$')
    {
      // Parse the name of variable
      src++;
      char *var = src;
      char one_letter_varname [2];
      if (*src == '(')
      {
        // Parse until the end of variable, skipping pairs of '(' and ')'
        int level = 1;
        src++; var++;
        while (level && *src)
        {
          if (*src == '(')
            level++;
          else if (*src == ')')
            level--;
          else
            src++;
        } /* endwhile */
        // Replace closing parenthesis with \0
        *src++ = 0;
      }
      else
      {
        var = one_letter_varname;
        var [0] = *src++;
        var [1] = 0;
      }

      char *alternative = strchr (var, ':');
      if (alternative)
        *alternative++ = 0;
      else
        alternative = strchr (var, 0);

      const char *value = GetValue (Parent, var);
      if (!value)
      {
        if (*alternative)
          dst += Expand (Parent, dst, alternative, size - (dst - org));
      }
      else
      {
        strcpy (dst, value);
        dst = strchr (dst, 0);
      }
    } /* endif */
    else
      *dst++ = *src++;
  } /* endif */
  *dst = 0;
  return dst - org;
}

const char *VfsNode::GetValue (csVFS *Parent, const char *VarName)
{
  // Look in environment first
  const char *value = getenv (VarName);
  if (value)
    return value;

  iConfigFile *Config = &(Parent->config);

  // Now look in "VFS.Solaris" section, for example
  csString Keyname;
  Keyname << "VFS." CS_PLATFORM_NAME "." << VarName;
  value = Config->GetStr (Keyname, NULL);
  if (value)
    return value;

  // Now look in "VFS.Alias" section for alias section name
  const char *alias = Config->GetStr ("VFS.Alias." CS_PLATFORM_NAME, NULL);
  // If there is one, look into that section too
  if (alias) {
    Keyname.Clear();
    Keyname << alias << '.' << VarName;
    value = Config->GetStr (Keyname, NULL);
  }
  if (value)
    return value;

  // Handle predefined variables here so that user
  // can override them in config file or environment
  static char path_separator [] = {VFS_PATH_SEPARATOR, 0};
  if (strcmp (VarName, path_separator) == 0)	// Path separator variable?
  {
    static char path_sep [] = {PATH_SEPARATOR, 0};
    return path_sep;
  }

  if (strcmp (VarName, "@") == 0) // Installation directory?
    return Parent->basedir;

  return NULL;
}

void VfsNode::FindFiles (const char *Suffix, const char *Mask,
  iStrVector *FileList)
{
  // Look through all RPathV's for file or directory
  for (int i = 0; i < RPathV.Length (); i++)
  {
    char *rpath = (char *)RPathV [i];
    size_t rpl = strlen (rpath);
    if (rpath [rpl - 1] == PATH_SEPARATOR)
    {
      // rpath is a directory
      DIR *dh;
      struct dirent *de;

      char tpath [MAXPATHLEN + 1];
      memcpy (tpath, rpath, rpl);
      strcpy (tpath + rpl, Suffix);
      rpl = strlen (tpath);
      if ((rpl > 1)
#if defined (OS_OS2) || defined (OS_DOS) || defined (OS_WIN32)
       && ((rpl > 3) || (tpath [1] != ':'))
#endif
       && ((tpath [rpl - 1] == '/') || (tpath [rpl - 1] == PATH_SEPARATOR)))
        tpath [rpl - 1] = 0;		// remove trailing PATH_SEPARATOR

      if ((dh = opendir (tpath)) == NULL)
        continue;
      while ((de = readdir (dh)) != NULL)
      {
        if ((strcmp (de->d_name, ".") == 0)
         || (strcmp (de->d_name, "..") == 0))
          continue;

        if (!csGlobMatches (de->d_name, Mask))
          continue;

        bool append_slash = isdir (tpath, de);
        char *vpath = new char [strlen (VPath) + strlen (Suffix) +
	  strlen (de->d_name) + (append_slash ? 2 : 1)];
	strcpy (vpath, VPath);
	strcat (vpath, Suffix);
	strcat (vpath, de->d_name);
	if (append_slash)
	{
	  size_t sl = strlen (vpath);
	  vpath [sl++] = VFS_PATH_SEPARATOR;
	  vpath [sl] = 0;
	}

	FileList->Push (vpath);
      } /* endwhile */
      closedir (dh);
    }
    else
    {
      // rpath is an archive
      int idx = ArchiveCache->FindKey (rpath);
      // archive not in cache?
      if (idx < 0)
      {
        // does file rpath exist?
        if (access (rpath, F_OK) != 0)
          continue;

        idx = ArchiveCache->Length ();
        ArchiveCache->Push (new VfsArchive (rpath, System));
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
	  while (cur < fnl)
	  {
	    if (fname [cur] == VFS_PATH_SEPARATOR)
	      break;
	    cur++;
	  }
	  if (cur < fnl)
	    cur++;
          size_t vpl = strlen (VPath);
          char *vpath = new char [vpl + cur + 1];
          memcpy (vpath, VPath, vpl);
          memcpy (vpath + vpl, fname, cur);
	  vpath [vpl + cur] = 0;
	  if (FileList->Find (vpath) == -1)
            FileList->Push (vpath);
	  else
	    delete [] vpath;
        }
      }
    }
  }
}

iFile *VfsNode::Open (int Mode, const char *FileName)
{
  csFile *f = NULL;

  // Look through all RPathV's for file or directory
  for (int i = 0; i < RPathV.Length (); i++)
  {
    char *rpath = (char *)RPathV [i];
    if (rpath [strlen (rpath) - 1] == PATH_SEPARATOR)
    {
      // rpath is a directory
      f = new DiskFile (Mode, this, i, FileName);
      if (f->GetStatus () == VFS_STATUS_OK)
        break;
      else
      {
        delete f;
	f = NULL;
      }
    }
    else
    {
      // rpath is an archive
      int idx = ArchiveCache->FindKey (rpath);
      // archive not in cache?
      if (idx < 0)
      {
        if ((Mode & VFS_FILE_MODE) != VFS_FILE_WRITE)
	{
          // does file rpath exist?
	  if (access (rpath, F_OK) != 0)
            continue;
	}

        idx = ArchiveCache->Length ();
        ArchiveCache->Push (new VfsArchive (rpath, System));
      }

      f = new ArchiveFile (Mode, this, i, FileName, ArchiveCache->Get (idx));
      if (f->GetStatus () == VFS_STATUS_OK)
        break;
      else
      {
        delete f;
	f = NULL;
      }
    }
  }
  return f;
}

bool VfsNode::FindFile (const char *Suffix, char *RealPath,
  csArchive *&Archive) const
{
  // Look through all RPathV's for file or directory
  for (int i = 0; i < RPathV.Length (); i++)
  {
    char *rpath = (char *)RPathV [i];
    if (rpath [strlen (rpath) - 1] == PATH_SEPARATOR)
    {
      // rpath is a directory
      size_t rl = strlen (rpath);
      memcpy (RealPath, rpath, rl);
      strcpy (RealPath + rl, Suffix);
      Archive = NULL;
      if (access (RealPath, F_OK) == 0)
        return true;
    }
    else
    {
      // rpath is an archive
      int idx = ArchiveCache->FindKey (rpath);
      // archive not in cache?
      if (idx < 0)
      {
        // does file rpath exist?
        if (access (rpath, F_OK) != 0)
          continue;

        idx = ArchiveCache->Length ();
        ArchiveCache->Push (new VfsArchive (rpath, System));
      }

      VfsArchive *a = ArchiveCache->Get (idx);
      a->UpdateTime ();
      if (a->FileExists (Suffix, NULL))
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
  char fname [MAXPATHLEN + 1];
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
  char fname [MAXPATHLEN + 1];
  csArchive *a;
  return FindFile (Suffix, fname, a);
}

bool VfsNode::GetFileTime (const char *Suffix, csFileTime &oTime) const
{
  char fname [MAXPATHLEN + 1];
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
  char fname [MAXPATHLEN + 1];
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
  char fname [MAXPATHLEN + 1];
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

csVFS::VfsVector::VfsVector () : csVector (16, 16)
{
}

csVFS::VfsVector::~VfsVector ()
{
  DeleteAll ();
}

bool csVFS::VfsVector::FreeItem (csSome Item)
{
  delete (VfsNode *)Item;
  return true;
}

int csVFS::VfsVector::Compare (csSome Item1, csSome Item2, int Mode) const
{
  (void)Mode;
  return strcmp (((VfsNode *)Item1)->VPath, ((VfsNode *)Item2)->VPath);
}

int csVFS::VfsVector::CompareKey (csSome Item, csConstSome Key, int Mode) const
{
  (void)Mode;
  return strcmp (((VfsNode *)Item)->VPath, (char *)Key);
}

// --------------------------------------------------------------- csVFS --- //

SCF_IMPLEMENT_IBASE (csVFS)
  SCF_IMPLEMENTS_INTERFACE (iVFS)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csVFS::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csVFS)

SCF_EXPORT_CLASS_TABLE (vfs)
  SCF_EXPORT_CLASS (csVFS, "crystalspace.kernel.vfs",
    "Crystal Space Virtual File System")
SCF_EXPORT_CLASS_TABLE_END

csVFS::csVFS (iBase *iParent) : dirstack (8, 8)
{
  System = NULL;
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  cwd = new char [2];
  cwd [0] = VFS_PATH_SEPARATOR;
  cwd [1] = 0;
  cnode = NULL;
  cnsufx [0] = 0;
  basedir = NULL;
  CS_ASSERT (!ArchiveCache);
  ArchiveCache = new VfsArchiveCache ();
}

csVFS::~csVFS ()
{
  delete [] cwd;
  delete [] basedir;
  CS_ASSERT (ArchiveCache);
  delete ArchiveCache;
  ArchiveCache = NULL;
}

bool csVFS::Initialize (iSystem *iSys)
{
  System = iSys;
  char vfsconfigpath [MAXPATHLEN + 1];
  iSys->GetInstallPath (vfsconfigpath, sizeof (vfsconfigpath));
  basedir = csStrNew (vfsconfigpath);
  strcat (vfsconfigpath, "vfs.cfg");
  config.Load (vfsconfigpath);
  return ReadConfig ();
}

bool csVFS::ReadConfig ()
{
  iConfigIterator *iterator = config.Enumerate ("VFS.Mount.");
  while (iterator->Next ())
    AddLink (iterator->GetKey (true), iterator->GetStr ());
  iterator->DecRef ();
  NodeList.QuickSort (0);
  return true;
}

bool csVFS::AddLink (const char *VirtualPath, const char *RealPath)
{
  char *xp = _ExpandPath (VirtualPath, true);
  VfsNode *e = new VfsNode (xp, VirtualPath, System);
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
      int sl = strlen (tmp);
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

iDataBuffer *csVFS::ExpandPath (const char *Path, bool IsDir) const
{
  char *xp = _ExpandPath (Path, IsDir);
  return new csDataBuffer (xp, strlen (xp) + 1);
}

VfsNode *csVFS::GetNode (const char *Path, char *NodePrefix,
  size_t NodePrefixSize) const
{
  int best_i = -1;
  size_t best_l = 0, path_l = strlen (Path);
  for (int i = 0; i < NodeList.Length (); i++)
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
  if (best_i >= 0)
  {
    if (NodePrefixSize)
    {
      size_t taillen = path_l - best_l + 1;
      if (taillen > NodePrefixSize)
        taillen = NodePrefixSize;
      memcpy (NodePrefix, Path + best_l, taillen);
      NodePrefix [taillen - 1] = 0;
    }
    return (VfsNode *)NodeList [best_i];
  }
  return NULL;
}

bool csVFS::PreparePath (const char *Path, bool IsDir, VfsNode *&Node,
  char *Suffix, size_t SuffixSize) const
{
  Node = NULL; *Suffix = 0;
  char *fname = _ExpandPath (Path, IsDir);
  if (!fname)
    return false;

  Node = GetNode (fname, Suffix, SuffixSize);
  delete [] fname;
  return (Node != NULL);
}

bool csVFS::ChDir (const char *Path)
{
  // First, transform Path to absolute
  char *newwd = _ExpandPath (Path, true);
  if (!newwd)
    return false;

  // Find the current directory node and directory suffix
  cnode = GetNode (newwd, cnsufx, sizeof (cnsufx));

  delete [] cwd;
  cwd = newwd;

  ArchiveCache->CheckUp ();
  return true;
}

void csVFS::PushDir ()
{
  dirstack.Push (csStrNew (cwd));
}

bool csVFS::PopDir ()
{
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

  PreparePath (Path, false, node, suffix, sizeof (suffix));
  bool exists = (node && (!suffix [0] || node->Exists (suffix)));

  ArchiveCache->CheckUp ();
  return exists;
}

iStrVector *csVFS::FindFiles (const char *Path) const
{
  if (!Path)
    return NULL;

  VfsNode *node;				// the node we are searching
  char suffix [VFS_MAX_PATH_LEN + 1];		// the suffix relative to node
  char mask [VFS_MAX_PATH_LEN + 1];		// the filename mask
  char XPath [VFS_MAX_PATH_LEN + 1];		// the expanded path
  scfStrVector *fl = new scfStrVector (16, 16);// the output list

  PreparePath (Path, false, node, suffix, sizeof (suffix));

  // Now separate the mask from directory suffix
  int dirlen = strlen (suffix);
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
  for (int i = 0; i < NodeList.Length (); i++)
  {
    VfsNode *node = (VfsNode *)NodeList [i];
    if ((memcmp (node->VPath, XPath, sl) == 0)
     && (node->VPath [sl]))
    {
      const char *pp = node->VPath + sl;
      while (*pp && *pp == VFS_PATH_SEPARATOR)
	pp++;
      while (*pp && *pp != VFS_PATH_SEPARATOR)
        pp++;
      while (*pp && *pp == VFS_PATH_SEPARATOR)
        pp++;
      char *news = new char [pp - node->VPath + 1];
      memcpy (news, node->VPath, pp - node->VPath);
      news [pp - node->VPath] = 0;
      if (fl->Find (news) == -1)
        fl->Push (news);
      else
        delete [] news;
    }
  }

  // Now find all files in given directory node
  if (node)
    node->FindFiles (suffix, mask, fl);

  if (fl->Length () == 0)
  {
    delete fl;
    fl = NULL;
  }

  ArchiveCache->CheckUp ();
  return fl;
}

iFile *csVFS::Open (const char *FileName, int Mode)
{
  if (!FileName)
    return NULL;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  if (!PreparePath (FileName, false, node, suffix, sizeof (suffix)))
    return NULL;

  iFile *f = node->Open (Mode, suffix);

  ArchiveCache->CheckUp ();
  return f;
}

bool csVFS::Sync ()
{
  ArchiveCache->DeleteAll ();
  return (ArchiveCache->Length () == 0);
}

iDataBuffer *csVFS::ReadFile (const char *FileName)
{
  iFile *F = Open (FileName, VFS_FILE_READ);
  if (!F)
    return NULL;

  size_t Size = F->GetSize ();
  iDataBuffer *data = F->GetAllData ();
  if (data)
  {
    F->DecRef ();
    return data;
  }

  char *buff = new char [Size + 1];
  if (!buff)
  {
    F->DecRef ();
    return NULL;
  }

  // Make the file zero-terminated in the case we'll use it as an ASCIIZ string
  buff [Size] = 0;
  if (F->Read (buff, Size) != Size)
  {
    delete [] buff;
    F->DecRef ();
    return NULL;
  }

  F->DecRef ();
  return new csDataBuffer (buff, Size);
}

bool csVFS::WriteFile (const char *FileName, const char *Data, size_t Size)
{
  iFile *F = Open (FileName, VFS_FILE_WRITE);
  if (!F)
    return false;

  bool success = (F->Write (Data, Size) == Size);
  F->DecRef ();
  return success;
}

bool csVFS::DeleteFile (const char *FileName)
{
  if (!FileName)
    return false;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  if (!PreparePath (FileName, false, node, suffix, sizeof (suffix)))
    return false;

  bool rc = node->Delete (suffix);

  ArchiveCache->CheckUp ();
  return rc;
}

bool csVFS::Mount (const char *VirtualPath, const char *RealPath)
{
  ArchiveCache->CheckUp ();

  if (!VirtualPath || !RealPath)
    return false;

  VfsNode *node;
  char suffix [2];
  if (!PreparePath (VirtualPath, true, node, suffix, sizeof (suffix))
   || suffix [0])
  {
    char *xp = _ExpandPath (VirtualPath, true);
    node = new VfsNode (xp, VirtualPath, System);
    NodeList.Push (node);
  }

  node->AddRPath (RealPath, this);
  if (node->RPathV.Length () == 0)
  {
    int idx = NodeList.Find (node);
    if (idx >= 0)
      NodeList.Delete (idx);
    return false;
  }

  return true;
}

bool csVFS::Unmount (const char *VirtualPath, const char *RealPath)
{
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
    int idx = NodeList.Find (node);
    if (idx >= 0)
      NodeList.Delete (idx);
  }

  return true;
}

bool csVFS::SaveMounts (const char *FileName)
{
  for (int i = 0; i < NodeList.Length (); i++)
  {
    VfsNode *node = (VfsNode *)NodeList.Get (i);
    int j;
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

bool csVFS::GetFileTime (const char *FileName, csFileTime &oTime) const
{
  if (!FileName)
    return false;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
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
  PreparePath (FileName, false, node, suffix, sizeof (suffix));

  bool success = node ? node->GetFileSize (suffix, oSize) : false;

  ArchiveCache->CheckUp ();
  return success;
}

iDataBuffer *csVFS::GetRealPath (const char *FileName)
{
  if (!FileName)
    return NULL;

  VfsNode *node;
  char suffix [VFS_MAX_PATH_LEN + 1];
  PreparePath (FileName, false, node, suffix, sizeof (suffix));
  if (!node) return NULL;

  char path [MAXPATHLEN + 1];
  for (int i = 0; i < node->RPathV.Length (); i++)
  {
    const char *rpath = node->RPathV.Get (i);
    strcat (strcpy (path, rpath), suffix);
    if (!access (path, F_OK))
      goto done;
  }

  strcat (strcpy (path, node->RPathV.Get (0)), suffix);
done:
  return new csDataBuffer (csStrNew (path), strlen (path) + 1);
}
