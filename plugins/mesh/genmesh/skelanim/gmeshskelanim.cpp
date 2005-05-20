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
#include "csutil/util.h"
#include "csutil/event.h"
#include "csgeom/quaterni.h"
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "gmeshskelanim.h"
#include <imap/services.h>

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
	rot.x = rot.y = rot.z = 0;
	anim_control = animation_control;
	bone_mode = BM_SCRIPT;
	cb = csPtr<iGenMeshSkeletonBoneUpdateCallback>(new csSkelBoneDefaultUpdateCallback());
};

csSkelBone::~csSkelBone ()
{
	delete[] name;
	SCF_DESTRUCT_IBASE ();
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

void csSkelBone::SetAxisAngle (int axis, float angle)
{
	switch (axis)
	{
		case 0: 
			rot.x = angle;
			break;
		case 1:
			rot.y = angle;
			break;
		case 2:
			rot.z = angle;
			break;
	}
	transform.SetO2T (csXRotMatrix3(rot.x)*
					csYRotMatrix3(rot.y)*
					csZRotMatrix3(rot.z));
}

void csSkelBone::UpdateRotation()
{
	size_t scripts_len = anim_control->GetRunningScripts ().Length ();
	if (!scripts_len)
	{
		return;
	}

	bool updated = false;
	if (scripts_len == 1)
	{
		csQuaternion q;
		csSkelAnimControlRunnable *script = anim_control->GetRunningScripts ().Get (0);
		csSkelAnimControlRunnable::TransformHash& rotations = script->GetRotations ();
		bone_transform_data *b_rot = rotations.Get(this, 0);
		if (b_rot)
		{
			q = b_rot->quat;
			updated = true;
		}

		if (updated)
		{
			rot.quat = q;
			next_transform.SetO2T (csMatrix3(q));
		}
	}
	else
	{
		csQuaternion q;
		float min = 0;
		float max = 0;
		float script_factors_total = 0;
		bool slerp = false;
		
		for (size_t i = 0; i < scripts_len; i++)
		{
			csSkelAnimControlRunnable *script = anim_control->GetRunningScripts ().Get (i);
			csSkelAnimControlRunnable::TransformHash& rotations = script->GetRotations ();
			bone_transform_data *b_rot = rotations.Get(this, 0);
			if (b_rot && (script->GetFactor() > 0))
			{
				script_factors_total += script->GetFactor();
				if (slerp)
				{
					float max_over_factor = max/script_factors_total;
					if (script->GetFactor() >= min)
					{
						max = script->GetFactor();
						q = q.Slerp(b_rot->quat, max_over_factor);
					}
					else
					{
						min = script->GetFactor();
						q = b_rot->quat.Slerp(q, max_over_factor);
					}
					script_factors_total = min + max_over_factor;
				}
				else
				{
					slerp = true;
					min = max = script->GetFactor();
					q = b_rot->quat;
				}
				updated = true;
			}
		}

		if (updated)
		{
			rot.quat = q;
			next_transform.SetO2T (csMatrix3(q));
		}
	}
}

void csSkelBone::UpdatePosition()
{
	float final_pos_x = 0;
	float final_pos_y = 0;
	float final_pos_z = 0;
	float script_factors_total = 0;
	bool updated = false;

	for (size_t i = 0; i < anim_control->GetRunningScripts ().Length (); i++)
	{
		csSkelAnimControlRunnable *script = anim_control->GetRunningScripts ().Get(i);
		csSkelAnimControlRunnable::TransformHash& positions = script->GetPositions ();
		bone_transform_data *b_pos = positions.Get(this, 0);
		if (b_pos)
		{
			updated = true;
			final_pos_x += b_pos->x*script->GetFactor();
			final_pos_y += b_pos->y*script->GetFactor();
			final_pos_z += b_pos->z*script->GetFactor();
			script_factors_total += script->GetFactor();
		}
	}

	if (updated)
	{
		if (script_factors_total)
		{
			final_pos_x /= script_factors_total;
			final_pos_y /= script_factors_total;
			final_pos_z /= script_factors_total;
		}
		next_transform.SetOrigin (csVector3(final_pos_x, final_pos_y, final_pos_z));
	}
}

float csSkelBone::GetAxisAngle (int axis)
{
	return rot.data[axis];
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

	transform.SetOrigin(other->GetTransform().GetOrigin());
	transform.SetO2T(other->GetTransform().GetO2T());

	SetAxisAngle (0, other->GetAxisAngle (0));
	SetAxisAngle (1, other->GetAxisAngle (1));
	SetAxisAngle (2, other->GetAxisAngle (2));

	rot.quat.SetWithEuler(
		csVector3(
				  (other->GetAxisAngle (0)*180)/PI,
				  (other->GetAxisAngle (1)*180)/PI,
				  (other->GetAxisAngle (2)*180)/PI)
	);

	//Why?
	rot.quat.y = -rot.quat.y;
	//rot.quat = csQuaternion(transform.GetO2T());
	next_transform = transform;
}

void csSkelBone::GetSkinBox (csBox3 &box, csVector3 &center)
{
	if (vertices.Length () == 0)
	{
		box.Set (0, 0, 0, 0, 0, 0);
		center.Set (0, 0, 0);
	}
	else
	{
		csBox3 tmp_box;
		box.StartBoundingBox (vertices[0].pos);
		tmp_box.StartBoundingBox (full_transform.This2Other (vertices[0].pos));
		for (size_t i = 1; i < vertices.Length (); i++)
		{
			box.AddBoundingVertexSmart (vertices[i].pos);
			tmp_box.StartBoundingBox (full_transform.This2Other (vertices[i].pos));
		}
		center = tmp_box.GetCorner (0) + tmp_box.GetCenter ();
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

void csSkelBone::UpdateBones (iRigidBody *parent_body)
{
	if (!parent) full_transform = transform;

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
						bone->GetFullTransform () = bone->GetRigidBody ()->GetTransform ()/parent_body->GetTransform ();
					else
						bone->GetFullTransform () = bone->GetTransform ()*full_transform;
				break;
		}
		bone->UpdateBones (parent_body);
	}
}

//-------------------------------------------------------------------------

csSkelAnimControlScript::csSkelAnimControlScript (const char* name)
{
	csSkelAnimControlScript::name = csStrNew (name);
	time = 0;
}

sac_instruction& csSkelAnimControlScript::AddInstruction (sac_opcode opcode)
{
	sac_instruction instr;
	size_t idx = instructions.Push (instr);
	instructions[idx].opcode = opcode;
	return instructions[idx];
}

//-------------------------------------------------------------------------

csSkelAnimControlRunnable::csSkelAnimControlRunnable (csSkelAnimControlScript* script,
	csGenmeshSkelAnimationControl* anim_control)
{
	SCF_CONSTRUCT_IBASE (0);
	csSkelAnimControlRunnable::script = script;
	csSkelAnimControlRunnable::anim_control = anim_control;
	current_instruction = 0;
	delay.final = 0;
	morph_factor = 1;
	time_factor = 1;
}

csSkelAnimControlRunnable::~csSkelAnimControlRunnable ()
{
	release_tranform_data(positions);
	release_tranform_data(rotations);
	SCF_DESTRUCT_IBASE ();
}

void csSkelAnimControlRunnable::release_tranform_data(TransformHash& h)
{
	TransformHash::GlobalIterator it(h.GetIterator());
	while (it.HasNext())
	  delete it.Next();
}

bone_transform_data *csSkelAnimControlRunnable::GetBoneRotation(csSkelBone *bone)
{
	bone_transform_data *b_rot = rotations.Get(bone, 0);
	if (!b_rot) 
	{
		b_rot = new bone_transform_data ();
		b_rot->quat = bone->GetQuaternion();
		rotations.Put (bone, b_rot);
	}

	return b_rot;
}

bone_transform_data *csSkelAnimControlRunnable::GetBonePosition(csSkelBone *bone)
{
	bone_transform_data *b_pos = positions.Get(bone, 0);
	if (!b_pos) 
	{
		b_pos = new bone_transform_data ();
		b_pos->x = bone->GetTransform().GetOrigin().x;
		b_pos->y = bone->GetTransform().GetOrigin().y;
		b_pos->z = bone->GetTransform().GetOrigin().z;
		positions.Put (bone, b_pos);
	}	
	return b_pos;
}

bool csSkelAnimControlRunnable::Do (csTicks current, bool& stop)
{
	csRefArray<csSkelBone>& bones = anim_control->GetBones ();
	if (!bones.Length ()) return false;

	stop = false;
	bool mod = false;

	//-------
	// Perform all running moves.
	//-------

	size_t i = moves.Length ();
	while (i > 0)
	{
		i--;
		const sac_move_execution& m = moves[i];
		csVector3 current_pos;
		if (current < m.final)
		{
			current_pos = m.final_position - (float) (m.final-current)*m.delta_per_tick;
			m.bone_position->x = current_pos.x;
			m.bone_position->y = current_pos.y;
			m.bone_position->z = current_pos.z;
		}
		else
		{
			m.bone_position->x = m.final_position.x;
			m.bone_position->y = m.final_position.y;
			m.bone_position->z = m.final_position.z;
			moves.DeleteIndexFast (i);
		}
		mod = true;
	}
	//-------
	// Perform all running rotates.
	//-------
	i = rotates.Length ();
	while (i > 0)
	{
		i--;
		sac_rotate_execution& m = rotates[i];
		if (current < m.final)
		{
			float slerp = ((float)(m.final - current)/(float)m.duration);
			m.bone_rotation->quat = m.quat.Slerp(m.current_quat, slerp);
		}
		else
		{
			m.bone_rotation->quat = m.quat;
			rotates.DeleteIndexFast (i);
		}
		mod = true;
	}

	if (current < delay.final)
	{
		return mod;
	}

	if (!delay.final) 
	{
		delay.final = current;
	}

	const csArray<sac_instruction>& instructions = script->GetInstructions ();

	bool loop = true;
	while (loop)
	{
		const sac_instruction& inst = instructions[current_instruction];
		current_instruction++;
		switch (inst.opcode)
		{
			case AC_STOP:
				stop = true;
				loop = false;
				break;
			case AC_DELAY:
				{
					csTicks delay_time = (csTicks) ( (float)inst.delay.time*time_factor);
					delay.final += delay_time;
					loop = false;
				}
				break;
			case AC_REPEAT:
				current_instruction = 0;
				loop = false;
				break;
			case AC_ROT:
				if (bones[inst.rotate.bone_id]->GetMode () != BM_NONE)
				{
					if (!inst.rotate.duration)
					{
						bone_transform_data *bone_rot = GetBoneRotation(bones[inst.rotate.bone_id]);
						bone_rot->quat.x = inst.rotate.quat_x;
						bone_rot->quat.y = inst.rotate.quat_y;
						bone_rot->quat.z = inst.rotate.quat_z;
						bone_rot->quat.r = inst.rotate.quat_r;
					}
					else
					{
						sac_rotate_execution m;
						csTicks duration = (csTicks) ( (float)inst.rotate.duration*time_factor);
						m.final = delay.final + duration;
						m.bone = bones[inst.rotate.bone_id];
						m.duration = duration;

						m.bone_rotation = GetBoneRotation(m.bone);
						m.flag = false;

						m.current_quat = m.bone_rotation->quat;
						m.quat.x = inst.rotate.quat_x;
						m.quat.y = inst.rotate.quat_y;
						m.quat.z = inst.rotate.quat_z;
						m.quat.r = inst.rotate.quat_r;
						rotates.Push (m);
					}
				}
				break;
			case AC_MOVE:
				if (bones[inst.movement.bone_id]->GetMode () != BM_NONE)
				{
					if (!inst.movement.duration)
					{
						bone_transform_data *bone_pos = GetBonePosition(bones[inst.movement.bone_id]);
						bone_pos->x = inst.movement.posx;
						bone_pos->y = inst.movement.posy;
						bone_pos->z = inst.movement.posz;
					}
					else
					{
						sac_move_execution m;
						csTicks duration = (csTicks) ( (float)inst.movement.duration*time_factor);
						m.final = delay.final + duration;
						m.bone = bones[inst.movement.bone_id];
						m.final_position.x = inst.movement.posx;
						m.final_position.y = inst.movement.posy;
						m.final_position.z = inst.movement.posz;

						m.bone_position = GetBonePosition(m.bone);
						csVector3 current_position = csVector3 (m.bone_position->x, 
							m.bone_position->y, m.bone_position->z);
						csVector3 delta_position = m.final_position - current_position;
						m.delta_per_tick = delta_position/ (float)duration;
						moves.Push (m);
					}
				}
				break;
		}
	}
	return mod;
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControl::csGenmeshSkelAnimationControl (
	csGenmeshSkelAnimationControlFactory* fact, iObjectRegistry* object_reg)
{
	SCF_CONSTRUCT_IBASE (0);
	csGenmeshSkelAnimationControl::object_reg = object_reg;

	factory = fact;
	num_animated_verts = 0;
	animated_verts = 0;
	transformed_verts = 0;
	animated_colors = 0;

	last_update_time = (csTicks)~0;
	last_version_id = (uint32)~0;

	animates_vertices = fact->AnimatesVertices ();
	animates_texels = fact->AnimatesTexels ();
	animates_colors = fact->AnimatesColors ();
	animates_normals = fact->AnimatesNormals ();

	dirty_vertices = true;
	dirty_texels = true;
	dirty_colors = true;
	dirty_normals = true;
	vertices_mapped = false;

	always_update = false;
}

csGenmeshSkelAnimationControl::~csGenmeshSkelAnimationControl ()
{
	factory->UnregisterAUAnimation(this);
	delete[] animated_verts;
	delete[] animated_colors;
	SCF_DESTRUCT_IBASE ();
}

//void csGenmeshSkelAnimationControl::UpdateAnimation (csTicks current,
//	int num_verts, uint32 version_id)
void csGenmeshSkelAnimationControl::UpdateAnimation (csTicks current)
{
	if (!vertices_mapped) 
		return;
	// Make sure our arrays have the correct size.
	//UpdateArrays (num_verts);
	bool mod = false;
	if (current != last_update_time)
	{
		last_update_time = current;
		size_t i = running_scripts.Length ();
		while (i > 0)
		{
			i--;
			bool stop = false;
			if (running_scripts[i]->Do (current, stop))
				mod = true;
			if (stop)
				running_scripts.DeleteIndexFast (i);
		}
	}

	// If the input arrays changed we need to update animated vertex arrays
	// anyway.
	/*
	if (last_version_id != version_id)
	{
		mod = true;
		last_version_id = version_id;
	}
	*/

	if (mod)
	{
		dirty_vertices = true;
		dirty_texels = true;
		dirty_colors = true;
		dirty_normals = true;

		size_t i;
		for (i = 0 ; i < bones.Length () ; i++)
		{
			bones[i]->UpdateRotation();
			bones[i]->UpdatePosition();
			bones[i]->FireCallback();
		}

		// Update the animated vertices now.
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
						parent_bone->UpdateBones (parent_bone->GetRigidBody ());
					else
						parent_bone->UpdateBones ();
				break;
			}
		}
	}
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
		animated_colors = new csColor[num_verts];

		last_version_id = (uint32)~0;
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
			csSkelBone *p = CS_STATIC_CAST(csSkelBone*,f_bones[i]->GetParent());
			size_t parent_index = f_bones.Find (p);
			csRef<csSkelBone> child = bones[i];
			bones[parent_index]->AddBone (child);
			child->SetParent (bones[parent_index]);
		}
	}


	if (f_parent_bones.Length () > parent_bones.Length ())
		parent_bones.SetLength (f_parent_bones.Length ());

	for (i = 0 ; i < f_parent_bones.Length () ; i++)
		parent_bones[i] = f_parent_bones[i];

	for (i = 0 ; i < parent_bones.Length () ; i++)
		bones[parent_bones[i]]->UpdateBones ();

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

