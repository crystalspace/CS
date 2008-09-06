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

#include "cssysdef.h"

#include "csgeom/math3d.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/eventhandlers.h"
#include "csutil/util.h"
#include "imap/services.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "csgeom/tri.h"

#include "skeleton.h"



SCF_IMPLEMENT_FACTORY (csSkeletonGraveyard)

//-------------------------------iSkeletonBone------------------------------------------

csSkeletonBone::csSkeletonBone (csSkeleton *skeleton,
  csSkeletonBoneFactory *factory_bone) :
  scfImplementationType(this)
{
  parent = 0;
  csSkeletonBone::skeleton = skeleton;
  csSkeletonBone::factory_bone = factory_bone;
  name = factory_bone->GetName();
  SetTransform(factory_bone->GetTransform());
  next_transform = factory_bone->GetTransform();
  full_transform = factory_bone->GetFullTransform();
  skin_box = factory_bone->GetSkinBox();
  cb.AttachNew(new csSkeletonBoneDefaultUpdateCallback());
  transform_mode = CS_BTT_SCRIPT;
  //rigid_body = 0;
  //joint = 0;
}

csSkeletonBone::~csSkeletonBone ()
{
}

iSkeletonBoneFactory *csSkeletonBone::GetFactory() 
{
  return static_cast<iSkeletonBoneFactory*> (factory_bone);
}

void csSkeletonBone::SetParent (iSkeletonBone *par)
{
  if (parent && (par != parent))
  {
    size_t child_index = parent->FindChildIndex((iSkeletonBone *)this);
    if (child_index != csArrayItemNotFound)
    {
      parent->GetBones().DeleteIndexFast(child_index);
    }
  }
  parent = (csSkeletonBone *)par; 
  if (parent)
  {
    parent->AddBone(this);
  }
}

iSkeletonBone *csSkeletonBone::FindChild (const char *name)
{
  for (size_t i = 0; i < bones.GetSize () ; i++)
  {
    if (!strcmp (bones[i]->GetName (), name))
    {
      return bones[i];
    }
  }
  return 0;
}

size_t csSkeletonBone::FindChildIndex (iSkeletonBone *child)
{
  for (size_t i = 0; i < bones.GetSize () ; i++)
  {
    if (bones[i] == (csSkeletonBone *)child)
    {
      return i;
    }
  }
  return csArrayItemNotFound;
}

void csSkeletonBone::UpdateTransform ()
{
  size_t scripts_len = skeleton->GetRunningScripts ().GetSize ();
  if (!scripts_len || skeleton->IsInInitialState ()) 
  {
    next_transform = transform;
    return;
  }

  if (scripts_len == 1)
  {
    csSkeletonAnimationInstance *script = skeleton->GetRunningScripts ().Get (0);
    csSkeletonAnimationInstance::TransformHash& transforms = script->GetTransforms ();
    bone_transform_data *b_tr = transforms.Get (factory_bone, 0);
    if (b_tr)
    {
      rot_quat = b_tr->quat;
      next_transform.SetO2T (csMatrix3 (rot_quat));
      next_transform.SetOrigin(b_tr->pos);
    }
  }
  else
  {
    // blend_factor is a ratio of the existing weights
    float total_factor = 0;
    size_t num_anims = 0;
    // compute the total factor of all running animations
    for (size_t i = 0; i < scripts_len; i++)
    {
      csSkeletonAnimationInstance *script = skeleton->GetRunningScripts ().Get (i);
      csSkeletonAnimationInstance::TransformHash& transforms = script->GetTransforms ();
      // does this animation affect this bone?
      if (transforms.Get (factory_bone, 0))
      {
        // accumulate the factors
        total_factor += script->GetFactor ();
        num_anims++;
      }
    }
    // now each time we use the blend factor, we will divide it
    // by total_factor to obtain a ratio

    // the end quaternion used
    csQuaternion quat;
    // the final position used
    csVector3 pos (0);

    for (size_t i = 0; i < scripts_len; i++)
    {
      csSkeletonAnimationInstance *script = skeleton->GetRunningScripts ().Get (i);
      csSkeletonAnimationInstance::TransformHash& transforms = script->GetTransforms ();
      bone_transform_data *b_tr = transforms.Get (factory_bone, 0);
      // we can either use the factor of the animation, or if it has none
      // make sure there are no other factors for any other animations
      if (b_tr && ((script->GetFactor () > 0) || total_factor <= 0))
      {
        // interpolation ratio
        float i;
        // if anims are using factors then compute this factor as a ratio
        // compared to the total factor overall
        if (total_factor > 0)
          i = script->GetFactor () / total_factor;
        // else, we can share the animation equally between all running
        // animations on this bone
        else
          i = 1.0f / num_anims;
        // interpolate the position
        pos = (pos * (1 - i) + b_tr->pos * i);
        quat = quat.SLerp (b_tr->quat, i);
      }
    }
    // use the transforms we calculated
    next_transform.SetO2T (csMatrix3 (quat));
    next_transform.SetOrigin (pos);
  }
}

void csSkeletonBone::UpdateBones ()
{
  if (!parent) 
  {
    switch (transform_mode)
    {
    case CS_BTT_NONE:
    case CS_BTT_SCRIPT:
      full_transform = transform;
      break;
    case CS_BTT_RIGID_BODY:
      full_transform = transform;
      break;
    }
  }

  csArray<csSkeletonBone *>::Iterator it = bones.GetIterator ();
  while (it.HasNext ())
  {
    csSkeletonBone *bone = it.Next ();
    switch (transform_mode)
    {
    case CS_BTT_NONE:
    case CS_BTT_SCRIPT:
      bone->GetFullTransform () = bone->GetTransform ()*full_transform;
      break;
    case CS_BTT_RIGID_BODY:
      /*
      if (bone->GetRigidBody())
      {
        bone->GetFullTransform () = 
        bone->GetRigidBody()->GetTransform ()/skeleton->GetBone(0)->GetRigidBody()->GetTransform();
      }
      else
      {
        bone->GetFullTransform () = bone->GetTransform ()*full_transform;
      }
      */
      break;
    }
    bone->UpdateBones ();
  }
}

