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
#ifndef _ddgMesh_Class_
#define _ddgMesh_Class_
#ifdef DDG
#include "ddghemap.h"
#include "ddgvec.h"
#include "ddgcache.h"
#ifdef HASH
#include "ddghash.h"
#else
#include "ddgsplay.h"
#endif
#else
#include "csterr/ddghemap.h"
#include "csterr/ddgbbox.h"
#include "csterr/ddgsplay.h"
#include "csterr/ddgvec.h"
#include "csterr/ddgcache.h"
#endif
class ddgTBinTree;
class ddgTBinMesh;

// Block size of bintrees within a binmesh.
#define ddgTBinMesh_size 64

typedef unsigned int ddgTriIndex;
typedef unsigned int ddgTreeIndex;
typedef enum { eINNER = 0, eTOP = 1, eLEFT = 2, eDIAG = 3} Edge;
/**
 * The information in this class is the same for each TBinTree and can
 * be computed once (and stored) and can be shared amoung all TBinTrees.
 */
class WEXP ddgMSTri
{
public:
	/// Our position on the height grid.
	unsigned int row, col; // Maybe possible to calculate.
	/**
	 *  The indices of the 2 other vertices which are a part of this triangle.
	 *  The 3rd vertex is our own index in the mesh.
	 */
	ddgTriIndex v0, v1;
	/**
	 * Index of the merge diamond which is our neighbour.
	 * If we are on an edge, this may be in another TBinTree, and the
	 * neighbour field will indicate which one.
	 */
	ddgTriIndex neighbour;
	/**
	 * If this is not zero, the neighbour is in another TBinTree.
	 * the flag indicates which neighbour tree the edge is in.
	 * 0 - inner, 1 - top, 2 - left, 3 - diag.
	 */
	Edge edge(void)
	{
		if (row == 0)
			return eTOP;
		if (col == 0)
			return eLEFT;
		if (row + col == ddgTBinMesh_size)
			return eDIAG;
		return eINNER;
	}
};

/**
 *  Vertex cache.
 * all vertices which are transformed to camera space
 * are kept by this cache in case they are needed again for
 * another calculation during this frame.
 * Note entry 0 in the cache is unused.
 */
class WEXP ddgVCache {
	/// Max size of cache.
	unsigned short	_size;
	/// Current size.
	unsigned short _used;
	/// Pointer to array of Vector3s.
	ddgVector3		*_cache;
public:
	/// Create cache.
	ddgVCache();
	/// Destroy cache.
	~ddgVCache();
	/// Initialize the cache.
	void init (unsigned int size );
	/// Get entry.
	inline ddgVector3		*get(unsigned short index)
	{
		return &(_cache[index]);
	}
	/// New entry.
	inline unsigned short	allocate(void)
	{
		ddgAssert(_used + 1 < _size);
		_used++;
		return _used;
	}
	/// Reset cache to empty.
	inline void reset(void)
	{
		_used = 0;
	}
};

/**
 * Triangle cache.
 * the triangles which will be rendered a kept in this cache.
 * this cache must be able to hold the max number of vis triangles
 * this may be more than the final number of triangles on screen.
 * Max number of entries this cache can hold is 0x7FFF.
 * The way this cache works is as follows
 */
