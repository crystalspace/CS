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

#include "csterr/ddghemap.h"
#include "csterr/ddgbbox.h"
#include "csterr/ddgsplay.h"

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
	 * Index of the merge diamond which is our brother.
	 * If we are on an edge, this may be in another TBinTree, and the
	 * neighbour field will indicate which one.
	 */
	unsigned int brother;
	/**
	 * If this is not zero, the brother is in another TBinTree.
	 * the flag indicates which neighbour the edge is in.
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
 * Brothers and Merge Diamonds:<br>
 * If the triangles 2 and 3 are split again, their vc vertices will lie
 * halfway between vertices 0 and 1.  They will produce 4 children which
 * all share this common vc, however since the triangles (2 and 3) are
 * seperate triangles, the vc point is stored twice, once for 2 and once for 3.
 * To keep the mesh from cracking, if 2 is split, 3 must split also. No
 * T-junctions are allowed.  Because of this rule, 2 and 3 are considered
 * brothers and they form a merge diamond.
 * There is no easy way to know which triangle is the brother of another,
 * for this reason brother indices are precomputed and stored in a table.
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
	/// Dirty state.
	bool		_dirty;
	/// Max number of bintrees to manage.
	unsigned int _bintreeMax;
	/// Bintree's managed by this mesh
	ddgTBinTree ** _bintree;
	/// Number of visible triangles in the mesh.
	unsigned int _visTri;
	/// Split queue.
	ddgSplayTree	*_qs;
	/// Merge queue.
	ddgSplayTree	*_qm;
	/// Z depth queue.
	ddgSplayTree	*_qz;
	/// Split queue iterator
	ddgSplayIterator		*_qsi;
	/// Merge queue
	ddgSplayIterator		*_qmi;
	/// Z depth queue iterator.
	ddgSplayIterator		*_qzi;
	/// Number of rows of bintrees.
	unsigned int	_nr;
	/// Number of columns of bintrees.
	unsigned int	_nc;
    /// World to camera space transformation matrix must be initialized before calling calculate.
    double          *_wtoc;
    /// The bounding box in screen space.
    ddgBBox         *_camBBox;
    /// The field of view.
    float           _tanHalfFOV;
	/// The number of triangles that should be displayed.
	unsigned int	_detail;
	/// Distant clip range triangles beyond this point are not needed.
	float		_farclip;
	/// Near clip range triangles beyond this point are not needed.
	float		_nearclip;
	/// Triangles beyond this point have their priority recalculated once every n frames.
	float		_progDist;
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
	bool init( double *worldToCameraMatrix, ddgBBox *camClipBox, float fov );
	/**
	 * Initialize the brother field of all triangles.
	 */
	void initBrothers( void );
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
	unsigned int detail( void ) { return _detail; }
	/// Set the required LOD.
	void detail( unsigned int d ) { _detail = d; }

	/// Return the split queue 
	ddgSplayTree *qs(void) { return _qs; }
	/// Return the merge queue 
	ddgSplayTree *qm(void) { return _qm; }
	/// Return the merge queue 
	ddgSplayTree *qz(void) { return _qz; }
	/// Return an iterator for the visible set of triangles.
	ddgSplayIterator* qsi(void) { return _qsi; }
	/// Return an iterator for the visible set of triangles.
	ddgSplayIterator* qzi(void) { return _qzi; }
	/// Return a priority from the merge queue.
	unsigned int prioritySQ( ddgSplayIterator *i );
	/// Return the tree of an item from the split queue.
	ddgTBinTree* treeSQ( ddgSplayIterator *i )
	{
		ddgSplayKey *sk = _qs->retrieve(i->current());
		return getBinTree(sk->tree());
	}
	/// Return an item from the split queue.
	unsigned int indexSQ(ddgSplayIterator *i)
	{
		ddgSplayKey *sk = _qs->retrieve(i->current());
		return sk->index();
	}
	/// Return a priority from the merge queue.
	unsigned int priorityMQ( ddgSplayIterator *i );
	/// Return the tree of an item from the merge queue.
	ddgTBinTree* treeMQ( ddgSplayIterator *i )
	{
		ddgSplayKey *sk = _qm->retrieve(i->current());
		return getBinTree(sk->tree());
	}
	/// Return an item from the split queue.
	unsigned int indexMQ(ddgSplayIterator *i)
	{
		ddgSplayKey *sk = _qm->retrieve(i->current());
		return sk->index();
	}
	/// Return the tree of an item from the merge queue.
	ddgTBinTree* treeZQ( ddgSplayIterator *i )
	{
		ddgSplayKey *sk = _qz->retrieve(i->current());
		return getBinTree(sk->tree());
	}
	/// Return an item from the split queue.
	unsigned int indexZQ(ddgSplayIterator *i)
	{
		ddgSplayKey *sk = _qz->retrieve(i->current());
		return sk->index();
	}
	/// Return the number of triangles.
	unsigned int triNo(void) { return _triNo; }
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

	/// Return the number of visible triangles in the mesh.
	unsigned int visTri(void) { return _visTri; }

	/// Recalculate the number of visibile triangles.
	unsigned int updateVisibleTri(void);

	/// Return the number of split levels.
	unsigned int maxLevel( void ) { return _maxLevel; }

	/// Set dirty state.
	void dirty( bool d ) { _dirty = d; }
	/// Get dirty state.
	bool dirty( void ) { return _dirty; }

	/// Calculate the optimal set of triangles for the mesh at current camera pos.
	void calculate(void);

	/// Function to add a bintree.
	void addBinTree( ddgTBinTree *bt );
	/// Function to remove a bintree.
	void removeBinTree( ddgTBinTree *bt );
	/// Return the bintree for a given index.
	ddgTBinTree *getBinTree( ddgTreeIndex i)
	{ assert(i < _bintreeMax); return _bintree[i]; }
	/// Distant clip range triangles beyond this point are not needed.
	float		farclip(void) { return _farclip; }
	/// Near clip range triangles beyond this point are not needed.
	float		nearclip(void) { return _nearclip; }
	/// Distant clip range triangles beyond this point are not needed.
	inline void farclip(float f) { _farclip = f; }
	/// Near clip range triangles beyond this point are not needed.
	inline void nearclip(float n) { _nearclip = n; }
	/// Return the row/col index and tree at a given location.
	float progDist(void) { return _progDist; }
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
	/** Generate a normal map from this image.
	 *  height map should be a single component image.
	 *  this map will be a three component image of
	 *  the same dimension as heights.
	 *  aspect is the ratio of vertical units vs horizontal units.
	 *  Default every horizontal unit = 10m.
	 *                vertical   unit = 0.1m.
	 */
	void generateNormals(void);
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

#ifdef _DEBUG
	/// Debug method to check that tree is still sane.
	void verify(void);
#endif
   /// Return the camera bounding box.
    ddgBBox *camBBox(void) { return _camBBox; }
    /// Return the half field of view.
    float tanHalfFOV(void) { return _tanHalfFOV; }
    /// Transform coordinate from world to camera space.
    void transform( ddgVector3 vin, ddgVector3 *vout );
#ifndef DDG
private:
	/// A transform method.
	void (*_transform)(ddgVector3 vin, ddgVector3 *vout);
public:
	/// Specify a transform method.
	void settransform( void (*t)(ddgVector3 vin, ddgVector3 *vout)) { _transform = t; }
#endif

};


#endif
