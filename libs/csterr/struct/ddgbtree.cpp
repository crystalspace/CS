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
#define _ddgBinTree_Impl_
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


ddgTBinTree::ddgTBinTree( ddgTreeIndex i, int dr, int dc, bool mirror )
{
	_index = i;
    _dr = dr;
    _dc = dc;
	ddgAssert(_mesh);
    _mirror = mirror;
	_pNeighbourTop = NULL;
	_pNeighbourLeft = NULL;
	_pNeighbourDiag = NULL;
	_cacheIndex = new ddgCacheIndex[_mesh->triNo()+2];
	ddgAsserts(_cacheIndex,"Failed to Allocate memory");
	ddgMemorySet(ddgCacheIndex,_mesh->triNo()+2);

	_rawHeight = new short[_mesh->triNo()+2];
	ddgAsserts(_rawHeight,"Failed to Allocate memory");
	ddgMemorySet(short,_mesh->triNo()+2);

// LeafNo
	_rawMaxVal = new short[_mesh->leafTriNo()];
	ddgAsserts(_rawMaxVal,"Failed to Allocate memory");
	ddgMemorySet(unsigned short,_mesh->leafTriNo());

	_rawMinVal = new short[_mesh->leafTriNo()];
	ddgAsserts(_rawMinVal,"Failed to Allocate memory");
	ddgMemorySet(short,_mesh->leafTriNo());

	_treeError = new unsigned short[_mesh->leafTriNo()];
	ddgAsserts(_treeError,"Failed to Allocate memory");
	ddgMemorySet(unsigned short,_mesh->leafTriNo());

/*
	if (!recurStack)
	{
		recurStack = new ddgTriIndex[2*_mesh->maxLevel()];
		ddgMemorySet(ddgTriIndex,2*_mesh->maxLevel());
	}
*/
}

ddgTBinTree::~ddgTBinTree(void)
{
	delete [] _cacheIndex;
	delete [] _rawHeight;
	delete [] _rawMinVal;
	delete [] _rawMaxVal;
	delete [] _treeError;
}


/** Initialize the bintree structure including
 *  the wedges and other mesh values.
 */
bool ddgTBinTree::init( void )
{
	// Set the top level height values.
	_rawHeight[0] =                _mesh->heightMap()->get(mrow(0),mcol(0)) ;
	_rawHeight[_mesh->triNo()] =   _mesh->heightMap()->get(mrow(_mesh->triNo()),mcol(_mesh->triNo())) ;
	_rawHeight[_mesh->triNo()+1] = _mesh->heightMap()->get(mrow(_mesh->triNo()+1),mcol(_mesh->triNo()+1)) ;

	// Initialize the whole bin tree.
	initTriangle(1);
	// Set the top level min/max values
	_rawMaxVal[0] = _rawMaxVal[1];
	_rawMinVal[0] = _rawMinVal[1];
	_treeError[0] = _treeError[1];
	// Initialize the tcache.
	_cacheIndex[0] = 0;
	_cacheIndex[1] = 0;
	_cacheIndex[_mesh->triNo()] = 0;
	_cacheIndex[_mesh->triNo()+1] = 0;
	_treeVis = ddgUNDEF;

	return ddgSuccess;
}

// planes with better near/far distance.
ddgContext *ddgTBinTree::_ctx;
ddgVector2	ddgTBinTree::_pos, ddgTBinTree::_forward;
float ddgTBinTree::_nearClip;
float ddgTBinTree::_farClip;
float ddgTBinTree::_farClip2;
float ddgTBinTree::_farClipSQ;
float ddgTBinTree::_varianceScale = 1;
float ddgTBinTree::_cosHalfFOV = 0.7;
ddgMSTri	*ddgTBinTree::_stri;
ddgTBinMesh	*ddgTBinTree::_mesh = NULL;
ddgTCache	*ddgTBinTree::_tcache = NULL;
static bool levelView = false;
void ddgTBinTree::initContext( ddgContext *ctx, ddgTBinMesh *mesh )
{
	// Copy this data to a private structure.
	_mesh = mesh;
	_ctx = ctx;
	_pos.set( _ctx->control()->position()->v[0], _ctx->control()->position()->v[2]);
	_forward.set( _ctx->forward()->v[0], _ctx->forward()->v[2]);
	_farClip = mesh->farClip();
	_nearClip = mesh->nearClip();             
	_farClip2 = _farClip + _farClip;
	_farClipSQ = _farClip * _farClip;
	_stri = mesh->stri;
	_tcache = mesh->tcache();
	_cosHalfFOV = cosf(ddgAngle::degtorad(ctx->fov()/2.0));
	if ((mesh->absMaxError()*_farClip)>0)
		_varianceScale = (ddgPriorityResolution)/(mesh->absMaxError()*_farClip2);
	else
		_varianceScale = 1;
	// Forget about near/far clipping planes.
	// Far is handled seprately and near is not worth bothering with.
	_ctx->rootHull()->noPlanes = 4;
	// Forget about near/far clipping planes.
	_ctx->topDownWedge()->noLines = 2;
}


