/*
    Copyright (C) 2006 by Jorrit Tyberghein and Benjamin Stover

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

#include "csplugincommon/pvsdata/pvsdata.h"
#include "csutil/scf.h"
#include "csutil/csendian.h"
#include "csutil/ref.h"
#include "iutil/vfs.h"
#include "iutil/objreg.h"
#include "csgeom/statickdtree.h"
#include "cstool/initapp.h"

// TODO:  This would be useful in the main CS library.
// Read functions return the native-platform type.
// Write functions assume argument passed in native-platform type.
class csBinaryFileHelper
{
  csRef<iFile> file;

public:
  csBinaryFileHelper(csRef<iFile> file) : file(file)
  {
  }

  void Read(char* c, int size)
  {
    file->Read(c, size);
  }

  char ReadChar()
  {
    char c;
    file->Read(&c, sizeof(char));
    return c;
  }

  int32 ReadInt16()
  {
    int16 c;
    file->Read((char*)&c, sizeof(int16));
    return csLittleEndian::Convert(c);
  }

  int32 ReadInt32()
  {
    int32 c;
    file->Read((char*)&c, sizeof(int32));
    return csLittleEndian::Convert(c);
  }

  int64 ReadInt64()
  {
    int64 c;
    file->Read((char*)&c, sizeof(int64));
    return csLittleEndian::Convert(c);
  }

  float ReadFloat()
  {
    uint32 f;
    file->Read((char*)&f, sizeof(uint32));
    return csIEEEfloat::ToNative(f);
  }

  void Write(const char* c, int size)
  {
    file->Write(c, size);
  }

  void WriteChar(char c)
  {
    file->Write((char*)&c, sizeof(char));
  }

  void WriteInt16(int16 c)
  {
    c = csLittleEndian::Convert(c);
    file->Write((char*)&c, sizeof(int16));
  }

  void WriteInt32(int32 c)
  {
    c = csLittleEndian::Convert(c);
    file->Write((char*)&c, sizeof(int32));
  }

  void WriteInt64(int64 c)
  {
    c = csLittleEndian::Convert(c);
    file->Write((char*)&c, sizeof(int64));
  }
  
  void WriteFloat(float f)
  {
    uint32 c = csIEEEfloat::FromNative(f);
    file->Write((char*)&c, sizeof(uint32));
  }
};

static csString ReadString (csBinaryFileHelper* file)
{
  int len = file->ReadInt32 ();
  char *string = new char[len];
  file->Read (string, len);
  csString ret (string);
  delete string;
  return ret;
}

static void WriteString (csBinaryFileHelper* file, const csString& csstring)
{
  const char* string = csstring;
  file->WriteInt32 (csstring.Length());
  file->Write (string, csstring.Length ());
}

static csBinaryFileHelper* OpenForReading (const char* file, 
    iObjectRegistry* registry)
{
  iVFS* vfs = csInitializer::SetupVFS (registry);
  return new csBinaryFileHelper(vfs->Open(file, VFS_FILE_READ));
}

static csBinaryFileHelper* OpenForWriting (const char* file, 
    iObjectRegistry* registry)
{
  iVFS* vfs = csInitializer::SetupVFS (registry);
  return new csBinaryFileHelper(vfs->Open(file, VFS_FILE_WRITE));
}

static void Close (csBinaryFileHelper* file)
{
  delete file;
}

//-------------------------------------------------------------------------

csPVSNodeData::csPVSNodeData (csString* names, int total)
{
  if (total > 0)
  {
    pvs = new void*[total];
    memset ((void*) pvs, 0, sizeof(void*) * total);
    pvsnames = names;
    numRegistered = 0;
    numTotal = total;
  }
  else
  {
    pvs = NULL;
    pvsnames = NULL;
    numRegistered = numTotal = 0;
  }
}

csPVSNodeData::~csPVSNodeData ()
{
  delete[] pvs;
  delete[] pvsnames;
}

bool csPVSNodeData::PVSNamesContains (const char* name)
{
  if (!pvsnames)
    return false;
  if (pvsnames) 
  {
    for (int i = 0; i < numTotal; i++)
    {
      if (pvsnames[i] == name)
        return true;
    }
  }
  return false;
}

void csPVSNodeData::RemoveFromPVS (void *object)
{
  for (int i = 0; i < numRegistered; i++)
  {
    if (pvs[i] == object)
    {
      pvs[i] = pvs[(numRegistered--) - 1];
      break;
    }
  }
}

csPVSID::csPVSID (int numnodes, const csString& name) : name (name),
    nodes (numnodes)
{
}

//-------------------------------------------------------------------------

int FindIndex (const csString& string, csArray<csPVSID>& idarray)
{
  int length = idarray.Length();
  for (int i = 0; i < length; i++)
  {
    if (idarray[i].name == string)
      return i;
  }
  return -1;
}

void WriteNode (csBinaryFileHelper* file, const csStaticKDTree* node,
    csArray<csPVSID>& idarray)
{
  file->WriteChar (node->IsLeafNode() ? 'L' : 'I');

  // Write the PVS array
  csPVSNodeData* nodedata = (csPVSNodeData*) node->GetNodeData ();
  file->WriteInt32 (nodedata->numTotal);
  for (int i = 0; i < nodedata->numTotal; i++)
    file->WriteInt32 (FindIndex (nodedata->pvsnames[i], idarray));

  if (node->IsLeafNode ())
  {
    // This is a leaf node.
    // Nothing else is needed.
  }
  else
  {
    // This is an interior node.
    file->WriteInt32 (node->GetAxis ());
    file->WriteFloat (node->GetSplitLocation ());

    WriteNode (file, node->GetChild1 (), idarray);
    WriteNode (file, node->GetChild2 (), idarray);
  }
}

void SavePVSDataFile (iObjectRegistry* registry, const char* filename, 
    const csStaticKDTree* tree, csArray<csPVSID>& idarray)
{
  // Open file for writing
  csBinaryFileHelper *file = OpenForWriting(filename, registry);

  // Write all PVS IDs
  file->WriteInt32 (idarray.Length ());
  int length = idarray.Length ();
  for (int i = 0; i < length; i++)
  {
    file->WriteInt32 (idarray[i].nodes.Length());
    WriteString(file, idarray[i].name);
  }

  // Write all nodes using depth-first, child1-first algorithm
  WriteNode (file, tree, idarray);

  Close(file);
}

//-------------------------------------------------------------------------

static csStaticKDTree* ReadNode (csBinaryFileHelper* file, 
    csStaticKDTree* parent, bool isChild1, csArray<csPVSID>& idarray)
{
  static csArray<csStaticKDTreeObject*> emptylist;
  csStaticKDTree* ret;

  char c = file->ReadChar ();

  // Read in PVS array
  int pvssize = file->ReadInt32 ();
  csPVSNodeData* nodedata = new csPVSNodeData(new csString[pvssize], pvssize);
  for (int i = 0; i < pvssize; i++)
  {
    int index = file->ReadInt32 ();
    nodedata->pvsnames[i] = idarray[index].name;
    idarray[index].nodes.Push (nodedata);
  }

  if (c == 'L')
  {
    // This is a leaf node.
    ret = new csStaticKDTree  (parent, isChild1, emptylist);
    ret->SetNodeData (nodedata);
  }
  else if (c == 'I')
  {
    // This is an interior node.
    int axis = file->ReadInt32 ();
    float splitLocation = file->ReadFloat ();
    ret = new csStaticKDTree  (parent, isChild1, axis, splitLocation);
    ret->SetNodeData (nodedata);

    ReadNode (file, ret, true, idarray);
    ReadNode (file, ret, false, idarray);
  }
  else
  {
    // A file error occured
    // TODO:  replace with real error
    CS_ASSERT (false);
  }

  return ret;
}

csStaticKDTree* LoadPVSDataFile (iObjectRegistry* registry, 
    const char* filename, csArray<csPVSID>& idarray)
{
  // Use iFile to read data
  csBinaryFileHelper *file = OpenForReading(filename, registry);

  // Read in all PVS IDs
  int idsize = file->ReadInt32 ();
  idarray.Empty ();
  idarray.SetCapacity (idsize);
  for (int i = 0; i < idsize; i++)
  {
    int nodesize = file->ReadInt32 ();
    csString nodename = ReadString(file);
    idarray.Push (csPVSID (nodesize, nodename));
  }

  // Read in all nodes, which uses a depth-first algorithm with child1 done
  // first.
  csStaticKDTree* tree = ReadNode (file, NULL, true, idarray);

  Close(file);
  return tree;
}
