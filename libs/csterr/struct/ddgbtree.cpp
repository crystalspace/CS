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
#include "math/ddggeom.h"
#include "math/ddgmatrx.h"
#include "struct/ddgtmesh.h"
#include "struct/ddgbtree.h"
/*
static ddgTriIndex	*recurStack = NULL;	// mesh->_maxLevel
static unsigned int recurStackSize = 0;
#define recurStackPush(a)	{ ddgAsserts( recurStackSize <= _mesh->maxLevel(),"Recursion stack overflow!");recurStack[recurStackSize] = a; recurStackSize++; }
#define recurStackPop(a)	{ ddgAsserts( recurStackSize > 0,"Recursion stack underflow!");recurStackSize--; a = recurStack[recurStackSize]; }
*/
// Constant used for identifying neighbours.
const	unsigned int ddgNINIT = 0xFFFFFFFF;

ddgTBinTree::ddgTBinTree( ddgTBinMesh *m, ddgTreeIndex i, int dr, int dc, bool mirror )
{
	_index = i;
    ddgAssert(m);
	_mesh = m;
    _dr = dr;
    _dc = dc;

    _mirror = mirror;
	_pNeighbourTop = NULL;
	_pNeighbourLeft = NULL;
	_pNeighbourDiag = NULL;
	_cacheIndex = new ddgCacheIndex[triNo()+2];
	ddgAsserts(_cacheIndex,"Failed to Allocate memory");
	ddgMemorySet(ddgCacheIndex,triNo()+2);
	_rawHeight = new short[triNo()+2];
	ddgAsserts(_rawHeight,"Failed to Allocate memory");
	ddgMemorySet(short,triNo()+2);
	_rawMinVal = new short[triNo()+2];
	ddgAsserts(_rawMinVal,"Failed to Allocate memory");
	ddgMemorySet(short,triNo()+2);
	_rawMaxVal = new short[triNo()+2];
	ddgAsserts(_rawMaxVal,"Failed to Allocate memory");
	ddgMemorySet(short,triNo()+2);
/*
	if (!recurStack)
	{
		recurStack = new ddgTriIndex[2*_mesh->maxLevel()];
		ddgMemorySet(ddgTriIndex,2*_mesh->maxLevel());
	}
*/
	_chain = 0;
}

ddgTBinTree::~ddgTBinTree(void)
{
	delete [] _cacheIndex;
	delete [] _rawHeight;
	delete [] _rawMinVal;
	delete [] _rawMaxVal;
}

/** Initialize the bintree structure including
 *  the wedges and other mesh values.
 */
bool ddgTBinTree::init( void )
{
	// Set the top level height values.
	_rawHeight[0] =			_mesh->heightMap()->get(mrow(0),mcol(0)) ;
	_rawHeight[triNo()] =	_mesh->heightMap()->get(mrow(triNo()),mcol(triNo())) ;
	_rawHeight[triNo()+1] = _mesh->heightMap()->get(mrow(triNo()+1),mcol(triNo()+1)) ;

	// Initialize the whole bin tree.

	initTriangle(1,0,triNo(),triNo()+1,1);
	// Set the top level min/max values
	_rawMaxVal[0] = _rawMaxVal[1];
	_rawMinVal[0] = _rawMinVal[1];
	_rawMaxVal[triNo()] = _rawMaxVal[1];
	_rawMinVal[triNo()] = _rawMinVal[1];
	_rawMaxVal[triNo()+1] = _rawMaxVal[1];
	_rawMinVal[triNo()+1] = _rawMinVal[1];
	// Initialize the tcache.
	_cacheIndex[0] = 0;
	_cacheIndex[1] = 0;
	_cacheIndex[triNo()] = 0;
	_cacheIndex[triNo()+1] = 0;

	return ddgSuccess;
}

// planes with better near/far distance.
ddgContext *ddgTBinTree::_ctx;
int _priorityScale = 1;
void ddgTBinTree::updateContext( ddgContext *ctx, ddgTBinMesh *mesh )
{
	// Copy this data to a private structure.
	_ctx = ctx;
	_priorityScale = mesh->absDiffHeight() / (ddgPriorityResolution*4);
}

