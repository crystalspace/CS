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

#ifndef __CS_SHADOW_H__
#define __CS_SHADOW_H__

#include "imesh/object.h"
#include "csgeom/box.h"

struct iEngine;
struct iMeshWrapper;

/**
 * BugPlug is the hiding place for many dark creatures. While Spider only
 * places a curse on the country and any unsuspecting wanderer that might
 * visit the country, Shadow takes possession of some creature and clings
 * to it with all its power. Unless Shadow willingly releases the creature
 * there is nothing that can be done to unbind Shadow.
 * The Shadow slowly devours the poor creature from all its knowledge.
 * <p>
 * For the more technically minded:
 * <ul>
 * <li>Shadow: mesh object
 * <li>Creature: another mesh object
 * </ul>
 * This mesh object follows another mesh object and it will render a bounding
 * box for that object.
 */
class csShadow : public iMeshObject
{
private:
  iBase* logparent;
  iMeshWrapper* wrap;
  iMeshWrapper* shadow_mesh;
  bool do_bbox;	// Show bounding box.
  bool do_rad;	// Show bounding sphere.
  bool do_beam; // Show the intersection beam.
  csVector3 beam[2];
  csVector3 isec;

public:
  csShadow ();
  virtual ~csShadow ();

  /**
   * Add Shadow to the engine.
   */
  bool AddToEngine (iEngine* engine);

  /**
   * Remove the shadow from the engine.
   */
  void RemoveFromEngine (iEngine* engine);

  /**
   * Set the mesh that we are shadowing.
   */
  void SetShadowMesh (iMeshWrapper* sh);

  /**
   * Set what we are showing.
   */
  void SetShowOptions (bool bbox, bool rad, bool beam)
  {
    do_bbox = bbox;
    do_rad = rad;
    do_beam = beam;
  }

  /**
   * Get what we are showing.
   */
  void GetShowOptions (bool& bbox, bool& rad, bool& beam) const
  {
    bbox = do_bbox;
    rad = do_rad;
    beam = do_beam;
  }

  void SetBeam (csVector3& start, csVector3& finish, csVector3& intersect)
  {
    beam[0] = start;
    beam[1] = finish;
    isec = intersect;
  }

  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return NULL; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight**, int, iMovable*) { }
  virtual bool Draw (iRenderView*, iMovable*, csZBufMode);
  virtual void SetVisibleCallback (iMeshObjectDrawCallback*) { }
  virtual iMeshObjectDrawCallback* GetVisibleCallback () const { return NULL; }
  virtual void GetObjectBoundingBox (csBox3& bbox, int type = CS_BBOX_NORMAL)
  {
    (void)type;
    bbox.Set (-100000, -100000, -100000, 100000, 100000, 100000);
    return;
  }
  virtual void GetRadius (csVector3& rad, csVector3& cent)
  {
     rad.Set(200000, 200000, 200000);
	 cent.Set(0,0,0);
  }
  virtual void NextFrame (csTicks) { }
  virtual bool WantToDie () const { return false; }
  virtual void HardTransform (const csReversibleTransform&) { }
  virtual bool SupportsHardTransform () const { return false; }
  virtual bool HitBeamOutline (const csVector3&, const csVector3&,
        csVector3&, float*)
  { return false; }
  virtual bool HitBeamObject (const csVector3&, const csVector3&,
  	csVector3&, float*) { return false; }
  virtual long GetShapeNumber () const { return 1; }
  virtual void SetLogicalParent (iBase* lp) { logparent = lp; }
  virtual iBase* GetLogicalParent () const { return logparent; }
  virtual iPolygonMesh* GetWriteObject () { return NULL; }
};

#endif // __CS_SHADOW_H__
