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
#include "csutil/parser.h"
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
#include "imesh/object.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/portal.h"
#include "imesh/thing/polytmap.h"
#include "imesh/thing/ptextype.h"
#include "imesh/thing/curve.h"
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

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (CIRCLE)
  CS_TOKEN_DEF (CLONE)
  CS_TOKEN_DEF (CURVE)
  CS_TOKEN_DEF (CURVECENTER)
  CS_TOKEN_DEF (CURVECONTROL)
  CS_TOKEN_DEF (CURVESCALE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FASTMESH)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FIRST_LEN)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (LEN)
  CS_TOKEN_DEF (MAT_SET_SELECT)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MOVEABLE)
  CS_TOKEN_DEF (NAME)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (PART)
  CS_TOKEN_DEF (POLYGON)
  CS_TOKEN_DEF (P)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (SECOND_LEN)
  CS_TOKEN_DEF (TEMPLATE)
  CS_TOKEN_DEF (TEXLEN)
  CS_TOKEN_DEF (UVEC)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (VVEC)
  CS_TOKEN_DEF (VISTREE)
  CS_TOKEN_DEF (VBLOCK)
  CS_TOKEN_DEF (VROOM)
  CS_TOKEN_DEF (W)
  CS_TOKEN_DEF (V)
CS_TOKEN_DEF_END

enum
{
  XMLTOKEN_CLONE = 1,
  XMLTOKEN_CURVE,
  XMLTOKEN_CURVECENTER,
  XMLTOKEN_CURVECONTROL,
  XMLTOKEN_CURVESCALE,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FASTMESH,
  XMLTOKEN_FOG,
  XMLTOKEN_MATSETSELECT,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MOVEABLE,
  XMLTOKEN_PART,
  XMLTOKEN_P,
  XMLTOKEN_RADIUS,
  XMLTOKEN_TEXLEN,
  XMLTOKEN_VISTREE,
  XMLTOKEN_V,

  // Below is for plane loader.
  XMLTOKEN_ORIG,
  XMLTOKEN_FIRSTLEN,
  XMLTOKEN_FIRST,
  XMLTOKEN_SECONDLEN,
  XMLTOKEN_SECOND,
  XMLTOKEN_MATRIX,
  //XMLTOKEN_V,
  XMLTOKEN_NAME,
  XMLTOKEN_UVEC,
  XMLTOKEN_VVEC
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

SCF_IMPLEMENT_IBASE (csPlaneLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPlaneLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csPlaneSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPlaneSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBezierLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBezierLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBezierSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBezierSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingLoader)
SCF_IMPLEMENT_FACTORY (csThingSaver)
SCF_IMPLEMENT_FACTORY (csPlaneLoader)
SCF_IMPLEMENT_FACTORY (csPlaneSaver)
SCF_IMPLEMENT_FACTORY (csBezierLoader)
SCF_IMPLEMENT_FACTORY (csBezierSaver)

SCF_EXPORT_CLASS_TABLE (thingldr)
  SCF_EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.factory.thing",
    "Crystal Space Thing Mesh Factory Loader")
  SCF_EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.factory.thing",
    "Crystal Space Thing Mesh Factory Saver")
  SCF_EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.thing",
    "Crystal Space Thing Mesh Loader")
  SCF_EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.thing",
    "Crystal Space Thing Mesh Saver")
  SCF_EXPORT_CLASS (csPlaneLoader, "crystalspace.mesh.loader.thing.plane",
    "Crystal Space Thing Plane Loader")
  SCF_EXPORT_CLASS (csPlaneSaver, "crystalspace.mesh.saver.thing.plane",
    "Crystal Space Thing Plane Saver")
  SCF_EXPORT_CLASS (csBezierLoader, "crystalspace.mesh.loader.thing.bezier",
    "Crystal Space Thing Bezier Loader")
  SCF_EXPORT_CLASS (csBezierSaver, "crystalspace.mesh.saver.thing.bezier",
    "Crystal Space Thing Bezier Saver")
SCF_EXPORT_CLASS_TABLE_END

#define MAXLINE 200 /* max number of chars per line... */

static void ReportError (iReporter* reporter, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (reporter)
  {
    reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

//---------------------------------------------------------------------------

csThingLoader::csThingLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  synldr = NULL;
  reporter = NULL;
  plugin_mgr = NULL;
}

csThingLoader::~csThingLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (reporter);
}

