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
  for (size_t i = 0; i < bones.Length () ; i++)
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
  for (size_t i = 0; i < bones.Length () ; i++)
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
  size_t scripts_len = skeleton->GetRunningScripts ().Length ();
  if (!scripts_len) return;

  if (scripts_len == 1)
  {
    csSkeletonRunnable &script = skeleton->GetRunningScripts ().Get (0);
    csSkeletonRunnable::TransformHash& transforms = script.GetTransforms ();
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
    csQuaternion q;
    float min = 0; float max = 0; float script_factors_total = 0;
    bool slerp = false; bool updated = false;
    csVector3 final_pos = csVector3 (0);

    for (size_t i = 0; i < scripts_len; i++)
    {
      csSkeletonRunnable &script = skeleton->GetRunningScripts ().Get (i);
      csSkeletonRunnable::TransformHash& transforms = script.GetTransforms ();
      bone_transform_data *b_tr = transforms.Get (factory_bone, 0);
      if (b_tr && (script.GetFactor () > 0))
      {
        final_pos += b_tr->pos*script.GetFactor ();
        script_factors_total += script.GetFactor ();
        if (slerp)
        {
          float max_over_factor = max/script_factors_total;
          if (script.GetFactor () >= min)
          {
            max = script.GetFactor ();
            q = q.SLerp (b_tr->quat, max_over_factor);
          }
          else
          {
            min = script.GetFactor ();
            q = b_tr->quat.SLerp (q, max_over_factor);
          }
          script_factors_total = min + max_over_factor;
        }
        else
        {
          slerp = true;
          min = max = script.GetFactor ();
          q = b_tr->quat;
        }
        updated = true; 
      }
    }

    if (updated)
    {
      rot_quat = q;
      if (script_factors_total)
      {
        final_pos /= script_factors_total;
      }
      next_transform.SetO2T (csMatrix3 (rot_quat));
      next_transform.SetOrigin (final_pos);
    }
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
  for (size_t i = 0; i < bones.Length () ; i++)
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
  for (size_t i = 0; i < bones.Length () ; i++)
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


//--------------------------iSkeletonScriptKeyFrame-----------------------------------

csSkeletonScriptKeyFrame::csSkeletonScriptKeyFrame (const char* name) :
  scfImplementationType(this)
{
  csSkeletonScriptKeyFrame::name = name;
}

csSkeletonScriptKeyFrame::~csSkeletonScriptKeyFrame () 
{
}

//--------------------------iSkeletonScript-----------------------------------

csSkeletonScript::csSkeletonScript (const char* name) :
  scfImplementationType(this)
{
  csSkeletonScript::name = name;
  time = 0;
  loop = false; //for now
  loop_times = -1;
  forced_duration = 0;
}

csSkeletonScript::~csSkeletonScript () 
{
}

iSkeletonScriptKeyFrame *csSkeletonScript::CreateFrame(const char* name)
{
  csRef<csSkeletonScriptKeyFrame> key_frame;
  key_frame.AttachNew(new csSkeletonScriptKeyFrame (name));
  key_frames.Push(key_frame);
  return key_frame;
}

//------------------------csSkeletonRunnable-------------------------------------

csSkeletonRunnable::csSkeletonRunnable (csSkeletonScript* script,
  csSkeleton *skeleton)
{
  csSkeletonRunnable::script = script;
  csSkeletonRunnable::skeleton = skeleton;
  current_instruction = 0;
  current_frame = -1;
  delay.current = 0;
  delay.final = 0;
  delay.diff = 0;
  morph_factor = 1;
  time_factor = 1;
  current_ticks = 0;
  parse_key_frame = true;

  loop_times = script->GetLoopTimes ();
  if (!script->GetLoop())
  {
    loop_times = 1;
  }

  /*
  zero_quat.SetWithEuler (csVector3 (0));

  if (script->GetLoop ())
  {
    loop_times = script->GetLoopTimes ();
  }
  else
  {
    loop_times = 1;
  }

  for (size_t i = 0; i < script->GetFrames ().Length (); i++)
  {
    runnable_frame rf;
    rf.active = script->GetFrames ().Get (i).active;
    rf.repeat_times = script->GetFrames ().Get (i).repeat_times;
    runnable_frames.Push (rf);
  }

  csTicks forced_duration = script->GetForcedDuration ();
  if (forced_duration)
  {
    SetTime (forced_duration);
  }
  */
}

csSkeletonRunnable::~csSkeletonRunnable ()
{
  release_tranform_data (transforms);
}

void csSkeletonRunnable::ParseFrame(csSkeletonScriptKeyFrame *frame)
{
  for (size_t i = 0; i < frame->GetTransformsCount() ; i++)
  {
    iSkeletonBoneFactory * bone_fact;
    csReversibleTransform transform;
    bool relative;
    frame->GetKeyFrameData(i, bone_fact, transform, relative);

    bone_transform_data *bone_transform = GetBoneTransform ((csSkeletonBoneFactory *)bone_fact);
    if (!frame->GetDuration())
    {
      //TODO
    }
    else
    {
      sac_transform_execution m;
      //m.bone = frame->GetBone(i);
      m.bone_transform = bone_transform;
      m.elapsed_ticks = 0;
      m.curr_quat = m.bone_transform->quat;
      m.position = bone_transform->pos;
      m.type = 1;
      //m.quat = csQuaternion (transform.GetT2O());
      m.quat.SetMatrix (transform.GetO2T());
      m.final_position = transform.GetOrigin();

      csVector3 delta;
      if (relative)
      {
        m.type = 2;
        delta = m.final_position;
      }
      else
      {
        m.type = 1;
        delta = m.final_position - m.position;
      }

      m.delta_per_tick = delta/ (float) (delay.final);
      runnable_transforms.Push (m);
    }
  }
}

csSkeletonScriptKeyFrame *csSkeletonRunnable::NextFrame()
{
  size_t frames_count = script->GetFramesCount();
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
    if (loop_times > 0)
    {
      loop_times -= 1;
    }
    current_frame = 0;
  }

  if (skeleton->GetScriptCallback())
  {
    skeleton->GetScriptCallback()->Execute(script, current_frame);
  }

  return (csSkeletonScriptKeyFrame *)script->GetFrame(current_frame);
}

