/*
    Copyright (C) 2005 by Hristo Hristov

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

#ifndef __CS_ISKELETON_H__
#define __CS_ISKELETON_H__

#include "csutil/scf.h"
#include "csgeom/box.h"

struct iSkeleton;
struct iSkeletonGraveyard;
struct iSkeletonFactory;
struct iSkeletonBoneFactory;
struct iSkeletonBoneUpdateCallback;
struct iSkeletonSocket;
struct iSkeletonSocketFactory;
struct iSceneNode;
//struct iSkeletonBoneRagdollInfo;

/*
struct iODEGeom;
struct iODERigidBody;
struct iODEJoint;
struct iODEDynamicSystem;
*/

enum csBoneTransformType
{
  CS_BTT_NONE = 0,
  CS_BTT_SCRIPT,
  CS_BTT_RIGID_BODY
};

enum csBoneGeomType
{
  CS_BGT_NONE = 0,
  CS_BGT_BOX,
  CS_BGT_SPHERE,
  CS_BGT_CYLINDER
};

class csReversibleTransform;

/**
 * The skeleton bone class.
 */
struct iSkeletonBone : public virtual iBase
{
  SCF_INTERFACE (iSkeletonBone, 1, 0, 1);

  /**
   * Get name of the bone.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set name of the bone.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Get transform of the bone.
   */
  virtual csReversibleTransform &GetTransform () = 0;

  /**
   * Set transform of the bone in parent's coordsys.
   */
  virtual void SetTransform (const csReversibleTransform &transform) = 0;

  /**
   * Get full transform of the bone.
   */
  virtual csReversibleTransform &GetFullTransform () = 0;

  /**
   * Set parent bone.
   */
  virtual void SetParent (iSkeletonBone *parent) = 0;

  /**
   * Get parent bone
   */
  virtual iSkeletonBone *GetParent () = 0;

  /**
   * Get number of children bones.
   */
  virtual size_t GetChildrenCount () = 0;

  /**
   * Set child bone by index.
   */
  virtual iSkeletonBone *GetChild (size_t i) = 0;

  /**
   * Find child bone by name.
   */
  virtual iSkeletonBone *FindChild (const char *name) = 0;

  /**
   * Find child bone index.
   */
  virtual size_t FindChildIndex (iSkeletonBone *child) = 0;

  /**
   * Set skin bbox (usefull for creating collider or ragdoll object).
   */
  virtual void SetSkinBox (csBox3 &box) = 0;

  /**
   * Get skin bbox.
   */
  virtual csBox3 &GetSkinBox () = 0;

  /**
   * Set callback to the bone. By default there
   * is callback that sets bone transform when updating.
   */
  virtual void SetUpdateCallback (iSkeletonBoneUpdateCallback *callback) = 0;

  /**
   * Get update callback.
   */
  virtual iSkeletonBoneUpdateCallback *GetUpdateCallback () = 0;

  /**
   * Get skeleton factory.
   */
  virtual iSkeletonBoneFactory *GetFactory() = 0;

  /**
   * Set bone transform mode.
   * Possible values are:
   * - #CS_BTT_NONE: Same as CS_BTT_SCRIPT.
   * - #CS_BTT_SCRIPT: Normal default behaviour. Animations control the bone.
   * - #CS_BTT_RIGID_BODY: Unimplemented rigid body ragdoll.
   */
  virtual void SetTransformMode(csBoneTransformType mode) = 0;

  /**
   * Get bone transform mode.
   */
  virtual csBoneTransformType GetTransformMode() = 0;

  //virtual void SetRigidBody(iODERigidBody *rigid_body, const csReversibleTransform & offset_transform) = 0;
  //virtual iODERigidBody *GetRigidBody() = 0;
  //virtual void SetJoint(iODEJoint *joint) = 0;
  //virtual iODEJoint *GetJoint() = 0;
};

/**
 * This callback fires every time when bone changes it's transform.
 */
