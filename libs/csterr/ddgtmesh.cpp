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


// ----------------------------------------------------------------------
// NOTE This value is the maximum number of vertices that can be converted
// per frame, this is approximatesly 3x the number of triangles in the frame.
#define ddgVCACHESIZE	20000
ddgVCache::ddgVCache( void )
{
	_cache = 0;
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

	_mindetail = 3000;
	_maxdetail = 3200;

	_farclip = 150;
	_nearclip = 0;
	_progDist = _farclip/15;
	_merge = true;
	// Allocate memory for transformed vertices.
	_vcache.init(ddgVCACHESIZE);

	_maxLevel = (unsigned int) (log(ddgTBinMesh_size)/log(2.0) * 2); // Could be constant.
	_triNo = (unsigned int)pow(2,(_maxLevel)); // Inside points can be shared.
	stri = new ddgMSTri[_triNo+2];

	// Queues to hold the visible, clipped and sorted visible triangles.
	_qs = new ddgSplayTree( );
	_qm = new ddgSplayTree( );
	// By default the iterators support 1000 levels of nesting.
	_qsi = new ddgSplayIterator(_qs);
	_qmi = new ddgSplayIterator(_qm);

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
	initNeighbours();

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
	// Force calculate to run 1st time.
	_balanceCount = 1;

}

ddgTBinMesh::~ddgTBinMesh(void)
{
	delete stri;
	delete _qs;
	delete _qm;

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
	}
}
/// Function to remove a bintree.
void ddgTBinMesh::removeBinTree( ddgTBinTree *bt )
{
	unsigned int i = 0;
	while (i < _bintreeMax && _bintree[i] != bt) i++;
	if (i < _bintreeMax)
	{
		_bintree[i] = NULL;
	}
}

/** Initialize the bintree structure including
 *  the wedges and other mesh values.
 */
