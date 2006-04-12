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

struct iSkeletonBone : public virtual iBase
{
  SCF_INTERFACE (iSkeletonBone, 1, 0, 0);

  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual csReversibleTransform &GetTransform () = 0;
  virtual void SetTransform (const csReversibleTransform &transform) = 0;
  virtual csReversibleTransform &GetFullTransform () = 0;
  virtual  void SetParent (iSkeletonBone *parent) = 0;
  virtual iSkeletonBone *GetParent () = 0;
  virtual int GetChildrenCount () = 0;
  virtual iSkeletonBone *GetChild (size_t i) = 0;
  virtual iSkeletonBone *FindChild (const char *name) = 0;
  virtual size_t FindChildIndex (iSkeletonBone *child) = 0;
  virtual void SetSkinBox (csBox3 & box) = 0;
  virtual csBox3 & GetSkinBox () = 0;
  virtual void SetUpdateCallback (iSkeletonBoneUpdateCallback *callback) = 0;
  virtual iSkeletonBoneUpdateCallback *GetUpdateCallback () = 0;
  virtual iSkeletonBoneFactory *GetFactory() = 0;
  virtual void SetTransformMode(csBoneTransformType mode) = 0;
  virtual csBoneTransformType GetTransformMode() = 0;
  //virtual void SetRigidBody(iODERigidBody *rigid_body, const csReversibleTransform & offset_transform) = 0;
  //virtual iODERigidBody *GetRigidBody() = 0;
  //virtual void SetJoint(iODEJoint *joint) = 0;
  //virtual iODEJoint *GetJoint() = 0;
};

SCF_VERSION  (iSkeletonBoneUpdateCallback, 0, 0, 1);

struct iSkeletonBoneUpdateCallback : public virtual iBase
{
	virtual void UpdateTransform(iSkeletonBone *bone, const csReversibleTransform & transform) = 0;
};

struct iSkeletonScriptKeyFrame : public virtual iBase
{
  SCF_INTERFACE (iSkeletonScriptKeyFrame, 1, 0, 0);
  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual csTicks GetDuration () = 0;
  virtual void SetDuration (csTicks time) = 0;
  virtual size_t GetTransformsCount() = 0;
  virtual void AddTransform(iSkeletonBoneFactory *bone, 
	  csReversibleTransform &transform) = 0;
  virtual csReversibleTransform & GetTransform(iSkeletonBoneFactory *bone) = 0;
  virtual void SetTransform(iSkeletonBoneFactory *bone, 
	  csReversibleTransform &transform) = 0;

  virtual csReversibleTransform & GetTransform(size_t i) = 0;
  virtual iSkeletonBoneFactory *GetBone(size_t i) = 0;
};

struct iSkeletonScript : public virtual iBase
{
  SCF_INTERFACE (iSkeletonScript, 1, 0, 0);

  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual csTicks GetTime () = 0;
  virtual void SetTime (csTicks time) = 0;
  virtual float GetSpeed () = 0;
  virtual void SetSpeed (float speed) = 0;
  virtual void SetFactor (float factor) = 0;
  virtual float GetFactor () = 0;
  virtual void SetLoop (bool loop) = 0;
  virtual bool GetLoop () = 0;

  virtual iSkeletonScriptKeyFrame *CreateFrame(const char* name) = 0;
  virtual size_t GetFramesCount() = 0;
  virtual iSkeletonScriptKeyFrame *GetFrame(size_t i) = 0;
  virtual size_t FindFrameIndex(const char *name) = 0;
  virtual void RemoveFrame(size_t i) = 0;
};

struct iSkeletonScriptCallback : public virtual iBase
{
    SCF_INTERFACE (iSkeletonScriptCallback, 1, 0, 0);
	virtual void Execute(iSkeletonScript *script, size_t frame_idx) = 0;
	virtual void OnFinish(iSkeletonScript *script) = 0;
};

struct iSkeletonUpdateCallback : public virtual iBase
{
    SCF_INTERFACE (iSkeletonUpdateCallback, 1, 0, 0);
	virtual void Execute(iSkeleton *skeleton, const csTicks & current_ticks) = 0;
};

