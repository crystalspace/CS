/*
    Copyright (C) 2004 by Hristo Hristov
                          Boyan Hristov

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
#include "csutil/util.h"
#include "iengine/movable.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivideo/shader/shader.h"
#include "csgeom/tri.h"
#include <iengine/scenenode.h>

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
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControlFactory)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csGenmeshSkelAnimationControl)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshAnimationControl)
  SCF_IMPLEMENTS_INTERFACE (iGenMeshSkeletonControlState)
SCF_IMPLEMENT_IBASE_END

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControl::csGenmeshSkelAnimationControl (
  csGenmeshSkelAnimationControlFactory* fact, iMeshObject *mesh, iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (0);
  csGenmeshSkelAnimationControl::object_reg = object_reg;
  mesh_obj = mesh;
  factory = fact;
  num_animated_verts = 0;
  animated_verts = 0;
  transformed_verts = 0;
  animated_colors = 0;
  animated_face_norms = 0;
  num_animated_face_norms = 0;
  animated_vert_norms = 0;
  num_animated_vert_norms = 0;
  animated_tangents = 0;
  animated_bitangents = 0;



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
  normals_mapped = false;
  tangents_mapped = false;
  bitangents_mapped = false;

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);

 
 bones_name = strings->Request ("bones");

 initialized = false;
 use_parent = fact->GetUseParent();
 used_bones = factory->GetUsedBones();

if (!use_parent)
 {
	 skeleton = factory->gr->CreateSkeleton(factory->GetSkeletonFactory(), "");
 }

}

csGenmeshSkelAnimationControl::~csGenmeshSkelAnimationControl ()
{
	delete[] animated_verts;
	delete[] animated_colors;
	delete[] animated_vert_norms;
	delete[] animated_tangents;
	delete[] animated_bitangents;
	SCF_DESTRUCT_IBASE ();
}

void csGenmeshSkelAnimationControl::Initialize ()
{
	if (!initialized)
	{
		 if (use_parent)
		 {
			 csRef<iMeshWrapper> parent_mesh = 0;
			 iSceneNode *parent_node = mesh_obj->GetMeshWrapper()->QuerySceneNode()->GetParent();
			 if (parent_node)
			 {
				parent_mesh = SCF_QUERY_INTERFACE(parent_node, iMeshWrapper);
			 }

			 if (parent_mesh)
			 {
				csRef<iGeneralMeshState> genmesh_state = 
					SCF_QUERY_INTERFACE(parent_mesh->GetMeshObject(), iGeneralMeshState);
				CS_ASSERT(genmesh_state);

				csRef<iGenMeshSkeletonControlState> par_skel_state = 
					SCF_QUERY_INTERFACE(genmesh_state->GetAnimationControl(),
						iGenMeshSkeletonControlState);
				CS_ASSERT(par_skel_state);
				skeleton = par_skel_state->GetSkeleton();
			 }
		 }
		initialized = true;
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
    animated_colors = new csColor4[num_verts];

	delete[]animated_vert_norms;
	animated_vert_norms = new csVector3[num_verts];

	delete[] animated_tangents;
	animated_tangents = new csVector3[num_verts];

	delete[] animated_bitangents;
	animated_bitangents = new csVector3[num_verts];

    last_version_id = (uint32)~0;
  }
}

void csGenmeshSkelAnimationControl::UpdateVertNormArrays (int num_norms)
{
}

const csVector3* csGenmeshSkelAnimationControl::UpdateVertices (csTicks current,
	const csVector3* verts, int num_verts, uint32 version_id)
{
	if (!mesh_obj->GetMeshWrapper())
	{
		return verts;
	}

	Initialize();

	csRef<csShaderVariable> _bones = mesh_obj->GetMeshWrapper()->GetSVContext ()->GetVariable (bones_name);
	if (_bones.IsValid())
	{
		for (size_t i=0; i< used_bones.Length(); ++i)
		{
			int bone_idx = used_bones[i];
			csReversibleTransform offset_tr = 
				skeleton->GetBone(bone_idx)->GetFactory()->GetFullTransform().GetInverse()*
			skeleton->GetBone(bone_idx)->GetFullTransform();

			csShaderVariable* boneQuat = _bones->GetArrayElement (i*2+0);
			csQuaternion quat = csQuaternion(offset_tr.GetO2T());
			boneQuat->SetValue(csVector4 (quat.x, quat.y, quat.z, quat.r));

			csShaderVariable *boneOffs = _bones->GetArrayElement (i*2+1);
			csVector3 offset_pos = offset_tr.GetOrigin();
			boneOffs->SetValue(csVector4(offset_pos.x, offset_pos.y, offset_pos.z, 0));
		}
	}
	else
	{
		_bones = csPtr<csShaderVariable> (new csShaderVariable(bones_name));
		_bones->SetType (csShaderVariable::ARRAY);

		_bones->SetArraySize (used_bones.Length()*2);

		for (size_t i=0; i< used_bones.Length(); ++i)
		{
			int bone_idx = used_bones[i];
			csReversibleTransform offset_tr = 
				skeleton->GetBone(bone_idx)->GetFactory()->GetFullTransform().GetInverse()*
			skeleton->GetBone(bone_idx)->GetFullTransform();

			csRef<csShaderVariable> boneQuat = csPtr<csShaderVariable> (new csShaderVariable(csInvalidStringID));
			_bones->SetArrayElement (i*2+0, boneQuat);
			csQuaternion quat;
			if (quat.x != 0 || quat.y != 0 || quat.z != 0 || quat.r != 0)
			{
				boneQuat->SetValue(csVector4 (quat.x, quat.y, quat.z, quat.r));
			}

			csRef<csShaderVariable> boneOffs = csPtr<csShaderVariable> (new csShaderVariable(csInvalidStringID));
			_bones->SetArrayElement (i*2+1, boneOffs);
			csVector3 offset_pos = offset_tr.GetOrigin();
			boneOffs->SetValue(csVector4(offset_pos.x, offset_pos.y, offset_pos.z, 0));
		}
		mesh_obj->GetMeshWrapper()->GetSVContext ()->AddVariable (_bones);
		vertices_mapped = true;
	}
	return verts;
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
	return normals;
}

const csColor4* csGenmeshSkelAnimationControl::UpdateColors (csTicks current,
  const csColor4* colors, int num_colors, uint32 version_id)
{
  return colors;
}

const csVector3* csGenmeshSkelAnimationControl::UpdateTangents (csTicks current, 
  const csVector3* tangents, int num_tangents, uint32 version_id)
{
	return tangents;
}

const csVector3* csGenmeshSkelAnimationControl::UpdateBiTangents (csTicks current, 
  const csVector3* bitangents, int num_bitangents, uint32 version_id)
{
	return bitangents;
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
  flags.SetAll(0);
  skeleton_factory = 0;
  use_parent = false;
}

csGenmeshSkelAnimationControlFactory::~csGenmeshSkelAnimationControlFactory ()
{
	SCF_DESTRUCT_IBASE ();
}

csPtr<iGenMeshAnimationControl> csGenmeshSkelAnimationControlFactory::
  CreateAnimationControl (iMeshObject *mesh)
{
  csGenmeshSkelAnimationControl* ctrl = new csGenmeshSkelAnimationControl (this, mesh, object_reg);

  size_t i;
  for (i = 0 ; i < autorun_scripts.Length () ; i++)
	ctrl->GetSkeleton()->Execute (autorun_scripts[i]);
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

const char* csGenmeshSkelAnimationControlFactory::Load (iDocumentNode* node)
{
  csRef<iPluginManager> plugin_mgr (
    CS_QUERY_REGISTRY (object_reg, iPluginManager));

  csRef<iLoaderPlugin> ldr_plg = CS_QUERY_PLUGIN_CLASS(plugin_mgr, 
    "crystalspace.graveyard.loader", iLoaderPlugin);
  
  if (!ldr_plg)
  {
    ldr_plg = CS_LOAD_PLUGIN(plugin_mgr, 
      "crystalspace.graveyard.loader", iLoaderPlugin);
    if (!ldr_plg )
    {
        printf("Missing <crystalspace.graveyard.loader> plugin!\n");
		return 0;
    }
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
      case XMLTOKEN_SKELFACT:
        {
			csRef<iDocumentNodeIterator> it = child->GetNodes ();
			while (it->HasNext ())
			{
				csRef<iDocumentNode> child = it->Next ();
				if (child->GetType () != CS_NODE_ELEMENT) continue;
				const char* value = child->GetValue ();
				csStringID id = xmltokens.Request (value);
				switch (id)
				{
					case XMLTOKEN_USE_PARENT:
					{
						use_parent = true;
					}
					break;
				}
			}
			if (!use_parent)
			{
				csRef<iBase> skf = ldr_plg->Parse(child, 0, 0, 0);
				skeleton_factory = SCF_QUERY_INTERFACE (skf, iSkeletonFactory);
				gr = skeleton_factory->GetGraveyard();
			}
        }
        break;
      case XMLTOKEN_SKELFILE:
        {
			csRef<iVFS> vfs = CS_QUERY_REGISTRY(object_reg, iVFS);
			csRef<iDataBuffer> buf (vfs->ReadFile(child->GetContentsValue()));
			if (buf || buf->GetSize())
			{
				csRef<iDocument> doc;

				const char* b = **buf;
				const char* error = 0;
				while (*b == ' ' || *b == '\n' || *b == '\t') b++;
				if (*b == '<')
				{
					csRef<iDocumentSystem> xml (CS_QUERY_REGISTRY (object_reg, iDocumentSystem));
					if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
					doc = xml->CreateDocument ();
					error = doc->Parse(buf);
				}
				if (!error)
				{
					csRef<iBase> skf = ldr_plg->Parse(doc->GetRoot(), 0, 0, 0);
					skeleton_factory = SCF_QUERY_INTERFACE (skf, iSkeletonFactory);
					gr = skeleton_factory->GetGraveyard();
				}
				else
				{
					return error;
				}
			}
        }
        break;
      case XMLTOKEN_USE_BONES:
        {
			csRef<iDocumentNodeIterator> it = child->GetNodes ();
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
						used_bones.Push(child->GetContentsValueAsInt());
					}
					break;
				}
			}
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

  return 0;
}

const char* csGenmeshSkelAnimationControlFactory::Save (iDocumentNode* parent)
{
  csRef<iFactory> plugin = SCF_QUERY_INTERFACE (type, iFactory);
  if (!plugin) return "Couldn't get Class ID";
  parent->SetAttribute ("plugin", plugin->QueryClassID ());
  return "Not implemented yet!";
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControlType::csGenmeshSkelAnimationControlType (
  iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

}

csGenmeshSkelAnimationControlType::~csGenmeshSkelAnimationControlType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGenmeshSkelAnimationControlType::Initialize (iObjectRegistry* object_reg)
{
  csGenmeshSkelAnimationControlType::object_reg = object_reg;
  return true;
}

csPtr<iGenMeshAnimationControlFactory> csGenmeshSkelAnimationControlType::
  CreateAnimationControlFactory ()
{
  csGenmeshSkelAnimationControlFactory* ctrl = new csGenmeshSkelAnimationControlFactory
     (this, object_reg);
  return csPtr<iGenMeshAnimationControlFactory> (ctrl);
}
