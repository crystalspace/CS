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
#ifdef DDG
#include "ddgsplay.h"
#include "ddgtmesh.h"
#include "ddgbtree.h"
#include "ddggeom.h"
#include "ddgmatrx.h"
#else
#include "sysdef.h"
#include "csterr/ddgsplay.h"
#include "csterr/ddgtmesh.h"
#include "csterr/ddgbtree.h"
#endif

/*
There *are* some more complications having to do with the frustum 
OUT labels.  When triangles transition from OUT to not OUT or 
back this requires some checking for merge/split queue updates 
for the tris that transition and their parents/kids/neighbors. 
This is because I don't waste effort splitting/merging tris 
outside the frustum unless continuity dictates.  Mergable diamonds 
with one or more OUT labels in the two tris are still on the merge 
queue, but with the lowest possible priority (so they get merged first). 

Any leaf tri with an OUT label is taken off the split queue. 
Another minor boundary condition is that finest-resolution leaves 
are not put on the split queue.  These cases are dealt with by (1) 
avoiding putting any OUT or finest-res leaf on the split queue (just 
check right before inserting); (2) any split-queue member that 
goes OUT gets removed from the split queue; (3) any non-finest 
leaf that transitions to not OUT gets put on the split queue. 
*/

// Constant used for identifying neighbours.
const	unsigned int ddgNINIT = 0xFFFFFFFF;

ddgVector3 ddgTBinTree::_unit;

static ddgTriIndex	*recurStack = NULL;	// mesh->_maxLevel
static unsigned int recurStackSize = 0;
#define recurStackPush(a)	{ ddgAsserts( recurStackSize <= _mesh->maxLevel(),"Recursion stack overflow!");recurStack[recurStackSize] = a; recurStackSize++; }
#define recurStackPop(a)	{ ddgAsserts( recurStackSize > 0,"Recursion stack underflow!");recurStackSize--; a = recurStack[recurStackSize]; }

/// s = size along one edge of mesh, mesh is 'square'.
ddgTBinTree::ddgTBinTree( ddgTBinMesh *m, ddgTreeIndex i, ddgHeightMap* h, unsigned short *idx, int dr, int dc, bool mirror )
{
	_index = i;
    ddgAssert(m);
	_mesh = m;
    _dr = dr;
    _dc = dc;
	heightMap = h;
	normalIdx = idx;
    _mirror = mirror;
	_pNeighbourTop = NULL;
	_pNeighbourLeft = NULL;
	_pNeighbourDiag = NULL;
	_tri = new ddgMTri[triNo()+2];
	ddgAsserts(_tri,"Failed to Allocate memory");
	ddgMemorySet(ddgMTri,triNo()+2);

	if (!recurStack)
	{
		recurStack = new ddgTriIndex[2*_mesh->maxLevel()];
		ddgMemorySet(ddgTriIndex,2*_mesh->maxLevel());
	}

}

ddgTBinTree::~ddgTBinTree(void)
{
	delete [] _tri;
}

static float maxTh = 0;

/** Initialize the bintree structure including
 *  the wedges and other mesh values.
 */
bool ddgTBinTree::init( void )
{
	//          va     1 size,0
	// Dummy entries.

	float th = split(1,0,triNo(),triNo()+1,1);
	ddgAssert( maxTh < 0xFFFF);

	tri(0)->thick( th);
	tri(1)->thick( th);
	tri(triNo())->thick(th);
	tri(triNo()+1)->thick(th); 

	return ddgSuccess;
}

/// The bounding box in screen space.
ddgBBox         *_camBBox = NULL;
/// The field of view.
float           _tanHalfFOV   = 1.0;

#ifdef DDG
/// World to camera space transformation matrix must be initialized before calling calculate.
ddgMatrix4 _wtoc;
ddgPlane		*_frustrum;
void ddgTBinTree::transform( ddgVector3 *vino, ddgVector3 *vouto )
{
	float *m = _wtoc, *vin = (float*)vino, *vout = (float*)vouto;
    // Convert the world coordinates to camera coordinates.
    // Perform matrix multiplication.
    vout[0] = m[0]*vin[0]+m[4]*vin[1]+m[8]*vin[2]+m[12];
    vout[1] = m[1]*vin[0]+m[5]*vin[1]+m[9]*vin[2]+m[13];
    vout[2] = m[2]*vin[0]+m[6]*vin[1]+m[10]*vin[2]+m[14];
}

