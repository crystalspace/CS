/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csutil/util.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/portal.h"
#include "imesh/object.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "thingldr.h"
#include "qint.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_CLONE = 1,
  XMLTOKEN_COSFACT,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FASTMESH,
  XMLTOKEN_FOG,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_REPLACEMATERIAL,
  XMLTOKEN_MOVEABLE,
  XMLTOKEN_PART,
  XMLTOKEN_P,
  XMLTOKEN_TEXLEN,
  XMLTOKEN_VISTREE,
  XMLTOKEN_V,
  XMLTOKEN_SMOOTH
};

SCF_IMPLEMENT_IBASE (csThingLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csThingSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingLoader)
SCF_IMPLEMENT_FACTORY (csThingFactoryLoader)
SCF_IMPLEMENT_FACTORY (csThingSaver)


#define MAXLINE 200 /* max number of chars per line... */

//---------------------------------------------------------------------------

csThingLoader::csThingLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingLoader::~csThingLoader ()
{
}

bool csThingLoader::Initialize (iObjectRegistry* object_reg)
{
  csThingLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("clone", XMLTOKEN_CLONE);
  xmltokens.Register ("cosfact", XMLTOKEN_COSFACT);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("fastmesh", XMLTOKEN_FASTMESH);
  xmltokens.Register ("fog", XMLTOKEN_FOG);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("replacematerial", XMLTOKEN_REPLACEMATERIAL);
  xmltokens.Register ("moveable", XMLTOKEN_MOVEABLE);
  xmltokens.Register ("part", XMLTOKEN_PART);
  xmltokens.Register ("p", XMLTOKEN_P);
  xmltokens.Register ("smooth", XMLTOKEN_SMOOTH);
  xmltokens.Register ("texlen", XMLTOKEN_TEXLEN);
  xmltokens.Register ("vistree", XMLTOKEN_VISTREE);
  xmltokens.Register ("v", XMLTOKEN_V);
  return true;
}

bool csThingLoader::LoadThingPart (iThingEnvironment* te, iDocumentNode* node,
	iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, ThingLoadInfo& info,
	iEngine* engine, int vt_offset, bool isParent)
{
#define CHECK_TOPLEVEL(m) \
if (!isParent) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"'%s' flag only for top-level thing!", m); \
return false; \
}

#define CHECK_OBJECTONLY(m) \
if (info.load_factory) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"'%s' is not for factories!", m); \
return false; \
} \
if (!info.thing_state) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"Factory must be given before using '%s'!", m); \
return false; \
}

#define CHECK_DONTEXTENDFACTORY \
if ((!info.load_factory) && info.global_factory) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"Can't change the object when using 'factory' or 'clone'!"); \
return false; \
}

#define CREATE_FACTORY_IF_NEEDED \
if (!info.thing_fact_state) \
{ \
  csRef<iMeshObjectFactory> fact; \
  fact = info.type->NewFactory (); \
  info.thing_fact_state = SCF_QUERY_INTERFACE (fact, \
	iThingFactoryState); \
  info.obj = fact->NewInstance (); \
  info.thing_state = SCF_QUERY_INTERFACE (info.obj, \
	iThingState); \
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
      case XMLTOKEN_VISTREE:
	synldr->ReportError (
	    "crystalspace.thingloader.parse.vistree",
	    child, "'vistree' no longer supported! Convert your level to Dynavis using 'levtool'!\n");
	printf ("'vistree' no longer supported! Convert your level to Dynavis using 'levtool'!\n");
	return false;
      case XMLTOKEN_COSFACT:
        CHECK_TOPLEVEL("cosfact");
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        info.thing_fact_state->SetCosinusFactor (
		child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_FASTMESH:
        CHECK_TOPLEVEL("fastmesh");
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        info.thing_fact_state->GetFlags ().Set (CS_THING_FASTMESH);
        break;
      case XMLTOKEN_MOVEABLE:
        CHECK_TOPLEVEL("moveable");
	CHECK_OBJECTONLY("moveable");
        info.thing_state->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
        break;
      case XMLTOKEN_FACTORY:
        CHECK_TOPLEVEL("factory");
        if (info.load_factory)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "Can't use 'factory' when parsing a factory!");
	  return false;
	}
        if (info.thing_fact_state)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "'factory' already specified!");
	  return false;
	}
	info.global_factory = true;

        {
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
          if (!fact)
          {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.factory",
              child, "Couldn't find thing factory '%s'!", factname);
            return false;
          }
	  info.thing_fact_state = SCF_QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory (), iThingFactoryState);
	  if (!info.thing_fact_state)
	  {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.factory",
              child, "Object '%s' is not a thing factory!", factname);
            return false;
	  }
	  info.obj = fact->GetMeshObjectFactory ()->NewInstance ();
	  info.thing_state = SCF_QUERY_INTERFACE (info.obj, iThingState);
        }
        break;
      case XMLTOKEN_CLONE:
        CHECK_TOPLEVEL("clone");
        if (info.load_factory)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "Parsing a factory, so can't use 'clone'!");
	  return false;
	}
        if (info.thing_fact_state)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "'factory' already specified, so can't use 'clone'!");
	  return false;
	}
	info.global_factory = true;

        {
	  const char* meshname = child->GetContentsValue ();
	  iMeshWrapper* wrap = ldr_context->FindMeshObject (meshname);
          if (!wrap)
          {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.clone",
              child, "Couldn't find thing '%s'!", meshname);
            return false;
          }

	  csRef<iThingState> other_thing_state (SCF_QUERY_INTERFACE (
	  	wrap->GetMeshObject (), iThingState));
	  if (!other_thing_state)
	  {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.clone",
              child, "Object '%s' is not a thing!", meshname);
            return false;
	  }
	  info.thing_fact_state = other_thing_state->GetFactory ();
	  info.obj = wrap->GetFactory ()->GetMeshObjectFactory ()
	  	->NewInstance ();
	  info.thing_state = SCF_QUERY_INTERFACE (info.obj, iThingState);
        }
        break;
      case XMLTOKEN_PART:
	if (!LoadThingPart (te, child, ldr_context, object_reg, reporter,
		synldr, info, engine, info.thing_fact_state
			? info.thing_fact_state->GetVertexCount () : 0,
		false))
	  return false;
        break;
      case XMLTOKEN_V:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        {
	  csVector3 v;
	  if (!synldr->ParseVector (child, v))
	    return false;
          info.thing_fact_state->CreateVertex (v);
        }
        break;
      case XMLTOKEN_FOG:
	synldr->ReportError (
	      "crystalspace.thingloader.parse.fog",
      	      child, "FOG for things is currently not supported!\n\
Nag to Jorrit about this feature if you want it.");
	return false;

      case XMLTOKEN_P:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        {
	  iPolygon3DStatic* poly3d = info.thing_fact_state->CreatePolygon (
			  child->GetAttributeValue ("name"));
	  if (info.default_material)
	    poly3d->SetMaterial (info.default_material);
	  if (!synldr->ParsePoly3d (child, ldr_context,
	  			    engine, poly3d,
				    info.default_texlen, info.thing_fact_state,
				    vt_offset))
	  {
	    info.thing_fact_state->RemovePolygon (
	      info.thing_fact_state->FindPolygonIndex (poly3d));
	    return false;
	  }
        }
        break;

      case XMLTOKEN_REPLACEMATERIAL:
        CHECK_TOPLEVEL("replacematerial");
        CHECK_OBJECTONLY("replacematerial");
	{
	  int idx = info.replace_materials.Push (RepMaterial ());
	  info.replace_materials[idx].oldmat = csStrNew (
		child->GetAttributeValue ("old"));
	  info.replace_materials[idx].newmat = csStrNew (
		child->GetAttributeValue ("new"));
	}
	break;

      case XMLTOKEN_MATERIAL:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
	{
	  const char* matname = child->GetContentsValue ();
          info.default_material = ldr_context->FindMaterial (matname);
          if (info.default_material == 0)
          {
	    synldr->ReportError (
	        "crystalspace.thingloader.parse.material",
                child, "Couldn't find material named '%s'!", matname);
            return false;
          }
	}
        break;
      case XMLTOKEN_TEXLEN:
	info.default_texlen = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_SMOOTH:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
	info.thing_fact_state->SetSmoothingFlag (true);
	break;
      default:
        synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

