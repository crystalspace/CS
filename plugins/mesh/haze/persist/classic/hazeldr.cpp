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
#include "hazeldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/haze.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (DIRECTIONAL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (HAZEBOX)
  CS_TOKEN_DEF (HAZECONE)
  CS_TOKEN_DEF (LAYER)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (SCALE)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csHazeFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csHazeFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csHazeLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csHazeSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csHazeFactoryLoader)
SCF_IMPLEMENT_FACTORY (csHazeFactorySaver)
SCF_IMPLEMENT_FACTORY (csHazeLoader)
SCF_IMPLEMENT_FACTORY (csHazeSaver)

SCF_EXPORT_CLASS_TABLE (hazeldr)
  SCF_EXPORT_CLASS (csHazeFactoryLoader,
    "crystalspace.mesh.loader.factory.haze",
    "Crystal Space Haze Factory Loader")
  SCF_EXPORT_CLASS (csHazeFactorySaver, "crystalspace.mesh.saver.factory.haze",
    "Crystal Space Haze Factory Saver")
  SCF_EXPORT_CLASS (csHazeLoader, "crystalspace.mesh.loader.haze",
    "Crystal Space Haze Mesh Loader")
  SCF_EXPORT_CLASS (csHazeSaver, "crystalspace.mesh.saver.haze",
    "Crystal Space Haze Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csHazeFactoryLoader::csHazeFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csHazeFactoryLoader::~csHazeFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
}

bool csHazeFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csHazeFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

static iHazeHull* ParseHull (csParser* parser, char* buf, 
			     iHazeFactoryState *fstate, float &s)
{
  CS_TOKEN_TABLE_START (emits)
    CS_TOKEN_TABLE (HAZEBOX)
    CS_TOKEN_TABLE (HAZECONE)
    CS_TOKEN_TABLE (SCALE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  iHazeHull* result = 0;
  csVector3 a,b;
  int number;
  float p,q;

  iHazeHullCreation *hullcreate = SCF_QUERY_INTERFACE(fstate,
    iHazeHullCreation);

  while ((cmd = parser->GetObject (&buf, emits, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return 0;
    }
    switch (cmd)
    {
      case CS_TOKEN_HAZEBOX:
        {
          csScanStr (params, "%f,%f,%f,%f,%f,%f", &a.x, &a.y, &a.z,
	    &b.x, &b.y, &b.z);
	  iHazeHullBox *ebox = hullcreate->CreateBox(a, b);
	  result = SCF_QUERY_INTERFACE(ebox, iHazeHull);
	  CS_ASSERT(result);
          ebox->DecRef();
	}
	break;
      case CS_TOKEN_HAZECONE:
        {
          csScanStr (params, "%d, %f,%f,%f,%f,%f,%f, %f, %f", &number,
	    &a.x, &a.y, &a.z, &b.x, &b.y, &b.z, &p, &q);
	  iHazeHullCone *econe = hullcreate->CreateCone(number, a, b, p, q);
	  result = SCF_QUERY_INTERFACE(econe, iHazeHull);
	  CS_ASSERT(result);
          econe->DecRef();
	}
	break;
      case CS_TOKEN_SCALE:
        {
          csScanStr (params, "%f", &s);
	}
	break;
    }
  }
  if(hullcreate) hullcreate->DecRef();
  return result;
}

iBase* csHazeFactoryLoader::Parse (const char* string,
	iLoaderContext* ldr_context,
	iBase* /* context */)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (DIRECTIONAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (LAYER)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (ORIGIN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  csVector3 a;

  csParser* parser = ldr_context->GetParser ();

  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.haze", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.haze",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.haze\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  iHazeFactoryState* hazefactorystate = SCF_QUERY_INTERFACE(
    fact, iHazeFactoryState);
  CS_ASSERT(hazefactorystate);

  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (hazefactorystate) hazefactorystate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
	    printf ("Could not find material '%s'!\n", str);
            // @@@ Error handling!
	    if (hazefactorystate) hazefactorystate->DecRef ();
            return NULL;
	  }
	  hazefactorystate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	uint mode;
	if (synldr->ParseMixmode (parser, params, mode))
          hazefactorystate->SetMixMode (mode);
	break;
      case CS_TOKEN_ORIGIN:
        {
          csScanStr (params, "%f,%f,%f", &a.x, &a.y, &a.z);
          hazefactorystate->SetOrigin (a);
	}
	break;
      case CS_TOKEN_DIRECTIONAL:
        {
          csScanStr (params, "%f,%f,%f", &a.x, &a.y, &a.z);
          hazefactorystate->SetDirectional (a);
	}
	break;
      case CS_TOKEN_LAYER:
        {
	  float layerscale = 1.0;
	  iHazeHull *hull = ParseHull(parser, params, hazefactorystate, layerscale);
          hazefactorystate->AddLayer (hull, layerscale);
	}
	break;
    }
  }

  if (hazefactorystate) hazefactorystate->DecRef ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csHazeFactorySaver::csHazeFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csHazeFactorySaver::~csHazeFactorySaver ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
}

bool csHazeFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csHazeFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

