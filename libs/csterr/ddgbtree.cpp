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

//
#include "sysdef.h"
#include "csterr/ddgsplay.h"
#include "csterr/ddgtmesh.h"
#include "csterr/ddgbtree.h"

extern void transformer(csVector3 vin, csVector3 *vout);

// Constant used for identifying brothers.
const	unsigned int ddgNINIT = 0xFFFFFFFF;

// ----------------------------------------------------------------------

csVector3 ddgTBinTree::_unit;

#if 0
ostream& operator << ( ostream&s, ddgTBinTree *b )
     {
	  return s << 
	 (void*)b << ":" << 
	 (b->mirror() ? "m":"-") << " T" <<
	 (void*)b->pNeighbourTop() << " L" << 
	 (void*)b->pNeighbourLeft() << " D" << 
	 (void*)b->pNeighbourDiag() << " (" <<
	 b->dr() << "," << 
	 b->dc() << ")";
}

ostream& operator << ( ostream&s, ddgTBinTree b )
     { return s << 
	 (void*)&b << "	" << 
	 (b.mirror() ? "m":"-") << " T" <<
	 (void*)b.pNeighbourTop() << " L" << 
	 (void*)b.pNeighbourLeft() << " D" << 
	 (void*)b.pNeighbourDiag() << " (" <<
	 b.dr() << "," << 
	 b.dc() << ")";
}
#endif

/// s = size along one edge of mesh, mesh is 'square'.
ddgTBinTree::ddgTBinTree( ddgTBinMesh *m, ddgTreeIndex i, ddgHeightMap* h, int dr, int dc, bool mirror )
{
    _init = false;
	_index = i;
	_mesh = m;
    _dr = dr;
    _dc = dc;
	heightMap = h;
    _mirror = mirror;
	_pNeighbourTop = NULL;
	_pNeighbourLeft = NULL;
	_pNeighbourDiag = NULL;
	_tri = new ddgMTri[triNo()+2];
	// Initialize split queue with top triangle.
	init();
	// Top level triangles.
	tri(0)->_state.all = 0;
	tri(1)->_state.all = 0;
	tri(0)->reset();
	tri(1)->reset();
	tri(triNo())->_state.all = 0;
	tri(triNo()+1)->_state.all = 0;
	tri(triNo())->reset();
	tri(triNo()+1)->reset();
	insertSQ(1);
}

ddgTBinTree::~ddgTBinTree(void)
{
	delete _tri;
}

static float maxTh = 0;

/** Initialize the bintree structure including
 *  the wedges and other mesh values.
 */
bool ddgTBinTree::init( void )
{
	// Avoid being called twice.
	if (_init)
		return false;
	else
		_init = true;

	float th = split(1,0,triNo(),triNo()+1,1);
	ddgAssert( maxTh < 0xFFFF);

	tri(0)->thick( th);
	tri(1)->thick( th);
	tri(triNo())->thick(th);
	tri(triNo()+1)->thick(th); 

	return false;
}

void ddgTBinTree::initWtoC( ddgTBinMesh * /*mesh*/)
{
	csVector3 ev0,ev1,tmp;
//	float * meshWtoC = mesh->wtoc();
//	int i;
//	for(i = 0; i < 16; i++)
//	_wtoc.m[i] = meshWtoC[i];

	transformer( csVector3(0,0,0), &ev0 );
	transformer( csVector3(0,1,0), &ev1 );
	tmp = ev1 - ev0;
	_unit = tmp;
}

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

	tri(vc)->_state.flags.sq = false;
	tri(vc)->_state.flags.mq = false;
#ifdef _DEBUG
	maxTh = ddgUtil::max(maxTh,ddgUtil::max(th0,th1) + ddgUtil::diff(Z_vc,Zt_vc)/2.0);
#endif
	return tri(vc)->thick();
}




/// Remove triangle i.
void ddgTBinTree::removeSQ(ddgTriIndex tindex )
{
    ddgAsserts(tindex >=0 && tindex <= triNo()+2,"Index is out of range.");
    ddgAsserts(tri(tindex)->_state.flags.sq,"Triangle is not in split queue.");
    ddgAsserts(!_mesh->merge() || !tri(tindex)->_state.flags.mq,"Triangle also in merge queue!");

	tri(tindex)->_state.flags.sq = false;
	// If we were visible and not a leaf node actually remove us.
	_mesh->remCountIncr();
	unsigned int p = tri(tindex)->priority();
//	ddgAsserts(tindex == 1 || p < 2 || p < tri(parent(tindex))->priority(),"Child has greater priority than parent!");

	_mesh->qs()->remove(_index,tindex,p);
	// If we were visible, reduce the count.
	if (!tri(tindex)->_vis.flags.allout)
	{
		ddgAssert(_visTri > 0);
		_visTri--;
	}

	//  then remove it from the merge queue if it is there.
	if (_mesh->merge())
		removeMQ(parent(tindex));
}