csPtr<iBase> csThingLoader::Parse (iDocumentNode* node,
			     iLoaderContext* ldr_context, iBase*)
{
  ThingLoadInfo info;
  info.load_factory = false;
  info.global_factory = false;

  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  info.type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.thing", iMeshObjectType);
  if (!info.type)
  {
    info.type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.thing",
    	iMeshObjectType);
  }
  if (!info.type)
  {
    synldr->ReportError (
		"crystalspace.thingloader.setup.objecttype",
		node, "Could not load the thing mesh object plugin!");
    return 0;
  }
  csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (info.type,
  	iThingEnvironment);
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  if (!LoadThingPart (te, node, ldr_context, object_reg, reporter, synldr, info,
  	engine, 0, true))
  {
    info.obj = 0;
  }
  else
  {
    int i;
    for (i = 0 ; i < info.replace_materials.Length () ; i++)
    {
      RepMaterial& rm = info.replace_materials[i];
      iMaterialWrapper* old_mat = ldr_context->FindMaterial (rm.oldmat);
      if (!old_mat)
      {
	synldr->ReportError (
	        "crystalspace.thingloader.parse.material",
                node, "Couldn't find material named '%s'!", rm.oldmat);
	return 0;
      }
      iMaterialWrapper* new_mat = ldr_context->FindMaterial (rm.newmat);
      if (!new_mat)
      {
	synldr->ReportError (
	        "crystalspace.thingloader.parse.material",
                node, "Couldn't find material named '%s'!", rm.newmat);
	return 0;
      }
      info.thing_state->ReplaceMaterial (old_mat, new_mat);
    }
  }

  return csPtr<iBase> (info.obj);
}

csPtr<iBase> csThingFactoryLoader::Parse (iDocumentNode* node,
			     iLoaderContext* ldr_context, iBase*)
{
  ThingLoadInfo info;
  info.load_factory = true;
  info.global_factory = false;

  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  info.type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.thing", iMeshObjectType);
  if (!info.type)
  {
    info.type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.thing",
    	iMeshObjectType);
  }
  if (!info.type)
  {
    synldr->ReportError (
		"crystalspace.thingloader.setup.objecttype",
		node, "Could not load the thing mesh object plugin!");
    return 0;
  }
  csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (info.type,
  	iThingEnvironment);
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  csRef<iMeshObjectFactory> fact;
  fact = info.type->NewFactory ();
  info.thing_fact_state = SCF_QUERY_INTERFACE (fact, iThingFactoryState);

  if (!LoadThingPart (te, node, ldr_context, object_reg, reporter, synldr, info,
  	engine, 0, true))
  {
    fact = 0;
  }
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csThingSaver::csThingSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingSaver::~csThingSaver ()
{
}

bool csThingSaver::Initialize (iObjectRegistry* object_reg)
{
  csThingSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

void csThingSaver::WriteDown (iBase* /*obj*/, iFile *file)
{
  csString str;
  csRef<iFactory> fact (SCF_QUERY_INTERFACE (this, iFactory));
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace (name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf (buf, "FACTORY ('%s')\n", name);
  str.Append (buf);
  file->Write ((const char*)str, str.Length ());
}