void csSkeletonBone::UpdateBones (csSkeletonBone*)
{
  // TODO - Ragdoll
}

//-------------------------------iSkeletonBoneFactory------------------------------------------

csSkeletonBoneFactory::csSkeletonBoneFactory (
  csSkeletonFactory *skeleton_factory) :
  scfImplementationType(this)
{
  csSkeletonBoneFactory::skeleton_factory = skeleton_factory;
  parent = 0;
  skin_box.Set(-0.1f, -0.1f, -0.1f, 0.1f, 0.1f, 0.1f);
  ragdoll_info.AttachNew(new csSkeletonBoneRagdollInfo(this));
}

csSkeletonBoneFactory::~csSkeletonBoneFactory()
{
}

iSkeletonBoneFactory *csSkeletonBoneFactory::FindChild (const char *name)
{
  for (size_t i = 0; i < bones.GetSize () ; i++)
  {
    if (!strcmp (bones[i]->GetName (), name))
    {
      return (iSkeletonBoneFactory *)bones[i];
    }
  }
  return 0;
}

size_t csSkeletonBoneFactory::FindChildIndex (iSkeletonBoneFactory *child)
{
  for (size_t i = 0; i < bones.GetSize () ; i++)
  {
    if (bones[i] == (csSkeletonBoneFactory *)child)
    {
      return i;
    }
  }
  return csArrayItemNotFound;
}

void csSkeletonBoneFactory::SetParent (iSkeletonBoneFactory *par)
{
  if (parent && (par != parent))
  {
    size_t child_index = parent->FindChildIndex((iSkeletonBoneFactory *)this);
    if (child_index != csArrayItemNotFound)
    {
      parent->GetBones().DeleteIndexFast(child_index);
    }
  }
  parent = (csSkeletonBoneFactory *)par; 
  if (parent)
  {
    parent->AddBone(this);
  }
}

void csSkeletonBoneFactory::UpdateBones ()
{
  if (!parent) full_transform = transform;

  csArray<csSkeletonBoneFactory *>::Iterator it = bones.GetIterator ();
  while (it.HasNext ())
  {
    csSkeletonBoneFactory *bone = it.Next ();
    bone->GetFullTransform () = bone->GetTransform ()*full_transform;
    bone->UpdateBones ();
  }
}

csSkeletonBoneRagdollInfo::csSkeletonBoneRagdollInfo (
  csSkeletonBoneFactory *bone_fact) :
  scfImplementationType(this)
{
  geom_name = "";
  body_name = "";
  joint_name = "";
  skel_bone_fact = bone_fact;
  enabled = true;
  attach_to_parent = false;
  geom_type = CS_BGT_BOX;
  dimensions.Set(0, 0, 0);
  friction = 1000;
  elasticity = 0;
  softness = 0.01f;
  slip = 0.07f;
  mass = 1;
  gravmode = 1;
  min_rot_constrints.Set(0, 0, 0);
  max_rot_constrints.Set(0, 0, 0);
  min_trans_constrints.Set(0, 0, 0);
  max_trans_constrints.Set(0, 0, 0);
}

csSkeletonBoneRagdollInfo::~csSkeletonBoneRagdollInfo ()
{
}

void csSkeletonBoneRagdollInfo::SetAttachToParent(bool attach)
{
  attach_to_parent = attach; 
  /*
  if (skel_bone_fact->GetParent())
  {
    iSkeletonBoneRagdollInfo *parent_ragdoll_info = 
      skel_bone_fact->GetParent()->GetRagdollInfo();
    if (parent_ragdoll_info)
    {
      csVector3 parent_geom_dim = parent_ragdoll_info->GetGeomDimensions();
      csVector3 tr_parent_geom_dim = skel_bone_fact->GetTransform().This2Other
        (parent_geom_dim);
    }
  }
  */
  //enabled = false;
}


//--------------------------iSkeletonSocket-----------------------------------

csSkeletonSocket::csSkeletonSocket (csSkeleton *skeleton,
  csSkeletonSocketFactory *socket_factory) :
  scfImplementationType(this)
{
  node = 0;
  transform = socket_factory->GetTransform();
  csSkeletonSocket::name = socket_factory->GetName();
  csSkeletonSocket::factory = socket_factory;
}

csSkeletonSocket::~csSkeletonSocket () 
{
}

iSkeletonSocketFactory *csSkeletonSocket::GetFactory ()
{
  return static_cast<iSkeletonSocketFactory*> (factory);
}

//--------------------------iSkeletonSocketFactory-----------------------------------

csSkeletonSocketFactory::csSkeletonSocketFactory (const char *name,
  iSkeletonBoneFactory *bone) :
  scfImplementationType(this)
{
  csSkeletonSocketFactory::name = name;
  csSkeletonSocketFactory::bone = bone;
}

csSkeletonSocketFactory::~csSkeletonSocketFactory () 
{
}


//--------------------------iSkeletonAnimationKeyFrame-----------------------------------

csSkeletonAnimationKeyFrame::csSkeletonAnimationKeyFrame (const char* name) :
  scfImplementationType(this)
{
  csSkeletonAnimationKeyFrame::name = name;
  duration = 0;
}

