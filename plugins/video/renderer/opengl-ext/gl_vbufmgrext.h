#ifndef GL_VBUFMGREXT_H_INCLUDED
#define GL_VBUFMGREXT_H_INCLUDED
/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: gl_vbufext.h
 *
 * DESCRIPTION:
 *  Implements a Vertex Buffer Manager suitable for any standard OGL
 *  1.2 implementation.
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
 * AUTHOR:
 *    Philipp R. Aumayr
 *
 * CVS/RCS ID:
 *    $Id:
 *
 * === COPYRIGHT (c)2002 ============== PROJECT CRYSTAL SPACE 3D === */
/* -----------------------------------------------------------------
 * Preprocessor Includes
 * ----------------------------------------------------------------- */
#include "csutil/csvector.h"
#include "csutil/typedvec.h"
#include "ivideo/vbufmgr.h"
#include "gl_vbufext.h"

/* -----------------------------------------------------------------
 * Preprocessor Defines and Enumeration Types
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Public Class Declarations
 * ----------------------------------------------------------------- */

class csVertexBufferManagerEXT;

/* -----------------------------------------------------------------
 * Public Class Definitions
 * ----------------------------------------------------------------- */
class csVertexBufferManagerEXT : public iVertexBufferManager
{
  CS_DECLARE_TYPED_VECTOR_NODELETE (csVBufVector, csVertexBufferEXT);
  /// List of vertex buffers.
  csVBufVector buffers;

  /// Object Registry
  iObjectRegistry *object_reg;

  /// list of registered clients
  csVector vClients;


public:
    csVertexBufferManagerEXT(iObjectRegistry* object_reg);
    virtual ~csVertexBufferManagerEXT();
 
   SCF_DECLARE_IBASE;

  
   /**
   * Create an empty vertex buffer. The ref count of this vertex buffer
   * will be one. To remove it use DecRef(). The priority number can be
   * anything. Higher numbers mean the vertex buffer is more important.
   * A high priority vertex buffer should be used for objects that are
   * visible often. Low priority vertex buffers should be used for objects
   * that are rarely visible.
   */
  virtual csPtr<iVertexBuffer> CreateBuffer (int priority);

  /**
   * Change the priority of a vertex buffer. This can be used when some
   * low-priority object becomes more important for example.
   */
  void ChangePriority (iVertexBuffer* buf, int new_priority);

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
  virtual bool LockBuffer (iVertexBuffer* buf,
  	csVector3* verts,
	csVector2* texels,
	csColor* colors,
	int num_verts, int buf_number);

   virtual bool LockUserArray (iVertexBuffer* buf,
		int index, float* user, 
		int num_components, int buf_number);

  /**
   * Unlock a vertex buffer.
   */
  virtual void UnlockBuffer (iVertexBuffer* buf);

  //---------- Polygon Buffers -----------------------------------------------

  /**
   * Create an empty polygon buffer. The ref count of this polygon buffer
   * will be one. To remove it use DecRef().
   */
  virtual iPolygonBuffer* CreatePolygonBuffer ();

  //---------- client handling -----------------------------------------------

  /**
   * A client using the services of the manager can register with it to receive
   * information about the state of the manager.
   */
  virtual void AddClient (iVertexBufferManagerClient *client);
  virtual void RemoveClient (iVertexBufferManagerClient *client);
 
};

#endif
