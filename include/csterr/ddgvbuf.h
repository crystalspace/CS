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
#ifndef _ddgVBuffer_Class_
#define _ddgVBuffer_Class_

#include "sysdef.h"
#include "csengine/terrain.h"
#include "csterr/ddgvec.h"

//#include "ddgcolor.hpp"

typedef unsigned int ddgVBIndex;
/**
 * A class supporting vertex array buffers for rendering.
 */
class WEXP ddgVBuffer  {
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
public:
	/// Vertex buffer.
	csVector3	*vbuf;
	/// Index buffer.
	ddgVBIndex *ibuf;
	/// Texture coord buffer.
	ddgVector2 *tbuf;
	/// Normal coord buffer.
	csVector3 *nbuf;
	/// Color buffer.
	float *cbuf;
	/// Create a object but don't allocate any memory.
	ddgVBuffer( void );
	/// Destructor free all memory.
	~ddgVBuffer( void );
	/// Render the buffer object.
//	bool draw( ddgContext *ctx );
	/// Initialize the vector Buffer object and allocate buffers if size is set.
	bool init( /*ddgContext *ctx*/ );
    /// Reset the buffers, Must be called before each frame.
    void reset(void) { _num = 0; _inum = 0; }
    /// Initial buffer size to allocate.  Must be called before init.
    void size(unsigned int s)
	{ _num = s; }
	/// Return the number of triangles in the buffer.
	unsigned int size(void) { return _inum/3; }
	/// Set the rendering mode.
	void renderMode( bool t = true, bool n = true, bool c = true)
    { _fTexture = t; _fNormal = n; _fColor = c; }
	/// Is color active
	bool colorOn(void) { return _fColor; }
	/// Is texture active
	bool textureOn(void) { return _fTexture; }
	/// Is normal active
	bool normalOn(void) { return _fNormal; }
    /// Push a triangle into the buffer.
    ddgVBIndex pushVTNC(csVector3 *p, ddgVector2 *t, csVector3 *n, ddgColor3 *c);
    /// Push a triangle into the buffer.
    ddgVBIndex pushVTC(csVector3 *p, ddgVector2 *t, ddgColor3 *c);
    /// Push a triangle into the buffer.
    ddgVBIndex pushVTN(csVector3 *p, ddgVector2 *t, csVector3 *n); 
    /// Push a triangle into the buffer.
    ddgVBIndex pushVT(csVector3 *p, ddgVector2 *t);
    /// Push an index into the buffer.
    unsigned int pushIndex( ddgVBIndex i1, ddgVBIndex i2, ddgVBIndex i3 );
	/// Depth Sort the triangles in the buffer.
	void sort(void);
};

#endif