void ddgTBinTree::updateContext( ddgContext *ctx, ddgTBinMesh * )
{
	_pos.set( ctx->control()->position()->v[0], ctx->control()->position()->v[2]);
	_forward.set( ctx->forward()->v[0], ctx->forward()->v[2]);
	levelView = ctx->levelView();
}

/// Return the index of the 1st child.
ddgTriIndex ddgTBinTree::firstInMesh(void)
{
	// Find 1st triangle in the mesh.
	ddgTriIndex tindex = 1;
	while (tcacheId(tindex) == 0)
	{
		ddgAssert(tindex < _mesh->triNo());
		tindex *= 2;
	}
	return tindex;
}

/** Return the index of the 2nd child.
 *  end must be initialized to 0 and will be reset to 0 when there is nothing left to do.
 */
ddgTriIndex ddgTBinTree::nextInMesh(ddgTriIndex tindex, ddgTriIndex *end )
{
	if (*end == 0) *end = tindex *2;

	// Go to next triangle.
	tindex++;
	// If it is not in the mesh.
	if (tcacheId(tindex) == 0)
	{
		// Try parent.
		if (tcacheId(parent(tindex)) != 0)
		{
			tindex = parent(tindex);
			*end /= 2;
		}
		else // Try 1st child.
		{
			tindex = first(tindex);
			*end *= 2;
		}
	}

	if (tindex == *end)
		*end = 0;

	return tindex;
}
/**
 * Initialize the static information in the bintree.
 *  This method is only called at initialization.
 * Recursively calculate the min/max value of a triangle.
 * With this information we establish a convex hull
 * for each level of the hierarchy, this hull can be
 * use for:
 * visibility culling.
 * collision detection.
 * ray intersection testing.
 * This method is called at initialization and may be
 * called later on when the heights of the terrain are
 * modified.
 */
void ddgTBinTree::initTriangle( ddgTriIndex tindex )
{
	_cacheIndex[tindex] = 0;
	// Get the raw height and store it.
	_rawHeight[tindex] = _mesh->heightMap()->get(mrow(tindex),mcol(tindex));

	// See if we are not at the bottom.
	if (tindex < _mesh->leafTriNo())
	{
		initTriangle(first(tindex)),
		initTriangle(second(tindex));
	}
	calculateVariance(tindex);
}

void ddgTBinTree::calculateVariance(ddgTriIndex tindex)
{
	// Variance for leaf nodes is zero and these are not stored.
	// See if we are not at the bottom.
	if (tindex < _mesh->leafTriNo())
	{
		// Measure displacement of height(tindex) from midpoint of height(first),height(second).
		int hmin, hmax, error;
		if (first(tindex) < _mesh->leafTriNo())
		{
			hmin = (int)ddgUtil::min(_rawMinVal[first(tindex)],_rawMinVal[second(tindex)]);
			hmax = (int)ddgUtil::max(_rawMaxVal[first(tindex)],_rawMaxVal[second(tindex)]);
			error = (int)ddgUtil::max(_treeError[first(tindex)],_treeError[second(tindex)]);
		}
		else // Leaf node.
		{
			hmin = (int)ddgUtil::min(_rawHeight[first(tindex)],_rawHeight[second(tindex)]);
			hmax = (int)ddgUtil::max(_rawHeight[first(tindex)],_rawHeight[second(tindex)]);
			error = 0;
		}

		_rawMinVal[tindex] = (short)ddgUtil::min(_rawHeight[parent(tindex)],hmin);
		_rawMaxVal[tindex] = (short)ddgUtil::max(_rawHeight[parent(tindex)],hmax);
	    _treeError[tindex] = (unsigned short)(error+ddgUtil::abs(((_rawHeight[_stri[tindex].v0]-_rawHeight[_stri[tindex].v1])/2)-_rawHeight[tindex]));
	}
}


