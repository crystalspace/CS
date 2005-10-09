/*
    Copyright (C) 2003 by Philipp Aumayr

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

#include "csgfx/renderbuffer.h"
#include "cstool/userrndbuf.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

iRenderBuffer* csUserRenderBufferManager::GetRenderBuffer (csStringID name) const
{
  size_t bufIndex = userBuffers.FindSortedKey (UserBufArrayCmp (name));
  if (bufIndex == csArrayItemNotFound) return 0;

  return userBuffers[bufIndex].buf;
}

int csUserRenderBufferManager::BufCompare (userbuffer const& r, 
					   userbuffer const& k)
{ 
  return r.name - k.name;
}

bool csUserRenderBufferManager::AddRenderBuffer (csStringID name, 
						 iRenderBuffer* buffer)
{
  size_t bufIndex = userBuffers.FindSortedKey (UserBufArrayCmp (name));
  if (bufIndex != csArrayItemNotFound) return false;

  userbuffer ub;
  ub.buf = buffer;
  ub.name = name;
  userBuffers.InsertSorted (ub, &BufCompare);
  return true;
}

bool csUserRenderBufferManager::RemoveRenderBuffer (csStringID name)
{
  size_t bufIndex = userBuffers.FindSortedKey (UserBufArrayCmp (name));
  if (bufIndex != csArrayItemNotFound) return false;
  userBuffers.DeleteIndex (bufIndex);
  return true;
}

class BufferNameIter : public scfImplementation1<BufferNameIter, 
                                                 iUserRenderBufferIterator>
{
  size_t index;
public:
  csArray<csStringID> names;
  csRefArray<iRenderBuffer> buffers;

  BufferNameIter() : scfImplementationType (this), index(0) 
  {
  }
  virtual ~BufferNameIter() 
  {
  }

  bool HasNext();
  csStringID Next (csRef<iRenderBuffer>* buf = 0);
  void Reset();
};

csRef<iUserRenderBufferIterator> csUserRenderBufferManager::GetBuffers() const
{
  csRef<BufferNameIter> newIter;
  newIter.AttachNew (new BufferNameIter);
  for (size_t i = 0; i < userBuffers.Length(); i++)
  {
    newIter->names.Push (userBuffers[i].name);
    newIter->buffers.Push (userBuffers[i].buf);
  }
  return newIter;
}

//---------------------------------------------------------------------------


bool BufferNameIter::HasNext()
{
  return index < names.Length();
}

csStringID BufferNameIter::Next (csRef<iRenderBuffer>* buf)
{
  if (index < names.Length())
  {
    if (buf != 0) *buf = buffers[index];
    return names[index++];
  }
  else
  {
    if (buf != 0) *buf = 0;
    return csInvalidStringID;
  }
}

void BufferNameIter::Reset()
{
  index = 0;
}
