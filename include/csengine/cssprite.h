/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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

#ifndef __CS_CSSPRITE_H__
#define __CS_CSSPRITE_H__

#include "csutil/cscolor.h"
#include "csutil/csvector.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "csengine/polyint.h"
#include "csengine/bspbbox.h"
#include "csengine/rview.h"
#include "csengine/material.h"
#include "csengine/tranman.h"
#include "csengine/triangle.h"
#include "csengine/movable.h"
#include "igraph3d.h"
#include "ipolmesh.h"
#include "imspr3d.h"
#include "imater.h"

class Dumper;
class csMaterialList;
class csMaterialWrapper;
class csLightHitsSprite;
class csSkeleton;
class csSkeletonState;
class csSprite3D;
class csBspContainer;
class csRandomGen;
struct iTextureHandle;

/// A callback function for csSprite3D::Draw().
typedef void (csSpriteCallback) (csSprite3D* spr, csRenderView* rview);
/// A callback function for csSprite3D::Draw().
typedef void (csSpriteCallback2)
  (csSprite3D* spr, csRenderView* rview, csObject *callbackData);

/**
 * The base class for all types of sprites.
 */
class csSprite : public csPObject
{
  friend class Dumper;
  friend class csMovable;

protected:
  /// Points to Actor class which "owns" this sprite.
  csObject* myOwner;

  /**
   * Points to the parent container object of this sprite.
   * This is usually csEngine or csParticleSystem.
   */
  csObject* parent;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current cookie from the
   * transformation manager to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;

  /// Current cookie for camera_bbox.
  csTranCookie camera_cookie;

  /// Mixmode for the triangles/polygons of the sprite.
  UInt MixMode;

  /// Defered lighting. If > 0 then we have defered lighting.
  int defered_num_lights;

  /// Flags to use for defered lighting.
  int defered_lighting_flags;

  /// The callback which is called just before drawing.
  csSpriteCallback* draw_callback;

  /// This callback is only called if the sprite is actually drawn.
  csSpriteCallback2* draw_callback2;

  /**
   * Flag which is set to true when the sprite is visible.
   * This is used by the c-buffer/bsp routines. The sprite itself
   * will not use this flag in any way at all. It is simply intended
   * for external visibility culling routines.
   */
  bool is_visible;

  /**
   * Pointer to the object to place in the polygon tree.
   */
  csPolyTreeObject* ptree_obj;

  /**
   * Update this sprite in the polygon trees.
   */
  virtual void UpdateInPolygonTrees () = 0;

  /// Update defered lighting.
  void UpdateDeferedLighting (const csVector3& pos);

protected:
  /**
   * Position in the world.
   */
  csMovable movable;

  /// Move this sprite to the specified sector. Can be called multiple times.
  virtual void MoveToSector (csSector* s);

  /// Remove this sprite from all sectors it is in (but not from the engine).
  virtual void RemoveFromSectors ();

  /**
   * Update transformations after the sprite has moved
   * (through updating the movable instance).
   * This MUST be done after you change the movable otherwise
   * some of the internal data structures will not be updated
   * correctly. This function is called by movable.UpdateMove();
   */
  virtual void UpdateMove ();

public:
  /// Constructor.
  csSprite (csObject* theParent);
  /// Destructor.
  virtual ~csSprite ();

  /// Set owner (actor) for this sprite.
  void SetMyOwner (csObject* newOwner) { myOwner = newOwner; }
  /// Get owner (actor) for this sprite.
  csObject* GetMyOwner () { return myOwner; }

  /// Set parent container for this sprite.
  void SetParentContainer (csObject* newParent) { parent = newParent; }
  /// Get parent container for this sprite.
  csObject* GetParentContainer () { return parent; }

  /// Get the pointer to the object to place in the polygon tree.
  csPolyTreeObject* GetPolyTreeObject ()
  {
    return ptree_obj;
  }

  /**
   * Do some initialization needed for visibility testing.
   * i.e. clear camera transformation.
   */
  virtual void VisTestReset () { }

  /// Mark this sprite as visible.
  void MarkVisible () { is_visible = true; }

  /// Mark this sprite as invisible.
  void MarkInvisible () { is_visible = false; }

  /// Return if this sprite is visible.
  bool IsVisible () { return is_visible; }

  /**
   * Update lighting as soon as the sprite becomes visible.
   * This will call engine->GetNearestLights with the supplied
   * parameters.
   */
  virtual void DeferUpdateLighting (int flags, int num_lights);

  /// Sets the mode that is used, when drawing that sprite.
  virtual void SetMixmode (UInt m) { MixMode = m; }

  /// Gets the mode that is used, when drawing that sprite.
  virtual UInt GetMixmode () { return MixMode; }

  /**
   * Set a callback which is called just before the sprite is drawn.
   * This is useful to do some expensive computations which only need
   * to be done on a visible sprite.
   */
  void SetDrawCallback (csSpriteCallback* callback)
  { draw_callback = callback; }

  /**
   * Set a callback which is called only if the sprite is actually drawn.
   */
  void SetDrawCallback2 (csSpriteCallback2* callback)
  { draw_callback2 = callback; }

  /**
   * Get the draw callback. If there are multiple draw callbacks you can
   * use this function to chain.
   */
  csSpriteCallback* GetDrawCallback () { return draw_callback; }

  /**
   * Get the draw callback. If there are multiple draw callbacks you can
   * use this function to chain.
   */
  csSpriteCallback2* GetDrawCallback2 () { return draw_callback2; }

  /**
   * Light sprite according to the given array of lights (i.e.
   * fill the vertex color array).
   * No shadow calculation will be done. This is assumed to have
   * been done earlier. This is a primitive lighting process
   * based on the lights which hit one point of the sprite (usually
   * the center). More elaborate lighting systems are possible
   * but this will do for now.
   */
  virtual void UpdateLighting (csLight** lights, int num_lights) = 0;

  /**
   * Draw this sprite given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  virtual void Draw (csRenderView& rview) = 0;

  /**
   * Control animation given the current time.
   */
  virtual void NextFrame (cs_time current_time) = 0;

  /**
   * Get the movable instance for this sprite.
   * It is very important to call GetMovable().UpdateMove()
   * after doing any kind of modification to this movable
   * to make sure that internal data structures are
   * correctly updated.
   */
  csMovable& GetMovable () { return movable; }

  /**
   * Check if this sprite is hit by this object space vector.
   * Return the collision point in object space coordinates.
   */
  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr) = 0;
  /**
   * Check if this sprite is hit by this world space vector.
   * Return the collision point in world space coordinates.
   */
  bool HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect, float* pr);

  /**
   * Rotate sprite in some manner in radians.
   * This function operates by rotating the movable transform.
   */
  void Rotate (float angle);

  /**
   * Scale sprite by this factor.
   * This function operates by scaling the movable transform.
   */
  void ScaleBy (float factor);

  /// Returns true if this object wants to die.
  virtual bool WantToDie () { return false; }

  CSOBJTYPE;
};

#endif // __CS_CSSPRITE_H__