csSkeletonAnimationKeyFrame::~csSkeletonAnimationKeyFrame () 
{
}

//--------------------------iSkeletonAnimation-----------------------------------

csSkeletonAnimation::csSkeletonAnimation (csSkeletonFactory *factory, const char* name) :
  scfImplementationType(this)
{
  csSkeletonAnimation::name = name;
  loop = false; //for now
  loop_times = -1;
  fact = factory;
  time_factor = 1;
}

csSkeletonAnimation::~csSkeletonAnimation () 
{
}

iSkeletonAnimationKeyFrame *csSkeletonAnimation::CreateFrame(const char* name)
{
  csRef<csSkeletonAnimationKeyFrame> key_frame;
  key_frame.AttachNew(new csSkeletonAnimationKeyFrame (name));
  key_frames.Push(key_frame);
  return key_frame;
}
csTicks csSkeletonAnimation::GetFramesTime ()
{
  csTicks time = 0;
  for (size_t i = 0; i < key_frames.GetSize (); i++)
  {
    time += key_frames[i]->GetDuration ();
  }
  return time;
}
csTicks csSkeletonAnimation::GetTime () 
{
  return (csTicks) (GetFramesTime () * time_factor);
}
void csSkeletonAnimation::SetTime (csTicks time)
{
  float f_time = GetFramesTime ();
  if (f_time)
  {
    time_factor = time / f_time;
  }
}
void csSkeletonAnimation::RecalcSpline()
{
  const csRefArray<csSkeletonBoneFactory>& bones = fact->GetBones ();
  for (size_t i = 0; i < bones.GetSize () ; i++ )
  {
    csArray<bone_key_info *> tmp_arr;
    for (size_t j= 0; j < key_frames.GetSize (); j++ )
    {
      csRef<csSkeletonAnimationKeyFrame> key_frame = key_frames[j];
      bone_key_info & bt = key_frame->GetKeyInfo(bones[i]);
      tmp_arr.Push(&bt);
    }

    if (tmp_arr.GetSize () < 2)
    {
      continue;
    }

    //check for backward quaternions
    for (size_t j = 0; j < tmp_arr.GetSize () - 1; j++)
    {
      csQuaternion q1 = tmp_arr[j]->rot;
      csQuaternion q2 = tmp_arr[j+1]->rot;
      float a = (q1-q2).SquaredNorm ();
      float b = (q1+q2).SquaredNorm ();
      if (a > b)
      {
        tmp_arr[j+1]->rot = -1*q2;
      }
    }

    //build the spline
    size_t num = tmp_arr.GetSize ();
    for(size_t j = 0; j < num; j++)
    {
      csQuaternion p1, p2;

      bone_key_info& boneinfo = *tmp_arr[j];
      const csQuaternion& p = boneinfo.rot;

      csQuaternion inv = p.GetConjugate();
      if (j == 0)
      {        
        if (loop)
        {
          p1 = tmp_arr[1]->rot;
          p2 = tmp_arr[num-1]->rot;
        }
        else
        {
          // No difference to use, reuse ourselves as tangent
          boneinfo.tangent = p;
          continue;
        }
      }
      else if (j == (num-1))
      {
        if (loop)
        {
          p1 = tmp_arr[0]->rot;
          p2 = tmp_arr[j-1]->rot;
        }
        else
        {
          // No difference to use, reuse ourselves as tangent
          boneinfo.tangent = p;
          continue;
        }
      }
      else
      {
        p1 = tmp_arr[j+1]->rot;
        p2 = tmp_arr[j-1]->rot;
      }

      p1 = (inv * p1).Log();
      p2 = (inv * p2).Log();

      tmp_arr[j]->tangent = p*((p1 + p2)/-4.0f).Exp ();
    }
  }
}

//------------------------csSkeletonAnimationInstance-------------------------------------

csSkeletonAnimationInstance::csSkeletonAnimationInstance (csSkeletonAnimation* animation,
                                                          csSkeleton *skeleton) : 
                                                          scfImplementationType(this)
{
  csSkeletonAnimationInstance::animation = animation;
  csSkeletonAnimationInstance::skeleton = skeleton;
  current_instruction = 0;
  current_frame = -1;
  current_frame_time = 0;
  current_frame_duration = 0;
  blend_factor = 1;
  time_factor = 1;
  current_frame_duration = 0;
  anim_state = CS_ANIM_STATE_PARSE_NEXT;

  loop_times = animation->GetLoopTimes ();
  if (!animation->GetLoop())
  {
    loop_times = 1;
  }

  /*
  csTicks forced_duration = script->GetForcedDuration ();
  if (forced_duration)
  {
    SetTime (forced_duration);
  }
  */
}

csSkeletonAnimationInstance::~csSkeletonAnimationInstance ()
{
  release_tranform_data (transforms);
}

