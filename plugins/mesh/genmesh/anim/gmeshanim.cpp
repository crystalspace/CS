/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"
#include "gmeshanim.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csGenmeshAnimationControlType)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControlType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshAnimationControlType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGenmeshAnimationControlType)

SCF_IMPLEMENT_IBASE (csGenmeshAnimationControlFactory)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControlFactory)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGenmeshAnimationControl)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControl)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControlState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGenmeshAnimationControlType::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

//-------------------------------------------------------------------------

csAnimControlGroup::csAnimControlGroup (const char* name)
{
  csAnimControlGroup::name = csStrNew (name);
  parent = 0;
  color.Set (1, 1, 1);
}

void csAnimControlGroup::AddVertex (int idx, float weight, float col_weight)
{
  ac_vertex_data vt;
  vt.idx = idx;
  vt.weight = weight;
  vt.col_weight = col_weight;
  vertices.Push (vt);
}

//-------------------------------------------------------------------------

csAnimControlScript::csAnimControlScript (const char* name)
{
  csAnimControlScript::name = csStrNew (name);
}

ac_instruction& csAnimControlScript::AddInstruction (ac_opcode opcode)
{
  ac_instruction instr;
  size_t idx = instructions.Push (instr);
  instructions[idx].opcode = opcode;
  return instructions[idx];
}

//-------------------------------------------------------------------------

csAnimControlRunnable::csAnimControlRunnable (csAnimControlScript* script,
    csGenmeshAnimationControlFactory* factory)
{
  csAnimControlRunnable::script = script;
  csAnimControlRunnable::factory = factory;
  current_instruction = 0;
  delay.final = 0;
}

csAnimControlRunnable::~csAnimControlRunnable ()
{
}