/**
 * Recursively calculate the min/max value of a triangle.
 * With this information we establish a convex hull
 * for each level of the hierarchy, this hull can be
 * use for:
 * visibility culling.
 * collision detection.
 * ray intersection testing.
 *  Passed in are the triangles which carry the points
 *  that describe the current triangle.
 *  va is immediate parent.
 *  v1 and v0 are the left and right vertices of va.
 *  This method is only called at initialization.
 */
void ddgTBinTree::initTriangle( unsigned int level,
					 ddgTriIndex va,	// Top
					 ddgTriIndex v0,	// Left
					 ddgTriIndex v1,	// Right
					 ddgTriIndex vc)	// Centre
{
	short hmin, hmax;
	// Get the raw height and store it.
	_rawHeight[vc] = _mesh->heightMap()->get(mrow(vc),mcol(vc));

	// See if we are not at the bottom.
	if (level < maxLevel())
	{
		initTriangle(level+1,vc,va,v0,left(vc)),
		initTriangle(level+1,vc,v1,va,right(vc));

		hmax = (short)ddgUtil::max(_rawMaxVal[left(vc)],_rawMaxVal[right(vc)]);
		hmin = (short)ddgUtil::min(_rawMinVal[left(vc)],_rawMinVal[right(vc)]);
	}
	else
	{
		hmax = (short)ddgUtil::max(_rawHeight[v0],_rawHeight[v1]);
		hmin = (short)ddgUtil::min(_rawHeight[v0],_rawHeight[v1]);
	}

	_rawMaxVal[vc] = (short)ddgUtil::max(_rawHeight[va],hmax);
	_rawMinVal[vc] = (short)ddgUtil::min(_rawHeight[va],hmin);
	_cacheIndex[vc] = 0;
	// Copy the height map's value into this triangle.

}




/// QUEUE OPERATIONS


/// Insert triangle tindex into queue.
void ddgTBinTree::insertSQ(ddgTriIndex tindex, ddgPriority pp, ddgCacheIndex ci, ddgVisState vp)
{
    ddgAsserts(tindex >0 && tindex < triNo()+2,"Index is out of range.");
	ddgAsserts(	tcacheId(tindex) == 0,"Triangle already in queue.");

	// If parent is completely inside or completely outside of view, inherit.
	// Otherwise calculate triangle's visibility.
	ddgVisState v = (tindex, (vp == ddgIN || vp == ddgOUT) ? vp : visibilityTriangle(tindex));

	ddgAssert(v != ddgUNDEF);

	ddgTNode * tn;
	// Get a new cache entry if we need one.
	if (ci == 0)
	{

		ci = _chain = _mesh->tcache()->insertHead(_chain);
		// Insert a node at front of the chain.
		ddgAssert(ci);
	}
	tn = _mesh->tcache()->get(ci);
	tn->tindex(tindex);
	// Record its value
	tn->vis(v);
	tn->vbufferIndex(0xFFFF);
	tn->qscacheIndex(0);
	tn->qmcacheIndex(0);
/*
	tn->height(_mesh->wheight(_rawHeight[tindex]));
	tn->minHeight(_mesh->wheight(_rawMinVal[tindex]));
	tn->maxHeight(_mesh->wheight(_rawMaxVal[tindex]));
*/
	tn->priorityFactor(v ? (_rawMaxVal[tindex]-_rawMinVal[tindex])/_priorityScale : 0);

	// Record the cache index for the entry in this bintree.
	tcacheId(tindex, ci);

	// Determine what this triangle's priority is going to be.
	ddgPriority pr = (v == ddgOUT) ? 0 : priorityCalc(tindex);
	// Ensure our priority is less than our parent's.
	if (pr > pp && pp > 1)
		pr = pp-1;
	ddgAssert(pr >= 0 && pr <= ddgPriorityResolution-1);
	tn->priority(pr);

	if (v!=ddgOUT)
		_mesh->incrTriVis();
	// If we are not a leaf node.
	if (tindex < _mesh->leafTriNo())
	{
		if (v!=ddgOUT)
		{
#ifdef _DEBUG
			_mesh->insCountIncr();
#endif
			// Find insertion point.
			ddgCacheIndex qci = _mesh->qscache()->insert(_index,tindex,pr);
			ddgAssert(qci);
			tn->qscacheIndex(qci);
		}

		if (!_mesh->merge())
		{
			// We need to clear the next level and the one below.
			tcacheId(left(tindex), 0);
			tcacheId(right(tindex), 0);

			if (left(tindex) < _mesh->leafTriNo())
			{
				tcacheId(left(left(tindex)), 0);
				tcacheId(left(right(tindex)), 0);
				tcacheId(right(left(tindex)), 0);
				tcacheId(right(right(tindex)), 0);
			}
		}
	}
}