struct iSkeletonBoneUpdateCallback : public virtual iBase
{
  SCF_INTERFACE  (iSkeletonBoneUpdateCallback, 0, 0, 1);

  virtual void UpdateTransform(iSkeletonBone *bone,
      const csReversibleTransform & transform) = 0;
};

class csQuaternion;

/**
 * The script key frame contains all bones that will be transformed in 
 * a specific time of a skeleton script.
 */
struct iSkeletonAnimationKeyFrame : public virtual iBase
{
  SCF_INTERFACE (iSkeletonAnimationKeyFrame, 1, 0, 0);

  /**
   * Get name of the key frame.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set name of the key frame.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Get key frame duration.
   */
  virtual csTicks GetDuration () = 0;

  /**
   * Set key frame duration.
   */
  virtual void SetDuration (csTicks time) = 0;

  /**
   * Get number of key transforms.
   */
  virtual size_t GetTransformsCount() = 0;

  /**
   * Add new bone transform to the key frame.
   */
  virtual void AddTransform (iSkeletonBoneFactory *bone, 
    csReversibleTransform &transform, bool relative = false) = 0;

  /**
   * Get the transform of a bone. Returns 'false' when there won't be 
   * any transform data for given bone.
   */
  virtual bool GetTransform (iSkeletonBoneFactory *bone,
      csReversibleTransform &dst_trans) = 0;

  /**
   * Set the transform of a bone.
   */
  virtual void SetTransform(iSkeletonBoneFactory *bone, 
    csReversibleTransform &transform) = 0;

  /**
   * Get key frame specific data. Returns false when frame don't have
   * data for given bone.
   */
  virtual bool GetKeyFrameData (iSkeletonBoneFactory *bone_fact, 
    csQuaternion & rot, csVector3 & pos, csQuaternion & tangent,
    bool & relative) = 0;
};

/**
 * This interface provides animation of a skeleton.
 */
struct iSkeletonAnimation : public virtual iBase
{
  SCF_INTERFACE (iSkeletonAnimation, 1, 0, 0);

  /**
   * Get animation name.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set animation name.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Get animation duration.
   */
  virtual csTicks GetTime () = 0;

  /**
   * Set animation duration.
   */
  virtual void SetTime (csTicks time) = 0;

  /**
   * Get animation speed.
   */
  virtual float GetSpeed () = 0;

  /**
   * Set animation speed (default = 1.0).
   */
  virtual void SetSpeed (float speed) = 0;

  /**
   * Set animation factor.
   */
  virtual void SetFactor (float factor) = 0;

  /**
   * Get animation factor.
   */
  virtual float GetFactor () = 0;

  /**
   * Set animation loop value.
   */
  virtual void SetLoop (bool loop) = 0;

  /**
   * Get animation loop value.
   */
  virtual bool GetLoop () = 0;

  /**
   * Create new key frame.
   */
  virtual iSkeletonAnimationKeyFrame *CreateFrame (const char* name) = 0;

  /**
   * Get number of frames in the animation.
   */
  virtual size_t GetFramesCount () = 0;

  /**
   * Get key frame by index.
   */
  virtual iSkeletonAnimationKeyFrame *GetFrame (size_t i) = 0;

  /**
   * Find key frame by name.
   */
  virtual size_t FindFrameIndex (const char *name) = 0;

  /**
   * Remove frame by index.
   */
  virtual void RemoveFrame (size_t i) = 0;

  /**
   * Remove all frames.
   */
  virtual void RemoveAllFrames () = 0;

  /**
   * Recalculates spline for bones rotations.
   * Needs to be called every time when new 
   * frames are added or removed.
   */
  virtual void RecalcSpline () = 0;
};

/**
 * This is a callback function of an animation.
 * It is called every time when animation is started or finished.
 */
struct iSkeletonAnimationCallback : public virtual iBase
{
  SCF_INTERFACE (iSkeletonAnimationCallback, 1, 0, 0);