bool csAnimControlRunnable::Do (csTicks current, bool& stop)
{
  stop = false;
  bool mod = false;

  const csPDelArray<csAnimControlGroup>& groups = factory->GetGroups ();

  //-------
  // Perform all running colors.
  //-------
  size_t i = colors.Length ();
  while (i > 0)
  {
    i--;
    ac_color_execution& m = colors[i];
    if (current < m.final)
    {
      m.group->GetColor ().Set (m.final_color
	  -float (m.final-current) * m.delta_per_tick);
    }
    else
    {
      m.group->GetColor ().Set (m.final_color);
      colors.DeleteIndexFast (i);
    }
    mod = true;
  }

  //-------
  // Perform all running moves.
  //-------
  i = moves.Length ();
  while (i > 0)
  {
    i--;
    ac_move_execution& m = moves[i];
    if (current < m.final)
    {
      m.group->GetTransform ().SetOrigin (m.final_position
	  -float (m.final-current) * m.delta_per_tick);
    }
    else
    {
      m.group->GetTransform ().SetOrigin (m.final_position);
      moves.DeleteIndexFast (i);
    }
    mod = true;
  }

  //-------
  // Perform all running scales.
  //-------
  i = scales.Length ();
  while (i > 0)
  {
    i--;
    ac_scale_execution& m = scales[i];
    float scale;
    if (current < m.final)
      scale = m.final_scale - (m.final-current) * m.delta_scale_per_tick;
    else
      scale = m.final_scale;
    csMatrix3 sca;
    switch (m.axis)
    {
      case 0: sca = csXScaleMatrix3 (scale); break;
      case 1: sca = csYScaleMatrix3 (scale); break;
      case 2: sca = csZScaleMatrix3 (scale); break;
    }
    m.group->GetTransform () = m.base_transform *
	csReversibleTransform (sca, csVector3 (0));

    if (current >= m.final)
      scales.DeleteIndexFast (i);
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
      angle = m.final_angle - (m.final-current) * m.delta_angle_per_tick;
    else
      angle = m.final_angle;
    csMatrix3 rot;
    switch (m.axis)
    {
      case 0: rot = csXRotMatrix3 (angle); break;
      case 1: rot = csYRotMatrix3 (angle); break;
      case 2: rot = csZRotMatrix3 (angle); break;
    }
    m.group->GetTransform () = m.base_transform *
	csReversibleTransform (rot, csVector3 (0));

    if (current >= m.final)
      rotates.DeleteIndexFast (i);
    mod = true;
  }

  //-------
  // Delay if needed.
  //-------
  if (current < delay.final)
  {
    return mod;
  }

  const csArray<ac_instruction>& instructions = script->GetInstructions ();
  // We keep executing instructions until one of the following occurs:
  // 	'stop' operation
  //	'delay' operation
  //	any operation that changes control flow (to prevent loops)
  while (true)
  {
    const ac_instruction& inst = instructions[current_instruction];
    current_instruction++;
    switch (inst.opcode)
    {
      case AC_STOP:
        stop = true;
        return mod;
      case AC_DELAY:
        delay.final = current + inst.delay.time;
        return mod;
      case AC_REPEAT:
        current_instruction = 0;
        return mod;
      case AC_SCALEX:
      case AC_SCALEY:
      case AC_SCALEZ:
	{
	  ac_scale_execution m;
	  m.final = current + inst.scale.duration;
	  m.group = groups[inst.scale.group_id];
	  m.final_scale = inst.scale.scale;
	  m.axis = inst.opcode - AC_SCALEX;
	  if (inst.scale.duration == 0)
	  {
	    // Scale instantly.
	    csMatrix3 sca;
	    switch (m.axis)
	    {
	      case 0: sca = csXScaleMatrix3 (m.final_scale); break;
	      case 1: sca = csYScaleMatrix3 (m.final_scale); break;
	      case 2: sca = csZScaleMatrix3 (m.final_scale); break;
	    }
	    m.group->GetTransform () *= csReversibleTransform (sca,
	    	csVector3 (0));
	  }
	  else
	  {
	    m.base_transform = m.group->GetTransform ();
	    m.delta_scale_per_tick = m.final_scale
	    	/ float (inst.scale.duration);
	    scales.Push (m);
	  }
	}
	break;
      case AC_ROTX:
      case AC_ROTY:
      case AC_ROTZ:
	{
	  ac_rotate_execution m;
	  m.final = current + inst.rotate.duration;
	  m.group = groups[inst.rotate.group_id];
	  m.final_angle = inst.rotate.angle;
	  m.axis = inst.opcode - AC_ROTX;
	  if (inst.rotate.duration == 0)
	  {
	    // Rotate instantly.
	    csMatrix3 rot;
	    switch (m.axis)
	    {
	      case 0: rot = csXRotMatrix3 (m.final_angle); break;
	      case 1: rot = csYRotMatrix3 (m.final_angle); break;
	      case 2: rot = csZRotMatrix3 (m.final_angle); break;
	    }
	    m.group->GetTransform () *= csReversibleTransform (rot,
	    	csVector3 (0));
	  }
	  else
	  {
	    m.base_transform = m.group->GetTransform ();
	    m.delta_angle_per_tick = m.final_angle
	    	/ float (inst.rotate.duration);
	    rotates.Push (m);
	  }
	}
	break;
      case AC_COLOR:
	{
	  ac_color_execution m;
	  m.final = current + inst.color.duration;
	  m.group = groups[inst.color.group_id];
	  csColor4 current_col = m.group->GetColor ();
	  m.final_color.Set (inst.color.red, inst.color.green,
	  	inst.color.blue);
	  if (inst.color.duration == 0)
	  {
	    // Move instantly.
	    m.group->GetColor ().Set (m.final_color);
	  }
	  else
	  {
	    m.delta_per_tick = (m.final_color-current_col)
	    	/ float (inst.color.duration);
	    colors.Push (m);
	  }
	}
        break;
      case AC_MOVE:
	{
	  ac_move_execution m;
	  m.final = current + inst.movement.duration;
	  m.group = groups[inst.movement.group_id];
	  csVector3 current_pos = m.group->GetTransform ().GetOrigin ();
	  csVector3 delta (inst.movement.dx, inst.movement.dy,
	  	inst.movement.dz);
	  m.final_position = current_pos + delta;
	  if (inst.movement.duration == 0)
	  {
	    // Move instantly.
	    m.group->GetTransform ().SetOrigin (m.final_position);
	  }
	  else
	  {
	    m.delta_per_tick = delta / float (inst.movement.duration);
	    moves.Push (m);
	  }
	}
        break;
    }
  }

  return mod;
}