void ddgTBinTree::initWtoC( ddgMatrix4 *wtoc, ddgBBox *camClipBox, float fov, ddgPlane frustrum[6])
{
	ddgAssert(camClipBox);
	int i;
    // Set world to camera transformation matrix.
	for(i = 0; i < 16; i++)
	_wtoc.v[i] = wtoc->v[i];

    // Set the camera bounding box.
    _camBBox = camClipBox;
    // Set the field of view.
    _tanHalfFOV = tan(ddgAngle::degtorad(fov/2.0));
	_frustrum = frustrum;
	ddgVector3 ev0,ev1;
	ddgTBinTree::transform( ddgVector3(0,0,0), ev0 );
	ddgTBinTree::transform( ddgVector3(0,1,0), ev1 );
	_unit =  ev1 - ev0;
}
#else


void ddgTBinTree::initWtoC( ddgBBox *camClipBox )
{
	csVector3 ev0,ev1,tmp;
    // Set the camera bounding box.
    _camBBox = camClipBox;

	ev0 = transformer (csVector3 (0,0,0));
	ev1 = transformer (csVector3 (0,1,0));
	tmp = ev1 - ev0;
	_unit = tmp;
}
#endif

/**
 *  Split this triangle into 2, and return the thickness.
 *  Splitting occurs along this point's parent va. 
 *  Passed in are the triangles which carry the points
 *  that describe the current triangle.
 *  va is immediate parent.
 *  v1 and v0 are the left and right vertices of va.
 *  This method is only called at initialization.
 */
float ddgTBinTree::split( unsigned int level,
					 ddgTriIndex va,	// Top
					 ddgTriIndex v0,	// Left
					 ddgTriIndex v1,	// Right
					 ddgTriIndex vc)	// Centre
{
	float th0 = 0, th1 = 0;
	// See if we are not at the bottom.
	if (level < maxLevel())
	{
		th0 = split(level+1,vc,va,v0,left(vc)),
		th1 = split(level+1,vc,v1,va,right(vc));
	}

	// Get true height at this location.
   	float Z_vc = height(vc);
	// Get avg height at this location.
	float Zt_vc = 0.5f*((float)(height(v0) + height(v1)));
	// Calculate thickness of this wedgie.
	tri(vc)->thick( ddgUtil::max(th0,th1) + (ddgUtil::diff(Z_vc,Zt_vc))/2.0);

	DDG_BCLEAR(tri(vc)->_state, ddgMTri::SF_SQ);
	DDG_BCLEAR(tri(vc)->_state, ddgMTri::SF_MQ);
#ifdef _DEBUG
	maxTh = ddgUtil::max(maxTh,ddgUtil::max(th0,th1) + ddgUtil::diff(Z_vc,Zt_vc)/2.0);
#endif
	return tri(vc)->thick();
}

/// Remove triangle i.
void ddgTBinTree::removeSQ(ddgTriIndex tindex )
{
    ddgAsserts(tindex >=0 && tindex <= triNo()+2,"Index is out of range.");
    ddgAsserts(DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ),"Triangle is not in split queue.");
    ddgAsserts(!_mesh->merge() || !DDG_BGET(tri(tindex)->_state, ddgMTri::SF_MQ),"Triangle also in merge queue!");

	DDG_BCLEAR(tri(tindex)->_state, ddgMTri::SF_SQ);

	unsigned int p = tri(tindex)->priority();
	_mesh->tcache()->freeNode(tri(tindex)->tcacheId);
	// If we are not a leaf node...
	if (tindex < _mesh->leafTriNo())
	{
	// If we were visible and not a leaf node actually remove us.
		_mesh->remCountIncr();
//	ddgAsserts(tindex == 1 || p < 2 || p < tri(parent(tindex))->priority(),"Child has greater priority than parent!");
		_mesh->qs()->remove(_index,tindex,p);
	}
	_queueTri--;

	//  then remove it from the merge queue if it is there.
	if (_mesh->merge())
		removeMQ(parent(tindex));
}

