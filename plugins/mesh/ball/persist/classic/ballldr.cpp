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
#include "csutil/cscolor.h"
#include "ballldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "imesh/ball.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "isys/plugin.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (TILING)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (LIGHTING)

  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (NUMRIM)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (SHIFT)
  CS_TOKEN_DEF (REVERSED)
  CS_TOKEN_DEF (TOPONLY)
  CS_TOKEN_DEF (CYLINDRICAL)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csBallFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallFactoryLoader::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBallFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallFactorySaver::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBallLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallLoader::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBallSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallSaver::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csBallFactoryLoader)
SCF_IMPLEMENT_FACTORY (csBallFactorySaver)
SCF_IMPLEMENT_FACTORY (csBallLoader)
SCF_IMPLEMENT_FACTORY (csBallSaver)

SCF_EXPORT_CLASS_TABLE (ballldr)
  SCF_EXPORT_CLASS (csBallFactoryLoader,
    "crystalspace.mesh.loader.factory.ball",
    "Crystal Space Ball Factory Loader")
  SCF_EXPORT_CLASS (csBallFactorySaver, "crystalspace.mesh.saver.factory.ball",
    "Crystal Space Ball Factory Saver")
  SCF_EXPORT_CLASS (csBallLoader, "crystalspace.mesh.loader.ball",
    "Crystal Space Ball Mesh Loader")
  SCF_EXPORT_CLASS (csBallSaver, "crystalspace.mesh.saver.ball",
    "Crystal Space Ball Mesh Saver")
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
    printf ("Error ID: %s\n", id);
    printf ("Description: %s\n", buf);
    fflush (stdout);
  }
  va_end (arg);
}

csBallFactoryLoader::csBallFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  reporter = NULL;
}

csBallFactoryLoader::~csBallFactoryLoader ()
{
  if (reporter) reporter->DecRef ();
}

bool csBallFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  object_reg = system->GetObjectRegistry ();
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_REPORTER, iReporter);
  return true;
}

iBase* csBallFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.ball", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.ball",
    	"MeshObj", iMeshObjectType);
  }
  if (!type)
  {
    ReportError (reporter,
		"crystalspace.ballfactoryloader.setup.objecttype",
		"Could not load the ball mesh object plugin!");
    return NULL;
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csBallFactorySaver::csBallFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  reporter = NULL;
}

csBallFactorySaver::~csBallFactorySaver ()
{
  if (reporter) reporter->DecRef ();
}

bool csBallFactorySaver::Initialize (iSystem* system)
{
  sys = system;
  object_reg = system->GetObjectRegistry ();
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_REPORTER, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csBallFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/,
  iEngine* /*engine*/)
{
  // no params
}

//---------------------------------------------------------------------------

csBallLoader::csBallLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  reporter = NULL;
}

csBallLoader::~csBallLoader ()
{
  if (reporter) reporter->DecRef ();
}

bool csBallLoader::Initialize (iSystem* system)
{
  sys = system;
  object_reg = system->GetObjectRegistry ();
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_REPORTER, iReporter);
  return true;
}

static UInt ParseMixmode (iReporter* reporter, char* buf)
{
  CS_TOKEN_TABLE_START (modes)
    CS_TOKEN_TABLE (COPY)
    CS_TOKEN_TABLE (MULTIPLY2)
    CS_TOKEN_TABLE (MULTIPLY)
    CS_TOKEN_TABLE (ADD)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (KEYCOLOR)
    CS_TOKEN_TABLE (TILING)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  UInt Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.ballloader.parse.mixmode.badformat",
		"Bad format while parsing mixmode!");
      return ~0;
    }
    switch (cmd)
    {
      case CS_TOKEN_TILING: Mixmode |= CS_FX_TILING; break;
      case CS_TOKEN_COPY: Mixmode |= CS_FX_COPY; break;
      case CS_TOKEN_MULTIPLY: Mixmode |= CS_FX_MULTIPLY; break;
      case CS_TOKEN_MULTIPLY2: Mixmode |= CS_FX_MULTIPLY2; break;
      case CS_TOKEN_ADD: Mixmode |= CS_FX_ADD; break;
      case CS_TOKEN_ALPHA:
	Mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
        csScanStr (params, "%f", &alpha);
	Mixmode |= CS_FX_SETALPHA(alpha);
	break;
      case CS_TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    ReportError (reporter,
		"crystalspace.ballloader.parse.mixmode.badtoken",
		"Token '%s' not found while parsing mixmodes!",
		csGetLastOffender ());
    return ~0;
  }
  return Mixmode;
}

