/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
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

#include "util/ddgerror.h"
#include "struct/ddgvarr.h"

// ----------------------------------------------------------------------
// ddgVArray
//   Initialize the ddgVArray arrays.
// ----------------------------------------------------------------------
ddgVArray::ddgVArray( ddgBufType type  )
{
	ibuf = NULL;
    vbuf = NULL;
	tbuf = NULL;
	tibuf = NULL;
	cbuf = NULL;
	nbuf = NULL;
    _num = 0;
	_inum = 0;
    _fNormal = true;
    _fTexture = true;
    _fTextureCoord = false;
    _fColor = true;
	_type = type;
	_bufsize = 0;
}

ddgVArray::~ddgVArray(void)
{
	delete []ibuf;
	delete []vbuf;
	delete []tbuf;
	delete []tibuf;
	delete []cbuf;
	delete []nbuf;
}

bool ddgVArray::init(void)
{

    if (_bufsize)
    {
	    ibuf = new int[_bufsize*2];
		ddgAsserts(ibuf,"Failed to Allocate memory");
		ddgMemorySet(int,_bufsize*2);
        vbuf = new ddgVector3[_bufsize];
		ddgAsserts(vbuf,"Failed to Allocate memory");
		ddgMemorySet(ddgVector3,_bufsize);
        if (_fNormal)
		{
            nbuf = new ddgVector3[_bufsize];
			ddgAsserts(nbuf,"Failed to Allocate memory");
			ddgMemorySet(ddgVector3,_bufsize);
		}
        if (_fColor)
		{
	    	cbuf = new ddgColor4[_bufsize];
			ddgAsserts(cbuf,"Failed to Allocate memory");
			ddgMemorySet(ddgColor4,_bufsize);
		}
        if (_fTexture)
		{
			if (_fTextureCoord)
			{
    			tibuf = new ddgTexCoord2[_bufsize];
				ddgAsserts(tibuf,"Failed to Allocate memory");
				ddgMemorySet(ddgTexCoord2,_bufsize);
			}
			else
			{
    			tbuf = new ddgVector2[_bufsize];
				ddgAsserts(tbuf,"Failed to Allocate memory");
				ddgMemorySet(ddgVector2,_bufsize);
			}
		}
    }
 
	return ddgSuccess;
}


/// Push a given vertex into the buffer.
ddgVBIndex ddgVArray::pushV(ddgVector3 *p1)
{
	ddgAsserts(_num < _bufsize,"Buffer overflow");
	// Push the vertex.
    vbuf[_num] =(*p1);
    _num++;

    return _num-1;
}
/// Push a given vertex into the buffer.
ddgVBIndex ddgVArray::pushVT(ddgVector3 *p1, ddgVector2 *t1)
{
	ddgAsserts(_num < _bufsize,"Buffer overflow");
	// Push the vertex.
    vbuf[_num] =(*p1);
    tbuf[_num] =(*t1);
    _num++;

    return _num-1;
}
/// Push a given vertex into the buffer.
ddgVBIndex ddgVArray::pushVT(ddgVector3 *p1, ddgTexCoord2 *t1)
{
	ddgAsserts(_num < _bufsize,"Buffer overflow");
	// Push the vertex.
    vbuf[_num] =(*p1);
    tibuf[_num].v[0] =t1->v[0];
    tibuf[_num].v[1] =t1->v[1];
    _num++;

    return _num-1;
}
/// Push a given vertex into the buffer.
ddgVBIndex ddgVArray::pushVTN(ddgVector3 *p1, ddgVector2 *t1, ddgVector3 *n1 )
{
	ddgAsserts(_num < _bufsize,"Buffer overflow");
	// Push the vertex.
    vbuf[_num] =(*p1);
    nbuf[_num] =(*n1);
    tbuf[_num] =(*t1);
    _num++;

    return _num-1;
}
/// Push a vertex into the buffer.
ddgVBIndex ddgVArray::pushVTNC(ddgVector3 *p1, ddgVector2 *t1, ddgVector3 *n1, ddgColor4 *c1)
{
	ddgAsserts(_num < _bufsize,"Buffer overflow");
	// Push the vertex.
    vbuf[_num] = (*p1);
    if (_fNormal)
        nbuf[_num] = (*n1);
    if (_fTexture)
        tbuf[_num] = (*t1);
    if (_fColor)
    {
        cbuf[_num].v[0]=c1->v[0];
        cbuf[_num].v[1]=c1->v[1];
        cbuf[_num].v[2]=c1->v[2];
        cbuf[_num].v[3]=c1->v[3];
    }
    _num++;

    return _num-1;
}
/// Push a vertex into the buffer.
ddgVBIndex ddgVArray::pushVC(ddgVector3 *p1, ddgColor4 *c1)
{
	ddgAsserts(_num < _bufsize,"Buffer overflow");
	// Push the vertex.
    vbuf[_num] = (*p1);
    if (_fColor)
    {
        cbuf[_num].v[0]=c1->v[0];
        cbuf[_num].v[1]=c1->v[1];
        cbuf[_num].v[2]=c1->v[2];
        cbuf[_num].v[3]=c1->v[3];
    }
    _num++;

    return _num-1;
}

