/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Alex Pfaffe.

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
#ifndef __CS_COLLIDER_H__
#define __CS_COLLIDER_H__

#include "csobject/pobject.h"
#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"

class csTransform;

/// Abstract class for low level collision detection
class csCollider : public csPObject
{
protected:
  
  /// If true this is an active collision object.
  bool m_CollisionDetectionActive;
  
public:

  /// Create the collider object. The parent of the collider will be null.
  csCollider ();

  /// Create the collider object. Sets parent as parent of the collider,
  /// and sets the collider as child of parent.
  csCollider (csObject &parent);
  
  /// Destroy the collider object.
  virtual ~csCollider () {}

  /// Enable collision detection for this object.
  void Activate (bool on) {m_CollisionDetectionActive = on;}

  /// Check if this collider collides with otherCollider.
  /// Returns true if collision detected and adds the pair to the collisions
  /// hists vector.
  /// This collider and otherCollider must be of comparable subclasses, if
  /// not false is returned.
  virtual bool Collide (csCollider &otherCollider,
                        csTransform *pThisTransform = 0,
                        csTransform *pOtherTransform = 0) = 0;
  /// Similar to Collide for csCollider.
  virtual bool Collide (csObject &otherObject,
                        csTransform *pThisTransform = 0,
                        csTransform *pOtherTransform = 0) = 0;
  
  CSOBJTYPE;
};

class csCdModel;
class csCdBBox;
class csPolygonSet;
class csSprite3D;
struct csCdTriangle;
struct collision_pair;

/// Low level collision detection using the RAPID algorithm
class csRAPIDCollider : public csCollider {
  friend class csCdBBox;
  friend class csBeing;

  /// The internal collision object.
  csCdModel* m_pCollisionModel;

  /**
   * Find the first collision that involved this object, and return who
   * it was.  This call does not alter queue state.  Return 0 if no
   * collision involving this object occurred.
   * Optionally return the triangles involved in the collision.
   */
  csRAPIDCollider* FindCollision (csCdTriangle **tr1 = 0, csCdTriangle **tr2 = 0);
  /// Get top level bounding box.
  const csCdBBox* GetBbox(void) const;

  /// Delete and free memory of this objects oriented bounding box.
  void DestroyBbox ();

  /// Recursively test collisions of bounding boxes.
  static int CollideRecursive (csCdBBox *b1, csCdBBox *b2, csMatrix3 R, csVector3 T);
  /**
    * Global variables
    * Matrix, and Vector used for collision testing.
    */
  static csMatrix3 mR;
  static csVector3 mT;
  /// Statistics, to allow early bailout.
  /// If the number of triangles tested is too high the BBox structure
  /// probably isn't very good.
  static int trianglesTested;		// TEMPORARY.
  /// The number of boxes tested.
  static int boxesTested;		// TEMPORARY.
  /// If bbox is less than this size, dont bother testing further,
  /// just return with the results so far.
  static float minBBoxDiam;
  /// Number of levels to test.
  static int testLevel;
  /// Test only up to the 1st hit.
  static bool firstHit;

  void PolygonInitialize(csPolygonSet *ps);
  void Sprite3DInitialize(csSprite3D *sp);
  
public:

  static int numHits;
  
  /// Create a collider based on a csPolygonSet.
  csRAPIDCollider (csPolygonSet *ps);
  csRAPIDCollider (csObject &parent, csPolygonSet *ps);
  /// Create a collider based on a Sprite3D.
  csRAPIDCollider (csSprite3D *sp);
  csRAPIDCollider (csObject &parent, csSprite3D *sp);

  /// Destroy the RAPID collider object
  virtual ~csRAPIDCollider();

  /// Check if this collider collides with pOtherCollider.
  /// Returns true if collision detected and adds the pair to the collisions
  /// hists vector.
  /// This collider and pOtherCollider must be of comparable subclasses, if
  /// not false is returned.
  virtual bool Collide (csCollider &pOtherCollider,
                        csTransform *pThisTransform = NULL,
                        csTransform *pOtherTransform = NULL);
  /// Similar to Collide for csCollider. Calls GetCollider for otherCollider.
  virtual bool Collide (csObject &otherObject,
                        csTransform *pThisTransform = 0,
                        csTransform *pOtherTransform = 0);

  /// Query the array with collisions (and their count)
  static collision_pair *GetCollisions ();

  static void CollideReset ();
  static void SetFirstHit (bool fh) {firstHit = fh;}
  static bool GetFirstHit () {return firstHit;}
  static int Report (csCollider **id1, csCollider **id2);
  const csVector3 &GetRadius() const;
  /// If object has a child of type csCollider it is returned. Otherwise 0
  /// is returned.
  static csRAPIDCollider *GetRAPIDCollider(csObject &object);
  
  CSOBJTYPE;
};


#endif // __CS_COLLIDER_H__
