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

#ifdef DDG
#include "ddgtmesh.h"
#include "ddgbbox.h"
#include "ddggeom.h"
#else
#include "csterr/ddgtmesh.h"
extern csVector3 transformer (csVector3 vin);
#endif

/**
 * A triangle object maintained by a TBinTree mesh.
 * The data in this class is unique for each TBinTree.
 * 2+2+2+1+1=8 bytes.
 * Should be able to do this in 8 bytes.
 * Convert to Structure Of Arrays.
 */
#define ddgMAXPRI 0xFFFF
#define ddgMINPRI 0
class WEXP ddgMTri {
	friend class ddgTBinTree;
	friend class ddgTBinMesh;  // May be able to remove dependency.
	/**
	 * Thickness of this triangle's wedgie.					(2 bytes)
	 * Equals the sum of thickness of all sub triangles.
	 */
	unsigned short _thick;
	/** Triangle priority.									(2 bytes)
     *	0 - ddgMAXPRI   - Normal priorities.
	 *  ddgMAXPRI + 1   undefined.
	 */
	unsigned short _priority;
    /**
	 * This value is used for 2 purposes:						(2 bytes)
	 * Initially it is an index into the transformed vertex cache.
	 * at rendertime it is reused as an index into the vertex buffers.
	 */
	unsigned short _cbufindex;
    /**
	 * The state which indicates whether we are visible w.r.t. the viewing frustrum. (1 byte)
	 * Only 2 bits are currently used.
	 */
	ddgVisState _vis;
	/// Flags indicating the triangle's state.
	enum {	// if all priority bits are clear, priority must be recalced.
	  SF_PRIORITY0 = 1 << 0, /// 0 delay bit 0 LSB.
	  SF_PRIORITY1 = 1 << 1, /// 1 .
	  SF_PRIORITY2 = 1 << 2, /// 2 .
	  SF_PRIORITY3 = 1 << 3, /// 3 delay bit 3 MSB.
	  SF_COORD     = 1 << 4, /// 4 Flag is true if coord cpqr is correct for frame.
	  SF_VBUFFER   = 1 << 5, /// 5 Flag is true if the coord is in the vertex buffer for frame.
	  SF_SQ        = 1 << 6, /// 6 Flag is true if triangle is in the split queue.
      SF_MQ        = 1 << 7  /// 7 Flag is true if triangle is in the merge queue.
	};
	#define SF_PRIORITY (SF_PRIORITY0 | SF_PRIORITY1 | SF_PRIORITY2 | SF_PRIORITY3)
	typedef unsigned char ddgStateFlags;

    /// State of triangle.												(1 byte)
    ddgStateFlags  _state;
public:
    /**
	 * This value if used for triangle cache id:				(2 bytes)
	 */
	unsigned short tcacheId;
	/// Return the objects visibility state.
	inline ddgVisState vis(void) { return _vis; }
	/// Set the objects visibility state.
	inline void vis(ddgVisState v) { _vis = v; }
	/// Return the objects visibility flags.
	inline ddgStateFlags state(void) { return _state; }
    /// Set the priority of this triangle.
    inline unsigned short  priority(unsigned short p) { return _priority = p; }
    /// Return the priority of this triangle.
    inline unsigned short  priority(void) { return _priority; }
    /// Set the buffer index of this triangle.
    inline unsigned int  vbufindex(unsigned int i) { return _cbufindex = i; }
    /// Return the buffer index of this triangle.
    inline unsigned int  vbufindex(void) { return _cbufindex; }
    /// Set the buffer flag for this triangle.
    inline unsigned int  setvbufflag(void) { DDG_BSET(_state, SF_VBUFFER); return true; }
	///	get wedge thickness.
	inline float	thick(void )	{return(_thick/4.0);}
	///	set wedge thickness.
	inline void	thick(float t )	{_thick = (unsigned short)(4.0 * t);}

	/// Reset priority delay to zero.
	inline void resetPriorityDelay()
	{
		DDG_BCLEAR(_state, SF_PRIORITY);
	}
	/// Set priority delay.  v should be a value from 0 to 15. 0 - invalid.
	inline void setPriorityDelay(unsigned char v)
	{
		resetPriorityDelay();
		_state += v;
	}
	/// Return priority delay.
	inline unsigned char getPriorityDelay()
	{
		return DDG_BGET(_state, SF_PRIORITY);
	}
	/// Decrement the delay by one.
	inline void decrPriorityDelay()
	{
		unsigned char v = getPriorityDelay();
		if (v>0)
			setPriorityDelay(v-1);
		ddgAssert(getPriorityDelay()>=0 && getPriorityDelay() < 16);
	}
    /// Reset all but queue status.
    inline void reset()
	{
		DDG_BCLEAR(_state, SF_COORD);
		DDG_BCLEAR(_state, SF_VBUFFER);
		decrPriorityDelay();
		vis( ddgUNDEF);
		_cbufindex = 0;
	}
};


