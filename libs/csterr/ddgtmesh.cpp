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
#include "csterr/ddgtmesh.h"
#include "csterr/ddgsplay.h"
#include "csterr/ddgbtree.h"

extern void transformer(csVector3 vin, csVector3 *vout);

// ----------------------------------------------------------------------
// NOTE This value is the maximum number of vertices that can be converted
// per frame, this is approximatesly 3x the number of triangles in the frame.
#define ddgVCACHESIZE	15000
ddgVCache::ddgVCache( void )
{
	_size = 0;
	_used = 0;
}

void ddgVCache::init (unsigned int size )
{
	_cache = new csVector3[size];
	_size = size;
	_used = 0;
}
///
ddgVCache::~ddgVCache( void )
{
	delete _cache;
}
/// Get entry.
csVector3		*ddgVCache::get(unsigned short index)
{
	return &_cache[index];
}
/// New entry.
unsigned short	ddgVCache::alloc(void)
{
	assert(_used + 1 < _size);
	_used++;
	return _used;
}
///
void ddgVCache::reset(void)
{
	_used = 0;
}


/// Return a priority from the split queue.
unsigned int ddgTBinMesh::prioritySQ( ddgSplayIterator *i )
{
	ddgSplayKey *sk = _qs->retrieve(i->current());
	return getBinTree(sk->tree())->priority(sk->index());
}
/// Return a priority from the merge queue.
unsigned int ddgTBinMesh::priorityMQ( ddgSplayIterator *i )
{
	ddgSplayKey *sk = _qm->retrieve(i->current());
	return getBinTree(sk->tree())->priority(sk->index());
}

// ----------------------------------------------------------------------
//const unsigned int TBinMesh_size = 64;	// 64 seems to be a good size.
ddgHeightMap *heightMap = 0;
ddgTBinMesh::ddgTBinMesh( ddgHeightMap * h )
{
	heightMap = h;
	_bintreeMax = 2*(heightMap->cols()-1)/(ddgTBinMesh_size) * (heightMap->rows()-1)/(ddgTBinMesh_size);
	typedef ddgTBinTree *ddgTBinTreeP;
	_bintree = new ddgTBinTreeP[_bintreeMax];
	_detail = 2000;
	_farclip = 150;

	_nearclip = 0;
	_progDist = _farclip/3;
	// Allocate memory for transformed vertices.
	_vcache.init(ddgVCACHESIZE);

	_maxLevel = (unsigned int) (log(ddgTBinMesh_size)/log(2.0) * 2); // Could be constant.
	_triNo = (unsigned int)pow(2,(_maxLevel)); // Inside points can be shared.
	stri = new ddgMSTri[_triNo+2];

	// Queues to hold the visible, clipped and sorted visible triangles.
	_qs = new ddgSplayTree( );
	_qm = new ddgSplayTree( );
	_qz = new ddgSplayTree( );
	// By default the iterators support 1000 levels of nesting.
	_qsi = new ddgSplayIterator(_qs);
	_qmi = new ddgSplayIterator(_qm);
	_qzi = new ddgSplayIterator(_qz);

	unsigned int i;
	for (i = 0; i < _bintreeMax; i++ )
		_bintree[i] = NULL;
	// Initialize the queue on the 1st frame.
	clearSQ();
	clearMQ();

	// Dummy entries.
	stri[_triNo].row = ddgTBinMesh_size;
	stri[_triNo].col = 0;
	stri[_triNo+1].row = 0;
	stri[_triNo+1].col = ddgTBinMesh_size;
	// Top level triangles.
	stri[0].row = 0;
	stri[0].col = 0;
	stri[1].row = (ddgTBinMesh_size)/2;
	stri[1].col = (ddgTBinMesh_size)/2;
    initVertex(1,0,_triNo,_triNo+1,1);
	initBrothers();

    _tanHalfFOV = tan(ddgAngle::degtorad(90.0/2.0));
    _camBBox = NULL;
	/// Total number of triangles rendered by this camera.
	_triCount = 0;
	/// Total number of priorities calculated by this camera.
	_priCount = 0;
	/// Total number of queue insertions by this camera.
	_insCount = 0;
	/// Total number of queue removals by this camera.
	_remCount = 0;
	/// Total number of queue updates by this camera.
	_movCount = 0;

}

