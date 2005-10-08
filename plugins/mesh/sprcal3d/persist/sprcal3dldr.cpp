/*
Copyright (C) 2003 by Keith Fulton

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
#include "csutil/sysfunc.h"
#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/quaternion.h"
#include "csgeom/transfrm.h"
#include "csutil/scanstr.h"
#include "csutil/util.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/spritecal3d.h"
#include "ivideo/graph3d.h"
#include "csqint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "imap/ldrctxt.h"
#include "sprcal3dldr.h"

// Hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#include <cal3d/loader.h>

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_PATH,
  XMLTOKEN_SCALE,
  XMLTOKEN_SKELETON,
  XMLTOKEN_ANIMATION,
  XMLTOKEN_MESH,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MORPHTARGET,
  XMLTOKEN_MORPHANIMATION,
  XMLTOKEN_OPTIONS,
  XMLTOKEN_HARDTRANSFORM,
  XMLTOKEN_SOCKET,
  XMLTOKEN_FACTORY,
  XMLTOKEN_ANIMCYCLE,
  XMLTOKEN_IDLEANIM,
  XMLTOKEN_IDLE
};

SCF_IMPLEMENT_IBASE (csSpriteCal3DFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpriteCal3DFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpriteCal3DLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpriteCal3DSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpriteCal3DSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSpriteCal3DFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSpriteCal3DFactorySaver)
SCF_IMPLEMENT_FACTORY (csSpriteCal3DLoader)
SCF_IMPLEMENT_FACTORY (csSpriteCal3DSaver)


csSpriteCal3DFactoryLoader::csSpriteCal3DFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpriteCal3DFactoryLoader::~csSpriteCal3DFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpriteCal3DFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSpriteCal3DFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  vfs    = CS_QUERY_REGISTRY (object_reg, iVFS);

  xmltokens.Register ("path",           XMLTOKEN_PATH);
  xmltokens.Register ("scale",          XMLTOKEN_SCALE);
  xmltokens.Register ("skeleton",       XMLTOKEN_SKELETON);
  xmltokens.Register ("animation",      XMLTOKEN_ANIMATION);
  xmltokens.Register ("mesh",           XMLTOKEN_MESH);
  xmltokens.Register ("material",       XMLTOKEN_MATERIAL);
  xmltokens.Register ("morphtarget",    XMLTOKEN_MORPHTARGET);
  xmltokens.Register ("morphanimation", XMLTOKEN_MORPHANIMATION);
  xmltokens.Register ("options",        XMLTOKEN_OPTIONS);
  xmltokens.Register ("hardtransform",  XMLTOKEN_HARDTRANSFORM);
  xmltokens.Register ("socket",         XMLTOKEN_SOCKET);
  return true;
}


csPtr<iBase> csSpriteCal3DFactoryLoader::Parse (iDocumentNode* node,
						iStreamSource*,
						iLoaderContext* ldr_context, 
						iBase* context)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
    iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
    "crystalspace.mesh.object.sprite.cal3d", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.sprite.cal3d",
      iMeshObjectType);
  }
  if (!type)
  {
    synldr->ReportError (
      "crystalspace.spritecal3dfactoryloader.setup.objecttype",
      node, "Could not load the sprite.cal3d mesh object plugin!");
    return 0;
  }

  // sprcal3d is absolutely predicated upon having skeleton and misbehaves (or
  // crashes) if the skeleton is missing, thus we must take care to only
  // consider the mesh valid if the skeleton is present.
  bool skel_present = false;

  // @@@ Temporary fix to allow to set actions for objects loaded
  // with impexp. Once those loaders move to another plugin this code
  // below should be removed.
  csRef<iMeshObjectFactory> fact;
  if (context)
    fact = SCF_QUERY_INTERFACE (context, iMeshObjectFactory);
  // If there was no factory we create a new one.
  if (!fact)
    fact = type->NewFactory ();

  csRef<iSpriteCal3DFactoryState> newspr (
    SCF_QUERY_INTERFACE (fact, iSpriteCal3DFactoryState));

  if (!newspr->Create("dummy"))
  {
    newspr->ReportLastError();
    return 0; // failed
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  float scale = 0.0;

  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_OPTIONS:
      {
          bool rotate = child->GetAttributeValueAsBool("rotate_x_axis");
          bool invert = child->GetAttributeValueAsBool("flip_textures");
          newspr->SetLoadFlags( rotate?LOADER_ROTATE_X_AXIS:0
	      | invert?LOADER_INVERT_V_COORD:0 );
          break;
      }
    case XMLTOKEN_PATH:
      {
	const char *path = child->GetAttributeValue("dir");
	if (path)
	  newspr->SetBasePath(path);
	else
	{
	  synldr->ReportError (
	    "crystalspace.spritecal3dfactoryloader.parse.badpath", child,
	    "dir is a required attribute of <path> token in cal3d files.");
	  return 0;
	}
	break;
      }
    case XMLTOKEN_SCALE:
      {
	scale = child->GetAttributeValueAsFloat("value");
	if (!scale)
	{
	  synldr->ReportError (
	    "crystalspace.spritecal3dfactoryloader.parse.badvalue", child,
	    "value is a required attribute of <scale> token in cal3d files.");
	  return 0;
	}
	break;
      }
    case XMLTOKEN_HARDTRANSFORM:
      {
	float ax,ay,az,angle;
	csVector3 translation;
	ax = child->GetAttributeValueAsFloat("rot_axis_x");
	ay = child->GetAttributeValueAsFloat("rot_axis_y");
	az = child->GetAttributeValueAsFloat("rot_axis_z");
	angle = child->GetAttributeValueAsFloat("rot_angle");
	translation.x = child->GetAttributeValueAsFloat("delta_x");
	translation.y = child->GetAttributeValueAsFloat("delta_y");
	translation.z = child->GetAttributeValueAsFloat("delta_z");

	csMatrix3 rotation(ax,ay,az,angle*TWO_PI/360);
	csReversibleTransform rt(rotation,translation);
	fact->HardTransform(rt);
	break;
      }
    case XMLTOKEN_SKELETON:
      {
	const char *file = child->GetAttributeValue("file");
	if (file)
	{
	  if (!newspr->LoadCoreSkeleton(vfs,file))
	  {
	    synldr->ReportError (
  	      "crystalspace.spritecal3dfactoryloader.parse.badfile",
	      child,"Could not load cal3d skeleton file <%s>.",file);

	    newspr->ReportLastError();
	    return 0;
	  }
	}
	else
	{
	  synldr->ReportError (
	    "crystalspace.spritecal3dfactoryloader.parse.badfile", child,
	    "file is a required attribute of <skeleton> token in cal3d "
	    "files.");
	  return 0;
	}
	// We've loaded the skeleton
	skel_present = true;
	break;
      }
    case XMLTOKEN_ANIMATION:
      {
	const char *file = child->GetAttributeValue("file");
	const char *name = child->GetAttributeValue("name");
	if (!name)
	{
	  synldr->ReportError (
	    "crystalspace.spritecal3dfactoryloader.parse.badfile", child,
	    "name is a required attribute of <animation> token in cal3d "
	    "files.");
	  return 0;
	}
	int type;
        csString ctype = child->GetAttributeValue("type");
        if (ctype=="idle")
          type = iSpriteCal3DState::C3D_ANIM_TYPE_IDLE;
        else if (ctype == "travel")
          type = iSpriteCal3DState::C3D_ANIM_TYPE_TRAVEL;
        else if (ctype == "cycle")
          type = iSpriteCal3DState::C3D_ANIM_TYPE_CYCLE;
        else if (ctype == "style_cycle")
          type = iSpriteCal3DState::C3D_ANIM_TYPE_STYLE_CYCLE;
        else if (ctype == "action")
          type = iSpriteCal3DState::C3D_ANIM_TYPE_ACTION;
        else
          type = iSpriteCal3DState::C3D_ANIM_TYPE_NONE;

        float base_vel = child->GetAttributeValueAsInt("base_vel");
	float min_vel = child->GetAttributeValueAsFloat("min_vel");
	float max_vel = child->GetAttributeValueAsFloat("max_vel");
	int  max_interval = child->GetAttributeValueAsInt("max_random");
	int  min_interval = child->GetAttributeValueAsInt("min_random");
	int  idle_pct     = child->GetAttributeValueAsInt("idle_pct");
        bool lock         = child->GetAttributeValueAsBool("lock");
	if (file)
	{
	  int animID = newspr->LoadCoreAnimation(vfs,file,
	    name,
	    type,
	    base_vel,
	    min_vel,
	    max_vel,
            min_interval,
            max_interval,
            idle_pct, lock);

	  if (animID == -1)
	  {
            synldr->ReportError (
  	      "crystalspace.spritecal3dfactoryloader.parse.badfile",
	      child,"Could not load cal3d anim file <%s>.",file);

            newspr->ReportLastError();
	    return 0;
	  }
	}
	else
	{
	  synldr->ReportError (
	    "crystalspace.spritecal3dfactoryloader.parse.badfile", child,
	    "file is a required attribute of <animation> token in cal3d "
	    "files.");
	  return 0;
	}
	break;
      }
    case XMLTOKEN_MESH:
      {
	const char *file = child->GetAttributeValue("file");
	const char *name = child->GetAttributeValue("name");
	bool      attach;
	const char *a = child->GetAttributeValue("attach");
	if (a && *a == 'n')
	  attach = false;
	else
	  attach = true;
	const char *def_matl = child->GetAttributeValue("material");

	if (file)
	{
	  iMaterialWrapper *mat=0;
	  if (def_matl)
	  {
	    mat = LoadMaterialTag(newspr,child,ldr_context,def_matl,def_matl);
	  }
	  int mesh_index = newspr->LoadCoreMesh(vfs,file,name,attach,mat);
          if (mesh_index == -1)
	  {
	      synldr->ReportError (
	        "crystalspace.spritecal3dfactoryloader.parse.badfile",
	        child,"Could not load mesh file <%s>.",file);
	        newspr->ReportLastError();
	        return 0;
	  }

          csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
          while (child_it->HasNext ())
          {
            csRef<iDocumentNode> childchild = child_it->Next ();
            if (childchild->GetType () != CS_NODE_ELEMENT) continue;
            const char* child_value = childchild->GetValue ();
            csStringID child_id = xmltokens.Request (child_value);
            switch (child_id)
            {
              case XMLTOKEN_MORPHTARGET:
              {
                const char *morph_file = childchild->GetAttributeValue("file");
                const char *morph_name = childchild->GetAttributeValue("name");
                if (morph_file)
                {
                  int morph_index = newspr->LoadCoreMorphTarget(
		      vfs,mesh_index,morph_file,morph_name);
                  if (morph_index == -1)
                  {
                    newspr->ReportLastError();
                    return 0;
                  }
                }
              }
            }
          }
	}
	else
	{
	  synldr->ReportError (
	    "crystalspace.spritecal3dfactoryloader.parse.badfile", child,
	    "file is a required attribute of <mesh> token in cal3d files.");
	  return 0;
	}
	break;
      }
    case XMLTOKEN_MORPHANIMATION:
      {
        const char *name = child->GetAttributeValue("name");
        int morphanimationid = newspr->AddMorphAnimation(name);
        csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
        while (child_it->HasNext ())
        {
          csRef<iDocumentNode> childchild = child_it->Next ();
          if (childchild->GetType () != CS_NODE_ELEMENT) continue;
          const char* child_value = childchild->GetValue ();
          csStringID child_id = xmltokens.Request (child_value);
          switch (child_id)
          {
            case XMLTOKEN_MORPHTARGET:
            {
              const char *mesh_name = childchild->GetAttributeValue("mesh");
              const char *morph_name =
		childchild->GetAttributeValue("morphtarget");
              newspr->AddMorphTarget(morphanimationid,mesh_name,morph_name);
            }
          }
        }
        break;
      }
    case XMLTOKEN_MATERIAL:
      {
#ifdef CS_DEBUG
	static bool styleReminder = true;
	if (styleReminder)
	{
	  synldr->Report ("crystalspace.mesh.loader.factory.sprite.cal3d", 
	    CS_REPORTER_SEVERITY_NOTIFY, child, 
	    "Material definitions in meshes is not considered good style, "
	    "if you need to define a mesh and material at the same time use "
	    "libraries.");
	  styleReminder = false;
	}
#endif
        const char *file = child->GetAttributeValue("file");
        const char *matName = child->GetAttributeValue("name");
        if (!LoadMaterialTag(newspr,child,ldr_context,file, matName))
          return 0;
        break;
      }
    case XMLTOKEN_SOCKET:
      {
        int a = child->GetAttributeValueAsInt ("tri");
        int submesh = child->GetAttributeValueAsInt ("submesh");
        int mesh = child->GetAttributeValueAsInt ("mesh");
        iSpriteCal3DSocket* sprite_socket = newspr->AddSocket ();
        sprite_socket->SetName (child->GetAttributeValue ("name"));
        sprite_socket->SetTriangleIndex (a);
        sprite_socket->SetSubmeshIndex (submesh);
        sprite_socket->SetMeshIndex (mesh);
        break;
      }
    default:
      synldr->ReportBadToken (child);
      return 0;
    }
  }
	
  // If we haven't loaded the skeleton, report error and abort load.
  if (!skel_present)
  {
    synldr->ReportError(
      "crystalspace.spritecal3dfactoryloader.parse.badfile",
      node, "No <skeleton> token found in cal3d file.");
    return 0;
  }
    
  if (scale)
    newspr->RescaleFactory(scale);  // this calls the function below itself
  else
    newspr->CalculateAllBoneBoundingBoxes();

  // Wrapup cal3d initialization
  newspr->BindMaterials();

  return csPtr<iBase> (fact);
}

iMaterialWrapper *csSpriteCal3DFactoryLoader::LoadMaterialTag(
	iSpriteCal3DFactoryState *newspr,
	iDocumentNode* child,
	iLoaderContext* ldr_context,
	const char *file, 
	const char* name)
{
  iMaterialWrapper* mat=0;
  if (file)
  {
    if (!name)
      name = file;
    else if (!file)
      file = name;
    mat = ldr_context->FindNamedMaterial (name, file);
    if (!mat)
    {
      synldr->ReportError (
	"crystalspace.spritecal3dfactoryloader.parse.unknownmaterial",
	child, "Couldn't find material named '%s'", name);
      return 0;
    }
        
    newspr->AddCoreMaterial(mat);
  }
  else
  {
    synldr->ReportError (
      "crystalspace.spritecal3dfactoryloader.parse.badfile", child,
      "file is a required attribute of <material> token in cal3d files.");
    return 0;
  }
  return mat;
}


//---------------------------------------------------------------------------

csSpriteCal3DFactorySaver::csSpriteCal3DFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpriteCal3DFactorySaver::~csSpriteCal3DFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpriteCal3DFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSpriteCal3DFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

//TBD
bool csSpriteCal3DFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent || !obj)
    return false;
 
  csRef<iDocumentNode> paramsNode =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for cal3d mesh");
 
  csRef<iSpriteCal3DFactoryState> cal3dfact = 
    SCF_QUERY_INTERFACE (obj, iSpriteCal3DFactoryState);
  csRef<iMeshObjectFactory> meshfact =
    SCF_QUERY_INTERFACE (obj, iMeshObjectFactory);
#if 0
  if ( cal3dfact && meshfact )
  {
    //Write Option Tag
    int flags = cal3dfact->GetLoadFlags();
    bool rotate = flags & LOADER_ROTATE_X_AXIS;
    bool invert = flags & LOADER_INVERT_V_COORD;
    csRef<iDocumentNode> optionNode =
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    optionNode->SetValue("socket");
    optionNode->SetAttribute("rotate_x_axis", (rotate)?"yes":"no");
    optionNode->SetAttribute("flip_textures", (invert)?"yes":"no");
 
    //Write Path Tag
    const char* dir = cal3dfact->GetBasePath();
    csRef<iDocumentNode> pathNode =
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    pathNode->SetValue("path");
    pathNode->SetAttribute("dir", dir);
 
    //Write Scale Tag
    float scale = cal3dfact->GetScale();
    csRef<iDocumentNode> scaleNode =
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    scaleNode->SetValue("scale");
    scaleNode->SetAttributeAsFloat("value", scale);
 
    //Write Skeleton Tag
    const char* skeleton = cal3dfact->GetCoreSkeleton()->GetFileName();
    csRef<iDocumentNode> pathNode =
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    pathNode->SetValue("skeleton");
    pathNode->SetAttribute("file", skeleton);
 
    //Write Material Tag
    const char* file = cal3dfact->Get...
    const char* matName = cal3dfact->Get...
    csRef<iDocumentNode> pathNode =
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    pathNode->SetValue("material");
    child->SetAttribute("file", file );
    child->SetAttribute("name", matName);
 
    //Write Mesh Tags
    for (int i=0; i<cal3dfact->GetMeshCount(); i++)
    {
      csRef<iDocumentNode> meshNode =
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      meshNode->SetValue("mesh");
 
      const char* meshname = cal3dfact->GetMeshName(i);
      meshNode->SetAttribute("name", meshname);
 
      //Write MorphTargets Tags
      for (int j=0; j<cal3dfact->GetMorphTargetCount(i); j++)
      {
        const char *morph_file, *morph_name;
        cal3dfact->GetCoreMorphTarget(i,morph_file,morph_name);
 
        csRef<iDocumentNode> morphtargetNode =
          meshNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        morphtargetNode->SetValue("morphtarget");
        meshNode->SetAttribute("name", morph_name);
        meshNode->SetAttribute("file", morph_file);
      }
 
    }
 
    //Write MorphAnimation Tags
    for (int i=0; i<cal3dfact->GetMorphAnimationCount(); i++)
    {
      csRef<iDocumentNode> morphNode =
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      morphNode->SetValue("morphanimation");
 
      const char* morphname = cal3dfact->GetMorphAnimationName(i);
      meshNode->SetAttribute("name", morphname);
 
      //Write MorphTargets Tags
      for (int j=0; j<cal3dfact->GetMorphTargetCount(i); j++)
      {
        const char *morph_file, *morph_name;
        cal3dfact->GetCoreMorphTarget(i,morph_file,morph_name);
 
        csRef<iDocumentNode> morphtargetNode =
          meshNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        morphtargetNode->SetValue("morphtarget");
        meshNode->SetAttribute("name", morph_name);
        meshNode->SetAttribute("file", morph_file);
      }
    }
 
    //Write Animation Tags
    for (int i=0; i<cal3dfact->GetAnimationCount(); i++)
    {
      const char *name, *file;
      bool lock;
      int  type, max_interval, min_interval, idle_pct;
      float base_vel, min_vel, max_vel;
      cal3dfact->GetCoreAnimation(i, file, name, type, base_vel,
	min_vel, max_vel, min_interval, max_interval, idle_pct, lock);
 
      csRef<iDocumentNode> aniNode =
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      aniNode->SetValue("animation");
 
      aniNode->SetAttribute("name", name);
      aniNode->SetAttribute("file", file);
 
      if (type == iSpriteCal3DState::C3D_ANIM_TYPE_IDLE)
        aniNode->SetAttribute("type", "idle");
      else if (type == iSpriteCal3DState::C3D_ANIM_TYPE_TRAVEL)
        aniNode->SetAttribute("type", "travel");
      else if (type == iSpriteCal3DState::C3D_ANIM_TYPE_CYCLE)
        aniNode->SetAttribute("type", "cycle");
      else if (type == iSpriteCal3DState::C3D_ANIM_TYPE_STYLE_CYCLE)
        aniNode->SetAttribute("type", "style_cycle");
      else if (type == iSpriteCal3DState::C3D_ANIM_TYPE_ACTION)
        aniNode->SetAttribute("type", "action");
 
      aniNode->SetAttributeAsInt("base_vel", base_vel);
      aniNode->SetAttributeAsFloat("min_vel", min_vel);
      aniNode->SetAttributeValueAsFloat("max_vel", max_vel);
      aniNode->SetAttributeValueAsInt("max_random", max_interval);
      aniNode->SetAttributeValueAsInt("min_random", min_interval);
      aniNode->SetAttributeValueAsInt("idle_pct", idle_pct);
      aniNode->SetAttributeValueAsBool("lock", lock);
    }
 
    //Write Socket Tags
    for (int i=0; i<cal3dfact->GetSocketCount(); i++)
    {
      csRef<iDocumentNode> socketNode =
	paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      socketNode->SetValue("socket");
      iSpriteCal3DSocket* sprite_socket = cal3dfact->GetSocket(i);
      socketNode->SetAttribute("name", sprite_socket->GetName());
      socketNode->SetAttributeAsInt("tri", sprite_socket->GetTriangleIndex());
      socketNode->SetAttributeAsInt("submesh",
        sprite_socket->GetSubmeshIndex());
      socketNode->SetAttributeAsInt("mesh", sprite_socket->GetMeshIndex());
    }
 
  }
#endif
  return true;
}  

//---------------------------------------------------------------------------

csSpriteCal3DLoader::csSpriteCal3DLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpriteCal3DLoader::~csSpriteCal3DLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpriteCal3DLoader::Initialize (iObjectRegistry* object_reg)
{
  csSpriteCal3DLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("animcycle", XMLTOKEN_ANIMCYCLE);
  xmltokens.Register ("idleanim", XMLTOKEN_IDLEANIM);
  xmltokens.Register ("idle", XMLTOKEN_IDLE);

  return true;
}

csPtr<iBase> csSpriteCal3DLoader::Parse (iDocumentNode* node,
					 iStreamSource*,
					 iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iSpriteCal3DState> sprCal3dLook;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_FACTORY:
      {
	const char* factname = child->GetContentsValue ();
	iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	if (!fact)
	{
	  synldr->ReportError (
	    "crystalspace.spritecal3dloader.parse.unknownfactory",
	    child, "Couldn't find factory '%s'!", factname);
	  return 0;
	}
	mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	sprCal3dLook = SCF_QUERY_INTERFACE (mesh, iSpriteCal3DState);
      }
      break;
    case XMLTOKEN_ANIMCYCLE:
      if (!sprCal3dLook)
      {
	synldr->ReportError (
	  "crystalspace.spritecal3dloader.parse.motion.missingfactory",
	  child,
	  "No Factory! Please define 'factory' before 'animcycle'!");
	return 0;
      }
      if (sprCal3dLook->FindAnim(child->GetContentsValue ()) != -1)
      {
         sprCal3dLook->SetAnimCycle (child->GetContentsValue (),1.0);
      }
      else
      {
	synldr->ReportError (
	  "crystalspace.spritecal3dloader.parse.motion.missingfactory",
	  child,
	  "Anim cycle not found!!!");
	  // Non fatal error?
      }
	 
      break;
    case XMLTOKEN_IDLEANIM:
      if (!sprCal3dLook)
      {
	synldr->ReportError (
	  "crystalspace.spritecal3dloader.parse.motion.missingfactory",
	  child,
	  "No Factory! Please define 'factory' before 'idleanim'!");
	return 0;
      }
      if (sprCal3dLook->FindAnim(child->GetContentsValue ()) != -1)
      {
      	sprCal3dLook->SetDefaultIdleAnim (child->GetContentsValue ());
      }
      else
      {
	synldr->ReportError (
	  "crystalspace.spritecal3dloader.parse.motion.missingfactory",
	  child,
	  "Anim cycle not found!!!");
	  // Non fatal error?
      }
      break;
    case XMLTOKEN_IDLE:
      if (!sprCal3dLook)
      {
	synldr->ReportError (
	  "crystalspace.spritecal3dloader.parse.motion.missingfactory",
	  child,
	  "No Factory! Please define 'factory' before 'idle'!");
	return 0;
      }
      sprCal3dLook->SetVelocity(0);
      break;

    default:
      synldr->ReportBadToken (child);
      return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csSpriteCal3DSaver::csSpriteCal3DSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpriteCal3DSaver::~csSpriteCal3DSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpriteCal3DSaver::Initialize (iObjectRegistry* object_reg)
{
  csSpriteCal3DSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

bool csSpriteCal3DSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent)
    return false;
  
  csRef<iDocumentNode> paramsNode =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for cal3d mesh");
  paramsNode=0;
  
  return true;
}