/// Remove triangle tindex.
ddgCacheIndex ddgTBinTree::removeSQ(ddgTriIndex tindex )
{
    ddgAsserts(tindex >=0 && tindex <= triNo()+2,"Index is out of range.");
    ddgAsserts(tcacheId(tindex),"Triangle is not in split queue.");

	// If we were part of a merge diamond, remove us from the merge queue if it is there.
	if (_mesh->merge())
		removeMQ(tindex);

	// Dont physically remove this entry from tcache next insert will reuse it.
	ddgCacheIndex ci = tcacheId(tindex);
	ddgAssert(ci);

	if (vis(tindex) !=ddgOUT)
		_mesh->decrTriVis();

	// If we are not a leaf node...
	if (tindex < _mesh->leafTriNo())
	{
	    // If we were visible remove us from the queue.
		if (vis(tindex) !=ddgOUT )
		{
#ifdef _DEBUG
			_mesh->remCountIncr();
#endif
			ddgAssert(_mesh->tcache()->get(ci)->qscacheIndex());
			ddgCacheIndex csi = qscacheIndex(tindex);
			_mesh->qscache()->remove(csi);
			qscacheIndex(tindex,0);
		}
	}
	tcacheId(tindex,0);
	return ci;
}



/*
 Insert triangle tindex into queue if it isn't already there.
 If pr = ddgMAXPRI we need to determine the maximum priority ourselves.
 */
void ddgTBinTree::insertMQ(ddgTriIndex tindex, ddgPriority pr)
{
    ddgAsserts(tindex >=0 && tindex < triNo()+2,"Index is out of range.");
	ddgAssert(_mesh->merge());
	// If we are at the root, or we are already in the merge queue we are done.
	if ( tindex < 1 || qmcacheIndex(tindex) )
		return;

	updateMerge(tindex, pr);
}

/*
 Remove triangle i from merge queue if it is in the queue.
 If it is not, but its neigbour is, then remove it instead.
 If neither is in the merge queue, do nothing.
*/
void ddgTBinTree::removeMQ(ddgTriIndex tindex)
{
    ddgAsserts(tindex >=0 && tindex < triNo()+2,"Index is out of range.");
	ddgAssert(_mesh->merge());

	// If we are the root or not part of a merge diamond we are done.
	if ( tindex < 1 || !qmcacheIndex(tindex) )
		return;

	// Find the neigbour if there is one.
	ddgTriIndex p = parent(tindex);
	ddgTriIndex n = neighbour(p);
	ddgTBinTree *nt = n ? neighbourTree(p) : 0;
	ddgAssert((n && nt) || (!n && !nt));

	ddgCacheIndex cl, cr, nl = 0, nr = 0;

	// If one triangle is missing we can not be merged.
	cl=tcacheId(left(p));
	cr=tcacheId(right(p));
	ddgAssert(cl&&cr);
	if (n)
	{
		nl=nt->tcacheId(left(n));
		nr=nt->tcacheId(right(n));
		ddgAssert(nl&&nr);
	}

	// Find item in merge queue and remove it.
	ddgCacheIndex mi = qmcacheIndex(tindex);
	ddgAssert(mi);
	_mesh->qmcache()->remove(mi);

	// Mark entries in the tmesh as not present in the merge queue.
	_mesh->tcache()->get(cl)->qmcacheIndex(0);
	_mesh->tcache()->get(cr)->qmcacheIndex(0);
	qmcacheIndex(left(p),0);
	qmcacheIndex(right(p),0);
	if (n)
	{
		_mesh->tcache()->get(nl)->qmcacheIndex(0);
		_mesh->tcache()->get(nr)->qmcacheIndex(0);
		nt->qmcacheIndex(left(n),0);
		nt->qmcacheIndex(right(n),0);
	}
}


/*
 * Split a triangle while keeping the mesh consistent.
 * there should be no T junctions. 'i' is a triangle index number.
 */
