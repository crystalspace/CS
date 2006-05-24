/*
    Copyright (C) 2006 by Hristo Hristov

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

#ifndef __CS_SKELETON_H__
#define __CS_SKELETON_H__

#include "csgeom/quaternion.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/flags.h"
#include "csutil/hash.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/strhash.h"
#include "csutil/stringarray.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshskel.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "imesh/object.h"
//#include "ivaria/dynamics.h"
#include "imesh/skeleton.h"
#include <iengine/scenenode.h>
#include <iengine/movable.h>
//#include <iphysics/dynamics.h>

class csSkeleton;
class csSkeletonFactory;
class csSkeletonBoneFactory;
class csSkeletonGraveyard;
class csSkeletonSocketFactory;

class csSkeletonBone : public iSkeletonBone
{
private:
  csString name;
  csSkeleton *skeleton;
  csSkeletonBoneFactory *factory_bone;
  csSkeletonBone* parent;
  csArray<csSkeletonBone *> bones;
  csReversibleTransform next_transform;
  csReversibleTransform transform;
  csReversibleTransform full_transform;
  csReversibleTransform offset_body_transform;
  csQuaternion rot_quat;
  csRef<iSkeletonBoneUpdateCallback> cb;
  csBox3 skin_box;
  csBoneTransformType transform_mode;
  //iODERigidBody *rigid_body;
  //iODEJoint *joint;
  csReversibleTransform offset_transform;
public:
  csReversibleTransform & GetOffsetTransform() 
    { return offset_body_transform; }
  csQuaternion & GetQuaternion () { return rot_quat; };
  void AddBone (csSkeletonBone *bone) { bones.Push (bone); }
  csArray<csSkeletonBone *> & GetBones () { return bones; }

  void UpdateBones ();
  void UpdateBones (csSkeletonBone* parent_bone);

  void UpdateTransform();
  void FireCallback() 
  { if (cb) cb->UpdateTransform(this, next_transform); }

  //------------------------------------------------------------------------

  csSkeletonBone (csSkeleton *skeleton, csSkeletonBoneFactory *factory_bone);
  virtual ~csSkeletonBone ();

  SCF_DECLARE_IBASE;
  virtual const char* GetName () const { return name.GetData(); }
  virtual void SetName (const char* name) { csSkeletonBone::name = name; }
  virtual csReversibleTransform &GetTransform () { return transform; }
  virtual void SetTransform (const csReversibleTransform &transform) 
  {
    csSkeletonBone::transform = transform;
    rot_quat.SetMatrix (transform.GetT2O());
  }
  virtual csReversibleTransform &GetFullTransform () 
    { return full_transform; }
  virtual void SetParent (iSkeletonBone* par);
  virtual iSkeletonBone* GetParent () { return parent; }
  virtual int GetChildrenCount () { return (int)bones.Length () ;}
  virtual iSkeletonBone *GetChild (size_t i) { return bones[i]; }
  virtual iSkeletonBone *FindChild (const char *name);
  virtual void SetUpdateCallback (iSkeletonBoneUpdateCallback *callback) 
    { cb = callback; }
  virtual iSkeletonBoneUpdateCallback *GetUpdateCallback () 
    { return cb; };
  virtual iSkeletonBoneFactory *GetFactory() 
    { return (iSkeletonBoneFactory *)factory_bone; }
  virtual size_t FindChildIndex (iSkeletonBone *child);
  virtual void SetSkinBox (csBox3 & box) { skin_box = box; }
  virtual csBox3 & GetSkinBox () { return skin_box; }
  /*
  virtual void SetRigidBody(iODERigidBody *rigid_body, const csReversibleTransform & offset_transform)
  { 
     csSkeletonBone::rigid_body = rigid_body;
    offset_body_transform = offset_transform; 
  }
  virtual iODERigidBody *GetRigidBody()
  { return rigid_body; }
  virtual void SetJoint(iODEJoint *joint)
  { csSkeletonBone::joint = joint; }
  virtual iODEJoint *GetJoint()
  { return joint; }
  */
  virtual void SetTransformMode(csBoneTransformType mode)
  { transform_mode = mode; }
  virtual csBoneTransformType GetTransformMode()
  { return transform_mode; }
};