void csSkeletonAnimationInstance::ParseFrame(csSkeletonAnimationKeyFrame *frame)
{
  for (size_t i = 0; i < skeleton->GetFactory()->GetBonesCount(); i++)
  {
    iSkeletonBoneFactory * bone_fact = skeleton->GetFactory()->GetBone(i);

    //key frame bone data
    csQuaternion rot, tangent;
    csVector3 pos;
    bool relative;
    if (frame->GetKeyFrameData(bone_fact, rot, pos, tangent, relative))
    {
      //current transform data
      bone_transform_data *bone_transform = GetBoneTransform ((csSkeletonBoneFactory *)bone_fact);
      if (!frame->GetDuration())
      {
        //TODO
      }
      else
      {
        sac_transform_execution m;
        m.bone_transform = bone_transform;
        m.elapsed_ticks = 0;
        m.curr_quat = m.bone_transform->quat;
        m.position = bone_transform->pos;
        //m.type = 1;
        m.quat = rot;
        m.tangent = tangent;
        m.final_position = pos;

        csVector3 delta;
        if (relative)
        {
          m.type = sac_transform_execution::CS_TRANS_RELATIVE;
          delta = m.final_position;
        }
        else
        {
          m.type = sac_transform_execution::CS_TRANS_FIXED;
          delta = m.final_position - m.position;
        }

        m.delta_per_tick = delta / (float) (current_frame_duration);
        runnable_transforms.Push (m);
      }
    }
  }
}
csSkeletonAnimationKeyFrame *csSkeletonAnimationInstance::PrevFrame()
{
  size_t frames_count = animation->GetFramesCount();
  
  if (frames_count == 0)
    return 0;

  if (current_frame < 1)
  {
    return 0;
  }
  else
  {
    current_frame--;
  }

  if (skeleton->GetScriptCallback())
  {
    skeleton->GetScriptCallback()->Execute(animation, current_frame);
  }

  return (csSkeletonAnimationKeyFrame *)animation->GetFrame (current_frame);
}
csSkeletonAnimationKeyFrame *csSkeletonAnimationInstance::NextFrame()
{
  size_t frames_count = animation->GetFramesCount();
  
  if (frames_count == 0)
    return 0;

  if (current_frame == -1)
  {
    current_frame = 0;
  }
  else
  {
    current_frame++;
  }

  if ( (size_t)current_frame >= frames_count) 
  {
  	// only decrement the loop_times counter provided the animation
  	// is not looping forever.
    if (!animation->GetLoop() && loop_times > 0)
    {
      loop_times -= 1;
    }
    current_frame = 0;
  }

  if (skeleton->GetScriptCallback())
  {
    skeleton->GetScriptCallback()->Execute(animation, current_frame);
  }

  return (csSkeletonAnimationKeyFrame *)animation->GetFrame (current_frame);
}

void csSkeletonAnimationInstance::release_tranform_data(TransformHash& h)
{
  h.Empty();
}

//static csQuaternion zero_quat = csQuaternion(1.0f);
void csSkeletonAnimationInstance::SetDuration (csTicks time)
{
  float anim_time = animation->GetTime ();
  if (anim_time)
  {
    time_factor = time / anim_time;
  }
}
bool csSkeletonAnimationInstance::Do (long elapsed, bool& stop, long &left)
{
  stop = false;
  bool mod = false;
  size_t i;

  if (anim_state != CS_ANIM_STATE_CURRENT)
  {
    csSkeletonAnimationKeyFrame *frame = (anim_state == CS_ANIM_STATE_PARSE_NEXT)?  NextFrame () :
      PrevFrame ();
    if (!frame) 
    {
      left = 0;
      return false;
    }

    current_frame_duration = (long) ((float)(frame->GetDuration())*time_factor);

    ParseFrame (frame);
    anim_state = CS_ANIM_STATE_CURRENT;
  }
  long time_tmp = (current_frame_time == 0 && elapsed < 0)?  current_frame_duration + elapsed : 
                                                             current_frame_time + elapsed;
  float delta = 0;

  if (!loop_times)
  {
    stop = true;
  }

  if (time_tmp > current_frame_duration)
  {
    left = time_tmp - current_frame_duration;
    current_frame_time = 0;
    anim_state = CS_ANIM_STATE_PARSE_NEXT;
  }else if (time_tmp < 0)
  {
    left = time_tmp;
    current_frame_time = 0;
    anim_state = CS_ANIM_STATE_PARSE_PREV;
  }
  else
  {
    delta = current_frame_duration - current_frame_time;
    current_frame_time = time_tmp;
    left = 0;
  }

  i = runnable_transforms.GetSize ();
  while (i > 0)
  {
    i--;
    sac_transform_execution& m = runnable_transforms[i];
    if (m.type == sac_transform_execution::CS_TRANS_FIXED)
    {
      if (delta)
      {
        csVector3 current_pos = 
          m.final_position - delta * m.delta_per_tick;
        m.bone_transform->pos = current_pos;

        float slerp = 
          (float)current_frame_time / (float) current_frame_duration;
        //m.bone_transform->quat = m.curr_quat.SLerp (m.quat, slerp);
        m.bone_transform->quat = m.curr_quat.Squad(m.bone_transform->tangent, 
          m.tangent, m.quat, slerp);
      }
      else
      {
        m.bone_transform->pos = m.final_position;
        m.bone_transform->quat = m.quat;
        m.bone_transform->tangent = m.tangent;
        runnable_transforms.DeleteIndexFast (i);
      }
    }
    else
    {
      if (delta)
      {
        csVector3 current_pos = 
          (float) (current_frame_time - m.elapsed_ticks)*m.delta_per_tick;

        m.bone_transform->pos += current_pos;
        float slerp = 
          ( (float) (current_frame_time - m.elapsed_ticks)/ (float)current_frame_duration);
        csQuaternion zero_quat;
        m.curr_quat = zero_quat.SLerp (m.quat, slerp);
        m.bone_transform->quat = m.curr_quat*m.bone_transform->quat;
        m.elapsed_ticks = current_frame_time;
      }
      else
      {
        csVector3 current_pos = 
          (float) (current_frame_duration - m.elapsed_ticks)*m.delta_per_tick;
        m.bone_transform->pos += current_pos;
        float slerp = 
          ( (float) (current_frame_duration - m.elapsed_ticks)/ (float)current_frame_duration);
        csQuaternion zero_quat;
        m.curr_quat = zero_quat.SLerp (m.quat, slerp);
        m.bone_transform->quat = m.curr_quat*m.bone_transform->quat;
        runnable_transforms.DeleteIndexFast (i);
      }
    }
    mod = true;
  }
  return mod;
}