void ddgTBinTree::forceSplit( ddgTriIndex tindex )
{
	ddgPriority pr = forceSplit2(tindex);
	// This operation may have created a mergable diamond.
	if (_mesh->merge())
		insertMQ(left(tindex),pr);
}

/*
 * Split a triangle while keeping the mesh consistent.
 * there should be no T junctions. 'i' is a triangle index number.
 * This may require splitting the neighbour as well.
 * return the max priority value of the triangles which were split.
 */
ddgPriority ddgTBinTree::forceSplit2( ddgTriIndex tindex )
{
	ddgTriIndex l = left(tindex), r = right(tindex), n = neighbour(tindex);
	// If current triangle is not in the queue,
	// split its parent, to get it into the queue.
	if (!tcacheId(tindex))
	{
        // If we are a top level triangle and we are not in the tree.
        // Then we cannot force the parent to split.
        if (tindex < 2)
            return 0;
        ddgAssert(tindex > 1);
        // $TODO, optimize to not insert ourselves (tindex),
        // because we need to remove ourselves again anyways.
		forceSplit2(parent(tindex));
	}
    // We are in the queue at this time, remove ourselves.
	ddgCacheIndex ci = 0;
	ddgVisState v = ddgUNDEF;
	ddgPriority pr = priority(tindex);
	if (tcacheId(tindex))
	{
		v = vis(tindex);
		ci = removeSQ(tindex);
	}
    // Insert the left and right child.
	insertSQ(l,pr,ci,v);
	insertSQ(r,pr,0,v);

    // If this triangle has a merge neighbour, it also needs to be split.
	if (n)
	{
		ddgTBinTree *nt = neighbourTree(tindex);
		ddgAssert(nt);
		// Make sure its children are in the queue.
		if (!nt->tcacheId(left(n)) || !nt->tcacheId(right(n)))
		{
			ddgPriority npr = nt->priority(n);
			if (npr > pr)
				pr = npr;
			nt->forceSplit2(n);
		}
	}

	return pr;
}

/// 'i' is a triangle index.
void ddgTBinTree::forceMerge( ddgTriIndex tindex)
{
	// Find the neigbour if there is one.
	ddgTriIndex p = parent(tindex);
	ddgTriIndex n = neighbour(p);
	ddgTBinTree *nt = n ? neighbourTree(p) : 0;
	ddgAssert((n && nt) || (!n && !nt));

	// If one triangle is missing we can not be merged.
	ddgAsserts (tcacheId(left(p)) && tcacheId(right(p)),"Incomplete merge diamond/invalid mesh!");
	ddgAsserts (!n || (nt->tcacheId(left(n)) && nt->tcacheId(right(n))),"Incomplete merge diamond/invalid mesh!");

	// Remove its children and neighbour's children.
	ddgCacheIndex ci = 0;
	ci = removeSQ(left(p));
	if (ci == _chain)
	{
		_chain = _mesh->tcache()->get(ci)->next();
	}
	_mesh->tcache()->remove(ci);
	ci = removeSQ(right(p));

	// Insert parent and parent's neighbour.
	insertSQ(p,ddgMAXPRI,ci,ddgUNDEF);
	// This operation may have created a mergable diamond.
	if (_mesh->merge())
		insertMQ(p,ddgMAXPRI);

    if (n)
    {
		ddgAssert(nt);
		ci = nt->removeSQ(left(n));
		if (ci == nt->_chain)
		{
			nt->_chain = _mesh->tcache()->get(ci)->next();
		}
		_mesh->tcache()->remove(ci);
		ci = nt->removeSQ(right(n));
		nt->insertSQ(n,ddgMAXPRI,ci,ddgUNDEF);
		// This operation may have created a mergable diamond.
		if (_mesh->merge())
			nt->insertMQ(n,ddgMAXPRI);
    }
}