/// Insert triangle i into queue.
void ddgTBinTree::insertSQ(ddgTriIndex tindex)
{
    ddgAsserts(tindex >=0 && tindex < triNo()+2,"Index is out of range.");
    ddgAsserts(!DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ),"Triangle already in split queue.");

	if (_mesh->merge())
	{
		removeMQ(tindex);
	}
	DDG_BSET(tri(tindex)->_state, ddgMTri::SF_SQ);
	tri(tindex)->reset();
	tri(tindex)->resetPriorityDelay();

	unsigned int p = priority(tindex);
	tri(tindex)->tcacheId = _mesh->tcache()->set(_index,tindex);
	// If we are not a leaf node...
	if (tindex < _mesh->leafTriNo())
	{
		_mesh->insCountIncr();
		// Find insertion point.
		ddgAsserts(tindex == 1 || p < 2 || p < priority(parent(tindex)),"Child has greater priority than parent!");
		_mesh->qs()->insert(_index,tindex,p);
	}
	_queueTri++;

	if (_mesh->merge())
	{
		if (tindex  > 1 && isDiamond(parent(tindex)))
			insertMQ(parent(tindex));
	}
}

/*
 Remove triangle i from merge queue if it is in the queue.
 If it is not, but its neigbour is, then remove it instead.
 If neither is in the merge queue, do nothing.
*/
void ddgTBinTree::removeMQ(ddgTriIndex tindex)
{
    ddgAsserts(tindex >=0 && tindex < triNo()+2,"Index is out of range.");
	// See which is actually in the queue and remove it.
	ddgTriIndex n = neighbour(tindex);
	ddgTBinTree *nt = n ? neighbourTree(tindex) : 0;

	if (DDG_BGET(tri(tindex)->_state, ddgMTri::SF_MQ))
	{
		ddgAsserts(!DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ),"Triangle is also in split queue!");
		ddgAsserts(!n || !DDG_BGET(nt->tri(n)->_state, ddgMTri::SF_MQ),"Cant both be in merge queue");
		unsigned int p = tri(tindex)->priority();
//		ddgAsserts(tindex == 1 || p < 2 || p < tri(parent(tindex))->priority(),"Child has greater priority than parent!");

		_mesh->qm()->remove(_index,tindex,p);
		_mesh->remCountIncr();
		DDG_BCLEAR(tri(tindex)->_state, ddgMTri::SF_MQ);
	}
	else if (n && DDG_BGET(nt->tri(n)->_state, ddgMTri::SF_MQ))
	{
		nt->removeMQ(n);
	}
}

/*
 Insert triangle i into queue if it isn't already there.
 */
void ddgTBinTree::insertMQ(ddgTriIndex tindex)
{
    ddgAsserts(tindex >=0 && tindex < triNo()+2,"Index is out of range.");
	// If we have a neighbour,
	// Insert the one with higher priority into the queue.
	ddgTriIndex n = neighbour(tindex);
	ddgTBinTree *nt = n ? neighbourTree(tindex) : 0;

	if (n)
		nt->tri(n)->resetPriorityDelay();
	tri(tindex)->resetPriorityDelay();
	unsigned int p = priority(tindex);

	if (n && p < nt->priority(n))
	{
		if (!DDG_BGET(nt->tri(n)->_state, ddgMTri::SF_MQ))
			nt->insertMQ(n);
	}
	else if (!DDG_BGET(_tri[tindex]._state, ddgMTri::SF_MQ))
	{
	    ddgAsserts(!DDG_BGET(_tri[tindex]._state, ddgMTri::SF_SQ),"Triangle is also in Split Queue!");
		ddgAsserts(!n|| !DDG_BGET(nt->tri(n)->_state, ddgMTri::SF_MQ),"Cant both be in merge queue");
		_mesh->insCountIncr();

		ddgAsserts(tindex == 1 || p < 2 || p < priority(parent(tindex)),"Child has greater priority than parent!");

		_mesh->qm()->insert(_index,tindex,p);
		DDG_BSET(tri(tindex)->_state, ddgMTri::SF_MQ);
	}
}