  /**
   * On execute action.
   */
  virtual void Execute(iSkeletonAnimation *animation, size_t frame_idx) = 0;

  /**
   * On finish action.
   */
  virtual void OnFinish(iSkeletonAnimation *animation) = 0;
};

/**
 * This is a callback function of a skeleton.
 * It is called every time when skeleton is updated.
 */

struct iSkeletonUpdateCallback : public virtual iBase
{
  SCF_INTERFACE (iSkeletonUpdateCallback, 1, 0, 0);

  /**
   * General skeleon update callback.
   */
  virtual void Execute(iSkeleton *skeleton, const csTicks & current_ticks) = 0;
};

/**
 * This interface provides played animation instance of a skeleton. 
 */
struct iSkeletonAnimationInstance : public virtual iBase
{
  SCF_INTERFACE (iSkeletonAnimationInstance, 1, 0, 0);

  /**
   * Get animation speed.
   */
  virtual float GetSpeed () = 0;

  /**
   * Set animation speed (default = 1.0).
   */
  virtual void SetSpeed (float speed) = 0;

  /**
   * Set animation factor.
   */
  virtual void SetFactor (float factor) = 0;

  /**
   * Get animation factor.
   */
  virtual float GetFactor () = 0;

  /**
   * Get animation instance duration.
   */
  virtual csTicks GetDuration () = 0;

  /**
   * Set animation instance duration.
   */
  virtual void SetDuration (csTicks time) = 0;
};

/**
 * The skeleton interface provides needed functionality
 * of a skeleton animation. It holds bones, sockets and scripts.
 * Skeleton is an independend object and it is not realted to a mesh.
 * Genmesh Skelton Animation 2 plugin makes the connection between
 * mesh and skeleton. Users can query the iSkeleton from genmeshes as follows:
 *
 *   csRef<iGeneralMeshState> genmesh_state (
 *     scfQueryInterface<iGeneralMeshState> (mesh_wrapper->GetMeshObject ()));
 *   csRef<iGenMeshSkeletonControlState> animcontrol (
 *     scfQueryInterface<iGenMeshSkeletonControlState> (
 *     genmesh_state->GetAnimationControl ()));
 *   iSkeleton* skeleton = animcontrol->GetSkeleton ();
 */
struct iSkeleton : public virtual iBase
{
  SCF_INTERFACE (iSkeleton, 1, 0, 0);

  /**
   * Get skeleton name.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set skeleton name.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Get number of bones in the skeleton.
   */
  virtual size_t GetBonesCount () = 0;

  /**
   * Get bone by index.
   */
  virtual iSkeletonBone *GetBone (size_t i) = 0;

  /**
   * Find bone by name.
   */
  virtual iSkeletonBone *FindBone (const char *name) = 0;

  /**
   * Find bine index by name.
   */
  virtual size_t FindBoneIndex (const char *name) = 0;

  /**
   * Execute specific animation.
   */
  virtual iSkeletonAnimation* Execute (const char *animation_name, float blend_factor = 0.0f) = 0;

  /**
   * Append animation for execution. Plays after all current animations
   * have finished. Then the first Append'ed animation is played...
   * then the next... and so on.
   */
  virtual iSkeletonAnimation* Append (const char *animation_name) = 0;

  /**
   * Play specific animation. Returns played animation instance.
   */
  virtual iSkeletonAnimationInstance *Play (const char *animation_name) = 0;

  /**
   * Stop animation. 
   */
  virtual void Stop (iSkeletonAnimationInstance *anim_instance) = 0;

  /**
   * Clear animations for execution.
   */
  virtual void ClearPendingAnimations () = 0;

  /**
   * Get number of running animations.
   */
  virtual size_t GetAnimationsCount () = 0;

  /**
   * Get animation by index.
   */
  virtual iSkeletonAnimation* GetAnimation (size_t i) = 0;

  /**
   * Find animation by name.
   */
  virtual iSkeletonAnimation* FindAnimation (const char *animation_name) = 0;

