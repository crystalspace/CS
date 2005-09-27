/*
    Copyright (C) 2004 by Jorrit Tyberghein
                          Hristo Hristov
                          Boyan Hristov
                          Vladimir Ivanov
                          Simeon Ivanov

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
#include "csgeom/quaterni.h"
#include "csutil/event.h"
#include "csutil/util.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "csgeom/tri.h"

#include "gmeshskelanim.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csGenmeshSkelAnimationControlType)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControlType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshSkelAnimationControlType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGenmeshSkelAnimationControlType)

SCF_IMPLEMENT_IBASE (csGenmeshSkelAnimationControlFactory)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonControlFactory)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGenmeshSkelAnimationControl)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControl)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonControlState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csSkelBone)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonBone)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csSkelAnimControlRunnable)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonScript)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csSkelBoneDefaultUpdateCallback)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonBoneUpdateCallback)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGenmeshSkelAnimationControlType::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

//-------------------------------------------------------------------------

csSkelBone::csSkelBone (csGenmeshSkelAnimationControl *animation_control)
{
  SCF_CONSTRUCT_IBASE (0); 
  parent = 0;
  rigid_body = 0;
  anim_control = animation_control;
  bone_mode = BM_SCRIPT;
  cb = csPtr<iGenMeshSkeletonBoneUpdateCallback> (new csSkelBoneDefaultUpdateCallback ());
}

csSkelBone::~csSkelBone ()
{
  delete[] name;
  rigid_body = 0;
  SCF_DESTRUCT_IBASE ();
}

void csSkelBone::SetMode (csBoneTransformMode mode) 
{
  bone_mode = mode; 
  anim_control->SetForceUpdate (bone_mode != BM_SCRIPT);
  if (bone_mode != BM_PHYSICS)
  {
    rigid_body = 0;
  }
}

iGenMeshSkeletonBone *csSkelBone::FindChild (const char *name)
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

void csSkelBone::UpdateRotation ()
{
  size_t scripts_len = anim_control->GetRunningScripts ().Length ();
  if (!scripts_len)
  {
    return;
  }

  if (scripts_len == 1)
  {
    csQuaternion q;
    csSkelAnimControlRunnable *script = anim_control->GetRunningScripts ().Get (0);
    csSkelAnimControlRunnable::TransformHash& rotations = script->GetRotations ();
    bone_transform_data *b_rot = rotations.Get (this, 0);
    if (b_rot)
    {
      q = b_rot->quat;
      rot_quat = q;
      next_transform.SetO2T (csMatrix3 (rot_quat));
    }
  }
  else
  {
    csQuaternion q;
    float min = 0;
    float max = 0;
    float script_factors_total = 0;
    bool slerp = false;
    bool updated = false;

    for (size_t i = 0; i < scripts_len; i++)
    {
      csSkelAnimControlRunnable *script = anim_control->GetRunningScripts ().Get (i);
      csSkelAnimControlRunnable::TransformHash& rotations = script->GetRotations ();
      bone_transform_data *b_rot = rotations.Get (this, 0);
      if (b_rot && (script->GetFactor () > 0))
      {
        script_factors_total += script->GetFactor ();
        if (slerp)
        {
          float max_over_factor = max/script_factors_total;
          if (script->GetFactor () >= min)
          {
            max = script->GetFactor ();
            q = q.Slerp (b_rot->quat, max_over_factor);
          }
          else
          {
            min = script->GetFactor ();
            q = b_rot->quat.Slerp (q, max_over_factor);
          }
          script_factors_total = min + max_over_factor;
        }
        else
        {
          slerp = true;
          min = max = script->GetFactor ();
          q = b_rot->quat;
        }
        updated = true; 
      }
    }

    if (updated)
    {
      rot_quat = q;
      next_transform.SetO2T (csMatrix3 (rot_quat));
    }
  }
}

void csSkelBone::UpdatePosition ()
{
  csVector3 final_pos = csVector3 (0);
  float script_factors_total = 0;
  bool updated = false;

  for (size_t i = 0; i < anim_control->GetRunningScripts ().Length (); i++)
  {
    csSkelAnimControlRunnable *script = anim_control->GetRunningScripts ().Get (i);
    csSkelAnimControlRunnable::TransformHash& positions = script->GetPositions ();
    bone_transform_data *b_pos = positions.Get (this, 0);
    if (b_pos)
    {
    updated = true;
    final_pos += b_pos->axis*script->GetFactor ();
    script_factors_total += script->GetFactor ();
    }
  }

  if (updated)
  {
    if (script_factors_total)
    {
      final_pos /= script_factors_total;
    }
    next_transform.SetOrigin (final_pos);
  }
}

void csSkelBone::CopyFrom (csSkelBone *other)
{
  name = csStrNew (other->GetName ());
  parent = 0;
  for (size_t i = 0; i < other->GetVertexData ().Length (); i++)
  {
    sac_vertex_data v_data (other->GetVertexData ().Get (i));
    vertices.Push (v_data);
  }

  SetTransform (other->GetTransform ());
  next_transform = transform;
}

void csSkelBone::GetSkinBox (csBox3 &box, csVector3 &center)
{
  box.Set (0, 0, 0, 0, 0, 0);
  center.Set (0, 0, 0);
  if (vertices.Length ())
  {
    csBox3 tmp_box;
    box.StartBoundingBox (vertices[0].pos);
    tmp_box.StartBoundingBox (full_transform.This2Other (vertices[0].pos));
    for (size_t i = 1; i < vertices.Length (); i++)
    {
      box.AddBoundingVertexSmart (vertices[i].pos);
      tmp_box.AddBoundingVertexSmart (full_transform.This2Other (vertices[i].pos));
    }
  center = tmp_box.GetCenter ();
  }
}

void csSkelBone::AddVertex (int idx, float weight, float col_weight)
{
  sac_vertex_data vt;
  vt.idx = idx;
  vt.weight = weight;
  vt.col_weight = col_weight;
  vertices.Push (vt);
}

void csSkelBone::UpdateBones ()
{
  if (!parent) full_transform = transform;

  csRefArray<csSkelBone>::Iterator it = bones.GetIterator ();
  while (it.HasNext ())
  {
    csSkelBone *bone = it.Next ();
    bone->GetFullTransform () = bone->GetTransform ()*full_transform;
    bone->UpdateBones ();
  }
}

void csSkelBone::UpdateBones (csSkelBone* parent_bone)
{
  if (!parent) 
  {
    if (bone_mode == BM_PHYSICS && rigid_body)
    {
      full_transform = offset_body_transform;
    }
    else
    {
      full_transform = transform;
    }
  }
  csRefArray<csSkelBone>::Iterator it = bones.GetIterator ();
  while (it.HasNext ())
  {
    csSkelBone *bone = it.Next ();
    switch (bone->GetMode ())
    {
      case BM_NONE:
      case BM_SCRIPT:
          bone->GetFullTransform () = bone->GetTransform ()*full_transform;
      break;
      case BM_PHYSICS:
        if (bone->GetRigidBody ())
        {
          bone->GetFullTransform () = 
            (bone->GetOffsetTransform ()*bone->GetRigidBody ()->GetTransform ())/
            (parent_bone->GetRigidBody ()->GetTransform ());
        }
        else
        {
          bone->GetFullTransform () = bone->GetTransform ()*full_transform;
        }
      break;
    }
    bone->UpdateBones (parent_bone);
  }
}

//-------------------------------------------------------------------------

csSkelAnimControlScript::csSkelAnimControlScript (const char* name)
{
  csSkelAnimControlScript::name = csStrNew (name);
  time = 0;
  loop = false;
  loop_times = -1;
  forced_duration = 0;
}

sac_instruction& csSkelAnimControlScript::AddInstruction (sac_frame & frame, sac_opcode opcode)
{
  sac_instruction instr;
  size_t idx = frame.instructions.Push (instr);
  frame.instructions[idx].opcode = opcode;
  return frame.instructions[idx];
}

sac_frame& csSkelAnimControlScript::AddFrame (csTicks duration)
{
  sac_frame frame;
  size_t idx = frames.Push (frame);
  frames[idx].duration = duration;
  return frames[idx];
}


//-------------------------------------------------------------------------

csSkelAnimControlRunnable::csSkelAnimControlRunnable (csSkelAnimControlScript* script,
  csGenmeshSkelAnimationControl* anim_control)
{
  SCF_CONSTRUCT_IBASE (0);
  csSkelAnimControlRunnable::script = script;
  csSkelAnimControlRunnable::anim_control = anim_control;
  current_instruction = 0;
  current_frame = -1;
  delay.current = 0;
  delay.final = 0;
  delay.diff = 0;
  morph_factor = 1;
  time_factor = 1;
  current_ticks = 0;
  parse_key_frame = true;

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
}

csSkelAnimControlRunnable::~csSkelAnimControlRunnable ()
{
  printf ("csSkelAnimControlRunnable::~csSkelAnimControlRunnable ()\n");
  release_tranform_data (positions);
  release_tranform_data (rotations);
  SCF_DESTRUCT_IBASE ();
}

void csSkelAnimControlRunnable::release_tranform_data (TransformHash& h)
{
  //TransformHash::GlobalIterator it (h.GetIterator ());
  //while (it.HasNext ())
  //  delete it.Next ();
  h.Empty();
}

bone_transform_data *csSkelAnimControlRunnable::GetBoneRotation (csSkelBone *bone)
{
  bone_transform_data *b_rot = rotations.Get (bone, 0);
  if (!b_rot) 
  {
    b_rot = new bone_transform_data ();
    b_rot->quat = bone->GetQuaternion ();
    rotations.Put (bone, b_rot);
  }
  return b_rot;
}

bone_transform_data *csSkelAnimControlRunnable::GetBonePosition (csSkelBone *bone)
{
  bone_transform_data *b_pos = positions.Get (bone, 0);
  if (!b_pos) 
  {
    b_pos = new bone_transform_data ();
  b_pos->axis = bone->GetTransform ().GetOrigin ();
    positions.Put (bone, b_pos);
  }
  return b_pos;
}

bool csSkelAnimControlRunnable::Do (csTicks elapsed, bool& stop, csTicks & left)
{
  stop = false;
  bool mod = false;
  size_t i;
  
  delay.diff += elapsed;
  if (parse_key_frame)
  {
    sac_frame & frame = NextFrame ();
    delay.final = (csTicks) ( (float)frame.duration*time_factor);
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
  
  i = absolute_moves.Length ();
  while (i > 0)
  {
    i--;
    sac_move_execution& m = absolute_moves[i];
    if (delay.current < delay.final)
    {
      csVector3 current_pos = 
        m.final_position - ( (float) (delay.final - delay.current))*m.delta_per_tick;
      m.bone_position->axis = current_pos;
    }
    else
    {
      m.bone_position->axis = m.final_position;
      absolute_moves.DeleteIndexFast (i);
    }
    mod = true;
  }

  i = relative_moves.Length ();
  while (i > 0)
  {
    i--;
    sac_move_execution& m = relative_moves[i];
    if (delay.current <  delay.final)
    {
      csVector3 current_pos = 
        (float) (delay.current - m.elapsed_ticks)*m.delta_per_tick;
      m.bone_position->axis += current_pos;
      m.elapsed_ticks = delay.current;
    }
    else
    {
      csVector3 current_pos = 
        (float) (delay.final - m.elapsed_ticks)*m.delta_per_tick;
      m.bone_position->axis += current_pos;
      relative_moves.DeleteIndexFast (i);
    }
    mod = true;
  }

  i = absolute_rotates.Length ();
  while (i > 0)
  {
    i--;
    sac_rotate_execution& m = absolute_rotates[i];
    if (delay.current < delay.final)
    {
      float slerp = 
        (float)delay.current/ (float)delay.final;
      m.bone_rotation->quat = m.curr_quat.Slerp (m.quat, slerp);
    }
    else
    {
      m.bone_rotation->quat = m.quat;
      absolute_rotates.DeleteIndexFast (i);
    }
    mod = true;
  }

  i = relative_rotates.Length ();
  while (i > 0)
  {
    i--;
    sac_rotate_execution& m = relative_rotates[i];
    if (delay.current < delay.final)
    {
      float slerp = 
        ( (float) (delay.current - m.elapsed_ticks)/ (float)delay.final);
      m.quat = zero_quat.Slerp (m.curr_quat, slerp);
      m.bone_rotation->quat = m.quat*m.bone_rotation->quat;
      m.elapsed_ticks = delay.current;
    }
    else
    {
      float slerp = 
        ( (float) (delay.final - m.elapsed_ticks)/ (float)delay.final);
      m.quat = zero_quat.Slerp (m.curr_quat, slerp);
      m.bone_rotation->quat = m.quat*m.bone_rotation->quat;
      relative_rotates.DeleteIndexFast (i);
    }
    mod = true;
  }

  return mod;
}

void csSkelAnimControlRunnable::ParseFrame (const sac_frame & frame)
{
  csRefArray<csSkelBone> & bones = anim_control->GetBones ();
  size_t num_instructions = frame.instructions.Length ();
  
  size_t i;
  for (i = 0; i < num_instructions ; i++)
  {
    const sac_instruction& inst = frame.instructions[i];
    switch (inst.opcode)
    {
      case AC_ROT:
        if (bones[inst.bone_id]->GetMode () != BM_NONE)
        {
          if (!frame.duration)
          {
            bone_transform_data *bone_rot = GetBoneRotation (bones[inst.bone_id]);
            switch (inst.tr_mode)
            {
              case AC_TRANSFORM_ABSOLUTE:
                bone_rot->quat = csQuaternion (inst.rotate.quat_r, 
                  inst.rotate.quat_x, inst.rotate.quat_y, inst.rotate.quat_z);
              break;
              case AC_TRANSFORM_RELATIVE:
                csQuaternion q = csQuaternion (inst.rotate.quat_r,
                  inst.rotate.quat_x, inst.rotate.quat_y, inst.rotate.quat_z);
                bone_rot->quat = q*bone_rot->quat;
              break;
            }
          }
          else
          {
            sac_rotate_execution m;
            m.bone = bones[inst.bone_id];
            m.bone_rotation = GetBoneRotation (bones[inst.bone_id]);
            m.elapsed_ticks = 0;

            switch (inst.tr_mode)
            {
              case AC_TRANSFORM_ABSOLUTE:
                m.curr_quat = m.bone_rotation->quat;
                m.quat = csQuaternion (inst.rotate.quat_r, 
                  inst.rotate.quat_x, inst.rotate.quat_y, inst.rotate.quat_z);
                absolute_rotates.Push (m);
              break;
              case AC_TRANSFORM_RELATIVE:
              {
                m.quat = m.bone_rotation->quat;
                m.curr_quat = csQuaternion (inst.rotate.quat_r, 
                  inst.rotate.quat_x, inst.rotate.quat_y, inst.rotate.quat_z);
                relative_rotates.Push (m);
              }
              break;
            }
          }
        }
      break;
      case AC_MOVE:
      {
        if (bones[inst.bone_id]->GetMode () != BM_NONE)
        {
          if (!frame.duration)
          {
            bone_transform_data *bone_pos = GetBonePosition (bones[inst.bone_id]);
            switch (inst.tr_mode)
            {
              case AC_TRANSFORM_ABSOLUTE:
                bone_pos->axis = 
                  csVector3 (inst.movement.posx, inst.movement.posy, inst.movement.posz);
              break;
              case AC_TRANSFORM_RELATIVE:
                bone_pos->axis += 
                  csVector3 (inst.movement.posx, inst.movement.posy, inst.movement.posz);
              break;
            }
          }
          else
          {
            sac_move_execution m;
            m.bone = bones[inst.bone_id];
            m.final_position =
              csVector3 (inst.movement.posx, inst.movement.posy, inst.movement.posz);
            m.bone_position = GetBonePosition (m.bone);
            m.elapsed_ticks = 0;

            switch (inst.tr_mode)
            {
              case AC_TRANSFORM_ABSOLUTE:
              {
                csVector3 delta = m.final_position - m.bone_position->axis;
                m.delta_per_tick = delta/ (float) (delay.final);
                absolute_moves.Push (m);
              }
              break;
              case AC_TRANSFORM_RELATIVE:
                m.delta_per_tick = m.final_position/ (float) (delay.final);
                relative_moves.Push (m);
              break;
            }
          }
        }
      }
      break;
    }
  }
}

sac_frame & csSkelAnimControlRunnable::NextFrame ()
{
  size_t frames_count = runnable_frames.Length ();
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

  while (!runnable_frames[current_frame].active)
  {
    current_frame++;
    if ( (size_t)current_frame >= frames_count) 
    {
      if (loop_times > 0)
      {
        loop_times -= 1;
      }
      current_frame = 0;
    }
  }
  
  if (runnable_frames[current_frame].repeat_times > 0) 
  {
    runnable_frames[current_frame].repeat_times--;
    if (!runnable_frames[current_frame].repeat_times)
    {
      runnable_frames[current_frame].active = false;
    }
  }
  
  return script->GetFrames ().Get (current_frame);
}


//-------------------------------------------------------------------------

csGenmeshSkelAnimationControl::csGenmeshSkelAnimationControl (
  csGenmeshSkelAnimationControlFactory* fact, iMeshObject *mesh, iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (0);
  csGenmeshSkelAnimationControl::object_reg = object_reg;
  genmesh_fact_state = SCF_QUERY_INTERFACE (mesh->GetFactory (), iGeneralFactoryState);
  CS_ASSERT (genmesh_fact_state != 0);
  factory = fact;
  num_animated_verts = 0;
  animated_verts = 0;
  transformed_verts = 0;
  animated_colors = 0;
  animated_face_norms = 0;
  num_animated_face_norms = 0;
  animated_vert_norms = 0;
  num_animated_vert_norms = 0;


  last_update_time = 0;
  last_version_id = (uint32)~0;
  elapsed = 0;

  animates_vertices = fact->AnimatesVertices ();
  animates_texels = fact->AnimatesTexels ();
  animates_colors = fact->AnimatesColors ();
  animates_normals = fact->AnimatesNormals ();

  dirty_vertices = true;
  dirty_texels = true;
  dirty_colors = true;
  dirty_normals = true;
  vertices_mapped = false;

  bones_updated = false;
  vertices_updated = false;

  force_bone_update = true;
}

csGenmeshSkelAnimationControl::~csGenmeshSkelAnimationControl ()
{
  printf ("csGenmeshSkelAnimationControl::~csGenmeshSkelAnimationControl ()\n");
  factory->UnregisterAUAnimation (this);
  delete[] animated_verts;
  delete[] animated_colors;
  delete[] animated_vert_norms;
  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshSkelAnimationControl::UpdateAnimation (csTicks current)
{
  if (!vertices_mapped) 
  {
    return false;
  }

  bool mod = false;

  if (!last_update_time) 
  {
    last_update_time = current;
  }
  elapsed = current - last_update_time;
  last_update_time = current;

  if (elapsed)
  {
    last_update_time = current;
    size_t i = running_scripts.Length ();
    while (i > 0)
    {
      i--;
      bool stop = false;
      csTicks left;
      if (running_scripts[i]->Do (elapsed, stop, left))
      {
        mod = true;
        while (left)
        {
          running_scripts[i]->Do (left, stop, left);
        }
      }

      if (stop)
      {
        running_scripts.DeleteIndexFast (i);
      }
    }
  }

  if (mod || force_bone_update)
  {
    dirty_vertices = true;
    dirty_texels = true;
    dirty_colors = true;
    dirty_normals = true;
    
    bones_updated = false;
    vertices_updated = false;
    force_bone_update = false;

    if (factory->GetFlags ().Check (SKEL_ANIMATION_ALWAYS_UPDATE))
    {
      if (factory->GetFlags ().Check (
        SKEL_ANIMATION_ALWAYS_UPDATE_BONES|SKEL_ANIMATION_ALWAYS_UPDATE_VERTICES))
      {
        UpdateBones ();
      }

      if (factory->GetFlags ().Check (SKEL_ANIMATION_ALWAYS_UPDATE_VERTICES))
      {
        UpdateAnimatedVertices (current, animated_verts, num_animated_verts);
      }
    }

    return true;
  }

  return false;
}

void csGenmeshSkelAnimationControl::UpdateArrays (int num_verts)
{
  if (num_verts != num_animated_verts)
  {
    num_animated_verts = num_verts;

    delete[] animated_verts;
    animated_verts = new csVector3[num_verts];

    delete[] transformed_verts;
    transformed_verts = new csVector3[num_verts];

    delete[] animated_colors;
    animated_colors = new csColor4[num_verts];

    last_version_id = (uint32)~0;
  }
}

void csGenmeshSkelAnimationControl::UpdateVertNormArrays (int num_norms)
{
  if (num_norms != num_animated_vert_norms)
  {
    num_animated_vert_norms = num_norms;

    delete[] animated_vert_norms;
    animated_vert_norms = new csVector3[num_norms];

    num_animated_face_norms = genmesh_fact_state->GetTriangleCount ();
    delete[] animated_face_norms;
    animated_face_norms = new csVector3[num_animated_face_norms];

    vts_con_tris.SetLength (genmesh_fact_state->GetVertexCount ());
    csTriangle *triangles = genmesh_fact_state->GetTriangles ();
    for (int i = 0; i < genmesh_fact_state->GetTriangleCount () ; i++)
    {
      face_data fda;
      fda.face_idx = i;
      fda.time = 0;
      vts_con_tris[triangles[i].a].tri_indices.Push (fda);
      vts_con_tris[triangles[i].a].time = 0;

      face_data fdb;
      fdb.face_idx = i;
      fdb.time = 0;
      vts_con_tris[triangles[i].b].tri_indices.Push (fdb);
      vts_con_tris[triangles[i].b].time = 0;

      face_data fdc;
      fdc.face_idx = i;
      fdc.time = 0;
      vts_con_tris[triangles[i].c].tri_indices.Push (fdc);
      vts_con_tris[triangles[i].c].time = 0;
    }
  }
}

csArray<csReversibleTransform> csGenmeshSkelAnimationControl::bone_transforms;

void csGenmeshSkelAnimationControl::TransformVerticesToBones (const csVector3* verts, int num_verts)
{
  csRefArray<csSkelBone>& f_bones = factory->GetBones ();
  csArray<size_t>& f_parent_bones = factory->GetParentBones ();
  size_t i;

  bones.SetLength (0);
  for (i = 0 ; i < f_bones.Length () ; i++)
  {
    csRef<csSkelBone> gr = csPtr<csSkelBone> (new csSkelBone (this));
    gr->CopyFrom (f_bones[i]);
    bones.Push (gr);
  }

  for (i = 0 ; i < f_bones.Length () ; i++)
  {
    // Really stuppid
    if (f_bones[i]->GetParent ())
    {
      csSkelBone *p = CS_STATIC_CAST (csSkelBone*,f_bones[i]->GetParent ());
      size_t parent_index = f_bones.Find (p);
      csRef<csSkelBone> child = bones[i];
      bones[parent_index]->AddBone (child);
      child->SetParent (bones[parent_index]);
    }
  }


  if (f_parent_bones.Length () > parent_bones.Length ())
  {
    parent_bones.SetLength (f_parent_bones.Length ());
  }

  for (i = 0 ; i < f_parent_bones.Length () ; i++)
  {
    parent_bones[i] = f_parent_bones[i];
  }

  for (i = 0 ; i < parent_bones.Length () ; i++)
  {
    bones[parent_bones[i]]->UpdateBones ();
  }

  csArray<csArray<sac_bone_data> >& bones_vertices = factory->GetBonesVerticesMapping ();
  for (i = 0 ; i < (size_t)num_verts ; i++)
  {
    if (i >= bones_vertices.Length ())
      animated_verts[i] = verts[i];
    else
    {
      csArray<sac_bone_data>& vtgr = bones_vertices[i];
      if (vtgr.Length () == 0)
      {
        animated_verts[i] = verts[i];
      }
      else 
      {
        for (size_t j = 0 ; j < vtgr.Length () ; j++)
        {
          sac_bone_data & g_data = vtgr[j];
          csRef<csSkelBone> bone = bones[vtgr[j].idx];
          sac_vertex_data & v_data = bone->GetVertexData ()[g_data.v_idx];
          csReversibleTransform& transform = bone->GetFullTransform ();
          v_data.pos = transform.Other2This (verts[i]);
        }
      }
    }
  }
}

void csGenmeshSkelAnimationControl::UpdateBones ()
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
  {
    bones[i]->UpdateRotation ();
    bones[i]->UpdatePosition ();
    if (running_scripts.Length ())
      bones[i]->FireCallback ();
  }

  for (i = 0 ; i < parent_bones.Length () ; i++)
  {
    csRef<csSkelBone> parent_bone = bones[parent_bones[i]];
    switch (parent_bone->GetMode ())
    {
      case BM_NONE:
      case BM_SCRIPT:
        parent_bone->UpdateBones ();
      break;
      case BM_PHYSICS:
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
    }
  }
  bones_updated = true;
}

void csGenmeshSkelAnimationControl::UpdateAnimatedVertices (csTicks current, 
  const csVector3* verts, int num_verts)
{
  if (dirty_vertices)
  {
    size_t i;
    csArray<csArray<sac_bone_data> >& bones_vertices = factory->GetBonesVerticesMapping ();
    for (i = 0 ; i < (size_t)num_verts ; i++)
    {
      if (i >= bones_vertices.Length ())
        animated_verts[i] = verts[i];
      else
      {
        csArray<sac_bone_data>& vtgr = bones_vertices[i];
        if (vtgr.Length () == 0)
        {
          animated_verts[i] = verts[i];
        }
        else
        {
          csVector3 orig = csVector3 (0);
          float total_weight = 0;
          for (size_t j = 0 ; j < vtgr.Length () ; j++)
          {
            sac_bone_data & g_data = vtgr[j];
            csRef<csSkelBone> bone = bones[g_data.idx];
            sac_vertex_data & v_data = bone->GetVertexData ()[g_data.v_idx];
            csReversibleTransform& transform = bone->GetFullTransform ();
            total_weight += v_data.weight;
            orig += v_data.weight * transform.This2Other (v_data.pos);
          }
          animated_verts[i] = orig / total_weight;
        }
      }
    }

  switch (calc_norms_method)
  {
  case CALC_NORMS_NONE:
    // do nothing
    break;
  case CALC_NORMS_FAST:
    {
      csTriangle *triangles = genmesh_fact_state->GetTriangles ();
      for (int i = 0; i < genmesh_fact_state->GetTriangleCount () ; i++)
      {
        csVector3 face_normal = (animated_verts[triangles[i].b] - animated_verts[triangles[i].a]) %
          (animated_verts[triangles[i].c] - animated_verts[triangles[i].a]);
        face_normal.Normalize ();
        animated_vert_norms[triangles[i].a] = face_normal;
        animated_vert_norms[triangles[i].b] = face_normal;
        animated_vert_norms[triangles[i].c] = face_normal;
      }
    }
    break;
  case CALC_NORMS_ACCURATE:
    {
      csTriangle *triangles = genmesh_fact_state->GetTriangles ();
      for (size_t i = 0; i < vts_con_tris.Length () ; i++)
      {
        if (vts_con_tris[i].time != current)
        {
          vts_con_tris[i].time = current;
          animated_vert_norms[i].Set (0, 0, 0);
          for (size_t j = 0; j < vts_con_tris[i].tri_indices.Length () ; j++)
          {
            size_t f_idx = vts_con_tris[i].tri_indices[j].face_idx;
            if (vts_con_tris[i].tri_indices[j].time != current)
            {
              vts_con_tris[i].tri_indices[j].time = current;
              csVector3 face_normal = (animated_verts[triangles[f_idx].b] - animated_verts[triangles[f_idx].a]) %
                (animated_verts[triangles[f_idx].c] - animated_verts[triangles[f_idx].a]);
              face_normal.Normalize ();
              animated_face_norms[f_idx] = face_normal;
            }
            animated_vert_norms[i] += animated_face_norms[f_idx];
          }
          float norm = animated_vert_norms[i].Norm ();
          if (norm)
          {
            animated_vert_norms[i] /= norm;
          }
        }
      }
    }
    break;
  }
  }
  vertices_updated = true;
}

const csVector3* csGenmeshSkelAnimationControl::UpdateVertices (csTicks current,
  const csVector3* verts, int num_verts, uint32 version_id)
{
  if (!animates_vertices) return verts;

  if (!vertices_mapped)
  {
    TransformVerticesToBones (verts, num_verts);
    vertices_mapped = true;
  }

  UpdateArrays (num_verts);

  if (!factory->GetFlags ().Check (SKEL_ANIMATION_ALWAYS_UPDATE))
  {
    if (UpdateAnimation (current))
    {
      if (!bones_updated) 
      {
        UpdateBones ();
      }
      
      if (!vertices_updated) 
      {
        UpdateAnimatedVertices (current, verts, num_verts);
      }
    }
  }
  
  return animated_verts;
}

const csVector2* csGenmeshSkelAnimationControl::UpdateTexels (csTicks current,
  const csVector2* texels, int num_texels, uint32 version_id)
{
  if (!animates_texels) return texels;
  return texels;
}

const csVector3* csGenmeshSkelAnimationControl::UpdateNormals (csTicks current,
  const csVector3* normals, int num_normals, uint32 version_id)
{
  if (!animates_normals) return normals;
  UpdateVertNormArrays (num_normals);

  return animated_vert_norms;
}

const csColor4* csGenmeshSkelAnimationControl::UpdateColors (csTicks current,
  const csColor4* colors, int num_colors, uint32 version_id)
{
  return colors;
}

iGenMeshSkeletonScript* csGenmeshSkelAnimationControl::Execute (const char* scriptname)
{
  csSkelAnimControlScript* script = factory->FindScript (scriptname);
  if (!script) return 0;
  csRef<csSkelAnimControlRunnable> runnable = csPtr<csSkelAnimControlRunnable> (new csSkelAnimControlRunnable (script, this));
  running_scripts.Push (runnable);
  return runnable;
}

void csGenmeshSkelAnimationControl::StopAll ()
{
  running_scripts.DeleteAll ();
}

void csGenmeshSkelAnimationControl::Stop (const char* scriptname)
{
  size_t i = running_scripts.Length ();
  while (i > 0)
  {
    i--;
    if (strcmp (running_scripts[i]->GetName (), scriptname) == 0)
    {
      running_scripts.DeleteIndexFast (i);
      break;
    }
  }
}

void csGenmeshSkelAnimationControl::Stop (iGenMeshSkeletonScript *script)
{
  running_scripts.DeleteFast ( (csSkelAnimControlRunnable *)script);
}

size_t csGenmeshSkelAnimationControl::GetScriptsCount ()
{
  return running_scripts.Length ();
}

iGenMeshSkeletonScript* csGenmeshSkelAnimationControl::GetScript (size_t i)
{
  if (i < running_scripts.Length ())
    return running_scripts[i];
  return 0;
}

iGenMeshSkeletonScript* csGenmeshSkelAnimationControl::FindScript (const char *scriptname)
{
  size_t i;
  for (i = 0 ; i < running_scripts.Length () ; i++)
    if (strcmp (running_scripts[i]->GetName (), scriptname) == 0)
      return running_scripts[i];
  return 0;
}

iGenMeshSkeletonBone *csGenmeshSkelAnimationControl::FindBone (const char *name)
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
    if (strcmp (bones[i]->GetName (), name) == 0)
      return bones[i];
  return 0;
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControlFactory::csGenmeshSkelAnimationControlFactory (
  csGenmeshSkelAnimationControlType* type, iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (type);
  csGenmeshSkelAnimationControlFactory::type = type;
  csGenmeshSkelAnimationControlFactory::object_reg = object_reg;
  InitTokenTable (xmltokens);

  animates_vertices = false;
  animates_texels = false;
  animates_colors = false;
  animates_normals = false;
  has_hierarchical_bones = false;
  calc_norms_method = CALC_NORMS_NONE;
  flags.SetAll (0);
}

csGenmeshSkelAnimationControlFactory::~csGenmeshSkelAnimationControlFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

csPtr<iGenMeshAnimationControl> csGenmeshSkelAnimationControlFactory::
  CreateAnimationControl (iMeshObject *mesh)
{
  csGenmeshSkelAnimationControl* ctrl = new csGenmeshSkelAnimationControl (this, mesh, object_reg);
  ctrl->SetCalcNormsMethod (calc_norms_method);
  size_t i;
  for (i = 0 ; i < autorun_scripts.Length () ; i++)
  ctrl->Execute (autorun_scripts[i]);

  if (flags.Check (SKEL_ANIMATION_ALWAYS_UPDATE))
  {
    RegisterAUAnimation (ctrl);
  }
  
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

void csGenmeshSkelAnimationControlFactory::UpdateBonesMapping ()
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
  {
    csSkelBone* g = bones[i];
    const csArray<sac_vertex_data>& vtdata = g->GetVertexData ();
    size_t j;
    for (j = 0 ; j < vtdata.Length () ; j++)
    {
      if (vtdata[j].weight > SMALL_EPSILON)
      {
        csArray<sac_bone_data>& vertices = bones_vertices.GetExtend (vtdata[j].idx);
        sac_bone_data gd;
        gd.idx = (int)i;
        gd.v_idx = (int)j;
        vertices.Push (gd); // Push bone index.
      }
    }
  }
}

csSkelAnimControlScript* csGenmeshSkelAnimationControlFactory::FindScript (
  const char* scriptname) const
{
  size_t i;
  for (i = 0 ; i < scripts.Length () ; i++)
    if (strcmp (scripts[i]->GetName (), scriptname) == 0)
      return scripts[i];
  return 0;
}

const char* csGenmeshSkelAnimationControlFactory::LoadScriptFile (const char *filename)
{
  //TODO
  return 0;
}

void csGenmeshSkelAnimationControlFactory::DeleteScript (const char *script_name)
{
  //TODO
}

void csGenmeshSkelAnimationControlFactory::DeleteAllScripts ()
{
  //TODO
}

csSkelBone* csGenmeshSkelAnimationControlFactory::FindBone (
  const char* bonename) const
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
    if (strcmp (bones[i]->GetName (), bonename) == 0)
      return bones[i];
  return 0;
}

size_t csGenmeshSkelAnimationControlFactory::FindBoneIndex (
  const char* bonename) const
{
  size_t i;
  for (i = 0 ; i < bones.Length () ; i++)
    if (strcmp (bones[i]->GetName (), bonename) == 0)
      return i;
  return (size_t)~0;
}

const char* csGenmeshSkelAnimationControlFactory::ParseBone (iDocumentNode* node,
  csSkelBone* parent)
{
  const char* bonename = node->GetAttributeValue ("name");

  if (!bonename)
    return "Name of the bone is missing!";

  csRef<csSkelBone> bone = csPtr<csSkelBone> (new csSkelBone (0));
  bone->SetName (bonename);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_RANGE:
        {
          int from_idx = child->GetAttributeValueAsInt ("from");
          int to_idx = child->GetAttributeValueAsInt ("to");
          if (to_idx < from_idx)
          {
            return "Bad range in bone definition!";
          }
          float weight = child->GetAttributeValueAsFloat ("weight");
          float col_weight = child->GetAttributeValueAsFloat ("col_weight");
          int i;
          for (i = from_idx ; i <= to_idx ; i++)
          {
            bone->AddVertex (i, weight, col_weight);
          }
        }
        break;
      case XMLTOKEN_VERTEX:
        {
          int idx = child->GetAttributeValueAsInt ("idx");
          float weight = child->GetAttributeValueAsFloat ("weight");
          float col_weight = child->GetAttributeValueAsFloat ("col_weight");
          bone->AddVertex (idx, weight, col_weight);
        }
        break;
      case XMLTOKEN_MOVE:
        {
          csRef<iSyntaxService> SyntaxService = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
            bone->GetTransform ().SetOrigin (v);
          }

          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
            bone->GetTransform ().SetO2T (m);
          }
        }
        break;
      case XMLTOKEN_BONE:
        {
          const char* err = ParseBone (child, bone);
          if (err != 0) return err;
        }
        break;
      default:
        error_buf.Format (
        "Don't recognize token '%s' in anim control bone!",
        value);
        delete bone;
        return error_buf;
    }
  }
  if (parent)
  {
    parent->AddBone (bone);
    bone->SetParent (parent);
    has_hierarchical_bones = true;
  }
  bones.Push (bone);
  return 0;
}

const char* csGenmeshSkelAnimationControlFactory::ParseScript (iDocumentNode* node)
{
  const char* scriptname = node->GetAttributeValue ("name");
  if (!scriptname)
  return "Name of the script is missing!";

  // Small class to make sure script gets deleted in case of error.
  struct AutoDelete
  {
    csSkelAnimControlScript* script;
    ~AutoDelete () { delete script; }
  } ad;
  ad.script = new csSkelAnimControlScript (scriptname);

  csRef< iDocumentAttribute > duration_attr = node->GetAttribute ("duration");
  if (duration_attr)
  {
    ad.script->SetForcedDuration (duration_attr->GetValueAsInt ());
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FRAME:
      {
        csTicks duration = child->GetAttributeValueAsInt ("duration");
        ad.script->GetTime () += duration;
        sac_frame& frame = ad.script->AddFrame (duration);
        csRef< iDocumentAttribute > attr = child->GetAttribute ("repeat");
        if (attr)
        {
          frame.repeat_times = attr->GetValueAsInt ();
        }
        else
        {
          frame.repeat_times = -1;
        }

        frame.active = (frame.repeat_times != 0);

        csRef<iDocumentNodeIterator> it = child->GetNodes ();
        while (it->HasNext ())
        {
          csRef<iDocumentNode> child = it->Next ();
          if (child->GetType () != CS_NODE_ELEMENT) continue;
          const char* value = child->GetValue ();
          csStringID id = xmltokens.Request (value);
          switch (id)
          {
            case XMLTOKEN_MOVE:
            {
              const char* bonename = child->GetAttributeValue ("bone");
              if (!bonename) return "Missing bone name for <move>!";
              size_t bone_id = FindBoneIndex (bonename);
              if (bone_id == (size_t)~0) return "Can't find bone for <move>!";
              sac_instruction& instr = ad.script->AddInstruction (frame, AC_MOVE);
              instr.bone_id = bone_id;

              instr.tr_mode = AC_TRANSFORM_ABSOLUTE;
              const char *transform_mode = child->GetAttributeValue ("type");
              if (transform_mode)
              {
                if (!strcmp (transform_mode, "abs"))
                {
                  instr.tr_mode = AC_TRANSFORM_ABSOLUTE;
                }
                else
                if (!strcmp (transform_mode, "rel"))
                {
                  instr.tr_mode = AC_TRANSFORM_RELATIVE;
                }
              }
                
              instr.movement.posx = child->GetAttributeValueAsFloat ("x");
              instr.movement.posy = child->GetAttributeValueAsFloat ("y");
              instr.movement.posz = child->GetAttributeValueAsFloat ("z");
              animates_vertices = true;
            }
            break;
            case XMLTOKEN_ROT:
            {
              const char* bonename = child->GetAttributeValue ("bone");
              if (!bonename) return "Missing bone name for <rotx>!";
              size_t bone_id = FindBoneIndex (bonename);
              if (bone_id == (size_t)~0) return "Can't find bone for <rot>!";
              sac_instruction& instr = ad.script->AddInstruction (frame, AC_ROT);
              instr.bone_id = bone_id;

              instr.tr_mode = AC_TRANSFORM_ABSOLUTE;
              const char *transform_mode = child->GetAttributeValue ("type");
              if (transform_mode)
              {
                if (!strcmp (transform_mode, "abs"))
                {
                  instr.tr_mode = AC_TRANSFORM_ABSOLUTE;
                }
                else
                if (!strcmp (transform_mode, "rel"))
                {
                  instr.tr_mode = AC_TRANSFORM_RELATIVE;
                }
              }

              
              csVector3 v (0);
              float x_val = child->GetAttributeValueAsFloat ("x");
              if (x_val)
              {
                v.x = (x_val*180)/PI;
              }

              float y_val = child->GetAttributeValueAsFloat ("y");
              if (y_val)
              {
                v.y = (y_val*180)/PI;
              }

              float z_val = child->GetAttributeValueAsFloat ("z");
              if (z_val)
              {
                v.z = (z_val*180)/PI;
              }
              

              csQuaternion q;
              
              q.SetWithEuler (v);

              instr.rotate.quat_x = q.x;

              //Why?
              instr.rotate.quat_y = -q.y;

              instr.rotate.quat_z = q.z;
              instr.rotate.quat_r = q.r;
              
              /// The code below is correct ,but generated quaternion
              /// is incorrect in certain cases and animation bumps ugly sometimes

              /*
              csMatrix3 m = csXRotMatrix3 (child->GetAttributeValueAsFloat ("x"))*
                csYRotMatrix3 (child->GetAttributeValueAsFloat ("y"))*
                csZRotMatrix3 (child->GetAttributeValueAsFloat ("z"));
              q = csQuaternion (m);
              //csPrintf ("%s x %.3f y %.3f z %.3f r %.3f\n\n", bones[bone_id]->GetName (), q.axis.x, q.axis.y, q.axis.z, q.r);
              instr.rotate.quat_x = q.x;
              instr.rotate.quat_y = q.y;
              instr.rotate.quat_z = q.z;
              instr.rotate.quat_r = q.r;
              */
              animates_vertices = true;
            }
            break;
            default:
              error_buf.Format (
              "Don't recognize token '%s' in anim control script!",
              value);
            return error_buf;
          }
        }
      }
      break;
      case XMLTOKEN_LOOP:
      {
        ad.script->SetLoop (true);
        csRef< iDocumentAttribute > attr = child->GetAttribute ("times");
        if (attr)
        {
          ad.script->SetLoopTimes (attr->GetValueAsInt ());
        }
      }
      break;
    }
  }
  scripts.Push (ad.script);
  ad.script = 0;  // Prevent DeleteScript instance from deleting script.
  return 0;
}

