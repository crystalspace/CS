/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddgVArray_Class_
#define _ddgVArray_Class_

#include "util/ddgerror.h"
#include "math/ddgvec.h"
#include "struct/ddgcolor.h"

typedef unsigned int ddgVBIndex;
/**
 * A class supporting vertex array buffers.
 * It can allocate memory and supports pushing data into
 * buffers in various convenient configurations.
 */
class WEXP ddgVArray 
 {
public:
	/// Enumeration of tree types.
	enum ddgBufType{ dummy = 0, point = 1, line = 2, triangle = 3, quad = 4 };
private:
	/// Size of the buffers allocated by this vertex array.
	unsigned int _bufsize;
    /**
     * Number of vertices/texture/normal and color elements in buffer.
     */
    ddgVBIndex _num;
    /// Number of indices.
    unsigned int _inum;
    /// Textured triangle set.
    bool    _fTexture:1;
    /// Render normals
    bool    _fNormal:1;
    /// Colored triangle set.
    bool    _fColor:1;
    /// The type of objects this buffer is managing.
    ddgBufType    _type;
public:
	/// Vertex buffer.
	ddgVector3	*vbuf;
	/// Index buffer.
	ddgVBIndex *ibuf;
	/// Texture coord buffer.
	ddgVector2 *tbuf;
	/// Normal coord buffer.
	ddgVector3 *nbuf;
	/// Color buffer.
	ddgColor4 *cbuf;
	/// Create a object but don't allocate any memory.
	ddgVArray( ddgBufType type = triangle);
	/// Destructor free all memory.
	~ddgVArray( void );
	/// Initialize the vector Buffer object and allocate buffers if size is set.
	bool init(void );
    /// Reset the buffers, Must be called before filling buffer.
    void reset(void) { _num = 0; _inum = 0; }
    /// Initial buffer size to allocate.  Must be called before init.
    void size(unsigned int s) { _bufsize = s; }
	/// Return the number of triangles in the buffer.
	unsigned int size(void) { return _inum/3; }
	/// Set the rendering mode.
	void renderMode( bool t = true, bool n = true, bool c = true)
    { _fTexture = t; _fNormal = n; _fColor = c; }
	/// Is color active
	inline bool colorOn(void) { return _fColor; }
	/// Is texture active
	inline bool textureOn(void) { return _fTexture; }
	/// Is normal active
	inline bool normalOn(void) { return _fNormal; }
	/// How many items are in the stack.
	inline unsigned int inum(void) { return _inum; }
	/// What type of primitive are we managing.
	inline ddgBufType type(void) { return _type; }
     /// Push a triangle into the buffer.
    ddgVBIndex pushVTNC(ddgVector3 *p, ddgVector2 *t, ddgVector3 *n, ddgColor3 *c);
    /// Push a triangle into the buffer.
    ddgVBIndex pushVTN(ddgVector3 *p, ddgVector2 *t, ddgVector3 *n); 
    /// Push a triangle into the buffer.
    ddgVBIndex pushVT(ddgVector3 *p, ddgVector2 *t);
    /// Push a vertex into the buffer.
    ddgVBIndex pushVC(ddgVector3 *p, ddgColor3 *c);
    /// Push a vertex into the buffer.
    ddgVBIndex pushV(ddgVector3 *p);
    /// Push an index into the buffer.
    unsigned int pushTriangle( ddgVBIndex i1, ddgVBIndex i2, ddgVBIndex i3 );
    /// Push an index into the buffer.
    unsigned int pushQuad( ddgVBIndex i1, ddgVBIndex i2, ddgVBIndex i3, ddgVBIndex i4 );
    /// Push an index into the buffer.
    unsigned int pushLine( ddgVBIndex i1, ddgVBIndex i2 );
    /// Push an index into the buffer.
    unsigned int pushPoint( ddgVBIndex i1 );
 	/// Depth Sort the data in the buffer.
	void sort(void);
};

#endif