// UNTESTED.
void ddgTBinTree::updateVariance(ddgTriIndex tindex) 
{
	ddgAssert(neighbour(tindex) != tindex);
	// Don't bother with leaf nodes.
	if (tindex > _mesh->leafTriNo())
		return;
	// To avoid loops, we only update the triangle with the higher index.

	if (neighbour(tindex) < tindex)
	{
		ddgTBinTree *bt = neighbourTree(tindex);
		// Transfer data to the neighbour.
		bt->_rawHeight[neighbour(tindex)] = _rawHeight[tindex];

		bt->updateVariance(neighbour(tindex)); 
	}
	else
	{
		int hmin = _rawMinVal[tindex];
		int hmax = _rawMaxVal[tindex];
  
		calculateVariance(tindex);

		// update the parent of this vertex 
		if (tindex > 0 && (hmax != _rawMaxVal[tindex] || hmin != _rawMinVal[tindex] )) 
		{
			updateVariance(tindex / 2); 
			if (neighbour(tindex))
			{
				neighbourTree(tindex)->calculateVariance(neighbour(tindex));
				neighbourTree(tindex)->updateVariance(neighbour(tindex) / 2);
			}
		} 
	}
}

/// Return the quad neighbour. If 0 there is no neighbour.
ddgTriIndex ddgTBinTree::neighbour( ddgTriIndex i)
{
	switch(_mesh->stri[i].edge())
	{
	case eINNER:
		return _mesh->stri[i].neighbour;
	case eTOP:
		return _pNeighbourTop ? _mesh->stri[i].neighbour : 0;
	case eLEFT:
		return _pNeighbourLeft ? _mesh->stri[i].neighbour : 0;
	case eDIAG:
		return _pNeighbourDiag ? _mesh->stri[i].neighbour : 0;
	default:
		ddgAsserts(0,"Invalid edge");
		return 0;
	}
}
/// Return the bintree of the neighbour.
ddgTBinTree *ddgTBinTree::neighbourTree( ddgTriIndex i)
{
	switch(_mesh->stri[i].edge())
	{
	case eINNER:
		return this;
	case eTOP:
		return _pNeighbourTop;
	case eLEFT:
		return _pNeighbourLeft;
	case eDIAG:
		return _pNeighbourDiag;
	default:
		ddgAsserts(0,"Invalid edge");
		return 0;
	}
}

/**
 *  Return the cache node which stores the extra data for this triangle
 *  for the purpose of reading data.  If the cacheIndex = 0, we will
 *  get back the default node.
 */
ddgTNode *ddgTBinTree::gnode(ddgTriIndex tindex)
{
	return _mesh->tcache()->get(tcacheId(tindex));
}

/**
 * Return the cache node which stores the extra data for this triangle,
 * for the purpose of setting data.  If the cacheIndex = 0, we redirect
 * to the scratch entry.
 */
ddgTNode *ddgTBinTree::snode(ddgTriIndex tindex)
{
	return _mesh->tcache()->get(tcacheId(tindex)?tcacheId(tindex):1);
}

float ddgTBinTree::heightByPos(unsigned int r, unsigned int c)
{
	// See if we are in the correct tree to find this location.
	ddgAsserts(r + c <= ddgTBinMesh_size+1,"Looking for data out of range.");

	ddgTriIndex tindex = _mesh->lookupIndex(r,c);
	return _mesh->wheight(_rawHeight[tindex]);
}

void ddgTBinTree::vertex(ddgTriIndex tindex, ddgVector3 *vout)
{
/*
	int h = _rawHeight[tindex];

	if (tindex > 3 && tindex < _mesh->leafTriNo())
	{
		unsigned short ti = tcacheId(parent(tindex));

		ddgTNode *tn = gnode(tindex);
		if (tn)
		{
			int d = tn->delta();
			if (d)
			{
				int c = tn->current() + d;
				if ((d < 0 && c < h) ||(d > 0 && c > h))
					tn->delta(0);
				else
				{
					tn->current(c);
					h = c;
				}
			}
		}
	}
*/
	vout->set(mrow(tindex),_mesh->wheight(_rawHeight[tindex]),mcol(tindex));
}

