/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "cssys/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "starldr.h"
#include "imesh/object.h"
#include "imesh/stars.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (BOX)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (MAXCOLOR)
  CS_TOKEN_DEF (DENSITY)
  CS_TOKEN_DEF (MAXDISTANCE)
  CS_TOKEN_DEF (FACTORY)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csStarFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csStarFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csStarLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csStarSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csStarFactoryLoader)
SCF_IMPLEMENT_FACTORY (csStarFactorySaver)
SCF_IMPLEMENT_FACTORY (csStarLoader)
SCF_IMPLEMENT_FACTORY (csStarSaver)

SCF_EXPORT_CLASS_TABLE (starldr)
  SCF_EXPORT_CLASS (csStarFactoryLoader,
    "crystalspace.mesh.loader.factory.stars",
    "Crystal Space Star Factory Loader")
  SCF_EXPORT_CLASS (csStarFactorySaver, "crystalspace.mesh.saver.factory.stars",
    "Crystal Space Star Factory Saver")
  SCF_EXPORT_CLASS (csStarLoader, "crystalspace.mesh.loader.stars",
		    "Crystal Space Star Mesh Loader")
  SCF_EXPORT_CLASS (csStarSaver, "crystalspace.mesh.saver.stars",
    "Crystal Space Star Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

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

csStarFactoryLoader::csStarFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csStarFactoryLoader::~csStarFactoryLoader ()
{
  if (reporter) reporter->DecRef ();
  if (plugin_mgr) plugin_mgr->DecRef ();
}

bool csStarFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csStarFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

iBase* csStarFactoryLoader::Parse (const char* /*string*/,
	iMaterialList*, iMeshFactoryList*, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.stars", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.stars",
    	iMeshObjectType);
  }
  if (!type)
  {
    ReportError (reporter,
		"crystalspace.starfactoryloader.setup.objecttype",
		"Could not load the stars mesh object plugin!");
    return NULL;
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csStarFactorySaver::csStarFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csStarFactorySaver::~csStarFactorySaver ()
{
  if (reporter) reporter->DecRef ();
  if (plugin_mgr) plugin_mgr->DecRef ();
}

bool csStarFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csStarFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csStarFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/)
{
  // no params
}

//---------------------------------------------------------------------------

csStarLoader::csStarLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  synldr = NULL;
  plugin_mgr = NULL;
}

csStarLoader::~csStarLoader ()
{
  SCF_DEC_REF (reporter);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (plugin_mgr);
}

bool csStarLoader::Initialize (iObjectRegistry* object_reg)
{
  csStarLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      ReportError (reporter,
	"crystalspace.starloader.parse.initialize",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      ReportError (reporter,
	"crystalspace.starloader.parse.initialize",
	"Could not register the syntax services!");
      return false;
    }
  }
  return true;
}

iBase* csStarLoader::Parse (const char* string, iMaterialList* matlist,
	iMeshFactoryList* factlist,
	iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (MAXCOLOR)
    CS_TOKEN_TABLE (DENSITY)
    CS_TOKEN_TABLE (MAXDISTANCE)
    CS_TOKEN_TABLE (FACTORY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iStarsState* starstate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.starloader.parse.badformat",
		"Bad format while parsing star object!");
      if (starstate) starstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_BOX:
	{
	  csVector3 v1, v2;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	    &v1.x, &v1.y, &v1.z,
	    &v2.x, &v2.y, &v2.z);
	  starstate->SetBox (csBox3 (v1, v2));
	}
	break;
      case CS_TOKEN_COLOR:
	{
	  csColor col;
	  csScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  starstate->SetColor (col);
	}
	break;
      case CS_TOKEN_MAXCOLOR:
	{
	  csColor col;
	  csScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  starstate->SetMaxColor (col);
	}
	break;
      case CS_TOKEN_DENSITY:
	{
	  float d;
	  csScanStr (params, "%f", &d);
	  starstate->SetDensity (d);
	}
	break;
      case CS_TOKEN_MAXDISTANCE:
	{
	  float d;
	  csScanStr (params, "%f", &d);
	  starstate->SetMaxDistance (d);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = factlist->FindByName (str);
	  if (!fact)
	  {
      	    ReportError (reporter,
		"crystalspace.starloader.parse.unknownfactory",
		"Couldn't find factory '%s'!", str);
	    if (starstate) starstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          starstate = SCF_QUERY_INTERFACE (mesh, iStarsState);
	}
	break;
    }
  }

  if (starstate) starstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csStarSaver::csStarSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  synldr = NULL;
  plugin_mgr = NULL;
}

csStarSaver::~csStarSaver ()
{
  SCF_DEC_REF (reporter);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (plugin_mgr);
}

bool csStarSaver::Initialize (iObjectRegistry* object_reg)
{
  csStarSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      ReportError (reporter,
	"crystalspace.starsaver.parse.initialize",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      ReportError (reporter,
	"crystalspace.starsaver.parse.initialize",
	"Could not register the syntax services!");
      return false;
    }
  }
  return true;
}

void csStarSaver::WriteDown (iBase* /*obj*/, iStrVector* /*str*/)
{
  // @@@ Not implemented yet.
}
