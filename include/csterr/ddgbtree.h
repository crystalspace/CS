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

#include "csterr/ddgtmesh.h"

/**
 * A triangle object maintained by a TBinTree mesh.
 * The data in this class is unique for each TBinTree.
 * 2+2+2+1+1=8 bytes.
 * Should be able to do this in 8 bytes.
 */
#define ddgMAXPRI 0xFFFF
#define ddgMINPRI 0
class WEXP ddgMTri {
	friend class ddgTBinTree;
	friend class ddgTBinMesh;  // May be able to remove dependency.
	/**
	 * Thickness of this triangle's wedgie.		(2 bytes)
	 * Equals the sum of thickness of all sub triangles.
	 */
	unsigned short _thick;
	/** Triangle priority. (2 bytes)
     *	0 - ddgMAXPRI   - Normal priorities.
	 *  ddgMAXPRI + 1   undefined.
	 */
	unsigned short _priority;
    /**
	 * This value if used for 2 purposes:						(2 bytes)
	 * Initially it is an index into the transformed vertex cache.
	 * at rendertime it is reused as an index into the vertex buffers.
	 */
	unsigned short _cbufindex;
	unsigned short _vbufindex;
   /// The flags which incidate which frustrum sides we intersected. (1 byte)
	ddgClipFlags _vis;
	/// Flags indicating the triangle's state.
	typedef union {
		unsigned char all;
		struct {
			// if priority fields == 0, priority must be recalced.
	        bool priority0:1;	/// 0 delay bit 0 LSB.
	        bool priority1:1;	/// 1 .
	        bool priority2:1;	/// 2 .
	        bool priority3:1;	/// 3 delay bit 3 MSB.
	        bool coord:1;		/// 4 Flag is true if coord cpqr is correct for frame.
	        bool vbuffer:1;     /// 5 Flag is true if the coord is in the vertex buffer for frame.
	        bool sq:1;			/// 6 Flag is true if triangle is in the split queue.
            bool mq:1;			/// 7 Flag is true if triangle is in the merge queue.
		} flags;
	} ddgStateFlags;
    /// State of triangle.												(1 byte)
    ddgStateFlags  _state;
public:
	/// Return the objects visibility flags.
	inline ddgClipFlags vis(void) { return _vis; }
	/// Return the objects visibility flags.
	inline ddgStateFlags state(void) { return _state; }
    /// Set the priority of this triangle.
    inline unsigned short  priority(unsigned short p) { return _priority = p; }
    /// Return the priority of this triangle.
    inline unsigned short  priority(void) { return _priority; }
    /// Set the buffer index of this triangle.
    inline unsigned int  vbufindex(unsigned int i) { return _vbufindex = i; }
    /// Return the buffer index of this triangle.
    inline unsigned int  vbufindex(void) { return _vbufindex; }
    /// Set the buffer flag for this triangle.
    inline unsigned int  setvbufflag(void) { return _state.flags.vbuffer = true; }
	///	get wedge thickness.
	inline float	thick(void )	{return(_thick/4.0);}
	///	set wedge thickness.
	inline void	thick(float t )	{_thick = (unsigned short)(4 * t);}
    /// Reset all but queue status.
    inline void reset()
	{
		_state.flags.coord = _state.flags.vbuffer = false;
		decrPriorityDelay();
		_vis.visibility = ddgINIT;
		_cbufindex = 0;
	}

	/// Set priority delay.  v should be a value from 0 to 15. 0 - invalid.
	inline void setPriorityDelay(unsigned char v)
	{
		resetPriorityDelay();
		_state.all += v;
	}
	/// Reset priority delay to zero.
	inline void resetPriorityDelay()
	{
#ifdef WIN32
		_state.all &= 0xF0;
#else
		_state.flags.priority0 =
		_state.flags.priority1 =
		_state.flags.priority2 =
		_state.flags.priority3 = false;
#endif
	}
	/// Decrement the delay by one.
	inline void decrPriorityDelay()
	{
		unsigned char v = getPriorityDelay();
		if (v>0)
			setPriorityDelay(v-1);
		ddgAssert(getPriorityDelay()>=0 && getPriorityDelay() < 16);
	}
	/// Return priority delay.
	inline unsigned char getPriorityDelay()
	{
#ifdef WIN32
		return _state.all & 0x0F;
#else
		return
			   ( _state.flags.priority3 ? 8 : 0)
			+  ( _state.flags.priority2 ? 4 : 0)
			+  ( _state.flags.priority1 ? 2 : 0)
			+  ( _state.flags.priority0 ? 1 : 0);
#endif
	}
};


/// Index array offsets for each level of the tree.
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
    /// Record initialization state.
    bool    _init:1;
    /// Col offset in maps.
    int     _dc;
    /// Row offset in maps.
    int     _dr;
    /// Is coordinate system inverted.
    bool    _mirror:1;
	/// Number of visible triangles in the bintree.
	unsigned int _visTri;
	/// Index of this tree in the mesh.
	unsigned int _index;
	/**
	 * Pointer to an array of MTri objects.
	 */
	ddgMTri* _tri;
	/// Height data map.
	ddgHeightMap *heightMap;
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
	static csVector3	_unit;

