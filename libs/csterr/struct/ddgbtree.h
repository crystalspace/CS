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
#ifndef _ddgBinTree_Class_
#define _ddgBinTree_Class_

#include "struct/ddgtmesh.h"
#include "struct/ddgcntxt.h"
#ifdef DDG
#include "struct/ddghist.h"
#endif

typedef unsigned short ddgPriority;

#define ddgMAXPRI 0xFFFF
#define ddgMINPRI 0

/**
 * Triangle Bintree mesh.
 * The mesh is a hierarchical binary tree of triangles.
 * The structure of this mesh can be precomputed and stored
 * as a large look-up-table.
 * Triangles are organized in the array so they can easily
 * find their parent or children as well as all the triangles
 * of their own generation. This can all be done with index
 * arithmetic (ie. no need for pointers).
 */

class WEXP ddgTBinTree {
	/// The SharedBinTree data.
    static ddgTBinMesh	*_mesh;
    /// Col offset in maps.
    int     _dc;
    /// Row offset in maps.
    int     _dr;
    /// Is coordinate system inverted.
    bool    _mirror:1;
	/// Index of this tree in the mesh.
	unsigned int _index;
	/// Mesh cache index array.										(2 bytes)
	ddgCacheIndex *_cacheIndex;
	/**
	 * Height value of this triangle's coord						(2 bytes)
	 * Converting these fields to floats has a negative speed impact.
	 */
	short *_rawHeight;
    /**
	 * Minimum value of all triangles of this triangle's subtree	(2 bytes)
	 */
	short *_rawMinVal;
    /**
	 * the Max height of all _rawHeights  in the subtree. (2 bytes)
	 */
	short *_rawMaxVal;
    /**
	 * the cumulative potential subtree error. (2 bytes)
	 */
	unsigned short *_treeError;
	/**
	 * Visibility state of the top level triangle in this tree.
	 */
	ddgVisState _treeVis;

	/**
	 * Pointer to neighbouring TBinTrees.
	 *<pre>
	 *          top
	 *        +------+
	 *        |     /|
	 *   left |    / | right
	 *        |   /  |
	 *        |  / M |    M - Triangle is mirrored
	 *        | /    |
	 *        +------+
	 *          bottom
	 *</pre>
	 */
	ddgTBinTree   *_pNeighbourDiag;
	/// For mirrored mesh this is the bottom side.
	ddgTBinTree   *_pNeighbourTop;
	/// For mirrored mesh this is the right side.
	ddgTBinTree   *_pNeighbourLeft;
	/// Various static variables to avoid dereferencing _mesh.
	static ddgContext	  *_ctx;
	/// 2D X/Z projection of the observer's location.
	static ddgVector2	_pos;
	/// 2D X/Z projection of the observer's forward vector.
	static ddgVector2	_forward;
	/// Far clipping distance.
	static float _farClip;
	/// Far clipping distance * 2.
	static float _farClip2;
	/// Near clipping distance.
	static float _nearClip;
	/// Squared far clipping distance.
	static float _farClipSQ;
	/// Cosine of the field of view / 2;
	static float _cosHalfFOV;
	/**
	 * This property is used to scale a wedgie's absolute variance
	 * into the available variance range.
	 * Variances beyond 1/4 the max variance are clamped to the max variance
	 * The frame variance for a wedgie is calculated as follows:
	 * priorityResolution * 4 / ( Tv * farClip) * Wv * Z.
	 * To avoid recalculating this for each wedgie for each frame:
	 * We precompute:
	 * varianceScale = priorityResolution * 4 / ( Tv * farClip) once and
	 * store this value.
	 * Then we need only compute varianceScale * Wv * Z for each frame.
	 * theoretically we could precompute varianceScale * Wv as well but
	 * then we could not dynamically adjust farClip and priorityResolution
	 * and we would probably need a float for each wedgie, instead we store
	 * the Wv and compute varianceScale * Wv only when the wedgie enters
	 * the visible set and store is as Wsv	(Wedgie scale variance)
	 * Then for each frame we need to only compute Wsv * Z.
	 *
	 * Where:
	 * Wv = wedgie variance (WmaxHeight-WminHeight)
	 * Tv = max variance across the entire terrain (TmaxHeight-TminHeight)
	 * Z = screen depth of wedgie centre point from the viewer.
	 *     this is calculated by projecting the vector ET onto
	 *     the forward vector.
	 *    Where E is the eye location.
	 *          T the wedgie's centre point.
	 * 
	 * Wsv = is the wedgies scaled variance (float).
	 * 
	 * Entry 0 contains the items with the highest variance
	 * Entry priorityResolution-1 contains the items with lowest variance (0).
	 */
	static float _varianceScale;
	/// Cached pointer to the static tree data.
	static ddgMSTri	*_stri;

	/// Cached pointer to the triangle cache.
	static ddgTCache	*_tcache;