bone_transform_data *csSkeletonAnimationInstance::GetBoneTransform(
    csSkeletonBoneFactory *bone_fact)
{
  bone_transform_data *b_tr = transforms.Get (bone_fact, 0);
  if (!b_tr) 
  {
    size_t index = (static_cast<csSkeletonFactory*>(skeleton->GetFactory()))
      ->FindBoneIndex(bone_fact);
    b_tr = new bone_transform_data ();
    b_tr->quat = ((csSkeletonBone *)skeleton->GetBone(index))->GetQuaternion ();
    b_tr->tangent = b_tr->quat;
    b_tr->pos = ((csSkeletonBone *)skeleton->GetBone(index))
      ->GetTransform ().GetOrigin ();
    transforms.Put (bone_fact, b_tr);
  }
  return b_tr;
}

//---------------------- iSkeleton ---------------------------------------

csSkeleton::csSkeleton(csSkeletonFactory* fact) :
  scfImplementationType(this)
{
  factory = fact;
  script_callback = 0;
  //dynamic_system = 0;
  csRefArray<csSkeletonBoneFactory>& fact_bones = fact->GetBones ();
  csRefArray<csSkeletonSocketFactory>& fact_sockets = fact->GetSockets ();
  for (size_t i = 0; i < fact_bones.GetSize (); i++ )
  {
    csRef<csSkeletonBone> bone;
    bone.AttachNew(new csSkeletonBone (this, fact_bones[i]));
    bone->SetName(fact_bones[i]->GetName());
    bones.Push(bone);
  }

  for (size_t i = 0; i < bones.GetSize (); i++ )
  {
    iSkeletonBoneFactory *fact_parent_bone = bones[i]->GetFactory()
      ->GetParent();
    if (fact_parent_bone)
    {
      size_t index = fact->FindBoneIndex((csSkeletonBoneFactory *)
	  fact_parent_bone);
      if (index != csArrayItemNotFound)
      {
        bones[i]->SetParent(bones[index]);
      }
    }
  }

  for (size_t i = 0; i < fact_sockets.GetSize (); i++ )
  {
    csRef<csSkeletonSocket> socket;
    socket.AttachNew(new csSkeletonSocket (this, fact_sockets[i]));

    size_t index = fact->FindBoneIndex((csSkeletonBoneFactory *)
	fact_sockets[i]->GetBone());
    if (index != csArrayItemNotFound)
    {
      socket->SetBone(bones[index]);
    }

    socket->SetName(fact_sockets[i]->GetName());
    sockets.Push(socket);
  }


  csArray<size_t> fact_parent_bones = fact->GetParentBones();
  for (size_t i=0; i < fact_parent_bones.GetSize () ; i++ )
  {
    parent_bones.Push(fact_parent_bones[i]);
  }

  last_update_time = -1;
  last_version_id = (uint32)~0;
  elapsed = 0;

  //create animation for direct pose change

}

csSkeleton::~csSkeleton ()
{
  StopAll();
  if (script_callback)
  {
    delete script_callback;
  }
}

iSkeletonFactory *csSkeleton::GetFactory() 
{
  return static_cast<iSkeletonFactory*> (factory);
}
iSkeletonAnimationInstance *csSkeleton::Play (const char *animation_name)
{
  csSkeletonAnimation* script = (csSkeletonAnimation*)(factory->FindAnimation (animation_name));
  if (!script) 
  {
    //printf("script %s doesn't exist\n", scriptname);
    return 0;
  }
  csRef<csSkeletonAnimationInstance> runnable;
  runnable.AttachNew (new csSkeletonAnimationInstance (script, this));
  running_animations.Push (runnable);
  return runnable;
}
void csSkeleton::Stop (iSkeletonAnimationInstance *anim_instance)
{
  running_animations.Delete ((csSkeletonAnimationInstance*)anim_instance);
}
void csSkeleton::UpdateBones ()
{
  size_t i;
  for (i = 0 ; i < bones.GetSize () ; i++)
  {
    bones[i]->UpdateTransform ();
  }

  for (i = 0 ; i < parent_bones.GetSize () ; i++)
  {
    csRef<csSkeletonBone> parent_bone (bones[parent_bones[i]]);
    {
      parent_bone->UpdateBones ();
      //break;
      //case BM_PHYSICS:
      /*
      if (parent_bone->GetRigidBody ())
      {
      parent_bone->UpdateBones (parent_bone);
      force_bone_update = true;
      }
      else
      {
      parent_bone->UpdateBones ();
      }
      break;
      */
    }
  }

  for (i = 0 ; i < bones.GetSize () ; i++)
  {
    bones[i]->FireCallback ();
  }

  bones_updated = true;
}

void csSkeleton::UpdateSockets ()
{
  for (size_t i = 0; i < sockets.GetSize (); i++)
  {
    sockets[i]->GetFullTransform () = 
		sockets[i]->GetTransform()*sockets[i]->GetBone()->GetFullTransform ();
    if (sockets[i]->GetSceneNode())
    {
      sockets[i]->GetSceneNode()->GetMovable()->SetTransform(
		  sockets[i]->GetFullTransform ());
    }
  }
}