void csSkeletonRunnable::release_tranform_data(TransformHash& h)
{
  h.Empty();
}

//static csQuaternion zero_quat = csQuaternion(1.0f);

bool csSkeletonRunnable::Do (csTicks elapsed, bool& stop, csTicks & left)
{
  stop = false;
  bool mod = false;
  size_t i;
  delay.diff += elapsed;
  if (parse_key_frame)
  {
    csSkeletonScriptKeyFrame *frame = NextFrame ();
    delay.final = (csTicks) ( (float)(frame->GetDuration())*time_factor);
    ParseFrame (frame);
    parse_key_frame = false;
  }


  if (!loop_times)
  {
    stop = true;
  }

  if (delay.diff > delay.final)
  {
    delay.current = delay.final;
    left = delay.diff - delay.final;
    delay.diff = 0;
    parse_key_frame = true;
  }
  else
  {
    delay.current = delay.diff;
    left = 0;
  }

  i = runnable_transforms.Length ();
  while (i > 0)
  {
    i--;
    sac_transform_execution& m = runnable_transforms[i];
    if (m.type == 1)
    {
      if (delay.current < delay.final)
      {
        csVector3 current_pos = 
          m.final_position - ( (float) (delay.final - delay.current))*m.delta_per_tick;
        m.bone_transform->pos = current_pos;

        float slerp = 
          (float)delay.current/ (float)delay.final;
        m.bone_transform->quat = m.curr_quat.SLerp (m.quat, slerp);
      }
      else
      {
        m.bone_transform->pos = m.final_position;
        m.bone_transform->quat = m.quat;
        runnable_transforms.DeleteIndexFast (i);
      }
    }
    else
    {
      if (delay.current < delay.final)
      {
        csVector3 current_pos = 
          (float) (delay.current - m.elapsed_ticks)*m.delta_per_tick;

        //m.bone_transform->pos = current_pos;
        m.bone_transform->pos += current_pos;
        float slerp = 
          ( (float) (delay.current - m.elapsed_ticks)/ (float)delay.final);
        csQuaternion zero_quat;
        

        //m.bone_transform->quat = zero_quat.Slerp (m.quat, slerp);
        m.curr_quat = zero_quat.SLerp (m.quat, slerp);
        m.bone_transform->quat = m.curr_quat*m.bone_transform->quat;
        m.elapsed_ticks = delay.current;
        //printf("m.curr_quat %.3f %.3f %.3f %.3f\n", m.curr_quat.x, m.curr_quat.y, m.curr_quat.z, m.curr_quat.r);
        //printf("m.bone_transform->quat %.3f %.3f %.3f %.3f\n", m.bone_transform->quat.x, m.bone_transform->quat.y, m.bone_transform->quat.z, m.bone_transform->quat.r);
        //printf("-------------------------\n");
      }
      else
      {
        csVector3 current_pos = 
          (float) (delay.final - m.elapsed_ticks)*m.delta_per_tick;
        //m.bone_transform->pos = current_pos;
        m.bone_transform->pos += current_pos;
        float slerp = 
          ( (float) (delay.final - m.elapsed_ticks)/ (float)delay.final);
        csQuaternion zero_quat;

        //m.bone_transform->quat = zero_quat.Slerp (m.quat, slerp);
        m.curr_quat = zero_quat.SLerp (m.quat, slerp);
        m.bone_transform->quat = m.curr_quat*m.bone_transform->quat;

        //printf("m.quat %.3f %.3f %.3f %.3f\n", m.quat.x, m.quat.y, m.quat.z, m.quat.r);
        //printf("zero_quat %.3f %.3f %.3f %.3f\n", zero_quat.x, zero_quat.y, zero_quat.z, zero_quat.r);
        //printf("m.curr_quat %.3f %.3f %.3f %.3f\n", m.curr_quat.x, m.curr_quat.y, m.curr_quat.z, m.curr_quat.r);
        //printf("m.bone_transform->quat %.3f %.3f %.3f %.3f\n", m.bone_transform->quat.x, m.bone_transform->quat.y, m.bone_transform->quat.z, m.bone_transform->quat.r);
        //printf("-------------------------\n");
        runnable_transforms.DeleteIndexFast (i);
      }
    }
    mod = true;
  }
  return mod;
}

