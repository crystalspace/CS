/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include <math.h>
#include <stdarg.h>

#include "cssysdef.h"
#include "vbufmgr.h"
#include "csutil/util.h"
#include "qint.h"

SCF_IMPLEMENT_IBASE (csVertexBuffer)
  SCF_IMPLEMENTS_INTERFACE (iVertexBuffer)
SCF_IMPLEMENT_IBASE_END

csVertexBuffer::csVertexBuffer (iVertexBufferManager* mgr) : verts (NULL)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csVertexBuffer::mgr = mgr;
  locked = false;
}

csVertexBuffer::~csVertexBuffer ()
{
  ((csVertexBufferManager*)mgr)->RemoveVBuf (this);
}

//--------------------------------------------- csVertexBufferManager -----//

SCF_IMPLEMENT_IBASE (csVertexBufferManager)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManager)
SCF_IMPLEMENT_IBASE_END

csVertexBufferManager::csVertexBufferManager (iObjectRegistry* object_reg)
	: buffers (16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csVertexBufferManager::object_reg = object_reg;
}

csVertexBufferManager::~csVertexBufferManager()
{
}

void csVertexBufferManager::RemoveVBuf (iVertexBuffer* buf)
{
  int idx = buffers.Find ((csVertexBuffer*)buf);
  if (idx >= 0) buffers.Delete (idx);
}

iVertexBuffer* csVertexBufferManager::CreateBuffer (int priority)
{
  csVertexBuffer* buf = new csVertexBuffer (this);
  buf->SetPriority (priority);
  buffers.Push (buf);
  return buf;
}

void csVertexBufferManager::ChangePriority (iVertexBuffer* buf,
	int new_priority)
{
  csVertexBuffer* vbuf = (csVertexBuffer*)buf;
  vbuf->SetPriority (new_priority);
}

bool csVertexBufferManager::LockBuffer (iVertexBuffer* buf,
  	csVector3* verts, int num_verts, int buf_number)
{
  (void)buf_number;
  csVertexBuffer* vbuf = (csVertexBuffer*)buf;
  vbuf->SetVertices (verts, num_verts);
  vbuf->SetLock (true);
  return true;
}

void csVertexBufferManager::UnlockBuffer (iVertexBuffer* buf)
{
  csVertexBuffer* vbuf = (csVertexBuffer*)buf;
  vbuf->SetLock (false);
}

