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
#ifndef _ddgBinTree_Class_
#define _ddgBinTree_Class_

#include "struct/ddgtmesh.h"
#include "struct/ddgcntxt.h"

typedef unsigned short ddgPriority;

#define ddgMAXPRI 0xFFFF
#define ddgMINPRI 0

/// Index array offsets for each level of the implicit binary tree.
const unsigned int ddgBintreeOffset[32] =
	{ 0x1,       0x2,       0x4,       0x8,
	  0x10,      0x20,      0x40,      0x80,
	  0x100,     0x200,     0x400,     0x800,
	  0x1000,    0x2000,    0x4000,    0x8000,
	  0x10000,   0x20000,   0x40000,   0x80000,
	  0x100000,  0x200000,  0x400000,  0x800000,
	  0x1000000, 0x2000000, 0x4000000, 0x8000000,
	  0x10000000,0x20000000,0x40000000,0x80000000
	};

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
	/// The SharedBinTree data. (Could be static)
    ddgTBinMesh	*_mesh;
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
	 * Maximum value of all triangles of this triangle's subtree	(2 bytes)
	 */
	short *_rawMaxVal;


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
	static ddgVector2	_pos;
	static ddgVector2	_forward;
	static float _farClip;
	static float _nearClip;
	static float _farClipSQ;
	static int _priorityScale;
	static ddgMSTri	*_stri;
	static ddgTCache	*_tcache;
	/// A cache chain for this bintree.
	ddgCacheIndex		_chain;
	/// The number of visible triangles.
	unsigned int		_visTriangle;
	/**
	 * Split a triangle into 2 and recusively split the parent
	 * neighbour triangles as needed.
	 */
	ddgPriority forceSplit2( ddgTriIndex tindex);