/// write hull to string
static void WriteHull(csString& str, iHazeHull *hull)
{
  char buf[MAXLINE];
  csVector3 a,b;
  int nr;
  float p,q;
  iHazeHullBox *ebox = SCF_QUERY_INTERFACE(hull, iHazeHullBox);
  if(ebox)
  {
    ebox->GetSettings(a, b);
    sprintf(buf, "  HAZEBOX(%g,%g,%g, %g,%g,%g)\n", a.x,a.y,a.z, b.x,b.y,b.z);
    str.Append(buf);
    ebox->DecRef();
    return;
  }
  iHazeHullCone *econe = SCF_QUERY_INTERFACE(hull, iHazeHullCone);
  if(econe)
  {
    econe->GetSettings(nr, a, b, p, q);
    sprintf(buf, "  HAZEBOX(%d, %g,%g,%g, %g,%g,%g, %g, %g)\n", nr,
      a.x,a.y,a.z, b.x,b.y,b.z, p, q);
    str.Append(buf);
    econe->DecRef();
    return;
  }
  printf ("Unknown hazehull type, cannot writedown!\n");
}

void csHazeFactorySaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  iHazeFactoryState *hazestate = SCF_QUERY_INTERFACE (obj, iHazeFactoryState);
  char buf[MAXLINE];

  if(hazestate->GetMixMode() != CS_FX_COPY)
  {
    str.Append (synldr->MixmodeToText (hazestate->GetMixMode(), 0, true));
  }
  sprintf(buf, "MATERIAL (%s)\n", hazestate->GetMaterialWrapper()->
    QueryObject()->GetName());
  str.Append(buf);

  sprintf(buf, "ORIGIN (%f,%f,%f)\n", hazestate->GetOrigin().x,
    hazestate->GetOrigin().y, hazestate->GetOrigin().z);
  str.Append(buf);
  sprintf(buf, "DIRECTIONAL (%f,%f,%f)\n", hazestate->GetDirectional().x,
    hazestate->GetDirectional().y, hazestate->GetDirectional().z);
  str.Append(buf);

  int i;
  for(i=0; i<hazestate->GetLayerCount(); i++)
  {
    sprintf(buf, "LAYER (%f ", hazestate->GetLayerScale(i) );
    WriteHull(str, hazestate->GetLayerHull(i));
    str.Append(" )\n");
  }

  if(hazestate) hazestate->DecRef();

  file->Write ((const char*)str, str.Length ());
}

//---------------------------------------------------------------------------

csHazeLoader::csHazeLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csHazeLoader::~csHazeLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
}

bool csHazeLoader::Initialize (iObjectRegistry* object_reg)
{
  csHazeLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

iBase* csHazeLoader::Parse (const char* string, 
			    iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (DIRECTIONAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (LAYER)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (ORIGIN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iHazeFactoryState* hazefactorystate = NULL;
  iHazeState* hazestate = NULL;
  csVector3 a;

  csParser* parser = ldr_context->GetParser ();

  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (hazestate) hazestate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (hazestate) hazestate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          hazestate = SCF_QUERY_INTERFACE (mesh, iHazeState);
	  hazefactorystate = SCF_QUERY_INTERFACE(fact->GetMeshObjectFactory(),
	    iHazeFactoryState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
	    printf ("Could not find material '%s'!\n", str);
            // @@@ Error handling!
            mesh->DecRef ();
	    if (hazestate) hazestate->DecRef ();
            return NULL;
	  }
	  hazestate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	uint mode;
	if (synldr->ParseMixmode (parser, params, mode))
          hazestate->SetMixMode (mode);
	break;
      case CS_TOKEN_ORIGIN:
        {
          csScanStr (params, "%f,%f,%f", &a.x, &a.y, &a.z);
          hazestate->SetOrigin (a);
	}
	break;
      case CS_TOKEN_DIRECTIONAL:
        {
          csScanStr (params, "%f,%f,%f", &a.x, &a.y, &a.z);
          hazestate->SetDirectional (a);
	}
	break;
      case CS_TOKEN_LAYER:
        {
	  float layerscale = 1.0;
	  iHazeHull *hull = ParseHull(parser, params, hazefactorystate, layerscale);
          hazestate->AddLayer (hull, layerscale);
	}
	break;
    }
  }

  if (hazestate) hazestate->DecRef ();
  if (hazefactorystate) hazefactorystate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csHazeSaver::csHazeSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csHazeSaver::~csHazeSaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csHazeSaver::Initialize (iObjectRegistry* object_reg)
{
  csHazeSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csHazeSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iHazeState *state = SCF_QUERY_INTERFACE (obj, iHazeState);
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append(buf);

  if(state->GetMixMode() != CS_FX_COPY)
  {
    str.Append (synldr->MixmodeToText (state->GetMixMode(), 0, true));
  }
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    QueryObject()->GetName());
  str.Append(buf);

  sprintf(buf, "ORIGIN (%f,%f,%f)\n", state->GetOrigin().x,
    state->GetOrigin().y, state->GetOrigin().z);
  str.Append(buf);
  sprintf(buf, "DIRECTIONAL (%f,%f,%f)\n", state->GetDirectional().x,
    state->GetDirectional().y, state->GetDirectional().z);
  str.Append(buf);

  int i;
  for(i=0; i<state->GetLayerCount(); i++)
  {
    sprintf(buf, "LAYER (%f ", state->GetLayerScale(i) );
    WriteHull(str, state->GetLayerHull(i));
    str.Append(" )\n");
  }

  fact->DecRef();
  state->DecRef();
  file->Write ((const char*)str, str.Length ());
}
