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

//--------------------------------------------- csPolygonBuffer -----//

SCF_IMPLEMENT_IBASE (csPolygonBuffer)
  SCF_IMPLEMENTS_INTERFACE (iPolygonBuffer)
SCF_IMPLEMENT_IBASE_END

csPolygonBuffer::csPolygonBuffer (iVertexBufferManager* mgr)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csPolygonBuffer::mgr = mgr;
}

csPolygonBuffer::~csPolygonBuffer ()
{
}

//---------------------------------------------- csVertexBuffer -----//

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
  // We don't clear our vertex buffers and polygon buffers here
  // because they should already be gone by the time we come here.
  // @@@ Maybe an assert here to check if that's the case?
  // CS_ASSERT (buffers.Length () == 0);

  /// go through list of registered client and tell 'em we are no longer
  /// available
  for (int i=0; i < vClients.Length (); i++)
    ((iVertexBufferManagerClient*)vClients.Get (i))->ManagerClosing ();
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
  	csVector3* verts, csVector2* texels,
	csColor* colors,
	int num_verts, int buf_number)
{
  (void)buf_number;
  csVertexBuffer* vbuf = (csVertexBuffer*)buf;
  vbuf->SetBuffers (verts, texels, colors, num_verts);
  vbuf->SetLock (true);
  return true;
}

void csVertexBufferManager::UnlockBuffer (iVertexBuffer* buf)
{
  csVertexBuffer* vbuf = (csVertexBuffer*)buf;
  vbuf->SetLock (false);
}

void csVertexBufferManager::AddClient (iVertexBufferManagerClient *client)
{
  vClients.Push ((csSome)client);
}

void csVertexBufferManager::RemoveClient (iVertexBufferManagerClient *client)
{
  int idx = vClients.Find ((csSome)client);
  if (idx != -1)
    vClients.Delete (idx);
}