	/// The number of visible triangles.
	unsigned int		_visTriangle;
	/// The number of unique vertices required to render.
	unsigned int		_uniqueVertex;
	/**
	 * Split a triangle into 2 and recusively split the parent
	 * neighbour triangles as needed.
	 */
	ddgPriority forceSplit2( ddgTriIndex tindex);
public:
	/**
	 *  Construct a Bintree mesh.
	 */
	ddgTBinTree(ddgTreeIndex i, int dr = 0, int dc = 0, bool mirror = false);
	/// Destroy the Bintree mesh.
	~ddgTBinTree(void);

	/// Return the number of visible triangles which this bin tree is managing.
	unsigned int visTriangle(void) { return _visTriangle; }
	/// Set the number of visible triangles which this bin tree is managing.
	void visTriangle(unsigned int v) { _visTriangle = v; }
	/// Return the number of unique vertices required to render.
	unsigned int uniqueVertex(void) { return _uniqueVertex; }
	/// Set the number of unique vertices required to render.
	void uniqueVertex(unsigned int v) { _uniqueVertex = v; }
	/// Return the neighbouring bin tree.
	ddgTBinTree* pNeighbourDiag(void) { return _pNeighbourDiag; }
	/// Return the neighbouring bin tree.
	ddgTBinTree* pNeighbourTop(void) { return _pNeighbourTop; }
	/// Return the neighbouring bin tree.
	ddgTBinTree* pNeighbourLeft(void) { return _pNeighbourLeft; }
	/// Set the neighbouring bin tree.
	void pNeighbourDiag(ddgTBinTree* t) { _pNeighbourDiag = t; }
	/// Set the neighbouring bin tree.
	void pNeighbourTop(ddgTBinTree* t) { _pNeighbourTop = t; }
	/// Set the neighbouring bin tree.
	void pNeighbourLeft(ddgTBinTree* t) { _pNeighbourLeft = t; }

	/// Returns the column offset in the height map.
	int dc(void) { return _dc; }
	/// Returns the row offset in the height map.
	int dr(void) { return _dr; }
	/// Returns true if this is a mirrored mesh.
	bool mirror(void) { return _mirror; }

	/// Initialize the bin tree.
	bool init(void);
private:
/*
	/// Get triangle row in the bin tree.
	unsigned int row(unsigned int i);
	/// Get triangle col.
	unsigned int col(unsigned int i);
*/
	/// Get the triangle row on the master mesh.
	unsigned int mrow(unsigned int i);
	/// Get the triangle column on the master mesh.
	unsigned int mcol(unsigned int i);

public:

	/// Return the parent of this element.
	static inline ddgTriIndex parent(ddgTriIndex i)
	{
//		asserts (i < _triNo,"Invalid element number.");
		if (i == 0)
			return 0; // Was _triNo
		if (i == 1) return 0;
		return i/2;
	}

	/// Return the index of the 1st child.
	static inline ddgTriIndex first(ddgTriIndex i) { ddgAssert(i>0); return i*2; }
	/// Return the index of the 2nd child.
	static inline ddgTriIndex second(ddgTriIndex i) { ddgAssert(i>0); return i*2+1; }
	/// Return the index of the 1st child.
	ddgTriIndex firstInMesh(void);

	/** Return the index of the 2nd child.
	 *  end must be initialized to 0 and will be reset to 0 when there is nothing left to do.
	 */
	ddgTriIndex nextInMesh(ddgTriIndex tindex, ddgTriIndex *end );

private:
	/// Return the quad neighbour. If 0 there is no neighbour.
	ddgTriIndex neighbour( ddgTriIndex i);
	/// Return the bintree of the neighbour.
	ddgTBinTree *neighbourTree( ddgTriIndex i);
public:
	/// Set the cache index for a triangle
	void tcacheId( ddgTriIndex tindex, ddgCacheIndex ci )
	{
		_cacheIndex[tindex] = ci;
	}
	/// Get the cache index for a triangle.
	ddgCacheIndex tcacheId( ddgTriIndex tindex )
	{
		return _cacheIndex[tindex];
	}

	/// Get the raw height for a triangle.
	short rawHeight( ddgTriIndex tindex )
	{
		return _rawHeight[tindex];
	}
	/// Get the raw height for a triangle.
	void rawHeight( ddgTriIndex tindex, short h )
	{
		_rawHeight[tindex] = h;
	}
	/// Get the min height for a triangle block.
	short rawMinVal( ddgTriIndex tindex )
	{
		return _rawMinVal[tindex];
	}
	/// Get the variance for a triangle block.
	short rawMaxVal( ddgTriIndex tindex )
	{
		return _rawMaxVal[tindex];
	}
	/// Get the variance for a triangle block.
	unsigned short treeError( ddgTriIndex tindex )
	{
		return _treeError[tindex];
	}
private:

	/**
	 *  Return the cache node which stores the extra data for this triangle
	 *  for the purpose of reading data.  If the cacheIndex = 0, we will
	 *  get back the default node.
	 */
	ddgTNode *gnode(ddgTriIndex tindex);
	/**
	 * Return the cache node which stores the extra data for this triangle,
	 * for the purpose of setting data.  If the cacheIndex = 0, we redirect
	 * to the scratch entry.
	 */
	ddgTNode *snode(ddgTriIndex tindex);
public:
	/// Return toplevel visibility state.
	ddgVisState treeVis(void)							{ return _treeVis; }
	/// Return toplevel visibility state.
	void treeVis(ddgVisState v )						{ _treeVis = v; }
	/// Return the objects visibility state.
	ddgVisState vis(ddgTriIndex tindex)				{ return gnode(tindex)->vis(); }
	/// Set the objects visibility state.
	void vis(ddgTriIndex tindex, ddgVisState v)		{ snode(tindex)->vis(v); }

    /// Return the priority of this triangle.
    int  priority(ddgTriIndex tindex)		{ return gnode(tindex)->priority(); }
    /// Set the priority of this triangle.
    void priority(ddgTriIndex tindex, int p)		{ snode(tindex)->priority( p ); }

    /// Return the priority of this triangle.
    float priorityFactor(ddgTriIndex tindex)		{ return gnode(tindex)->priorityFactor(); }
    /// Set the priority of this triangle.
    void priorityFactor(ddgTriIndex tindex, float pf)		{ snode(tindex)->priorityFactor( pf ); }

	/// Return split cache index if any.
	ddgCacheIndex qscacheIndex(ddgTriIndex tindex){ return gnode( tindex)->qscacheIndex(); }
	/// Set split cache index if any.
	void qscacheIndex(ddgTriIndex tindex, ddgCacheIndex i) { snode( tindex)->qscacheIndex(i); }

	/// Return split cache index if any.
	ddgCacheIndex qmcacheIndex(ddgTriIndex tindex){ return gnode( tindex)->qmcacheIndex(); }
	/// Set split cache index if any.
	void qmcacheIndex(ddgTriIndex tindex, ddgCacheIndex i) { snode( tindex)->qmcacheIndex(i); }

	/// Return the height of a location on the bintree mesh.
    float heightByPos(unsigned int r, unsigned int c);

    /** Get height of mesh at a real location.
     * Coords should be unscaled in x,z direction.
     */
    float treeHeight(unsigned int r, unsigned int c, float dx, float dz);
    /// Get vertex location in world space from the cache, return the cache index if we have it.
    void vertex(ddgTriIndex tindex, ddgVector3 *vout);

    /// Get texture coord data, as a value from 0 to 1 on the bintree.
    void textureC(unsigned int tindex, ddgVector2 *vout);

	/// Calculate visibility of a triangle.
	ddgVisState visibilityTriangle(ddgTriIndex tvc);
	/// Is triangle visible (even partially)?
	bool visible(ddgTriIndex tindex)
	{
		return (vis(tindex) != ddgOUT) ? true : false;
	}
	/// Is triangle fully visible?
	bool fullyvisible(ddgTriIndex tindex)
	{
		return (vis(tindex) == ddgIN) ? true : false;
	}
	/// Recusively initialize precomputed static parameters.
	void initTriangle( ddgTriIndex tindex);

	/// Calculate the variance for a triangle.
	void calculateVariance(ddgTriIndex tindex);

	/// Update variance for triangle and all affected triangles. (upward).
	void updateVariance( ddgTriIndex tindex);

	/// Split queue operations.
	void insertSQ(ddgTriIndex tindex, ddgPriority p, ddgCacheIndex ci, ddgVisState parentVis );
	ddgCacheIndex removeSQ(ddgTriIndex tindex);
	/// Split queue operations.
	void insertMQ(ddgTriIndex tindex, ddgPriority p);
	void removeMQ(ddgTriIndex tindex);

	/// Split a triangle into 2.
	void forceSplit( ddgTriIndex tindex);
	/// Merge 4 triangles into 2.
	void forceMerge( ddgTriIndex tindex);

	/// Recursively update the split info for a triangle which is in the mesh.
	void updateSplit(ddgTriIndex tindex, ddgVisState newVis );

	/// Update the merge info for a triangle which is in the mesh.
	void updateMerge(ddgTriIndex tindex, ddgPriority pr);
	/// Calculate priority of triangle tindex  We assume that we only get called
	ddgPriority priorityCalc(ddgTriIndex tindex, float priorityFactor);

	/// Return the index in the mesh.
	unsigned int index(void) { return _index; }

	/// Initialize view context called once during initialization.
	static void initContext( ddgContext *context, ddgTBinMesh *mesh );
	/// Update view context called each frame.
	static void updateContext( ddgContext *context, ddgTBinMesh *mesh );

	/**
	 * Ray test a line segment to see if it collides with the terrain.
	 * A depth of 0 implies testing to the leaf nodes.
	 * Returns true and the index of the triangle if a hit occured.
	 * Returns false otherwise.
	 */
	bool rayTest( ddgVector3 p1, ddgVector3 p2, ddgTriIndex tindex, int depth = -1 );
#ifdef DDG
	/// Generate a histogram of usage.
	static void setHist( ddgHistogram * hist);
#endif
};


#ifdef DDGSTREAM
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgTBinTree v );
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgTBinTree* v );
#endif // DDGSTREAM
#endif