ddgTBinMesh::~ddgTBinMesh(void)
{
	delete stri;
	delete _qs;
	delete _qm;
	delete _qz;

	delete _bintree;
}


/// Function to add a bintree.
void ddgTBinMesh::addBinTree( ddgTBinTree *bt )
{
	unsigned int i = 0;
	while (i < _bintreeMax && _bintree[i] != NULL) i++;
	if (i < _bintreeMax)
	{
		_bintree[i] = bt;
		bt->index(i);
	}
}
/// Function to remove a bintree.
void ddgTBinMesh::removeBinTree( ddgTBinTree *bt )
{
	unsigned int i = 0;
	while (i < _bintreeMax && _bintree[i] != bt) i++;
	if (i < _bintreeMax)
	{
		_bintree[i]->index(0);
		_bintree[i] = NULL;
	}
}

/** Initialize the bintree structure including
 *  the wedges and other mesh values.
 */
bool ddgTBinMesh::init( double *worldToCameraMatrix, ddgBBox *camClipBox, float fov )
{
    // Set world to camera transformation matrix.
    _wtoc = worldToCameraMatrix; 
    // Set the camera bounding box.
    _camBBox = camClipBox;
    // Set the field of view.
    _tanHalfFOV = tan(ddgAngle::degtorad(fov/2.0));

    ddgTBinTree	*bintree1 = NULL,
				*bintree2 = NULL;
	// Created the enough bintrees to cover the terrain.
	unsigned int i, j;
	_nc = (heightMap->cols()-1)/ddgTBinMesh_size;
	_nr = (heightMap->rows()-1)/ddgTBinMesh_size;

//cerr << "Creating " << _nc << " x " << _nr << " Tiles" << endl;
	for (i = 0; i < _nr; i++)
	{
		for (j = 0; j < _nc; j++)
		{
			bintree1 = new ddgTBinTree(this,heightMap,i*ddgTBinMesh_size,j*ddgTBinMesh_size);
			bintree2 = new ddgTBinTree(this,heightMap,(i+1)*ddgTBinMesh_size,(j+1)*ddgTBinMesh_size,true);
		}
	}

	// Define relationship between meshes.
	for (i = 0; i < _nr; i++)
	{
		for (j = 0; j < _nc; j++)
		{
			_bintree[2*(i*_nc+j)]->pNeighbourDiag( _bintree[2*(i*_nc+j)+1]);
			_bintree[2*(i*_nc+j)]->pNeighbourLeft( (j==0)? 0: _bintree[2*(i*_nc+j-1)+1]);
			_bintree[2*(i*_nc+j)]->pNeighbourTop( (i==0)? 0: _bintree[2*((i-1)*_nc+j)+1]);
			//
			_bintree[2*(i*_nc+j)+1]->pNeighbourDiag( _bintree[2*(i*_nc+j)]);
			_bintree[2*(i*_nc+j)+1]->pNeighbourLeft( (j+1==_nc)? 0: _bintree[2*(i*_nc+j+1)]);
			_bintree[2*(i*_nc+j)+1]->pNeighbourTop( (i+1==_nr)? 0: _bintree[2*((i+1)*_nc+j)]);
		}
	}
	i = 0;

    return false;
}

/**
 *  Initialize the vertex and row/col data of the STri.
 *  Passed in are the triangles which carry the points
 *  that describe the current triangle.
 *  va is immediate parent.
 *  v1 and v0 are the left and right vertices of va. 
 */

