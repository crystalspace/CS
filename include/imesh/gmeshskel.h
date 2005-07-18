/*
    Copyright  (C) 2004 by Hristo Hristov

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or  (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_IMESH_GMESHSKEL_H__
#define __CS_IMESH_GMESHSKEL_H__

#include "csutil/scf.h"
#include "csgeom/transfrm.h"
#include "csgeom/box.h"
#include "imesh/genmesh.h"

struct iRigidBody;
struct iGenMeshSkeletonBone;
struct iGenMeshSkeletonScript;
struct iGenMeshSkeletonBoneUpdateCallback;
struct iGenMeshSkeletonControlFactory;

enum csBoneTransformMode
{
	BM_SCRIPT = 0,
	BM_PHYSICS,
	BM_NONE
};

enum csAULevel
{
	AUL_BONES = 0,
	AUL_VERTICES
};

SCF_VERSION  (iGenMeshSkeletonControlState, 0, 0, 1);

/**
 * This interface describes the API for setting up the skeleton animation
 * control as implemented by the 'gmeshskelanim' plugin. The objects that
 * implement iGenMeshSkeletonControlState also implement this interface.
 */

struct iGenMeshSkeletonControlState : public iBase
{
  /**
   * Get skeleton bones count.
   */
  virtual int GetBonesCount () = 0;

  /**
   * Get bone by id.
   */
  virtual iGenMeshSkeletonBone *GetBone (int i) = 0;

  /**
   * Get bone by name.
   */
  virtual iGenMeshSkeletonBone *FindBone (const char *name) = 0;

  /**
   * Execute the given animation script. This will be done in addition
   * to the scripts that are already running. Returns false in case of
   * failure (usually a script that doesn't exist).
   */
  virtual iGenMeshSkeletonScript* Execute (const char *scriptname) = 0;

  /**
   * Get number of running scripts.
   */
  virtual size_t GetScriptsCount () = 0;

  /**
   * Get script by id.
   */
  virtual iGenMeshSkeletonScript* GetScript (size_t i) = 0;

  /**
   * Get script by name.
   */
  virtual iGenMeshSkeletonScript* FindScript (const char *scriptname) = 0;

  /**
   * Stop execution of all animation scripts.
   */
  virtual void StopAll () = 0;

  /**
   * Stop execution of the given script by name.
   */
  virtual void Stop (const char* scriptname) = 0;

  /**
   * Stop execution of the given script.
   */
  virtual void Stop (iGenMeshSkeletonScript *script) = 0;

  /**
   * Get factory
   */
  virtual iGenMeshSkeletonControlFactory *GetFactory() = 0;

  /**
   * Get animated vertices 
   */
  virtual csVector3 *GetAnimatedVertices() = 0;

  /**
   * Get animated vertices count
   */
  virtual int GetAnimatedVerticesCount() = 0;
};

SCF_VERSION  (iGenMeshSkeletonControlFactory, 0, 0, 1);

struct iGenMeshSkeletonControlFactory: public iGenMeshAnimationControlFactory 
{
  /**
   * Load animation script from file
   */
  virtual const char* LoadScriptFile(const char *filename) =0;
  
  /**
   * Delete script by name
   */
  virtual void DeleteScript(const char *script_name) = 0;
  
  /**
   * Delete all animation scripts
   */
  virtual void DeleteAllScripts() = 0;
};

SCF_VERSION  (iGenMeshSkeletonBone, 0, 0, 1);

struct iGenMeshSkeletonBone : public iBase
{
  /**
   * Get bone name.
   */
  virtual const char *GetName () const = 0;

  /**
   * Set bone name.
   */
  virtual void SetName (const char *name) = 0;

  /**
   * Get bone triansformation in local coordsys of parent.
   */
  virtual csReversibleTransform &GetTransform () = 0;

  /**
   * Set bone triansformation in local coordsys of parent.
   */
  virtual void SetTransform (const csReversibleTransform &transform) = 0;

