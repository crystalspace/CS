/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef _CS_PVS_H
#define _CS_PVS_H

#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "csengine/arrays.h"

class csOctree;
class csOctreeNode;
class csPVS;
class Dumper;

/**
 * A visibility info node for one octree node.
 * This node represents a visible octree node and possibly
 * all visible polygons (if the node is a leaf).
 */
class csOctreeVisible
{
  friend class csPVS;

private:
  // Next visibility entry.
  csOctreeVisible* next;
  // Previous visibility entry.
  csOctreeVisible* prev;
  // The visible polygons.
  csPolygonArrayNoFree polygons;
  // The visible node.
  csOctreeNode* node;
  // If true then this node is TRUELY visible. i.e. we
  // know that there is at least one point in the leaf from where this
  // node can be seen.
  bool really_visible;
  
public:
  csOctreeVisible() : next (NULL), prev (NULL), polygons (8, 16),
  	node (NULL), really_visible (false) {}
  /// Set octree node.
  void SetOctreeNode (csOctreeNode* onode) { node = onode; }
  /// Get octree node.
  csOctreeNode* GetOctreeNode () { return node; }
  /// Get the polygon array.
  csPolygonArrayNoFree& GetPolygons () { return polygons; }
  /// Return true if this node is really visible.
  bool IsReallyVisible () { return really_visible; }
  /// Mark this node as really visible.
  void MarkReallyVisible () { really_visible = true; }
};

/**
 * The PVS itself.
 */
class csPVS
{
private:
  // Linked list of visible nodes (with polygons attached).
  csOctreeVisible* visible;

public:
  /// Constructor.
  csPVS () : visible (NULL) { }
  /// Destructor.
  ~csPVS ();
  /// Clear the PVS.
  void Clear ();

  /// Add a new visibility struct.
  csOctreeVisible* Add ();

  /// Find the visibility struct for a node.
  csOctreeVisible* FindNode (csOctreeNode* onode);

  /// Delete a visibility struct.
  void Delete (csOctreeVisible* ovis);

  /// Delete a visibility struct.
  void Delete (csOctreeNode* onode)
  {
    csOctreeVisible* ovis = FindNode (onode);
    if (ovis) Delete (ovis);
  }

  /// Get the first visibility struct.
  csOctreeVisible* GetFirst () { return visible; }
  /// Get the next one.
  csOctreeVisible* GetNext (csOctreeVisible* ovis) { return ovis->next; }
};

#endif /*_CS_PVS_H*/