void ddgVArray::pushPoint( ddgVBIndex i1 )
{
    ibuf[_inum] = i1;
    _inum ++;
}

void ddgVArray::pushLine( ddgVBIndex i1, ddgVBIndex i2 )
{
    ibuf[_inum] = i1;
    ibuf[_inum+1] = i2;
    _inum += 2;
}

void ddgVArray::pushTriangle( ddgVBIndex i1, ddgVBIndex i2, ddgVBIndex i3 )
{
    ibuf[_inum] = i1;
    ibuf[_inum+1] = i2;
    ibuf[_inum+2] = i3;
    _inum += 3;
}

void ddgVArray::pushQuad( ddgVBIndex i1, ddgVBIndex i2, ddgVBIndex i3, ddgVBIndex i4 )
{
    ibuf[_inum] = i1;
    ibuf[_inum+1] = i2;
    ibuf[_inum+2] = i3;
    ibuf[_inum+3] = i4;
    _inum += 4;
}

#define ZCACHESIZE	10000
static float ddgZcache[ZCACHESIZE];	// $TODO This could run out of space!
static ddgVArray *ddgCurrentVBuf = NULL;

int compareObjects( const void* p1, const void* p2)
{
	unsigned int *t1 = (unsigned int*)p1;
	unsigned int *t2 = (unsigned int*)p2;
	unsigned int n = ddgCurrentVBuf->type(), i;
	float z1 = ddgZcache[t1[0]/n],
		  z2 = ddgZcache[t2[0]/n];
	if (z1 == 0.0)
	{
		// Find object's nearest point and use that.
		z1 = (ddgCurrentVBuf->vbuf[t1[0]])[2];
		for (i=1; i < n; i++)
		{
			if ((ddgCurrentVBuf->vbuf[t1[i]])[2] < z1)
				z1 = (ddgCurrentVBuf->vbuf[t1[i]])[2];
		}
		ddgZcache[t1[0]/n] = z1;
	}
	if (z2 == 0.0)
	{
		// Find nearest point and use that.
		z2 = (ddgCurrentVBuf->vbuf[t2[0]])[2];
		for (i=1; i < n; i++)
		{
		if ((ddgCurrentVBuf->vbuf[t2[i]])[2] < z2)
			z2 = (ddgCurrentVBuf->vbuf[t2[i]])[2];
		}
		ddgZcache[t2[0]/n] = z2;
	}
	// Compare the two points.
	if      ( z1 < z2 ) return -1;
	else if ( z2 < z1 ) return 1;
	return 0;
}

void ddgVArray::sort(void)
{
	// Initialize the zcache.
	unsigned int n = _type;
	int i;
	for (i = 0; i < ZCACHESIZE; i++)
		ddgZcache[i] = 0;
	ddgCurrentVBuf = this;
	qsort((void*)ibuf,_inum/n,n*sizeof(int),compareObjects);
}
