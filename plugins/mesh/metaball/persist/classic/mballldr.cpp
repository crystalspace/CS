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
#include "mballldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/metaball.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
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
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (ISO_LEVEL)
  CS_TOKEN_DEF (CHARGE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (TRUE_MAP)
  CS_TOKEN_DEF (TEX_SCALE)
  CS_TOKEN_DEF (RATE)

  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (SHIFT)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csMetaBallFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csMetaBallFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csMetaBallLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csMetaBallSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csMetaBallFactoryLoader)
SCF_IMPLEMENT_FACTORY (csMetaBallFactorySaver)
SCF_IMPLEMENT_FACTORY (csMetaBallLoader)
SCF_IMPLEMENT_FACTORY (csMetaBallSaver)

SCF_EXPORT_CLASS_TABLE (mballldr)
  SCF_EXPORT_CLASS (csMetaBallFactoryLoader, "crystalspace.mesh.loader.factory.metaball",
    "Crystal Space MetaBall Factory Loader")
  SCF_EXPORT_CLASS (csMetaBallFactorySaver, "crystalspace.mesh.saver.factory.metaball",
    "Crystal Space MetaBall Factory Saver")
  SCF_EXPORT_CLASS (csMetaBallLoader, "crystalspace.mesh.loader.metaball",
    "Crystal Space MetaBall Mesh Loader")
  SCF_EXPORT_CLASS (csMetaBallSaver, "crystalspace.mesh.saver.metaball",
    "Crystal Space MetaBall Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csMetaBallFactoryLoader::csMetaBallFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csMetaBallFactoryLoader::~csMetaBallFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csMetaBallFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csMetaBallFactoryLoader::Parse (const char* /*string*/,
	iLoaderContext* ldr_context, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.metaball", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.metaball",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.metaball\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csMetaBallFactorySaver::csMetaBallFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csMetaBallFactorySaver::~csMetaBallFactorySaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csMetaBallFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csMetaBallFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/)
{
  // no params
}

//---------------------------------------------------------------------------

csMetaBallLoader::csMetaBallLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csMetaBallLoader::~csMetaBallLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csMetaBallLoader::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallLoader::object_reg = object_reg;
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
    printf ("Token '%s' not found while parsing the modes!\n", csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

iBase* csMetaBallLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (LIGHTING)
	CS_TOKEN_TABLE (ISO_LEVEL)
	CS_TOKEN_TABLE (CHARGE)
	CS_TOKEN_TABLE (NUMBER)
	CS_TOKEN_TABLE (RATE)
	CS_TOKEN_TABLE (TRUE_MAP)
	CS_TOKEN_TABLE (TEX_SCALE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iMetaBallState* ballstate = NULL;
  MetaParameters* mp = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (ballstate) ballstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_ISO_LEVEL:
	{
	  if (!mp) { printf("Please set FACTORY before ISO_LEVEL\n"); return NULL; }
	  float f;
	  csScanStr (params, "%f", &f);
	  mp->iso_level = f;
	}
	break;
      case CS_TOKEN_CHARGE:
	{
	  if (!mp) { printf("Please set FACTORY before CHARGE\n"); return NULL; }
	  float f;
	  csScanStr (params, "%f", &f);
	  mp->charge = f;
	}
	break;
      case CS_TOKEN_NUMBER:
	{
	  if (!ballstate) { printf("Please set FACTORY before NUMBER\n"); return NULL; }
	  int r;
	  csScanStr (params, "%d", &r);
	  ballstate->SetMetaBallCount (r);
	}
	break;
      case CS_TOKEN_RATE:
	{
	  if (!mp) { printf("Please set FACTORY before RATE\n"); return NULL; }
	  float r;
	  csScanStr (params, "%f", &r);
	  mp->rate = r;
	}
	break;
      case CS_TOKEN_TRUE_MAP:
	{
	  if (!ballstate) { printf("Please set FACTORY before TRUE_MAP\n"); return NULL; }
	  bool m;
	  csScanStr (params, "%b", &m);
	  ballstate->SetQualityEnvironmentMapping (m);
	}
	break;
      case CS_TOKEN_TEX_SCALE:
	{
	  if (!ballstate) { printf("Please set FACTORY before TEX_SCALE\n"); return NULL; }
	  float s;
	  csScanStr (params, "%f", &s);
	  ballstate->SetEnvironmentMappingFactor(s);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
      csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
      ballstate = SCF_QUERY_INTERFACE (mesh, iMetaBallState);
	  mp = ballstate->GetParameters();
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
	  if (!ballstate) { printf("Please set FACTORY before MATERIAL\n"); return NULL; }
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            mesh->DecRef ();
            return NULL;
	  }
	  ballstate->SetMaterial(mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
		if (!ballstate) { printf("Please set FACTORY before MIXMODE\n"); return NULL; }
        ballstate->SetMixMode (ParseMixmode (params));
	break;
	  case CS_TOKEN_LIGHTING:
		if (!ballstate) { printf("Please set FACTORY before MIXMODE\n"); return NULL; }
		bool l;
		csScanStr(params, "%b", &l);
		ballstate->SetLighting(l);
	break;
    }
  }

  if (ballstate) ballstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csMetaBallSaver::csMetaBallSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csMetaBallSaver::~csMetaBallSaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csMetaBallSaver::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

static void WriteMixmode(iStrVector *str, uint mixmode)
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

void csMetaBallSaver::WriteDown (iBase* obj, iStrVector *str)
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
  iMetaBallState *state = SCF_QUERY_INTERFACE(obj, iMetaBallState);
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
  MetaParameters *mp = state->GetParameters();
  sprintf(buf, "NUMBER (%d)\n", state->GetMetaBallCount());
  str->Push(csStrNew(buf));
  sprintf(buf, "ISO_LEVEL (%f)\n",mp->iso_level );
  str->Push(csStrNew(buf));
  sprintf(buf, "CHARGE (%f)\n", mp->charge);
  str->Push(csStrNew(buf));
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterial()->
    QueryObject ()->GetName());
  str->Push(csStrNew(buf));
  sprintf(buf, "LIGHTING(%s)\n",(state->IsLighting())? "true" : "false");
  str->Push (csStrNew (buf));
  sprintf(buf, "NUMBER (%d)\n", state->GetMetaBallCount());
  str->Push(csStrNew(buf));
  sprintf(buf, "RATE (%f)\n",mp->rate);
  str->Push(csStrNew(buf));
  sprintf(buf, "TRUE_MAP (%s)\n",(state->GetQualityEnvironmentMapping())?"true":"false");
  str->Push(csStrNew(buf));
  sprintf(buf, "TEX_SCALE (%f)\n",state->GetEnvironmentMappingFactor());
  str->Push(csStrNew(buf));

  fact->DecRef();
  mesh->DecRef();
  state->DecRef();
}
