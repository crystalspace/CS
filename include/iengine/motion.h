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

#ifndef __IENGINE_MOTION_H__
#define __IENGINE_MOTION_H__

#include "csutil/scf.h"
#include "iengine/skelbone.h"

class csMatrix3;
class csVector3;
class csQuaternion;
class csTransform;

SCF_VERSION (iMotionTemplate, 0, 10, 0);

/**
 * Interface to loading motion data.
 * This represents a motion that can be used on a skeleton
 * via the iMotionController interface.
 */
struct iMotionTemplate : public iBase
{
  /// Get the name of this motion.
  virtual const char* GetName () = 0;
  /// Set the length of this motion.
  virtual void SetDuration (float duration) = 0;
  /**
   * Set the default number of times to looping this motion.
   * Setting this value to -1 will loop forever.
   * Setting this value to 0 will disable looping.
   */
  virtual void SetLoopCount (int looping) = 0;
  /**
   * This setting controls whether the motion
   * will be played reversed after each loop.
   */
  virtual void SetLoopFlip (bool enable) = 0;
  /**
   * Add the bone name to the list of handled bones.
   * Returns the index of the into the array.
   */
  virtual int AddBone (const char* name) = 0;
  /// Find a bone index by name.
  virtual int FindBoneByName (const char* name) = 0;
  /// Add a keyframe to a handled bone by the bones index.
  virtual void AddFrameBone (int boneid, float frametime, const csVector3 &position, const csQuaternion &rotation) = 0;
};

SCF_VERSION (iMotionController, 0, 10, 0);

/**
 * Interface for setting motion parameters on a skeleton.
 * This is the structure that actually does the animation on
 * the skeleton.
 */
struct iMotionController : public iBase
{
  /// Return the Skeleton I control
  virtual iSkeletonBone* GetSkeleton() = 0;
  /// Set the motion stack to just this motion
  virtual void SetMotion(iMotionTemplate *motion) = 0;
  /// Blend another motion on the motion stack
  virtual void BlendMotion(iMotionTemplate *motion) = 0;
  /// Pause or Unpause this motion.
  virtual void Pause(bool enable) = 0;

// All sorts of various features such as loop on/off, per character timescaling, etc
};

SCF_VERSION (iMotionManager, 0, 10, 0);

/**
 * Engine for managing MotionTemplates and MotionControllers.
 * It keeps track of the loaded motions, the controllers which
 * bind the motions to the skeletons, and time.
 */
struct iMotionManager : public iBase
{
  /// Create a MotionTemplate.
  virtual iMotionTemplate* AddMotion (const char* name) = 0;
  /// Delete a MotionTemplate.
  virtual void DeleteMotion (iMotionTemplate* motiontemp) = 0;
  /// Find a MotionTemplate by name.
  virtual iMotionTemplate* FindMotionByName (const char* name) = 0;

  /**
   * Add a MotionController for animating a skeleton.
   * Will automatically get animated next call to UpdateAll() unless
   * you specifically pause the controller.
   */
  virtual iMotionController* AddController (iSkeletonBone *skel) = 0;
  /**
   * Delete a MotionController from a skeleton.
   *
   * Note: Use when deleting a skeleton, not when pausing or changing
   *       animations (Memory fragmentation!).
   */
  virtual void DeleteController (iMotionController *inst) = 0;
  /// Find a MotionController by its skeleton pointer
  virtual iMotionController* FindControllerBySkeleton (iSkeletonBone *skel) = 0;

  /**
   * Progress all motions forward by amount of time in seconds.
   *
   * Note: Use this if you want to support pause and per-scene timescaling.
   */
  virtual void UpdateAll ( float timedelta ) = 0;
  /**
   * Progress all motions forward to time in milliseconds.
   *
   * Note: Use this if you want to support pause, but not per-scene timescaling.
   */
  virtual void UpdateAll ( unsigned int curtime ) = 0;
  /**
   * Progress all motions forward based on the realtime clock.
   *
   * Note: Don't use this if you plan to implement pause or per-scene timescaling.
   */
  virtual void UpdateAll () = 0;
};

#endif