class csSkeletonBoneDefaultUpdateCallback : public iSkeletonBoneUpdateCallback
{
public:
  SCF_DECLARE_IBASE;
  csSkeletonBoneDefaultUpdateCallback()
  {
    SCF_CONSTRUCT_IBASE(0);
  }
  
  virtual ~csSkeletonBoneDefaultUpdateCallback()
  {
    SCF_DESTRUCT_IBASE();
  }

  virtual void UpdateTransform(iSkeletonBone *bone, const csReversibleTransform & transform)
  {
    //bone->GetTransform().SetO2T(transform.GetO2T());
    //bone->GetTransform().SetOrigin(transform.GetOrigin());
    bone->SetTransform(transform);
  }
};

//----------------------------------------- csSkeletonSocket ------------------------------------------

class csSkeletonSocket : public iSkeletonSocket
{
private:
  csString name;
  csReversibleTransform transform;
  csReversibleTransform full_transform;
  iSkeletonBone *bone;
  iSceneNode *node;
  csSkeletonSocketFactory *factory;
public:
  csSkeletonSocket (csSkeleton *skeleton, csSkeletonSocketFactory *socket_factory);
  virtual ~csSkeletonSocket ();
  SCF_DECLARE_IBASE;
  virtual const char* GetName () const
  { return name; }
  virtual void SetName (const char* name)
  { csSkeletonSocket::name = name; }
  virtual csReversibleTransform &GetTransform ()
  { return transform; }
  virtual void SetTransform (const csReversibleTransform &transform)
  { csSkeletonSocket::transform = transform; }
  virtual csReversibleTransform &GetFullTransform ()
  { return full_transform; }
  virtual void SetBone (iSkeletonBone *bone)
  { csSkeletonSocket::bone = bone; }
  virtual iSkeletonBone *GetBone ()
  { return bone; }
    virtual void SetSceneNode (iSceneNode *node)
  { csSkeletonSocket::node = node; }
    virtual iSceneNode *GetSceneNode ()
  { return node; }
    virtual iSkeletonSocketFactory *GetFactory ()
  { return (iSkeletonSocketFactory *)factory; }
};


//----------------------------- csSkeletonBoneFactory -------------------------------

class csSkeletonBoneFactory : public iSkeletonBoneFactory
{
private:
  csString name;
  csSkeletonFactory *skeleton_factory;
  csSkeletonBoneFactory* parent;
  csArray<csSkeletonBoneFactory *> bones;
  csReversibleTransform transform;
  csReversibleTransform full_transform;
  csBox3 skin_box;
  csRef<iSkeletonBoneRagdollInfo> ragdoll_info;
public:
  void UpdateBones();

  void AddBone (csSkeletonBoneFactory* bone) { bones.Push (bone); }
  csArray<csSkeletonBoneFactory *>& GetBones () { return bones; }

  //------------------------------------------------------------------------

  csSkeletonBoneFactory (csSkeletonFactory *);
  virtual ~csSkeletonBoneFactory ();

  SCF_DECLARE_IBASE;
  virtual const char* GetName () const { return name.GetData(); }
  virtual void SetName (const char* name){ csSkeletonBoneFactory::name = name; }
  virtual csReversibleTransform &GetTransform () { return transform; }
  virtual void SetTransform (const csReversibleTransform &transform) 
  { csSkeletonBoneFactory::transform = transform; }
  virtual csReversibleTransform &GetFullTransform () { return full_transform; }
  virtual  void SetParent (iSkeletonBoneFactory *par);
  virtual iSkeletonBoneFactory* GetParent () { return parent; }
  virtual int GetChildrenCount () { return (int)bones.Length (); }
  virtual iSkeletonBoneFactory *GetChild (size_t i) { return bones[i]; }
  virtual iSkeletonBoneFactory *FindChild (const char *name);
  virtual size_t FindChildIndex (iSkeletonBoneFactory *child);
  virtual void SetSkinBox (csBox3 & box) { skin_box = box; }
  virtual csBox3 & GetSkinBox () { return skin_box; }
    virtual iSkeletonBoneRagdollInfo *GetRagdollInfo()
  { return ragdoll_info; }
};

//---------------------------------iSkeletonBoneRagdollInfo ----------

