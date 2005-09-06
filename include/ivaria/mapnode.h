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

#ifndef __CS_IVARIA_MAPNODE_H__
#define __CS_IVARIA_MAPNODE_H__

#include "csutil/scf.h"

struct iObject;
struct iSector;

/**
 * A node. This is an iObject that is bound to a position and a sector in
 * the world. Nodes are typically created from a map file using the \<node\>
 * attribute.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>The main loader creates instances of this internally.
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() from iObject instances you get by
 *       calling iObject::GetIterator() on iObject instances you
 *       get from iSector::QueryObject().
 *   </ul>
 */
struct iMapNode : public virtual iBase
{
  SCF_INTERFACE(iMapNode, 2, 0, 0);

  /// Get the iObject.
  virtual iObject *QueryObject() = 0;

  /// Set the position of the node
  virtual void SetPosition (const csVector3& pos) = 0;
  /// Get the position of the node
  virtual const csVector3& GetPosition () const = 0;

  /// Set the x vector of the node
  virtual void SetXVector (const csVector3& vec) = 0;
  /// Get the x vector of the node
  virtual const csVector3& GetXVector () const = 0;
  /// Set the y vector of the node
  virtual void SetYVector (const csVector3& vec) = 0;
  /// Get the y vector of the node
  virtual const csVector3& GetYVector () const = 0;
  /// Set the z vector of the node
  virtual void SetZVector (const csVector3& vec) = 0;
  /// Get the z vector of the node
  virtual const csVector3& GetZVector () const = 0;

  /// Set the sector of the node
  virtual void SetSector (iSector *pSector) = 0;
  /// Get the sector of the node
  virtual iSector *GetSector () const = 0;
};

#endif // __CS_IVARIA_MAPNODE_H__
