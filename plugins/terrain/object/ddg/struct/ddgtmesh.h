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
#ifndef _ddgMesh_Class_
#define _ddgMesh_Class_

#include "math/ddgvec.h"
#include "math/ddgbbox.h"
#include "struct/ddghemap.h"
#include "struct/ddgcache.h"

class ddgTBinTree;
class ddgTBinMesh;
class ddgContext;

// Block size of bintrees within a binmesh.
#define ddgTBinMesh_size 32
// Number of buckets.
#define	ddgPriorityResolution 256

typedef unsigned int ddgTriIndex;
typedef unsigned int ddgTreeIndex;
typedef unsigned short ddgPriority;
typedef int ddgVBIndex;

typedef enum { eINNER = 0, eTOP = 1, eLEFT = 2, eDIAG = 3} Edge;
/**
 * The information in this class is the same for each TBinTree and can
 * be computed once (and stored) and can be shared amoung all TBinTrees.
 */
class WEXP ddgMSTri
{
public:
	/// Our position on the height grid w.r.t. the offsets of a given bintree.
	int row, col;
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
	inline Edge edge(void)
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
 * Triangle cache.
 * the triangles which will be rendered a kept in this cache.
 * this cache must be able to hold the max number of vis triangles
 * this may be more than the final number of triangles on screen.
 * Max number of entries this cache can hold is 0xFFFF.
 */
class WEXP ddgTNode : public ddgLNode {
	friend class ddgTCache;
private:
 	/// Index to Triangle in the bin tree.				(2 byte)
	ddgTriIndex	_tindex;
	/// Priority of triangle for this frame.			(2 byte)
	ddgPriority _priority;
	/// Index into the split queue cache if applicable.	(2 byte)
	ddgCacheIndex	_qscacheIndex;
	/// Index into the merge queue cache if applicable. (2 byte)
	ddgCacheIndex	_qmcacheIndex;
	/// Multiplier for priority recomputation. (4 byte)
	float		_priorityFactor;
/*
	/// Morphing amount to morph in next step.
	int			_delta;
	///	Current morph height.
	int			_current;
*/
    /**
	 * The state which indicates whether we are visible w.r.t.
	 * the viewing frustrum.							(1 byte)
	 * Only 2 bits are currently used.
	 */
	ddgVisState _vis;
public:
	/// Set the queue cache index.
	void qscacheIndex( ddgCacheIndex ci ) { _qscacheIndex = ci; }
	/// Get the queue cache index.
	ddgCacheIndex qscacheIndex( void ) { return _qscacheIndex; }
	/// Set the queue cache index.
	void qmcacheIndex( ddgCacheIndex ci ) { _qmcacheIndex = ci; }
	/// Get the queue cache index.
	ddgCacheIndex qmcacheIndex( void ) { return _qmcacheIndex; }
	/// Get the triangle index.
	inline ddgTriIndex	tindex(void) { return _tindex; }
	/// Set the triangle index.
	inline void tindex(ddgTriIndex ti) { _tindex = ti; }
	///
	inline void priority(int p ) { _priority = p; }
	///
	inline int priority(void) { return _priority; }

	///
	inline void vis(float v ) { _vis = ddgVisState(v); }
	///
	inline ddgVisState vis(void) { return _vis; }
	///
	inline void priorityFactor(float pf) { _priorityFactor = pf; }
	///
	inline float priorityFactor(void) { return _priorityFactor; }
/*
	///
	inline int current(void) { return _current; }
	///
	inline int delta(void) { return _delta; }
	///
	///
	inline void current(int c) { _current = c; }
	///
	inline void delta(int d) { _delta = d; }
	///

	inline void reset(short h, short v1, short v2)
	{
		// Set current value to midpoint.
		_current = (v1 + v2)/2;
		// Delta is 1/8th of total change.
		_delta = (h - _current)/8;
	}
*/
};

/**
 *  This class caches all the active nodes in the system.
 *  nodes are created for triangle which are either directly visible or indirectly active
 *  to ensure the continuity of the mesh.
 *  Entry 0 is the default entry and will return the values for all elements which
 *  are not actually in the cache.
 *  Entry 1 is a scratch entry which elements who are not actually active can use to
 *  write a value to.  This is to avoid writing complicated functions which check all the time.
 */
class WEXP ddgTCache : public ddgLCache {
	typedef	ddgLCache	super;
public:
	/// Initialize the cache.
	inline void init (unsigned int size ) { super::init(size,sizeof(ddgTNode)); }
	/// Get entry.
	inline ddgTNode* get(ddgCacheIndex n)
	{
		return (ddgTNode*) ddgLCache::get(n);
	}
};




/**
 * Queue cache.
 * the triangles which will be rendered a kept in this cache.
 * this cache must be able to hold the max number of vis triangles
 * this may be more than the final number of triangles on screen.
 * Max number of entries this cache can hold is 0xFFFF.
 * The way this cache works is as follows
 */
class WEXP ddgQNode : public ddgSNode {
	friend class ddgQTCache;
private:
 	/// Index to Triangle in the bin tree.  (2 byte)
	ddgTriIndex		_tindex;
 	/// Index to Triangle in the bin tree.  (2 byte)
	ddgTreeIndex	_tree;

public:
	/// Get the triangle index.
	inline ddgTriIndex	tindex(void) { return _tindex; }
	/// Set the triangle index.
	inline void tindex(ddgTriIndex ti) { _tindex = ti; }
	/// Get the tree index.
	inline ddgTreeIndex	tree(void) { return _tree; }
	/// Set the tree index.
	inline void tree(ddgTreeIndex t) { _tree = t; }
};


/**
 *  This class caches a roughly sorted set of triangles.
 */
class WEXP ddgQCache : public ddgSCache {
	typedef ddgSCache super;
public:
	/**
	 *Initialize the cache.
	 * size is the total number of entries the cache should be able to hold.
	 * bn is the number of priority slots.
	 * r indicates if this queue is sorted in reverse.
	 */
	void init (unsigned int size, unsigned int bn, bool r = false ) {
		super::init(size,sizeof(ddgQNode),bn,r);
	}