/// Insert triangle i into queue.
void ddgTBinTree::insertSQ(ddgTriIndex tindex)
{
    ddgAsserts(tindex >=0 && tindex < triNo()+2,"Index is out of range.");
    ddgAsserts(!tri(tindex)->_state.flags.sq,"Triangle already in split queue.");

	if (_mesh->merge())
	{
		removeMQ(tindex);
	}
	tri(tindex)->_state.flags.sq = true;
	reset(tindex);
	_mesh->insCountIncr();
	tri(tindex)->resetPriorityDelay();
	// Find insertion point.
	unsigned int p = priority(tindex);
	ddgAsserts(tindex == 1 || p < 2 || p < priority(parent(tindex)),"Child has greater priority than parent!");

	_mesh->qs()->insert(_index,tindex,p);
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

	if (tri(tindex)->_state.flags.mq)
	{
		ddgAsserts(!tri(tindex)->_state.flags.sq,"Triangle is also in split queue!");
		ddgAsserts(!n || !nt->tri(n)->_state.flags.mq,"Cant both be in merge queue");
		unsigned int p = tri(tindex)->priority();
//		ddgAsserts(tindex == 1 || p < 2 || p < tri(parent(tindex))->priority(),"Child has greater priority than parent!");

		_mesh->qm()->remove(_index,tindex,p);
		_mesh->remCountIncr();
		tri(tindex)->_state.flags.mq = false;
	}
	else if (n && nt->tri(n)->_state.flags.mq)
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
		if (!nt->tri(n)->_state.flags.mq)
			nt->insertMQ(n);
	}
	else if (!_tri[tindex]._state.flags.mq)
	{
	    ddgAsserts(!_tri[tindex]._state.flags.sq,"Triangle is also in Split Queue!");
		ddgAsserts(!n|| !nt->tri(n)->_state.flags.mq,"Cant both be in merge queue");
		_mesh->insCountIncr();

		ddgAsserts(tindex == 1 || p < 2 || p < priority(parent(tindex)),"Child has greater priority than parent!");

		_mesh->qm()->insert(_index,tindex,p);
		tri(tindex)->_state.flags.mq = true;
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
	if (!tri(tindex)->_state.flags.sq)
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
	if (tri(tindex)->_state.flags.sq)
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
		if (!neighbourTree(tindex)->tri(left(neighbour(tindex)))->_state.flags.sq
		 || !neighbourTree(tindex)->tri(right(neighbour(tindex)))->_state.flags.sq)
		{
			neighbourTree(tindex)->forceSplit(neighbour(tindex));
		}
	}
}

/// 'i' is a triangle index.
void ddgTBinTree::forceMerge( ddgTriIndex tindex)
{
    // This triangle is in the merge queue.
    ddgAssert(tri(tindex)->_state.flags.mq);
    // This triangle's neighbour must not be in the merge queue.
    ddgAssert(!neighbour(tindex) || !neighbourTree(tindex)->tri(neighbour(tindex))->_state.flags.mq);
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
       && tri(left(tindex))->_state.flags.sq && tri(right(tindex))->_state.flags.sq
       && (!n || (nt->tri(left(n))->_state.flags.sq
               && nt->tri(right(n))->_state.flags.sq)))
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
	if (tri(tindex)->_vis.flags.allout && !u)
		return false;
	if (!u)
		return true;
	if (neighbourTree(u)->tri(u)->_vis.flags.allout)
		return false;

	return true;
}

// Reset upto the point that the triangle are inserted but not below.
void ddgTBinTree::reset(ddgTriIndex tindex)
{
	ddgAsserts(tindex <= triNo(), "No leaf node was found in the queue!");

	tri(tindex)->reset();  // Reset all but queue status.

	// This will only go as far as the leaves which are currently in the split queue.
	if (!tri(tindex)->_state.flags.sq)
	{
		reset(left(tindex));
		reset(right(tindex));
	}
}