/// Get texture coord data, as a value from 0 to 1 on the bintree.
void ddgTBinTree::textureC(unsigned int tindex, ddgVector2 *vout)
{
	if (_mirror)
		vout->set( ddgTBinMesh_size - _mesh->stri[tindex].row, ddgTBinMesh_size- _mesh->stri[tindex].col);
	else
		vout->set(_mesh->stri[tindex].row, _mesh->stri[tindex].col);

	vout->multiply( 1.0 / (float)(ddgTBinMesh_size+1));
}

/*
/// Get triangle row in the bin tree.
unsigned int ddgTBinTree::row(unsigned int i)
{ return _mesh->stri[i].row; }
/// Get triangle col.
unsigned int ddgTBinTree::col(unsigned int i)
{ return _mesh->stri[i].col; }
*/
/// Get the triangle row on the master mesh.
unsigned int ddgTBinTree::mrow(unsigned int i)
{ return _mirror ? _dr - _mesh->stri[i].row : _dr + _mesh->stri[i].row; }
/// Get the triangle column on the master mesh.
unsigned int ddgTBinTree::mcol(unsigned int i)
{ return _mirror ? _dc - _mesh->stri[i].col : _dc + _mesh->stri[i].col; }

/// QUEUE OPERATIONS


/// Insert triangle tindex into queue.
void ddgTBinTree::insertSQ(ddgTriIndex tindex, ddgPriority pp, ddgCacheIndex ci, ddgVisState vp)
{
    ddgAsserts(tindex >0 && tindex < _mesh->triNo()+2,"Index is out of range.");
	ddgAsserts(tcacheId(tindex) == 0,"Triangle already in queue.");

	// If parent is completely inside or completely outside of view, inherit.
	// Otherwise calculate triangle's visibility.
	ddgVisState v = (vp == ddgIN || vp == ddgOUT || tindex >= _mesh->leafTriNo()) ? vp : visibilityTriangle(tindex);

	ddgAssert(v != ddgUNDEF);

	ddgTNode * tn;
	// Get a new cache entry if we need one.
	if (ci == 0)
	{
		ci = _tcache->allocNode();
		ddgAssert(ci);
	}
	tn = _tcache->get(ci);
	tn->tindex(tindex);
	// Record its value
	tn->vis(v);
	tn->qscacheIndex(0);
	tn->qmcacheIndex(0);
/*
	tn->reset(_rawHeight[tindex],_rawHeight[first(tindex)],_rawHeight[second(tindex)]);
*/
	// Determine what this triangle's priority is going to be.
	ddgPriority pr = 0;
	// Record the cache index for the entry in this bintree.
	tcacheId(tindex, ci);
	if (v != ddgOUT)
	{
		_mesh->incrTriVis();
		// Leaf nodes cannot be split.
		if (tindex < _mesh->leafTriNo())
		{
			tn->priorityFactor(_treeError[tindex]*_varianceScale);
			pr = priorityCalc(tindex,tn->priorityFactor());
			// Ensure our priority is less than our parent's.
			if (pp > 1 && pr >= pp)
				pr = pp-1;
#ifdef _DEBUG
			_mesh->insCountIncr();
#endif
			// If priority warrants a potential split add it to the split queue.
			if (pr > 0)
			{
				ddgCacheIndex qci = _mesh->qscache()->insert(_index,tindex,pr);
				ddgAssert(qci);
				tn->qscacheIndex(qci);
			}
		}
	}
	ddgAssert(pr >= 0 && pr <= ddgPriorityResolution-1);

	tn->priority(pr);
}


/// Remove triangle tindex.
ddgCacheIndex ddgTBinTree::removeSQ(ddgTriIndex tindex )
{
    ddgAsserts(tindex >=0 && tindex <= _mesh->triNo()+2,"Index is out of range.");
    ddgAsserts(tcacheId(tindex),"Triangle is not in mesh.");

	// Dont physically remove this entry from tcache next insert will reuse it.
	ddgCacheIndex ci = tcacheId(tindex);
	ddgAssert(ci);

	if (vis(tindex) !=ddgOUT)
		_mesh->decrTriVis();

	// If we are not a leaf node...
	if (tindex < _mesh->leafTriNo())
	{
		ddgCacheIndex csi = qscacheIndex(tindex);
	    // If we were in the split queue remove us from the queue.
		if (csi)
		{
#ifdef _DEBUG
			_mesh->remCountIncr();
#endif
			ddgAssert(_tcache->get(ci)->qscacheIndex());
			_mesh->qscache()->remove(csi);
			qscacheIndex(tindex,0);
		}
	}
	// Disassociate this triangle from the mesh.
	tcacheId(tindex,0);
	return ci;
}



