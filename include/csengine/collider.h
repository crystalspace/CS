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
#ifndef COLLIDER_H
#define COLLIDER_H

#include "csengine/polyset.h"
#include "csengine/cssprite.h"
#include "csengine/thing.h"
#include "csengine/polygon.h"
#include "csengine/world.h"
#include "csengine/camera.h"
#include "csengine/sector.h"

struct CDTriangle
{
  int id;
  csVector3 p1, p2, p3;
};

//
class BBox
{
public:

  // placement in parent's space
  // box to parent space: x_m = pR*x_b + pT
  // parent to box space: x_b = pR.T()*(x_m - pT)
  csMatrix3 pR;
  csVector3 pT;

  // this is "radius", that is, half the measure of a side length
  csVector3 d;
  // points to but does not "own".  
  BBox *P;
  BBox *N;

  CDTriangle *trp;

  BBox () : pT (0, 0, 0), d (0, 0, 0) { }

  int leaf ()
  { return (!P && !N); } 
  float size ()
  { return d.x; } 

  bool split_recurse(int *t, int n, BBox *&box_pool, CDTriangle *tris);
  // specialized for leaf nodes
  bool split_recurse(int *t, CDTriangle *tris);
  //
  static bool tri_contact (BBox *b1, BBox *b2);
};

class CD_model
{
public:

  // these are only for internal use
  BBox *b;
  int num_boxes_alloced;

  CDTriangle *tris;
  int num_tris;
  int num_tris_alloced;
  
  bool build_hierarchy();

public:

  /// Create a model object given number of triangles
  CD_model (int n_triangles);

  /// Free the memory allocated for this model
  ~CD_model ();

  /// Add a triangle to the model
  bool AddTriangle (int id, const csVector3 &p1, const csVector3 &p2, const csVector3 &p3);
};

/****************************************************************************/

// this is the collision query invocation.  It assumes that the 
// models are not being scaled up or down, but have their native
// dimensions.

// this is for the client
struct collision_pair
{
  CDTriangle *tr1;
  CDTriangle *tr2;
};

/****************************************************************************/

///
class csCollider
{
  /// If true this is an active collision object.
  int _cd_state;
  /// The internal collision object.
  CD_model *_rm;

public:
  typedef enum { POLYGONSET, SPRITE3D } ColliderType;
  ColliderType _type;
  union {
    csPolygonSet *_ps;
    csSprite3D *_sp;
  };

  /// Global variables
  /// Matrix, and Vector used for collision testing.
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
  /// Number of hits recorded.
  static int numHits;
  /// Create a collider based on a csPolygonSet.
  csCollider (csPolygonSet *ps);
  /// Create a collider based on a Sprite3D.
  csCollider (csSprite3D *sp);

  /// Destroy the collider object.
  virtual ~csCollider ();

  /// Enable collision detection for this object.
  void Activate (bool on);

  /// Get objects name.
  const char* GetName ();

  /// Delete and free memory of this objects oriented bounding box.
  void DestroyBbox ();

  /// Recursively test collisions of bounding boxes.
  static int CollideRecursive (BBox *b1, BBox *b2, csMatrix3 R, csVector3 T);

  /// Reset the collision hits vector.
  static void CollideReset ();

  /// Test collision detection between two objects.
  static int CollidePair (csCollider *c1, csCollider *c2, csTransform *t1 =0, csTransform *t2 = 0);

  /// Get the next collision from the queue.  Removes collision from queue.
  static int Report (csCollider **id1, csCollider **id2);
  /**
   * Find the first collision that involved this object, and return who
   * it was.  This call does not alter queue state.  Return 0 if no
   * collision involving this object occurred.
   * Optionally return the triangles involved in the collision.
   */
  csCollider* FindCollision (CDTriangle **tr1 = 0, CDTriangle **tr2 = 0);

  /// Get top level bounding box.
  BBox * GetBbox(void) { return _rm->b; }

  /// Query the array with collisions (and their count)
  static collision_pair *GetCollisions ();
};

// Classes to organize triangles in bounding boxes with.
class Moment
{
public:
  float A;	// Area of triangle.
  csVector3 m;	// Centriod.
  csMatrix3 s;	// Moment.
  inline void mean( csVector3 *v ) { *v = m; }
  static Moment *stack;

  inline void compute(csVector3 p, csVector3 q, csVector3 r)
  {
    csVector3 u, v, w;

    // compute the area of the triangle
    u = q - p;
    v = r - p;
    w = u % v; 

    if (fabs(w.x)+fabs(w.y)+fabs(w.z) > SMALL_EPSILON)
        A = 0.5 * w.Norm();
    else
        A = 0.0;

    // centroid
    m = (p + q + r) /3;

    if (A == 0.0)
    {
      // This triangle has zero area.  The second order components
      // would be eliminated with the usual formula, so, for the 
      // sake of robustness we use an alternative form.  These are the 
      // centroid and second-order components of the triangle's vertices.

      // second-order components
      s.m11 = (p.x*p.x + q.x*q.x + r.x*r.x);
      s.m12 = (p.x*p.y + q.x*q.y + r.x*r.y);
      s.m13 = (p.x*p.z + q.x*q.z + r.x*r.z);
      s.m22 = (p.y*p.y + q.y*q.y + r.y*r.y);
      s.m23 = (p.y*p.z + q.y*q.z + r.y*r.z);
      s.m33 = (p.z*p.z + q.z*q.z + r.z*r.z);      
    }
    else
    {
      // get the second order components weighted by the area
      s.m11 = A*(9*m.x*m.x+p.x*p.x+q.x*q.x+r.x*r.x)/12;
      s.m12 = A*(9*m.x*m.y+p.x*p.y+q.x*q.y+r.x*r.y)/12;
      s.m22 = A*(9*m.y*m.y+p.y*p.y+q.y*q.y+r.y*r.y)/12;
      s.m13 = A*(9*m.x*m.z+p.x*p.z+q.x*q.z+r.x*r.z)/12;
      s.m23 = A*(9*m.y*m.z+p.y*p.z+q.y*q.z+r.y*r.z)/12;
      s.m33 = A*(9*m.z*m.z+p.z*p.z+q.z*q.z+r.z*r.z)/12;
    }
    s.m32 = s.m23;
    s.m21 = s.m12;
    s.m31 = s.m13;
  } 
};

class Accum : public Moment
{
public:
  inline void clear ()
  {
    A = m.x = m.y = m.z = 0;
    s.m11 = s.m12 = s.m13 = 0;
    s.m21 = s.m22 = s.m23 = 0;
    s.m31 = s.m32 = s.m33 = 0;
  }
  inline void moment( Moment b )
  { m = m + b.m * b.A; s = s + b.s;  A += b.A; }
  inline void mean( csVector3 *v )
  { *v = m / A; }
  inline void covariance( csMatrix3 *C )
  {
    C->m11 = s.m11 - m.x*m.x/A;
    C->m21 = s.m21 - m.y*m.x/A;
    C->m31 = s.m31 - m.z*m.x/A;
    C->m12 = s.m12 - m.x*m.y/A;
    C->m22 = s.m22 - m.y*m.y/A;
    C->m32 = s.m32 - m.z*m.y/A;
    C->m13 = s.m13 - m.x*m.z/A;
    C->m23 = s.m23 - m.y*m.z/A;
    C->m33 = s.m33 - m.z*m.z/A;
  }
  inline void moments(int *t, int n)
  {
    clear ();
    for (int i = 0; i < n; i++)
      moment (Moment::stack [t [i]]);
  }
};

#endif
