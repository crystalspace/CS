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


// Constant used for identifying brothers.
const	unsigned int BINIT = 0xFFFFFFFF;

// ----------------------------------------------------------------------

ddgVector3 ddgTBinTree::_unit;
/// Set the unit vector.
void ddgTBinTree::unit(ddgVector3 *v)
{
	_unit.set(v);
}
/// Return the unit vector.
ddgVector3 * ddgTBinTree::unit(void)
{
	return &_unit;
}

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
ddgTBinTree::ddgTBinTree( ddgTBinMesh *m, ddgHeightMap* h, int dr, int dc, bool mirror )
{
    _init = false;
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
	// Add to the mesh.
	_mesh->addBinTree(this);
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
	//          va     1 size,0
	// Dummy entries.
	tri(triNo())->thick( 9999);
	tri(triNo()+1)->thick(9999);
	// Top level triangles.
	tri(0)->_state.all = 0;
	tri(1)->_state.all = 0;
	tri(0)->_cbufindex = 0;
	tri(1)->_cbufindex = 0;
	tri(1)->reset();
	tri(triNo())->_state.all = 0;
	tri(triNo()+1)->_state.all = 0;
	tri(triNo())->_cbufindex = 0;
	tri(triNo()+1)->_cbufindex = 0;

    split(1,0,triNo(),triNo()+1,1);

	tri(0)->thick( tri(2)->thick()> tri(3)->thick()?tri(2)->thick():tri(3)->thick());
	tri(1)->thick( tri(4)->thick()> tri(5)->thick()?tri(4)->thick():tri(5)->thick());
	tri(triNo())->thick(
		tri(0)->thick()> tri(1)->thick()?tri(0)->thick():tri(1)->thick());
	tri(triNo()+1)->thick(tri(triNo())->thick()); 

	return false;
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
    if (tri(tindex)->_state.flags.sq)
    {
	    tri(tindex)->_state.flags.sq = false;
		// If we were visible and not a leaf node actually remove us.
		_mesh->remCountIncr();
		unsigned int p = priority(tindex);
		_mesh->qs()->remove(_index,tindex,p);

		// If we were visible, reduce the count.
		if (!tri(tindex)->_vis.flags.none)
		{
			_visTri--;
		}
	}
	// If this triangle is in the merge queue and
	// it has a brother which is already in the
	// merge queue, then remove it from the merge queue.
	if (tri(parent(tindex))->_state.flags.mq)
		removeMQ(parent(tindex));
}

/// Insert triangle i into queue.
void ddgTBinTree::insertSQ(ddgTriIndex tindex)
{
	reset(tindex,0);
	tri(tindex)->_state.flags.sq = true;
	_mesh->insCountIncr();
	tri(tindex)->resetDelay();
	unsigned int p = priority(tindex);
	// Find insertion point.
	_mesh->qs()->insert(_index,tindex,p);

	ddgTriIndex b = 0;
	// If we are the child of a merge diamond.
	// The merge diamond is no longer mergable.
	if (tindex > 3 && tri(parent(parent(tindex)))->_state.flags.mq)
		removeMQ(parent(parent(tindex)));
	else if (tindex > 3 && (b = brother(tindex)) &&
        brotherTree(tindex)->tri(parent(parent(b)))->_state.flags.mq)
		brotherTree(tindex)->removeMQ(parent(parent(b)));
	if (isDiamond(tindex))
		insertMQ(parent(tindex));
}

/// Remove triangle i.
void ddgTBinTree::removeMQ(ddgTriIndex tindex)
{
	ddgTriIndex b;
	ddgTBinTree *bt;
	// See which is actually in the queue and remove it.
	if (tri(tindex)->_state.flags.mq)
	{
		unsigned int p = priority(tindex);
		_mesh->qm()->remove(_index,tindex,p);
	    tri(tindex)->_state.flags.mq = false;
	}
	else if ((b = brother(tindex)) != 0)
	{
		bt = brotherTree(tindex);
		unsigned int p = bt->priority(b);
		_mesh->qm()->remove(bt->index(),b,p);
	    bt->tri(b)->_state.flags.mq = false;
	}
	_mesh->remCountIncr();
}

