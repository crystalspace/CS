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
class csVertexBufferEXT {
  // -------------------------------------------------------
  // Public Constructors and Destructors
  // -------------------------------------------------------
  public:
    csVertexBufferEXT ( );
    ~csVertexBufferEXT ( );

  // -------------------------------------------------------
  // Public Member Functions
  // -------------------------------------------------------
  public:
    // Destroy the contained data, resetting everything to empty
    int Reset( );

    // Add a vertex to the vertex list
    int AddVertex( float x, float y, float z );
    // Add multiple vertices at once
    int AddVertices( const float *vertices, const int count );

    // -------------------------------------------------------
    // These functions add texture coordinates, normal vectors,
    // colors, and other associated data to the specified
    // vertex.  The Index parameter corresponds to the return
    // value from the associated AddVertex() call, or the return
    // value from the AddVertices() call plus the offset.
    // -------------------------------------------------------

    // Texture Coordinates
    int AddTexCoords( int index, float *coords, int components );
    int AddTexCoords( int index, float u );
    int AddTexCoords( int index, float u, float v );
    int AddTexCoords( int index, float u, float v, float s );

    // Vertex Normal Vectors
    int AddNormalVector( int index, float *vec, int components );
    int AddNormalVector( int index, float x, float y, float z );

    // Vertex Colors
    int AddColor( int index, float *color, int components );
    int AddColor( int index, float r, float g, float b );
    int AddColor( int index, float r, float g, float b, float a );

    // -------------------------------------------------------
    // The following functions are used for making triangles, as
    // well as triangle strips.
    // -------------------------------------------------------
    int AddTriangle( const int v1, const int v2, const int v3 );
    
    int TriangleStripBegin( const int v1, const int v2, const int v3 );
    int TriangleStripAdd  ( const int v_next );

    int TriangleStripFromData( const int *v_indices, const int count );

    // -------------------------------------------------------
    // This is used to get a pointer to the internal data array,
    // and is normally called when this vertex buffer is to be
    // rasterized.  This performs an implicit call to "CompactArrays"
    // if the data has not been compacted.
    // -------------------------------------------------------
    const float *GetCompactedArray( );

    // -------------------------------------------------------
    // Used to get information about the array
    // -------------------------------------------------------
    int GetArrayInfo( int &vertex_count,
                      int &rowstride,
                      int &c_per_vertex,
                      int &t_per_vertex,
                      int &color_per_vertex );

  // -------------------------------------------------------
  // Protected Member Functions
  // -------------------------------------------------------
  protected:
    // Compact all the data into a contiguous array
    int CompactArrays ( );
};


#endif /* _GL_VBUFEXT_H_ */