  /**
   * Find socket by name.
   */
  virtual iSkeletonSocket* FindSocket (const char *socketname) = 0;

  /**
   * Stop all executed animations.
   */
  virtual void StopAll () = 0;

  /**
   * Stop executed animation by name.
   */
  virtual void Stop (const char* animation_name) = 0;

  /**
   * Get skeleton factory.
   */
  virtual iSkeletonFactory *GetFactory () = 0;

  /**
   * Set animation callback.
   */
  virtual void SetAnimationCallback (iSkeletonAnimationCallback *cb) = 0;

  //virtual void CreateRagdoll(iODEDynamicSystem *dyn_sys, csReversibleTransform & transform) = 0;
  //virtual void DestroyRagdoll() = 0;

  /**
   * Adds skeleton update callback.
   */
  virtual size_t AddUpdateCallback (
      iSkeletonUpdateCallback *update_callback) = 0;

  /**
   * Get number of skeleton callbacks.
   */
  virtual size_t GetUpdateCallbacksCount () = 0;

  /**
   * Get callback by index.
   */
  virtual iSkeletonUpdateCallback *GetUpdateCallback(size_t callback_idx) = 0;

  /**
   * Remove skelton callback by index.
   */
  virtual void RemoveUpdateCallback (size_t callback_idx) = 0;

  /**
   * Update animations state. Returns 'false' when no update
   * was aplied.
   */
  virtual bool UpdateAnimation (csTicks current_time) = 0;

  /**
   * Update skeleton bones. Normaly you won't ever need to call this
   * method by yourself (updating bones is done via 'UpdateAnimation'),
   * but you will need to use it too see changes after direct pose changes
   * when you are not updating animations for some reason.
   */
  virtual void UpdateBones () = 0;
};

/**
 * The skeleton socket object wraps a relative transform of a bone.
 * It is used to attach meshes, cameras or lights to a bone.
 * When bone animates it moves the attached object to the socket too.
 * This is usefull to create an invertory of a model.
 */
struct iSkeletonSocket : public virtual iBase
{
  SCF_INTERFACE (iSkeletonSocket, 1, 0, 0);

  /**
   * Get socket name.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set socket name.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Get socket transform in parent's coordsys.
   */
  virtual csReversibleTransform &GetTransform () = 0;

  /**
   * Set socket transform in parent's coordsys.
   */
  virtual void SetTransform (const csReversibleTransform &transform) = 0;

  /**
   * Get full transform of the socket.
   */
  virtual csReversibleTransform &GetFullTransform () = 0;

  /**
   * Set parent bone.
   */
  virtual void SetBone (iSkeletonBone *bone) = 0;

  /**
   * Get parent bone.
   */
  virtual iSkeletonBone *GetBone () = 0;

  /**
   * Set scene node (mesh, camera or light).
   */
  virtual void SetSceneNode (iSceneNode *node) = 0;

  /**
   * Get scene node.
   */
  virtual iSceneNode *GetSceneNode () = 0;

  /**
   * Get factory of the socket.
   */
  virtual iSkeletonSocketFactory *GetFactory () = 0;
};

struct iSkeletonBoneRagdollInfo : public virtual iBase
{
  SCF_INTERFACE (iSkeletonBoneRagdollInfo, 1, 0, 0);

  virtual void SetEnabled(bool enabled) = 0;
  virtual bool GetEnabled() = 0;
  virtual void SetAttachToParent(bool attach) = 0;
  virtual bool GetAttachToParent() = 0;

  virtual void SetGeomName(const char *name) = 0;
  virtual const char *GetGeomName() = 0;
  virtual void SetGeomType(int geom_type) = 0;
  virtual int GetGeomType() = 0;
  virtual void SetGeomDimensions(csVector3 &size) = 0;
  virtual csVector3 &GetGeomDimensions() = 0;

