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

#include "ivaria/collider.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "rapcol.h"

/**
 * RAPID version of collider.
 */
class csRapidCollider : public iCollider
{
private:
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
  SCF_DECLARE_IBASE;
};

/**
 * RAPID implementation of the collision detection system.
 */
class csRapidCollideSystem : public iCollideSystem
{
public:
  SCF_DECLARE_IBASE;

  /// Create the plugin object
  csRapidCollideSystem (iBase *pParent);

  /// Create an iCollider for the given geometry.
  virtual iCollider* CreateCollider (iPolygonMesh* mesh);

  /**
   * Test collision between two colliders.
   * This is only supported for iCollider objects created by
   * this plugin.
   */
  virtual bool Collide (
  	iCollider* collider1, const csReversibleTransform* trans1,
  	iCollider* collider2, const csReversibleTransform* trans2);

  /**
   * Get pointer to current array of collision pairs.
   * This array will grow with every call to Collide until you clear
   * it using 'ResetCollisionPairs'.
   */
  virtual csCollisionPair* GetCollisionPairs ();

  /**
   * Get number of collision pairs in array.
   */
  virtual int GetCollisionPairCount ();

  /**
   * Reset the array with collision pairs.
   */
  virtual void ResetCollisionPairs ();

  /**
   * Indicate if we are interested only in the first hit that is found.
   */
  virtual void SetOneHitOnly (bool o)
  {
    csRAPIDCollider::SetFirstHit (o);
  }

  /**
   * Return true if this CD system will only return the first hit
   * that is found.
   */
  virtual bool GetOneHitOnly ()
  {
    return csRAPIDCollider::GetFirstHit ();
  }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRapidCollideSystem);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;
};

#endif // _RAPID_H_

