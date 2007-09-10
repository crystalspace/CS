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

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csSkeletonGraveyard)

//-------------------------------csSkeletonBone------------------------------------------

csSkeletonBone::csSkeletonBone (csSkeleton *skeleton,
  csSkeletonBoneFactory *factory_bone)
{
  parent = 0;
  csSkeletonBone::skeleton = skeleton;
  csSkeletonBone::factory_bone = factory_bone;
  name = factory_bone->GetName();
  SetTransform(factory_bone->GetTransform());
  full_transform = factory_bone->GetFullTransform();
  skin_box = factory_bone->GetSkinBox();
}

csSkeletonBone::~csSkeletonBone ()
{
}

csSkeletonBoneFactory *csSkeletonBone::GetFactory() 
{
  return static_cast<csSkeletonBoneFactory*> (factory_bone);
}

void csSkeletonBone::SetParent (csSkeletonBone *par)
{
  // remove cyclic contradictory depedencies
  // (child of this bone is also its parent)
  if (parent && (par != parent))
  {
    size_t child_index = parent->FindChildIndex((csSkeletonBone *)this);
    if (child_index != csArrayItemNotFound)
    {
      parent->GetBones().DeleteIndexFast(child_index);
    }
  }
  parent = (csSkeletonBone *)par;
  // add this bone as a child of its parent
  if (parent)
    parent->AddBone(this);
}

csSkeletonBone *csSkeletonBone::FindChild (const char *name)
{
  // find child bone matching the name
  for (size_t i = 0; i < bones.GetSize () ; i++)
  {
    if (!strcmp (bones[i]->GetName (), name))
    {
      return bones[i];
    }
  }
  return 0;
}

size_t csSkeletonBone::FindChildIndex (csSkeletonBone *child)
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
    return;

  if (scripts_len == 1)
  {
    csSkeletonAnimationInstance *script = skeleton->GetRunningScripts ().Get (0);
    csSkeletonAnimationInstance::TransformHash& transforms = script->GetTransforms ();
    bone_transform_data *b_tr = transforms.Get (factory_bone, 0);
    if (b_tr)
    {
      transform.SetO2T (csMatrix3 (b_tr->quat));
      transform.SetOrigin(b_tr->pos);
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
    transform.SetO2T (csMatrix3 (quat));
    transform.SetOrigin (pos);
  }
}

void csSkeletonBone::UpdateBones ()
{
  SetTransform (transform);
  if (!parent)
    full_transform = transform;

  csArray<csSkeletonBone *>::Iterator it = bones.GetIterator ();
  while (it.HasNext ())
  {
    csSkeletonBone *bone = it.Next ();
    bone->GetFullTransform () = bone->GetTransform () * full_transform;
    bone->UpdateBones ();
  }
}

//-------------------------------csSkeletonBoneFactory------------------------------------------

csSkeletonBoneFactory::csSkeletonBoneFactory (
  csSkeletonFactory *skeleton_factory)
{
  csSkeletonBoneFactory::skeleton_factory = skeleton_factory;
  parent = 0;
  skin_box.Set(-0.1f, -0.1f, -0.1f, 0.1f, 0.1f, 0.1f);
}

csSkeletonBoneFactory::~csSkeletonBoneFactory()
{
}

csSkeletonBoneFactory *csSkeletonBoneFactory::FindChild (const char *name)
{
  for (size_t i = 0; i < bones.GetSize () ; i++)
  {
    if (!strcmp (bones[i]->GetName (), name))
    {
      return (csSkeletonBoneFactory *)bones[i];
    }
  }
  return 0;
}

size_t csSkeletonBoneFactory::FindChildIndex (csSkeletonBoneFactory *child)
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

void csSkeletonBoneFactory::SetParent (csSkeletonBoneFactory *par)
{
  if (parent && (par != parent))
  {
    size_t child_index = parent->FindChildIndex((csSkeletonBoneFactory *)this);
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
  csSkeletonBoneFactory *bone) :
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
}

csSkeletonAnimationInstance::~csSkeletonAnimationInstance ()
{
  release_tranform_data (transforms);
}