class WEXP ddgTNode {
public:
	// Pointer to a bin tree.
	ddgTreeIndex	bt;
	// Index to Triangle in the bin tree.
	ddgTriIndex		tindex;
};
class WEXP ddgTCache : public ddgCache {
public:
	/// Initialize the cache.
	void init (unsigned int size ) { ddgCache::init(size,sizeof(ddgTNode)); }
	/// Get entry.
	inline ddgTNode* get(unsigned short index)
	{
		return (ddgTNode*) ddgCache::get(index);
	}
	/// Get entry.
	inline ddgCacheIndex set( ddgTreeIndex bt, ddgTriIndex tindex)
	{
		ddgCacheIndex ci = allocNode();
		ddgTNode * tn = (ddgTNode*) ddgCache::get(ci);
		tn->tindex = tindex;
		tn->bt = bt;
		return ci;
	}

};
/**
 * ROAM Triangle Bintree mesh shared data.<br>
 *
 * General construction of a BinTree and the indexing of vertices:<br>
 *
 * The top level triangle consists of vertices (0,3,2), triangle
 * vertices are enumerated counter clock-wise.
 * Va is the top of the triangle, Vc is the new split point or center,
 * V0 is the left coord w.r.t. vc and V1 is the right coord w.r.t. vc.
 * This triangle has 2 children (the Bin in BinTree) which are T0 and T1,
 * (0,1,n+1) and (0,n,1), respectively (the left and the right child with
 * respect to vc).<br>
 * Each sub triangle can be split again, 1st the left into (1,2,0) and
 * (1,n+1,2) and the right into (1,0,3) and (1,3,n).  For the left and right
 * sub triangle, 1 becomes va and 2 becomes vc for the left child and
 * 3 becomes vc for the right child, etc.
 * A triangle is identified by its Vc coordinate since that is unique, Va,
 * V0 and V1 are shared with other triangles.  Therefor all information
 * that it stores is associated with Vc.
 * Each sub level in the tree adds twice as many triangles as were present
 * in the previous level.  So with each level required storage double
 * That only leaves a problem for the top most triangle which needs to
 * store its left and right children from somwhere.  In this implementation,
 * this info is stored at index n and n+1.
 * This scheme allows very fast computation of the indices which represent
 * the parent or children of any given triangle.
 *<pre>
 * This diagram shows the index numbers, names for the top level triangle
 * (va,vc,v0,v1) and to map coordinates of the top level vertices.
 *
 *        0,0      3     size,0
 *    (va) 0 +-----+-----+ n (v1)
 *           |\    |    /
 *           | \   T1  /
 *           |  \  |  /
 *           |   \ | /
 *           |    \|/
 *         2 +-T0--+ 1 (vc)
 *           |    /  
 *           |   /
 *           |  /
 *           | /
 *           |/
 *   0,size  + n+1 (v0)
 *</pre>
 * Neighbours and Merge Diamonds:<br>
 * If the triangles 2 and 3 are split again, their vc vertices will lie
 * halfway between vertices 0 and 1.  They will produce 4 children which
 * all share this common vc, however since the triangles (2 and 3) are
 * seperate triangles, the vc point is stored twice, once for 2 and once for 3.
 * To keep the mesh from cracking, if 2 is split, 3 must split also. No
 * T-junctions are allowed.  Because of this rule, 2 and 3 are considered
 * neighbours and they form a merge diamond.
 * There is no easy way to know which triangle is the neighbour of another,
 * for this reason neigbour indices are precomputed and stored in a table.
 * All other indices are readily available from the given triangle's own index.
 *<p><h4>
 * The mesh update process:
 *</h4>
 *<pre>
 * Draw
 *   Calculate
 *       if Camera has moved or terrain has changed.
 *          {
 *          Update CameraMatrix for culling.
 *          Reset Visibility data for all triangles up to the point that they
 *             are in split queue.
 *          recalculate Visiblity of each visible triangle recursively
 *             sets update flag.
 *             calculates vertices in camera coords and caches them for later use.
 *          merge all diamonds which are not visible.
 *          UpdatePriorities
 *             for all visible triangles,
 *                compute their priority, sets uptodatep flag.    
 *             resort the split and merge queue accordingly.
 *          BalanceQueue
 *             split and merge triangles until
 *                the right number of triangles is available and
 *                the maximum split priority is greater than the minimum merge priority
 *          }
 *   Render
 *      for all visible triangles in the Split queue.
 *         build vertex array.
 *         render the vertex array.
 *</pre>
 */