/// Index array offsets for each level of the immplicit binary tree.
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
 * ROAM Triangle Bintree mesh.
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
	/// Number of triangles in queue for the bintree.
	unsigned int _queueTri;
	/// Index of this tree in the mesh.
	unsigned int _index;
	/**
	 * Pointer to an array of MTri objects.
	 */
	ddgMTri* _tri;
	/// Height data map.
	ddgHeightMap *heightMap;
	/// Normals data map.
	unsigned short *normalIdx;
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
	/// Unit vector updated once per frame.
	static ddgVector3	_unit;
public:
	/**
	 *  Construct a Bintree mesh.
	 */
	ddgTBinTree(ddgTBinMesh *m, ddgTreeIndex i, ddgHeightMap* h, unsigned short *idx, int dr = 0, int dc = 0, bool mirror = false);
	/// Destroy the Bintree mesh.
	~ddgTBinTree(void);

	/// Initialize the bin tree.
	bool init(void);
	/// Get maxLevel.
	inline unsigned int maxLevel(void) { return _mesh->maxLevel(); }
	/// Get number of triangles in a single BinTree.
	inline unsigned int triNo(void) { return _mesh->triNo(); }
	/// Get triangle row.
	inline unsigned int row(unsigned int i)
	{ return _mesh->stri[i].row; }
	/// Get triangle col.
	inline unsigned int col(unsigned int i)
	{ return _mesh->stri[i].col; }
	/// Get triangle vertex 0.
	inline ddgTriIndex v0(ddgTriIndex i)
	{ ddgAssert(i < _mesh->triNo()); return _mesh->stri[i].v0; }
	/// Get triangle vertex 1.
	inline ddgTriIndex v1(ddgTriIndex i)
	{ ddgAssert(i < _mesh->triNo()); return _mesh->stri[i].v1; }

	/// Return the triangle tree.
	inline ddgMTri* tri(unsigned int i) { return &_tri[i]; }
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

	/// Return the height of a location on the mesh.
    inline float height(ddgTriIndex tindex)
    {
        return _mirror 
            ? heightMap->getf(_dr-row(tindex),_dc-col(tindex))
            : heightMap->getf(_dr+row(tindex),_dc+col(tindex));
    }
	/// Return the height of a location on the mesh.
    inline float height(unsigned int r, unsigned int c)
    {
        return _mirror 
            ? heightMap->getf(_dr-r,_dc-c)
            : heightMap->getf(_dr+r,_dc+c);
    }
    /** Get height of mesh at a real location.
     * Coords should be unscaled in x,z direction.
     */
    float treeHeight(unsigned int r, unsigned int c, float dx, float dz);
    /// Get vertex location in world space from the cache, return the cache index if we have it.
    inline unsigned int vertex(ddgTriIndex tindex, ddgVector3 *vout)
    {
        if (_mirror)
            *vout = ddgVector3(_dr-row(tindex),height(tindex),_dc-col(tindex));
        else
            *vout = ddgVector3(_dr+row(tindex),height(tindex),_dc+col(tindex));

        if (DDG_BGET(tri(tindex)->state(), ddgMTri::SF_VBUFFER))
            return tri(tindex)->vbufindex();
        return 0;
    }

    inline void normal(ddgTriIndex tindex, ddgVector3* n)
    {
		unsigned int idx;
        if (_mirror)
        	idx = normalIdx[(_dr-row(tindex))*heightMap->cols()+(_dc-col(tindex))];
        else
        	idx = normalIdx[(_dr+row(tindex))*heightMap->cols()+(_dc+col(tindex))];
		n= _mesh->normalLUT(idx);
    }

    /// Get texture coord data.
    void textureC(unsigned int tindex, ddgVector2 *vout)
    {
        if (_mirror)
        	vout->set(_dr-row(tindex),_dc-col(tindex));
        else
        	vout->set(_dr+row(tindex),_dc+col(tindex));
	}

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
#ifdef WIN32
			while ((i=i>>1) > 1)
#else
			while ((i=i/2) > 1)