bone_transform_data *csSkeletonRunnable::GetBoneTransform(csSkeletonBoneFactory *bone_fact)
{
  bone_transform_data *b_tr = transforms.Get (bone_fact, 0);
  if (!b_tr) 
  {
    size_t index = ((csSkeletonFactory *)skeleton->GetFactory())->FindBoneIndex(bone_fact);
    b_tr = new bone_transform_data ();
    b_tr->quat = ((csSkeletonBone *)skeleton->GetBone(index))->GetQuaternion ();
    b_tr->pos = ((csSkeletonBone *)skeleton->GetBone(index))->GetTransform ().GetOrigin ();
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
  for (size_t i = 0; i < fact_bones.Length(); i++ )
  {
    csRef<csSkeletonBone> bone;
    bone.AttachNew(new csSkeletonBone (this, fact_bones[i]));
    bone->SetName(fact_bones[i]->GetName());
    bones.Push(bone);
  }

  for (size_t i = 0; i < bones.Length(); i++ )
  {
    iSkeletonBoneFactory *fact_parent_bone = bones[i]->GetFactory()->GetParent();
    if (fact_parent_bone)
    {
      size_t index = fact->FindBoneIndex((csSkeletonBoneFactory *)fact_parent_bone);
      if (index != csArrayItemNotFound)
      {
        bones[i]->SetParent(bones[index]);
      }
    }
  }


  for (size_t i = 0; i < fact_sockets.Length(); i++ )
  {
    csRef<csSkeletonSocket> socket;
    socket.AttachNew(new csSkeletonSocket (this, fact_sockets[i]));

    size_t index = fact->FindBoneIndex((csSkeletonBoneFactory *)fact_sockets[i]->GetBone());
    if (index != csArrayItemNotFound)
    {
      socket->SetBone(bones[index]);
    }

    socket->SetName(fact_sockets[i]->GetName());
    sockets.Push(socket);
  }


  csArray<size_t> fact_parent_bones = fact->GetParentBones();
  for (size_t i=0; i < fact_parent_bones.Length() ; i++ )
  {
    parent_bones.Push(fact_parent_bones[i]);
  }

  last_update_time = 0;
  last_version_id = (uint32)~0;
  elapsed = 0;
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

void csSkeleton::UpdateBones ()
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
  {
    bones[i]->UpdateTransform ();
  }

  for (i = 0 ; i < parent_bones.Length () ; i++)
  {
    csRef<csSkeletonBone> parent_bone = bones[parent_bones[i]];
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

  for (i = 0 ; i < bones.Length () ; i++)
  {
    bones[i]->FireCallback ();
  }

  bones_updated = true;
}

void csSkeleton::UpdateSockets ()
{
  for (size_t i = 0; i < sockets.Length(); i++)
  {
    sockets[i]->GetFullTransform () = sockets[i]->GetTransform()*sockets[i]->GetBone()->GetFullTransform ();
    //csVector3 pos_sock_local = sockets[i]->GetFullTransform ().GetOrigin();
    //printf("%.3f %.3f %.3f\n", pos_sock_local.x, pos_sock_local.y, pos_sock_local.z);

    //printf("-------------------------------------\n");
    if (sockets[i]->GetSceneNode())
    {
      sockets[i]->GetSceneNode()->GetMovable()->SetTransform(sockets[i]->GetFullTransform ());
      //sockets[i]->GetSceneNode()->GetMovable()->UpdateMove ();
    }
  }
}

bool csSkeleton::UpdateAnimation (csTicks current)
{
  if (!last_update_time) 
  {
    last_update_time = current;
    return false;
  }


  elapsed = current - last_update_time;
  last_update_time = current;

  if (elapsed)
  {
    size_t i;
    for (i = 0; i < update_callbacks.Length(); i++)
    {
      update_callbacks[i]->Execute(this, current);
    }

    last_update_time = current;
    i = running_scripts.Length ();
    while (i > 0)
    {
      i--;
      bool stop = false;
      csTicks left;
      if (running_scripts[i].Do (elapsed, stop, left))
      {
        while (left)
        {
          running_scripts[i].Do (left, stop, left);
        }
      }

      if (stop)
      {
        if (script_callback)
        {
          script_callback->OnFinish(running_scripts[i].GetScript());
        }

        running_scripts.DeleteIndexFast (i);
      }
    }

    if (!running_scripts.Length() && pending_scripts.Length())
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
  for (i = 0 ; i < bones.Length () ; i++)
    if (strcmp (bones[i]->GetName (), name) == 0)
      return (iSkeletonBone *)bones[i];
  return 0;
}

iSkeletonScript* csSkeleton::Execute (const char *scriptname)
{
  csSkeletonScript* script = (csSkeletonScript*)(factory->FindScript (scriptname));
  if (!script) 
  {
    //printf("script %s doesn't exist\n", scriptname);
    return 0;
  }

  csSkeletonRunnable runnable = csSkeletonRunnable (script, this);
  running_scripts.Push (runnable);
  return script;
}

iSkeletonScript* csSkeleton::Append (const char *scriptname)
{
  csSkeletonScript* script = (csSkeletonScript*)(
    factory->FindScript (scriptname));
  if (!script) 
  {
    return 0;
  }
  csString script_name = scriptname;
  pending_scripts.Push (script_name);
  return script;
}


iSkeletonScript* csSkeleton::FindScript (const char *scriptname)
{
  size_t i;
  for (i = 0 ; i < running_scripts.Length () ; i++)
    if (strcmp (running_scripts[i].GetName (), scriptname) == 0)
      return running_scripts[i].GetScript();
  return 0;
}

iSkeletonSocket* csSkeleton::FindSocket (const char *socketname)
{
  size_t i;
  for (i = 0 ; i < sockets.Length () ; i++)
    if (strcmp (sockets[i]->GetName (), socketname) == 0)
      return (iSkeletonSocket*)sockets[i];
  return 0;
}

void csSkeleton::StopAll ()
{
  running_scripts.DeleteAll ();
}

void csSkeleton::Stop (const char* scriptname)
{
  size_t i;
  for (i = 0 ; i < running_scripts.Length () ; i++)
    if (strcmp (running_scripts[i].GetName (), scriptname) == 0)
      running_scripts.DeleteIndexFast (i);
}

void csSkeleton::Stop (iSkeletonScript *script)
{
  //csSkeletonRunnable *cs_skel_runnable = static_cast<csSkeletonRunnable *> (script);
  //running_scripts.DeleteFast ( cs_skel_runnable );
}

iSkeletonScript* csSkeleton::GetScript (size_t i)
{
  if (i < running_scripts.Length ())
  {
    return running_scripts[i].GetScript();
  }
  return 0;
}

size_t csSkeleton::FindBoneIndex (const char* bonename)
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
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
  for (size_t i = 0; i < bones.Length(); i++)
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
    for (size_t i = 0; i < bones.Length(); i++)
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
  scfImplementationType(this, graveyard)
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

iSkeletonScript* csSkeletonFactory::FindScript (const char* scriptname)
{
  size_t i;
  for (i = 0 ; i < scripts.Length () ; i++)
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
  for (i = 0 ; i < bones.Length () ; i++)
    if (strcmp (bones[i]->GetName (), name) == 0)
      return bones[i];
  return 0;
}

size_t csSkeletonFactory::FindBoneIndex (const char* bonename)
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
  {
    if (strcmp (bones[i]->GetName (), bonename) == 0)
      return i;
  }
  return csArrayItemNotFound;
}

size_t csSkeletonFactory::FindBoneIndex (csSkeletonBoneFactory *bone) const
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
    if (bones[i] == bone)
      return i;
  return csArrayItemNotFound;
}

void csSkeletonFactory::UpdateParentBones ()
{
  parent_bones.SetLength (0);
  for (size_t i = 0; i < bones.Length (); i++)
  {
    if (!bones[i]->GetParent ())
    {
      bones[i]->UpdateBones();
      parent_bones.Push (i);
    }
  }
}

iSkeletonScript *csSkeletonFactory::CreateScript(const char *name)
{
  csRef<csSkeletonScript> script;
  script.AttachNew(new csSkeletonScript (name));
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
  for (i = 0 ; i < sockets.Length () ; i++)
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
  return sockets.Length ();
}

//--------------------------------iSkeletonGraveyard-----------------------------------------

csSkeletonGraveyard::csSkeletonGraveyard (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csSkeletonGraveyard::~csSkeletonGraveyard ()
{
   skeletons.DeleteAll();
   csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
   if (q)
      q->RemoveListener (this);
}

bool csSkeletonGraveyard::Initialize (iObjectRegistry* object_reg)
{
  csSkeletonGraveyard::object_reg = object_reg;
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  PreProcess = csevPreProcess (object_reg);
  csRef<iEventQueue> eq (CS_QUERY_REGISTRY (object_reg, iEventQueue));
  if (eq == 0) return false;
  eq->RegisterListener (this, PreProcess);

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

iSkeleton *csSkeletonGraveyard::CreateSkeleton(iSkeletonFactory *fact, const char *name)
{
  csSkeletonFactory *cs_skel_fact = static_cast<csSkeletonFactory*> (fact);
  cs_skel_fact->UpdateParentBones();
  csRef<csSkeleton> skeleton;
  skeleton.AttachNew (new csSkeleton (cs_skel_fact));
  skeleton->SetName(name);
  skeletons.Push(skeleton);
  return skeleton;
}

bool csSkeletonGraveyard::HandleEvent (iEvent& ev)
{
  if (ev.Name == PreProcess)
  {
    for (size_t i = 0; i < skeletons.Length() ; i++)
    {
      skeletons[i]->UpdateAnimation(vc->GetCurrentTicks ());
    }
    return true;
  }
  return false;
}
