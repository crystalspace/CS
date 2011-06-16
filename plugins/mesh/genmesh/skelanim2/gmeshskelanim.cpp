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

SCF_IMPLEMENT_FACTORY (csGenmeshSkelAnimationControlType)

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControl::csGenmeshSkelAnimationControl (
  csGenmeshSkelAnimationControlFactory* fact, iMeshObject *mesh,
  iObjectRegistry* object_reg) :
  scfImplementationType(this)
{
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

  csRef<iShaderVarStringSet> strings =
    csQueryRegistryTagInterface<iShaderVarStringSet> (object_reg,
      "crystalspace.shader.variablenameset");


  bones_name = strings->Request ("bones");

  initialized = false;
  use_parent = fact->GetUseParent();
  used_bones = factory->GetUsedBones();

  if (!use_parent)
  {
    skeleton = factory->GetSkeletonGraveyard()->CreateSkeleton(factory->GetSkeletonFactory(), "");
  }

}

csGenmeshSkelAnimationControl::~csGenmeshSkelAnimationControl ()
{
  if (skeleton && factory && factory->GetSkeletonGraveyard())
  {
    factory->GetSkeletonGraveyard()->RemoveSkeleton (skeleton);
  }
  delete[] animated_verts;
  delete[] animated_colors;
  delete[] animated_vert_norms;
  delete[] animated_tangents;
  delete[] animated_bitangents;
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
        parent_mesh = scfQueryInterface<iMeshWrapper> (parent_node);
      }

      if (parent_mesh)
      {
        csRef<iGeneralMeshState> genmesh_state = 
          scfQueryInterface<iGeneralMeshState> (parent_mesh->GetMeshObject());
        CS_ASSERT(genmesh_state);

        csRef<iGenMeshSkeletonControlState> par_skel_state = 
          
          scfQueryInterface<iGenMeshSkeletonControlState> (genmesh_state->GetAnimationControl());
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

void csGenmeshSkelAnimationControl::UpdateVertNormArrays (int /*num_norms*/)
{
}

void csGenmeshSkelAnimationControl::Update (csTicks current, int, uint32)
{
  if (last_update_time != current)
  {
     last_update_time = current;
  }
  else
  {
    return;
  }

  if (!mesh_obj->GetMeshWrapper() || !skeleton)
  {
    return;
  }

  Initialize();

  csRef<csShaderVariable> _bones = mesh_obj->GetMeshWrapper()->GetSVContext ()->GetVariable (bones_name);
  if (_bones.IsValid())
  {
    for (size_t i=0; i< used_bones.GetSize (); ++i)
    {
      int bone_idx = used_bones[i];
      csReversibleTransform offset_tr = 
        skeleton->GetBone(bone_idx)->GetFactory()->GetFullTransform().GetInverse()*
        skeleton->GetBone(bone_idx)->GetFullTransform();

      csShaderVariable* boneQuat = _bones->GetArrayElement (i*2+0);
      csQuaternion quat; quat.SetMatrix(offset_tr.GetT2O());
      boneQuat->SetValue(csVector4 (quat.v.x, quat.v.y, quat.v.z, quat.w));

      csShaderVariable *boneOffs = _bones->GetArrayElement (i*2+1);
      csVector3 offset_pos = offset_tr.GetOrigin();
      boneOffs->SetValue(csVector4(offset_pos.x, offset_pos.y, offset_pos.z, 0));
    }
  }
  else
  {
    csRef<csShaderVariable> _bones;
    _bones.AttachNew(new csShaderVariable(bones_name));
    _bones->SetType (csShaderVariable::ARRAY);

    _bones->SetArraySize (used_bones.GetSize ()*2);

    for (size_t i=0; i< used_bones.GetSize (); ++i)
    {
      int bone_idx = used_bones[i];
      csReversibleTransform offset_tr = 
        skeleton->GetBone(bone_idx)->GetFactory()->GetFullTransform().GetInverse()*
        skeleton->GetBone(bone_idx)->GetFullTransform();

      csRef<csShaderVariable> boneQuat;
      boneQuat.AttachNew(new csShaderVariable(CS::InvalidShaderVarStringID));
      _bones->SetArrayElement (i*2+0, boneQuat);
      boneQuat->SetValue(csVector4 (0, 0, 0, 1));

      csRef<csShaderVariable> boneOffs;
       boneOffs.AttachNew(new csShaderVariable(CS::InvalidShaderVarStringID));
      _bones->SetArrayElement (i*2+1, boneOffs);
      csVector3 offset_pos = offset_tr.GetOrigin();
      boneOffs->SetValue(csVector4(offset_pos.x, offset_pos.y, offset_pos.z, 0));
    }
    mesh_obj->GetMeshWrapper()->GetSVContext ()->AddVariable (_bones);
    vertices_mapped = true;
  }
}

const csVector3* csGenmeshSkelAnimationControl::UpdateVertices (csTicks current,
  const csVector3* verts, int /*num_verts*/, uint32 /*version_id*/)
{
  return verts;
}

const csVector2* csGenmeshSkelAnimationControl::UpdateTexels (csTicks,
  const csVector2* texels, int /*num_texels*/, uint32 /*version_id*/)
{
  return texels;
}

const csVector3* csGenmeshSkelAnimationControl::UpdateNormals (csTicks,
  const csVector3* normals, int /*num_normals*/, uint32 /*version_id*/)
{
  return normals;
}

const csColor4* csGenmeshSkelAnimationControl::UpdateColors (csTicks,
  const csColor4* colors, int /*num_colors*/, uint32 /*version_id*/)
{
  return colors;
}

const csVector3* csGenmeshSkelAnimationControl::UpdateTangents (csTicks,
  const csVector3* tangents, int /*num_tangents*/, uint32 /*version_id*/)
{
  return tangents;
}

const csVector3* csGenmeshSkelAnimationControl::UpdateBiTangents (csTicks,
  const csVector3* bitangents, int /*num_bitangents*/, uint32 /*version_id*/)
{
  return bitangents;
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControlFactory::csGenmeshSkelAnimationControlFactory (
  csGenmeshSkelAnimationControlType* type, iObjectRegistry* object_reg) :
  scfImplementationType(this, type)
{
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
  the_graveyard = 0;
}

csGenmeshSkelAnimationControlFactory::~csGenmeshSkelAnimationControlFactory ()
{
}

csPtr<iGenMeshAnimationControl> csGenmeshSkelAnimationControlFactory::
CreateAnimationControl (iMeshObject *mesh)
{
  csGenmeshSkelAnimationControl* ctrl = new csGenmeshSkelAnimationControl (this, mesh, object_reg);

  size_t i;
  for (i = 0 ; i < autorun_scripts.GetSize () ; i++)
    ctrl->GetSkeleton()->Execute (autorun_scripts[i]);
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

const char* csGenmeshSkelAnimationControlFactory::Load (iDocumentNode* node)
{
  csRef<iPluginManager> plugin_mgr (
    csQueryRegistry<iPluginManager> (object_reg));

  csRef<iLoaderPlugin> ldr_plg = CS_QUERY_PLUGIN_CLASS(plugin_mgr, 
    "crystalspace.graveyard.loader", iLoaderPlugin);

  if (!ldr_plg)
  {
    ldr_plg = csLoadPlugin<iLoaderPlugin> (plugin_mgr, 
      "crystalspace.graveyard.loader");
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
          skeleton_factory = scfQueryInterface<iSkeletonFactory> (skf);
          the_graveyard = skeleton_factory->GetGraveyard();
        }
      }
      break;
    case XMLTOKEN_SKELFILE:
      {
        csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
	const char* filename = child->GetContentsValue ();
        csRef<iDataBuffer> buf (vfs->ReadFile(filename));
        if (buf && buf->GetSize())
        {
          csRef<iDocument> doc;

          const char* b = **buf;
          const char* error = 0;
          while (*b == ' ' || *b == '\n' || *b == '\t') b++;
          if (*b == '<')
          {
            csRef<iDocumentSystem> xml (csQueryRegistry<iDocumentSystem> (object_reg));
            if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
            doc = xml->CreateDocument ();
            error = doc->Parse(buf);
          }
          if (!error)
          {
            csRef<iBase> skf = ldr_plg->Parse(doc->GetRoot(), 0, 0, 0);
            skeleton_factory = scfQueryInterface<iSkeletonFactory> (skf);
            the_graveyard = skeleton_factory->GetGraveyard();
          }
          else
          {
            return error;
          }
        }
	else
	{
          error_buf.Format (
            "Can't load skeleton file %s!", CS::Quote::Single (filename));
          return error_buf;
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
        "Don't recognize token %s in anim control!",
        CS::Quote::Single (value));
      return error_buf;
    }
  }

  return 0;
}

const char* csGenmeshSkelAnimationControlFactory::Save (iDocumentNode* parent)
{
  csRef<iFactory> plugin = scfQueryInterface<iFactory> (type);
  if (!plugin) return "Couldn't get Class ID";
  parent->SetAttribute ("plugin", plugin->QueryClassID ());
  return "Not implemented yet!";
}

//-------------------------------------------------------------------------

csGenmeshSkelAnimationControlType::csGenmeshSkelAnimationControlType (
  iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csGenmeshSkelAnimationControlType::~csGenmeshSkelAnimationControlType ()
{
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