#endif
				l++;
		return l;
	}
	/// If this is odd (right) or even(left) child in the tree.
	inline static bool	isRight(ddgTriIndex i)
	{
#ifdef WIN32
		return ((i&1) == 1);
#else
		return ((i%2) == 1);
#endif
	}
	/// If this is odd (right) or even(left) child in the tree.
	inline static bool	isLeft(ddgTriIndex i)
	{
#ifdef WIN32
		return ((i&1) == 0);
#else
		return ((i%2) == 0);
#endif
	}
	/// Return the parent of this element.
	inline static ddgTriIndex parent(ddgTriIndex i)
	{
//		asserts (i < _triNo,"Invalid element number.");
		if (i == 0)
			return 0; // Was _triNo
		if (i == 1) return 0;
#ifdef WIN32
		return i>>1;
#else
		return i/2;
#endif
	}
	inline static ddgTriIndex right(ddgTriIndex i)
	{
#ifdef WIN32
		return i<<1;
#else
		return i*2;
#endif
	}
	/// Return the index of the left child.
	inline static ddgTriIndex left(ddgTriIndex i)
	{
		return right(i)+1;
	}
	/// Return the index of the left child.
	/// Return the quad neighbour. If 0 there is no neighbour.
	inline ddgTriIndex neighbour( ddgTriIndex i)
	{
		switch(_mesh->edge(i))
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
		switch(_mesh->edge(i))
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
	/// Return the edge information.
	inline unsigned int edge( ddgTriIndex i)
	{ return _mesh->edge(i); }
	/// Is i part of a mergeble diamond?
	bool isDiamond(ddgTriIndex i);
    /**
     * Return if a mergeble diamond us within the viewing frustrum.
     * Returns true if at least one child is partially visible.
     */
    bool isDiamondVisible(ddgTriIndex i);

	/// Is triangle visible (even partially)?
	inline bool visible(ddgTriIndex i)
	{
		return (tri(i)->vis() != ddgOUT) ? true : false;
	}
	/// Is triangle fully visible?
	inline bool fullyvisible(ddgTriIndex i)
	{
		return (tri(i)->vis() == ddgIN) ? true : false;
	}

	/// Calculate visibility of tree below a triangle.
	void visibility(ddgTriIndex tvc);

	/// Update priorities of all triangles below this triangle.
	void priorityUpdate(ddgTriIndex tvc);
	/**
	 *  Split this triangle into 2, and return the thickness.
	 *  Splitting occurs along this points parent's 
	 *  Passed in are the triangles which carry the points
	 *  that describe the current triangle.
	 *  va is immediate parent.
	 *  v1 and v0 are the sub triangles of va. 
	 */
	float split( unsigned int level,
			   ddgTriIndex va,
			   ddgTriIndex v1,
			   ddgTriIndex v0,
			   ddgTriIndex vc);

	/// Split queue operations.
	void insertSQ(ddgTriIndex i);
	void removeSQ(ddgTriIndex i);

	/// Merge queue operations.
	void insertMQ(ddgTriIndex i);
	void removeMQ(ddgTriIndex i);

	/// Split a triangle into 2.
	void forceSplit( ddgTriIndex i);
	/// Merge 2 triangles into one.
	void forceMerge( ddgTriIndex i);

	/// Return priority of triangle tindex.
	unsigned short priority(ddgTriIndex tindex);

	/// Return the number of visible triangles in the mesh.
	inline unsigned int queueTri(void) { return _queueTri; }

	/// The mesh that manages this bintree.
	inline ddgTBinMesh *mesh(void) { return _mesh; }
	/// Return the index in the mesh.
	inline unsigned int index(void) { return _index; }
	/// Init wtoc.
#ifdef DDG
	static void initWtoC( ddgMatrix4 *wtoc, ddgBBox *camClipBox, float fov, ddgPlane frustrum[6]);
#else
	static void initWtoC( ddgBBox *camClipBox );
#endif
	/// Transform coordinate from world to camera space.
    static void transform( ddgVector3 *vin, ddgVector3 *vout );
	/// Return the camera space vector.
	inline ddgVector3* pos(ddgTriIndex tindex)
	{
		if (!DDG_BGET(tri(tindex)->_state, ddgMTri::SF_COORD))
		{
			ddgVector3 v;
			vertex(tindex,&v);
			ddgAssert(tri(tindex)->_cbufindex == 0);
			tri(tindex)->_cbufindex = _mesh->vcache()->allocate();
#ifdef DDG
			transform( v, _mesh->vcache()->get( tri(tindex)->_cbufindex ) );
#else
			*(_mesh->vcache()->get( tri(tindex)->_cbufindex )) = transformer( v );
#endif
			DDG_BSET(tri(tindex)->_state, ddgMTri::SF_COORD);
		}
		return _mesh->vcache()->get( tri(tindex)->_cbufindex );
	}
	/// Return the one dimension of the camera space vector.
#ifdef DDG
	inline float pos(ddgTriIndex tindex, unsigned int i) { return ((float*)(*pos(tindex)))[i]; }
#else
	inline float pos(ddgTriIndex tindex, unsigned int i) { return ((*pos(tindex)))[i]; }
#endif
#ifdef _DEBUG
	/// Test if queue status is correct for this triangle, return true on error.
	bool verify( ddgTriIndex tindex );
#endif
};

#ifdef DDG
WEXP ostream& WFEXP operator << ( ostream&s, ddgTBinTree v );
///
WEXP ostream& WFEXP operator << ( ostream&s, ddgTBinTree* v );
#endif

WEXP void WFEXP incrframe(void);
WEXP void WFEXP logactivate(void);
#endif