/// Return the camera space vector.
csVector3* ddgTBinTree::pos(ddgTriIndex tindex)
{
	csVector3 *ptri;
	if (tri(tindex)->_cbufindex == 0)
	{
		// Get new entry if we need it.
		tri(tindex)->_cbufindex = _mesh->vcache()->alloc();
	}
	ptri = _mesh->vcache()->get( tri(tindex)->_cbufindex );
	// Transform the vertex if we need to.
	if (!tri(tindex)->_state.flags.coord)
	{
		static csVector3 wpqr;
		vertex(tindex,&wpqr);
		transformer( wpqr, ptri );
		tri(tindex)->_state.flags.coord = true;
	}
	// Return the cache index to the transformed vertex.
	return ptri;
}

void ddgTBinTree::visibility(ddgTriIndex tvc)
{
	ddgTriIndex tva = parent(tvc),
		tv0 = v0(tvc),
		tv1 = v1(tvc);
 
    ddgAssert(_mesh->camBBox());
	ddgAsserts(tvc <= triNo(), " No leaf node was found in the queue!");

	bool oldVis = tri(tvc)->_vis.flags.allout;
	// If visibility information is not valid.
	if (tri(tvc)->_vis.visibility == ddgINIT)
	{
		if ((tri(tva)->_vis.visibility & ddgALLIN) 
		  ||(tri(tva)->_vis.visibility & ddgALLOUT)) 
		// If parent is completely inside or completely outside, inherit.
		{
			tri(tvc)->_vis = tri(tva)->_vis;
		}
		// We dont know so analyze ourselves.
		else
		{
			// Calculate bounding box of wedgie.
			csVector3 pmin;
			csVector3 pmax;
			csVector3 *ptri;
			csVector3 vu, vl;
			csVector3 th;
			csVector3 *un = &_unit;
			// Calculate the parent.
			ptri = pos(tva);
			th = (*un)*(tri(tvc)->thick());
			vu = *ptri+th;
			vl = *ptri-th;
			pmin = vu;
			pmax = vu;
			csMath3::SetMinMax(vl,pmin,pmax);

			// Calculate the left and right vertex.
			ptri = pos(tv0);
			th= (*un) *(tri(tv0)->thick());
			vu = *ptri+th;
			vl = *ptri-th;
			csMath3::SetMinMax(vu,pmin,pmax);
			csMath3::SetMinMax(vl,pmin,pmax);

			ptri = pos(tv1);
			th = (*un)*(tri(tv1)->thick());
			vu = *ptri+th;
			vl = *ptri-th;
			csMath3::SetMinMax(vu,pmin,pmax);
			csMath3::SetMinMax(vl,pmin,pmax);


			// Pmin and Pmax now define a bounding box that contains the wedgie.
			// Determine if this bounding box is visible in screen space.
		   	if (pmin.z> _mesh->farclip() || pmax.z < _mesh->nearclip())
				tri(tvc)->_vis.visibility = ddgALLOUT;
			else
			{
				float thfov = _mesh->tanHalfFOV();
				tri(tvc)->_vis = _mesh->camBBox()->visibleSpace(ddgBBox(pmin,pmax), thfov);
			}
		}
		if (tri(tvc)->_state.flags.sq && !tri(tvc)->_vis.flags.allout)
			// This triangle is in the mesh and is visible.
		{
			_visTri++;
		}
	}
	// Check if we need to reset the priority delay
	if (oldVis != tri(tvc)->_vis.flags.allout)
	{
		tri(tvc)->resetPriorityDelay();
	}
	// This will only go as far as the leaves which are currently in the split queue.
	if (!tri(tvc)->_state.flags.sq )
	{
		visibility(left(tvc));
		visibility(right(tvc));
	}

}

