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

#ifndef _RAPID_H_
#define _RAPID_H_

#include "icollide.h"

class csRAPIDCollider;

/**
 * RAPID version of collider.
 */
class csRapidCollider : public iCollider
{
private:
  /// The mesh.
  iPolygonMesh* mesh;
  /// The RAPID collider.
  csRAPIDCollider* collider;

public:
  /// Constructor.
  csRapidCollider (iPolygonMesh* mesh);

  /// Destructor.
  virtual ~csRapidCollider ();

  /// Get the internal RAPID implementation.
  csRAPIDCollider* GetPrivateCollider () { return collider; }

public:
  ///------------------------ iCollider implementation ------------------------
  DECLARE_IBASE;

  /// Return a reference to the polygon mesh for this collider.
  virtual iPolygonMesh* GetGeometry () { return mesh; }
};

/**
 * RAPID implementation of the collision detection system.
 */
class csRapidCollideSystem : public iCollideSystem
{
public:
  DECLARE_IBASE;

  /// Create the plugin object
  csRapidCollideSystem (iBase *pParent);

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSystem);

  /// Create an iCollider for the given geometry.
  virtual iCollider* CreateCollider (iPolygonMesh* mesh);

  /**
   * Test collision between two colliders.
   * This is only supported for iCollider objects created by
   * this plugin.
   */
  virtual bool Collide (iCollider* collider1, csTransform* trans1,
  	iCollider* collider2, csTransform* trans2);
};

#endif // _RAPID_H_