  virtual void SetFriction(float friction) = 0;
  virtual float GetFriction() = 0;
  virtual void SetElasticity(float elasticity) = 0;
  virtual float GetElasticity() = 0;
  virtual void SetSoftness(float softness) = 0;
  virtual float GetSoftness() = 0;
  virtual void SetSlip(float slip) = 0;
  virtual float GetSlip() = 0;

  virtual void SetBodyName(const char *name) = 0;
  virtual const char *GetBodyName() = 0;
  virtual void SetBodyMass(float mass) = 0;
  virtual float GetBodyMass() = 0;
  virtual void SetBodyGravmode(int gravmode) = 0;
  virtual int GetBodyGravmode() = 0;

  virtual void SetJointName(const char *name) = 0;
  virtual const char *GetJointName() = 0;
  virtual void SetJointMinRotContraints(csVector3 & constraints) = 0;
  virtual csVector3 & GetJointMinRotContraints() = 0;
  virtual void SetJointMaxRotContraints(csVector3 & constraints) = 0;
  virtual csVector3 & GetJointMaxRotContraints() = 0;
  virtual void SetJointMinTransContraints(csVector3 & constraints) = 0;
  virtual csVector3 & GetJointMinTransContraints() = 0;
  virtual void SetJointMaxTransContraints(csVector3 & constraints) = 0;
  virtual csVector3 & GetJointMaxTransContraints() = 0;
};

/**
 * The skeleton bone factory is class that is used to create
 * skeleton bones of a iSkeleton object.
 */
struct iSkeletonBoneFactory : public virtual iBase
{
  SCF_INTERFACE (iSkeletonBoneFactory, 1, 0, 0);

  /**
   * Get bone factory name.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set bone factory of name.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Get bone factory transform in parent's coordsys.
   */
  virtual csReversibleTransform &GetTransform () = 0;

  /**
   * Set bone factory transform in parent's coordsys.
   */
  virtual void SetTransform (const csReversibleTransform &transform) = 0;

  /**
   * Get bone factory full transform.
   */
  virtual csReversibleTransform &GetFullTransform () = 0;

  /**
   * Set parent bone factory .
   */
  virtual  void SetParent (iSkeletonBoneFactory *parent) = 0;

  /**
   * Get parent bone factory .
   */
  virtual iSkeletonBoneFactory *GetParent () = 0;

  /**
   * Get number of children factories.
   */
  virtual size_t GetChildrenCount () = 0;

  /**
   * Get factory child by index.
   */
  virtual iSkeletonBoneFactory *GetChild (size_t i) = 0;

  /**
   * Find child by name.
   */
  virtual iSkeletonBoneFactory *FindChild (const char *name) = 0;

  /**
   * Find child index.
   */
  virtual size_t FindChildIndex (iSkeletonBoneFactory *child) = 0;

  /**
   * Set skin bbox.
   */
  virtual void SetSkinBox (csBox3 & box) = 0;

  /**
   * Get skin bbox.
   */
  virtual csBox3 & GetSkinBox () = 0;

  /**
   * Get ragdoll data.
   */
  virtual iSkeletonBoneRagdollInfo *GetRagdollInfo() = 0;
};

/**
 * The skeleton socket factory is class that is used to create
 * skeleton sockets of a iSkeleton object.
 */
struct iSkeletonSocketFactory : public virtual iBase
{
  SCF_INTERFACE (iSkeletonSocketFactory, 1, 0, 0);

  /**
   * Get name of the socket factory.
   */
  virtual const char* GetName () const = 0;

  /**
   * Set name.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Get transform in parent's coordsys.
   */
  virtual csReversibleTransform &GetTransform () = 0;

  /**
   * Set transform in parent's coordsys.
   */
  virtual void SetTransform (const csReversibleTransform &transform) = 0;

  /**
   * Get full transform.
   */
  virtual csReversibleTransform &GetFullTransform () = 0;

  /**
   * Set parent bone factory.
   */
  virtual void SetBone (iSkeletonBoneFactory *bone) = 0;

