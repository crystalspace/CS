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
#include "csutil/util.h"
#include "csgeom/math3d.h"
#include "iutil/document.h"
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

//-------------------------------------------------------------------------

csAnimControlGroup::csAnimControlGroup (const char* name)
{
  csAnimControlGroup::name = csStrNew (name);
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

//-------------------------------------------------------------------------

csGenmeshAnimationControl::csGenmeshAnimationControl (
	csGenmeshAnimationControlFactory* fact)
{
  SCF_CONSTRUCT_IBASE (0);
  factory = fact;
  num_animated_verts = 0;
  animated_verts = 0;
}

csGenmeshAnimationControl::~csGenmeshAnimationControl ()
{
  delete[] animated_verts;

  SCF_DESTRUCT_IBASE ();
}

const csVector3* csGenmeshAnimationControl::UpdateVertices (csTicks current,
	const csVector3* verts, int num_verts)
{
  if (num_verts != num_animated_verts)
  {
    num_animated_verts = num_verts;
    animated_verts = new csVector3[num_verts];
  }
  memcpy (animated_verts, verts, num_animated_verts * sizeof (csVector3));
  size_t i;
  for (i = 0 ; i < (size_t)num_animated_verts ; i++)
  {
    animated_verts[i].x += (float (rand () % 100) / 100.0)/5.0 -.1;
    animated_verts[i].y += (float (rand () % 100) / 100.0)/5.0 -.1;
    animated_verts[i].z += (float (rand () % 100) / 100.0)/5.0 -.1;
  }
  return animated_verts;
}

const csVector2* csGenmeshAnimationControl::UpdateTexels (csTicks current,
	const csVector2* texels, int num_texels)
{
  return texels;
}

const csVector3* csGenmeshAnimationControl::UpdateNormals (csTicks current,
	const csVector3* normals, int num_normals)
{
  return normals;
}

const csColor* csGenmeshAnimationControl::UpdateColors (csTicks current,
	const csColor* colors, int num_colors)
{
  return colors;
}

//-------------------------------------------------------------------------

csGenmeshAnimationControlFactory::csGenmeshAnimationControlFactory (
	iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (0);
  csGenmeshAnimationControlFactory::object_reg = object_reg;
  init_token_table (xmltokens);
}

csGenmeshAnimationControlFactory::~csGenmeshAnimationControlFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

csPtr<iGenMeshAnimationControl> csGenmeshAnimationControlFactory::
	CreateAnimationControl ()
{
  csGenmeshAnimationControl* ctrl = new csGenmeshAnimationControl (this);
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

const char* csGenmeshAnimationControlFactory::ParseGroup (iDocumentNode* node)
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
      default:
        sprintf (error_buf,
		"Don't recognize token '%s' in anim control group!",
		value);
	delete group;
        return error_buf;
    }
  }
  groups.Push (group);
  return 0;
}

const char* csGenmeshAnimationControlFactory::ParseScript (iDocumentNode* node)
{
  const char* scriptname = node->GetAttributeValue ("name");
  if (!scriptname)
    return "Name of the script is missing!";

  csAnimControlScript* script = new csAnimControlScript (scriptname);

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
        break;
      case XMLTOKEN_DELAY:
        break;
      case XMLTOKEN_REPEAT:
        break;
      default:
        sprintf (error_buf,
		"Don't recognize token '%s' in anim control script!",
		value);
	delete script;
        return error_buf;
    }
  }
  scripts.Push (script);
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
	  const char* err = ParseGroup (child);
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
	break;
      default:
        sprintf (error_buf,
		"Don't recognize token '%s' in anim control!",
		value);
        return error_buf;
    }
  }
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
}

csGenmeshAnimationControlType::~csGenmeshAnimationControlType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshAnimationControlType::Initialize (iObjectRegistry* object_reg)
{
  csGenmeshAnimationControlType::object_reg = object_reg;
  return true;
}

csPtr<iGenMeshAnimationControlFactory> csGenmeshAnimationControlType::
	CreateAnimationControlFactory ()
{
  csGenmeshAnimationControlFactory* ctrl = new csGenmeshAnimationControlFactory
  	(object_reg);
  return csPtr<iGenMeshAnimationControlFactory> (ctrl);
}