bool ddgTBinTree::updateMerge(ddgTriIndex tindex, ddgPriority pr )
{
	bool retval = false;
	// Check all 4 members of this potential mergable group.
	// A mergeble group looks as follows:
	//      /|\
	//     / | \
	//    /__|__\
	//    \  |  /
	//     \ | /
	//      \|/
	//
	// We are mergable if all 4 triangles are in the tcache.
	// Unless we have no neighbour in which case only this
	// triangle (tindex) and its brother will do.
	//
	// If we are mergable we will add a queue entry for this
	// triangle and mark all 4
	// triangles as being part of a merge diamond.
	ddgTriIndex p = parent(tindex);
	ddgTriIndex n = neighbour(p);
	ddgCacheIndex cl, cr, nl = 0, nr = 0;

	ddgAssert( n < ddgNINIT );
	ddgAssert( tindex > 0 );
	ddgTBinTree *nt = n ? neighbourTree(p) : 0;
	ddgAssert((n && nt) || (!n && !nt));

	// If one triangle is missing we can not be merged.
	if    (!(cl=tcacheId(left(p)))
		|| !(cr=tcacheId(right(p)))
		||  (nt && (!(nl=nt->tcacheId(left(n)))
		         || !(nr=nt->tcacheId(right(n)))
			)))
	{
		ddgAssert(!qmcacheIndex(tindex));
		return false;
	}
#ifdef _DEBUG
	_mesh->insCountIncr();
#endif

	if (pr == ddgMAXPRI)
	{
		// Find the highest priority among the 4 triangles.
		pr = priority(left(p));
		pr = (ddgPriority)ddgUtil::max(pr,priority(right(p)));
		if (n)
		{
			pr = (ddgPriority)ddgUtil::max(pr,nt->priority(left(n)));
			pr = (ddgPriority)ddgUtil::max(pr,nt->priority(right(n)));
		}
	}

	// If we were already in the queue, this will become a move.
	ddgPriority opr = ddgMAXPRI;
	if (qmcacheIndex(tindex))
	{
		opr = _mesh->tcache()->get(tcacheId(tindex))->qmcacheIndex();
	}

	ddgCacheIndex mi;
	// Add a merge queue entry.
	if (opr != pr || opr == ddgMAXPRI)
	{
		if (opr != ddgMAXPRI)
		{
			 mi = qmcacheIndex(tindex);
			_mesh->qmcache()->remove(mi);
		}
		mi = _mesh->qmcache()->insert(_index,tindex,pr);
		// Mark these entries in the tmesh as present in the merge queue.
		_mesh->tcache()->get(cl)->qmcacheIndex(mi);
		_mesh->tcache()->get(cr)->qmcacheIndex(mi);
		qmcacheIndex(left(p),mi);
		qmcacheIndex(right(p),mi);
		if (nt)
		{
			_mesh->tcache()->get(nl)->qmcacheIndex(mi);
			_mesh->tcache()->get(nr)->qmcacheIndex(mi);
			nt->qmcacheIndex(left(n),mi);
			nt->qmcacheIndex(right(n),mi);
		}
		retval = true;

	}
	return retval;
}


bool ddgTBinTree::updateSplit(ddgTriIndex tindex, ddgVisState parVis )
{
	ddgAsserts(tindex <= triNo(), " No leaf node was found in the queue!");

	ddgVisState v;

	bool modified = false;

	if (parVis == ddgOUT || parVis == ddgIN)
	// If parent is completely inside or completely outside, inherit.
	{
		v = parVis;
	}
	// We dont know so analyze ourselves.
	else
	{
		v = visibilityTriangle(tindex);
	}
	// If this is a leaf, update it.
	if (tcacheId(tindex))
	{
		// Adjust its priority.
		// Add and remove from queues as needed.
		ddgVisState ov = vis(tindex);
		ddgAssert(v != ddgUNDEF);

		// Determine what this triangle's priority is going to be.
		ddgPriority opr = priority(tindex);
		ddgPriority pr = (v == ddgOUT) ? 0 : priorityCalc(tindex);

		// If priority or visibility has changed we need to update the state.
		if (pr != opr || v != ov)
		{
#ifdef _DEBUG
			_mesh->movCountIncr();
#endif
			modified = true;
		}
		if (pr != opr)
			priority(tindex,pr);

		// Update split queue state.
		// Was visible but is not anymore => remove from split queue.
		if (v == ddgOUT && ov != ddgOUT)
		{
			ddgAssert(tindex >= _mesh->leafTriNo() || qscacheIndex(tindex));
			if (tindex < _mesh->leafTriNo())
			{
				ddgCacheIndex qcio = qscacheIndex(tindex);
				_mesh->qscache()->remove(qcio);
				qscacheIndex(tindex,0);
			}
			_mesh->decrTriVis();
			vis(tindex,v);
		}
		// Was not visible but is now => add it to the split queue.
		else if (v != ddgOUT && ov == ddgOUT)
		{
			ddgAssert(!qscacheIndex(tindex));
			if (tindex < _mesh->leafTriNo())
			{
				ddgCacheIndex qci = _mesh->qscache()->insert(_index,tindex,pr);
				ddgAssert(qci);
				qscacheIndex(tindex,qci);
			}
			_mesh->incrTriVis();
			vis(tindex,v);
		}
		// Visibility did not change but priority increased/decreased.
		// Change queue position.
		else if (pr != opr && tindex < _mesh->leafTriNo() && qscacheIndex(tindex))
		{
			ddgCacheIndex qcio = qscacheIndex(tindex);
			_mesh->qscache()->remove(qcio);
			ddgCacheIndex qci = _mesh->qscache()->insert(_index,tindex,pr);
			ddgAssert(qci);
			qscacheIndex(tindex,qci);
		}
		ddgAssert(v != ddgOUT || pr == 0);
		modified = true;

	}
	// This will only go as far as the leaves which are currently in the mesh.
	else
	{
		modified |= updateSplit(left(tindex),v);
		modified |= updateSplit(right(tindex),v);
	}
	return modified;
}


