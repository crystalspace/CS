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
#include "struct/ddgtmesh.h"
#include "struct/ddgbtree.h"
// Size should be (# of tiles + #of triangles/frame ) *3 + 10 %

// ----------------------------------------------------------------------
//const unsigned int TBinMesh_size = 64;	// 64 seems to be a good size.
ddgTBinMesh::ddgTBinMesh( ddgHeightMap * h )
{
	_heightMap = h;	// Temporary 
	_scale = h->scale();
	_base = h->base();
	_bintreeMax = 2*(h->cols()-1)/(ddgTBinMesh_size) * (h->rows()-1)/(ddgTBinMesh_size);
	typedef ddgTBinTree *ddgTBinTreeP;
	_bintree = new ddgTBinTreeP[_bintreeMax];
	ddgAsserts(_bintree,"Failed to Allocate memory");
	ddgMemorySet(ddgTBinTreeP,_bintreeMax);
	_minDetail = 3000;
	_maxDetail = 3200;
	_absMaxDetail = 5000;
	_absMaxHeight = -0xFFFF;
	_absMinHeight = 0xFFFF;
	_absDiffHeight = 0;
	// Note Z axis is negative into the screen (Right Handed coord sys).
	farClip(150);
	_merge = true;
	_nearClip = 1;
	// Allocate memory for active triangles.
	_tcache.init(((_bintreeMax + _absMaxDetail) * 11)/10);  // (# of tiles + #of triangles/frame ) + 10 %
	// Create the default entry and the scratch entry.
	ddgTNode *tn = _tcache.get(0);
	// Initialize the default entry.
	tn->tindex(0);
	tn->priority(0);
	tn->vis(0);
	tn->vbufferIndex(0xFFFF);
	// Allocate scratch entry.
	_tcache.allocNode();

	_triVis = 0;

	_maxLevel = (unsigned int) (log(ddgTBinMesh_size)/log(2.0) * 2.01); // Could be constant.
	_triNo = (unsigned int)pow(2,(_maxLevel)); // Inside points can be shared.
	_leafTriNo = _triNo / 2;
	stri = new ddgMSTri[_triNo+2];
	ddgAsserts(stri,"Failed to Allocate memory");
	ddgMemorySet(ddgMSTri,_triNo+2);
	_indexLUT = new ddgTriIndex[(ddgTBinMesh_size+1)*(ddgTBinMesh_size+1)];
	ddgAsserts(_indexLUT,"Failed to Allocate memory");
	ddgMemorySet(ddgTriIndex,(ddgTBinMesh_size+1)*(ddgTBinMesh_size+1));
	// Queues to hold the visible, clipped and sorted visible triangles.
	// Estimate how many entries we can possibly have in the split and merge queues.
	_qscache.init(_absMaxDetail, ddgPriorityResolution);
	_qmcache.init(_absMaxDetail, ddgPriorityResolution,true);

	unsigned int i;
	for (i = 0; i < _bintreeMax; i++ )
		_bintree[i] = NULL;

	i = 0;
	while (i < (ddgTBinMesh_size+1)*(ddgTBinMesh_size+1))
	{
		_indexLUT[i] = 9999;
		i++;
	}
	// Top level triangles.
	stri[0].row = 0;
	stri[0].col = 0;
	// Corner entries.
	stri[_triNo].row = ddgTBinMesh_size;
	stri[_triNo].col = 0;
	stri[_triNo+1].row = 0;
	stri[_triNo+1].col = ddgTBinMesh_size;

	_indexLUT[0] = 0;
	_indexLUT[ddgTBinMesh_size] = _triNo+1;
	_indexLUT[(ddgTBinMesh_size+1)*(ddgTBinMesh_size+1)-ddgTBinMesh_size-1] = _triNo;
	_indexLUT[(ddgTBinMesh_size+1)*(ddgTBinMesh_size+1)-1] = 0;

	// Initialize the other vertices.
    initVertex(1,0,_triNo,_triNo+1,1);
	initNeighbours();

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
	_visCount = 0;

}