  /**
   * Get bone triansformation in model coordsys.
   */
  virtual csReversibleTransform &GetFullTransform () = 0;

  /**
   * Get bone parent.
   */
  virtual iGenMeshSkeletonBone *GetParent () = 0;

  /**
   * Get bounding box of vertices attached to bone.
   * This is very usable if you want to create rigid body or collider.
   */
  virtual void GetSkinBox (csBox3 &box, csVector3 &center) = 0;

  /**
   * Set bone transform mode
   * BM_SCRIPT - by script instructions
   * BM_PHYSICS - by attached rigid body
   * BM_NONE - free bone transform
   * default is BM_SCRIPT
   */
  virtual void SetMode (csBoneTransformMode mode) = 0;

  /**
   * Get bone transform mode.
   */
  virtual csBoneTransformMode GetMode () = 0;

  /**
   * Attach rigid body to bone.
   */
  virtual void SetRigidBody (iRigidBody *r_body, csReversibleTransform & offset_transform) = 0;

  /**
   * Get attached rigid body.
   */
  virtual iRigidBody *GetRigidBody () = 0;

  /**
   * Get number of bones attached to this bone.
   */
  virtual int GetChildrenCount () = 0;

  /**
   * Get child bone by id.
   */
  virtual iGenMeshSkeletonBone *GetChild (int i) = 0;

  /**
   * Get child bone by name.
   */
  virtual iGenMeshSkeletonBone *FindChild (const char *name) = 0;

  /**
   * Set bone callback fuction.
   */
  virtual void SetUpdateCallback (iGenMeshSkeletonBoneUpdateCallback *callback) = 0;

  /**
   * Get bone callback fuction.
   */
  virtual iGenMeshSkeletonBoneUpdateCallback *GetUpdateCallback () = 0;
};

SCF_VERSION  (iGenMeshSkeletonBoneUpdateCallback, 0, 0, 1);

struct iGenMeshSkeletonBoneUpdateCallback : public iBase
{
	virtual void UpdateTransform(iGenMeshSkeletonBone *bone, const csReversibleTransform & transform) = 0;
};

SCF_VERSION  (iGenMeshSkeletonScript, 0, 0, 1);

struct iGenMeshSkeletonScript : public iBase
{
  /**
   * Get script name.
   */
  virtual const char *GetName () = 0;

  /**
   * Get script time.
   */
  virtual size_t GetTime () = 0;

  /**
   * Set script time.
   */
  virtual void SetTime (size_t time) = 0;

  /**
   * Get script influence factor.
   * Script's influence is available only if there are
   * two or more executing scripts.
   *
   * Example: We have script "walk" an script "run"
   * "walk" duration = 1000 ms
   * "run" duration = 500 ms
   * and we want to transform "walk" to "run" for 500 ms.
   * The transition from "walk" animation to "run" 
   * for a given period of time is achieved by slow 
   * decrement of "walk" factor and increment of "run" factor.
   * Also we have to make time synchronization.
   * 
   * Here are the steps for trnasformation separated by time:
   *
   *   0ms walk.time = 1000 walk.factor = 1
   *       run.time  = 1000 run.factor  = 0
   *
   * 100ms walk.time =  900 walk.factor = 0.8
   *       run.time  =  900 run.factor  = 0.2
   *
   * 200ms walk.time =  800 walk.factor = 0.6
   *       run.time  =  800 run.factor  = 0.4
   *
   * 300ms walk.time =  700 walk.factor = 0.4
   *       run.time  =  700 run.factor  = 0.6
   *
   * 400ms walk.time =  600 walk.factor = 0.2
   *       run.time  =  600 run.factor  = 0.0
   *
   * 500ms walk.time =  500 walk.factor = 0 //or just remove "walk" script
   *       run.time  =  500 run.factor  = 1
   */
  virtual void SetFactor (float factor) = 0;

  /**
   * Get script influence factor
   */
  virtual float GetFactor () = 0;
};

#endif // __CS_IMESH_GMESHSKEL_H__
