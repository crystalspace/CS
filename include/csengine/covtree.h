/*
    Copyright (C) 1999 by Jorrit Tyberghein
  
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

#ifndef COVTREE_H
#define COVTREE_H

#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "csengine/covmask.h"

class Dumper;
struct iGraphics2D;

/**
 * The coverage mask tree.
 */
class csCoverageMaskTree
{
  friend class Dumper;

private:
  /**
   * The pointer to the templated tree. This is a void pointer for two reasons:
   * <ul>
   *   <li> To keep the template stuff out of the rest of the code.
   *   <li> It isn't known in advance which of the template instances will
   *        be used here and we don't have a common supertype.
   * </ul>
   */
  void* tree;
  /// The bounding box of the entire tree.
  csBox bbox;
  /// The coverage mask lookup table.
  csCovMaskLUT* lut;

public:
  /// Create a new tree for the given box.
  csCoverageMaskTree (csCovMaskLUT* lut, const csBox& box);

  /// Destructor.
  virtual ~csCoverageMaskTree ();

  /**
   * Initialize the coverage mask tree with an inverted
   * polygon. This can be used after MakeEmpty() to set
   * the coverage mask tree to the view frustrum.
   */
  void UpdatePolygonInverted (csVector2* verts, int num_verts);

  /**
   * Insert a polygon into the coverage mask tree.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible.
   */
  bool InsertPolygon (csVector2* verts, int num_verts,
  	const csBox& pol_bbox);

  /**
   * Test for polygon visibility with the coverage mask tree.
   * Return true if polygon is visible.
   */
  bool TestPolygon (csVector2* verts, int num_verts,
  	const csBox& pol_bbox) const;

  /// Make the tree empty.
  void MakeEmpty ();

  /// Make the tree invalid (for debugging).
  void MakeInvalid ();

  /// Test if tree is full.
  bool IsFull ();

  /**
   * Test consistancy for the tree (for debugging).
   */
  void TestConsistency ();

  /**
   * Do a graphical dump of the coverage mask tree
   * upto the specified level.
   */
  void GfxDump (iGraphics2D* ig2d, int level);
};

#endif /*COVTREE_H*/

