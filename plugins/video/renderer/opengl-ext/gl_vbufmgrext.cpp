/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: gl_vbufext.cpp
 *
 * DESCRIPTION:
 *  Implements a Vertex Buffer suitable for forming triangle strips, as well
 *  as individual triangles.
 * 
 * LICENSE:
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * AUTHOR:
 *    Thomas H. Hendrick
 *
 * CVS/RCS ID:
 *    $Id$
 *
 * === COPYRIGHT (c)2002 ============== PROJECT CRYSTAL SPACE 3D === */

/* -----------------------------------------------------------------
 * Preprocessor Includes
 * ----------------------------------------------------------------- */

#include <math.h>
#include <stdarg.h>

#include "cssysdef.h"
#include "csutil/util.h"
#include "qint.h"

#include "gl_vbufmgrext.h"
#include "gl_vbufext.h"
/* -----------------------------------------------------------------
 * Preprocessor Defines
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Private Enumeration Types
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Static Data Declarations
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Public Function Defintions
 * ----------------------------------------------------------------- */

SCF_IMPLEMENT_IBASE (csVertexBufferManagerEXT)
  SCF_IMPLEMENTS_INTERFACE (iVertexBufferManager)
SCF_IMPLEMENT_IBASE_END

csVertexBufferManagerEXT::csVertexBufferManagerEXT (iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csVertexBufferManagerEXT::object_reg = object_reg;
}

csVertexBufferManagerEXT::~csVertexBufferManagerEXT()
{
  for (int i=0; i < vClients.Length (); i++)
    ((iVertexBufferManagerClient*)vClients.Get (i))->ManagerClosing ();
}

/**
  * Create an empty vertex buffer. The ref count of this vertex buffer
  * will be one. To remove it use DecRef(). The priority number can be
  * anything. Higher numbers mean the vertex buffer is more important.
  * A high priority vertex buffer should be used for objects that are
  * visible often. Low priority vertex buffers should be used for objects
  * that are rarely visible.
  */
iVertexBuffer* csVertexBufferManagerEXT::CreateBuffer (int priority)
{
  csVertexBufferEXT* buf = new csVertexBufferEXT (this);
  buf->SetPriority (priority);
  buffers.Push (buf);
  return buf;
}

/**
  * Change the priority of a vertex buffer. This can be used when some
  * low-priority object becomes more important for example.
  */
void csVertexBufferManagerEXT::ChangePriority (iVertexBuffer* buf, int new_priority)
{
  csVertexBufferEXT* vbuf = (csVertexBufferEXT*)buf;
  vbuf->SetPriority (new_priority);
}

/**
  * Lock this vertex buffer for use. Only when the vertex buffer is locked
  * are you allowed to make calls to iGraphics3D functions that actually
  * use the vertex buffer. While the buffer is locked the arrays
  * that are given here may not be modified or altered in any way! The
  * buf_number indicates if the buffer has modified or not. You MUST
  * call UnlockBuffer() when you are ready with the buffer. Deleting a
  * vertex buffer with DecRef() automatically implies an UnlockBuffer().
  *<p>
  * This function will return false if the buffer could not be locked
  * for some reason. This may happen if too many buffers are locked at
  * the same time or if memory is low.
  */
bool csVertexBufferManagerEXT::LockBuffer (iVertexBuffer* buf,
                 csVector3* verts,
                 csVector2* texels,
                 csColor* colors,
                 int num_verts, int buf_number)
{
  csVertexBufferEXT *realbuf = (csVertexBufferEXT*)buf;
  realbuf->SetBuffers(verts, texels, colors, num_verts);
  realbuf->LockBuffer();
  return true;
}

/**
  * Unlock a vertex buffer.
  */
void csVertexBufferManagerEXT::UnlockBuffer (iVertexBuffer* buf)
{
  csVertexBufferEXT *realbuf = (csVertexBufferEXT*)buf;
  realbuf->UnLockBuffer();
}

//---------- Polygon Buffers -----------------------------------------------

/**
  * Create an empty polygon buffer. The ref count of this polygon buffer
  * will be one. To remove it use DecRef().
  */
iPolygonBuffer* csVertexBufferManagerEXT::CreatePolygonBuffer ()
{
  return NULL;
}

//---------- client handling -----------------------------------------------

/**
  * A client using the services of the manager can register with it to receive
  * information about the state of the manager.
  */
void csVertexBufferManagerEXT::AddClient (iVertexBufferManagerClient *client)
{
  vClients.Push ((csSome)client);
}

void csVertexBufferManagerEXT::RemoveClient (iVertexBufferManagerClient *client)
{
  int idx = vClients.Find ((csSome)client);
  if (idx != -1)
    vClients.Delete (idx);
}