// Recusively update priorities.
void ddgTBinTree::priorityUpdate(ddgTriIndex tindex)
{ 
	ddgAsserts(tindex <= triNo(), " No leaf node was found in the queue!");
	// If priority information is not valid.
	if (!tri(tindex)->getPriorityDelay()
		&& (tri(tindex)->_state.flags.sq || tri(tindex)->_state.flags.mq))
	{
		unsigned int op = tri(tindex)->priority();
		unsigned int p = priority(tindex);
		ddgAsserts(tindex == 1 || p < 2 || p < priority(parent(tindex)),"Child has greater priority than parent!");

		if (op != p)
		{
			_mesh->movCountIncr();
			if (tri(tindex)->_state.flags.sq)
			{
				_mesh->qs()->remove(_index,tindex,op);
				_mesh->qs()->insert(_index,tindex,p);
			}

			if (tri(tindex)->_state.flags.mq )
			{
				_mesh->qm()->remove(_index,tindex,op);
				_mesh->qm()->insert(_index,tindex,p);
			}
		}
	}
	// This will only go as far as the leaves which are currently in the split queue.
	if (!tri(tindex)->_state.flags.sq )
	{
		priorityUpdate(left(tindex));
		priorityUpdate(right(tindex));
	}
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
	if (tri(tvc)->_vis.visibility == ddgINIT)
	{
		visibility(tvc);
	}

	// Wedgie is not visible.
	if (tri(tvc)->_vis.flags.allout )
	{
		tri(tvc)->setPriorityDelay( 1 );
		return tri(tvc)->priority( 0 );
	}

	// Our priority data is out of date, recalculate it.
	ddgTriIndex tva = parent(tvc),
		tv0 = v0(tvc),
		tv1 = v1(tvc);

	csVector3 wpqr;

    // Get screen space error metric of wedgie thickness.
	// Note were are switching y & z since ROAM paper is z
	// oriented for thickness and we are y oriented.

	if (!tri(tva)->_state.flags.coord)
	{
        vertex(tva,&wpqr);
		transformer( wpqr, pos(tva) );
		tri(tva)->_state.flags.coord = true;
	}

	// Left and right vertex should be calculated.
	if (!tri(tv0)->_state.flags.coord)
	{
        vertex(tv0,&wpqr);
		transformer( wpqr, pos(tv0) );
		tri(tv0)->_state.flags.coord = true;
	}
	if (!tri(tv1)->_state.flags.coord)
	{
        vertex(tv1,&wpqr);
		transformer( wpqr, pos(tv1) );
		tri(tv1)->_state.flags.coord = true;
	}

	// Calculate the maximum screen space thickness.
	csVector3 th = _unit*(tri(tvc)->thick());

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
    r[0] = pos(tv0)->z;
	rr[0] = r[0]*r[0];
	if (rr[0] <= cc + ddgEPSILON)
		goto skip;
    r[1] = pos(tv1)->z;
	rr[1] = r[1]*r[1];
	if (rr[1] <= cc + ddgEPSILON)
		goto skip;
	r[2] = pos(tva)->z;
	rr[2] = r[2]*r[2];
	if (rr[2] <= cc + ddgEPSILON)
		goto skip;

	// Perform expensive screen space error calculations.
    p[0] = pos(tv0)->x;
    q[0] = -1*pos(tv0)->y;
	tp[0] = a*r[0]-c*p[0];
	tq[0] = b*r[0]-c*q[0];
	d[0] = (sq(tp[0])+sq(tq[0]));
	m[0] = 2.0 / (rr[0] - cc);

    p[1] = pos(tv1)->x;
    q[1] = -1*pos(tv1)->y;
	tp[1] = a*r[1]-c*p[1];
	tq[1] = b*r[1]-c*q[1];
	d[1] = (sq(tp[1])+sq(tq[1]));
	m[1] = 2.0 / (rr[1] - cc);

	p[2] = pos(tva)->x;
	q[2] = -1*pos(tva)->y;
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
	if (_mesh->progdist() != 0)
	{
		v = int(zMin / _mesh->progdist());
		v = int(ddgUtil::clamp(v,1,15));
	}
	tri(tvc)->setPriorityDelay(v);

	return tri(tvc)->priority();
}

// Get height of bintree at a real location.
float ddgTBinTree::treeHeight(unsigned int r, unsigned int c, float dx, float dz)
{
    // Is this triangle mirrored.
    csVector3 v(dx,0,dz);
/*
    unsigned int m = (dx+dz > 1.0) ? 1 : 0;
    float h1 = height(r+ m,  c + m);
    float h2 = height(r,  c+1);
    float h3 = height(r+1,c);
    csVector3 p0(m,h1,m),
    csVector3 p1(0,h2,1),
    csVector3 p2(1,h3,0),
    ddgPlane p(p0,p1,p2);
    p.projectAlongY(&v);
*/
    return v.y;
}

#ifdef _DEBUG
bool ddgTBinTree::verify( ddgTriIndex tindex )
{
	if  (_mesh->merge() && tindex < _mesh->triNo()/2 && isDiamond(tindex))
	{
		ddgTriIndex n = neighbour(tindex);
		ddgTBinTree *nt = n ? neighbourTree(tindex) : 0;
		if ((tri(tindex)->_state.flags.mq == false) &&
			(n&& nt->tri(n)->_state.flags.mq == false))
			return true;
	}
	return false;
}
#endif