/*
 Insert triangle tindex into queue if it isn't already there.
 If pr = ddgMAXPRI we need to determine the maximum priority ourselves.
 */
void ddgTBinTree::insertMQ(ddgTriIndex tindex, ddgPriority pr)
{
    ddgAsserts(tindex >=0 && tindex < _mesh->triNo()+2,"Index is out of range.");
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
    ddgAsserts(tindex >=0 && tindex < _mesh->triNo()+2,"Index is out of range.");
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
	cl=tcacheId(first(p));
	cr=tcacheId(second(p));
	ddgAssert(cl&&cr);
	if (n)
	{
		nl=nt->tcacheId(first(n));
		nr=nt->tcacheId(second(n));
		ddgAssert(nl&&nr);
	}

	// Find item in merge queue and remove it.
	ddgCacheIndex mi = qmcacheIndex(tindex);
	ddgAssert(mi);

	_mesh->qmcache()->remove(mi);

	// Mark entries in the tmesh as not present in the merge queue.
	_tcache->get(cl)->qmcacheIndex(0);
	_tcache->get(cr)->qmcacheIndex(0);
	qmcacheIndex(first(p),0);
	qmcacheIndex(second(p),0);
	if (n)
	{
		_tcache->get(nl)->qmcacheIndex(0);
		_tcache->get(nr)->qmcacheIndex(0);
		nt->qmcacheIndex(first(n),0);
		nt->qmcacheIndex(second(n),0);
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
		insertMQ(first(tindex),pr);
}

/*
 * Split a triangle while keeping the mesh consistent.
 * there should be no T junctions. 'i' is a triangle index number.
 * This may require splitting the neighbour as well.
 * return the max priority value of the triangles which were split.
 */
ddgPriority ddgTBinTree::forceSplit2( ddgTriIndex tindex )
{
	ddgTriIndex l = first(tindex), r = second(tindex), n = neighbour(tindex);
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
	ddgVisState v = vis(tindex);
	ddgPriority pr = priority(tindex);
	// We had better be in the queue.
	ddgAssert(tcacheId(tindex));

	// If we were part of a merge diamond, remove us from the merge queue if it is there.
	if (_mesh->merge())
		removeMQ(tindex);

	ci = removeSQ(tindex);

    // Insert the left and right child.
	insertSQ(l,pr,ci,v);
	insertSQ(r,pr,0,v);

    // If this triangle has a merge neighbour, it also needs to be split.
	if (n)
	{
		ddgTBinTree *nt = neighbourTree(tindex);
		ddgAssert(nt);
		// Make sure its children are in the queue.
		if (!nt->tcacheId(first(n)) || !nt->tcacheId(second(n)))
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
	ddgAsserts (tcacheId(first(p)) && tcacheId(second(p)),"Incomplete merge diamond/invalid mesh!");
	ddgAsserts (!n || (nt->tcacheId(first(n)) && nt->tcacheId(second(n))),"Incomplete merge diamond/invalid mesh!");

	// If we were part of a merge diamond, remove us from the merge queue if it is there.
	if (_mesh->merge())
		removeMQ(first(p));
	// Remove its children and neighbour's children.
	ddgCacheIndex ci = 0;
	ci = removeSQ(first(p));

	_tcache->remove(ci);
	ci = removeSQ(second(p));

	// Insert parent and parent's neighbour.
	insertSQ(p,ddgMAXPRI,ci,ddgUNDEF);
	// This operation may have created a mergable diamond.
	if (_mesh->merge())
		insertMQ(p,ddgMAXPRI);

    if (n)
    {
		ddgAssert(nt);
		// If we were part of a merge diamond, remove us from the merge queue if it is there.
		if (_mesh->merge())
			nt->removeMQ(first(n));

		ci = nt->removeSQ(first(n));

		_tcache->remove(ci);
		ci = nt->removeSQ(second(n));
		nt->insertSQ(n,ddgMAXPRI,ci,ddgUNDEF);
		// This operation may have created a mergable diamond.
		if (_mesh->merge())
			nt->insertMQ(n,ddgMAXPRI);
    }
}

void ddgTBinTree::updateMerge(ddgTriIndex tindex, ddgPriority pr )
{
	// Check all 4 members of this potential mergable group.
	// A mergeble group looks as follows:
	//      /|\				//
	//     / | \			//
	//    /__|__\			//
	//    \  |  /			//
	//     \ | /			//
	//      \|/				//
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
	ddgCacheIndex cl = 0, cr = 0, nl = 0, nr = 0;

	ddgAssert( n < ddgNINIT );
	ddgAssert( tindex > 0 );
	ddgTBinTree *nt = n ? neighbourTree(p) : 0;
	ddgAssert((n && nt) || (!n && !nt));

	// If we are the top triangle or if one triangle is missing we can not be merged.
	if    (!p || !(cl=tcacheId(first(p)))
		|| !(cr=tcacheId(second(p)))
		||  (nt && (!(nl=nt->tcacheId(first(n)))
		         || !(nr=nt->tcacheId(second(n)))
			)))
	{
		ddgAssert(!qmcacheIndex(tindex));
		return;
	}
#ifdef _DEBUG
	_mesh->insCountIncr();
#endif
	// Determine the priority for this merge group.
	if (pr == ddgMAXPRI)
	{
		pr = 0;
		if (p < _mesh->leafTriNo())
		{
			// If one of the leaves is visible, find the priority.
			if (vis(first(p)) || vis(second(p)))
			{
				pr = priorityCalc(p,_treeError[p]*_varianceScale);
			}
			// If there are neighbours and one of them is visible.
			if (nt && (nt->vis(first(n)) || nt->vis(second(n))))
			{
				ddgPriority pr2 = priorityCalc(n, nt->_treeError[n]*_varianceScale);
				if (pr2 > pr)
					pr = pr2;
			}
		}
	}

	// If we were already in the queue, this will become a move.
	ddgPriority opr = ddgMAXPRI;
	ddgCacheIndex mi = qmcacheIndex(tindex);
	if (mi)
	{
		ddgQNode *qn = _mesh->qmcache()->get(mi);
		opr = _mesh->qmcache()->convert(qn->bucket());
	}

	// If nothing changed, leave..
	if (opr == pr)
		return;

	// Move the old entry.
	if (mi)
	{
		_mesh->qmcache()->move(mi,pr);
		return;
	}

	// Insert a new entry.
	mi = _mesh->qmcache()->insert(_index,tindex,pr);
	// Mark these entries in the tmesh as present in the merge queue.
	_tcache->get(cl)->qmcacheIndex(mi);
	_tcache->get(cr)->qmcacheIndex(mi);
	qmcacheIndex(first(p),mi);
	qmcacheIndex(second(p),mi);
	if (nt)
	{
		_tcache->get(nl)->qmcacheIndex(mi);
		_tcache->get(nr)->qmcacheIndex(mi);
		nt->qmcacheIndex(first(n),mi);
		nt->qmcacheIndex(second(n),mi);
	}
}

// TODO: this method can be written not to be recursive, but the last time
// I did that it didn't have and performance impact.

void ddgTBinTree::updateSplit(ddgTriIndex tindex, ddgVisState parVis )
{
	ddgAsserts(tindex <= _mesh->triNo(), " No leaf node was found in the queue!");

	ddgVisState v;

	// If parent is completely inside or completely outside, inherit.
	if (parVis == ddgOUT || parVis == ddgIN || tindex >= _mesh->leafTriNo())
	{
		v = parVis;
	}
	// We dont know so analyze ourselves.
	else
	{
		v = visibilityTriangle(tindex);
		if (tindex == 1)
			treeVis(v);
	}

	// This will only go as far as the leaves which are currently in the mesh.
	if (!tcacheId(tindex))
	{
		updateSplit(first(tindex),v);
		updateSplit(second(tindex),v);
		return;
	}

	// This is a leaf, update it.
	// Adjust its priority.
	// Add and remove from queues as needed.
	ddgVisState ov = vis(tindex);
	ddgAssert(v != ddgUNDEF);

	// Determine what this triangle's priority is going to be.
	ddgPriority opr = priority(tindex);
	ddgPriority pr = (v == ddgOUT || tindex >= _mesh->leafTriNo()) ? 0 : priorityCalc(tindex,priorityFactor(tindex));

	// If nothing changed return
	if (pr == opr && v == ov)
		return;

	// If priority or visibility has changed we need to update the state.
#ifdef _DEBUG
	if (pr != opr || v != ov)
	{
		_mesh->movCountIncr();
	}
#endif
	vis(tindex,v);

	if (v && !ov)
		_mesh->incrTriVis();
	else if (!v && ov)
		_mesh->decrTriVis();

	// Leaf nodes couldn't possibly be in the split queue, so leave now.
	if ((pr == opr)||(tindex >= _mesh->leafTriNo()))
		return;

	priority(tindex,pr);

	// Priority increased/decreased.
	// Change queue position.
	ddgCacheIndex qci = qscacheIndex(tindex);

	if (opr)
	{
		_mesh->qscache()->remove(qci);
		qscacheIndex(tindex,0);
	}
	if (pr)
	{
		qci = _mesh->qscache()->insert(_index,tindex,pr);
		ddgAssert(qci);
		qscacheIndex(tindex,qci);
	}

	ddgAssert(v != ddgOUT || pr == 0);
}


/// VISIBILITY
static ddgBBox3		bbox;
static ddgRect2	    rect;

ddgVisState ddgTBinTree::visibilityTriangle(ddgTriIndex tindex)
{
	ddgAsserts(tindex <= _mesh->triNo(), " No leaf node was found in the queue!");
	ddgAsserts(tindex < _mesh->leafTriNo(), " Dont bother with leaf node testing, inherit from parent!");
#ifdef _DEBUG
	_mesh->visCountIncr();
#endif

	// See if the triangle is too far from the camera.  Ingore height component.
	// Use the triangle's centre point.
	int cr = _dr, cc = _dc;
	if (_mirror) 
		{ cr -= _stri[tindex].row;  cc -= _stri[tindex].col; }
	else 
		{ cr += _stri[tindex].row;  cc += _stri[tindex].col; }
	cr -= (int)_pos[0];
	cc -= (int)_pos[1];
	ddgVector2 center(cr ,cc );

	// Do a rough distance test and optionally a true distance test in 2D
	if (center[0] > _farClip || center[0] < - _farClip
	  ||center[1] > _farClip || center[1] < - _farClip
	  ||center.sizesq() > _farClipSQ)
		return ddgOUT;

	ddgTriIndex tva = parent(tindex),
		tv0 = _mesh->v0(tindex),
		tv1 = _mesh->v1(tindex);

	int r0 = _dr, r1 = _dr, ra = _dr, c0 = _dc, c1 = _dc, ca = _dc;
	if (_mirror)
	{
		r0 -= _stri[tv0].row; r1 -= _stri[tv1].row; ra -= _stri[tva].row;
		c0 -= _stri[tv0].col; c1 -= _stri[tv1].col; ca -= _stri[tva].col;
	}
	else
	{
		r0 += _stri[tv0].row; r1 += _stri[tv1].row; ra += _stri[tva].row;
		c0 += _stri[tv0].col; c1 += _stri[tv1].col; ca += _stri[tva].col;
	}

	int rmin, rmax, cmin, cmax;
	// Bounding box is defined by points tva, v0 and v1.
	rmin = (r0 < r1) ? r0 : r1;
	cmin = (c0 < c1) ? c0 : c1;
	rmax = (r0 > r1) ? r0 : r1;
	cmax = (c0 > c1) ? c0 : c1;
	if (ra < rmin) rmin = ra;
	if (ca < cmin) cmin = ca;
	if (ra > rmax) rmax = ra;
	if (ca > cmax) cmax = ca;
	// Calculate bounding box of bintree.
	// Update the Y component if the view is not level.
	if (!levelView )
		{
		bbox.min.v[0] = rmin;
		bbox.max.v[0] = rmax;
		bbox.min.v[2] = cmin;
		bbox.max.v[2] = cmax;
		// Adjust box by bintree thickness.
		bbox.min.v[1] = _mesh->wheight(_rawMinVal[tindex]);
		bbox.max.v[1] = _mesh->wheight(_rawMaxVal[tindex]);
		return _ctx->rootHull()->clip(&bbox);
		}
	else
		{
		rect.min.v[0] = rmin;
		rect.max.v[0] = rmax;
		rect.min.v[1] = cmin;
		rect.max.v[1] = cmax;
		return _ctx->topDownWedge()->clip(&rect);
		}
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
	ddgPlane3 p(p0,p1,p2);
	p.projectAlongY(&v);

    return v[1];
}
#ifdef DDG
ddgHistogram *prioHist = NULL;
void ddgTBinTree::setHist( ddgHistogram * hist)
{
	prioHist = hist;
}
#endif
/// Calculate priority of triangle tindex  We assume that we only get called
/// for visible triangles which are not leaves, because all others have priority 0.
ddgPriority ddgTBinTree::priorityCalc(ddgTriIndex tindex, float pf)
{
	ddgAssert(tindex < _mesh->leafTriNo());

	// Calculate the distance of the nearest point to the
	// near clipping plane by taking the dot product of
	// the near clipping plane's vector with the vector
	// from the camera to the point.
	// $TODO if camera is moving forward, we can store z and estimate delta z.
	float z;
	int cr = _dr, cc = _dc;
	if (_mirror) 
	{ cr -= _stri[tindex].row;  cc -= _stri[tindex].col; }
	else 
	{ cr += _stri[tindex].row;  cc += _stri[tindex].col; }
	cr -= (int)_pos[0];
	cc -= (int)_pos[1];
	ddgVector2 p(cr,cc);
	// This is the same a f dot p1 - f dot p2, f dot p2 can be precomputed,
	// but I don't think that will speed things up.
	z = _farClip2 - _forward.dot(&p);
	if (z < _nearClip)
		z = _nearClip;
	// Priority is factor of:
	// Distance from viewer.
	// Convex hull thickness.
	ddgAsserts(z > 0 && z < 1000,"Bad distance");
	// zDist is >= nearClip     && < farClip
	// Max - Min >= 0   && < _absDiffHeight.

	z *= pf;

#ifdef _DEBUG
	_mesh->priCountIncr();
#endif
	ddgAssert(z >= 0);
#ifdef DDG
	if (prioHist)
	{
		int i = z >= ddgPriorityResolution ? ddgPriorityResolution-1 : z;
		prioHist->incr(i);
	}
#endif
	return (ddgPriority)(z >= ddgPriorityResolution ? ddgPriorityResolution-1 : z);

}


// UNTESTED

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
		tv1 = _mesh->v0(tindex),
		tv0 = _mesh->v1(tindex);
	static ddgVector3 v0,v1,va;
    vertex(tva,&v0);
    vertex(tv0,&v1);
    vertex(tv1,&va);
	// Find bounding square of this triangle.
	static ddgVector2 r1, r2, r3, r4;
	r1.set(v0[0],v0[2]);
	r2.set(v1[0],v1[2]);
	static ddgRect2 t(&r1, &r2);
	// Find bounding rectangle of ray.
	r3.set(p1[0],p1[2]);
	r4.set(p2[0],p2[2]);
	static ddgRect2 r(&r3, &r4);
	// See if rectangles intersect.
	if (!t.intersect(&r))
		return false;
	// Find line equation.
	ddgLine3 line(p1,p2);
	ddgVector3 e1(va);	// Edge1
	ddgVector3 e2(va);	// Edge2
	// Find the Z coord where X = Xa.
	line.solve(&e1,0);
	// Find the X coord where Z = Za.
	line.solve(&e2,0);
	// The two intersection points of the line with the triangle.
	ddgVector3 s[2];
	int i = 0;
	if (e1[0] >= t.min[0] && e1[0] < t.max[0])
		s[i++] = e1;
	if (e2[2] >= t.min[2] && e2[2] < t.max[2])
		s[i++] = e2;
	// We must cross 2 of the three sides of the triangle with the line.
	// If we haven't crossed any side yet, then we must have missed this triangle.
	// See if segment fall outside of triangle.
	if (i==0)
		return false;
	if (i==1)
	{
		// We must be crossing the diagonal, calculate that point.
		ddgLine3 diag(v0,v1);
		line.intersect(&diag,&(s[1]));
	}
	// We have the line segment which crosses this triangle in s[0]->s[1].
	ddgVector2 sg1(s[0][0],s[0][1]), sg2(s[1][0],s[1][1]);
	ddgRect2 sr(&sg1, &sg2),cs;
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
		if (rayTest(p1,p2,first(tindex),depth))
			return true;
		return rayTest(p1,p2,second(tindex),depth);
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