public:
	/**
	 *  Construct a Bintree mesh.
	 */
	ddgTBinTree(ddgTBinMesh *m, ddgTreeIndex i, ddgHeightMap* h, int dr = 0, int dc = 0, bool mirror = false);
	/// Destroy the Bintree mesh.
	~ddgTBinTree(void);

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
	int dc(void) { return _dc; }
	/// Returns the row offset in the height map.
	int dr(void) { return _dr; }
	/// Returns true if this is a mirrored mesh.
	bool mirror(void) { return _mirror; }

	/// Initialize the bin tree.
	bool init(void);
	/// Get maxLevel.
	unsigned int maxLevel(void) { return _mesh->maxLevel(); }
	/// Get number of triangles in a single BinTree.
	unsigned int triNo(void) { return _mesh->triNo(); }
	/// Get triangle row.
	unsigned int row(unsigned int i)
	{ return _mesh->stri[i].row; }
	/// Get triangle col.
	unsigned int col(unsigned int i)
	{ return _mesh->stri[i].col; }
	/// Get triangle vertex 0.
	ddgTriIndex v0(ddgTriIndex i)
	{ return _mesh->stri[i].v0; }
	/// Get triangle vertex 1.
	ddgTriIndex v1(ddgTriIndex i)
	{ return _mesh->stri[i].v1; }

	/// Return the height of a location on the mesh.
    float height(ddgTriIndex tindex)
    {
        return _mirror 
            ? heightMap->getf(_dr-row(tindex),_dc-col(tindex))
            : heightMap->getf(_dr+row(tindex),_dc+col(tindex));
    }
	/// Return the height of a location on the mesh.
    float height(unsigned int r, unsigned int c)
    {
        return _mirror 
            ? heightMap->getf(_dr-r,_dc-c)
            : heightMap->getf(_dr+r,_dc+c);
    }
    /** Get height of mesh at a real location.
     * Coords should be unscaled in x,z direction.
     */
    float treeHeight(unsigned int r, unsigned int c, float dx, float dz);
    /// Get vertex location.
    unsigned int vertex(ddgTriIndex tindex, csVector3 *vout)
    {
        if (_mirror)
            vout->Set(_dr-row(tindex),height(tindex),_dc-col(tindex));
        else
            vout->Set(_dr+row(tindex),height(tindex),_dc+col(tindex));
        if (tri(tindex)->state().flags.vbuffer)
            return tri(tindex)->vbufindex();
        return 0;
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
	static unsigned int offset(unsigned int l)
	{
		return ddgBintreeOffset[l];
	}

	/// Return the level of this triangle based on its index.
	static unsigned int level(ddgTriIndex i)
	{
		unsigned int l = 0;
		while ((i=i/2) > 1) l++;
		return l;
	}
	/// If this is odd (right) or even(left) child in the tree.
	static bool	isRight(ddgTriIndex i)
	{
		return ((i%2) == 1);
	}
	/// If this is odd (right) or even(left) child in the tree.
	static bool	isLeft(ddgTriIndex i)
	{
		return ((i%2) == 0);
	}
	/// Return the parent of this element.
	static ddgTriIndex parent(ddgTriIndex i)
	{
		if (i == 0)
			return 0; // Was _triNo
		if (i == 1) return 0;
		return i/2;
	}
	/// Return the index of the left child.
	static ddgTriIndex left(ddgTriIndex i)
	{
		return right(i)+1;
	}
	/// Return the index of the left child.
	static ddgTriIndex right(ddgTriIndex i)
	{
		return i*2;
	}
	/// Return the quad neighbour. If 0 there is no neighbour.
	ddgTriIndex neighbour( ddgTriIndex i)
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
	ddgTBinTree *neighbourTree( ddgTriIndex i)
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
	unsigned int edge( ddgTriIndex i)
	{ return _mesh->edge(i); }
	/// Is i part of a mergeble diamond?
	bool isDiamond(ddgTriIndex i);
    /**
     * Return if a mergeble diamond us within the viewing frustrum.
     * Returns true if at least one child is partially visible.
     */
    bool isDiamondVisible(ddgTriIndex i);

	/// Is triangle visible?
	bool visible(ddgTriIndex i)
	{
		return (tri(i)->vis().visibility > 0) ? true : false;
	}

	/// Calculate visibility of tree below a triangle.
	void visibility(ddgTriIndex tvc);

	/// Reset flags in the tree below this triangle.
	void reset(ddgTriIndex tvc);

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
	unsigned int visTri(void) { return _visTri; }
	/// Reset the visible number of triangles to zero.
	void resetVisTri(void) { _visTri = 0; }

	/// The mesh that manages this bintree.
	ddgTBinMesh *mesh(void) { return _mesh; }
	/// Return the index in the mesh.
	unsigned int index(void) { return _index; }
	/// Init wtoc.
	static void initWtoC( ddgTBinMesh *mesh);
	/// Return the camera space vector.
	csVector3* pos(ddgTriIndex ti);
	/// Return the camera space vector.
	inline float pos(ddgTriIndex ti, unsigned int i)
	{
		return (*pos( ti ))[i];
	}
#ifdef _DEBUG
	/// Test if queue status is correct for this triangle, return true on error.
	bool verify( ddgTriIndex tindex );
#endif
};

#endif