bool csThingLoader::Initialize (iObjectRegistry* object_reg)
{
  csThingLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("clone", XMLTOKEN_CLONE);
  xmltokens.Register ("curve", XMLTOKEN_CURVE);
  xmltokens.Register ("curvecenter", XMLTOKEN_CURVECENTER);
  xmltokens.Register ("curvecontrol", XMLTOKEN_CURVECONTROL);
  xmltokens.Register ("curvescale", XMLTOKEN_CURVESCALE);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("fastmesh", XMLTOKEN_FASTMESH);
  xmltokens.Register ("fog", XMLTOKEN_FOG);
  xmltokens.Register ("matsetselect", XMLTOKEN_MATSETSELECT);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("moveable", XMLTOKEN_MOVEABLE);
  xmltokens.Register ("part", XMLTOKEN_PART);
  xmltokens.Register ("p", XMLTOKEN_P);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("texlen", XMLTOKEN_TEXLEN);
  xmltokens.Register ("vistree", XMLTOKEN_VISTREE);
  xmltokens.Register ("v", XMLTOKEN_V);
  return true;
}

static bool load_thing_part (csParser* parser, iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, ThingLoadInfo& info,
	iEngine* engine, iThingState* thing_state,
	char* buf, int vt_offset, bool isParent)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (VBLOCK)
    CS_TOKEN_TABLE (VROOM)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (CIRCLE)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (CURVECENTER)
    CS_TOKEN_TABLE (CURVESCALE)
    CS_TOKEN_TABLE (CURVECONTROL)
    CS_TOKEN_TABLE (CURVE)
    CS_TOKEN_TABLE (MAT_SET_SELECT)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (TEXLEN)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (MOVEABLE)
    CS_TOKEN_TABLE (TEMPLATE)
    CS_TOKEN_TABLE (CLONE)
    CS_TOKEN_TABLE (VISTREE)
    CS_TOKEN_TABLE (FASTMESH)
    CS_TOKEN_TABLE (PART)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (P)
  CS_TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;
  char str[255];

  while ((cmd = parser->GetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
	"crystalspace.thingloader.parse.badformat",
        "Expected parameters instead of '%s'!", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_VISTREE:
        if (!isParent)
	{
	  ReportError (reporter,
	    "crystalspace.thingloader.parse.vistree",
	    "VISTREE flag only for top-level thing!");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_VISTREE);
        break;
      case CS_TOKEN_FASTMESH:
        if (!isParent)
	{
	  ReportError (reporter,
	    "crystalspace.thingloader.parse.fastmesh",
	    "FASTMESH flag only for top-level thing!");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_FASTMESH);
        break;
      case CS_TOKEN_MOVEABLE:
        if (!isParent)
	{
	  ReportError (reporter,
	    "crystalspace.thingloader.parse.moveable",
	    "MOVEABLE flag only for top-level thing!");
	  return false;
	}
        else thing_state->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
        break;
      case CS_TOKEN_TEMPLATE:
      case CS_TOKEN_FACTORY:
        if (!isParent)
	{
	  ReportError (reporter,
	    "crystalspace.thingloader.parse.factory",
	    "FACTORY or TEMPLATE statement only for top-level thing!");
	  return false;
	}
	else
        {
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
          if (!fact)
          {
	    ReportError (reporter,
	      "crystalspace.thingloader.parse.factory",
              "Couldn't find thing factory '%s'!", str);
            return false;
          }
	  iThingState* tmpl_thing_state = SCF_QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory (), iThingState);
	  if (!tmpl_thing_state)
	  {
	    ReportError (reporter,
	      "crystalspace.thingloader.parse.factory",
              "Object '%s' is not a thing!", str);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  tmpl_thing_state->DecRef ();
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case CS_TOKEN_CLONE:
        if (!isParent)
	{
	  ReportError (reporter,
	    "crystalspace.thingloader.parse.clone",
	    "CLONE statement only for top-level thing!");
	  return false;
	}
	else
        {
          csScanStr (params, "%s", str);
	  iMeshWrapper* wrap = ldr_context->FindMeshObject (str);
          if (!wrap)
          {
	    ReportError (reporter,
	      "crystalspace.thingloader.parse.clone",
              "Couldn't find thing '%s'!", str);
            return false;
          }

	  iThingState* tmpl_thing_state = SCF_QUERY_INTERFACE (
	  	wrap->GetMeshObject (), iThingState);
	  if (!tmpl_thing_state)
	  {
	    ReportError (reporter,
	      "crystalspace.thingloader.parse.clone",
              "Object '%s' is not a thing!", str);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  tmpl_thing_state->DecRef ();
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case CS_TOKEN_PART:
	if (!load_thing_part (parser, ldr_context, object_reg, reporter, synldr, info,
		engine, thing_state, params,
		thing_state->GetVertexCount (), false))
	  return false;
        break;
      case CS_TOKEN_V:
      case CS_TOKEN_VERTEX:
        {
	  csVector3 v;
          csScanStr (params, "%f,%f,%f", &v.x, &v.y, &v.z);
          thing_state->CreateVertex (v);
        }
        break;
      case CS_TOKEN_CIRCLE:
        {
          float x, y, z, rx, ry, rz;
          int num, dir;
          csScanStr (params, "%f,%f,%f:%f,%f,%f,%d",
	  	&x, &y, &z, &rx, &ry, &rz, &num);
          if (num < 0) { num = -num; dir = -1; }
          else dir = 1;
          for (int i = 0 ; i < num ; i++)
          {
            float rad;
            if (dir == 1) rad = TWO_PI * (num-i-1)/(float)num;
            else rad = TWO_PI * i / (float)num;

            float cx = 0, cy = 0, cz = 0;
            float cc = cos (rad);
            float ss = sin (rad);
            if      (ABS (rx) < SMALL_EPSILON)
	    { cx = x; cy = y+cc*ry; cz = z+ss*rz; }
            else if (ABS (ry) < SMALL_EPSILON)
	    { cy = y; cx = x+cc*rx; cz = z+ss*rz; }
            else if (ABS (rz) < SMALL_EPSILON)
	    { cz = z; cx = x+cc*rx; cy = y+ss*ry; }
            thing_state->CreateVertex (csVector3 (cx, cy, cz));
          }
        }
        break;
      case CS_TOKEN_FOG:
	ReportError (reporter,
	      "crystalspace.thingloader.parse.fog",
      	      "FOG for things is currently not supported!\n\
Nag to Jorrit about this feature if you want it.");
	return false;
#if 0
//@@@
        if (!isParent)
	{
	  printf ("FOG statement only for top-level thing!\n");
	  return false;
	}
	else
        {
          csFog& f = thing->GetFog ();
          f.enabled = true;
          csScanStr (params, "%f,%f,%f,%f",
	  	&f.red, &f.green, &f.blue, &f.density);
        }
#endif
        break;

      case CS_TOKEN_VROOM:
        {
	  float minx, miny, minz, maxx, maxy, maxz;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	    &minx, &miny, &minz,
	    &maxx, &maxy, &maxz);
          thing_state->CreateVertex (csVector3 (maxx, maxy, maxz));
          thing_state->CreateVertex (csVector3 (minx, maxy, maxz));
          thing_state->CreateVertex (csVector3 (maxx, maxy, minz));
          thing_state->CreateVertex (csVector3 (minx, maxy, minz));
          thing_state->CreateVertex (csVector3 (maxx, miny, maxz));
          thing_state->CreateVertex (csVector3 (minx, miny, maxz));
          thing_state->CreateVertex (csVector3 (maxx, miny, minz));
          thing_state->CreateVertex (csVector3 (minx, miny, minz));
	}
	break;

      case CS_TOKEN_VBLOCK:
        {
	  float minx, miny, minz, maxx, maxy, maxz;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	    &minx, &miny, &minz,
	    &maxx, &maxy, &maxz);
          thing_state->CreateVertex (csVector3 (minx, miny, minz));
          thing_state->CreateVertex (csVector3 (maxx, miny, minz));
          thing_state->CreateVertex (csVector3 (minx, miny, maxz));
          thing_state->CreateVertex (csVector3 (maxx, miny, maxz));
          thing_state->CreateVertex (csVector3 (minx, maxy, minz));
          thing_state->CreateVertex (csVector3 (maxx, maxy, minz));
          thing_state->CreateVertex (csVector3 (minx, maxy, maxz));
          thing_state->CreateVertex (csVector3 (maxx, maxy, maxz));
	}
	break;

      case CS_TOKEN_P:
      case CS_TOKEN_POLYGON:
        {
	  iPolygon3D* poly3d = thing_state->CreatePolygon (xname);
	  if (info.default_material)
	    poly3d->SetMaterial (info.default_material);
	  if (!synldr->ParsePoly3d (parser, ldr_context,
	  			    engine, poly3d, params,
				    info.default_texlen, thing_state,
				    vt_offset))
	  {
	    poly3d->DecRef ();
	    return false;
	  }
        }
        break;

      case CS_TOKEN_CURVE:
        {
	  char cname[100];
	  csScanStr (params, "%s", cname);
	  iThingEnvironment* te = SCF_QUERY_INTERFACE (engine->GetThingType (),
	    iThingEnvironment);
	  iCurveTemplate* ct = te->FindCurveTemplate (cname);
	  te->DecRef ();
	  iCurve* p = thing_state->CreateCurve (ct);
	  p->QueryObject()->SetName (cname);
          if (!ct->GetMaterial ())
	    p->SetMaterial (info.default_material);
        }
        break;

      case CS_TOKEN_CURVECENTER:
        {
          csVector3 c;
          csScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
          thing_state->SetCurvesCenter (c);
        }
        break;
      case CS_TOKEN_CURVESCALE:
        {
	  float f;
          csScanStr (params, "%f", &f);
	  thing_state->SetCurvesScale (f);
          break;
        }
      case CS_TOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
          csScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
          thing_state->AddCurveVertex (v, t);
        }
        break;

      case CS_TOKEN_MATERIAL:
        csScanStr (params, "%s", str);

        info.default_material = ldr_context->FindMaterial (str);
        if (info.default_material == NULL)
        {
	  ReportError (reporter,
	      "crystalspace.thingloader.parse.material",
              "Couldn't find material named '%s'!", str);
          return false;
        }
        break;
      case CS_TOKEN_TEXLEN:
        csScanStr (params, "%f", &info.default_texlen);
        break;
      case CS_TOKEN_MAT_SET_SELECT:
        csScanStr (params, "%s", str);
        info.SetTextureSet (str);
        info.use_mat_set = true;
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    ReportError (reporter,
	"crystalspace.thingloader.parse.badformat",
	"Token '%s' not found while parsing a thing!",
    	parser->GetLastOffender ());
    return false;
  }
  return true;
}

