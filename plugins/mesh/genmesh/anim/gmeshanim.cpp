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
}

void csAnimControlGroup::AddVertex (int idx, float weight)
{
  ac_vertex_data vt;
  vt.idx = idx;
  vt.weight = weight;
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

csAnimControlRunnable::csAnimControlRunnable (csAnimControlScript* script)
{
  csAnimControlRunnable::script = script;
  current_instruction = 0;
  movement.final = 0;
  delay.final = 0;
}

csAnimControlRunnable::~csAnimControlRunnable ()
{
}

bool csAnimControlRunnable::Do (csTicks current, bool& stop)
{
  stop = false;
  bool mod = false;

  if (current < movement.final)
  {
    mod = true;
  }
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
    switch (inst.opcode)
    {
      case AC_STOP:
        stop = true;
        return mod;
      case AC_DELAY:
        delay.final = current + inst.delay.time;
	printf ("delay=%d %d\n", delay.final, current);
        current_instruction++;
        return mod;
      case AC_REPEAT:
        current_instruction = 0;
	printf ("repeat called!\n"); fflush (stdout);
        return mod;
      case AC_MOVE:
        // If there is a movement operation in progress we abort that.
	// @@@ Take final position from there???
	movement.final = current + inst.movement.duration;
	movement.delta_per_tick = 1.0;//@@@
	//@@@movement.final_position;
	printf ("move called!\n"); fflush (stdout);
        current_instruction++;
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

  last_update_time = ~0;
  last_version_id = ~0;
}

csGenmeshAnimationControl::~csGenmeshAnimationControl ()
{
  delete[] animated_verts;

  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshAnimationControl::UpdateAnimation (csTicks current)
{
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
        running_scripts.DeleteIndex (i);
      }
    }
  }
  return mod;
}

void csGenmeshAnimationControl::UpdateArrays (int num_verts)
{
  if (num_verts != num_animated_verts)
  {
    num_animated_verts = num_verts;
    animated_verts = new csVector3[num_verts];
    last_version_id = ~0;
  }
}

const csVector3* csGenmeshAnimationControl::UpdateVertices (csTicks current,
	const csVector3* verts, int num_verts, uint32 version_id)
{
  // Make sure our arrays have the correct size.
  UpdateArrays (num_verts);

  // Perform all running scripts.
  bool mod = UpdateAnimation (current);

  // If the input arrays changed we need to update animated vertex arrays
  // anyway.
  if (last_version_id != version_id)
  {
    mod = true;
    last_version_id = version_id;
  }

  // Update the animated vertices now.
  if (mod)
  {
    const csPDelArray<csAnimControlGroup>& groups = factory->GetGroups ();
    const csArray<csArray<ac_group_data> >& groups_per_vertex = factory
    	->GetGroupsPerVertexMapping ();
    size_t i;
    for (i = 0 ; i < (size_t)num_verts ; i++)
    {
      if (i >= groups_per_vertex.Length ())
        animated_verts[i] = verts[i];
      else
      {
        const csArray<ac_group_data>& vtgr = groups_per_vertex[i];
	if (vtgr.Length () == 0)
	{
	  animated_verts[i] = verts[i];
	}
        else if (vtgr.Length () == 1)
	{
	  csReversibleTransform& transform = groups[vtgr[0].idx]
	  	->GetTransform ();
	  animated_verts[i] = transform.Other2This (verts[i]);
	}
	else
	{
	  csReversibleTransform& transform = groups[vtgr[0].idx]
	  	->GetTransform ();
	  float total_weight = vtgr[0].weight;
	  csVector3 orig = vtgr[0].weight * transform.Other2This (verts[i]);
	  size_t j;
	  for (j = 1 ; j < vtgr.Length () ; j++)
	  {
	    csReversibleTransform& transform2 = groups[vtgr[j].idx]
	    	->GetTransform ();
	    total_weight += vtgr[j].weight;
	    orig += vtgr[j].weight * transform2.Other2This (verts[i]);
	  }
	  orig /= total_weight;
	}
      }
    }
  }


#if 0
  memcpy (animated_verts, verts, num_animated_verts * sizeof (csVector3));
  size_t i;
  for (i = 0 ; i < (size_t)num_animated_verts ; i++)
  {
    animated_verts[i].x += (float (rand () % 100) / 100.0)/5.0 -.1;
    animated_verts[i].y += (float (rand () % 100) / 100.0)/5.0 -.1;
    animated_verts[i].z += (float (rand () % 100) / 100.0)/5.0 -.1;
  }
#endif

  return animated_verts;
}

const csVector2* csGenmeshAnimationControl::UpdateTexels (csTicks current,
	const csVector2* texels, int num_texels, uint32 version_id)
{
  //UpdateAnimation (current);
  return texels;
}

const csVector3* csGenmeshAnimationControl::UpdateNormals (csTicks current,
	const csVector3* normals, int num_normals, uint32 version_id)
{
  //UpdateAnimation (current);
  return normals;
}

const csColor* csGenmeshAnimationControl::UpdateColors (csTicks current,
	const csColor* colors, int num_colors, uint32 version_id)
{
  //UpdateAnimation (current);
  return colors;
}

bool csGenmeshAnimationControl::Execute (const char* scriptname)
{
  csAnimControlScript* script = factory->FindScript (scriptname);
  if (!script) return false;
  csAnimControlRunnable* runnable = new csAnimControlRunnable (script);
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
  init_token_table (xmltokens);

  autorun_script = 0;
}

csGenmeshAnimationControlFactory::~csGenmeshAnimationControlFactory ()
{
  delete[] autorun_script;
  SCF_DESTRUCT_IBASE ();
}

csPtr<iGenMeshAnimationControl> csGenmeshAnimationControlFactory::
	CreateAnimationControl ()
{
  csGenmeshAnimationControl* ctrl = new csGenmeshAnimationControl (this);
  if (autorun_script)
    ctrl->Execute (autorun_script);
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

void csGenmeshAnimationControlFactory::UpdateGroupsPerVertexMapping ()
{
  size_t i;
  for (i = 0 ; i < groups.Length () ; i++)
  {
    csAnimControlGroup* g = groups[i];
    const csArray<ac_vertex_data>& vtdata = g->GetVertexData ();
    size_t j;
    for (j = 0 ; j < vtdata.Length () ; j++)
    {
      csArray<ac_group_data>& vertices = groups_per_vertex
      	.GetExtend (vtdata[j].idx);
      ac_group_data gd;
      gd.idx = i;
      gd.weight = vtdata[j].weight;
      vertices.Push (gd);	// Push group index.
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
  return ~0;
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
      case XMLTOKEN_VERTEX:
        {
	  int idx = child->GetAttributeValueAsInt ("idx");
	  float weight = child->GetAttributeValueAsFloat ("weight");
	  group->AddVertex (idx, weight);
	}
        break;
      case XMLTOKEN_GROUP:
        {
	  const char* err = ParseGroup (child, group);
	  if (err != 0) return err;
	}
        break;
      default:
        sprintf (error_buf,
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
	  autorun_script = csStrNew (scriptname);
	}
	break;
      default:
        sprintf (error_buf,
		"Don't recognize token '%s' in anim control!",
		value);
        return error_buf;
    }
  }

  UpdateGroupsPerVertexMapping ();

  return 0;
}

const char* csGenmeshAnimationControlFactory::Save (iDocumentNode* parent)
{
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

