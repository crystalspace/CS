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

SCF_IMPLEMENT_IBASE (csVertexBufferEXT)
  SCF_IMPLEMENTS_INTERFACE (iVertexBuffer)
SCF_IMPLEMENT_IBASE_END

csVertexBufferEXT::csVertexBufferEXT(iVertexBufferManager* mgr)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csVertexBufferEXT::mgr = mgr;
  m_locked = false;

  m_vertexcount = 0;
  m_vertices = 0;
  m_colors = 0;
  m_texcoords = 0;

}

csVertexBufferEXT::~csVertexBufferEXT ()
{
}

int csVertexBufferEXT::GetPriority () const
{
  return m_priority;
}

/// Check if the buffer is locked.
bool csVertexBufferEXT::IsLocked () const
{
  return m_locked;
}

/**
* Get the current array of vertices.
*/
csVector3* csVertexBufferEXT::GetVertices () const
{
  return m_vertices;
}

/**
* Get the current array of texels.
*/
csVector2* csVertexBufferEXT::GetTexels () const 
{
  return m_texcoords;
}
/**
* Get the current array of colors.
*/
csColor* csVertexBufferEXT::GetColors () const 
{
  return m_colors;
}

/**
* Get the number of vertices.
*/
int csVertexBufferEXT::GetVertexCount () const
{
  return m_vertexcount;
}

void csVertexBufferEXT::SetBuffers (csVector3 *verts, 
                  csVector2 *texcoords, 
                  csColor *colors, 
                  int count)
{
  m_vertices = verts;
  m_colors = colors;
  m_texcoords = texcoords;
  m_vertexcount = count;
}

void csVertexBufferEXT::LockBuffer()
{
  m_locked = true;
}

void csVertexBufferEXT::UnLockBuffer()
{
  m_locked = false;
}

void csVertexBufferEXT::SetPriority(int priority)
{
  m_priority = priority;
}
