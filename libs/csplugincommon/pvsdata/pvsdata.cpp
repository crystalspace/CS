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
#include "csutil/csendian.h"
#include "csutil/ref.h"
#include "iutil/vfs.h"
#include "csgeom/statickdtree.h"

// TODO:  This would be useful in the main CS library.
// Read functions return the native-platform type.
// Write functions assume argument passed in native-platform type.
class csBinaryFileHelper
{
  csRef<iFile> file;
public:
  csBinaryFileHelper(csRef<iFile> file);

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

bool csPVSNodeData::PVSNamesContains(const char* name)
{
  //iVisibilityObject *vis = reg->visobj;
  //csFlags flags = reg->GetMeshWrapper ()->GetMeshObject()->GetFlags ();
  // If mesh is not staticpos or staticshape, we will not find it in
  // the pvsnames list.
  //if (!(flags & (CS_MESH_STATICPOS | CS_MESH_STATICSHAPE)))
  //  return false;
  // If pvsnames is NULL, reg's name is not in
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

void csPVSNodeData::RemoveFromPVS(void *object)
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

void SavePVSDataFile(const char* filename, const StaticKDTree* tree)
{
}

StaticKDTree* LoadPVSDataFile(const char* filename)
{
}