/// VISIBILITY

ddgVisState ddgTBinTree::visibilityTriangle(ddgTriIndex tindex)
{
	ddgAsserts(tindex <= triNo(), " No leaf node was found in the queue!");

	ddgTriIndex tva = parent(tindex),
		tv0 = v0(tindex),
		tv1 = v1(tindex);
#ifdef _DEBUG
	_mesh->visCountIncr();
#endif
	// Calculate bounding box of bintree.
	ddgBBox		bbox;
	ddgVector3 *pmin = bbox.min();
	ddgVector3 *pmax = bbox.max();

	// See if the triangle is too far from the camera.  Ingore height component.
	ddgVector2 c(mrow(tindex),mcol(tindex));
	c.v[0] -= _ctx->control()->position()->v[0];
	c.v[1] -= _ctx->control()->position()->v[2];
	// Do a rough distance test and optionally a true distance test.
	if (c[0] > _mesh->farClip() || c[0] < - _mesh->farClip()
	  ||c[1] > _mesh->farClip() || c[1] < - _mesh->farClip()
	  ||c.sizesq() > _mesh->farClipSQ())
		return ddgOUT;
	
	// Bounding box is defined by points tva, v0 and v1.
	pmin->v[0] = (mrow(tv0) < mrow(tv1)) ? mrow(tv0) : mrow(tv1);
	pmin->v[2] = (mcol(tv0) < mcol(tv1)) ? mcol(tv0) : mcol(tv1);
	pmax->v[0] = (mrow(tv0) > mrow(tv1)) ? mrow(tv0) : mrow(tv1);
	pmax->v[2] = (mcol(tv0) > mcol(tv1)) ? mcol(tv0) : mcol(tv1);
	if (mrow(tva) < pmin->v[0]) pmin->v[0] = mrow(tva);
	if (mcol(tva) < pmin->v[2]) pmin->v[2] = mcol(tva);
	if (mrow(tva) > pmax->v[0]) pmax->v[0] = mrow(tva);
	if (mcol(tva) > pmax->v[2]) pmax->v[2] = mcol(tva);

	// Adjust box by bintree thickness.
	pmin->v[1] = minVal(tindex);
	pmax->v[1] = maxVal(tindex);

	return bbox.isVisible(_ctx->frustrum(),5);
}


/// PRIORITY CALCULATION



// Priority of Triangle tindex.  We assume that we only get called
// for visible triangles.