iBase* csBallLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (NUMRIM)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (SHIFT)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (REVERSED)
    CS_TOKEN_TABLE (TOPONLY)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (CYLINDRICAL)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iBallState* ballstate = NULL;
  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.ballloader.parse.badformat",
		"Bad format while parsing ball object!");
      if (ballstate) ballstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_REVERSED:
	{
	  bool r;
	  csScanStr (params, "%b", &r);
	  ballstate->SetReversed (r);
	}
	break;
      case CS_TOKEN_TOPONLY:
	{
	  bool r;
	  csScanStr (params, "%b", &r);
	  ballstate->SetTopOnly (r);
	}
	break;
      case CS_TOKEN_CYLINDRICAL:
	{
	  bool r;
	  csScanStr (params, "%b", &r);
	  ballstate->SetCylindricalMapping (r);
	}
	break;
      case CS_TOKEN_LIGHTING:
	{
	  bool r;
	  csScanStr (params, "%b", &r);
	  ballstate->SetLighting (r);
	}
	break;
      case CS_TOKEN_COLOR:
	{
	  csColor col;
	  csScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  ballstate->SetColor (col);
	}
	break;
      case CS_TOKEN_RADIUS:
	{
	  float x, y, z;
	  csScanStr (params, "%f,%f,%f", &x, &y, &z);
	  ballstate->SetRadius (x, y, z);
	}
	break;
      case CS_TOKEN_SHIFT:
	{
	  float x, y, z;
	  csScanStr (params, "%f,%f,%f", &x, &y, &z);
	  ballstate->SetShift (x, y, z);
	}
	break;
      case CS_TOKEN_NUMRIM:
	{
	  int f;
	  csScanStr (params, "%d", &f);
	  ballstate->SetRimVertices (f);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
      	    ReportError (reporter,
		"crystalspace.ballloader.parse.unknownfactory",
		"Couldn't find factory '%s'!", str);
	    if (ballstate) ballstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
          ballstate = SCF_QUERY_INTERFACE (mesh, iBallState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
      	    ReportError (reporter,
		"crystalspace.ballloader.parse.unknownmaterial",
		"Couldn't find material '%s'!", str);
            mesh->DecRef ();
            return NULL;
	  }
	  ballstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	UInt mm = ParseMixmode (reporter, params);
	if (mm == (UInt)~0)
	{
	  if (ballstate) ballstate->DecRef ();
	  mesh->DecRef ();
	  return NULL;
	}
        ballstate->SetMixMode (mm);
	break;
    }
  }

  if (ballstate) ballstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csBallSaver::csBallSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  reporter = NULL;
}

csBallSaver::~csBallSaver ()
{
  if (reporter) reporter->DecRef ();
}

bool csBallSaver::Initialize (iSystem* system)
{
  sys = system;
  object_reg = system->GetObjectRegistry ();
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_REPORTER, iReporter);
  return true;
}

static void WriteMixmode(iStrVector *str, UInt mixmode)
{
  str->Push(csStrNew("  MIXMODE ("));
  if(mixmode&CS_FX_COPY) str->Push(csStrNew(" COPY ()"));
  if(mixmode&CS_FX_ADD) str->Push(csStrNew(" ADD ()"));
  if(mixmode&CS_FX_MULTIPLY) str->Push(csStrNew(" MULTIPLY ()"));
  if(mixmode&CS_FX_MULTIPLY2) str->Push(csStrNew(" MULTIPLY2 ()"));
  if(mixmode&CS_FX_KEYCOLOR) str->Push(csStrNew(" KEYCOLOR ()"));
  if(mixmode&CS_FX_TILING) str->Push(csStrNew(" TILING ()"));
  if(mixmode&CS_FX_TRANSPARENT) str->Push(csStrNew(" TRANSPARENT ()"));
  if(mixmode&CS_FX_ALPHA) {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(csStrNew(buf));
  }
  str->Push(csStrNew(")"));
}

void csBallSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iMeshObject *mesh = SCF_QUERY_INTERFACE(obj, iMeshObject);
  if(!mesh)
  {
    printf("Error: non-mesh given to %s.\n",
      fact->QueryDescription () );
    fact->DecRef();
    return;
  }
  iBallState *state = SCF_QUERY_INTERFACE(obj, iBallState);
  if(!state)
  {
    printf("Error: invalid mesh given to %s.\n",
      fact->QueryDescription () );
    fact->DecRef();
    mesh->DecRef();
    return;
  }

  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(csStrNew(buf));
  if(state->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, state->GetMixMode());
  }

  // Mesh information
  float x=0, y=0, z=0;
  state->GetRadius(x, y, z);
  sprintf(buf, "RADIUS (%g, %g, %g)\n", x, y, z);
  str->Push(csStrNew(buf));
  sprintf(buf, "SHIFT (%g, %g, %g)\n", state->GetShift ().x,
    state->GetShift ().y, state->GetShift ().z);
  str->Push(csStrNew(buf));
  sprintf(buf, "NUMRIM (%d)\n", state->GetRimVertices());
  str->Push(csStrNew(buf));
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str->Push(csStrNew(buf));
  if (!state->IsLighting ())
    str->Push (csStrNew ("LIGHTING (no)\n"));
  if (state->IsReversed ())
    str->Push (csStrNew ("REVERSED (yes)\n"));
  if (state->IsTopOnly ())
    str->Push (csStrNew ("TOPONLY (yes)\n"));
  if (state->IsCylindricalMapping ())
    str->Push (csStrNew ("CYLINDRICAL (yes)\n"));
  csColor col = state->GetColor ();
  sprintf(buf, "COLOR (%g,%g,%g)\n", col.red, col.green, col.blue);
  str->Push(csStrNew(buf));

  fact->DecRef();
  mesh->DecRef();
  state->DecRef();

}