//-------------------------------------------------------------------------

csGenmeshAnimationControl::csGenmeshAnimationControl (
	csGenmeshAnimationControlFactory* fact)
{
  SCF_CONSTRUCT_IBASE (0);
  factory = fact;
  num_animated_verts = 0;
  animated_verts = 0;
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
}

csGenmeshAnimationControl::~csGenmeshAnimationControl ()
{
  delete[] animated_verts;
  delete[] animated_colors;

  SCF_DESTRUCT_IBASE ();
}

void csGenmeshAnimationControl::UpdateAnimation (csTicks current,
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

void csGenmeshAnimationControl::UpdateArrays (int num_verts)
{
  if (num_verts != num_animated_verts)
  {
    num_animated_verts = num_verts;
    delete[] animated_verts;
    animated_verts = new csVector3[num_verts];
    delete[] animated_colors;
    animated_colors = new csColor4[num_verts];
    last_version_id = (uint32)~0;
  }
}

csArray<csReversibleTransform> csGenmeshAnimationControl::group_transforms;
csArray<csColor4> csGenmeshAnimationControl::group_colors;

const csVector3* csGenmeshAnimationControl::UpdateVertices (csTicks current,
	const csVector3* verts, int num_verts, uint32 version_id)
{
  if (!animates_vertices) return verts;

  // Perform all running scripts.
  UpdateAnimation (current, num_verts, version_id);

  // Update the animated vertices now.
  if (dirty_vertices)
  {
    const csPDelArray<csAnimControlGroup>& groups = factory->GetGroups ();
    size_t i;

    if (groups.Length () > group_transforms.Length ())
      group_transforms.SetLength (groups.Length ());

    if (factory->HasHierarchicalGroups ())
      for (i = 0 ; i < groups.Length () ; i++)
        group_transforms[i] = groups[i]->GetFullTransform ();
    else
      for (i = 0 ; i < groups.Length () ; i++)
        group_transforms[i] = groups[i]->GetTransform ();

    const csArray<csArray<ac_group_data> >& groups_vertices = factory
    	->GetGroupsVerticesMapping ();
    for (i = 0 ; i < (size_t)num_verts ; i++)
    {
      if (i >= groups_vertices.Length ())
        animated_verts[i] = verts[i];
      else
      {
        const csArray<ac_group_data>& vtgr = groups_vertices[i];
	if (vtgr.Length () == 0)
	{
	  animated_verts[i] = verts[i];
	}
        else if (vtgr.Length () == 1)
	{
	  csReversibleTransform& transform = group_transforms[vtgr[0].idx];
	  animated_verts[i] = transform.Other2This (verts[i]);
	}
	else
	{
	  csReversibleTransform& transform = group_transforms[vtgr[0].idx];
	  float total_weight = vtgr[0].weight;
	  csVector3 orig = vtgr[0].weight * transform.Other2This (verts[i]);
	  size_t j;
	  for (j = 1 ; j < vtgr.Length () ; j++)
	  {
	    csReversibleTransform& transform2 = group_transforms[vtgr[j].idx];
	    total_weight += vtgr[j].weight;
	    orig += vtgr[j].weight * transform2.Other2This (verts[i]);
	  }
	  animated_verts[i] = orig / total_weight;
	}
      }
    }
  }

  return animated_verts;
}

const csVector2* csGenmeshAnimationControl::UpdateTexels (csTicks current,
	const csVector2* texels, int num_texels, uint32 version_id)
{
  if (!animates_texels) return texels;
  //UpdateAnimation (current);
  return texels;
}

const csVector3* csGenmeshAnimationControl::UpdateNormals (csTicks current,
	const csVector3* normals, int num_normals, uint32 version_id)
{
  if (!animates_normals) return normals;

  return normals;
}

const csColor4* csGenmeshAnimationControl::UpdateColors (csTicks current,
	const csColor4* colors, int num_colors, uint32 version_id)
{
  if (!animates_colors) return colors;

  // Perform all running scripts if needed.
  UpdateAnimation (current, num_colors, version_id);

  // Update the animated colors now.
  if (dirty_colors)
  {
    const csPDelArray<csAnimControlGroup>& groups = factory->GetGroups ();
    size_t i;

    if (groups.Length () > group_colors.Length ())
      group_colors.SetLength (groups.Length ());

    if (factory->HasHierarchicalGroups ())
      for (i = 0 ; i < groups.Length () ; i++)
        group_colors[i] = groups[i]->GetFullColor ();
    else
      for (i = 0 ; i < groups.Length () ; i++)
        group_colors[i] = groups[i]->GetColor ();

    const csArray<csArray<ac_group_data> >& gc = factory
    	->GetGroupsColorsMapping ();
    for (i = 0 ; i < (size_t)num_colors ; i++)
    {
      if (i >= gc.Length ())
        animated_colors[i] = colors[i];
      else
      {
        const csArray<ac_group_data>& vtgr = gc[i];
	if (vtgr.Length () == 0)
	{
	  animated_colors[i] = colors[i];
	}
        else if (vtgr.Length () == 1)
	{
	  csColor4& color = group_colors[vtgr[0].idx];
	  animated_colors[i] = color * colors[i];
	}
	else
	{
	  csColor4& color = group_colors[vtgr[0].idx];
	  float total_weight = vtgr[0].weight;
	  csColor4 orig = vtgr[0].weight * color * colors[i];
	  size_t j;
	  for (j = 1 ; j < vtgr.Length () ; j++)
	  {
	    csColor4& color2 = group_colors[vtgr[j].idx];
	    total_weight += vtgr[j].weight;
	    orig += vtgr[j].weight * color2 * colors[i];
	  }
	  animated_colors[i] = orig / total_weight;
	}
      }
    }
  }

  return animated_colors;
}

bool csGenmeshAnimationControl::Execute (const char* scriptname)
{
  csAnimControlScript* script = factory->FindScript (scriptname);
  if (!script) return false;
  csAnimControlRunnable* runnable = new csAnimControlRunnable (script,
      factory);
  running_scripts.Push (runnable);
  return true;
}

//-------------------------------------------------------------------------

csGenmeshAnimationControlFactory::csGenmeshAnimationControlFactory (
	csGenmeshAnimationControlType* type, iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (0);
  csGenmeshAnimationControlFactory::type = type;
  csGenmeshAnimationControlFactory::object_reg = object_reg;
  InitTokenTable (xmltokens);

  animates_vertices = false;
  animates_texels = false;
  animates_colors = false;
  animates_normals = false;
  has_hierarchical_groups = false;
}

csGenmeshAnimationControlFactory::~csGenmeshAnimationControlFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

csPtr<iGenMeshAnimationControl> csGenmeshAnimationControlFactory::
	CreateAnimationControl ()
{
  csGenmeshAnimationControl* ctrl = new csGenmeshAnimationControl (this);
  size_t i;
  for (i = 0 ; i < autorun_scripts.Length () ; i++)
    ctrl->Execute (autorun_scripts[i]);
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

void csGenmeshAnimationControlFactory::UpdateGroupsMapping ()
{
  size_t i;
  for (i = 0 ; i < groups.Length () ; i++)
  {
    csAnimControlGroup* g = groups[i];
    const csArray<ac_vertex_data>& vtdata = g->GetVertexData ();
    size_t j;
    for (j = 0 ; j < vtdata.Length () ; j++)
    {
      if (vtdata[j].weight > SMALL_EPSILON)
      {
        csArray<ac_group_data>& vertices = groups_vertices
      	  .GetExtend (vtdata[j].idx);
        ac_group_data gd;
        gd.idx = (int)i;
        gd.weight = vtdata[j].weight;
        vertices.Push (gd);	// Push group index.
      }
      if (vtdata[j].col_weight > SMALL_EPSILON)
      {
        csArray<ac_group_data>& colors = groups_colors
      	  .GetExtend (vtdata[j].idx);
        ac_group_data gd;
        gd.idx = (int)i;
        gd.weight = vtdata[j].col_weight;
        colors.Push (gd);	// Push group index.
      }
    }
  }
}

csAnimControlScript* csGenmeshAnimationControlFactory::FindScript (
	const char* scriptname) const
{
  size_t i;
  for (i = 0 ; i < scripts.Length () ; i++)
    if (strcmp (scripts[i]->GetName (), scriptname) == 0)
      return scripts[i];
  return 0;
}

csAnimControlGroup* csGenmeshAnimationControlFactory::FindGroup (
	const char* groupname) const
{
  size_t i;
  for (i = 0 ; i < groups.Length () ; i++)
    if (strcmp (groups[i]->GetName (), groupname) == 0)
      return groups[i];
  return 0;
}

size_t csGenmeshAnimationControlFactory::FindGroupIndex (
	const char* groupname) const
{
  size_t i;
  for (i = 0 ; i < groups.Length () ; i++)
    if (strcmp (groups[i]->GetName (), groupname) == 0)
      return i;
  return (size_t)~0;
}

const char* csGenmeshAnimationControlFactory::ParseGroup (iDocumentNode* node,
	csAnimControlGroup* parent)
{
  const char* groupname = node->GetAttributeValue ("name");
  if (!groupname)
    return "Name of the group is missing!";
  csAnimControlGroup* group = new csAnimControlGroup (groupname);

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
	    return "Bad range in group definition!";
	  float weight = child->GetAttributeValueAsFloat ("weight");
	  float col_weight = child->GetAttributeValueAsFloat ("col_weight");
	  int i;
	  for (i = from_idx ; i <= to_idx ; i++)
	    group->AddVertex (i, weight, col_weight);
	}
        break;
      case XMLTOKEN_VERTEX:
        {
	  int idx = child->GetAttributeValueAsInt ("idx");
	  float weight = child->GetAttributeValueAsFloat ("weight");
	  float col_weight = child->GetAttributeValueAsFloat ("col_weight");
	  group->AddVertex (idx, weight, col_weight);
	}
        break;
      case XMLTOKEN_GROUP:
        {
	  const char* err = ParseGroup (child, group);
	  if (err != 0) return err;
	}
        break;
      default:
        error_buf.Format (
		"Don't recognize token '%s' in anim control group!",
		value);
	delete group;
        return error_buf;
    }
  }
  if (parent)
  {
    parent->AddGroup (group);
    group->SetParent (parent);
    has_hierarchical_groups = true;
  }
  groups.Push (group);
  return 0;
}