bool ddgTBinMesh::init( double *worldToCameraMatrix, ddgBBox *camClipBox, float fov )
{
	ddgAssert(camClipBox);
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

#ifdef DDG
	cerr << "Creating " << _nc << " x " << _nr << " Tiles" << endl;
#endif
	ddgTBinTree::initWtoC(this);

	ddgTreeIndex index = 0;
	for (i = 0; i < _nr; i++)
	{
		for (j = 0; j < _nc; j++)
		{
			bintree1 = new ddgTBinTree(this,index,heightMap,i*ddgTBinMesh_size,j*ddgTBinMesh_size);
			ddgMemorySet(ddgTBinTree,1);
			addBinTree(bintree1);
			index++;
			bintree2 = new ddgTBinTree(this,index,heightMap,(i+1)*ddgTBinMesh_size,(j+1)*ddgTBinMesh_size,true);
			ddgMemorySet(ddgTBinTree,1);
			addBinTree(bintree2);
			index++;
		}
	}

	// Define relationship between meshes.
	for (i = 0; i < _nr; i++)
	{
		for (j = 0; j < _nc; j++)
		{
			_bintree[2*(i*_nc+j)]->pNeighbourDiag( _bintree[2*(i*_nc+j)+1]);
			_bintree[2*(i*_nc+j)]->pNeighbourLeft( (j==0)? NULL: _bintree[2*(i*_nc+j-1)+1]);
			_bintree[2*(i*_nc+j)]->pNeighbourTop( (i==0)? NULL: _bintree[2*((i-1)*_nc+j)+1]);
			//
			_bintree[2*(i*_nc+j)+1]->pNeighbourDiag( _bintree[2*(i*_nc+j)]);
			_bintree[2*(i*_nc+j)+1]->pNeighbourLeft( (j+1==_nc)? NULL: _bintree[2*(i*_nc+j+1)]);
			_bintree[2*(i*_nc+j)+1]->pNeighbourTop( (i+1==_nr)? NULL: _bintree[2*((i+1)*_nc+j)]);
		}
	}
#ifdef DDG
	cerr << endl << "Calculating wedgies:";
#endif
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

// Constant used for identifying neighbour.
const	unsigned int ddgNINIT = 0xFFFFFFFF;

// Precompute the neighbour field, this is used for merge diamonds.
// This function can now initialize in constant time, there is an algoritm!
void ddgTBinMesh::initNeighbours( void )
{
	// Find the neighbours of all triangles.
	unsigned int t, b, k = 2, lk = 1, l = 0, klk, kk, kt;

	// initialize all neighbours to ddgNINIT.
	for (t = 0; t < _triNo+2; t++)
		stri[t].neighbour = ddgNINIT;

	stri[1].neighbour = 1;
	stri[0].neighbour = 0;
	stri[_triNo].neighbour = _triNo;
	stri[_triNo+1].neighbour = _triNo+1;
#ifdef DDG
	cerr << "Computing neighbour...";
#endif
	// Find the neighbour of each tri.
	while ((++l) < _maxLevel)
	{
        b = lk;
		klk = k+lk;
		kk = k+k;
		for (t = 0; t < lk; t++) // Only do half because of symmetry.
		{
			kt = k+t;
			if (stri[kt].neighbour != ddgNINIT)
			{
				continue;
			}
            else if (l > 2 && l % 2 == 1) // Odd levels are a double copy of prev even level.
            {
                stri[kt].neighbour = stri[lk+t].neighbour + b;
                stri[klk+t].neighbour = stri[lk+t].neighbour + lk + b;
            }
    	    // Check Edge cases
			else if (b = edge(kt))
			{
				if (b==3) // Diagonal.
				{
				stri[kt].neighbour = kk-t-1;
				stri[kk-t-1].neighbour = kt;
				}
				else
				{
				stri[kt].neighbour = klk-t-1;
				stri[kk-t-1].neighbour = klk+t;
				}
			}
			// Normal case
			else
			{
				ddgAssert(stri[kt].neighbour == ddgNINIT);
				// If the same row in the previous level was known, inherit.
                if (lk && t < lk && !edge(lk + t))
                {
					stri[kt].neighbour = stri[lk + t].neighbour + lk;
					stri[stri[kt].neighbour].neighbour = kt;
					stri[klk+t].neighbour = stri[lk + t].neighbour + k;
					stri[lk+stri[kt].neighbour].neighbour = klk + t;
                }
                else // It is a new neighbour.
                {
					b = k - 1 - t;
					ddgAssert ( stri[kt].col == stri[k+b].col
					  && stri[kt].row == stri[k+b].row);
					stri[kt].neighbour = k+b;
					stri[k+b].neighbour = kt;
                }
			}
		}
        lk = k;
		k = kk;  // 2^n
	}
}

bool ddgTBinMesh::calculate( void )
{
	// Dont do anything if we didnt move and the current frame is done.
	if (!_dirty && !_balanceCount)
		return  false;

	unsigned int i = 0;
	// Calc transform of unit vector once per frame and scale the rest.
	ddgTBinTree::initWtoC(this);

	// Reset the visibility counter.
	_visTri = 0;
	_vcache.reset();
	// Calculate visibility info for all triangle currently in the mesh
	// at the current camera position.
	static bool lastMerge = false;
	if (!lastMerge || !merge())
	{
		clearSQ();
		clearMQ();
	}

	while (i < _bintreeMax)
	{
		if (_bintree[i])
		{
			_bintree[i]->resetVisTri();
			_bintree[i]->tri(triNo())->reset();
			_bintree[i]->tri(triNo()+1)->reset();
			_bintree[i]->tri(0)->reset();
			if (!lastMerge || !merge())
			{
				// Top level triangles.
				_bintree[i]->tri(0)->_state.all = 0;
				_bintree[i]->tri(1)->_state.all = 0;
				_bintree[i]->tri(triNo())->_state.all = 0;
				_bintree[i]->tri(triNo()+1)->_state.all = 0;
				_bintree[i]->tri(0)->reset();
				_bintree[i]->tri(1)->reset();
				_bintree[i]->tri(triNo())->reset();
				_bintree[i]->tri(triNo()+1)->reset();
				_bintree[i]->insertSQ(1);
			}
			_bintree[i]->reset(1);
			_bintree[i]->visibility(1);

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
			_bintree[i]->priorityUpdate(1);
		}
		i++;
	}

	// Get the optimal number of triangles in the queue.
	_balanceCount = balanceQueue();
	lastMerge = merge();
    // Track the number of triangles produced be this calculation.
	_triCount += _qs->size();

	_dirty = false;
	return _balanceCount ? true : false;
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

// Returns false if we are blocked from splitting any further.
bool ddgTBinMesh::splitQueue(void)
{
	bool split = false;
	// Remove the biggest item, split it.
	_qsi->reset(true);
	while (!_qsi->end())
	{
		ddgTBinTree *bt = treeSQ(_qsi);
		ddgTriIndex i = indexSQ(_qsi);
		if ( i < _triNo/2  // Not a leaf!
			&& !bt->tri(i)->_vis.flags.allout)
		{
			bt->forceSplit(i);
			split = true;
			break;
		}
		_qsi->prev();
	} 
	return split;
}

// Returns false if we are blocked from merging any further.
bool ddgTBinMesh::mergeQueue(void)
{
	bool merged = false;
	// Remove the smallest item, merge it.
	_qmi->reset();
	while (!_qmi->end())
	{
		ddgTBinTree *bt = treeMQ(_qmi);
		ddgTriIndex i = indexMQ(_qmi);
		if ( i > 0 )	 // Not a root!
		{
			bt->forceMerge(i);
			merged = true;
			break;
		}
		_qmi->next();
	}
	return merged;
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
 * Return the number of balance operations done.
 */
unsigned int ddgTBinMesh::balanceQueue(void)
{
	// Loops should be gone now that we have the flags to indicate if a triangle has been split/merged
	// in this cycle.
    unsigned int maxLoop = merge()?_mindetail/4 : 0xFFFF; // Don't make more than maxLoop adjustments.
	unsigned int priDiff = _mindetail/30;		// Allowable difference in priorities between queues.
	unsigned int minDiff = _mindetail+_mindetail/100; // Min # triangles to start merging.
    unsigned int count = 0;
	bool merged = true, split = true;
    // General case.
	while (merged || split)			// We are still making progress.
	{
		updateVisibleTri();
		merged = split = false;
        // See if we need to merge
		if ( merge() && _qm->size())
		{
			// Merge the diamond of lowest priority.
			if ( _visTri > _maxdetail)  // We have too many triangles.
			{
				merged = mergeQueue();
			}
			else	// Check if the queues are balanced.
			{
				_qmi->reset();
				_qsi->reset(true);
				unsigned int pm = priorityMQ(_qmi);
				unsigned int ps = prioritySQ(_qsi);
				if ((pm == 0)			// We have useless merge diamonds, merge these 1st.
										// We safely beyond the minimum # of triangles, and
										// there is significant difference between the
										// priorities of the split and merge queues.
				  ||(_visTri > minDiff &&  (ps > pm + priDiff)) )
				{
					merged = mergeQueue();
				}
			}
		}
		// See if we need to split.
		if (!merged && _visTri < _mindetail )                     // We don't have enough triangles.
		{
			// So  split the triangle of highest priority.
			split = splitQueue();
		}

		if (merged || split)
			count++;

        if ( count > maxLoop )
        {
            break;  // We are taking too long.
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