// $TODO convert the divide to a multiply.
// clip priority to max bucket.
ddgPriority ddgTBinTree::priorityCalc(ddgTriIndex tindex)
{
    ddgAssert(tindex <= triNo());

	float result;

	// Calculate the distance of the nearest point to the
	// near clipping plane by taking the dot product of
	// the near clipping plane's vector with the vector
	// from the camera to the point.
	// $TODO if camera is moving forward, we can estimate delta z.
	float z;
	/*
	ddgVector3 p1(mrow(tindex),height(tindex),mcol(tindex));
	ddgVector3 p2(_ctx->control()->position());
	*/
	ddgVector2 p1(mrow(tindex),mcol(tindex));
	ddgVector2 p2(_ctx->control()->position()->v[0],_ctx->control()->position()->v[2]);
	ddgVector2 f(_ctx->forward()->v[0],_ctx->forward()->v[2]);
	// This is the same a f dot p1 - f dot p2, f dot p2 can be precomputed,
	// but I don't think that will speed things up.
	z = f.dot(p1 - p2);
	if (z < _mesh->nearClip())
		z = _mesh->nearClip();
	// Priority is factor of:
	// Distance from viewer.
	// Angle of triangle normal vs vector to viewer. == Precompute factor with look up table.
	// Convex hull thickness.
	ddgAsserts(z > 0 && z < 1000,"Bad distance");
	ddgAsserts(_priorityScale > 1,"Bad priorityScale");
	// zDist is > 0     && < farClip
	// Max - Min >= 0   && < _absDiffHeight.
	// _priorityScale = absDiff / buckNo, since absdiff > bucketNo  ==> > 1
	// => result >= 1.

	result = 1+priorityFactor(tindex)/z;
	
#ifdef _DEBUG
	_mesh->priCountIncr();
#endif
	// Priority of 0 is reserved for triangles which are not visible.
	ddgAssert(result >= 1);
	return (ddgPriority)(result >= ddgPriorityResolution ? ddgPriorityResolution-1 : result);

}


// Get height of bintree at a real location.
float ddgTBinTree::treeHeight(unsigned int r, unsigned int c, float dx, float dz)
{
	// Is this triangle mirrored.
    unsigned int m = (dx+dz > 1.0) ? 1 : 0;
    float h1 = heightByPos(r + m, c + m);
    float h2 = heightByPos(r,     c + 1);
    float h3 = heightByPos(r + 1, c    );
	ddgVector3 p0(m,h1,m),
			   p1(0,h2,1),
			   p2(1,h3,0),
			   v(dx,0,dz);
	ddgPlane p(p0,p1,p2);
	p.projectAlongY(&v);

    return v[1];
}

// p1 - start point of ray,
// p2 - end point of ray.
// Cases:  Line p1 -> p2 does not intersect this triangle.
//         Line intersects this triangle, but point p1 is past points intersecting this triangle.
//         Line intersects this triangle, but point p2 is before points intersecting this triangle.
//         p1 is inside this triangle.
//         p1 is on egde of this triangle.

bool ddgTBinTree::rayTest( ddgVector3 p1, ddgVector3 p2, ddgTriIndex tindex, int depth )
{
    ddgTriIndex 
		tva = parent(tindex),
		tv1 = v0(tindex),
		tv0 = v1(tindex);
	ddgVector3 v0,v1,va;
    vertex(tva,&v0);
    vertex(tv0,&v1);
    vertex(tv1,&va);
	// Find bounding square of this triangle.
	ddgVector2 t_1 (v0[0], v0[2]);
	ddgVector2 t_2 (v1[0], v1[2]);
	ddgRect t(t_1,t_2);
	// Find bounding rectangle of ray.
	ddgVector2 r_1 (p1[0],p1[2]);
	ddgVector2 r_2 (p2[0],p2[2]);
	ddgRect r(r_1,r_2);
	// See if rectangles intersect.
	if (!t.intersect(&r))
		return false;
	// Find line equation.
	ddgLine3 line(p1,p2);
	ddgVector3 e1(va);	// Edge1
	ddgVector3 e2(va);	// Edge2
	// Find the Z coord where X = Xa.
	line.solve(e1,0);
	// Find the X coord where Z = Za.
	line.solve(e2,0);
	// The two intersection points of the line with the triangle.
	ddgVector3 s[2];
	int i = 0;
	if (e1[0] >= t.min[0] && e1[0] < t.max[0])
		s[i++].set(e1);
	if (e2[2] >= t.min[2] && e2[2] < t.max[2])
		s[i++].set(e2);
	// We must cross 2 of the three sides of the triangle with the line.
	// If we haven't crossed any side yet, then we must have missed this triangle.
	// See if segment fall outside of triangle.
	if (i==0)
		return false;
	if (i==1)
	{
		// We must be crossing the diagonal, calculate that point.
		ddgLine3 diag(v0,v1);
		line.intersect(&diag,s[1]);
	}
	// We have the line segment which crosses this triangle in s[0]->s[1].
	ddgVector2 sr_1 (s[0][0],s[0][1]);
	ddgVector2 sr_2 (s[1][0],s[1][1]);
	ddgRect sr(sr_1,sr_2),cs;
	// Clip the segment to p1 and p2 incase p1 and p2 fall inside this triangle.
	sr.intersect(&r,&cs);
	// Now we have a line segment guaranteed to be both in the triangle and within the
	// limits of the ray.

	// Check for definite miss.
	if (s[0].v[1] > _rawMaxVal[tindex] || s[1].v[1] > _rawMaxVal[tindex])
		return false;
	// Check for definite hit.
	if (s[0].v[1] < _rawMinVal[tindex] || s[1].v[1] < _rawMinVal[tindex])
	{
		// Record hit in buffer... $TODO
		return true;
	}

	// Perform recursive test...
	// See if we are not at the bottom.
	depth--;
	if (depth != 0 && tindex < _mesh->leafTriNo())
	{	
		// TODO perform directional test to ensure points closest to p1 are tested first.
		// Skip the odd level.
		if (rayTest(p1,p2,left(tindex),depth))
			return true;
		return rayTest(p1,p2,right(tindex),depth);
	}
	// Default is not hit.
	else
		return false;
}
/**
 * Ray test a line segment to see if it collides with the terrain.
 * A depth of 0 implies testing to the leaf nodes.
 * Returns true and the index of the triangle if a hit occured.
 * Returns false otherwise.
 */