void ddgTBinMesh::initVertex( unsigned int level,
					 ddgTriIndex va,	// Top
					 ddgTriIndex v0,	// Left
					 ddgTriIndex v1,	// Right
					 ddgTriIndex vc)	// Centre
{
	// Initialize row and col for this triangle.
	stri[vc].row = (stri[v0].row + stri[v1].row) / 2;
	stri[vc].col = (stri[v0].col + stri[v1].col) / 2;

	// See if we are not at the bottom.
	if (level < _maxLevel)
	{
		initVertex(level+1,vc,va,v0,ddgTBinTree::left(vc)),
		initVertex(level+1,vc,v1,va,ddgTBinTree::right(vc));
	}
	stri[vc].v0 = v0;
	stri[vc].v1 = v1;

}

// Constant used for identifying brothers.
const	unsigned int BINIT = 0xFFFFFFFF;

// Precompute the brother field, this is used for merge diamonds.
// This function can now initialize in constant time, there is an algoritm!!!
// This function used to take 101689 ticks on a 256x256 mesh,
//  it now takes only 168, that is over 600x faster (previous logic was O(nxn)).
void ddgTBinMesh::initBrothers( void )
{
	// Find the brothers of all triangles.
	unsigned int t, b, k = 2, lk = 1, l = 0, klk, kk, kt;

	// initialize all brothers to BINIT.
	for (t = 0; t < _triNo+2; t++)
		stri[t].brother = BINIT;

	stri[1].brother = 1;
	stri[0].brother = 0;
	stri[_triNo].brother = _triNo;
	stri[_triNo+1].brother = _triNo+1;
//cerr << "Computing brothers...";
	// Find the brother of each tri.
	while ((++l) < _maxLevel)
	{
        b = lk;
		klk = k+lk;
		kk = k+k;
		for (t = 0; t < lk; t++) // Only do half because of symmetry.
		{
			kt = k+t;
			if (stri[kt].brother != BINIT)
			{
				continue;
			}
            else if (l > 2 && l % 2 == 1) // Odd levels are a double copy of prev even level.
            {
                stri[kt].brother = stri[lk+t].brother + b;
                stri[klk+t].brother = stri[lk+t].brother + lk + b;
            }
    	    // Check Edge cases
			else if ((b = edge(kt)) != 0)
			{
				if (b==3) // Diagonal.
				{
				stri[kt].brother = kk-t-1;
				stri[kk-t-1].brother = kt;
				}
				else
				{
				stri[kt].brother = klk-t-1;
				stri[kk-t-1].brother = klk+t;
				}
			}
			// Normal case
			else
			{
				// If the same row in the previous level was known, inherit.
                if (lk && t < lk && !edge(lk + t))
                {
					stri[kt].brother = stri[lk + t].brother + lk;
					stri[stri[kt].brother].brother = kt;
					stri[klk+t].brother = stri[lk + t].brother + k;
					stri[lk+stri[kt].brother].brother = klk + t;
                }
                else // It is a new brother.
                {
					b = k - 1 - t;
					stri[kt].brother = k+b;
					stri[k+b].brother = kt;
                }
			}
		}
        lk = k;
		k = kk;  // 2^n
	}
}

