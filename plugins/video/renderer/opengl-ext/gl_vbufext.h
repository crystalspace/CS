#ifndef _GL_VBUFEXT_H_
#define _GL_VBUFEXT_H_
/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: gl_vbufext.h
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

#include "ivideo/vbufmgr.h"

struct iObjectRegistry;
class csVector3;
class csVector2;
class csColor;

/* -----------------------------------------------------------------
 * Preprocessor Defines and Enumeration Types
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Public Class Declarations
 * ----------------------------------------------------------------- */

class csVertexBufferEXT;

/* -----------------------------------------------------------------
 * Public Class Definitions
 * ----------------------------------------------------------------- */
class csVertexBufferEXT : public iVertexBuffer
{
  // -------------------------------------------------------
  // Public Constructors and Destructors
  // -------------------------------------------------------

  public:
    csVertexBufferEXT (iVertexBufferManager* mgr);
    virtual ~csVertexBufferEXT ();


  // -------------------------------------------------------
  // Public Member Functions
  // -------------------------------------------------------
  public:
    /// Get the priority.
    virtual int GetPriority () const;

    /// Set the priority.
    void SetPriority(int priority);

    /// Check if the buffer is locked.
    virtual bool IsLocked () const;
    
    /**
    * Get the current array of vertices.
    */
    virtual csVector3* GetVertices () const;
    /**
    * Get the current array of texels.
    */
    virtual csVector2* GetTexels () const;
    /**
    * Get the current array of colors.
    */
    virtual csColor* GetColors () const;
    /**
    * Get all of the current user arrays.
    */
    virtual float* GetUserArray (int index) const;
    /**
    * Get the number of components of one of the current user arrays.
    */
    virtual int GetUserArrayComponentCount (int index) const;
    /**
    * Get the number of vertices.
    */
    virtual int GetVertexCount () const;

    void SetBuffers (csVector3 *verts, 
                     csVector2 *texcoords, 
                     csColor *colors, 
                     int count);

    void LockBuffer();
    void UnLockBuffer();

    SCF_DECLARE_IBASE;
    // -------------------------------------------------------
    // Protected Member Functions
    // -------------------------------------------------------
  protected:
    int         m_priority;
    bool        m_locked;
    int         m_vertexcount;
    csVector3 * m_vertices;
    csColor   * m_colors;
    csVector2 * m_texcoords;
    iVertexBufferManager* mgr;
 
};


#endif /* _GL_VBUFEXT_H_ */