void csSkeletonAnimationInstance::ParseFrame(csSkeletonAnimationKeyFrame *frame)
{
  for (size_t i = 0; i < skeleton->GetFactory()->GetBonesCount(); i++)
  {
    csSkeletonBoneFactory * bone_fact = skeleton->GetFactory()->GetBone(i);

    //key frame bone data
    csQuaternion rot;
    csVector3 pos;
    if (frame->GetKeyFrameData(bone_fact, rot, pos))
    {
      //current transform data
      bone_transform_data *bone_transform = GetBoneTransform ((csSkeletonBoneFactory *)bone_fact);
      if (!frame->GetDuration())
      {
        //TODO: This seems a bit pointless.
      }
      else
      {
        sac_transform_execution m;
        m.bone_transform = bone_transform;
        m.elapsed_ticks = 0;
        m.prev_quat = bone_transform->quat;
        m.position = bone_transform->pos;

        /*size_t frames_count = animation->GetFramesCount();
        if (frames_count > 0)
        {
          size_t prev_frame = current_frame - 1;
          if (current_frame <= 1)
            prev_frame = frames_count - 1;
          csSkeletonAnimationKeyFrame *pframe =
            (csSkeletonAnimationKeyFrame *)animation->GetFrame (prev_frame);
          if (!pframe->GetKeyFrameData (bone_fact, m.prev_quat, m.position))
        }*/
        m.next_quat = rot;
        m.final_position = pos;

        csVector3 delta = m.final_position - m.position;
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
      loop_times--;
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

  // so are we switching to another keyframe now?
  if (anim_state != CS_ANIM_STATE_CURRENT)
  {
    // usually we go to the next frame
    csSkeletonAnimationKeyFrame *frame = (anim_state == CS_ANIM_STATE_PARSE_NEXT)?  NextFrame () :
      PrevFrame ();
    // ouch
    if (!frame)
    {
      left = 0;
      return false;
    }
    // find the duration of this keyframe
    current_frame_duration = (csTicks) ((float)(frame->GetDuration())*time_factor);

    ParseFrame (frame);
    anim_state = CS_ANIM_STATE_CURRENT;
  }
  // Shouldn't be needed...
  //long time_tmp = (current_frame_time == 0 && elapsed < 0)?  current_frame_duration + elapsed : 
  //                                                           current_frame_time + elapsed;
  // The absolute time from this keyframe so far in ticks
  csTicks abs_time = current_frame_time + elapsed;
  /*printf ("%s -----------------\n", GetName ());
  printf ("current frame time: %ld\n", current_frame_time);
  printf ("current frame dur:  %ld\n", current_frame_duration);
  printf ("current frame: %i\n", current_frame);
  printf ("elapsed: %ld\n", elapsed);
  printf ("-----------------\n");*/
  // how far we must interpolate along the keyframe
  float delta = 0;
  // stop looping!
  if (!loop_times)
  {
    stop = true;
  }
  // if the time from the last keyframe is greater than the length of time
  // between this and the previous keyframe then we can switch to the next
  // keyframe already.
  if (abs_time > current_frame_duration)
  {
    // how long (in ticks) is left until the next keyframe
    left = abs_time - current_frame_duration;
    current_frame_time = 0;
    anim_state = CS_ANIM_STATE_PARSE_NEXT;
  }
  else
  {
    delta = current_frame_duration - current_frame_time;
    current_frame_time = abs_time;
    left = 0;
  }

  i = runnable_transforms.GetSize ();
  while (i > 0)
  {
    i--;
    sac_transform_execution& m = runnable_transforms[i];
    // if we need to interpolate
    if (delta)
    {
      csVector3 current_pos = 
        m.final_position - delta * m.delta_per_tick;
      m.bone_transform->pos = current_pos;

      // use slerp interpolation
      float slerp =
        (float)current_frame_time / (float) current_frame_duration;
      m.bone_transform->quat = m.prev_quat.SLerp (m.next_quat, slerp);
    }
    else
    {
      // otherwise just use end keyframe
      m.bone_transform->pos = m.final_position;
      m.bone_transform->quat = m.next_quat;
      runnable_transforms.DeleteIndexFast (i);
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
    csSkeletonBoneFactory *fact_parent_bone = bones[i]->GetFactory()
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
    parent_bone->UpdateBones ();
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
      // 
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

csSkeletonBone *csSkeleton::FindBone (const char *name)
{
  size_t i;
  for (i = 0 ; i < bones.GetSize () ; i++)
    if (strcmp (bones[i]->GetName (), name) == 0)
      return (csSkeletonBone *)bones[i];
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

//---------------------- iSkeletonFactory ---------------------------------------

csSkeletonFactory::csSkeletonFactory (csSkeletonGraveyard* graveyard, 
                                      iObjectRegistry* object_reg) :
  scfImplementationType(this)
{
  csSkeletonFactory::graveyard = graveyard;
  csSkeletonFactory::object_reg = object_reg;
}

csSkeletonBoneFactory *csSkeletonFactory::CreateBone(const char *name)
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

csSkeletonBoneFactory* csSkeletonFactory::FindBone (const char* name)
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

iSkeletonSocketFactory *csSkeletonFactory::CreateSocket(const char *name, csSkeletonBoneFactory *bone)
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
  PreProcess = csevPreProcess (object_reg);
  csRef<iEventQueue> eq (csQueryRegistry<iEventQueue> (object_reg));
  if (eq == 0) return false;
  evhandler.AttachNew (new csSkelEventHandler (this));
  eq->RegisterListener (evhandler, PreProcess);

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
  if (ev.Name == PreProcess && !manual_updates)
  {
    Update (vc->GetCurrentTicks ());
    return true;
  }
  return false;
}