/*
bool ddgTBinTree::rayTest( ddgVector3 p1, ddgVector3 p2, ddgTriIndex *tindex, unsigned int depth )
{
	// Translate line into coord sys of this bintree.
	p1 = p1 - ddgVector3(_dc,0,_dr);
	p2 = p2 - ddgVector3(_dc,0,_dr);
	// Convert Y coord to raw height.
	p1.v[1] = heightMap->iconvert(p1.v[1]);
	p2.v[1] = heightMap->iconvert(p2.v[1]);
	// $TODO handle mirror case.
	if (_mirror)
	{
		// flip X/Z and translate.
	}
	// See if points span this tree.
	if (p1.v[0] > ddgTBinMesh_size &&  p2.v[0] > ddgTBinMesh_size )
		return false;
	if (p1.v[0] < 0 &&  p2.v[0] < 0 )
		return false;
	if (p1.v[2] > ddgTBinMesh_size &&  p2.v[2] > ddgTBinMesh_size )
		return false;
	if (p1.v[2] < 0 &&  p2.v[2] < 0 )
		return false;

	// Find line equation.
	ddgLine3 line(p1,p2);
	ddgVector3 pl(0,0,0);	// Left point
	ddgVector3 pt(0,0,0);	// Top point
	// Find the Z coord where X = 0.
	line.solve(pl,0);
	// Find the X coord where Z = 0.
	line.solve(pt,0);
	// Coords of the segment crossing this bintree.
	ddgVector3 s[2];
	int i = 0;
	// Does segment go through top of triangle?
	if (pt[2] >= 0 && pt[2] < ddgTBinMesh_size)
		s[i++].set(pt);
	// Does segment go through left of triangle?
	if (pl[0] >= 0 && pl[2] < ddgTBinMesh_size)
		s[i++].set(pl);

	// Segment does not go through triangle.
	if (i == 0)
		return false;

	// Segment crosses diagonal.
	if (i == 1)
	{
		s[1].v[0] = (line.d()[0] * (ddgTBinMesh_size - s[0].v[2]) + line.d()[2]*s[0].v[0])
			/(line.d()[0]+line.d()[2]);
		line.solve(s[1],0);
	}
	// Clip segment to search segment.
	if (s[0].v[0] < p1[0])
		s[0].set( p1);
	if (s[1].v[0] > p2[0])
		s[1].set( p2);
	// Check for definite miss.
	if (s[0].v[1] > tri(0)->rawMaxVal() || s[1].v[1] > tri(0)->rawMaxVal())
		return false;
	// Check for definite hit.
	if (s[0].v[1] < tri(0)->rawMinVal() || s[1].v[1] < tri(0)->rawMinVal())
		return true;
	// Perform recursive test...
	return false;
}
*/