csPtr<iBase> csThingLoader::Parse (const char* string, 
			     iLoaderContext* ldr_context, iBase*)
{
  csParser* parser = ldr_context->GetParser ();

  // Things only work with the real 3D engine and not with the iso engine.
  iEngine* engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  CS_ASSERT (engine != NULL);
  iMeshObjectFactory* fact = NULL;
  iThingState* thing_state = NULL;

  iMeshObjectType* type = engine->GetThingType (); // @@@ CS_LOAD_PLUGIN LATER!
  // We always do NewFactory() even for mesh objects.
  // That's because csThing implements both so a factory is a mesh object.
  fact = type->NewFactory ();
  thing_state = SCF_QUERY_INTERFACE (fact, iThingState);

  char* buf = (char*)string;
  ThingLoadInfo info;
  if (!load_thing_part (parser, ldr_context, object_reg, reporter, synldr, info,
  	engine, thing_state, buf, 0, true))
  {
    fact->DecRef ();
    fact = NULL;
  }
  thing_state->DecRef ();
  engine->DecRef ();
  return csPtr<iBase> (fact);
}

// XML versions: -------------------------------------------------

bool csThingLoader::LoadThingPart (iDocumentNode* node, iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, ThingLoadInfo& info,
	iEngine* engine, iThingState* thing_state,
	int vt_offset, bool isParent)
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
      case XMLTOKEN_VISTREE:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.vistree",
	    child, "'vistree' flag only for top-level thing!");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_VISTREE);
        break;
      case XMLTOKEN_FASTMESH:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.fastmesh",
	    child, "'fastmesh' flag only for top-level thing!");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_FASTMESH);
        break;
      case XMLTOKEN_MOVEABLE:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.moveable",
	    child, "'moveable' flag only for top-level thing!");
	  return false;
	}
        else thing_state->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
        break;
      case XMLTOKEN_FACTORY:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "'factory' statement only for top-level thing!");
	  return false;
	}
	else
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
	  csRef<iThingState> tmpl_thing_state (SCF_QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory (), iThingState));
	  if (!tmpl_thing_state)
	  {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.factory",
              child, "Object '%s' is not a thing!", factname);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case XMLTOKEN_CLONE:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.clone",
	    child, "CLONE statement only for top-level thing!");
	  return false;
	}
	else
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

	  csRef<iThingState> tmpl_thing_state (SCF_QUERY_INTERFACE (
	  	wrap->GetMeshObject (), iThingState));
	  if (!tmpl_thing_state)
	  {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.clone",
              child, "Object '%s' is not a thing!", meshname);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case XMLTOKEN_PART:
	if (!LoadThingPart (child, ldr_context, object_reg, reporter,
		synldr, info, engine, thing_state,
		thing_state->GetVertexCount (), false))
	  return false;
        break;
      case XMLTOKEN_V:
        {
	  csVector3 v;
	  if (!synldr->ParseVector (child, v))
	    return false;
          thing_state->CreateVertex (v);
        }
        break;
      case XMLTOKEN_FOG:
	synldr->ReportError (
	      "crystalspace.thingloader.parse.fog",
      	      child, "FOG for things is currently not supported!\n\
Nag to Jorrit about this feature if you want it.");
	return false;

      case XMLTOKEN_P:
        {
	  iPolygon3D* poly3d = thing_state->CreatePolygon (
			  child->GetAttributeValue ("name"));
	  if (info.default_material)
	    poly3d->SetMaterial (info.default_material);
	  if (!synldr->ParsePoly3d (child, ldr_context,
	  			    engine, poly3d,
				    info.default_texlen, thing_state,
				    vt_offset))
	  {
	    poly3d->DecRef ();
	    return false;
	  }
        }
        break;

      case XMLTOKEN_CURVE:
        {
	  const char* cname = child->GetContentsValue ();
	  csRef<iThingEnvironment> te (
	  	SCF_QUERY_INTERFACE (engine->GetThingType (),
		iThingEnvironment));
	  iCurveTemplate* ct = te->FindCurveTemplate (cname);
	  iCurve* p = thing_state->CreateCurve (ct);
	  p->QueryObject()->SetName (cname);
          if (!ct->GetMaterial ())
	    p->SetMaterial (info.default_material);
        }
        break;

      case XMLTOKEN_CURVECENTER:
        {
          csVector3 c;
	  if (!synldr->ParseVector (child, c))
	    return false;
          thing_state->SetCurvesCenter (c);
        }
        break;
      case XMLTOKEN_CURVESCALE:
        {
	  float f = child->GetContentsValueAsFloat ();
	  thing_state->SetCurvesScale (f);
          break;
        }
      case XMLTOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
	  v.x = child->GetAttributeValueAsFloat ("x");
	  v.y = child->GetAttributeValueAsFloat ("y");
	  v.z = child->GetAttributeValueAsFloat ("z");
	  t.x = child->GetAttributeValueAsFloat ("u");
	  t.y = child->GetAttributeValueAsFloat ("v");
          thing_state->AddCurveVertex (v, t);
        }
        break;

      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          info.default_material = ldr_context->FindMaterial (matname);
          if (info.default_material == NULL)
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
      case XMLTOKEN_MATSETSELECT:
        info.SetTextureSet (child->GetContentsValue ());
        info.use_mat_set = true;
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
  // Things only work with the real 3D engine and not with the iso engine.
  iEngine* engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  CS_ASSERT (engine != NULL);
  iMeshObjectFactory* fact = NULL;
  iThingState* thing_state = NULL;

  iMeshObjectType* type = engine->GetThingType (); // @@@ CS_LOAD_PLUGIN LATER!
  // We always do NewFactory() even for mesh objects.
  // That's because csThing implements both so a factory is a mesh object.
  fact = type->NewFactory ();
  thing_state = SCF_QUERY_INTERFACE (fact, iThingState);

  ThingLoadInfo info;
  if (!LoadThingPart (node, ldr_context, object_reg, reporter, synldr, info,
  	engine, thing_state, 0, true))
  {
    fact->DecRef ();
    fact = NULL;
  }
  thing_state->DecRef ();
  engine->DecRef ();
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csThingSaver::csThingSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csThingSaver::~csThingSaver ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (reporter);
}