void ddgTBinMesh::calculate( void )
{
	_dirty = true; // Assume always dirty for CS
	if (_dirty)
	{
		csVector3 ev0,ev1, tmp;
		unsigned int i = 0;

		// Calc transform of unit vector once per frame and scale the rest.
		transformer( csVector3(0,0,0), &ev0 );
		transformer( csVector3(0,1,0), &ev1 );
		tmp = ev1 - ev0;
		ddgTBinTree::unit( &tmp);
		// Reset the visibility counter.
		_visTri = 0;
		_vcache.reset();
		// Calculate visibility info for all triangle currently in the mesh
		// at the current camera position.
		while (i < _bintreeMax)
		{
			if (_bintree[i])
			{
				_bintree[i]->resetVisTri();
				_bintree[i]->tri(_bintree[i]->triNo())->reset();
				_bintree[i]->tri(_bintree[i]->triNo()+1)->reset();
				_bintree[i]->tri(0)->reset();

				_bintree[i]->reset(1,maxLevel());
				_bintree[i]->visibility(1,maxLevel());
			}
			i++;
		}
		// Update priorities based on frustrum culling etc.
		// If a priority has changed, the triangle needs to be removed and reinserted
		i = 0;
		while (i < _bintreeMax)
		{
			if (_bintree[i])
			{
				_bintree[i]->priorityUpdate(1,maxLevel());
			}
			i++;
		}
	
		// See if there are merge diamonds which are not visible, if so, merge them.
		{
			bool done = false;
			ddgTriIndex tindex;
			ddgTBinTree *bt;
			while(!done && _qm->size())
			{
			// This not the most efficient way but will do for now, perhaps this
			// operation can be rolled in to the priority Update?
				_qmi->reset();
				ddgSplayKey *sk;
				while (!_qmi->end()) 
				{
					sk = _qm->retrieve(_qmi->current());
					bt = getBinTree(sk->tree());
					tindex = sk->index();
					if (!bt->isDiamondVisible(tindex)
						&& !bt->tri(tindex)->state().flags.merged)
					{
						bt->forceMerge(tindex);
						break;
					}
					_qmi->next();
				}
				if (_qmi->end())
				  done = true;
			}
		}
	
		// Get the optimal number of triangles in the queue.
		balanceQueue();
        // Track the number of triangles produced be this calculation.
	    _triCount += _qs->size();

		_dirty = false;
	}

}

// Destroy all elements in the queue.
void ddgTBinMesh::clearSQ(void)
{
	_qsi->reset();
	while(!_qsi->end()) 
	{
		treeSQ(_qsi)->tri(indexSQ(_qsi))->_state.flags.sq = false;
		_qsi->next();
	}

	// Clear queue.
	_qs->clear();
}

// Destroy all elements in the queue.
void ddgTBinMesh::clearMQ(void)
{
	_qmi->reset();
	while(!_qmi->end()) 
	{
		treeMQ(_qmi)->tri(indexMQ(_qmi))->_state.flags.sq = false;
		_qmi->next();
	}
	// Clear queue.
	_qm->clear();
}

//
bool ddgTBinMesh::splitQueue(void)
{
	bool done = false;
	bool split = false;
	// Remove the biggest item, split it.
	_qsi->reset(true);
	while (!split)
	{
		ddgTBinTree *bt = treeSQ(_qsi);
		ddgTriIndex i = indexSQ(_qsi);
		if ( i < _triNo/2 
			&& !bt->tri(i)->_state.flags.split
			&& !bt->tri(i)->_state.flags.merged
			&& !bt->tri(i)->_vis.flags.none) // Not a leaf!
		{
			bt->forceSplit(i);
			split = true;
		}
		else
		{
			if (_qsi->end())
				done = split = true;
			_qsi->next();
		}
	}
	return done;
}

//
bool ddgTBinMesh::mergeQueue(void)
{
	bool done = false;
	bool merged = false;
	// Remove the smallest item, merge it.
	_qmi->reset();
	while (!merged)
	{
		ddgTBinTree *bt = treeMQ(_qmi);
		ddgTriIndex i = indexMQ(_qmi);
		if ( i > 1 && !bt->tri(i)->_state.flags.merged) // Not a root!
		{
			bt->forceMerge(i);
			merged = true;
		}
		else
		{
			if (_qmi->end())
				done = merged = true;
			_qmi->next();
		}
	}
	return done;
}

unsigned int ddgTBinMesh::updateVisibleTri(void)
{
	unsigned int i = 0;
	_visTri = 0;
	while (i < _bintreeMax)
	{
		if (_bintree[i])
		{
			_visTri += _bintree[i]->visTri();
		}
		i++;
	}
	return _visTri;
}
/* Reoptimize the mesh by splitting high priority triangles
 * and merging low priority diamonds.
 */