const csVector3* csGenmeshSkelAnimationControl::UpdateVertices (csTicks current,
	const csVector3* verts, int num_verts, uint32 version_id)
{
	if (!animates_vertices) return verts;

	// Perform all running scripts.
	size_t i;

	if (!vertices_mapped)
	{
		TransformVerticesToBones (verts, num_verts);
		vertices_mapped = true;
	}
	
	UpdateArrays (num_verts);

	if (!always_update)
	{
		UpdateAnimation (current);
		//UpdateAnimation (current, num_verts, version_id);
	}

	/*
	for (i = 0 ; i < bones.Length () ; i++)
	{
		bones[i]->UpdateRotation();
		bones[i]->UpdatePosition();
	}
	*/

	// Update the animated vertices now.
	if (dirty_vertices)
	{
		/*
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
						parent_bone->UpdateBones (parent_bone->GetRigidBody ());
					else
						parent_bone->UpdateBones ();
				break;
			}
		}
		*/
		
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

	return normals;
}

const csColor* csGenmeshSkelAnimationControl::UpdateColors (csTicks current,
	const csColor* colors, int num_colors, uint32 version_id)
{
	return colors;
}

iGenMeshSkeletonScript* csGenmeshSkelAnimationControl::Execute (const char* scriptname)
{
	csSkelAnimControlScript* script = factory->FindScript (scriptname);
	if (!script) return 0;
	csRef<csSkelAnimControlRunnable> runnable = 
		csPtr<csSkelAnimControlRunnable> (new csSkelAnimControlRunnable (script, this));
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

void csGenmeshSkelAnimationControl::SetAlwaysUpdate(bool always_update)
{
	always_update ? factory->RegisterAUAnimation(this) : factory->UnregisterAUAnimation(this);
	csGenmeshSkelAnimationControl::always_update = always_update; 
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControlFactory::csGenmeshSkelAnimationControlFactory (
	csGenmeshSkelAnimationControlType* type, iObjectRegistry* object_reg)
{
	SCF_CONSTRUCT_IBASE (0);
	csGenmeshSkelAnimationControlFactory::type = type;
	csGenmeshSkelAnimationControlFactory::object_reg = object_reg;
	InitTokenTable (xmltokens);

	animates_vertices = false;
	animates_texels = false;
	animates_colors = false;
	animates_normals = false;
	has_hierarchical_bones = false;
	always_update = false;
}

csGenmeshSkelAnimationControlFactory::~csGenmeshSkelAnimationControlFactory ()
{
	SCF_DESTRUCT_IBASE ();
}

csPtr<iGenMeshAnimationControl> csGenmeshSkelAnimationControlFactory::
	CreateAnimationControl ()
{
	csGenmeshSkelAnimationControl* ctrl = new csGenmeshSkelAnimationControl (this, object_reg);
	size_t i;
	for (i = 0 ; i < autorun_scripts.Length () ; i++)
		ctrl->Execute (autorun_scripts[i]);
	if (always_update)
	{
		ctrl->SetAlwaysUpdate(true);
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

const char* csGenmeshSkelAnimationControlFactory::LoadScriptFile(const char *filename)
{
	return 0;
}

void csGenmeshSkelAnimationControlFactory::DeleteScript(const char *script_name)
{
}

void csGenmeshSkelAnimationControlFactory::DeleteAllScripts()
{
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
						return "Bad range in bone definition!";
					float weight = child->GetAttributeValueAsFloat ("weight");
					float col_weight = child->GetAttributeValueAsFloat ("col_weight");
					int i;
					for (i = from_idx ; i <= to_idx ; i++)
						bone->AddVertex (i, weight, col_weight);
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
						bone->GetTransform().SetOrigin (v);
					}

					csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
					if (matrix_node)
					{
						csRef<iDocumentNodeIterator> it = matrix_node->GetNodes ();
						while (it->HasNext ())
						{
							csRef<iDocumentNode> child = it->Next ();
							if (child->GetType () != CS_NODE_ELEMENT) continue;
							const char* value = child->GetValue ();
							csStringID id = xmltokens.Request (value);
							switch (id)
							{
								case XMLTOKEN_ROTX:
										bone->SetAxisAngle (0, child->GetContentsValueAsFloat ());
									break;
								case XMLTOKEN_ROTY:
										bone->SetAxisAngle (1, child->GetContentsValueAsFloat ());
									break;
								case XMLTOKEN_ROTZ:
										bone->SetAxisAngle (2, child->GetContentsValueAsFloat ());
									break;
							}
						}
						
						/*
						csMatrix3 m;
						if (!SyntaxService->ParseMatrix (matrix_node, m))
							return false;
						bone->GetTransform().SetO2T (m);
						*/
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

	csRef<iDocumentNodeIterator> it = node->GetNodes ();
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
					sac_instruction& instr = ad.script->AddInstruction (AC_MOVE);
					instr.movement.bone_id = bone_id;
					instr.movement.duration = child->GetAttributeValueAsInt ("duration");
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
					sac_instruction& instr = ad.script->AddInstruction (AC_ROT);
					instr.rotate.bone_id = bone_id;
					instr.rotate.duration = child->GetAttributeValueAsInt ("duration");
					csVector3 v;
					v.x = (child->GetAttributeValueAsFloat ("x")*180)/PI;
					v.y = (child->GetAttributeValueAsFloat ("y")*180)/PI;
					v.z = (child->GetAttributeValueAsFloat ("z")*180)/PI;
					csQuaternion q;
					q.SetWithEuler(v);

					instr.rotate.quat_x = q.x;

					//Why?
					instr.rotate.quat_y = -q.y;

					instr.rotate.quat_z = q.z;
					instr.rotate.quat_r = q.r;
					/*
					csMatrix3 m = csXRotMatrix3(child->GetAttributeValueAsFloat ("x"))*
								csYRotMatrix3(child->GetAttributeValueAsFloat ("y"))*
								csZRotMatrix3(child->GetAttributeValueAsFloat ("z"));
					q = csQuaternion(m);
					csPrintf("%s x %.3f y %.3f z %.3f r %.3f\n\n", bones[bone_id]->GetName(), q.x, q.y, q.z, q.r);
					instr.rotate.quat_x = q.x;
					instr.rotate.quat_y = q.y;
					instr.rotate.quat_z = q.z;
					instr.rotate.quat_r = q.r;
					*/

					animates_vertices = true;
				}
				break;
			case XMLTOKEN_DELAY:
				{
					sac_instruction& instr = ad.script->AddInstruction (AC_DELAY);
					instr.delay.time = child->GetAttributeValueAsInt ("time");
					ad.script->GetTime () += instr.delay.time;
				}
				break;
			case XMLTOKEN_REPEAT:
				ad.script->AddInstruction (AC_REPEAT);
				break;
			default:
				error_buf.Format (
				"Don't recognize token '%s' in anim control script!",
				value);
				return error_buf;
		}
	}
	ad.script->AddInstruction (AC_STOP);
	scripts.Push (ad.script);
	ad.script = 0;	// Prevent DeleteScript instance from deleting script.
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
			case XMLTOKEN_UPDATE:
				{
					const char* update_mode = child->GetAttributeValue ("mode");
					if (!strcmp(update_mode, "always"))
					{
						always_update = true;
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

void csGenmeshSkelAnimationControlFactory::RegisterAUAnimation(csGenmeshSkelAnimationControl *anim)
{
	type->RegisterAUAnimation(anim);
}

void csGenmeshSkelAnimationControlFactory::UnregisterAUAnimation(csGenmeshSkelAnimationControl *anim)
{
	type->UnregisterAUAnimation(anim);
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
  if (ev.Type == csevBroadcast and csCommandEventHelper::GetCode(&ev) == cscmdPreProcess)
  {
      UpdateAUAnimations(vc->GetCurrentTicks());
	  return true;
  }
  return false;
}