const char* csGenmeshSkelAnimationControlFactory::Load (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_BONE:
        {
          const char* err = ParseBone (child, 0);
          if (err) return err;
        }
        break;
      case XMLTOKEN_SCRIPT:
        {
          const char* err = ParseScript (child);
          if (err) return err;
        }
        break;
      case XMLTOKEN_RUN:
        {
          const char* scriptname = child->GetAttributeValue ("script");
          if (!scriptname)
            return "Missing script name attribute for <run>!";
          autorun_scripts.Push (scriptname);
        }
        break;
      case XMLTOKEN_ALWAYS_UPDATE:
        {
      flags.SetBool (SKEL_ANIMATION_ALWAYS_UPDATE, true);
      csRef<iDocumentNodeIterator> it = child->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
          case XMLTOKEN_BONES:
            flags.SetBool (SKEL_ANIMATION_ALWAYS_UPDATE_BONES, true);
          break;
          case XMLTOKEN_VERTICES:
            flags.SetBool (SKEL_ANIMATION_ALWAYS_UPDATE_VERTICES, true);
          break;
        }
      }
    }
    break;
    case XMLTOKEN_CALCNORMS:
    {
      const char *calc_method = child->GetContentsValue ();
      if (calc_method)
      {
        if (!strcmp (calc_method, "fast"))
        {
          calc_norms_method = CALC_NORMS_FAST;
          animates_normals = true;
        }
        else
        if (!strcmp (calc_method, "accurate"))
        {
          calc_norms_method = CALC_NORMS_ACCURATE;
          animates_normals = true;
        }
      }
        }
        break;
      default:
        error_buf.Format (
        "Don't recognize token '%s' in anim control!",
        value);
        return error_buf;
    }
  }

  UpdateBonesMapping ();
  UpdateParentBones ();
  return 0;
}

