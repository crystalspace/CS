/*
    Copyright (C) 2001 by Jorrit Tyberghein
  
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

#ifndef __CS_TERRVIS_H__
#define __CS_TERRVIS_H__

#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "iterrain/object.h"
#include "iengine/terrain.h"
#include "iterrain/terrfunc.h"

#define CS_QUAD_TOPLEFT 0
#define CS_QUAD_TOPRIGHT 1
#define CS_QUAD_BOTLEFT 2
#define CS_QUAD_BOTRIGHT 3

/**
 * This is a node in the quadtree we use for terrain visibility
 * testing.
 */
class csTerrainQuad
{
private:
  csTerrainQuad* children[4];
  float min_height;
  float max_height;
  int visnr;	// Visible if equal to global_visnr;
  static int global_visnr;

public:
  csTerrainQuad ();
  ~csTerrainQuad ();

  /// Get minimum height.
  float GetMinimumHeight () { return min_height; }
  /// Set minimum height.
  void SetMinimumHeight (float h) { min_height = h; }
  /// Get maximum height.
  float GetMaximumHeight () { return max_height; }
  /// Set maximum height.
  void SetMaximumHeight (float h) { max_height = h; }

  /// Return true if node is visible.
  bool IsVisible () { return visnr == global_visnr; }
  /// Mark visible.
  void MarkVisible () { visnr = global_visnr; }
  /// Mark all nodes as invisible.
  static void MarkAllInvisible () { global_visnr++; }

  /// Build the quadtree for the given number of levels.
  void Build (int depth);

  /// Return true if this node is a leaf.
  bool IsLeaf () { return children[0] == NULL; }

  /// Get a child.
  csTerrainQuad* GetChild (int idx)
  {
    CS_ASSERT (idx >= 0 && idx < 4);
    return children[idx];
  }
};


#endif // __CS_TERRVIS_H__