/*
 * Split a triangle while keeping the mesh consistent.
 * there should be no T junctions. 'i' is a triangle index number.
 */
void ddgTBinTree::forceSplit( ddgTriIndex tindex)
{
	ddgTriIndex l = left(tindex), r = right(tindex);
	// If current triangle is not in the queue,
	// split its parent, to get it into the queue.
	if (!DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ))
	{
        // If we are a top level triangle and we are not in the tree.
        // Then we cannot force the parent to split.
        if (tindex < 2)
            return;
        ddgAssert(tindex > 1);
        // TODO, optimize to not insert ourselves (tindex),
        // because we need to remove ourselves again anyways.
		forceSplit(parent(tindex));
	}
    // We are in the queue at this time, remove ourselves.
	if (DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ))
	{
		removeSQ(tindex);
	}
    // Insert our the left and right child.
	// Invalidate their state.
	insertSQ(l);
	insertSQ(r);

    // If this triangle has a merge neighbour, it also needs to be in the split queue.
	if (neighbour(tindex))
	{
		// Make sure its children are in the queue.
		if (!DDG_BGET(neighbourTree(tindex)->tri(left(neighbour(tindex)))->_state, ddgMTri::SF_SQ)
		 || !DDG_BGET(neighbourTree(tindex)->tri(right(neighbour(tindex)))->_state, ddgMTri::SF_SQ))
		{
			neighbourTree(tindex)->forceSplit(neighbour(tindex));
		}
	}
}

/// 'i' is a triangle index.
void ddgTBinTree::forceMerge( ddgTriIndex tindex)
{
    // This triangle is in the merge queue.
    ddgAssert(DDG_BGET(tri(tindex)->_state, ddgMTri::SF_MQ));
    // This triangle's neighbour must not be in the merge queue.
    ddgAssert(!neighbour(tindex) || !DDG_BGET(neighbourTree(tindex)->tri(neighbour(tindex))->_state, ddgMTri::SF_MQ));
    // The children are in the split queue.
    ddgAssert(isDiamond(tindex));
	// Remove its children and neighbours children.
	removeSQ(left(tindex));
	removeSQ(right(tindex));
	// Insert self and neighbour.
	insertSQ(tindex);
	ddgTriIndex n = neighbour(tindex);
	ddgTBinTree *nt = n ? neighbourTree(tindex) : 0;
    if (n)
    {
		nt->removeSQ(left(n));
		nt->removeSQ(right(n));
		nt->insertSQ(n);
    }
}

// Return if a triangle is part of a mergeble diamond.
// It is if its children and its neighbour's children are in the split queue.
// If true, then this triangle's parent or neighbour should be in the merge queue.
bool ddgTBinTree::isDiamond(ddgTriIndex tindex)
{

    ddgTriIndex n = neighbour(tindex);		// Uncle.
	ddgTBinTree *nt = n ? neighbourTree(n) : NULL;
	if ( n < ddgNINIT 
       && DDG_BGET(tri(left(tindex))->_state, ddgMTri::SF_SQ) && DDG_BGET(tri(right(tindex))->_state, ddgMTri::SF_SQ)
       && (!n || (DDG_BGET(nt->tri(left(n))->_state, ddgMTri::SF_SQ)
               && DDG_BGET(nt->tri(right(n))->_state, ddgMTri::SF_SQ))))
		return true;

	return false;
}

/**
 * Return if a mergeble diamond is within the viewing frustrum.
 * Returns true if at least one child is partially visible.
 */
bool ddgTBinTree::isDiamondVisible(ddgTriIndex tindex)
{

    ddgTriIndex f = parent(tindex),		// Father.
				 u = neighbour(f);		// Uncle.
	ddgAssert(u<ddgNINIT);
	if (!visible(tindex) && !u)
		return false;
	if (!u)
		return true;
	if (!neighbourTree(u)->visible(u))
		return false;

	return true;
}


