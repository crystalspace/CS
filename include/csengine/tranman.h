/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __CS_TRANMAN_H__
#define __CS_TRANMAN_H__

#include "csutil/scf.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/poly3d.h"
#include "itranman.h"

class csTransformationManager;
class csTransformedSet;

/**
 * An array of vertices.
 */
class csVertexArray : public csVector3Array
{
  friend class csTransformationManager;
  friend class csTransformedSet;

private:
  /// Next in the pool of the transformation manager.
  csVertexArray* next;
  /// Next in the linked list of the transformed set.
  csVertexArray* next_set;
  /// Cookie used when generating this set.
  csTranCookie cookie;

public:
  /// Construct an empty vertex array.
  csVertexArray () { cookie = 0; }

  /// Destruct.
  ~csVertexArray () { }
};

/**
 * The transformation manager is responsible for maintaining all vertex
 * arrays of camera space coordinates for things, sectors, .... Every such
 * entity has a csTransformedSet instance which holds a csVertexArray
 * as given by this manager. The transformation manager also
 * has a pool of csVertexArray's that it will give to the csTransformedSet
 * instances that are being used and it will also keep track of a global
 * transformation cookie so that we can detect when a csTransformedSet
 * needs to be updated with a new csVertexArray. Every csTransformedSet has
 * internal information which enables it to do that.<p>
 *
 * We define two states in the session manager:<br>
 * <ul>
 * <li>Frame: this is everything that goes on during drawing of one frame.
 * <li>Camera Frame: this is part of a frame which depends on camera
 *     configuration. There is always at least one Camera Frame for every
 *     Frame but space warping portals will add more.
 * </ul><br>
 * Cookies for camera frames are guaranteed to be larger than the current frame
 * cookie. Also they are guaranteed to be lower than the next frame cookie. This
 * can be used to detect if you have data relevant for this frame or obsoleted
 * because we are in a new frame.
 */
class csTransformationManager : public iBase
{
private:
  /// Pool of free vertex arrays.
  csVertexArray* freed;
  /// Pool of allocated vertex arrays (currently in use).
  csVertexArray* alloced;
  /// Last allocated vertex array.
  csVertexArray* last_alloced;
  /// Current cookie.
  csTranCookie cookie;
  /// Maximum frame/camera frame cookie used upto now.
  csTranCookie max_cookie;
  /// Current frame cookie. This cookie indicates the start of this frame.
  csTranCookie frame_cookie;

public:
  /// Construct the manager.
  csTransformationManager ();
  /// Destruct.
  virtual ~csTransformationManager ();

  /// Initialize (useful when the engine has been cleared).
  void Initialize ();

  /**
   * Return a new vertex array. Note that it is not possible
   * to free arrays. They are automatically freed every frame.
   */
  csVertexArray* Alloc ()
  {
    csVertexArray* f;
    if (freed)
    {
      f = freed;
      freed = freed->next;
    }
    else
    {
      f = new csVertexArray ();
    }
    f->next = alloced;
    if (!alloced) last_alloced = f;
    alloced = f;
    return f;
  }

  /// Start a new frame.
  void NewFrame ();

  /**
   * Start a new camera frame (i.e. a sub-frame within a frame).
   * This is used when going through a space-warping portal for example.
   * The cookie returned by this function can be given by RestoreCameraFrame()
   * when coming back from the portal.
   */
  csTranCookie NewCameraFrame ();

  /**
   * Restore a camera frame (within the same frame).
   */
  void RestoreCameraFrame (csTranCookie prev_cookie);

  /// Get the current cookie.
  csTranCookie GetCookie () { return cookie; }

  /// Get the current frame cookie.
  csTranCookie GetFrameCookie () { return frame_cookie; }

  /**
   * Given a cookie number (camera frame cookie), this function will
   * test if the frame has changed.
   */
  bool HasFrameChanged (csTranCookie test_cookie)
  {
    return test_cookie < frame_cookie;
  }

  DECLARE_IBASE;

  //--------------------- iTransformationManager implementation --------------------//
  struct TransformationManager : public iTransformationManager
  {
    DECLARE_EMBEDDED_IBASE (csTransformationManager);
    virtual csTranCookie GetCookie () { return scfParent->GetCookie (); }
  } scfiTransformationManager;
};

/**
 * Every entity that is interested in having a set of transformed
 * camera vertices has to maintain an instance of this object.
 * It communicates with the transformation manager.
 */
class csTransformedSet
{
private:
  /// The transformation manager.
  csTransformationManager* tr_manager;
  /**
   * Last cookie used by this set. We use this to detect
   * if the frame has changed.
   */
  csTranCookie last_cookie;
  /// The vertex array set.
  csVertexArray* vertex_arrays;
  /// The current verex array.
  csVertexArray* current_array;

public:
  /// Constructor.
  csTransformedSet ();
  /// Constructor.
  csTransformedSet (csTransformationManager* manager);
  /// Destructor.
  ~csTransformedSet ();
  /// Set the transformation manager (required).
  void SetTransformationManager (csTransformationManager* manager)
  { tr_manager = manager; }

  /**
   * Get the current vertex array. This function does not check
   * if the array is indeed current.
   */
  csVertexArray* GetVertexArray () { return current_array; }

  /**
   * Check and update the vertex array if needed.
   */
  void Update ();

  /**
   * Same as Update() but a bit more loose. It will assume that
   * no new array will be created but there is one present which is ok.
   */
  void CheckUpdate ()
  {
    if (current_array->cookie != tr_manager->GetCookie ()) Update ();
  }

  /**
   * Transform (if needed). This function can safely be called
   * multiple times during a session. The session manager will only
   * really redo the transformation if needed.
   */
  void Transform (csVector3* wor_verts, int num_vertices,
  	const csTransform& w2c);

  /**
   * Translate (if needed). This function can safely be called
   * multiple times during a session. The session manager will only
   * really redo the transformation if needed.
   */
  void Translate (csVector3* wor_verts, int num_vertices,
  	const csVector3& trans);
};

#endif // __CS_TRANMAN_H__