struct iSkeleton : public virtual iBase
{
  SCF_INTERFACE (iSkeleton, 1, 0, 0);

  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual size_t GetBonesCount () = 0;
  virtual iSkeletonBone *GetBone (size_t i) = 0;
  virtual iSkeletonBone *FindBone (const char *name) = 0;
  virtual size_t FindBoneIndex (const char *name) = 0;
  virtual iSkeletonScript* Execute (const char *scriptname) = 0;
  virtual iSkeletonScript* Append (const char *scriptname) = 0;
  virtual void ClearPendingScripts () = 0;
  virtual size_t GetScriptsCount () = 0;
  virtual iSkeletonScript* GetScript (size_t i) = 0;
  virtual iSkeletonScript* FindScript (const char *scriptname) = 0;
  virtual iSkeletonSocket* FindSocket (const char *socketname) = 0;
  virtual void StopAll () = 0;
  virtual void Stop (const char* scriptname) = 0;
  virtual iSkeletonFactory *GetFactory() = 0;
  virtual void SetScriptCallback(iSkeletonScriptCallback *cb) = 0;
  //virtual void CreateRagdoll(iODEDynamicSystem *dyn_sys, csReversibleTransform & transform) = 0;
  //virtual void DestroyRagdoll() = 0;

  virtual size_t AddUpdateCallback(iSkeletonUpdateCallback *update_callback) = 0;
  virtual size_t GetUpdateCallbacksCount() = 0;
  virtual iSkeletonUpdateCallback *GetUpdateCallback(size_t callback_idx) = 0;
  virtual void RemoveUpdateCallback(size_t callback_idx) = 0;
};

struct iSkeletonSocket : public virtual iBase
{
  SCF_INTERFACE (iSkeletonSocket, 1, 0, 0);
  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual csReversibleTransform &GetTransform () = 0;
  virtual void SetTransform (const csReversibleTransform &transform) = 0;
  virtual csReversibleTransform &GetFullTransform () = 0;
  virtual void SetBone (iSkeletonBone *bone) = 0;
  virtual iSkeletonBone *GetBone () = 0;
  virtual void SetSceneNode (iSceneNode *node) = 0;
  virtual iSceneNode *GetSceneNode () = 0;
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

struct iSkeletonBoneFactory : public virtual iBase
{
  SCF_INTERFACE (iSkeletonBoneFactory, 1, 0, 0);
  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual csReversibleTransform &GetTransform () = 0;
  virtual void SetTransform (const csReversibleTransform &transform) = 0;
  virtual csReversibleTransform &GetFullTransform () = 0;
  virtual  void SetParent (iSkeletonBoneFactory *parent) = 0;
  virtual iSkeletonBoneFactory *GetParent () = 0;
  virtual int GetChildrenCount () = 0;
  virtual iSkeletonBoneFactory *GetChild (size_t i) = 0;
  virtual iSkeletonBoneFactory *FindChild (const char *name) = 0;
  virtual size_t FindChildIndex (iSkeletonBoneFactory *child) = 0;
  virtual void SetSkinBox (csBox3 & box) = 0;
  virtual csBox3 & GetSkinBox () = 0;
  virtual iSkeletonBoneRagdollInfo *GetRagdollInfo() = 0;
};

struct iSkeletonSocketFactory : public virtual iBase
{
  SCF_INTERFACE (iSkeletonSocketFactory, 1, 0, 0);
  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual csReversibleTransform &GetTransform () = 0;
  virtual void SetTransform (const csReversibleTransform &transform) = 0;
  virtual csReversibleTransform &GetFullTransform () = 0;
  virtual void SetBone (iSkeletonBoneFactory *bone) = 0;
  virtual iSkeletonBoneFactory *GetBone () = 0;
};

struct iSkeletonFactory : public virtual iBase
{
  SCF_INTERFACE (iSkeletonFactory, 1, 0, 0);
  virtual const char* GetName () const = 0;
  virtual void SetName (const char* name) = 0;
  virtual iSkeletonBoneFactory *CreateBone(const char *name) = 0;
  virtual iSkeletonScript *CreateScript(const char *name) = 0;
  virtual iSkeletonScript *FindScript(const char *name) = 0;
  virtual iSkeletonBoneFactory *FindBone (const char *name) = 0;
  virtual size_t FindBoneIndex (const char *name) = 0;
  virtual iSkeletonGraveyard *GetGraveyard  () = 0;

  virtual iSkeletonSocketFactory *CreateSocket(const char *name, iSkeletonBoneFactory *bone) = 0;
  virtual iSkeletonSocketFactory *FindSocket(const char *name) = 0;
  virtual iSkeletonSocketFactory *GetSocket (int i) = 0;
  virtual void RemoveSocket (int i) = 0;
  virtual size_t GetSocketsCount() = 0;
};

struct iSkeletonGraveyard : public virtual iBase
{
  SCF_INTERFACE (iSkeletonGraveyard, 1, 0, 0);
  virtual int GetFactoriesCount() = 0;
  virtual iSkeletonFactory *CreateFactory(const char *name) = 0;
  virtual iSkeletonFactory *LoadFactory(const char *file_name) = 0;
  virtual iSkeletonFactory *FindFactory(const char *name) = 0;
  virtual iSkeleton *CreateSkeleton(iSkeletonFactory *fact, const char *name = 0) = 0;
};

#endif //__CS_ISKELETON_H__