void ddgTBinTree::visibility(ddgTriIndex tindex)
{
    ddgAssert(_camBBox);
	ddgAsserts(tindex <= triNo(), " No leaf node was found in the queue!");

	recurStackPush(tindex);
	do
	{
		recurStackPop(tindex);
		ddgTriIndex tva = parent(tindex),
			tv0 = v0(tindex),
			tv1 = v1(tindex);

		ddgVisState oldVis = tri(tindex)->vis();
		tri(tindex)->reset();  // Reset all but queue status.

		if (fullyvisible(tva)||(!visible(tva))) 
		// If parent is completely inside or completely outside, inherit.
		{
			tri(tindex)->vis( tri(tva)->vis());
		}
		// We dont know so analyze ourselves.
		else
		{
			_mesh->visCountIncr();
			// Calculate bounding box of wedgie.
			ddgBBox		bbox;
			ddgVector3 *pmin = bbox.min();
			ddgVector3 *pmax = bbox.max();

			ddgVector3 vu, vl;
#if 1
			ddgVector3 th;
			ddgVector3 *ptri;
			th = _unit;
			th *= tri(tindex)->thick();

			// Calculate the parent.
			ptri = pos(tva);
			vu = *ptri+th;
			vl = *ptri - th;

			*pmin = vu;
			*pmax = vu;
#ifdef DDG
			pmin->minimum(vl);
			pmax->maximum(vl);
#else
			csMath3::SetMinMax(vl,*pmin,*pmax);
#endif
			// Calculate the left and right vertex.
			ptri = pos(tv0);
			vu = *ptri+th;
			vl = *ptri - th;
#ifdef DDG
			pmin->minimum(vu);
			pmax->maximum(vu);
			pmin->minimum(vl);
			pmax->maximum(vl);
#else
			csMath3::SetMinMax(vu,*pmin,*pmax);
			csMath3::SetMinMax(vl,*pmin,*pmax);
#endif

			ptri = pos(tv1);
			vu = *ptri+th;
			vl = *ptri-th;
#ifdef DDG
			pmin->minimum(vu);
			pmax->maximum(vu);
			pmin->minimum(vl);
			pmax->maximum(vl);
#else
			csMath3::SetMinMax(vu,*pmin,*pmax);
			csMath3::SetMinMax(vl,*pmin,*pmax);
#endif

			// Pmin and Pmax now define a bounding box that contains the wedgie.
			// Determine if this bounding box is visible in screen space.
#ifdef DDG
			if ((*pmax)[2]< _mesh->farClip() || (*pmin)[2] > _mesh->nearClip())
#else
			if ((*pmin)[2]> _mesh->farClip() || (*pmax)[2] < _mesh->nearClip())
#endif
			{
				tri(tindex)->vis(ddgOUT);
			}
			else
			{
				ddgClipFlags cf = _camBBox->visibleSpace(bbox,_tanHalfFOV );
				if (DDG_BGET(cf, DDGCF_ALLOUT))
					tri(tindex)->vis(ddgOUT);
				else if (DDG_BGET(cf,DDGCF_ALLIN))
					tri(tindex)->vis(ddgIN);
				else
					tri(tindex)->vis(ddgPART);
			}
#endif
			/* World space clipping.*/
#if 0
			// Bounding box is defined by points v0 and v1.
			vertex(tv0,pmin);
			vertex(tv0,pmax);
			vertex(tv1,&vl);
			pmin->minimum(vl);
			pmax->maximum(vl);
			vertex(tva,&vu);
			pmin->minimum(vu);
			pmax->maximum(vu);
			// Adjust box by wedgie thickness.
			pmin->v[1] = pmin->v[1] - tri(tindex)->thick();
			pmax->v[1] = pmax->v[1] + tri(tindex)->thick();
			tri(tindex)->vis( bbox.isVisible(_frustrum));
#endif
		}
		// Check if we need to reset the priority delay
		if ((oldVis == ddgOUT) && visible(tindex))
		{
			tri(tindex)->resetPriorityDelay();
		}
		// This will only go as far as the leaves which are currently in the split queue.
	if (!DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ))
		{
			recurStackPush(left(tindex));
			recurStackPush(right(tindex));
		}
	}
	while (recurStackSize);
}