/// Insert triangle i into queue.
void ddgTBinTree::insertMQ(ddgTriIndex tindex)
{
	// If we have a brother,
	// Insert the one with higher priority into the queue.
	ddgTriIndex b = brother(tindex);
	ddgTBinTree *bt = b ? brotherTree(tindex) : 0;
	ddgMTri *ti;
	_mesh->insCountIncr();
	if (b && priority(tindex) > bt->priority(b))
	{
		ti = bt->tri(b);
		ti->resetDelay();
		_mesh->qm()->insert(bt->index(),b,bt->priority(b));
	}
	else
	{
		ti = tri(tindex);
		ti->resetDelay();
		_mesh->qm()->insert(_index,tindex,priority(tindex));
	}
	ti->_state.flags.mq = true;
}

/** Split a triangle while keeping the mesh consistent.
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
        // TODO, optimize to not insert ourselves (tindex),
        // because we need to remove ourselves again anyways.
		forceSplit(parent(tindex));
		tri(tindex)->_state.flags.split = true;
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

    // If this triangle has a merge brother, it also needs to be in the split queue.
	if (brother(tindex))
	{
		// Make sure its children are in the queue.
		if (!brotherTree(tindex)->tri(left(brother(tindex)))->_state.flags.sq
		 || !brotherTree(tindex)->tri(right(brother(tindex)))->_state.flags.sq)
		{
			brotherTree(tindex)->forceSplit(brother(tindex));
		}
	}
}

/// 'i' is a triangle index.
void ddgTBinTree::forceMerge( ddgTriIndex tindex)
{
    // This triangle is in the merge queue.
    // This triangle's brother must not be in the merge queue.
    // The children are in the split queue.
 	// Remove its children and brothers children.
	// Insert self and brother.
	removeMQ(tindex);
	removeSQ(left(tindex));
	removeSQ(right(tindex));

	insertSQ(tindex);
	tri(tindex)->_state.flags.merged = true;
    if (brother(tindex))
    {
		brotherTree(tindex)->removeSQ(left(brother(tindex)));
		brotherTree(tindex)->removeSQ(right(brother(tindex)));
		brotherTree(tindex)->insertSQ(brother(tindex));
		tri(brother(tindex))->_state.flags.merged = true;
    }
}

/// Return if a triangle is part of a mergeble diamond.
// It is if its children and its brother's children are in the
// split queue.
bool ddgTBinTree::isDiamond(ddgTriIndex i)
{

    ddgTriIndex f = parent(i),		// Father.
				 u = brother(f);	// Uncle.
	if ( u < BINIT 
       && tri(left(f))->_state.flags.sq && tri(right(f))->_state.flags.sq
       && (!u || (brotherTree(u)->tri(left(u))->_state.flags.sq
               && brotherTree(u)->tri(right(u))->_state.flags.sq)))
		return true;

	return false;
}

/**
 * Return if a mergeble diamond is within the viewing frustrum.
 * Returns true if at least one child is partially visible.
 */
bool ddgTBinTree::isDiamondVisible(ddgTriIndex i)
{

    ddgTriIndex f = parent(i),		// Father.
				 u = brother(f);	// Uncle.
	if (tri(i)->_vis.flags.none)
		return false;
	if (!u)
		return true;
	if (brotherTree(u)->tri(u)->_vis.flags.none)
		return false;

	return true;
}

// Reset upto the point that the triangle are inserted but not below.
void ddgTBinTree::reset(ddgTriIndex tvc, unsigned int level)
{
    tri(tvc)->reset();  // Reset all but queue status.
	if (level)
	{
		if (!tri(tvc)->_state.flags.sq)
		{
			reset(left(tvc),level-1);
			reset(right(tvc),level-1);
		}
		else if (level>1)
		{
			tri(left(tvc))->_state.flags.dirty = true;
			tri(right(tvc))->_state.flags.dirty = true;
		}
    }

}

