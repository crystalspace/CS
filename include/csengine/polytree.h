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

#ifndef __CS_POLYTREE_H__
#define __CS_POLYTREE_H__

#include "csgeom/math3d.h"
#include "csgeom/plane3.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csengine/arrays.h"

class csThing;
class csPolygonInt;
class csPolygonTree;
class csPolygonTreeNode;
class csPolygonStub;
class csPolyTreeBBox;
class Dumper;
struct iFile;


#define NODE_OCTREE 1
#define NODE_BSPTREE 2

/**
 * Visit a node in a polygon tree. If this function returns non-NULL
 * the scanning will stop and the pointer will be returned.
 * The 'same_plane' bool will be true if all polygons are on the same
 * plane.
 */
typedef void* (csTreeVisitFunc)(csThing*, csPolygonInt**,
	int num, bool same_plane, void*);

/**
 * Potentially cull a node from the tree just before it would otherwise
 * have been traversed in Back2Front() or Front2Back().
 * If this function returns true then the node is potentially visible.
 */
typedef bool (csTreeCullFunc)(csPolygonTree* tree, csPolygonTreeNode* node,
	const csVector3& pos, void* data);

/**
 * A general node in a polygon tree.
 */
class csPolygonTreeNode
{
  friend class Dumper;

protected:
  /**
   * A linked list for all stubs that are added
   * to this node. These stubs represents parts of an object
   * that is located inside this node.
   */
  csPolygonStub* first_stub;

  /**
   * A linked list of all polygons stubs that still need to
   * be processed whenever this node becomse visible.
   */
  csPolygonStub* todo_stubs;

public:
  /**
   * Constructor.
   */
  csPolygonTreeNode () : first_stub (NULL), todo_stubs (NULL) { }

  /**
   * Destructor.
   */
  virtual ~csPolygonTreeNode ();

  /// Return true if node is empty.
  virtual bool IsEmpty () = 0;

  /// Return type (NODE_???).
  virtual int Type () = 0;

  /**
   * Unlink a stub from the stub list.
   * Warning! This function does not test if the stub
   * is really on the list!
   */
  void UnlinkStub (csPolygonStub* ps);

  /**
   * Link a stub to the todo list.
   */
  void LinkStubTodo (csPolygonStub* ps);

  /**
   * Link a stub to the stub list.
   */
  void LinkStub (csPolygonStub* ps);

  /**
   * Traverse all the polygons in the dynamic objects
   * added to this node.
   */
  void* TraverseObjects (csThing* thing, const csVector3& pos,
  	csTreeVisitFunc* func, void* data);
};

/**
 * A general polygon tree. This is an abstract data type.
 * Concrete implementations like csBspTree or csOctree inherit
 * from this class.
 */
class csPolygonTree
{
  friend class Dumper;

protected:
  /// The root of the tree.
  csPolygonTreeNode* root;

  /// The parent thing that this tree is made for.
  csThing* thing;

  /// Clear the nodes.
  void Clear () { delete root; }

  // Various routines to write to an iFile. Used by 'Cache'.
  void WriteString (iFile* cf, char* str, int len);
  void WriteBox3 (iFile* cf, const csBox3& box);
  void WriteVector3 (iFile* cf, const csVector3& v);
  void WritePlane3 (iFile* cf, const csPlane3& v);
  void WriteLong (iFile* cf, long l);
  void WriteUShort (iFile* cf, UShort l);
  void WriteByte (iFile* cf, unsigned char b);
  void WriteBool (iFile* cf, bool b);

  // Various routines to write from an iFile. Used by 'ReadFromCache'.
  void ReadString (iFile* cf, char* str, int len);
  void ReadBox3 (iFile* cf, csBox3& box);
  void ReadVector3 (iFile* cf, csVector3& v);
  void ReadPlane3 (iFile* cf, csPlane3& v);
  long ReadLong (iFile* cf);
  UShort ReadUShort (iFile* cf);
  unsigned char ReadByte (iFile* cf);
  bool ReadBool (iFile* cf);

public:
  /**
   * Constructor.
   */
  csPolygonTree (csThing* th) : root (NULL), thing (th) { }

  /**
   * Destructor.
   */
  virtual ~csPolygonTree () { }

  /// Get the thing for this tree.
  csThing* GetThing () { return thing; }

  /**
   * Create the tree with a given set of polygons.
   */
  virtual void Build (csPolygonInt** polygons, int num) = 0;

  /**
   * Create the tree with a given set of polygons.
   */
  void Build (csPolygonArray& polygons)
  {
    Build (polygons.GetArray (), polygons.Length ());
  }

  /**
   * Add a dynamic object to the tree.
   */
  void AddObject (csPolyTreeBBox* obj);

  /**
   * Add a stub to the todo list of the tree.
   */
  void AddStubTodo (csPolygonStub* stub)
  {
    root->LinkStubTodo (stub);
  }

  /**
   * Test if any polygon in the list overlaps any other polygon.
   * If this function returns false we have convexity.
   */
  bool Overlaps (csPolygonInt** polygons, int num);

  /// Traverse the tree from back to front starting at the root and 'pos'.
  virtual void* Back2Front (const csVector3& pos, csTreeVisitFunc* func,
  	void* data, csTreeCullFunc* cullfunc = NULL, void* culldata = NULL) = 0;
  /// Traverse the tree from front to back starting at the root and 'pos'.
  virtual void* Front2Back (const csVector3& pos, csTreeVisitFunc* func,
  	void* data, csTreeCullFunc* cullfunc = NULL, void* culldata = NULL) = 0;

  /// Print statistics about this tree.
  virtual void Statistics () = 0;

  /**
   * Classify a point with respect to this tree.
   * Return true if the point is in solid space or false otherwise.
   * This routine is not exact. In badly formed worlds it is possible
   * that it will generate a bad result (i.e. say solid if it isn't
   * solid space).
   */
  bool ClassifyPoint (const csVector3& p);
};

#endif // __CS_POLYTREE_H__