// Recusively update priorities.
void ddgTBinTree::priorityUpdate(ddgTriIndex tindex)
{ 
	ddgAsserts(tindex <= triNo(), " No leaf node was found in the queue!");
	ddgAsserts(recurStackSize == 0, "Recursion stack was not empty!");

	recurStackPush(tindex);
	do
	{
		recurStackPop(tindex);
		// If priority information is not valid.
		if (!tri(tindex)->getPriorityDelay()
			&& (DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ) || DDG_BGET(tri(tindex)->_state, ddgMTri::SF_MQ)))
		{
			unsigned int op = tri(tindex)->priority();
			unsigned int p = priority(tindex);
			ddgAsserts(tindex == 1 || p < 2 || p < priority(parent(tindex)),"Child has greater priority than parent!");

			if (op != p)
			{
				// If we are not a leaf node...
				if (tindex < _mesh->leafTriNo() && DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ))
				{
					_mesh->movCountIncr();
					_mesh->qs()->remove(_index,tindex,op);
					_mesh->qs()->insert(_index,tindex,p);
				}
				else if (DDG_BGET(tri(tindex)->_state, ddgMTri::SF_MQ) )
				{
					_mesh->movCountIncr();
					_mesh->qm()->remove(_index,tindex,op);
					_mesh->qm()->insert(_index,tindex,p);
				}
			}
		}
		// This will only go as far as the leaves which are currently in the split queue.
		if (!DDG_BGET(tri(tindex)->_state, ddgMTri::SF_SQ) )
		{
			recurStackPush(left(tindex));
			recurStackPush(right(tindex));
		}
	}
	while(recurStackSize);
	ddgAsserts(recurStackSize == 0, "Recursion stack was not empty!");
}