bool csThingSaver::Initialize (iObjectRegistry* object_reg)
{
  csThingSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

void csThingSaver::WriteDown (iBase* /*obj*/, iFile *file)
{
  csString str;
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace (name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf (buf, "FACTORY ('%s')\n", name);
  str.Append (buf);
  fact->DecRef ();
  file->Write ((const char*)str, str.Length ());
}

//---------------------------------------------------------------------------

csPlaneLoader::csPlaneLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  synldr = NULL;
  reporter = NULL;
  plugin_mgr = NULL;
}

csPlaneLoader::~csPlaneLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (reporter);
}

bool csPlaneLoader::Initialize (iObjectRegistry* object_reg)
{
  csPlaneLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("clone", XMLTOKEN_CLONE);
  xmltokens.Register ("orig", XMLTOKEN_ORIG);
  xmltokens.Register ("firstlen", XMLTOKEN_FIRSTLEN);
  xmltokens.Register ("first", XMLTOKEN_FIRST);
  xmltokens.Register ("secondlen", XMLTOKEN_SECONDLEN);
  xmltokens.Register ("second", XMLTOKEN_SECOND);
  xmltokens.Register ("matrix", XMLTOKEN_MATRIX);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("name", XMLTOKEN_NAME);
  xmltokens.Register ("uvec", XMLTOKEN_UVEC);
  xmltokens.Register ("vvec", XMLTOKEN_VVEC);
  return true;
}