	/// Get entry.
	inline ddgQNode* get(unsigned short index)
	{
		return (ddgQNode*) ddgCache::get(index);
	}
	/**
	 * Insert a node into the queue.
	 * returns the position in the cache that it was inserted into.
	 */
	inline ddgCacheIndex insert(ddgTreeIndex t, ddgTriIndex ti, short b)
	{
		ddgCacheIndex ci = super::insert(b);
		// Initialize the new node.
		ddgQNode *qn = get(ci);
		qn->tree(t);
		qn->tindex(ti);
		return ci;
	}
	/**
	 * Move a node from one position to another in the queue.
	 */
	inline void move(ddgCacheIndex ci, short b)
	{
		super::remove(ci);
		super::insert(b);
	}
};
/**
 * Triangle Bintree mesh shared data.<br>
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
 * Neighbours:<br>
 * If the triangles 2 and 3 are split again, their vc vertices will lie
 * halfway between vertices 0 and 1.  They will produce 4 children which
 * all share this common vc, however since the triangles (2 and 3) are
 * seperate triangles, the vc point is stored twice, once for 2 and once for 3.
 * To keep the mesh from cracking, if 2 is split, 3 must split also. No
 * T-junctions are allowed.  Because of this rule, 2 and 3 are considered
 * neighbours and they form a split diamond.
 * There is no easy way to know which triangle is the neighbour of another,
 * for this reason neigbour indices are precomputed and stored in a table.
 * All other indices are readily available from the given triangle's own index.
 *<p><h4>
 */


class WEXP ddgTBinMesh
{
	/// Levels in the mesh (log(size)/log(2));
	unsigned int _maxLevel;
	/// The number of triangles in the TBinTree.
	unsigned int _triNo;
	/// Index beyond which there are leaf triangles in the TBinTree.
	unsigned int _leafTriNo;
	/// Max number of bintrees to manage.
	unsigned int _bintreeMax;
	/// Bintree's managed by this mesh
	ddgTBinTree ** _bintree;
	/// A visible triangle cache.
	ddgTCache		_tcache;
	/// Merge queue active.
	bool			_merge;
	/// Are we doing a split only run.
	bool			_splitRun;
	/// A Split Priority Queue.
	ddgQCache		_qscache;
	/// A Merge Priority Queue.
	ddgQCache		_qmcache;
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
	/// Farclip distance squared.
	float		_farClipSQ;
	/// Near clip range triangles beyond this point are not needed.
	float		_nearClip;
	/// Height map.
	ddgHeightMap	*_heightMap;
	/// Scale of the height map.
	float		_scale;
	/// Base offset of the height map.
	float		_base;
	/// Absolute minimum height in the terrain.
	int		_absMinHeight;
	/// Absolute maximum height in the terrain.
	int		_absMaxHeight;
	/// Difference between maximum and minimum height in the terrain.
	int		_absMaxError;
	/// Total number of triangles in this frame.
	unsigned int _triVis;
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
	/// Number of balance operations.
	unsigned int _balCount;
	/**
	 * Coordinate lookup table.
	 * This 2D array maps x,y coords to a triangle index into
	 * the bintree.
	 */
	ddgTriIndex	*_indexLUT;
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
	bool init( ddgContext *ctx );
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
	void initVertex( int level,
			   ddgTriIndex va,
			   ddgTriIndex v1,
			   ddgTriIndex v0,
			   ddgTriIndex vc);
	/// Increment the number of visible triangles.
	inline void incrTriVis(void) { _triVis++; }
	/// Decrement the number of visible triangles.
	inline void decrTriVis(void) { _triVis--; }
	/// Return the number of visible triangles.
	unsigned int countTriVis(void) { return _triVis; }
    /// Return the edge status of a tri.
    inline unsigned int edge(ddgTriIndex i)
    {
        return stri[i].edge();
    }
	/// Return the index lookup table.
	ddgTriIndex	*indexLUT(void) { return _indexLUT; }
		/// Get triangle vertex 0.
	inline ddgTriIndex v0(ddgTriIndex i)
	{ ddgAssert(i < triNo()); return stri[i].v0; }
	/// Get triangle vertex 1.
	inline ddgTriIndex v1(ddgTriIndex i)
	{ ddgAssert(i < triNo()); return stri[i].v1; }

