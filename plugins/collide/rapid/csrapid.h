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

#ifndef __CS_RAPID_H__
#define __CS_RAPID_H__

#include "ivaria/collider.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "rapcol.h"

/**
 * RAPID implementation of the collision detection system.
 */
class csRapidCollideSystem : public iCollideSystem
{
private:
  csArray<csIntersectingTriangle> intersecting_triangles;

public:
  SCF_DECLARE_IBASE;

  /// Create the plugin object
  csRapidCollideSystem (iBase *pParent);
  virtual ~csRapidCollideSystem ();

  /// Create an iCollider for the given geometry.
 virtual csPtr<iCollider> CreateCollider (iPolygonMesh*);


  virtual bool Collide (
  	iCollider* collider1, const csReversibleTransform* trans1,
  	iCollider* collider2, const csReversibleTransform* trans2);

  virtual bool CollideRay (
  	iCollider*, const csReversibleTransform*,
	const csVector3&, const csVector3&)
  {
    CS_ASSERT (false);
    return false;
  }
  virtual const csArray<csIntersectingTriangle>& GetIntersectingTriangles ()
  	const
  {
    return intersecting_triangles;
  }

  virtual csCollisionPair* GetCollisionPairs ();
  virtual int GetCollisionPairCount ();
  virtual void ResetCollisionPairs ();

  virtual void SetOneHitOnly (bool o)
  {
    csRapidCollider::SetFirstHit (o);
  }

  /**
   * Return true if this CD system will only return the first hit
   * that is found.
   */
  virtual bool GetOneHitOnly ()
  {
    return csRapidCollider::GetFirstHit ();
  }

  // Debugging functions.
  csPtr<iString> Debug_UnitTest ();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csRapidCollideSystem);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;

  struct DebugHelper : public iDebugHelper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csRapidCollideSystem);
    virtual int GetSupportedTests () const
    {
      return CS_DBGHELP_UNITTEST;
    }
    virtual csPtr<iString> UnitTest ()
    {
      return scfParent->Debug_UnitTest ();
    }
    virtual csPtr<iString> StateTest ()
    {
      return 0;
    }
    virtual csTicks Benchmark (int)
    {
      return 0;
    }
    virtual csPtr<iString> Dump ()
    {
      return 0;
    }
    virtual void Dump (iGraphics3D*)
    {
    }
    virtual bool DebugCommand (const char*)
    {
      return false;
    }
  } scfiDebugHelper;
};

#endif // __CS_RAPID_H__