void ddgTBinTree::visibility(ddgTriIndex tvc, unsigned int level)
{
	ddgTriIndex tva = parent(tvc),
		tv0 = v0(tvc),
		tv1 = v1(tvc);
 
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
			ddgVector3 wpqr;
			// Calculate bounding box of wedgie.
			ddgVector3 pmin;
			ddgVector3 pmax;
			ddgVector3 *ptri;
			ddgVector3 vu, vl;
			ddgVector3 th;
			// Calculate the parent.
			ptri = pos(tva);
			if (!tri(tva)->_state.flags.coord)
			{
				vertex(tva,wpqr);
				_mesh->transform( wpqr, ptri );
				tri(tva)->_state.flags.coord = true;
			}
			th.set(unit());
			th.multiply(tri(tvc)->thick());
			vu = ptri+th;
			vl = ptri;
			vl.subtract(th);
			pmin.set(vu);
			pmax.set(vu);
			pmin.minimum(vl);
			pmax.maximum(vl);

			// Calculate the left and right vertex.
			ptri = pos(tv0);
			if (!tri(tv0)->_state.flags.coord)
			{
				vertex(tv0,wpqr);
				_mesh->transform( wpqr, ptri );
				tri(tv0)->_state.flags.coord = true;
			}
			th.set(unit());
			th.multiply(tri(tv0)->thick());
			vu = ptri+th;
			vl = ptri;
			vl.subtract(th);
			pmin.minimum(vu);
			pmax.maximum(vu);
			pmin.minimum(vl);
			pmax.maximum(vl);

			ptri = pos(tv1);
			if (!tri(tv1)->_state.flags.coord)
			{
				vertex(tv1,wpqr);
				_mesh->transform( wpqr, ptri );
				tri(tv1)->_state.flags.coord = true;
			}
			th.set(unit());
			th.multiply(tri(tv1)->thick());
			vu = ptri+th;
			vl = ptri;
			vl.subtract(th);
			pmin.minimum(vu);
			pmax.maximum(vu);
			pmin.minimum(vl);
			pmax.maximum(vl);

			if (!tri(tv1)->_state.flags.coord)
			{
				vertex(tv1,wpqr);
				_mesh->transform( wpqr, pos(tv1) );
				tri(tv1)->_state.flags.coord = true;
			}
			th.set(unit());
			th.multiply(tri(tv1)->thick());
			vu = pos(tv1)+th;
			vl = pos(tv1);
			vl.subtract(th);
			pmin.minimum(vu);
			pmax.maximum(vu);
			pmin.minimum(vl);
			pmax.maximum(vl);

			// Pmin and Pmax now define a bounding box that contains the wedgie.
			// Determine if this bounding box is visible in screen space.
#ifdef DDG
		   	if (pmin[2]< _mesh->farclip() || pmax[2] > _mesh->nearclip())
#else
		   	if (pmin[2]> _mesh->farclip() || pmax[2] < _mesh->nearclip())
#endif
				tri(tvc)->_vis.visibility = ddgALLOUT;
			else
			{
				float thfov = _mesh->tanHalfFOV();
				tri(tvc)->_vis = _mesh->camBBox()->visibleSpace(ddgBBox(pmin,pmax), thfov);
			}
		}
		if (tri(tvc)->_state.flags.sq && !tri(tvc)->_vis.flags.none)
			// This triangle is in the mesh and is visible.
		{
			_visTri++;
		}
	}
	// 
	if (!tri(tvc)->_state.flags.sq && level )
	{
		visibility(left(tvc),level-1);
		visibility(right(tvc),level-1);
	}
}

