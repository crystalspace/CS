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

#include "csterr/ddg.h"
#include "csterr/ddgvbuf.h"
// ----------------------------------------------------------------------
// VBuffer
//   Initialize the VBuffer arrays.
// ----------------------------------------------------------------------

ddgVBuffer::ddgVBuffer(void)
{
    vbuf = NULL;
	tbuf = NULL;
	ibuf = NULL;
	cbuf = NULL;
	nbuf = NULL;
    _num = 0;
    _fNormal = true;
    _fTexture = true;
    _fColor = true;
}

ddgVBuffer::~ddgVBuffer(void)
{
	delete vbuf;
	delete tbuf;
	delete ibuf;
	delete cbuf;
	delete nbuf;
}

bool ddgVBuffer::init( /*ddgContext *c */)
{
#ifdef DDG
    if (super::init(c))
        return true;
#endif
    if (_num)
    {
	    ibuf = new unsigned int[_num*2];
		ddgAsserts(ibuf,"Failed to Allocate memory");
		ddgMemorySet(unsigned int,_num*2);
        vbuf = new csVector3[_num];
		ddgAsserts(vbuf,"Failed to Allocate memory");
		ddgMemorySet(csVector3,_num);
        if (_fNormal)
		{
            nbuf = new csVector3[_num];
			ddgAsserts(nbuf,"Failed to Allocate memory");
			ddgMemorySet(csVector3,_num);
		}
        if (_fColor)
		{
	    	cbuf = new float[_num*4];
			ddgAsserts(cbuf,"Failed to Allocate memory");
			ddgMemorySet(float,_num*4);
		}
        if (_fTexture)
		{
    		tbuf = new ddgVector2[_num];
			ddgAsserts(tbuf,"Failed to Allocate memory");
			ddgMemorySet(ddgVector2,_num);
		}
    }

	return false;
}
/*
bool ddgVBuffer::draw(ddgContext *ctx)
{
	bool retval = false;

	if (_fTexture)
	{
		// Texture coordinates.
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer (2, GL_FLOAT, 0, _tbuf);
	}

	if (_fColor)
	{
		// 4 color components. [Unsigned char DOESNT WORK!]
		glEnableClientState (GL_COLOR_ARRAY);
		glColorPointer( 4, GL_FLOAT, 0, _cbuf );
	}

	if (_fNormal)
	{
		// Normal array for rendering.
		glEnableClientState (GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 12, _nbuf); // 3 floats = 16 bytes.
	}

	// Vertex array for rendering.
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer (3, GL_FLOAT, 12, _vbuf); // 3 floats = 16 bytes.
#if DDG_SUPPORT_COMPILED_VERTEX_ARRAYS
	glLockArraysEXT(0, _step*_step);
#endif
	glDrawElements(GL_TRIANGLES,_inum, GL_UNSIGNED_INT, _ibuf);
#if DDG_SUPPORT_COMPILED_VERTEX_ARRAYS
	glUnlockArraysEXT();
#endif

	glDisableClientState (GL_VERTEX_ARRAY);
	if (_fNormal)  glDisableClientState (GL_NORMAL_ARRAY);
	if (_fColor)  glDisableClientState (GL_COLOR_ARRAY);
	if (_fTexture) glDisableClientState (GL_TEXTURE_COORD_ARRAY);

    return retval;
}
*/
/// Push a given vertex into the buffer.
ddgVBIndex ddgVBuffer::pushVT(csVector3 *p1, ddgVector2 *t1)
{
	// Push the vertex.
    vbuf[_num]=*p1;
    tbuf[_num]=t1;
    _num++;

    return _num;
}
/// Push a given vertex into the buffer.
ddgVBIndex ddgVBuffer::pushVTN(csVector3 *p1, ddgVector2 *t1, csVector3 *n1 )
{
	// Push the vertex.
    vbuf[_num]=*p1;
    nbuf[_num]=*n1;
    tbuf[_num]=t1;
    _num++;

    return _num;
}
/// Push a vertex into the buffer.
ddgVBIndex ddgVBuffer::pushVTNC(csVector3 *p1, ddgVector2 *t1, csVector3 *n1, ddgColor3 *c1)
{
	// Push the vertex.
    const float df2 = 1.0/256.0;
    vbuf[_num]=*p1;
    if (_fNormal)
        nbuf[_num]=*n1;
    if (_fTexture)
        tbuf[_num].set(t1);
    if (_fColor)
    {
        cbuf[_num*4+0]=c1->v[0]*df2;
        cbuf[_num*4+1]=c1->v[1]*df2;
        cbuf[_num*4+2]=c1->v[2]*df2;
        cbuf[_num*4+3]=1.0;
    }
    _num++;

    return _num;
}
/// Push a vertex into the buffer.
ddgVBIndex ddgVBuffer::pushVTC(csVector3 *p1, ddgVector2 *t1, ddgColor3 *c1)
{
	// Push the vertex.
    const float df2 = 1.0/256.0;
    vbuf[_num]=*p1;
    if (_fTexture)
        tbuf[_num].set(t1);
    if (_fColor)
    {
        cbuf[_num*4+0]=c1->v[0]*df2;
        cbuf[_num*4+1]=c1->v[1]*df2;
        cbuf[_num*4+2]=c1->v[2]*df2;
        cbuf[_num*4+3]=1.0;
    }
    _num++;

    return _num;
}