unsigned int ddgTBinMesh::balanceQueue(void)
{
	unsigned int minTri = _detail;
	unsigned int maxTri = _detail+20;
	//unsigned int maxPri = 400; // Unused
	unsigned int maxSp = 0, minMp = 0, qsz;
	qsz =  _qs->size();
	if (qsz)
	{
		_qsi->reset(true);
		maxSp = prioritySQ(_qsi);
	}
	if (_qm->size())
	{
		_qmi->reset();
		minMp = priorityMQ(_qmi);
	}
	// Loops should be gone no that we have the flags to indicate if a triangle has been split/merged
	// in this cycle.
    unsigned int maxLoop = 1000; // Don't make more than 1000 adjustments. 
    unsigned int count = 0;
    unsigned int pmaxSp = 0, pminMp = 0, pqs = 0, pqm = 0;
    // General case.
	while (
        (( updateVisibleTri() > maxTri) &&_qm->size() )||    // Too many triangles.
        (_visTri < minTri ) ||                  // Too few triangles.
        (_qm->size() &&                         // Right amount of tris, but:
              (maxSp)			                // Highest split priority >
            > (minMp)                           // Lowest merge priority.
        ))

	{
        // See if we need to merge
		if ( _qm->size() &&
            (   _visTri > maxTri                          // We have too many triangles.
             || (maxSp > minMp && _visTri >= maxTri) )    // Some non-contributing triangles, can be merged.
           )
		{
			// Merge the diamond of lowest priority.
			mergeQueue();
		}
		// See if we need to split.
		if ((_visTri < minTri                           // We don't have enough triangles.
             || (_visTri < maxTri && maxSp > minMp ) )  // We have triangles which should be split.
            )
		{
			// So  split the triangle of highest priority.
			splitQueue();
		}

        if (count++ > maxLoop)
        {
            break;  // We are taking too long.
        }
        if (maxSp == pmaxSp && minMp == pminMp && _qs->size() == pqs && _qm->size() == pqm)
            break;  // We are not making progress.
        pmaxSp = maxSp;
        pminMp = minMp;
        pqs = _qs->size();
        pqm = _qm->size();
		if (pqs)
		{
			_qsi->reset(true);
		    maxSp = prioritySQ(_qsi);
		}
		if (pqm)
		{
			_qmi->reset();
			minMp = priorityMQ(_qmi);
		}
	}
	return count;
}


void ddgTBinMesh::pos(float *x, float *z, ddgTBinTree **bt, unsigned int *r, unsigned int *c)
{
    // Find the global(mesh) column and row of this coordinate.
	unsigned int gr = *x < 0 ? 0 : (unsigned int)(*x/ddgTBinMesh_size);
	unsigned int gc = *z < 0 ? 0 : (unsigned int)(*z/ddgTBinMesh_size);
	if (gr >= _nr) gr = _nr-1;
	if (gc >= _nc) gc = _nc-1;
    // Find the offset in the tree and row and column within the tree.
    *x = *x - gr * ddgTBinMesh_size;
	*z = *z - gc * ddgTBinMesh_size;
    // Do we fall in to the tree across the diagonal?
	bool mirror = (*x+*z < ddgTBinMesh_size)?false:true;
    // If so calculate the row and column w.r.t. that tree.
	if (mirror)
	{
        *x = ddgTBinMesh_size - *x;
        *z = ddgTBinMesh_size - *z;
	}
    *r = (unsigned int)*x;
    *c = (unsigned int)*z;
    // Calculate the distance from the triangle origin to the coord.
    *x = *x - *r;
    *z = *z - *c;
    // Calculate the bin tree.
	*bt = _bintree[(gr * _nc + gc)*2+(mirror?1:0)];
}

float ddgTBinMesh::height(float x, float z)
{
    // Find the binTree which owns this coordinate.
    ddgTBinTree *bt = 0;
    unsigned int r, c;
    pos(&x,&z,&bt, &r, &c);
    // Find mesh height at this location.
	return bt ? bt->treeHeight(r,c,x,z) : 0.0;
}