//unsigned short *_normalidx = 0;
ddgTBinMesh::~ddgTBinMesh(void)
{
	delete [] stri;
	delete _bintree;
	delete[] _indexLUT;
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
bool ddgTBinMesh::init( ddgContext *ctx )
{

    ddgTBinTree	*bintree1 = NULL,
				*bintree2 = NULL;
	// Created the enough bintrees to cover the terrain.
	unsigned int i, j;
	_nc = (_heightMap->cols()-1)/ddgTBinMesh_size;
	_nr = (_heightMap->rows()-1)/ddgTBinMesh_size;
	ddgConsole::s("Creating "); ddgConsole::i(_nc);ddgConsole::s(" x ");ddgConsole::i(_nr);ddgConsole::s(" Tiles");
	ddgConsole::end();

	ddgTreeIndex index = 0;
	for (i = 0; i < _nr; i++)
	{
		for (j = 0; j < _nc; j++)
		{
			bintree1 = new ddgTBinTree(this,index,i*ddgTBinMesh_size,j*ddgTBinMesh_size);
			ddgMemorySet(ddgTBinTree,1);
			addBinTree(bintree1);
			index++;
			bintree2 = new ddgTBinTree(this,index,(i+1)*ddgTBinMesh_size,(j+1)*ddgTBinMesh_size,true);
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
			_bintree[2*(i*_nc+j)]->pNeighbourLeft( (j==0)? 0: _bintree[2*(i*_nc+j-1)+1]);
			_bintree[2*(i*_nc+j)]->pNeighbourTop( (i==0)? 0: _bintree[2*((i-1)*_nc+j)+1]);
			//
			_bintree[2*(i*_nc+j)+1]->pNeighbourDiag( _bintree[2*(i*_nc+j)]);
			_bintree[2*(i*_nc+j)+1]->pNeighbourLeft( (j+1==_nc)? 0: _bintree[2*(i*_nc+j+1)]);
			_bintree[2*(i*_nc+j)+1]->pNeighbourTop( (i+1==_nr)? 0: _bintree[2*((i+1)*_nc+j)]);
		}
	}

	// We call this function twice.
	ddgTBinTree::initContext(ctx,this);
	for (i = 0; i < _bintreeMax; i++)
	{
		if (_bintree[i])
		{
			_bintree[i]->init();
			if ( _bintree[i]->rawMaxVal(0) > _absMaxHeight)
				_absMaxHeight = _bintree[i]->rawMaxVal(0);
			if ( _bintree[i]->rawMinVal(0) < _absMinHeight)
				_absMinHeight = _bintree[i]->rawMinVal(0);
		}
		ddgConsole::progress( "Initializing blocks", i,_bintreeMax-1);
	}
	// The difference between the highest point and the lowest point.
	_absDiffHeight = _absMaxHeight - _absMinHeight;
	// Call it again to update the DiffHeight etc info.
	ddgTBinTree::initContext(ctx,this);

	// Let go of the height map we do not need it anymore.
    _heightMap = 0;
	ddgConsole::end() ;

    return ddgSuccess;
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

	unsigned int r = stri[vc].row, c = stri[vc].col;
	unsigned int rp = ddgTBinMesh_size - c, cp = ddgTBinMesh_size - r;

	_indexLUT[r*(ddgTBinMesh_size+1)+c] = vc;
	// Fill in the mirror side.
 	if (r + c < ddgTBinMesh_size)
		_indexLUT[rp*(ddgTBinMesh_size+1)+cp] = vc;

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
		ddgConsole::progress("Computing neighbours",k,_triNo);
	}
	ddgConsole::progress("Computing neighbours",1,1);
	ddgConsole::end();
}

