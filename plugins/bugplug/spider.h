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

#ifndef __CS_SPIDER_H__
#define __CS_SPIDER_H__

#include "imesh/object.h"
#include "csgeom/box.h"

struct iEngine;
struct iCamera;
struct iMeshWrapper;

/**
 * Beware! In the dark recesses of BugPlug there lives a creature so awful
 * and fierce that even the mightiest heroes don't dare to look at it! When
 * the sun shines over the mountains and fields, everything is well and
 * Spider remains hidden in its dark cave. However, when the moonlight is
 * dim or shaded, and nasty bugs crawl over the country, Spider will weave
 * its web of deceit over the infected land! Unaware of the great danger, an
 * unsuspecting wanderer might look over the country. Here Spider will
 * strike! It will put a curse on the wanderer which will reveal his
 * whereabouts to Spider at any time in the future! The only remedy to this
 * curse is death!
 * <p>
 * For the more technically minded:
 * <ul>
 * <li>Spider: mesh object
 * <li>Mountains and fields: sectors
 * <li>Bugs: bugs :-)
 * <li>Wanderer: camera
 * </ul>
 */
class csSpider : public iMeshObject
{
private:
  iBase* logparent;
  iCamera* camera;
  iMeshWrapper* wrap;

public:
  csSpider ();
  virtual ~csSpider ();

  /// Get the camera that Spider catched.
  iCamera* GetCamera () { return camera; }

  /// Clear the camera so that Spider can catch a new one.
  void ClearCamera () { camera = NULL; }

  /**
   * Weave the web: i.e. distribute Spider over all engine sectors.
   * Will return false if this could not be done (i.e. no sectors)
   */
  bool WeaveWeb (iEngine* engine);

  /// Unweave the web.
  void UnweaveWeb (iEngine* engine);

  SCF_DECLARE_IBASE;

  virtual iMeshObjectFactory* GetFactory () const { return NULL; }
  virtual bool DrawTest (iRenderView* rview, iMovable* movable);
  virtual void UpdateLighting (iLight**, int, iMovable*) { }
  virtual bool Draw (iRenderView*, iMovable*, csZBufMode) { return false; }
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

#endif // __CS_SPIDER_H__