bool csSkeleton::UpdateAnimation (csTicks current)
{
  if (last_update_time == -1) 
  {
    last_update_time = current;
    return false;
  }

  elapsed = current - last_update_time;
  last_update_time = current;

  if (elapsed)
  {
    size_t i;
    for (i = 0; i < update_callbacks.GetSize (); i++)
    {
      update_callbacks[i]->Execute(this, current);
    }

    i = running_animations.GetSize ();
    while (i > 0)
    {
      i--;
      bool stop = false;
      long left;
      if (running_animations[i]->Do (elapsed, stop, left))
      {
        while (left)
        {
          running_animations[i]->Do (left, stop, left);
        }
      }

      if (stop)
      {
        if (script_callback)
        {
          script_callback->OnFinish(running_animations[i]->GetScript());
        }

        running_animations.DeleteIndexFast (i);
      }
    }

    if (!running_animations.GetSize () && pending_scripts.GetSize ())
    {
      Execute(pending_scripts[0]);
      pending_scripts.DeleteIndexFast(0);
    }

    UpdateBones();
    UpdateSockets();
  }

  return true;
}

iSkeletonBone *csSkeleton::FindBone (const char *name)
{
  size_t i;
  for (i = 0 ; i < bones.GetSize () ; i++)
    if (strcmp (bones[i]->GetName (), name) == 0)
      return (iSkeletonBone *)bones[i];
  return 0;
}

iSkeletonAnimation* csSkeleton::Execute (const char *scriptname, float blend_factor)
{
  csSkeletonAnimation* script = (csSkeletonAnimation*)(factory->FindAnimation (
    scriptname));
  if (!script)
  {
    //printf("script %s doesn't exist\n", scriptname);
    return 0;
  }

  csSkeletonAnimationInstance *runnable = new csSkeletonAnimationInstance (
    script, this);
  runnable->SetFactor (blend_factor);
  running_animations.Push (runnable);
  return script;
}

iSkeletonAnimation* csSkeleton::Append (const char *scriptname)
{
  csSkeletonAnimation* script = (csSkeletonAnimation*)(
    factory->FindAnimation (scriptname));
  if (!script) 
  {
    return 0;
  }
  csString script_name = scriptname;
  pending_scripts.Push (script_name);
  return script;
}


iSkeletonAnimation* csSkeleton::FindAnimation (const char *scriptname)
{
  size_t i;
  for (i = 0 ; i < running_animations.GetSize () ; i++)
    if (strcmp (running_animations[i]->GetName (), scriptname) == 0)
      return running_animations[i]->GetScript();
  return 0;
}

iSkeletonSocket* csSkeleton::FindSocket (const char *socketname)
{
  size_t i;
  for (i = 0 ; i < sockets.GetSize () ; i++)
    if (strcmp (sockets[i]->GetName (), socketname) == 0)
      return (iSkeletonSocket*)sockets[i];
  return 0;
}

void csSkeleton::StopAll ()
{
  running_animations.DeleteAll ();
}

void csSkeleton::Stop (const char* scriptname)
{
  size_t i;
  for (i = 0 ; i < running_animations.GetSize () ; i++)
    if (strcmp (running_animations[i]->GetName (), scriptname) == 0)
      running_animations.DeleteIndexFast (i);
}

void csSkeleton::Stop (iSkeletonAnimation *script)
{
  //csSkeletonAnimationInstance *cs_skel_runnable = static_cast<csSkeletonAnimationInstance *> (script);
  //running_animations.DeleteFast ( cs_skel_runnable );
}

iSkeletonAnimation* csSkeleton::GetAnimation (size_t i)
{
  if (i < running_animations.GetSize ())
  {
    return running_animations[i]->GetScript();
  }
  return 0;
}

size_t csSkeleton::FindBoneIndex (const char* bonename)
{
  size_t i;
  for (i = 0 ; i < bones.GetSize () ; i++)
  {
    if (strcmp (bones[i]->GetName (), bonename) == 0)
      return i;
  }
  return csArrayItemNotFound;
}