bool ddgTBinMesh::calculate( ddgContext *ctx )
{
	static bool lastDirty = true;
	// Dont do anything if we didnt move and the current frame is done.
	if (!_dirty && !lastDirty)
		return false;

	lastDirty = false;
	if (_dirty)
		lastDirty = true;

	// Initialize world to camera space matrix and viewing frustrum. 
	ddgTBinTree::initContext(ctx,this);
	// Calculate visibility info for all triangles currently in the mesh
	// at the current camera position.
	// Clear queue.
	unsigned int i = 0;
	static bool mergeOn = false;

	// Reset state in case of split only or if 1st run
	// with merge enabled.
	if (!mergeOn || !merge())
	{
		// If the merge queue was just enabled, reset it.
		if (merge())
		{
			qmcache()->reset();
		}
		if (!merge()||!mergeOn)
		{
			qscache()->reset();
			tcache()->reset();
			_triVis = 0;
		}
		mergeOn = merge();
		i = 0;
		while (i < _bintreeMax)
		{
			if (_bintree[i])
			{
				// Empty the active triangles chain for the bintree.
				_bintree[i]->freeChain();
				// Reset Top level triangles.
				_bintree[i]->tcacheId(0, 0);
				_bintree[i]->tcacheId(1, 0);
				_bintree[i]->tcacheId(triNo(), 0);
				_bintree[i]->tcacheId(triNo()+1, 0);
			}
			i++;
		}
		i = 0;
		while (i < _bintreeMax)
		{
			if (_bintree[i])
			{
				_bintree[i]->insertSQ(1,ddgMAXPRI,0,ddgUNDEF);
			}
			i++;
		}
	}
	// Update the current state.
	else 
	{
		i = 0;

		unsigned int r, c;
		// Include a 2 block border.
		float d = _farClip+2*ddgTBinMesh_size;
		// Only update those triangles which could possibly be in visible range.
		// See if the triangle is too far from the camera.  Ingore height component.
		ddgVector2 pos( ctx->control()->position()->v[0], ctx->control()->position()->v[2]);
		ddgVector2 min = pos - ddgVector2(d,d);
		ddgVector2 max = pos + ddgVector2(d,d);
		max /= ddgTBinMesh_size;
		min /= ddgTBinMesh_size;
		if (min[0] < 0) min[0] = 0;
		if (min[1] < 0) min[1] = 0;
		if (max[0] > _nc) max[0] = _nc;
		if (max[1] > _nr) max[1] = _nr;
		unsigned int rmin = (unsigned int)min[1];
		unsigned int rmax = (unsigned int)max[1];
		unsigned int cmin = (unsigned int)min[0];
		unsigned int cmax = (unsigned int)max[0];

		// Update the potentially visible square.
		for (r = rmin; r < rmax; r++)
			for (c = cmin; c < cmax; c++)
			{
				i = 2*(_nc * r + c);
				// Recursively update the state of the mesh
				if ( _bintree[i])
				{
					_bintree[i]->updateSplit(1,ddgUNDEF);
				}
				if ( _bintree[i+1])
				{
					_bintree[i+1]->updateSplit(1,ddgUNDEF);
				}
		}

		// Validate the merge queue.
		ddgCacheIndex ci = qmcache()->head();
		while (ci)
		{
			// $BUG cannot iterate over this since it modifies the order of items in the queue
			ddgQNode *qn = qmcache()->get(ci);
			ddgTBinTree *mbt = getBinTree(qn->tree());
			ci = qmcache()->next(ci);
			mbt->updateMerge(qn->tindex(),ddgMAXPRI);
		}

	}

	// Get the optimal number of visible triangles by balancing the queue.
	unsigned int priDiff = _minDetail/30;		// Allowable difference in priorities between queues.
	unsigned int minDiff = _minDetail-_minDetail/100; // Min # triangles to start merging.

	bool done = false;
	unsigned int count = 0;

	while (!done)
	{
		count++;
		if (count > _minDetail)
		{
			ddgAsserts(0,"Too many iterations to stabilization");
			done = true;
			break;
		}
		ddgCacheIndex csi = qscache()->head();
		ddgQNode *qn = qscache()->get(csi);
		ddgTBinTree *sbt = getBinTree(qn->tree());
		ddgTriIndex si = qn->tindex();
		ddgPriority ps = sbt->priority(si);
		if (merge())
		{
			ddgCacheIndex cmi = qmcache()->head();
			if (cmi)
			{
				qn = qmcache()->get(cmi);
				ddgTBinTree *mbt = getBinTree(qn->tree());
				ddgTriIndex mi = qn->tindex();
				ddgPriority pm = qmcache()->convert(qn->bucket());
				ddgAssert(ps == 0 || ps == qscache()->convert(qscache()->get(csi)->bucket()));
				if (( _triVis > _maxDetail) // We have too many triangles.
					|| (pm == 0)			// We have useless merge diamonds, merge these 1st.
										// We are safely beyond the minimum # of triangles, and
										// there is significant difference between the
										// priorities of the split and merge queues.
				  ||(_triVis > minDiff && (ps > pm + priDiff)) )
				{
					ddgAsserts(mi > 0,"Its a root node");
					mbt->forceMerge(mi);
					continue;
				}
			}
		}
		if (csi && _triVis < _minDetail )
		{
			ddgAsserts( sbt->visible(si), "Non visible triangle in the split queue!")
			sbt->forceSplit(si);
			continue;
		}
		// Nothing we can do, we probably ran out of triangles.
		done = true;

	}
	_balCount = count;
    // Track the number of triangles produced by this calculation.
	_triCount += _triVis;
	_dirty = false;
	return  true;
}

bool ddgTBinMesh::mapPosToTree(float *x, float *z, ddgTBinTree **bt, unsigned int *r, unsigned int *c)
{
	// Check that we are within limit.
	if (*x < 0 || *x > _nr * ddgTBinMesh_size
     || *z < 0 || *z > _nc * ddgTBinMesh_size)
	{
		*bt = NULL;
		return ddgFailure;
	}
    // Find the global(mesh) column and row of this coordinate.
	unsigned int gr = (unsigned int)(*x/ddgTBinMesh_size);
	unsigned int gc = (unsigned int)(*z/ddgTBinMesh_size);
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
	if (*bt)
		return ddgSuccess;
	else
		return ddgFailure;
}

float ddgTBinMesh::height(float x, float z)
{
    // Find the binTree which owns this coordinate.
    ddgTBinTree *bt = 0;
    unsigned int r, c;
    if (mapPosToTree(&x,&z,&bt, &r, &c) == ddgSuccess)
	{
		ddgAssert(bt);
		// Find mesh height at this location.
		return bt->treeHeight(r,c,x,z);
	}
	else
		return 0.0;
}