csPtr<iBase> csPlaneLoader::Parse (const char* string, 
			     iLoaderContext* ldr_context, iBase* /*context*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (FIRST_LEN)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (SECOND_LEN)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (NAME)
    CS_TOKEN_TABLE (UVEC)
    CS_TOKEN_TABLE (VVEC)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;

  csParser* parser = ldr_context->GetParser ();

  // Things only work with the real 3D engine and not with the iso engine.
  csRef<iEngine> engine (CS_QUERY_REGISTRY (object_reg, iEngine));
  CS_ASSERT (engine != NULL);

  csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (engine->GetThingType (),
  	iThingEnvironment));
  csRef<iPolyTxtPlane> ppl (te->CreatePolyTxtPlane ());

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = 0, tx2_len = 0;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char name[255]; name[0] = 0;
  char* buf = (char*)string;

  while ((cmd = parser->GetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
	"crystalspace.planeloader.parse.badformat",
        "Expected parameters instead of '%s'!", buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_NAME:
        csScanStr (params, "%s", name);
	ppl->QueryObject()->SetName (name);
        break;
      case CS_TOKEN_ORIG:
        tx1_given = true;
        synldr->ParseVector (parser, params, tx_orig);
        break;
      case CS_TOKEN_FIRST:
        tx1_given = true;
        synldr->ParseVector (parser, params, tx1);
        break;
      case CS_TOKEN_FIRST_LEN:
        csScanStr (params, "%f", &tx1_len);
        tx1_given = true;
        break;
      case CS_TOKEN_SECOND:
        tx2_given = true;
        synldr->ParseVector (parser, params, tx2);
        break;
      case CS_TOKEN_SECOND_LEN:
        csScanStr (params, "%f", &tx2_len);
        tx2_given = true;
        break;
      case CS_TOKEN_MATRIX:
        synldr->ParseMatrix (parser, params, tx_matrix);
        break;
      case CS_TOKEN_V:
        synldr->ParseVector (parser, params, tx_vector);
        break;
      case CS_TOKEN_UVEC:
        tx1_given = true;
        synldr->ParseVector (parser, params, tx1);
        tx1_len = tx1.Norm ();
        tx1 += tx_orig;
        break;
      case CS_TOKEN_VVEC:
        tx2_given = true;
        synldr->ParseVector (parser, params, tx2);
        tx2_len = tx2.Norm ();
        tx2 += tx_orig;
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    ReportError (reporter,
	"crystalspace.planeloader.parse.badformat",
        "Token '%s' not found while parsing a plane!",
    	parser->GetLastOffender ());
    return NULL;
  }

  if (tx1_given)
    if (tx2_given)
    {
      if (!tx1_len)
      {
	ReportError (reporter,
	  "crystalspace.planeloader.parse.badplane",
          "Bad texture specification for PLANE '%s'", name);
	tx1_len = 1;
      }
      if (!tx2_len)
      {
	ReportError (reporter,
	  "crystalspace.planeloader.parse.badplane",
          "Bad texture specification for PLANE '%s'", name);
	tx2_len = 1;
      }
      if ((tx1-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else if ((tx2-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else ppl->SetTextureSpace (tx_orig, tx1, tx1_len, tx2, tx2_len);
    }
    else
    {
      ReportError (reporter,
	  "crystalspace.planeloader.parse.badplane",
          "Not supported!");
      return NULL;
    }
  else
    ppl->SetTextureSpace (tx_matrix, tx_vector);

  ppl->IncRef ();	// Prevent smart pointer from releasing.
  return csPtr<iBase> (ppl);
}

csPtr<iBase> csPlaneLoader::Parse (iDocumentNode* node,
			     iLoaderContext* /*ldr_context*/,
			     iBase* /*context*/)
{
  // Things only work with the real 3D engine and not with the iso engine.
  csRef<iEngine> engine (CS_QUERY_REGISTRY (object_reg, iEngine));
  CS_ASSERT (engine != NULL);

  csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (engine->GetThingType (),
  	iThingEnvironment));
  csRef<iPolyTxtPlane> ppl (te->CreatePolyTxtPlane ());

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = 0, tx2_len = 0;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char name[255]; name[0] = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_NAME:
	strcpy (name, child->GetContentsValue () ?
			child->GetContentsValue () : "");
	ppl->QueryObject()->SetName (name);
        break;
      case XMLTOKEN_ORIG:
        tx1_given = true;
        synldr->ParseVector (child, tx_orig);
        break;
      case XMLTOKEN_FIRST:
        tx1_given = true;
        synldr->ParseVector (child, tx1);
        break;
      case XMLTOKEN_FIRSTLEN:
	tx1_len = child->GetContentsValueAsFloat ();
        tx1_given = true;
        break;
      case XMLTOKEN_SECOND:
        tx2_given = true;
        synldr->ParseVector (child, tx2);
        break;
      case XMLTOKEN_SECONDLEN:
	tx2_len = child->GetContentsValueAsFloat ();
        tx2_given = true;
        break;
      case XMLTOKEN_MATRIX:
        synldr->ParseMatrix (child, tx_matrix);
        break;
      case XMLTOKEN_V:
        synldr->ParseVector (child, tx_vector);
        break;
      case XMLTOKEN_UVEC:
        tx1_given = true;
        synldr->ParseVector (child, tx1);
        tx1_len = tx1.Norm ();
        tx1 += tx_orig;
        break;
      case XMLTOKEN_VVEC:
        tx2_given = true;
        synldr->ParseVector (child, tx2);
        tx2_len = tx2.Norm ();
        tx2 += tx_orig;
        break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }

  if (tx1_given)
    if (tx2_given)
    {
      if (!tx1_len)
      {
	synldr->ReportError (
	  "crystalspace.planeloader.parse.badplane",
          node, "Bad texture specification for PLANE '%s'", name);
	tx1_len = 1;
      }
      if (!tx2_len)
      {
	synldr->ReportError (
	  "crystalspace.planeloader.parse.badplane",
          node, "Bad texture specification for PLANE '%s'", name);
	tx2_len = 1;
      }
      if ((tx1-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else if ((tx2-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else ppl->SetTextureSpace (tx_orig, tx1, tx1_len, tx2, tx2_len);
    }
    else
    {
      synldr->ReportError (
	  "crystalspace.planeloader.parse.badplane",
          node, "Not supported!");
      return NULL;
    }
  else
    ppl->SetTextureSpace (tx_matrix, tx_vector);

  ppl->IncRef ();	// Prevent smart pointer release.
  return csPtr<iBase> (ppl);
}

//---------------------------------------------------------------------------

csPlaneSaver::csPlaneSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csPlaneSaver::~csPlaneSaver ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (reporter);
}

bool csPlaneSaver::Initialize (iObjectRegistry* object_reg)
{
  csPlaneSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

void csPlaneSaver::WriteDown (iBase* /*obj*/, iFile* /*file*/)
{
}

//---------------------------------------------------------------------------

csBezierLoader::csBezierLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csBezierLoader::~csBezierLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (reporter);
}

bool csBezierLoader::Initialize (iObjectRegistry* object_reg)
{
  csBezierLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("name", XMLTOKEN_NAME);
  return true;
}

csPtr<iBase> csBezierLoader::Parse (const char* string, 
			      iLoaderContext* ldr_context, iBase* /*context*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (NAME)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char *xname;
  long cmd;
  int i;
  char *params;
  char name[100];

  csParser* parser = ldr_context->GetParser ();

  // Things only work with the real 3D engine and not with the iso engine.
  csRef<iEngine> engine (CS_QUERY_REGISTRY (object_reg, iEngine));
  CS_ASSERT (engine != NULL);

  csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (engine->GetThingType (),
	    iThingEnvironment));
  csRef<iCurveTemplate> tmpl (te->CreateBezierTemplate ());

  iMaterialWrapper* mat = NULL;
  char str[255];

  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
	  "crystalspace.bezierloader.parse.badformat",
          "Expected parameters instead of '%s'!", buf);
      engine->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_NAME:
        csScanStr (params, "%s", name);
	tmpl->QueryObject()->SetName (name);
        break;
      case CS_TOKEN_MATERIAL:
        csScanStr (params, "%s", str);
        mat = ldr_context->FindMaterial (str);

        if (mat == NULL)
        {
	  ReportError (reporter,
	    "crystalspace.bezierloader.parse.material",
            "Couldn't find material named '%s'!", str);
          engine->DecRef ();
          return NULL;
        }
        tmpl->SetMaterial (mat);
        break;
      case CS_TOKEN_V:
      case CS_TOKEN_VERTICES:
        {
          int list[100], num;
          csScanStr (params, "%D", list, &num);
          if (num != 9)
          {
	    ReportError (reporter,
	      "crystalspace.bezierloader.parse.vertices",
              "Wrong number of vertices to bezier! %d should be 9!", num);
            engine->DecRef ();
            return NULL;
          }
          for (i = 0 ; i < num ; i++) tmpl->SetVertex (i, list[i]);
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    ReportError (reporter,
	  "crystalspace.bezierloader.parse.badformat",
          "Token '%s' not found while parsing a bezier template!",
    	  parser->GetLastOffender ());
    return NULL;
  }
  tmpl->IncRef ();	// Prevent smart pointer from releasing.
  return csPtr<iBase> (tmpl);
}

csPtr<iBase> csBezierLoader::Parse (iDocumentNode* node,
			      iLoaderContext* ldr_context, iBase* /*context*/)
{
  // Things only work with the real 3D engine and not with the iso engine.
  csRef<iEngine> engine (CS_QUERY_REGISTRY (object_reg, iEngine));
  CS_ASSERT (engine != NULL);
  csRef<iSyntaxService> synldr (CS_QUERY_REGISTRY (object_reg, iSyntaxService));

  csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (engine->GetThingType (),
	    iThingEnvironment));
  csRef<iCurveTemplate> tmpl (te->CreateBezierTemplate ());

  iMaterialWrapper* mat = NULL;

  int num_v = 0;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_NAME:
	tmpl->QueryObject()->SetName (child->GetContentsValue ());
        break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          mat = ldr_context->FindMaterial (matname);
          if (mat == NULL)
          {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.material",
              child, "Couldn't find material named '%s'!", matname);
            return NULL;
          }
          tmpl->SetMaterial (mat);
	}
        break;
      case XMLTOKEN_V:
        {
          if (num_v >= 9)
          {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.vertices",
              child, "Wrong number of vertices to bezier! Should be 9!");
            return NULL;
          }
	  tmpl->SetVertex (num_v, child->GetContentsValueAsInt ());
	  num_v++;
        }
        break;
      default:
	synldr->ReportBadToken (child);
	return NULL;
    }
  }
  
  if (num_v != 9)
  {
    synldr->ReportError (
      "crystalspace.bezierloader.parse.vertices",
      node, "Wrong number of vertices to bezier! %d should be 9!", num_v);
    return NULL;
  }
  tmpl->IncRef ();	// Prevent smart pointer from releasing.
  return csPtr<iBase> (tmpl);
}

//---------------------------------------------------------------------------

csBezierSaver::csBezierSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csBezierSaver::~csBezierSaver ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (reporter);
}

bool csBezierSaver::Initialize (iObjectRegistry* object_reg)
{
  csBezierSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

void csBezierSaver::WriteDown (iBase* /*obj*/, iFile* /*file*/)
{
}
