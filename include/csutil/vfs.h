/*
    Crystal Space Virtual File System class
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

#ifndef __VFS_H__
#define __VFS_H__

#include "csutil/csvector.h"
#include "types.h"
class VfsNode;
class csIniFile;
class csStrVector;

#define VFS_PATH_SEPARATOR '/'
#define VFS_PATH_DIVIDER ','
#define VFS_MAX_PATH_LEN 1023

enum csVfsStatus
{
  VFS_STATUS_OK,
  VFS_STATUS_NOSPC,
  VFS_STATUS_RESOURCES,
  VFS_STATUS_ACCESSDENIED,
  VFS_STATUS_IOERROR,
  VFS_STATUS_OTHER
};

enum csVfsFileMode
{
  VFS_FILE_UNCOMPRESSED = 1 << 0,
  VFS_FILE_READ = 1 << 1,
  VFS_FILE_WRITE = 1 << 2,
  VFS_FILE_MODE = ((1 << 1) | (1 << 2))
};

class csFile : public csBase
{
protected:
  VfsNode* Node;
  int Index;

public:
  size_t Size;
  csVfsStatus Error;
  char* Name;
  
public:
  csFile (int Mode, VfsNode *ParentNode, int RIndex, char *NameSuffix);
  virtual ~csFile ();
  virtual size_t Read (char *Data, size_t DataSize) = 0;
  virtual size_t Write (const char *Data, size_t DataSize) = 0;
  virtual bool AtEOF () = 0;
  virtual size_t GetPos () = 0;
  virtual int GetStatus ();
  virtual char *GetAllData ();
};


class csVFS : public csBase
{
public:
  class VfsVector : public csVector
  {
  public:
    VfsVector ();
    ~VfsVector ();
    bool FreeItem (csSome Item);
    int Compare (csSome Item1, csSome Item2, int Mode) const;
    int CompareKey (csSome Item, csConstSome Key, int Mode) const;
  };

private:
  char* cwd;
  csIniFile* config;
  VfsNode* cnode;
  char cnsufx[VFS_MAX_PATH_LEN];
  VfsVector NodeList;

public:
  csVFS (csIniFile*);
  ~csVFS ();
  static bool EnumConfig (csSome Parm, char *Name, size_t DataSize, csSome Data);
  bool AddLink (char *VirtualPath, char *RealPath);
  char *ExpandPath (const char *Path, bool IsDir = true) const;
  bool ChDir (const char *Path);
  char *GetCwd() const { return cwd; }
  bool Exists (const char *Path) const;
  csStrVector *FindFiles (const char *Path) const;
  csFile *Open (const char *FileName, int Mode);
  bool Sync ();
  char *ReadFile (const char *FileName, size_t &Size);
  bool WriteFile (const char *FileName, const char *Data, size_t Size);
  bool DeleteFile (const char *FileName);
  bool Mount (char *VirtualPath, char *RealPath);
  bool Unmount (char *VirtualPath, char *RealPath);
  bool SaveMounts (const char *FileName);

private:
  VfsNode *GetNode (const char *Path, char *NodePrefix,
    size_t NodePrefixSize) const;
  bool PreparePath (const char *Path, bool IsDir, VfsNode *&Node,
    char *Suffix, size_t SuffixSize) const;
};

#endif // __VFS_H__