class WEXP ddgTBinMesh
{
	/// Levels in the mesh (log(size)/log(2));
	unsigned int _maxLevel;
	/// The number of triangles in the TBinTree.
	unsigned int _triNo;
	/// Index beyond which there are leaf triangles in the TBinTree.
	unsigned int _leafTriNo;
	/// Dirty state.
	bool		_dirty;
	/// Max number of bintrees to manage.
	unsigned int _bintreeMax;
	/// Bintree's managed by this mesh
	ddgTBinTree ** _bintree;
	/// A transformed vertex cache.
	ddgVCache		_vcache;
	/// A visible triangle cache.
	ddgTCache		_tcache;
#ifdef HASH
	/// Split queue.
	ddgHashTable	*_qs;
	/// Merge queue.
	ddgHashTable	*_qm;
	/// Split queue iterator
	ddgHashIterator		*_qsi;
	/// Merge queue
	ddgHashIterator		*_qmi;
#else
	/// Split queue.
	ddgSplayTree	*_qs;
	/// Merge queue.
	ddgSplayTree	*_qm;
	/// Split queue iterator
	ddgSplayIterator		*_qsi;
	/// Merge queue
	ddgSplayIterator		*_qmi;
#endif
	/// Number of rows of bintrees.
	unsigned int	_nr;
	/// Number of columns of bintrees.
	unsigned int	_nc;
	/// The min number of triangles that should be displayed.
	unsigned int	_minDetail;
	/// The max number of triangles that should be currently displayed.
	unsigned int	_maxDetail;
	/// The max number of triangles that this implementation should handle.
	unsigned int	_absMaxDetail;
	/// Distant clip range triangles beyond this point are not needed.
	float		_farClip;
	/// Near clip range triangles beyond this point are not needed.
	float		_nearClip;
	/// Triangles beyond this point have their priority recalculated once every n frames.
	float		_progDist;
	/// Merge queue active.
	bool		_merge;
	/// The normal lookup table.
	ddgVector3	*_normalLUT;
	/// Total number of triangles rendered.  For statistics.
	unsigned int _triCount;
	/// Total number of priorities calculated.
	unsigned int _priCount;
	/// Total number of queue insertions.
	unsigned int _insCount;
	/// Total number of queue removals.
	unsigned int _remCount;
	/// Total number of queue updates.
	unsigned int _movCount;
	/// Number of visibility calculations.
	unsigned int _visCount;
	/// Number of reset operations.
	unsigned int _resetCount;
	/// Number of balanceOperations this frame.
	unsigned int _balanceCount;
public:
	/**
     * Constructor, pass in the size of the BinTree.
	 *  s = size along one edge of one BinTree.
	 *  i = image object from which to contruct the mesh.
     */
	ddgTBinMesh( ddgHeightMap *i);
	/// Destroy the Bintree mesh.
	~ddgTBinMesh(void);
	/**
	 * Pointer to an array of MTri objects.
	 */
	ddgMSTri* stri;
	/// Initialize the mesh.
	bool init( void );
	/**
	 * Initialize the neighbour field of all triangles.
	 */
	void initNeighbours( void );
	/**
	 *  Initialize the vertex and row/col data of the STri. 
	 *  Passed in are the triangles which carry the points
	 *  that describe the current triangle.
	 *  va is immediate parent.
	 *  v1 and v0 are the sub triangles of va. 
	 */
	void initVertex( unsigned int level,
			   ddgTriIndex va,
			   ddgTriIndex v1,
			   ddgTriIndex v0,
			   ddgTriIndex vc);
    /// Return the edge status of a tri.
    unsigned int edge(ddgTriIndex i)
    {
        return stri[i].edge();
    }

