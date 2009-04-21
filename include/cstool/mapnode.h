/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2000 by Thomas Hieber

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

#ifndef __CS_MAPNODE_H__
#define __CS_MAPNODE_H__

/**\file
 * Map node
 */

#include "csextern.h"

#include "csgeom/vector3.h"
#include "csutil/csobject.h"
#include "csutil/scf_implementation.h"
#include "ivaria/mapnode.h"

/**
 * A node. This is an iObject that is bound to a position and a sector in
 * the world.
 */
class CS_CRYSTALSPACE_EXPORT csMapNode : 
  public scfImplementationExt1<csMapNode, csObject, iMapNode>
{
public:
  /// The constructor. Requires the Nodes name!
  csMapNode (const char *Name);
  /// The destructor as usual
  virtual ~csMapNode ();

  /// Get a node with the given name and a given classname. (shortcut)
  static iMapNode *GetNode (iSector *pSector, const char *name,
    const char *classname = 0);

  //----------------------- iMapNode --------------------------
  virtual iObject *QueryObject() { return (csObject*)this; }
  virtual void SetPosition (const csVector3& pos) { position = pos; }
  virtual const csVector3& GetPosition () const { return position; }
  virtual void SetXVector (const csVector3& vec) { xvector = vec; }
  virtual const csVector3& GetXVector () const { return xvector; }
  virtual void SetYVector (const csVector3& vec) { yvector = vec; }
  virtual const csVector3& GetYVector () const { return yvector; }
  virtual void SetZVector (const csVector3& vec) { zvector = vec; }
  virtual const csVector3& GetZVector () const { return zvector; }

  virtual void SetSector (iSector *sec);
  virtual iSector *GetSector () const { return sector; }

private:
  iSector *sector;
  csVector3 position;
  csVector3 xvector;
  csVector3 yvector;
  csVector3 zvector;
};

/**
 * A node iterator.
 */
class CS_CRYSTALSPACE_EXPORT csNodeIterator
{
public:
  /**
   * The constructor. Theorectially, we could handle any iObject, but
   * that doesn't make sense for the current implementation, so we
   * restrict it to iSector to avoid some pitfalls.<p>
   *
   * If a classname is given, search is restricted to nodes, in which
   * the key "classname" has the same value as the given classname.
   * the classname string is _not_ duplicated, so the caller is
   * responsible to take care, that the string is available while
   * the Iterator is alive.
   */
  csNodeIterator (iSector *pSector, const char *classname = 0);

  /// The destructor as usual
  ~csNodeIterator ();

  /// Reuse the iterator for an other search
  void Reset (iSector *pSector, const char *classname = 0);
  /// Move forward
  iMapNode* Next ();
  /// Check if there are other nodes
  bool HasNext () const;

protected:
  /// Skip all nodes with wrong classname
  void SkipWrongClassname ();
  /// Step to the next node in the sector, ignoring its classname
  void NextNode ();

  csRef<iObjectIterator> Iterator;
  const char *Classname;
  csRef<iMapNode> CurrentNode;
};

#endif // __CS_MAPNODE_H__
