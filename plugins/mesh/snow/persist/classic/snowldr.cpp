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
#include "snowldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/snow.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "iutil/vfs.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FALLSPEED)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (BOX)
  CS_TOKEN_DEF (SWIRL)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csSnowFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSnowFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSnowLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSnowSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSnowFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSnowFactorySaver)
SCF_IMPLEMENT_FACTORY (csSnowLoader)
SCF_IMPLEMENT_FACTORY (csSnowSaver)

SCF_EXPORT_CLASS_TABLE (snowldr)
  SCF_EXPORT_CLASS (csSnowFactoryLoader,
    "crystalspace.mesh.loader.factory.snow",
    "Crystal Space Snow Factory Loader")
  SCF_EXPORT_CLASS (csSnowFactorySaver, "crystalspace.mesh.saver.factory.snow",
    "Crystal Space Snow Factory Saver")
  SCF_EXPORT_CLASS (csSnowLoader, "crystalspace.mesh.loader.snow",
    "Crystal Space Snow Mesh Loader")
  SCF_EXPORT_CLASS (csSnowSaver, "crystalspace.mesh.saver.snow",
    "Crystal Space Snow Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csSnowFactoryLoader::csSnowFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csSnowFactoryLoader::~csSnowFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csSnowFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSnowFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csSnowFactoryLoader::Parse (const char* /*string*/,
	iLoaderContext*, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.snow", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.snow",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.snow\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csSnowFactorySaver::csSnowFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csSnowFactorySaver::~csSnowFactorySaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csSnowFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSnowFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csSnowFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // nothing to do
}

//---------------------------------------------------------------------------
csSnowLoader::csSnowLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csSnowLoader::~csSnowLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
}

bool csSnowLoader::Initialize (iObjectRegistry* object_reg)
{
  csSnowLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

iBase* csSnowLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (FALLSPEED)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (SWIRL)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iParticleState* partstate = NULL;
  iSnowState* snowstate = NULL;

  csParser* parser = ldr_context->GetParser ();

  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (snowstate) snowstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_COLOR:
	{
	  csColor color;
	  csScanStr(params, "%f,%f,%f", &color.red, &color.green, &color.blue);
	  partstate->SetColor (color);
	}
	break;
      case CS_TOKEN_DROPSIZE:
	{
	  float dw, dh;
	  csScanStr (params, "%f,%f", &dw, &dh);
	  snowstate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_BOX:
	{
	  csVector3 minbox, maxbox;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	      	&minbox.x, &minbox.y, &minbox.z,
	      	&maxbox.x, &maxbox.y, &maxbox.z);
	  snowstate->SetBox (minbox, maxbox);
	}
	break;
      case CS_TOKEN_FALLSPEED:
	{
	  csVector3 s;
	  csScanStr (params, "%f,%f,%f", &s.x, &s.y, &s.z);
	  snowstate->SetFallSpeed (s);
	}
	break;
      case CS_TOKEN_SWIRL:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  snowstate->SetSwirl (f);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (partstate) partstate->DecRef ();
	    if (snowstate) snowstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          snowstate = SCF_QUERY_INTERFACE (mesh, iSnowState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            mesh->DecRef ();
	    if (partstate) partstate->DecRef ();
	    if (snowstate) snowstate->DecRef ();
            return NULL;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	uint mode;
	if (synldr->ParseMixmode (parser, params, mode))
          partstate->SetMixMode (mode);
	break;
      case CS_TOKEN_LIGHTING:
        {
          bool do_lighting;
          csScanStr (params, "%b", &do_lighting);
          snowstate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          csScanStr (params, "%d", &nr);
          snowstate->SetParticleCount (nr);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (snowstate) snowstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csSnowSaver::csSnowSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csSnowSaver::~csSnowSaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csSnowSaver::Initialize (iObjectRegistry* object_reg)
{
  csSnowSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csSnowSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  iSnowState *state = SCF_QUERY_INTERFACE (obj, iSnowState);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append(buf);

  if(partstate->GetMixMode() != CS_FX_COPY)
  {
    str.Append (synldr->MixmodeToText (partstate->GetMixMode(), 0, true));
  }

  sprintf(buf, "MATERIAL (%s)\n", partstate->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str.Append(buf);
  sprintf(buf, "COLOR (%g, %g, %g)\n", partstate->GetColor().red,
    partstate->GetColor().green, partstate->GetColor().blue);
  str.Append(buf);
  printf(buf, "NUMBER (%d)\n", state->GetParticleCount());
  str.Append(buf);
  sprintf(buf, "LIGHTING (%s)\n", state->GetLighting()?"true":"false");
  str.Append(buf);
  float sx = 0.0, sy = 0.0;
  state->GetDropSize(sx, sy);
  sprintf(buf, "DROPSIZE (%g, %g)\n", sx, sy);
  str.Append(buf);
  printf(buf, "FALLSPEED (%g, %g, %g)\n", state->GetFallSpeed().x,
    state->GetFallSpeed().y, state->GetFallSpeed().z);
  str.Append(buf);
  csVector3 minbox, maxbox;
  state->GetBox(minbox, maxbox);
  printf(buf, "BOX (%g,%g,%g, %g,%g,%g)\n", minbox.x, minbox.y, minbox.z,
    maxbox.x, maxbox.y, maxbox.z);
  printf(buf, "SWIRL (%d)\n", state->GetSwirl());
  str.Append(buf);

  fact->DecRef();
  partstate->DecRef();
  state->DecRef();
  file->Write ((const char*)str, str.Length ());
}