	/// Return the required LOD.
	unsigned int minDetail( void ) { return _minDetail; }
	/// Set the required LOD.
	void minDetail( unsigned int d ) { _minDetail = d; }
	/// Return the required LOD.
	unsigned int maxDetail( void ) { return _maxDetail; }
	/// Set the required LOD.
	void maxDetail( unsigned int d ) { _maxDetail = d; }
	/// Return the absolute maximum number of triangles to support.
	unsigned int absMaxDetail( void ) { return _absMaxDetail; }
	/// Set the absolute maximum number of triangles to support.
	void absMaxDetail( unsigned int d ) { _absMaxDetail = d; }
#ifdef HASH
	/// Return the split queue (For debugging only)
	ddgHashTable *qs(void) { return _qs; }
	/// Return the merge queue (For debugging only)
	ddgHashTable *qm(void) { return _qm; }
	/// Return an iterator for the visible set of triangles.
	ddgHashIterator* qsi(void) { return _qsi; }
	/// Return an iterator for the mergable set of triangles. DEBUG
	ddgHashIterator* qmi(void) { return _qmi; }
#else
	/// Return the split queue (For debugging only)
	ddgSplayTree *qs(void) { return _qs; }
	/// Return the merge queue (For debugging only)
	ddgSplayTree *qm(void) { return _qm; }
	/// Return an iterator for the visible set of triangles.
	ddgSplayIterator* qsi(void) { return _qsi; }
	/// Return an iterator for the mergable set of triangles. DEBUG
	ddgSplayIterator* qmi(void) { return _qmi; }
#endif
	/// Return the triangle cache.
	ddgTCache *tcache(void) { return &_tcache; }
	/// Return the number of triangles.
	unsigned int triNo(void) { return _triNo; }
	/// Return the number of triangles.
	unsigned int leafTriNo(void) { return _leafTriNo; }
	/**
	 *  Split the best split candidate.
	 */
	bool splitQueue(void);
	/**
	 *  Merge the best merge candidate.
	 */
	bool mergeQueue(void);
	/**
	 *  Balance the split and merge queues.
	 * Returns the number of iterations it took to get to a balanced state.
	 */
	unsigned int balanceQueue(void);
	/// Empty the split queue.
	void clearSQ(void);
	/// Empty the merge queue.
	void clearMQ(void);
#ifdef HASH
	/// Return a priority from the split queue.
	unsigned int prioritySQ( ddgHashIterator *i )
	{
		return i->current()->key();
	}
	/// Return a priority from the merge queue.
	unsigned int priorityMQ(  ddgHashIterator *i )
	{
		return i->current()->key();
	}
	/// Return the tree of an item from the split queue.
	ddgTBinTree* treeSQ(  ddgHashIterator *i )
	{
		return getBinTree(i->current()->tree());
	}
	/// Return an item from the split queue.
	unsigned int indexSQ( ddgHashIterator *i)
	{
		return i->current()->index();
	}
	/// Return the tree of an item from the merge queue.
	ddgTBinTree* treeMQ(  ddgHashIterator *i )
	{
		return getBinTree(i->current()->tree());
	}
	/// Return an item from the split queue.
	unsigned int indexMQ( ddgHashIterator *i)
	{
		return i->current()->index();
	}
#else
	/// Return a priority from the split queue.
	unsigned int prioritySQ( ddgSplayIterator *i)
	{
		return _qs->retrieve(i->current())->key();
	}
	/// Return a priority from the merge queue.
	unsigned int priorityMQ( ddgSplayIterator *i )
	{
		return _qm->retrieve(i->current())->key();
	}
	/// Return the tree of an item from the split queue.
	ddgTBinTree* treeSQ( ddgSplayIterator *i)
	{
		return getBinTree(_qs->retrieve(i->current())->tree());
	}
	/// Return an item from the split queue.
	unsigned int indexSQ(ddgSplayIterator *i)
	{
		return _qs->retrieve(i->current())->index();
	}
	/// Return the tree of an item from the merge queue.
	ddgTBinTree* treeMQ( ddgSplayIterator *i)
	{
		return getBinTree(_qm->retrieve(i->current())->tree());
	}
	/// Return an item from the split queue.
	unsigned int indexMQ(ddgSplayIterator *i)
	{
		return _qm->retrieve(i->current())->index();
	}
#endif

	/// Return the number of visible triangles in the mesh.
	unsigned int visTri(void) { return _tcache.size(); }

	/// Return the number of split levels.
	unsigned int maxLevel( void ) { return _maxLevel; }

	/// Set dirty state.
	void dirty( bool d ) { _dirty = d; }
	/// Get dirty state.
	bool dirty( void ) { return _dirty; }