// Recusively update priorities.
void ddgTBinTree::priorityUpdate(ddgTriIndex tindex, unsigned int level)
{ 
	// If priority information is not valid.
	if (!tri(tindex)->_state.flags.priority
		&& (tri(tindex)->_state.flags.sq || tri(tindex)->_state.flags.mq))
	{
		unsigned int op = tri(tindex)->priority();
		unsigned int p = priority(tindex);

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
	// 
	if (!tri(tindex)->_state.flags.sq && level )
	{
		priorityUpdate(left(tindex),level-1);
		priorityUpdate(right(tindex),level-1);
	}
}

// Priority of Triangle tindex.
unsigned short ddgTBinTree::priority(ddgTriIndex tvc)
{
	// Wedgies priority is uptodate.
	if (tri(tvc)->_state.flags.priority)
		return tri(tvc)->priority();

	// Ensure triangle's visibility is uptodate.
	if (!tri(tvc)->_state.flags.coord)
	{
		visibility(tvc,0);
	}

	tri(tvc)->_state.flags.priority = true;

	// Wedgie is not visible.
	if (tri(tvc)->_vis.flags.none )
		return tri(tvc)->priority( 0 );

	// For progressive calculation, don't bother recalcing unles delay = 1;
	if (tri(tvc)->delay() > 0)
	{
		tri(tvc)->decrDelay();
		return tri(tvc)->priority();
	}

	// Our priority data is out of date, recalculate it.
	ddgTriIndex tva = parent(tvc),
		tv0 = v0(tvc),
		tv1 = v1(tvc);

	ddgVector3 wpqr;

    // Get screen space error metric of wedgie thickness.
	// Note were are switching y & z since ROAM paper is z
	// oriented for thickness and we are y oriented.

	if (!tri(tva)->_state.flags.coord)
	{
        vertex(tva,wpqr);
		_mesh->transform( wpqr, pos(tva) );
		tri(tva)->_state.flags.coord = true;
	}

	// Left and right vertex should be calculated.
	if (!tri(tv0)->_state.flags.coord)
	{
        vertex(tv0,wpqr);
		_mesh->transform( wpqr, pos(tv0) );
		tri(tv0)->_state.flags.coord = true;
	}
	if (!tri(tv1)->_state.flags.coord)
	{
        vertex(tv1,wpqr);
		_mesh->transform( wpqr, pos(tv1) );
		tri(tv1)->_state.flags.coord = true;
	}

	// Calculate the maximum screen space thickness.
	ddgVector3 th(unit());
	th.multiply(tri(tvc)->thick());

	float result = ddgMAXPRI;
#define ddgEPSILON 0.00001
	ddgVector3 vp = pos(tva);
	float a = th[0],
		  b = th[2],
		  c = th[1],
		  p = vp[0],
		  q = vp[1],
		  r = vp[2],
		  d, da, d0, d1,
		  m, ma, m0, m1;
	if (r*r <= c*c + ddgEPSILON)
		goto skip;
	da = (sq(a*r-c*p)+sq(b*r-c*q));
	ma = 2.0 / (r*r - c*c);
	vp = pos(tv0);
    p = vp[0];
    q = vp[1];
    r = vp[2];
	if (r*r <= c*c + ddgEPSILON)
		goto skip;
	d0 = (sq(a*r-c*p)+sq(b*r-c*q));
	m0 = 2.0 / (r*r - c*c);
	vp = pos(tv1);
    p = vp[0];
    q = vp[1];
    r = vp[2];
	if (r*r <= c*c + ddgEPSILON)
		goto skip;
	d1 = (sq(a*r-c*p)+sq(b*r-c*q));
	m1 = 2.0 / (r*r - c*c);

    d = ddgUtil::max(da,ddgUtil::max(d0,d1));
    m = ddgUtil::max(ma,ddgUtil::max(m0,m1));
	result = sqrt(d) * m;
	result = ddgUtil::clamp(result*1000,1,ddgMAXPRI);
skip:
	_mesh->priCountIncr();
	// Map the priority into a range from 0 to ddgMAXPRI.
	tri(tvc)->priority( (unsigned short)result);
	// Calculate recalc delay.
	float zMin = pos(tva,2);
	if ( zMin < pos(tv0,2)) zMin = pos(tv0,2);
	if ( zMin < pos(tv1,2)) zMin = pos(tv1,2);
#ifdef DDG
	if (zMin < 2*_mesh->progDist())
		tri(tvc)->setDelay(2);
	else if (zMin < _mesh->progDist())
		tri(tvc)->setDelay(1);
#endif
	return tri(tvc)->priority();
}

// Get height of bintree at a real location.
float ddgTBinTree::treeHeight(unsigned int r, unsigned int c, float dx, float dz)
{
	// Is this triangle mirrored.
    float m = (dx+dz > 1.0) ? 1.0 : 0.0;
    float h1 = height((unsigned int)(r+ m),  (unsigned int)(c + m));
    float h2 = height(r,  c+1);
    float h3 = height(r+1,c);
	// Handle special cases.
	if (dx == m && dz == m)
		return h1;
	// Handle special cases.
	if (dx == 0 && dz == 1)
		return h2;
	// Handle special cases.
	if (dx == 1 && dz == 0)
		return h3;
	// Calculate new point on h2 h3 that intersects line h1 p.
	float k = 1 / (dx + dz);
	ddgVector2 n(k-dx*k,1-k-dz*k);
	float sz = n.size();
	// Interpolate new height.
	float nh = sz * (h2 - h3)+h3;
	// Calculate distance from new point to h1
	ddgVector2 nh1(m-dx*k,m-dz*k);
	float snh1 = nh1.size();
	// Calculate distance from p to h1.
	ddgVector2 ph1(m-dx,m-dz);
	// Interpolate new height.
	float sph1 = ph1.size();
	float ph = sph1/snh1 * (nh - h1) + h1;
    return ph;
}

