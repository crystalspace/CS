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

#include "struct/ddgtmesh.h"
#include "struct/ddgbbox.h"
#include "struct/ddgecos.h"
#include "struct/ddgimage.h"

ddgEcoSystem::~ddgEcoSystem(void)
{
	unsigned int i = 0;
	while (i < _size)
	{
		delete ecoBlocks[i];
		i++;
	}
	delete[] ecoBlocks;
}

bool ddgEcoSystem::init(void)
{
	if (blockNum)
		return ddgFailure;

	unsigned int _blockNumSQRT = 16;
	blockNum = _blockNumSQRT*_blockNumSQRT;
	unsigned int i = 0, j = 0, k = _size / (blockNum), e = 0;
	ddgVector3 p, s, wsize;
	// Size of the world.
	wsize= _bbox->size();
	// Size of the eco blocks.
	s.set(wsize[0]/_blockNumSQRT,-199999,wsize[2]/_blockNumSQRT);
	ecoBlocks = new ddgEcoBlock*[blockNum];
	ddgMemorySet(ddgEcoBlock*,blockNum);
	ddgMemorySet(ddgEcoBlock,blockNum);
	for (i = 0; i < _blockNumSQRT; i++)
	{
		for (j = 0; j < _blockNumSQRT; j++)
		{
			p.set(i*wsize[0]/_blockNumSQRT,99999,j*wsize[2]/_blockNumSQRT);
   			ecoBlocks[e] = new ddgEcoBlock(this,k,new ddgBBox(p,p+s));
			ecoBlocks[e]->init(this);
			e++;
		}
		ddgConsole::progress("Populating eco system",i*_blockNumSQRT,blockNum);
	}
	ddgConsole::progress("Populating eco system",1,1);
	ddgConsole::end();
	return ddgSuccess;
}

bool ddgEcoBlock::init( ddgEcoSystem *e )
{
	unsigned int i = 0, j = 0;
	bool		seedSet = false;
	pos = new ddgVector3[_size];
	ddgMemorySet(ddgVector3,_size);
	ddgVector3 bsize = bbox->size(),
				*pmin = bbox->min(),
				*pmax = bbox->max();
	// Brute for approach to seed trees where ever the ecomap indicates the objects
	// can appear.
	while (i < _size && j < _size *4)
	{
		seedSet = true;
		pos[i][0] = (*pmin)[0] + (float)(rand() % ((int)bsize[0]*1000))/1000.0;
		pos[i][2] = (*pmin)[2] + (float)(rand() % ((int)bsize[2]*1000))/1000.0;
		pos[i][1] = e->_mesh ? e->_mesh->height(pos[i][0],pos[i][2]) : 0.0;
		j++;
		// See if this is a valid location for the item.
		if ( e->_ecoMap )
		{
			unsigned char ep;
			e->_ecoMap->get((unsigned short)pos[i][0],(unsigned short)pos[i][2],&ep);
			if (ep < e->_min || ep > e->_max)
				seedSet = false;
		}

		if (seedSet)
		{
			(*pmax)[1] = ddgUtil::max((*pmax)[1],pos[i][1]+e->_osize[1]);
			(*pmin)[1] = ddgUtil::min((*pmin)[1],pos[i][1]);
			i++;
		}
	}
	// We may not have been able to reach the required number of objects due to
	// land scape restrictions.
	_size = i;
	return ddgSuccess;
}

unsigned int ddgEcoBlock::vis( ddgPlane *frustrum, ddgVector2 pos)
{
	_vis = 0;
	if (_size)
	{
		ddgVector2 c = bbox->centre();
		// See if the triangle is too far from the camera.  Ingore height component.
		c -= pos;

		// Do a rough distance test and optionally a true distance test.
		if (c[0] > _es->_farClip || c[0] < - _es->_farClip
		  ||c[1] > _es->_farClip || c[1] < - _es->_farClip
		  ||c.sizesq() > _es->_farClipSQ
		  )
			_vis = ddgOUT;
		else _vis = bbox->isVisible( frustrum,5);
	}
	return _vis;
}