/*
void csSkeleton::CreateRagdoll(iODEDynamicSystem *dyn_sys, csReversibleTransform & transform)
{
  dynamic_system = dyn_sys;
  for (size_t i = 0; i < bones.GetSize (); i++)
  {
    //printf("\n");
    iSkeletonBoneRagdollInfo *ragdoll_info = bones[i]->GetFactory()->GetRagdollInfo();
    if (!ragdoll_info->GetEnabled())
    {
      continue;
    }
    csReversibleTransform full_body_transform = bones[i]->GetFactory()->GetFullTransform()*transform;
    csBox3 skin_box = bones[i]->GetSkinBox();
    csVector3 skin_size = 
      csVector3(skin_box.GetSize().x < 0.2f ? 0.2f : skin_box.GetSize().x,
      skin_box.GetSize().y < 0.2f ? 0.2f : skin_box.GetSize().y,
      skin_box.GetSize().z < 0.2f ? 0.2f : skin_box.GetSize().z);
    csRef<iODEGeom> geom;
    if (i)
    {
      geom = dyn_sys->CreateGeom();
      geom->QueryObject()->SetName(ragdoll_info->GetGeomName());
      csReversibleTransform center;
      center.SetOrigin(skin_box.GetCenter());
      geom->SetBox(skin_size, center);
      geom->SetFriction(ragdoll_info->GetFriction());
      geom->SetElasticy(ragdoll_info->GetElasticity());
      geom->SetSoftness(ragdoll_info->GetSoftness());
      geom->SetSlip(ragdoll_info->GetSlip());

      if ((i == 0) || (i == 1))
        //if ((i == 1))
      {
        geom->SetCollideBits(COLLIDE_GEOM_LACK);
      }
      else
      {
        geom->SetCollideBits(COLLIDE_GEOM_NORMAL);
      }
      geom->SetCategoryBits(1);
      //geom->SetGroupID(1);
    }

    //printf("creating geom %s for bone %s\n", ragdoll_info->GetGeomName(), bones[i]->GetName());
    csRef<iODERigidBody> rigid_body = dyn_sys->CreateBody();
    rigid_body->QueryObject()->SetName(ragdoll_info->GetBodyName());
    csOrthoTransform tr;
    if (i)
    {
      rigid_body->SetGeom(geom, tr);
    }
    rigid_body->SetProperties(ragdoll_info->GetBodyMass(), csVector3(0), csMatrix3());
    rigid_body->SetTransform(full_body_transform);
    bones[i]->SetRigidBody(rigid_body, tr);
    //printf("creating body %s for bone %s\n", ragdoll_info->GetBodyName(), bones[i]->GetName());
    if (!bones[i]->GetParent())
    {
      continue;
    }

    iSkeletonBone *parent_bone = bones[i]->GetParent();

    csRef<iODEJoint> joint = dyn_sys->CreateJoint();
    joint->QueryObject()->SetName(ragdoll_info->GetJointName());

    //joint->Attach(parent_bone->GetRigidBody(), bones[i]->GetRigidBody());
    csReversibleTransform joint_tr = parent_bone->GetRigidBody()->GetTransform();
    joint_tr.SetOrigin(full_body_transform.GetOrigin());
    full_body_transform.SetO2T(joint_tr.GetO2T());
    joint->SetTransform(full_body_transform);
    //bones[i]->GetRigidBody()->SetTransform(joint_tr);

    csVector3 & min_rot_constr = ragdoll_info->GetJointMinRotContraints();
    csVector3 & max_rot_constr = ragdoll_info->GetJointMaxRotContraints();
    csVector3 & min_tr_constr = ragdoll_info->GetJointMinTransContraints();
    csVector3 & max_tr_constr = ragdoll_info->GetJointMaxTransContraints();

    //printf("Min Rot Contraints: %.3f %.3f %.3f\n", min_rot_constr.x, min_rot_constr.y, min_rot_constr.z);
    //printf("Max Rot Contraints: %.3f %.3f %.3f\n", max_rot_constr.x, max_rot_constr.y, max_rot_constr.z);

    bool trans_constr_x = !(min_tr_constr.x || max_tr_constr.x);
    bool trans_constr_y = !(min_tr_constr.y || max_tr_constr.y);
    bool trans_constr_z = !(min_tr_constr.z || max_tr_constr.z);
    bool rot_constr_x = !(min_rot_constr.x || max_rot_constr.x);
    bool rot_constr_y = !(min_rot_constr.y || max_rot_constr.y);
    bool rot_constr_z = !(min_rot_constr.z || max_rot_constr.z);
    if (trans_constr_x && trans_constr_y && trans_constr_z && 
      rot_constr_x && rot_constr_y && rot_constr_z)
    {
      joint->SetTransConstraints(trans_constr_x, trans_constr_y, trans_constr_z);
      joint->SetMinimumDistance(min_tr_constr);
      joint->SetMaximumDistance(max_tr_constr);
      joint->SetRotConstraints(rot_constr_x, rot_constr_y, rot_constr_z);
      joint->SetMinimumAngle(min_rot_constr);
      joint->SetMaximumAngle(max_rot_constr);
      //joint->SetMinimumAngle(csVector3(0, -0.01, 0));
      //joint->SetMaximumAngle(csVector3(0, 0.01, 0));
      joint->Attach(parent_bone->GetRigidBody(), bones[i]->GetRigidBody());
      joint->SetFixed();
      //printf("creating fixed joint %s for bones (%s -> %s)\n", ragdoll_info->GetJointName(), parent_bone->GetName(), bones[i]->GetName());
    }
    else
    {
      joint->SetTransConstraints(trans_constr_x, trans_constr_y, trans_constr_y);
      joint->SetMinimumDistance(min_tr_constr);
      joint->SetMaximumDistance(max_tr_constr);
      //joint->SetRotConstraints(true, false, true);
      //joint->SetMinimumAngle(csVector3(0, -1.5, 0));
      //joint->SetMaximumAngle(csVector3(0, 0, 0));
      joint->SetRotConstraints(rot_constr_x, rot_constr_y, rot_constr_z);
      joint->SetMinimumAngle(min_rot_constr);
      joint->SetMaximumAngle(max_rot_constr);
      //joint->Attach(parent_bone->GetRigidBody(), bones[i]->GetRigidBody());
      joint->Attach(parent_bone->GetRigidBody(), bones[i]->GetRigidBody());
      //joint->SetFixed();
      //printf("creating joint %s for bones (%s -> %s)\n", ragdoll_info->GetJointName(), parent_bone->GetName(), bones[i]->GetName());
    }
    //bones[i]->GetRigidBody()->SetTransform(full_body_transform);
    bones[i]->SetJoint(joint);
    bones[i]->SetTransformMode(CS_BTT_RIGID_BODY);
  }
}

void csSkeleton::DestroyRagdoll()
{
  if (dynamic_system)
  {
    for (size_t i = 0; i < bones.GetSize (); i++)
    {
      if (bones[i]->GetJoint())
      {
        dynamic_system->RemoveJoint(bones[i]->GetJoint());
      }
      if (bones[i]->GetRigidBody())
      {
        dynamic_system->RemoveBody(bones[i]->GetRigidBody());
        bones[i]->SetTransformMode(CS_BTT_SCRIPT);
      }
    }
  }
}
*/

//---------------------- iSkeletonFactory ---------------------------------------

csSkeletonFactory::csSkeletonFactory (csSkeletonGraveyard* graveyard, 
                                      iObjectRegistry* object_reg) :
  scfImplementationType(this)
{
  csSkeletonFactory::graveyard = graveyard;
  csSkeletonFactory::object_reg = object_reg;
}