class csSkeletonBoneRagdollInfo : public iSkeletonBoneRagdollInfo
{
private:
  bool enabled;
  csString geom_name, body_name, joint_name;
  bool attach_to_parent;
  int geom_type;
  csVector3 dimensions;
  float friction;
  float elasticity;
  float softness;
  float slip;
  float mass;
  int gravmode;
  csVector3 min_rot_constrints, max_rot_constrints;
  csVector3 min_trans_constrints, max_trans_constrints;
  csSkeletonBoneFactory *skel_bone_fact;
public:
  csSkeletonBoneRagdollInfo (csSkeletonBoneFactory *bone_fact);
  virtual ~csSkeletonBoneRagdollInfo ();

  SCF_DECLARE_IBASE;
  virtual void SetEnabled(bool enabled)
  { csSkeletonBoneRagdollInfo::enabled = enabled; }
  virtual bool GetEnabled()
  { return enabled; }
  virtual void SetAttachToParent(bool attach);
  virtual bool GetAttachToParent()
  { return attach_to_parent; }
  
  virtual void SetGeomName(const char *name)
  { geom_name = name; }
  virtual const char *GetGeomName()
  { return geom_name; }
  virtual void SetGeomType(int geom_type)
  { csSkeletonBoneRagdollInfo::geom_type = geom_type; }
  virtual int GetGeomType()
  { return geom_type; }
  virtual void SetGeomDimensions(csVector3 &size)
  { dimensions = size; }
  virtual csVector3 &GetGeomDimensions()
  { return dimensions; }
  
  virtual void SetFriction(float friction)
  { csSkeletonBoneRagdollInfo::friction = friction; }
  virtual float GetFriction()
  { return friction; }
  virtual void SetElasticity(float elasticity)
  { csSkeletonBoneRagdollInfo::elasticity = elasticity; }
  virtual float GetElasticity()
  { return elasticity; }
  virtual void SetSoftness(float softness)
  { csSkeletonBoneRagdollInfo::softness = softness; }
  virtual float GetSoftness()
  { return softness; }
  virtual void SetSlip(float slip)
  { csSkeletonBoneRagdollInfo::slip = slip; }
  virtual float GetSlip()
  { return slip; }
  
  virtual void SetBodyName(const char *name)
  { body_name = name; }
  virtual const char *GetBodyName()
  { return body_name; }
  virtual void SetBodyMass(float mass)
  { csSkeletonBoneRagdollInfo::mass = mass; }
  virtual float GetBodyMass()
  { return mass; }
  virtual void SetBodyGravmode(int gravmode)
  { csSkeletonBoneRagdollInfo::gravmode = gravmode; }
  virtual int GetBodyGravmode()
  { return gravmode; }
  
  virtual void SetJointName(const char *name)
  { joint_name = name; }
  virtual const char *GetJointName()
  { return joint_name; }
  virtual void SetJointMinRotContraints(csVector3 & constraints)
  { min_rot_constrints = constraints; }
  virtual csVector3 & GetJointMinRotContraints()
  { return min_rot_constrints; }
  virtual void SetJointMaxRotContraints(csVector3 & constraints)
  { max_rot_constrints = constraints; }
  virtual csVector3 & GetJointMaxRotContraints()
  { return max_rot_constrints; }
  virtual void SetJointMinTransContraints(csVector3 & constraints)
  { min_trans_constrints = constraints; }
  virtual csVector3 & GetJointMinTransContraints()
  { return min_trans_constrints; }
  virtual void SetJointMaxTransContraints(csVector3 & constraints)
  { max_trans_constrints = constraints; }
  virtual csVector3 & GetJointMaxTransContraints()
  { return max_trans_constrints; }
};


class csSkeletonScriptKeyFrame: public iSkeletonScriptKeyFrame
{
  private:
    csString name;
    csTicks duration;
    struct bone_key_frame
    {
      bool relative;
      csReversibleTransform transform;
      iSkeletonBoneFactory *bone;
    };
    csArray<bone_key_frame> bones_frame_transforms;
    csReversibleTransform fallback_transform;
  public:
    
    csSkeletonScriptKeyFrame (const char* name);
    virtual ~csSkeletonScriptKeyFrame ();

    SCF_DECLARE_IBASE;
    virtual const char* GetName () const { return name; }
    virtual void SetName (const char* name)
      { csSkeletonScriptKeyFrame::name = name; }
    virtual csTicks GetDuration () { return duration; }
    virtual void SetDuration (csTicks time) { duration = time; }
    virtual size_t GetTransformsCount() 
      { return bones_frame_transforms.Length(); }