	/// Return the cache of active triangles.
	ddgTCache *tcache(void) { return &_tcache; }
	/// Return merge queue state.
	bool merge(void) { return _merge; }
	/// Set merge queue state.
	void merge(bool m) { _merge = m; }
	/// Are we doing a split only run.
	bool splitRun(void) { return _splitRun; }
	/// Return the cache of priority queue.
	ddgQCache *qscache(void) { return &_qscache; }
	/// Return the cache of priority queue.
	ddgQCache *qmcache(void) { return &_qmcache; }
	/// Look up a triangle index based on a row and column.
	inline ddgTriIndex lookupIndex( unsigned short row, unsigned short col )
	{
		ddgAsserts(row >= 0 && row <= ddgTBinMesh_size
			    &&col >= 0 && col <= ddgTBinMesh_size,"Invalid coord");
		ddgAssert(_indexLUT[row * (ddgTBinMesh_size+1) + col] < _triNo+2);
		return _indexLUT[row * (ddgTBinMesh_size+1) + col];
	}
	/// Return a pointer to the height map object.
	ddgHeightMap *heightMap(void) { return _heightMap; }
	/// Return the raw height converted to world space.
	inline float wheight(short rh)
	{
		return ddgHeightMap::sconvert(rh,_base,_scale);
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
	/// Return the number of triangles.
	unsigned int triNo(void) { return _triNo; }
	/// Return the number of triangles.
	unsigned int leafTriNo(void) { return _leafTriNo; }
	/// Return the number of split levels.
	unsigned int maxLevel( void ) { return _maxLevel; }

	/**
	 * Calculate the optimal set of triangles for the mesh at current camera pos.
	 * returns true if any calculation was performed.
	 */
	bool calculate(ddgContext *ctx);

	/// Function to add a bintree.
	void addBinTree( ddgTBinTree *bt );
	/// Function to remove a bintree.
	void removeBinTree( ddgTBinTree *bt );
	/// Return the bintree for a given index.
	ddgTBinTree *getBinTree( ddgTreeIndex i)
	{ ddgAssert(i < _bintreeMax && i >= 0); return _bintree[i]; }
	/// Return the total number of bintrees managed by this mesh.
	unsigned int getBinTreeNo(void) { return _bintreeMax; }
	/// Distant clip range triangles beyond this point are not needed.
	float		farClip(void) { return _farClip; }
	/// Squared Distant clip range triangles beyond this point are not needed.
	float		farClipSQ(void) { return _farClipSQ; }
	/// Near clip range triangles beyond this point are not needed.
	float		nearClip(void) { return _nearClip; }
	/// Distant clip range triangles beyond this point are not needed.
	inline void farClip(float f) { _farClip = f; _farClipSQ = f * f; }
	/// Near clip range triangles beyond this point are not needed.
	inline void nearClip(float n) { _nearClip = n; }
	/**
     * Input: x,z in mesh coords (note these values are modified).
     * Output: Bintree which owns this coord.
     *         Row/Col of triangle which owns this coord in the bintree.
     *         x,z offsets from the triangle origin to the coord.
     *         Note for mirrored trees the x,z offsets are relative to the
     *         mirrored tree.
	 * Return: ddgFailure if input location does not map to a location on the mesh.
     */
	bool mapPosToTree(float *x, float *z, ddgTBinTree **bt, unsigned int *r, unsigned int *c);
    /** Get height of mesh at a real location.
     * Coords should be unscaled in x,z direction.
	 * Cannot be called before init.
     */
    float height(float x, float z);
    /** Set height of mesh at a real location.
     * Coords should be unscaled in x,z direction
	 * coord should map to actual heightmap locations.
	 * Cannot be called before init.
     */
    void setHeight(float x, float y, float z);
	/// The difference between the highest point on the mesh and the lowest point.
	int absMaxError(void) { return _absMaxError; }
	/// The lowest point on the mesh.
	int absMinHeight(void) { return _absMinHeight; }
	/// The highest point on the mesh.
	int absMaxHeight(void) { return _absMaxHeight; }
	/// Total number of triangles rendered.
    void triCountIncr(void) { _triCount++; }
#ifdef _DEBUG
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
#endif
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
	/// Total number visibility calculations.
	unsigned int visCount(void) { return _visCount; }
	/// Total number balance calculations for this frame.
	unsigned int balCount(void) { return _balCount; }

};


#endif