unsigned int ddgVBuffer::pushIndex( ddgVBIndex i1, ddgVBIndex i2, ddgVBIndex i3 )
{
    ibuf[_inum] = i1-1;
    ibuf[_inum+1] = i2-1;
    ibuf[_inum+2] = i3-1;
    _inum += 3;
	ddgAssert(_num *3 >= _inum);
    return _inum;
}

#define ZCACHESIZE	10000
static float ddgZcache[ZCACHESIZE];	// $TODO This could run out of space!
static ddgVBuffer *ddgCurrentVBuf = NULL;

int compareTriangles( const void* p1, const void* p2)
{
	unsigned int *t1 = (unsigned int*)p1;
	unsigned int *t2 = (unsigned int*)p2;
	float z1 = ddgZcache[t1[0]/3],
		  z2 = ddgZcache[t2[0]/3];

	if (z1 == 0.0)
	{
		// Find nearest point and use that.
		z1 = ddgCurrentVBuf->vbuf[t1[0]].z;
		if (ddgCurrentVBuf->vbuf[t1[1]].z < z1)
			z1 = ddgCurrentVBuf->vbuf[t1[1]].z;
		if (ddgCurrentVBuf->vbuf[t1[2]].z < z1)
			z1 = ddgCurrentVBuf->vbuf[t1[2]].z;
		ddgZcache[t1[0]/3] = z1;
	}
	if (z2 == 0.0)
	{
		// Find nearest point and use that.
		z2 = ddgCurrentVBuf->vbuf[t2[0]].z;
		if (ddgCurrentVBuf->vbuf[t2[1]].z < z2)
			z2 = ddgCurrentVBuf->vbuf[t2[1]].z;
		if (ddgCurrentVBuf->vbuf[t2[2]].z < z2)
			z2 = ddgCurrentVBuf->vbuf[t2[2]].z;
		ddgZcache[t2[0]/3] = z2;
	}
	// Compare the two points.
	if      ( z1 < z2 ) return -1;
	else if ( z2 < z1 ) return 1;
	return 0;
}

void ddgVBuffer::sort(void)
{
	// Initialize the zcache.
	int i;
	for (i = 0; i < ZCACHESIZE; i++)
		ddgZcache[i] = 0;
	ddgCurrentVBuf = this;
	qsort((void*)ibuf,_inum/3,3*sizeof(unsigned int),compareTriangles);
}
