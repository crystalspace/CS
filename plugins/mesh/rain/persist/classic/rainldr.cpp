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
#include "rainldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/rain.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iutil/vfs.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"

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

  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FALLSPEED)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (BOX)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csRainFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csRainFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csRainLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csRainSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csRainFactoryLoader)
SCF_IMPLEMENT_FACTORY (csRainFactorySaver)
SCF_IMPLEMENT_FACTORY (csRainLoader)
SCF_IMPLEMENT_FACTORY (csRainSaver)

SCF_EXPORT_CLASS_TABLE (rainldr)
  SCF_EXPORT_CLASS (csRainFactoryLoader,
    "crystalspace.mesh.loader.factory.rain",
    "Crystal Space Rain Factory Loader")
  SCF_EXPORT_CLASS (csRainFactorySaver, "crystalspace.mesh.saver.factory.rain",
    "Crystal Space Rain Factory Saver")
  SCF_EXPORT_CLASS (csRainLoader, "crystalspace.mesh.loader.rain",
    "Crystal Space Rain Mesh Loader")
  SCF_EXPORT_CLASS (csRainSaver, "crystalspace.mesh.saver.rain",
    "Crystal Space Rain Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csRainFactoryLoader::csRainFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csRainFactoryLoader::~csRainFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csRainFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csRainFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csRainFactoryLoader::Parse (const char* /*string*/,
	iLoaderContext*, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.rain", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.rain",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.rain\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------
csRainFactorySaver::csRainFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csRainFactorySaver::~csRainFactorySaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csRainFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csRainFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

static void WriteMixmode(csString &str, uint mixmode)
{
  str.Append("  MIXMODE (");
  if(mixmode&CS_FX_COPY) str.Append(" COPY ()");
  if(mixmode&CS_FX_ADD) str.Append(" ADD ()");
  if(mixmode&CS_FX_MULTIPLY) str.Append(" MULTIPLY ()");
  if(mixmode&CS_FX_MULTIPLY2) str.Append(" MULTIPLY2 ()");
  if(mixmode&CS_FX_KEYCOLOR) str.Append(" KEYCOLOR ()");
  if(mixmode&CS_FX_TILING) str.Append(" TILING ()");
  if(mixmode&CS_FX_TRANSPARENT) str.Append(" TRANSPARENT ()");
  if(mixmode&CS_FX_ALPHA)
  {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str.Append(buf);
  }
  str.Append(")");
}

void csRainFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // nothing to do
}
//---------------------------------------------------------------------------

csRainLoader::csRainLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csRainLoader::~csRainLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csRainLoader::Initialize (iObjectRegistry* object_reg)
{
  csRainLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

static uint ParseMixmode (char* buf)
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

  uint Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return 0;
    }
    switch (cmd)
    {
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
      case CS_TOKEN_TILING: Mixmode |= CS_FX_TILING; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing the modes!\n",
    	csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

iBase* csRainLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (FALLSPEED)
    CS_TOKEN_TABLE (BOX)
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
  iRainState* rainstate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (rainstate) rainstate->DecRef ();
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
	  rainstate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_BOX:
	{
	  csVector3 minbox, maxbox;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	      	&minbox.x, &minbox.y, &minbox.z,
	      	&maxbox.x, &maxbox.y, &maxbox.z);
	  rainstate->SetBox (minbox, maxbox);
	}
	break;
      case CS_TOKEN_FALLSPEED:
	{
	  csVector3 s;
	  csScanStr (params, "%f,%f,%f", &s.x, &s.y, &s.z);
	  rainstate->SetFallSpeed (s);
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
	    if (rainstate) rainstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          rainstate = SCF_QUERY_INTERFACE (mesh, iRainState);
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
	    if (rainstate) rainstate->DecRef ();
            return NULL;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        partstate->SetMixMode (ParseMixmode (params));
	break;
      case CS_TOKEN_LIGHTING:
        {
          bool do_lighting;
          csScanStr (params, "%b", &do_lighting);
          rainstate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          csScanStr (params, "%d", &nr);
          rainstate->SetParticleCount (nr);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (rainstate) rainstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csRainSaver::csRainSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csRainSaver::~csRainSaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csRainSaver::Initialize (iObjectRegistry* object_reg)
{
  csRainSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csRainSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  iRainState *state = SCF_QUERY_INTERFACE (obj, iRainState);
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append(buf);

  if(partstate->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, partstate->GetMixMode());
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

  fact->DecRef();
  partstate->DecRef();
  state->DecRef();
  file->Write ((const char*)str, str.Length ());
}