public:
	/**
	 *  Construct a Bintree mesh.
	 */
	ddgTBinTree(ddgTBinMesh *m, ddgTreeIndex i, int dr = 0, int dc = 0, bool mirror = false);
	/// Destroy the Bintree mesh.
	~ddgTBinTree(void);

	/// Free all the cached nodes in this bintree.
	void freeChain(void) {
		_chain = 0;
	}
	/// Free all the cached nodes in this bintree.
	ddgCacheIndex chain(void) {
		return _chain;
	}
	/// Return the number of visible triangles which this bin tree is managing.
	unsigned int visTriangle(void) { return _visTriangle; }
	/// Set the number of visible triangles which this bin tree is managing.
	void visTriangle(unsigned int v) { _visTriangle = v; }
	/// Return the neighbouring bin tree.
	inline ddgTBinTree* pNeighbourDiag(void) { return _pNeighbourDiag; }
	/// Return the neighbouring bin tree.
	inline ddgTBinTree* pNeighbourTop(void) { return _pNeighbourTop; }
	/// Return the neighbouring bin tree.
	inline ddgTBinTree* pNeighbourLeft(void) { return _pNeighbourLeft; }
	/// Set the neighbouring bin tree.
	inline void pNeighbourDiag(ddgTBinTree* t) { _pNeighbourDiag = t; }
	/// Set the neighbouring bin tree.
	inline void pNeighbourTop(ddgTBinTree* t) { _pNeighbourTop = t; }
	/// Set the neighbouring bin tree.
	inline void pNeighbourLeft(ddgTBinTree* t) { _pNeighbourLeft = t; }

	/// Returns the column offset in the height map.
	inline int dc(void) { return _dc; }
	/// Returns the row offset in the height map.
	inline int dr(void) { return _dr; }
	/// Returns true if this is a mirrored mesh.
	inline bool mirror(void) { return _mirror; }

	/// Initialize the bin tree.
	bool init(void);
	/// Get maxLevel.
	inline unsigned int maxLevel(void) { return _mesh->maxLevel(); }
	/// Get number of triangles in a single BinTree.
	inline unsigned int triNo(void) { return _mesh->triNo(); }
	/// Get triangle row in the bin tree.
	inline unsigned int row(unsigned int i)
	{ return _mesh->stri[i].row; }
	/// Get triangle col.
	inline unsigned int col(unsigned int i)
	{ return _mesh->stri[i].col; }
	/// Get the triangle row on the master mesh.
	inline unsigned int mrow(unsigned int i)
	{ return _mirror ? _dr - _mesh->stri[i].row : _dr + _mesh->stri[i].row; }
	/// Get the triangle column on the master mesh.
	inline unsigned int mcol(unsigned int i)
	{ return _mirror ? _dc - _mesh->stri[i].col : _dc + _mesh->stri[i].col; }
	/// Get triangle vertex 0.
	inline ddgTriIndex v0(ddgTriIndex i)
	{ ddgAssert(i < _mesh->triNo()); return _mesh->stri[i].v0; }
	/// Get triangle vertex 1.
	inline ddgTriIndex v1(ddgTriIndex i)
	{ ddgAssert(i < _mesh->triNo()); return _mesh->stri[i].v1; }
	/** Return the starting offset in the array where a
	 * given level is stored.
	 */
	inline static unsigned int offset(unsigned int l)
	{
		return ddgBintreeOffset[l];
	}

	/// Return the level of this triangle based on its index.
	inline static unsigned int level(ddgTriIndex i)
	{
		unsigned int l = 0;
		if (i > 0)
			while ((i=i/2) > 1)
				l++;
		return l;
	}
	/// If this is odd (right) or even(left) child in the tree.
	inline static bool	isRight(ddgTriIndex i)
	{
		return ((i%2) == 1);
	}
	/// If this is odd (right) or even(left) child in the tree.
	inline static bool	isLeft(ddgTriIndex i)
	{
		return ((i%2) == 0);
	}
	/// Return the parent of this element.
	inline static ddgTriIndex parent(ddgTriIndex i)
	{
//		asserts (i < _triNo,"Invalid element number.");
		if (i == 0)
			return 0; // Was _triNo
		if (i == 1) return 0;
		return i/2;
	}
	/// Return the index of the left child.
	inline static ddgTriIndex right(ddgTriIndex i)
	{
		return i*2;
	}
	/// Return the index of the left child.
	inline static ddgTriIndex left(ddgTriIndex i)
	{
		return right(i)+1;
	}
	/// Return the quad neighbour. If 0 there is no neighbour.
	inline ddgTriIndex neighbour( ddgTriIndex i)
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
	inline ddgTBinTree *neighbourTree( ddgTriIndex i)
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

	/// Set the cache index for a triangle
	inline void tcacheId( ddgTriIndex tindex, ddgCacheIndex ci )
	{
		_cacheIndex[tindex] = ci;
	}
	/// Get the cache index for a triangle.
	inline ddgCacheIndex tcacheId( ddgTriIndex tindex )
	{
		return _cacheIndex[tindex];
	}

	/// Get the raw height for a triangle.
	inline short rawHeight( ddgTriIndex tindex )
	{
		return _rawHeight[tindex];
	}
	/// Get the min height for a triangle block.
	inline short rawMinVal( ddgTriIndex tindex )
	{
		return _rawMinVal[tindex];
	}
	/// Get the max height for a triangle block.
	inline short rawMaxVal( ddgTriIndex tindex )
	{
		return _rawMaxVal[tindex];
	}
	/**
	 *  Return the cache node which stores the extra data for this triangle
	 *  for the purpose of reading data.  If the cacheIndex = 0, we will
	 *  get back the default node.
	 */
	inline ddgTNode *gnode(ddgTriIndex tindex)
	{
		return _mesh->tcache()->get(tcacheId(tindex));
	}

	/**
	 * Return the cache node which stores the extra data for this triangle,
	 * for the purpose of setting data.  If the cacheIndex = 0, we redirect
	 * to the scratch entry.
	 */
	inline ddgTNode *snode(ddgTriIndex tindex)
	{
		return _mesh->tcache()->get(tcacheId(tindex)?tcacheId(tindex):1);
	}
	
	/// Return the objects visibility state.
	inline ddgVisState vis(ddgTriIndex tindex)				{ return gnode(tindex)->vis(); }
	/// Set the objects visibility state.
	inline void vis(ddgTriIndex tindex, ddgVisState v)		{ snode(tindex)->vis(v); }

    /// Return the priority of this triangle.
    inline int  priority(ddgTriIndex tindex)		{ return gnode(tindex)->priority(); }
    /// Set the priority of this triangle.
    void priority(ddgTriIndex tindex, int p)		{ snode(tindex)->priority( p ); }

    /// Return the priority of this triangle.
    inline float priorityFactor(ddgTriIndex tindex)		{ return gnode(tindex)->priorityFactor(); }
    /// Set the priority of this triangle.
    void priorityFactor(ddgTriIndex tindex, float pf)		{ snode(tindex)->priorityFactor( pf ); }

	/// Return flags.
	inline unsigned char vbufferIndex(ddgTriIndex tindex){ return gnode( tindex)->vbufferIndex(); }
	/// Set flags.
	inline void vbufferIndex(ddgTriIndex tindex, unsigned int i) { snode( tindex)->vbufferIndex(i); }

	/// Return split cache index if any.
	inline ddgCacheIndex qscacheIndex(ddgTriIndex tindex){ return gnode( tindex)->qscacheIndex(); }
	/// Set split cache index if any.
	inline void qscacheIndex(ddgTriIndex tindex, ddgCacheIndex i) { snode( tindex)->qscacheIndex(i); }

	/// Return split cache index if any.
	inline ddgCacheIndex qmcacheIndex(ddgTriIndex tindex){ return gnode( tindex)->qmcacheIndex(); }
	/// Set split cache index if any.
	inline void qmcacheIndex(ddgTriIndex tindex, ddgCacheIndex i) { snode( tindex)->qmcacheIndex(i); }
	
	/// Return the height of a location on the mesh.
    inline float height(ddgTriIndex tindex)
    {
        return // _cacheIndex[tindex] ? gnode( tindex)->height() :
			 _mesh->wheight(_rawHeight[tindex]); 
    }
	/// Return the height of a location on the mesh.
    inline float minVal(ddgTriIndex tindex)
    {
        return // _cacheIndex[tindex] ? gnode( tindex)->minHeight() :
			 _mesh->wheight(_rawMinVal[tindex]); 
    }
	/// Return the height of a location on the mesh.
    inline float maxVal(ddgTriIndex tindex)
    {
        return // _cacheIndex[tindex] ? gnode( tindex)->maxHeight() :
			  _mesh->wheight(_rawMaxVal[tindex]); 
    }

	/// Return the height of a location on the bintree mesh.
    inline float heightByPos(unsigned int r, unsigned int c)
    {
		// See if we are in the correct tree to find this location.
		ddgAsserts(r + c <= ddgTBinMesh_size,"Looking for data out of range.");

		ddgTriIndex tindex = _mesh->lookupIndex(r,c);
		return height(tindex);
    }

    /** Get height of mesh at a real location.
     * Coords should be unscaled in x,z direction.
     */
    float treeHeight(unsigned int r, unsigned int c, float dx, float dz);
    /// Get vertex location in world space from the cache, return the cache index if we have it.
    inline unsigned int vertex(ddgTriIndex tindex, ddgVector3 *vout)
    {
		*vout = ddgVector3(mrow(tindex),height(tindex),mcol(tindex));

        if (vbufferIndex(tindex))
			return 0;

        return vbufferIndex(tindex);
    }

    /// Get texture coord data, as a value from 0 to 1 on the bintree.
    void textureC(unsigned int tindex, ddgVector2 *vout)
    {
        if (_mirror)
        	vout->set(ddgTBinMesh_size-row(tindex),ddgTBinMesh_size-col(tindex));
        else
        	vout->set(row(tindex),col(tindex));
		vout->multiply( 1.0 / (float)ddgTBinMesh_size);
	}

	/// Calculate visibility of a triangle.
	ddgVisState visibilityTriangle(ddgTriIndex tvc);
	/// Is triangle visible (even partially)?
	inline bool visible(ddgTriIndex i)
	{
		return (vis(i) != ddgOUT) ? true : false;
	}
	/// Is triangle fully visible?
	inline bool fullyvisible(ddgTriIndex i)
	{
		return (vis(i) == ddgIN) ? true : false;
	}
	/**
	 *  Initialize precomputed parameters.
	 *  Recusively calculate a convex hull for a triangle.
	 *  Splitting occurs along this points parent's 
	 *  Passed in are the triangles which carry the points
	 *  that describe the current triangle.
	 *  va is immediate parent.
	 *  v1 and v0 are the sub triangles of va. 
	 */
	void initTriangle( unsigned int level,
			   ddgTriIndex va,
			   ddgTriIndex v1,
			   ddgTriIndex v0,
			   ddgTriIndex vc);

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
	ddgPriority priorityCalc(ddgTriIndex tindex);

	/// The mesh that manages this bintree.
	inline ddgTBinMesh *mesh(void) { return _mesh; }

	/// Return the index in the mesh.
	inline unsigned int index(void) { return _index; }

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
};

#ifdef DDGSTREAM
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgTBinTree v );
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgTBinTree* v );
#endif // DDGSTREAM
#endif