const char* csGenmeshAnimationControlFactory::ParseScript (iDocumentNode* node)
{
  const char* scriptname = node->GetAttributeValue ("name");
  if (!scriptname)
    return "Name of the script is missing!";

  // Small class to make sure script gets deleted in case of error.
  struct AutoDelete
  {
    csAnimControlScript* script;
    ~AutoDelete () { delete script; }
  } ad;
  ad.script = new csAnimControlScript (scriptname);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLOR:
        {
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <color>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <color>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_COLOR);
	  instr.color.group_id = group_id;
	  instr.color.duration = child->GetAttributeValueAsInt ("duration");
	  instr.color.red = child->GetAttributeValueAsFloat ("red");
	  instr.color.green = child->GetAttributeValueAsFloat ("green");
	  instr.color.blue = child->GetAttributeValueAsFloat ("blue");
	  animates_colors = true;
	}
        break;
      case XMLTOKEN_MOVE:
        {
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <move>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <move>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_MOVE);
	  instr.movement.group_id = group_id;
	  instr.movement.duration = child->GetAttributeValueAsInt ("duration");
	  instr.movement.dx = child->GetAttributeValueAsFloat ("dx");
	  instr.movement.dy = child->GetAttributeValueAsFloat ("dy");
	  instr.movement.dz = child->GetAttributeValueAsFloat ("dz");
	  animates_vertices = true;
	}
        break;
      case XMLTOKEN_SCALEX:
	{
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <scalex>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <scalex>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_SCALEX);
	  instr.scale.group_id = group_id;
	  instr.scale.duration = child->GetAttributeValueAsInt ("duration");
	  instr.scale.scale = child->GetAttributeValueAsFloat ("scale");
	  animates_vertices = true;
	}
	break;
      case XMLTOKEN_SCALEY:
	{
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <scaley>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <scaley>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_SCALEY);
	  instr.scale.group_id = group_id;
	  instr.scale.duration = child->GetAttributeValueAsInt ("duration");
	  instr.scale.scale = child->GetAttributeValueAsFloat ("scale");
	  animates_vertices = true;
	}
	break;
      case XMLTOKEN_SCALEZ:
	{
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <scalez>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <scalez>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_SCALEZ);
	  instr.scale.group_id = group_id;
	  instr.scale.duration = child->GetAttributeValueAsInt ("duration");
	  instr.scale.scale = child->GetAttributeValueAsFloat ("scale");
	  animates_vertices = true;
	}
	break;
      case XMLTOKEN_ROTX:
	{
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <rotx>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <rotx>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_ROTX);
	  instr.rotate.group_id = group_id;
	  instr.rotate.duration = child->GetAttributeValueAsInt ("duration");
	  instr.rotate.angle = child->GetAttributeValueAsFloat ("angle");
	  animates_vertices = true;
	}
	break;
      case XMLTOKEN_ROTY:
	{
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <roty>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <roty>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_ROTY);
	  instr.rotate.group_id = group_id;
	  instr.rotate.duration = child->GetAttributeValueAsInt ("duration");
	  instr.rotate.angle = child->GetAttributeValueAsFloat ("angle");
	  animates_vertices = true;
	}
	break;
      case XMLTOKEN_ROTZ:
	{
	  const char* groupname = child->GetAttributeValue ("group");
	  if (!groupname) return "Missing group name for <rotz>!";
	  size_t group_id = FindGroupIndex (groupname);
	  if (group_id == (size_t)~0) return "Can't find group for <rotz>!";
	  ac_instruction& instr = ad.script->AddInstruction (AC_ROTZ);
	  instr.rotate.group_id = group_id;
	  instr.rotate.duration = child->GetAttributeValueAsInt ("duration");
	  instr.rotate.angle = child->GetAttributeValueAsFloat ("angle");
	  animates_vertices = true;
	}
	break;
      case XMLTOKEN_DELAY:
        {
	  ac_instruction& instr = ad.script->AddInstruction (AC_DELAY);
	  instr.delay.time = child->GetAttributeValueAsInt ("time");
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

const char* csGenmeshAnimationControlFactory::Load (iDocumentNode* node)
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
      case XMLTOKEN_GROUP:
        {
	  const char* err = ParseGroup (child, 0);
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
        error_buf.Format (
		"Don't recognize token '%s' in anim control!",
		value);
        return error_buf;
    }
  }

  UpdateGroupsMapping ();

  return 0;
}

const char* csGenmeshAnimationControlFactory::Save (iDocumentNode* parent)
{
  csRef<iFactory> plugin = SCF_QUERY_INTERFACE(type, iFactory);
  if (!plugin) return "Couldn't get Class ID";
  parent->SetAttribute("plugin", plugin->QueryClassID());
  return "Not implemented yet!";
}

//-------------------------------------------------------------------------

csGenmeshAnimationControlType::csGenmeshAnimationControlType (
	iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;
}

csGenmeshAnimationControlType::~csGenmeshAnimationControlType ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshAnimationControlType::Initialize (iObjectRegistry* object_reg)
{
  csGenmeshAnimationControlType::object_reg = object_reg;
  scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing);
  return true;
}

csPtr<iGenMeshAnimationControlFactory> csGenmeshAnimationControlType::
	CreateAnimationControlFactory ()
{
  csGenmeshAnimationControlFactory* ctrl = new csGenmeshAnimationControlFactory
  	(this, object_reg);
  return csPtr<iGenMeshAnimationControlFactory> (ctrl);
}

bool csGenmeshAnimationControlType::HandleEvent (iEvent& ev)
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

