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
#include "csgeom/quaterni.h"
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/eventq.h"
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
	SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControlFactory)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGenmeshSkelAnimationControl)
	SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControl)
	SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonControlState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGenmeshSkelAnimationControlType::EventHandler)
	SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csSkelBone)
	SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonBone)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csSkelAnimControlRunnable)
	SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonScript)
SCF_IMPLEMENT_IBASE_END

//-------------------------------------------------------------------------

csSkelBone::csSkelBone (csGenmeshSkelAnimationControl *animation_control) 
{
	SCF_CONSTRUCT_IBASE (0); 
	csSkelBone::animation_control = animation_control;
	parent = 0;
	bone_mode = BM_SCRIPT;
	rigid_body = 0;
	rot.x = rot.y = rot.z = 0;
	pos.x = pos.y = pos.z = 0;
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

void csSkelBone::UpdateRotation (int axis, float angle)
{
	switch (axis)
	{
		case 0: 
			rot.x = angle;
			rot.mx = csXRotMatrix3 (angle);
			break;
		case 1:
			rot.y = angle;
			rot.my = csYRotMatrix3 (angle);
			break;
		case 2:
			rot.z = angle;
			rot.mz = csZRotMatrix3 (angle);
			break;
	}
	transform.SetO2T (rot.mx*rot.my*rot.mz);
}

void csSkelBone::UpdateRotation (int axis, float angle, csSkelAnimControlRunnable *script_runnable)
{
	bone_transform_data *b_rot = script_rotations.GetElementPointer ( (uint32)script_runnable);
	if (!b_rot) 
		return;
	b_rot->data[axis] = angle;

	// if there is only one script update rotation directly
	if (animation_control->GetRunningScripts ().Length () == 1)
	{
		UpdateRotation (axis, angle);
		return;
	}
	else
	{
		float final_angle_x = 0; 
		float final_angle_y = 0; 
		float final_angle_z = 0;
		float script_factors_total = 0;
		for (size_t i = 0; i < animation_control->GetRunningScripts ().Length (); i++)
		{
			bone_transform_data *b_rot = 
				script_rotations.GetElementPointer ( (uint32)animation_control->GetRunningScripts ().Get (i));
			if (b_rot)
			{
				float script_factor = animation_control->GetRunningScripts ().Get (i)->GetFactor ();
				if (b_rot->active)
				{
					final_angle_x += b_rot->x*script_factor;
					final_angle_y += b_rot->y*script_factor;
					final_angle_z += b_rot->z*script_factor;
					script_factors_total += script_factor;
				}
			}
		}

		if (script_factors_total)
		{
			rot.x = final_angle_x/script_factors_total;
			rot.y = final_angle_y/script_factors_total;
			rot.z = final_angle_z/script_factors_total;
		}
		else
		{
			rot.x = final_angle_x;
			rot.y = final_angle_y;
			rot.z = final_angle_z;
		}

		rot.mx = csXRotMatrix3 (rot.x);
		rot.my = csYRotMatrix3 (rot.y);
		rot.mz = csZRotMatrix3 (rot.z);
		
		transform.SetO2T (rot.mx*rot.my*rot.mz);
	}
}

void csSkelBone::UpdatePosition (float posx, float posy, float posz, csSkelAnimControlRunnable *script_runnable)
{
	bone_transform_data *b_pos = script_positions.GetElementPointer ( (uint32)script_runnable);
	if (!b_pos) 
		return;
	b_pos->x = posx;
	b_pos->y = posy;
	b_pos->z = posz;

	if (animation_control->GetRunningScripts ().Length () == 1)
	{
		UpdatePosition (posx, posy, posz);
		return;
	}
	else
	{
		float final_pos_x = 0; 
		float final_pos_y = 0; 
		float final_pos_z = 0;
		float script_factors_total = 0;
		for (size_t i = 0; i < animation_control->GetRunningScripts ().Length (); i++)
		{
			bone_transform_data *b_pos = 
				script_positions.GetElementPointer ( (uint32)animation_control->GetRunningScripts ().Get (i));
			if (b_pos)
			{
				float script_factor = animation_control->GetRunningScripts ().Get (i)->GetFactor ();
				if (b_pos->active)
				{
					final_pos_x += b_pos->x*script_factor;
					final_pos_y += b_pos->y*script_factor;
					final_pos_z += b_pos->z*script_factor;
					script_factors_total += script_factor;
				}
			}
		}

		if (script_factors_total)
		{
			pos.x = final_pos_x/script_factors_total;
			pos.y = final_pos_y/script_factors_total;
			pos.z = final_pos_z/script_factors_total;
		}
		else
		{
			pos.x = final_pos_x;
			pos.y = final_pos_y;
			pos.z = final_pos_z;
		}

		transform.SetOrigin (csVector3 (pos.x, pos.y, pos.z));
	}
}

void csSkelBone::DeactivateScriptRotation (csSkelAnimControlRunnable *script_runnable)
{
	bone_transform_data *b_rot = script_rotations.GetElementPointer ( (uint32)script_runnable);
	if (!b_rot)
		return;
	b_rot->active = false;
}

void csSkelBone::DeactivateScriptPosition (csSkelAnimControlRunnable *script_runnable)
{
	bone_transform_data *b_pos = script_positions.GetElementPointer ( (uint32)script_runnable);
	if (!b_pos)
		return;
	b_pos->active = false;
}

float csSkelBone::GetRotation (int axis)
{
	return rot.data[axis];
}

bone_transform_data *csSkelBone::ActivateScriptRotation (csSkelAnimControlRunnable *script_runnable)
{
	bone_transform_data *b_rot = script_rotations.GetElementPointer ( (uint32)script_runnable);
	if (b_rot) 
	{
		b_rot->active = true;
		return b_rot;
	}

	b_rot = new bone_transform_data ();
	b_rot->active = true;
	b_rot->x = rot.x;
	b_rot->y = rot.y;
	b_rot->z = rot.z;
	script_rotations.Put ( (uint32)script_runnable, *b_rot);
	return b_rot;
}

bone_transform_data *csSkelBone::ActivateScriptPosition (csSkelAnimControlRunnable *script_runnable)
{
	bone_transform_data *b_pos = script_positions.GetElementPointer ( (uint32)script_runnable);
	if (b_pos) 
	{
		b_pos->active = true;
		return b_pos;
	}

	b_pos = new bone_transform_data ();
	b_pos->active = true;
	b_pos->x = pos.x;
	b_pos->y = pos.y;
	b_pos->z = pos.z;
	script_positions.Put ( (uint32)script_runnable, *b_pos);
	return b_pos;
}

/*
void csSkelBone::RemoveScriptRotation (csSkelAnimControlRunnable *script_runnable)
{
	script_rotations.DeleteAll ( (uint32)script_runnable);
}
*/

void csSkelBone::UpdatePosition (float posx, float posy, float posz)
{
	pos.x = posx; pos.y = posy; pos.z = posz;
	transform.SetOrigin (csVector3 (posx, posy, posz));
}

void csSkelBone::CopyFrom (csSkelBone *other)
{
	name = csStrNew (other->GetName ());
	parent = 0;
	for (size_t i = 0; i < other->GetVertexData ().Length (); i++)
	{
		ac_vertex_data v_data (other->GetVertexData ().Get (i));
		vertices.Push (v_data);
	}

	const csVector3 &other_pos = other->GetPosition ();
	UpdatePosition (other_pos.x, other_pos.y, other_pos.z);
	UpdateRotation (0, other->GetRotation (0));
	UpdateRotation (1, other->GetRotation (1));
	UpdateRotation (2, other->GetRotation (2));
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
	ac_vertex_data vt;
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

ac_instruction& csSkelAnimControlScript::AddInstruction (ac_opcode opcode)
{
	ac_instruction instr;
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
	SCF_DESTRUCT_IBASE ();
}

bool csSkelAnimControlRunnable::Do (csTicks current, bool& stop)
{
	if (!anim_control->GetBones ().Length ())
		return false;

	stop = false;
	bool mod = false;

	csRefArray<csSkelBone>& bones = anim_control->GetBones ();

	//-------
	// Perform all running moves.
	//-------
	size_t i = moves.Length ();
	while (i > 0)
	{
		i--;
		ac_move_execution& m = moves[i];
		csVector3 current_pos;
		if (current < m.final)
		{
			current_pos = m.final_position - (float) (m.final-current)*m.delta_per_tick;
			m.bone->UpdatePosition (current_pos.x, current_pos.y, current_pos.z, this);
		}
		else
		{
			current_pos = m.final_position;
			m.bone->UpdatePosition (current_pos.x, current_pos.y, current_pos.z, this);
			m.bone->DeactivateScriptPosition (this);
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
		ac_rotate_execution& m = rotates[i];
		float angle;
		if (current < m.final)
		{
			angle = m.final_angle - (m.final-current)*m.delta_angle_per_tick;
			m.bone->UpdateRotation (m.axis, angle, this);
		}
		else
		{
			angle = m.final_angle;
			m.bone->UpdateRotation (m.axis, angle, this);
			m.bone->DeactivateScriptRotation (this);
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

	const csArray<ac_instruction>& instructions = script->GetInstructions ();

	bool loop = true;
	while (loop)
	{
		const ac_instruction& inst = instructions[current_instruction];
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
			case AC_ROTX:
			case AC_ROTY:
			case AC_ROTZ:
			{
				if (bones[inst.movement.bone_id]->GetMode () != BM_NONE)
				{
					ac_rotate_execution m;
					csTicks duration = (csTicks) ( (float)inst.rotate.duration*time_factor);
					m.final = delay.final + duration;
					m.bone = bones[inst.rotate.bone_id];
					m.final_angle = inst.rotate.angle;
					m.axis = inst.opcode - AC_ROTX;

					if (!inst.rotate.duration)
					{
						m.bone->UpdateRotation (m.axis, m.final_angle, this);
					}
					else
					{
						bone_transform_data *script_rot = m.bone->ActivateScriptRotation (this);
						float current_rotation = script_rot->data[m.axis];

						float delta_angle = m.final_angle - current_rotation;

						if (delta_angle > PI) 
							delta_angle = delta_angle - TWO_PI;
						else 
							if (delta_angle < -PI) 
							delta_angle = delta_angle + TWO_PI;
					
						m.delta_angle_per_tick = delta_angle/ (float)duration;
						m.flag = false;
						rotates.Push (m);
					}
				}
			}
			break;
			case AC_MOVE:
			{
				if (bones[inst.movement.bone_id]->GetMode () != BM_NONE)
				{
					ac_move_execution m;
					csTicks duration = (csTicks) ( (float)inst.movement.duration*time_factor);
					m.final = delay.final + duration;
					m.bone = bones[inst.movement.bone_id];
					m.final_position.x = inst.movement.posx;
					m.final_position.y = inst.movement.posy;
					m.final_position.z = inst.movement.posz;

					if (!inst.movement.duration)
					{
						m.bone->UpdatePosition (inst.movement.posx, inst.movement.posy, inst.movement.posz, this);
					}
					else
					{
						bone_transform_data *script_pos = m.bone->ActivateScriptPosition (this);
						csVector3 current_position = csVector3 (script_pos->x, script_pos->y, script_pos->z);
						csVector3 delta_position = m.final_position - current_position;
						m.delta_per_tick = delta_position/ (float)duration;
						moves.Push (m);
					}
				}
			}
			break;
		}
	}
	return mod;
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControl::csGenmeshSkelAnimationControl (
	csGenmeshSkelAnimationControlFactory* fact)
{
	SCF_CONSTRUCT_IBASE (0);
	factory = fact;
	num_animated_verts = 0;
	animated_verts = 0;
	transformed_verts = 0;
	animated_colors = 0;

	last_update_time = ~0;
	last_version_id = ~0;

	animates_vertices = fact->AnimatesVertices ();
	animates_texels = fact->AnimatesTexels ();
	animates_colors = fact->AnimatesColors ();
	animates_normals = fact->AnimatesNormals ();

	dirty_vertices = true;
	dirty_texels = true;
	dirty_colors = true;
	dirty_normals = true;
	vertices_mapped = false;
}

csGenmeshSkelAnimationControl::~csGenmeshSkelAnimationControl ()
{
	delete[] animated_verts;
	delete[] animated_colors;
	SCF_DESTRUCT_IBASE ();
}

void csGenmeshSkelAnimationControl::UpdateAnimation (csTicks current,
	int num_verts, uint32 version_id)
{
	// Make sure our arrays have the correct size.
	UpdateArrays (num_verts);

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
			{
				running_scripts.DeleteIndexFast (i);
			}
		}
	}

	// If the input arrays changed we need to update animated vertex arrays
	// anyway.
	if (last_version_id != version_id)
	{
		mod = true;
		last_version_id = version_id;
	}

	if (mod)
	{
		dirty_vertices = true;
		dirty_texels = true;
		dirty_colors = true;
		dirty_normals = true;
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

		last_version_id = ~0;
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
		// Really stupid
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

	csArray<csArray<ac_bone_data> >& bones_vertices = factory->GetBonesVerticesMapping ();
	for (i = 0 ; i < (size_t)num_verts ; i++)
	{
		if (i >= bones_vertices.Length ())
			animated_verts[i] = verts[i];
		else
		{
			csArray<ac_bone_data>& vtgr = bones_vertices[i];
			if (vtgr.Length () == 0)
			{
				animated_verts[i] = verts[i];
			}
			else 
			{
				for (size_t j = 0 ; j < vtgr.Length () ; j++)
				{
					ac_bone_data & g_data = vtgr[j];
					csRef<csSkelBone> bone = bones[vtgr[j].idx];
					ac_vertex_data & v_data = bone->GetVertexData ()[g_data.v_idx];
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

	UpdateAnimation (current, num_verts, version_id);

	if (!vertices_mapped)
	{
		TransformVerticesToBones (verts, num_verts);
		vertices_mapped = true;
	}

	// Update the animated vertices now.
	if (dirty_vertices)
	{
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
		
		csArray<csArray<ac_bone_data> >& bones_vertices = factory->GetBonesVerticesMapping ();
		for (i = 0 ; i < (size_t)num_verts ; i++)
		{
			if (i >= bones_vertices.Length ())
				animated_verts[i] = verts[i];
			else
			{
				csArray<ac_bone_data>& vtgr = bones_vertices[i];
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
						ac_bone_data & g_data = vtgr[j];
						csRef<csSkelBone> bone = bones[g_data.idx];
						ac_vertex_data & v_data = bone->GetVertexData ()[g_data.v_idx];
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
}

csGenmeshSkelAnimationControlFactory::~csGenmeshSkelAnimationControlFactory ()
{
	SCF_DESTRUCT_IBASE ();
}

csPtr<iGenMeshAnimationControl> csGenmeshSkelAnimationControlFactory::
	CreateAnimationControl ()
{
	csGenmeshSkelAnimationControl* ctrl = new csGenmeshSkelAnimationControl (this);
	size_t i;
	for (i = 0 ; i < autorun_scripts.Length () ; i++)
		ctrl->Execute (autorun_scripts[i]);
	return csPtr<iGenMeshAnimationControl> (ctrl);
}

void csGenmeshSkelAnimationControlFactory::UpdateBonesMapping ()
{
	size_t i;
	for (i = 0 ; i < bones.Length () ; i++)
	{
		csSkelBone* g = bones[i];
		const csArray<ac_vertex_data>& vtdata = g->GetVertexData ();
		size_t j;
		for (j = 0 ; j < vtdata.Length () ; j++)
		{
			if (vtdata[j].weight > SMALL_EPSILON)
			{
				csArray<ac_bone_data>& vertices = bones_vertices.GetExtend (vtdata[j].idx);
				ac_bone_data gd;
				gd.idx = i;
				gd.v_idx = j;
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
	return ~0;
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
						bone->UpdatePosition (v.x, v.y, v.z);
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
										bone->UpdateRotation (0, child->GetContentsValueAsFloat ());
									break;
								case XMLTOKEN_ROTY:
										bone->UpdateRotation (1, child->GetContentsValueAsFloat ());
									break;
								case XMLTOKEN_ROTZ:
										bone->UpdateRotation (2, child->GetContentsValueAsFloat ());
									break;
							}
						}
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
				sprintf (error_buf,
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
					ac_instruction& instr = ad.script->AddInstruction (AC_MOVE);
					instr.movement.bone_id = bone_id;
					instr.movement.duration = child->GetAttributeValueAsInt ("duration");
					instr.movement.posx = child->GetAttributeValueAsFloat ("x");
					instr.movement.posy = child->GetAttributeValueAsFloat ("y");
					instr.movement.posz = child->GetAttributeValueAsFloat ("z");
					animates_vertices = true;
				}
				break;
			case XMLTOKEN_ROTX:
				{
					const char* bonename = child->GetAttributeValue ("bone");
					if (!bonename) return "Missing bone name for <rotx>!";
					size_t bone_id = FindBoneIndex (bonename);
					if (bone_id == (size_t)~0) return "Can't find bone for <rotx>!";
					ac_instruction& instr = ad.script->AddInstruction (AC_ROTX);
					instr.rotate.bone_id = bone_id;
					instr.rotate.duration = child->GetAttributeValueAsInt ("duration");
					instr.rotate.angle = child->GetAttributeValueAsFloat ("angle");
					animates_vertices = true;
				}
				break;
			case XMLTOKEN_ROTY:
				{
					const char* bonename = child->GetAttributeValue ("bone");
					if (!bonename) return "Missing bone name for <roty>!";
					size_t bone_id = FindBoneIndex (bonename);
					if (bone_id == (size_t)~0) return "Can't find bone for <roty>!";
					ac_instruction& instr = ad.script->AddInstruction (AC_ROTY);
					instr.rotate.bone_id = bone_id;
					instr.rotate.duration = child->GetAttributeValueAsInt ("duration");
					instr.rotate.angle = child->GetAttributeValueAsFloat ("angle");
					animates_vertices = true;
				}
				break;
			case XMLTOKEN_ROTZ:
				{
					const char* bonename = child->GetAttributeValue ("bone");
					if (!bonename) return "Missing bone name for <rotz>!";
					size_t bone_id = FindBoneIndex (bonename);
					if (bone_id == (size_t)~0) return "Can't find bone for <rotz>!";
					ac_instruction& instr = ad.script->AddInstruction (AC_ROTZ);
					instr.rotate.bone_id = bone_id;
					instr.rotate.duration = child->GetAttributeValueAsInt ("duration");
					instr.rotate.angle = child->GetAttributeValueAsFloat ("angle");
					animates_vertices = true;
				}
				break;
			case XMLTOKEN_DELAY:
				{
					ac_instruction& instr = ad.script->AddInstruction (AC_DELAY);
					instr.delay.time = child->GetAttributeValueAsInt ("time");
					ad.script->GetTime () += instr.delay.time;
				}
				break;
			case XMLTOKEN_REPEAT:
				ad.script->AddInstruction (AC_REPEAT);
				break;
			default:
				sprintf (error_buf,
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
			default:
				sprintf (error_buf,
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
#if 0
	else if (event.Type == csevBroadcast)
	{
		if (event.Command.Code == cscmdPreProcess)
		{
			return HandleStartFrame (event);
		}
#endif
	return false;
}