	/**
	 * Calculate the optimal set of triangles for the mesh at current camera pos.
	 * returns true if any calculation was performed.
	 */
	bool calculate(void);

	/// Function to add a bintree.
	void addBinTree( ddgTBinTree *bt );
	/// Function to remove a bintree.
	void removeBinTree( ddgTBinTree *bt );
	/// Return the bintree for a given index.
	ddgTBinTree *getBinTree( ddgTreeIndex i)
	{ ddgAssert(i < _bintreeMax && i >= 0); return _bintree[i]; }
	/// Distant clip range triangles beyond this point are not needed.
	float		farClip(void) { return _farClip; }
	/// Near clip range triangles beyond this point are not needed.
	float		nearClip(void) { return _nearClip; }
	/// Distant clip range triangles beyond this point are not needed.
	inline void farClip(float f) { _farClip = f; }
	/// Near clip range triangles beyond this point are not needed.
	inline void nearClip(float n) { _nearClip = n; }
	/// Return the row/col index and tree at a given location.
	void progDist(float p ) { _progDist = p; }
	/// Return the row/col index and tree at a given location.
	float progDist(void) { return _progDist; }
	/// Return the vertex cache.
	ddgVCache	*vcache(void) { return &_vcache; }
	/**
     * Input: x,z in mesh coords (note these values are modified).
     * Output: Bintree which owns this coord.
     *         Row/Col of triangle which owns this coord in the bintree.
     *         x,z offsets from the triangle origin to the coord.
     *         Note for mirrored trees the x,z offsets are relative to the
     *         mirrored tree.
     */
	void pos(float *x, float *z, ddgTBinTree **bt, unsigned int *r, unsigned int *c);
    /** Get height of mesh at a real location.
     * Coords should be unscaled in x,z direction.
	 * Cannot be called before init.
     */
    float height(float x, float z);
	/// Turn the usage of the merge queue on/off, should only be called inbetween frames.
	void merge( bool m ) { _merge = m; }
	/// Indicate if the merge queue is on/off.
	bool merge( void ) { return _merge; }
	/** 
	 *  Generate a normal map from this image.
	 *  height map should be a single component image.
	 *  The normal is calculated for each height sample, then appropriate
	 *  normal LUT index is stored.
	 *  aspect is the ratio of vertical units vs horizontal units.
	 *  Default every horizontal unit = 10m.
	 *                vertical   unit = 0.1m.
	 */
	bool generateNormals(void);
	/// The normal lookup table.
	ddgVector3	*normalLUT(unsigned int idx) { return &(_normalLUT[idx]); }
	/// Total number of triangles rendered.
    void triCountIncr(void) { _triCount++; }
	/// Total number of priorities calculated.
	void priCountIncr(void) { _priCount++; }
	/// Total number of queue insertions.
	void insCountIncr(void) { _insCount++; }
	/// Total number of queue removals.
	void remCountIncr(void) { _remCount++; }
	/// Total number of queue updates.
	void movCountIncr(void) { _movCount++; }
	/// Total number visibility calculations.
	void visCountIncr(void) { _visCount++; }
	/// Total number reset operations.
	void resetCountIncr(void) { _resetCount++; }
	/// Total number of triangles rendered.
    unsigned int triCount(void) { return _triCount; }
	/// Total number of priorities calculated.
	unsigned int priCount(void) { return _priCount; }
	/// Total number of queue insertions.
	unsigned int insCount(void) { return _insCount; }
	/// Total number of queue removals.
	unsigned int remCount(void) { return _remCount; }
	/// Total number of queue updates.
	unsigned int movCount(void) { return _movCount; }
	/// Total number of queue updates.
	unsigned int balanceCount(void) { return _balanceCount; }
	/// Total number visibility calculations.
	unsigned int visCount(void) { return _visCount; }
	/// Total number reset operations.
	unsigned int resetCount(void) { return _resetCount; }

#ifdef _DEBUG
	/// Debug method to check that tree is still sane.
	void verify(void);
#endif
	/// x and z location on the height map.
	float tx, tz;
};


#endif
