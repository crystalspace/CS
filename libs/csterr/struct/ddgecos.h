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
#ifndef _ddgEcoSystem_Class_
#define _ddgEcoSystem_Class_
#include "math/ddgvec.h"

class ddgImage;
class ddgTBinMesh;
class ddgBBox;
class ddgPlane;
class ddgEcoBlock;
/**
 * An ecology object defines the whereabouts of object data across a terrain.
 */
class WEXP ddgEcoSystem {
	friend class ddgEcoBlock;
	/// Total number of trees in the forest.
	unsigned int _size;
	/// The mesh upon which to place the trees.
	ddgTBinMesh	*_mesh;
	/// Ecosystem map to select trees.
	ddgImage	*_ecoMap;
	/// Tree locations.
	ddgVector3	*_pos;
	/// Bounding box where object can occur in the world.
	ddgBBox		*_bbox;
	/// Size of object to be placed in bbox where object can occur.
	ddgVector3	_osize;
	/// Min ecosystem value where this object can occur;
	unsigned char	_min;
	/// Max ecosystem value where this object can occur;
	unsigned char	_max;
	/// Far clipping plane.  0 = infinite.
	float			_farClip;
	/// Far clipping plane.  0 = infinite.
	float			_farClipSQ;
public:
	/// Array of eco blocks generated from this ecosystem.
	ddgEcoBlock		**ecoBlocks;
	/// Number of blocks.
	unsigned int	blockNum;
	/// Constructor
	ddgEcoSystem( unsigned int s, ddgTBinMesh *m,
		ddgBBox  *b, ddgVector3 *o,
		ddgImage *e = NULL, unsigned char min = 0, unsigned char max = 255,
		float f = 100)
		: ecoBlocks(0), blockNum(0), _size(s), _mesh(m), _bbox(b), _osize(o), _ecoMap(e), _min(min), _max(max), _farClip(f), _farClipSQ(f*f) {}
	/// Destructor.
	~ddgEcoSystem(void);
	/// Initialize the EcoSystem.
	bool init(void);
};
/**
 * A eco block object defines object positions within a region of an ecosystem.
 * These objects are managed by the ecosystem object.
 */
class WEXP ddgEcoBlock {
	/// Visibility state of this block.
	unsigned int	_vis;
	/**
	 * Desired number of objects in the eco block.  Maybe be adjusted after
	 * initialization.
	 */
	unsigned int _size;
	/// The ecosystem to which this block belongs.
	ddgEcoSystem	*_es;
public:
	/// Bounding box for this block where object can occur in the world.
	ddgBBox		*bbox;
	/// Tree locations.
	ddgVector3	*pos;
	/// Constructor
	ddgEcoBlock( ddgEcoSystem *es, unsigned int s, ddgBBox  *b)
		:  _es(es), _size(s), bbox(b) {}
	/// Initialize the EcoBlock.
	bool init( ddgEcoSystem *e);
	/// Calculate and return the visibility state of this block.
	unsigned int vis( ddgPlane *frustrum, ddgVector2 pos );
	/// Return the visibility state of this block.
	unsigned int vis( void ) { return _vis; }
	/// Return the number of objects in this block.
	unsigned int size(void) { return _size; }
	/// Return the 3D size of objects in this block.
	ddgVector3 osize(void) { return _es->_osize; }
};

#endif