    virtual void AddTransform(iSkeletonBoneFactory *bone, 
      csReversibleTransform &transform, bool relative)
    {
      bone_key_frame bf;
      bf.transform = transform;
      bf.bone = bone;
	  bf.relative = relative;
      bones_frame_transforms.Push(bf);
    }

    virtual csReversibleTransform & GetTransform(iSkeletonBoneFactory *bone)
    {
      for (size_t i = 0; i < bones_frame_transforms.Length() ; i++ )
      {
        if (bones_frame_transforms[i].bone == bone)
        {
          return bones_frame_transforms[i].transform;
        }
      }
      return fallback_transform;
    }

    virtual void SetTransform(iSkeletonBoneFactory *bone, 
      csReversibleTransform &transform)
    {
      for (size_t i = 0; i < bones_frame_transforms.Length() ; i++ )
      {
        if (bones_frame_transforms[i].bone == bone)
        {
          bones_frame_transforms[i].transform = transform;
        }
      }
    }

  virtual void GetKeyFrameData(size_t i, iSkeletonBoneFactory *& bone_fact, 
    csReversibleTransform & transform, bool & relative)
    {
	  const bone_key_frame & bkf = bones_frame_transforms[i];
	  transform = bkf.transform;
	  bone_fact = bkf.bone;
	  relative = bkf.relative;
    }
};

class csSkeletonScript : public iSkeletonScript
{
private:
  csString name;
  csTicks time, forced_duration;
  bool loop;
  int loop_times;
  csRefArray<csSkeletonScriptKeyFrame> key_frames;
public:

  csSkeletonScript (const char* name);
  virtual ~csSkeletonScript ();

  SCF_DECLARE_IBASE;

  void SetForcedDuration(csTicks new_duration)
  { forced_duration = new_duration; }

  csTicks GetForcedDuration()
  { return forced_duration; }

  //void SetLoop (bool loop) { csSkeletonScript::loop = loop; }
  //bool GetLoop () { return loop; }
  void SetLoopTimes (bool loop_times) 
    { csSkeletonScript::loop_times = loop_times; }
  int GetLoopTimes () { return loop_times; }


  virtual const char* GetName () const { return name; }
  virtual void SetName (const char* name){ csSkeletonScript::name = name; }
  virtual csTicks GetTime () { return time; }
  virtual void SetTime (csTicks time)  { csSkeletonScript::time = time; }
  virtual float GetSpeed () { return time; }
  virtual void SetSpeed (float speed)  {  }
  virtual void SetFactor (float factor) {}
  virtual float GetFactor () { return 0; }
  virtual void SetLoop (bool loop)
  { csSkeletonScript::loop = loop; }
  virtual bool GetLoop ()
  { return loop; }


  virtual iSkeletonScriptKeyFrame *CreateFrame(const char* name);
  virtual size_t GetFramesCount()  { return key_frames.Length(); }
  virtual iSkeletonScriptKeyFrame *GetFrame(size_t i)  { return key_frames[i]; }
  virtual size_t FindFrameIndex(const char *name)  { return 0; }
  virtual void RemoveFrame(size_t i) 
    { key_frames.DeleteIndexFast(i); }
};

struct bone_transform_data
{
  csQuaternion quat;
  csVector3 pos;
  csVector3 axis;
  csReversibleTransform transform;
};

struct sac_transform_execution
{
  csSkeletonBone* bone;
  bone_transform_data* bone_transform;
  csVector3 delta_per_tick;
  csVector3 final_position;
  csVector3 position;
  csQuaternion quat;
  csQuaternion curr_quat;
  csTicks elapsed_ticks;
  int type;
};

class csSkeletonRunnable
{
public:
  typedef csHash<bone_transform_data*, csPtrKey<csSkeletonBoneFactory> > 
    TransformHash;
private:
  csSkeleton *skeleton;
  csSkeletonScript* script;
  size_t current_instruction;
  int current_frame;
  float morph_factor;
  float time_factor;
  csQuaternion zero_quat;
  int loop_times;

  csArray<sac_transform_execution> runnable_transforms;

  struct runnable_frame
  {
    bool active;
    int repeat_times;
  };

  csArray<runnable_frame> runnable_frames;

  struct del
  {
    csTicks current;
    csTicks final;
    csTicks diff;
  } delay;

  bool parse_key_frame;

  csTicks current_ticks;
  TransformHash transforms;

