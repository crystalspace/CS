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

SCF_IMPLEMENT_IBASE (csGenmeshAnimationControlFactory)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControlFactory)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGenmeshAnimationControlFactory::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGenmeshAnimationControlFactory)

SCF_IMPLEMENT_IBASE (csGenmeshAnimationControl)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControl)
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
}

csGenmeshAnimationControl::~csGenmeshAnimationControl ()
{
  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshAnimationControl::UpdateVertices (csTicks current,
	csVector3* verts, int num_verts, csBox3& bbox)
{
  return false;
}

bool csGenmeshAnimationControl::UpdateTexels (csTicks current,
	csVector2* texels, int num_texels)
{
  return false;
}

bool csGenmeshAnimationControl::UpdateNormals (csTicks current,
	csVector3* normals, int num_normals)
{
  return false;
}

bool csGenmeshAnimationControl::UpdateColors (csTicks current, csColor* colors,
  	int num_colors)
{
  return false;
}

const char* csGenmeshAnimationControl::ParseGroup (iDocumentNode* node)
{
  const char* groupname = node->GetAttributeValue ("name");
  if (!groupname)
    return "Name of the group is missing!";
  csAnimControlGroup* group = new csAnimControlGroup (groupname);

  csStringHash& xmltokens = factory->xmltokens;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case csGenmeshAnimationControlFactory::XMLTOKEN_VERTEX:
        {
	  int idx = child->GetAttributeValueAsInt ("idx");
	  float weight = child->GetAttributeValueAsFloat ("weight");
	  group->AddVertex (idx, weight);
	}
        break;
      default:
        sprintf (factory->error_buf,
		"Don't recognize token '%s' in anim control group!",
		value);
	delete group;
        return factory->error_buf;
    }
  }
  groups.Push (group);
  return 0;
}

const char* csGenmeshAnimationControl::ParseScript (iDocumentNode* node)
{
  const char* scriptname = node->GetAttributeValue ("name");
  if (!scriptname)
    return "Name of the script is missing!";

  csAnimControlScript* script = new csAnimControlScript (scriptname);

  csStringHash& xmltokens = factory->xmltokens;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case csGenmeshAnimationControlFactory::XMLTOKEN_MOVE:
        break;
      case csGenmeshAnimationControlFactory::XMLTOKEN_DELAY:
        break;
      case csGenmeshAnimationControlFactory::XMLTOKEN_REPEAT:
        break;
      default:
        sprintf (factory->error_buf,
		"Don't recognize token '%s' in anim control script!",
		value);
	delete script;
        return factory->error_buf;
    }
  }
  scripts.Push (script);
  return 0;
}

const char* csGenmeshAnimationControl::Load (iDocumentNode* node)
{
  csStringHash& xmltokens = factory->xmltokens;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case csGenmeshAnimationControlFactory::XMLTOKEN_GROUP:
        {
	  const char* err = ParseGroup (child);
	  if (err) return err;
	}
        break;
      case csGenmeshAnimationControlFactory::XMLTOKEN_SCRIPT:
        {
	  const char* err = ParseScript (child);
	  if (err) return err;
	}
        break;
      case csGenmeshAnimationControlFactory::XMLTOKEN_RUN:
	break;
      default:
        sprintf (factory->error_buf,
		"Don't recognize token '%s' in anim control!",
		value);
        return factory->error_buf;
    }
  }
  return 0;
}

const char* csGenmeshAnimationControl::Save (iDocumentNode* parent)
{
  return "Not implemented yet!";
}

//-------------------------------------------------------------------------

csGenmeshAnimationControlFactory::csGenmeshAnimationControlFactory (
	iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  init_token_table (xmltokens);
}

csGenmeshAnimationControlFactory::~csGenmeshAnimationControlFactory ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshAnimationControlFactory::Initialize (iObjectRegistry* object_reg)
{
  csGenmeshAnimationControlFactory::object_reg = object_reg;
  return true;
}

csPtr<iGenMeshAnimationControl> csGenmeshAnimationControlFactory::
	CreateAnimationControl ()
{
  csGenmeshAnimationControl* ctrl = new csGenmeshAnimationControl (this);
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