  /**
   * Get parent bone factory.
   */
  virtual iSkeletonBoneFactory *GetBone () = 0;
};

/**
 * The skeleton factory is class that is used to create
 * skeleton objects in the scene.
 */
struct iSkeletonFactory : public virtual iBase
{
  SCF_INTERFACE (iSkeletonFactory, 1, 0, 0);

  /**
   * Get name of the skeleton factory.
   */
  virtual const char* GetName () const = 0;

  /**
   * Get name.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Create new bone factory.
   */
  virtual iSkeletonBoneFactory *CreateBone (const char *name) = 0;

  /**
   * Create new animation.
   */
  virtual iSkeletonAnimation *CreateAnimation (const char *name) = 0;

  /**
   * Find animation by name.
   */
  virtual iSkeletonAnimation *FindAnimation (const char *name) = 0;

  /**
   * Get number of available animations.
   */
  virtual size_t GetAnimationsCount () = 0;

  /**
   * Get animation by index.
   */
  virtual iSkeletonAnimation *GetAnimation (size_t idx) = 0;

  /**
   * Find bone factory by name.
   */
  virtual iSkeletonBoneFactory *FindBone (const char *name) = 0;

  /**
   * Find bone facotry index by name.
   */
  virtual size_t FindBoneIndex (const char *name) = 0;

  /**
   * Get number of bones factories.
   */
  virtual size_t GetBonesCount () const = 0;

  /**
   * Get bone factory by index.
   */
  virtual iSkeletonBoneFactory *GetBone(size_t i) = 0;

  /**
   * Get the Graveyard.
   */
  virtual iSkeletonGraveyard *GetGraveyard  () = 0;

  /**
   * Create new socket factory.
   */
  virtual iSkeletonSocketFactory *CreateSocket(const char *name,
      iSkeletonBoneFactory *bone) = 0;

  /**
   * Find socket factory by name.
   */
  virtual iSkeletonSocketFactory *FindSocket(const char *name) = 0;

  /**
   * Get socket factory by name.
   */
  virtual iSkeletonSocketFactory *GetSocket (int i) = 0;

  /**
   * Remove socket facotry by index.
   */
  virtual void RemoveSocket (int i) = 0;

  /**
   * Get number of socket factories.
   */
  virtual size_t GetSocketsCount() = 0;
};


/**
 * iSkeletonGraveyard is the interface that cares for all skeleton factories. 
 * It can be accessed via object registry. Also it holds and updates all 
 * existing skeleton objects.
 */
struct iSkeletonGraveyard : public virtual iBase
{
  SCF_INTERFACE (iSkeletonGraveyard, 1, 0, 0);

  /**
   * Get number of skeleton factories.
   */
  virtual size_t GetFactoriesCount() = 0;

  /**
   * Get skeleton factory by name.
   */
  virtual iSkeletonFactory *CreateFactory(const char *name) = 0;

  /**
   * Load skeleton factory from file.
   */
  virtual iSkeletonFactory *LoadFactory(const char *file_name) = 0;

  /**
   * Find skeleton factory by name.
   */
  virtual iSkeletonFactory *FindFactory(const char *name) = 0;

  /**
   * Create skeleton from specific factory.
   */
  virtual iSkeleton *CreateSkeleton(iSkeletonFactory *fact,
      const char *name = 0) = 0;

  /**
   * Set manual updates handling mode.
   */
  virtual void SetManualUpdates (bool man_updates) = 0;

  /**
   * Set manual updates handling mode.
   */
  virtual void Update (csTicks time) = 0;

  /**
   * Add skeleton that will be updated by this graveyard. 
   */
  virtual void AddSkeleton (iSkeleton *skeleton) = 0;

  /**
   * Remove a skeleton again.
   */
  virtual void RemoveSkeleton (iSkeleton* skeleton) = 0;
};

#endif //__CS_ISKELETON_H__