  void release_tranform_data(TransformHash&);
  
  void ParseFrame(csSkeletonScriptKeyFrame *frame);
  csSkeletonScriptKeyFrame *NextFrame();

public:
  csSkeletonScript *GetScript() { return script; }

  bool Do (csTicks elapsed, bool& stop, csTicks & left);

  bone_transform_data *GetBoneTransform(csSkeletonBoneFactory *bone);
  TransformHash& GetTransforms() { return transforms; };

  csSkeletonRunnable (csSkeletonScript* script, csSkeleton *skeleton);
  ~csSkeletonRunnable ();

  const char *GetName () const { return script->GetName (); }
  void SetName (const char* name) { return script->SetName (name); }
  float GetFactor () { return script->GetFactor (); }
  void SetFactor (float factor) { script->SetFactor (factor); }
  csTicks GetTime () { return script->GetTime (); }
  void SetTime (csTicks time) { script->SetTime (time); }
  float GetSpeed () { return script->GetSpeed (); }
  void SetSpeed (float speed) { return script->SetSpeed (speed); }
};

class csSkeleton : public iSkeleton
{
private:
  csString name;
  iObjectRegistry* object_reg;
  csSkeletonFactory *factory;

  csArray<csSkeletonRunnable> running_scripts;
  csArray<csString> pending_scripts;

  csTicks last_update_time;
  uint32 last_version_id;
  csTicks elapsed;

  static csArray<csReversibleTransform> bone_transforms;
  csRefArray<csSkeletonBone> bones;
  csRefArray<csSkeletonSocket> sockets;
  csArray<size_t> parent_bones;

  iSkeletonScriptCallback *script_callback;

  csRefArray<iSkeletonUpdateCallback> update_callbacks;

  //iODEDynamicSystem *dynamic_system;

  bool bones_updated;
  bool force_bone_update;
  void UpdateBones ();
  void UpdateSockets ();

public:

  iSkeletonScriptCallback *GetScriptCallback() 
  { return script_callback; }

  bool UpdateAnimation (csTicks current);

  csRefArray<csSkeletonBone>& GetBones () 
  { return bones; }
  csArray<size_t>& GetParentBones () 
  { return parent_bones; }
  csArray<csSkeletonRunnable> & GetRunningScripts () 
  { return running_scripts; }

  void SetForceUpdate(bool force_update)
  { force_bone_update = force_update; }

  csSkeleton (csSkeletonFactory* fact);
  virtual ~csSkeleton ();

  SCF_DECLARE_IBASE;

  virtual const char* GetName () const { return name; }
  virtual void SetName (const char* name) 
    { csSkeleton::name = name; }
  virtual size_t GetBonesCount () { return bones.Length (); }
  virtual iSkeletonBone *GetBone (size_t i) { return bones[i]; }
  virtual iSkeletonBone *FindBone (const char *name);

  virtual iSkeletonScript* Execute (const char *scriptname);
  virtual iSkeletonScript* Append (const char *scriptname);
  virtual void ClearPendingScripts ()
  { pending_scripts.DeleteAll(); }
  virtual size_t GetScriptsCount () { return running_scripts.Length (); }
  virtual iSkeletonScript* GetScript (size_t i);
  virtual iSkeletonScript* FindScript (const char *scriptname);
  virtual void StopAll ();
  virtual void Stop (const char* scriptname);
  virtual void Stop (iSkeletonScript *script);
  virtual size_t FindBoneIndex (const char* bonename);
  virtual iSkeletonFactory *GetFactory() 
  { return (iSkeletonFactory *)(csSkeletonFactory*)factory; }
    virtual void SetScriptCallback(iSkeletonScriptCallback *cb)
  { script_callback = cb; }
    virtual iSkeletonSocket* FindSocket (const char *socketname);
    //virtual void CreateRagdoll(iODEDynamicSystem *dyn_sys, csReversibleTransform & transform);
  //virtual void DestroyRagdoll();


  virtual size_t AddUpdateCallback(iSkeletonUpdateCallback *update_callback)
  { return update_callbacks.Push(update_callback); }
    virtual size_t GetUpdateCallbacksCount()
  { return update_callbacks.Length(); }
  virtual iSkeletonUpdateCallback *GetUpdateCallback(size_t callback_idx)
  { return update_callbacks[callback_idx]; }
  virtual void RemoveUpdateCallback(size_t callback_idx)
  { update_callbacks.DeleteIndexFast(callback_idx); }
};