const char* csGenmeshSkelAnimationControlFactory::Save (iDocumentNode* parent)
{
  csRef<iFactory> plugin = SCF_QUERY_INTERFACE (type, iFactory);
  if (!plugin) return "Couldn't get Class ID";
  parent->SetAttribute ("plugin", plugin->QueryClassID ());
  return "Not implemented yet!";
}

void csGenmeshSkelAnimationControlFactory::UpdateParentBones ()
{
  parent_bones.SetLength (0);
  for (size_t i = 0; i < bones.Length (); i++)
  {
    if (!bones[i]->GetParent ())
      parent_bones.Push (i);
  }
}

void csGenmeshSkelAnimationControlFactory::RegisterAUAnimation (csGenmeshSkelAnimationControl *anim)
{
  type->RegisterAUAnimation (anim);
}

void csGenmeshSkelAnimationControlFactory::UnregisterAUAnimation (csGenmeshSkelAnimationControl *anim)
{
  type->UnregisterAUAnimation (anim);
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControlType::csGenmeshSkelAnimationControlType (
  iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = 0;
}

csGenmeshSkelAnimationControlType::~csGenmeshSkelAnimationControlType ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshSkelAnimationControlType::Initialize (iObjectRegistry* object_reg)
{
  csGenmeshSkelAnimationControlType::object_reg = object_reg;
  scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing);
  return true;
}

csPtr<iGenMeshAnimationControlFactory> csGenmeshSkelAnimationControlType::
  CreateAnimationControlFactory ()
{
  csGenmeshSkelAnimationControlFactory* ctrl = new csGenmeshSkelAnimationControlFactory
     (this, object_reg);
  return csPtr<iGenMeshAnimationControlFactory> (ctrl);
}

bool csGenmeshSkelAnimationControlType::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode (&ev) == cscmdPreProcess)
  {
      UpdateAUAnimations (vc->GetCurrentTicks ());
    return true;
  }
  return false;
}
