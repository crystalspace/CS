/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "cubeldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "imesh/cube.h"
#include "imap/services.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "csutil/util.h"
#include "iutil/strvec.h"
#include "iutil/plugin.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (SHIFT)
  CS_TOKEN_DEF (SIZE)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csCubeFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csCubeFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csCubeLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csCubeSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csCubeFactoryLoader)
SCF_IMPLEMENT_FACTORY (csCubeFactorySaver)
SCF_IMPLEMENT_FACTORY (csCubeLoader)
SCF_IMPLEMENT_FACTORY (csCubeSaver)

SCF_EXPORT_CLASS_TABLE (cubeldr)
  SCF_EXPORT_CLASS (csCubeFactoryLoader,
    "crystalspace.mesh.loader.factory.cube",
    "Crystal Space Cube Mesh Factory Loader")
  SCF_EXPORT_CLASS (csCubeFactorySaver, "crystalspace.mesh.saver.factory.cube",
    "Crystal Space Cube Mesh Factory Saver")
  SCF_EXPORT_CLASS (csCubeLoader, "crystalspace.mesh.loader.cube",
    "Crystal Space Cube Mesh Loader")
  SCF_EXPORT_CLASS (csCubeSaver, "crystalspace.mesh.saver.cube",
    "Crystal Space Cube Mesh Saver")
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

csCubeFactoryLoader::csCubeFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  synldr = NULL;
  reporter = NULL;
}

csCubeFactoryLoader::~csCubeFactoryLoader ()
{
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (reporter);
}

bool csCubeFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csCubeFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter) reporter->IncRef ();
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      ReportError (reporter,
	"crystalspace.cubeloader.parse.initialize",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      ReportError (reporter,
	"crystalspace.cubeloader.parse.initialize",
	"Could not register the syntax services!");
      return false;
    }
  }
  else synldr->IncRef ();

  return true;
}

iBase* csCubeFactoryLoader::Parse (const char* string, iEngine* engine,
	iBase* /* context */)
{
  // @@@ Implement MIXMODE
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (SHIFT)
    CS_TOKEN_TABLE (SIZE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.cube", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.cube",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.cube\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  iCubeFactoryState* cubeLook = SCF_QUERY_INTERFACE (fact, iCubeFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      cubeLook->DecRef ();
      fact->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->GetMaterialList ()->
	  	FindByName (str);
	  if (!mat)
	  {
            // @@@ Error handling!
	    cubeLook->DecRef ();
            fact->DecRef ();
            return NULL;
	  }
	  cubeLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	{
	  UInt mixmode;
	  if (!synldr->ParseMixmode (params, mixmode))
	  {
	    cubeLook->DecRef ();
            fact->DecRef ();
            return NULL;
	  }
	  cubeLook->SetMixMode (mixmode);
	}
	break;
      case CS_TOKEN_SHIFT:
	{
	  float shiftx, shifty, shiftz;
	  csScanStr (params, "%f,%f,%f", &shiftx, &shifty, &shiftz);
	  cubeLook->SetShift (shiftx, shifty, shiftz);
	}
	break;
      case CS_TOKEN_SIZE:
	{
	  float sizex, sizey, sizez;
	  csScanStr (params, "%f,%f,%f", &sizex, &sizey, &sizez);
	  cubeLook->SetSize (sizex, sizey, sizez);
	}
	break;
    }
  }

  cubeLook->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csCubeFactorySaver::csCubeFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  synldr = NULL;
  reporter = NULL;
}

csCubeFactorySaver::~csCubeFactorySaver ()
{
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (reporter);
}

bool csCubeFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csCubeFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter) reporter->IncRef ();
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      ReportError (reporter,
	"crystalspace.cubesaver.parse.initialize",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      ReportError (reporter,
	"crystalspace.cubesaver.parse.initialize",
	"Could not register the syntax services!");
      return false;
    }
  }
  else synldr->IncRef ();

  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csCubeFactorySaver::WriteDown (iBase* obj, iStrVector * str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iCubeFactoryState *cubelook = SCF_QUERY_INTERFACE(obj, iCubeFactoryState);
  if(!cubelook)
  {
    printf("Error: non-cubefactorystate given to %s.\n",
      fact->QueryDescription () );
    fact->DecRef();
    return;
  }

  if(cubelook->GetMixMode() != CS_FX_COPY)
    str->Push(csStrNew(synldr->MixmodeToText (cubelook->GetMixMode (), 0)));

  char buf[MAXLINE];
  sprintf(buf, "MATERIAL (%s)\n", cubelook->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str->Push(csStrNew(buf));
  sprintf(buf, "SIZE (%g, %g, %g)\n", cubelook->GetSizeX (),
    cubelook->GetSizeY (), cubelook->GetSizeZ ());
  str->Push(csStrNew(buf));
  sprintf(buf, "SHIFT (%g, %g, %g)\n", cubelook->GetShiftX (),
    cubelook->GetShiftY (), cubelook->GetShiftZ ());
  str->Push(csStrNew(buf));

  cubelook->DecRef();
  fact->DecRef();
}

//---------------------------------------------------------------------------

csCubeLoader::csCubeLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
}

csCubeLoader::~csCubeLoader ()
{
  SCF_DEC_REF (reporter);
}

bool csCubeLoader::Initialize (iObjectRegistry* object_reg)
{
  csCubeLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter) reporter->IncRef ();
  return true;
}

iBase* csCubeLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  iMeshObject* mesh = 0;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
        csScanStr (params, "%s", str);
	iMeshFactoryWrapper* fact = engine->GetMeshFactories ()
		->FindByName (str);
	if (!fact)
	{
	  // @@@ Error handling!
	  return NULL;
	}
	mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	imeshwrap->SetFactory (fact);
	break;
    }
  }

  return mesh;
}

//---------------------------------------------------------------------------

csCubeSaver::csCubeSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
}

csCubeSaver::~csCubeSaver ()
{
  SCF_DEC_REF (reporter);
}

bool csCubeSaver::Initialize (iObjectRegistry* object_reg)
{
  csCubeSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter) reporter->IncRef ();
  return true;
}

void csCubeSaver::WriteDown (iBase* /*obj*/, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(csStrNew(buf));
  fact->DecRef();
}