iSkeletonBoneFactory *csSkeletonFactory::CreateBone(const char *name)
{
  csRef<csSkeletonBoneFactory> bone;
  bone.AttachNew(new csSkeletonBoneFactory(this));
  bone->SetName(name);
  bones.Push(bone);
  return bone;
}

csSkeletonFactory::~csSkeletonFactory ()
{
}

iSkeletonGraveyard *csSkeletonFactory::GetGraveyard  ()
{
  return static_cast<iSkeletonGraveyard*> (graveyard);
}

iSkeletonAnimation* csSkeletonFactory::FindAnimation (const char* scriptname)
{
  size_t i;
  for (i = 0 ; i < scripts.GetSize () ; i++)
  {
    if (strcmp (scripts[i]->GetName (), scriptname) == 0)
    {
      return scripts[i];
    }
  }
  return 0;
}

iSkeletonBoneFactory* csSkeletonFactory::FindBone (const char* name)
{
  size_t i;
  for (i = 0 ; i < bones.GetSize () ; i++)
    if (strcmp (bones[i]->GetName (), name) == 0)
      return bones[i];
  return 0;
}

size_t csSkeletonFactory::FindBoneIndex (const char* bonename)
{
  size_t i;
  for (i = 0 ; i < bones.GetSize () ; i++)
  {
    if (strcmp (bones[i]->GetName (), bonename) == 0)
      return i;
  }
  return csArrayItemNotFound;
}

size_t csSkeletonFactory::FindBoneIndex (csSkeletonBoneFactory *bone) const
{
  size_t i;
  for (i = 0 ; i < bones.GetSize () ; i++)
    if (bones[i] == bone)
      return i;
  return csArrayItemNotFound;
}

void csSkeletonFactory::UpdateParentBones ()
{
  parent_bones.SetSize (0);
  for (size_t i = 0; i < bones.GetSize (); i++)
  {
    if (!bones[i]->GetParent ())
    {
      bones[i]->UpdateBones();
      parent_bones.Push (i);
    }
  }
}

iSkeletonAnimation *csSkeletonFactory::CreateAnimation (const char *name)
{
  csRef<csSkeletonAnimation> script;
  script.AttachNew(new csSkeletonAnimation (this, name));
  scripts.Push(script);
  return script;
}

iSkeletonSocketFactory *csSkeletonFactory::CreateSocket(const char *name, iSkeletonBoneFactory *bone)
{
  csRef<csSkeletonSocketFactory> socket;
  socket.AttachNew(new csSkeletonSocketFactory (name, bone));
  sockets.Push(socket);
  return socket;
}

iSkeletonSocketFactory *csSkeletonFactory::FindSocket(const char *name)
{
  size_t i;
  for (i = 0 ; i < sockets.GetSize () ; i++)
    if (strcmp (sockets[i]->GetName (), name) == 0)
      return sockets[i];
  return 0;
}

iSkeletonSocketFactory *csSkeletonFactory::GetSocket (int i)
{
  return sockets[i];
}

void csSkeletonFactory::RemoveSocket (int)
{
  //TODO
}

size_t csSkeletonFactory::GetSocketsCount()
{
  return sockets.GetSize ();
}

//--------------------------------iSkeletonGraveyard-----------------------------------------

csSkeletonGraveyard::csSkeletonGraveyard (iBase* pParent) :
  scfImplementationType(this, pParent), object_reg(0)
{
  manual_updates = false;
}

csSkeletonGraveyard::~csSkeletonGraveyard ()
{
  skeletons.DeleteAll();
  if (object_reg && evhandler)
  {
    csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (object_reg);
    if (q)
      q->RemoveListener (evhandler);
    evhandler = 0;
  }
}

bool csSkeletonGraveyard::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  Frame = csevFrame (object_reg);
  csRef<iEventQueue> eq (csQueryRegistry<iEventQueue> (object_reg));
  if (eq == 0) return false;
  evhandler.AttachNew (new csSkelEventHandler (this));
  eq->RegisterListener (evhandler, Frame);

  return true;
}

iSkeletonFactory* csSkeletonGraveyard::CreateFactory(const char *name)
{
  csRef<csSkeletonFactory> fact;
  fact.AttachNew(new csSkeletonFactory (this, object_reg));
  fact->SetName(name);
  factories.Push(fact);
  return fact;
}

void csSkeletonGraveyard::AddSkeleton (iSkeleton *skeleton)
{
  skeletons.Push (skeleton);
}

void csSkeletonGraveyard::RemoveSkeleton (iSkeleton* skeleton)
{
  skeletons.Delete (skeleton);
}

iSkeleton *csSkeletonGraveyard::CreateSkeleton(iSkeletonFactory *fact,
    const char *name)
{
  csSkeletonFactory *cs_skel_fact = static_cast<csSkeletonFactory*> (fact);
  cs_skel_fact->UpdateParentBones();
  csRef<csSkeleton> skeleton;
  skeleton.AttachNew (new csSkeleton (cs_skel_fact));
  skeleton->SetName(name);
  skeletons.Push(skeleton);
  return skeleton;
}
void csSkeletonGraveyard::Update (csTicks time)
{
  for (size_t i = 0; i < skeletons.GetSize () ; i++)
  {
    skeletons[i]->UpdateAnimation (time);
  }
}
bool csSkeletonGraveyard::HandleEvent (iEvent& ev)
{
  if (ev.Name == Frame && !manual_updates)
  {
    Update (vc->GetCurrentTicks ());
    return true;
  }
  return false;
}