class csSkeletonFactory : public iSkeletonFactory
{
private:
  csString name;
  csSkeletonGraveyard* graveyard;
  iObjectRegistry* object_reg;
  csStringArray autorun_scripts;

  csRefArray<csSkeletonBoneFactory> bones;
  csRefArray<csSkeletonSocketFactory> sockets;
  csArray<size_t> parent_bones;
  csRefArray<csSkeletonScript> scripts;

public:

  csSkeletonFactory (csSkeletonGraveyard* graveyard,
    iObjectRegistry* object_reg);
  virtual ~csSkeletonFactory ();
  SCF_DECLARE_IBASE;

  virtual iSkeletonBoneFactory *CreateBone(const char *name);
  virtual const char* GetName () const { return name; }
  virtual void SetName (const char* name) 
    { csSkeletonFactory::name = name; }
  size_t FindBoneIndex (csSkeletonBoneFactory *bone) const;

  void UpdateParentBones ();

  csRefArray<csSkeletonBoneFactory>& GetBones () { return bones; }
  csRefArray<csSkeletonSocketFactory>& GetSockets () { return sockets; }
  csArray<size_t>& GetParentBones () { return parent_bones; }
  virtual iSkeletonScript *CreateScript(const char *name);
  virtual iSkeletonScript *FindScript(const char *name);
  virtual iSkeletonBoneFactory *FindBone (const char *name);
  virtual iSkeletonGraveyard *GetGraveyard  () { return (iSkeletonGraveyard *)graveyard; }
  virtual size_t FindBoneIndex (const char* bonename);

  virtual iSkeletonSocketFactory *CreateSocket(const char *name, iSkeletonBoneFactory *bone);
  virtual iSkeletonSocketFactory *FindSocket(const char *name);
  virtual iSkeletonSocketFactory *GetSocket (int i);
  virtual void RemoveSocket (int i);
  virtual size_t GetSocketsCount();
};

class csSkeletonSocketFactory : public iSkeletonSocketFactory
{
private:
  csString name;
  csReversibleTransform transform;
  csReversibleTransform full_transform;
  iSkeletonBoneFactory *bone;
public:
  csSkeletonSocketFactory (const char *name, iSkeletonBoneFactory *bone);
  virtual ~csSkeletonSocketFactory ();
  SCF_DECLARE_IBASE;
  virtual const char* GetName () const
  { return name; }
  virtual void SetName (const char* name)
  { csSkeletonSocketFactory::name = name; }
  virtual csReversibleTransform &GetTransform ()
  { return transform; }
  virtual void SetTransform (const csReversibleTransform &transform)
  { csSkeletonSocketFactory::transform = transform; }
  virtual csReversibleTransform &GetFullTransform ()
  { return full_transform; }
  virtual void SetBone (iSkeletonBoneFactory *bone)
  { csSkeletonSocketFactory::bone = bone; }
  virtual iSkeletonBoneFactory *GetBone ()
  { return bone; }
};

class csSkeletonGraveyard : public iSkeletonGraveyard
{
private:
  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  csRefArray<csSkeletonFactory> factories;
  csRefArray<csSkeleton> skeletons;
  csEventID PreProcess;
public:
  csSkeletonGraveyard (iBase*);
  virtual ~csSkeletonGraveyard ();
  bool Initialize (iObjectRegistry* object_reg);
  bool HandleEvent (iEvent& ev);

  virtual int GetFactoriesCount() { return factories.Length(); }
  virtual iSkeletonFactory *CreateFactory(const char *name);
  virtual iSkeletonFactory *LoadFactory(const char *file_name) { return 0; }
  virtual iSkeletonFactory *FindFactory(const char *name) { return 0; }
  virtual iSkeleton *CreateSkeleton(iSkeletonFactory *fact, const char *name);

  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSkeletonGraveyard);
    virtual bool Initialize (iObjectRegistry* object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;

  struct EventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSkeletonGraveyard);
    virtual bool HandleEvent (iEvent &event)
    { return scfParent->HandleEvent (event); }
    CS_EVENTHANDLER_NAMES("crystalspace.skeleton.graveyard")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  } scfiEventHandler;
  friend struct EventHandler;

};

#endif // __CS_SKELETON_H__