// Priority of Triangle tindex.
unsigned short ddgTBinTree::priority(ddgTriIndex tvc)
{
    ddgAssert(tvc <= triNo());

	// Wedgie's priority is uptodate.
	if (tri(tvc)->getPriorityDelay())
	{
		return tri(tvc)->priority();
	}

	// Ensure triangle's visibility is uptodate.
	if (tri(tvc)->vis() == ddgUNDEF)
	{
		visibility(tvc);
	}

	// Wedgie is not visible.
	if (!visible(tvc))
	{
		tri(tvc)->setPriorityDelay( 1 );
		return tri(tvc)->priority( 0 );
	}

	// Our priority data is out of date, recalculate it.
	ddgTriIndex tva = parent(tvc),
		tv0 = v0(tvc),
		tv1 = v1(tvc);

    // Get screen space error metric of wedgie thickness.
	// Note were are switching y & z since ROAM paper is z
	// oriented for thickness and we are y oriented.

	// Calculate the maximum screen space thickness.
	ddgVector3 th = _unit;
	th *= tri(tvc)->thick();

	// a,b,c = thickness vector in camera space.
	// p = screen right
	// q = screen down
	// r = into screen
	//
	// m = 2/(r^2-c^2) three times (once per vertex), and take the max
	// of the three.  You also compute m = ((ar-cp)^2+(br-cq)^2) 
	// three times and take the max.  You then take the first max times
	// the square root of the second.
	// max as your distortion bound and priority.

	float result = ddgMAXPRI;
#define ddgEPSILON 0.00001

	float a = th[0],
		  b = th[1],
		  c = th[2], cc,
		  p[3], tp[3],
		  q[3], tq[3],
		  r[3],
		  rr[3],
		  d[3], dd,
		  m[3], mm;
	cc = c*c;

	// Check for invalid values.
    r[0] = (*pos(tv0))[2];
	rr[0] = r[0]*r[0];
	if (rr[0] <= cc + ddgEPSILON)
		goto skip;
    r[1] = (*pos(tv1))[2];
	rr[1] = r[1]*r[1];
	if (rr[1] <= cc + ddgEPSILON)
		goto skip;
	r[2] = (*pos(tva))[2];
	rr[2] = r[2]*r[2];
	if (rr[2] <= cc + ddgEPSILON)
		goto skip;

	// Perform expensive screen space error calculations.
    p[0] = (*pos(tv0))[0];
    q[0] = -1*(*pos(tv0))[1];
	tp[0] = a*r[0]-c*p[0];
	tq[0] = b*r[0]-c*q[0];
	d[0] = (sq(tp[0])+sq(tq[0]));
	m[0] = 2.0 / (rr[0] - cc);

    p[1] = (*pos(tv1))[0];
    q[1] = -1*(*pos(tv1))[1];
	tp[1] = a*r[1]-c*p[1];
	tq[1] = b*r[1]-c*q[1];
	d[1] = (sq(tp[1])+sq(tq[1]));
	m[1] = 2.0 / (rr[1] - cc);

	p[2] = (*pos(tva))[0];
	q[2] = -1*(*pos(tva))[1];
	tp[2] = a*r[2]-c*p[2];
	tq[2] = b*r[2]-c*q[2];
	d[2] = (sq(tp[2])+sq(tq[2]));
	m[2] = 2.0 / (rr[2] - cc);

	// Find the maximum of both d and m.
    dd = ddgUtil::max(d[2],ddgUtil::max(d[0],d[1]));
    mm = ddgUtil::max(m[2],ddgUtil::max(m[0],m[1]));
	// Calculate priority.
	result = sqrt(dd) * mm;

	// Map the priority into a range from 0 to ddgMAXPRI.
	result = ddgUtil::clamp(result*1000,1,ddgMAXPRI);
skip:
	_mesh->priCountIncr();

	// This puts in invalid values unless parents priority is always updated.
	unsigned short tpriority = (unsigned short)result;

	if (tvc > 1 && tpriority > 1)
	{
		unsigned short ppriority = priority(parent(tvc)); 
		if (ppriority <= tpriority)
		{
			tpriority = ppriority;
			if (tpriority > 1)
				tpriority--;
		}
	}

	tri(tvc)->priority(tpriority);

	// Calculate recalc delay.
	float zMin = pos(tva,2);
	if ( zMin < pos(tv0,2)) zMin = pos(tv0,2);
	if ( zMin < pos(tv1,2)) zMin = pos(tv1,2);

	// Z is negative decreasing with distance.
	int v = 1;
	if (_mesh->progDist() != 0)
	{
		v = int(zMin / _mesh->progDist());
		v = int(ddgUtil::clamp(v,1,15));
	}
	tri(tvc)->setPriorityDelay(v);

	return tri(tvc)->priority();
}

// Get height of bintree at a real location.
float ddgTBinTree::treeHeight(unsigned int r, unsigned int c, float dx, float dz)
{
	// Is this triangle mirrored.
    float m = (dx+dz > 1.0) ? 1.0 : 0.0;
    float h1 = height(ddgTriIndex(r+ m), ddgTriIndex(c + m));
    float h2 = height(r,  c+1);
    float h3 = height(r+1,c);
	ddgVector3 p0(m,h1,m),
			   p1(0,h2,1),
			   p2(1,h3,0),
			   v(dx,0,dz);
	ddgPlane p(p0,p1,p2);
	p.projectAlongY(&v);

    return v[1];
}
#ifdef _DEBUG
bool ddgTBinTree::verify( ddgTriIndex tindex )
{
	if  (_mesh->merge() && tindex < _mesh->triNo()/2 && isDiamond(tindex))
	{
		ddgTriIndex n = neighbour(tindex);
		ddgTBinTree *nt = n ? neighbourTree(tindex) : 0;
		if (!DDG_BGET(tri(tindex)->_state,ddgMTri::SF_MQ) &&
			(n&& !DDG_BGET(nt->tri(n)->_state,ddgMTri::SF_MQ)))
			return true;
	}
	return false;
}
#endif