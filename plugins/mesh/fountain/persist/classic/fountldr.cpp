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
#include "fountldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

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

  CS_TOKEN_DEF (ACCEL)
  CS_TOKEN_DEF (AZIMUTH)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (ELEVATION)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FALLTIME)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (OPENING)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (SPEED)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csFountainFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFountainFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFountainLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFountainSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFountainFactoryLoader)
SCF_IMPLEMENT_FACTORY (csFountainFactorySaver)
SCF_IMPLEMENT_FACTORY (csFountainLoader)
SCF_IMPLEMENT_FACTORY (csFountainSaver)

SCF_EXPORT_CLASS_TABLE (fountldr)
  SCF_EXPORT_CLASS (csFountainFactoryLoader,
  	"crystalspace.mesh.loader.factory.fountain",
	"Crystal Space Fountain Factory Loader")
  SCF_EXPORT_CLASS (csFountainFactorySaver,
  	"crystalspace.mesh.saver.factory.fountain",
	"Crystal Space Fountain Factory Saver")
  SCF_EXPORT_CLASS (csFountainLoader, "crystalspace.mesh.loader.fountain",
    "Crystal Space Fountain Mesh Loader")
  SCF_EXPORT_CLASS (csFountainSaver, "crystalspace.mesh.saver.fountain",
    "Crystal Space Fountain Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csFountainFactoryLoader::csFountainFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csFountainFactoryLoader::~csFountainFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csFountainFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csFountainFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csFountainFactoryLoader::Parse (const char* /*string*/,
	iMaterialList*, iMeshFactoryList*, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.fountain", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.fountain",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.fountain\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csFountainFactorySaver::csFountainFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csFountainFactorySaver::~csFountainFactorySaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csFountainFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csFountainFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

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
  if(mixmode&CS_FX_ALPHA)
  {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(csStrNew(buf));
  }
  str->Push(csStrNew(")"));
}


void csFountainFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/)
{
  // nothing to do
}

//---------------------------------------------------------------------------

csFountainLoader::csFountainLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csFountainLoader::~csFountainLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csFountainLoader::Initialize (iObjectRegistry* object_reg)
{
  csFountainLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

static UInt ParseMixmode (char* buf)
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

iBase* csFountainLoader::Parse (const char* string, iMaterialList* matlist,
	iMeshFactoryList* factlist, iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (ACCEL)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (FALLTIME)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (OPENING)
    CS_TOKEN_TABLE (AZIMUTH)
    CS_TOKEN_TABLE (ELEVATION)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  iMeshObject* mesh = NULL;
  iParticleState* partstate = NULL;
  iFountainState* fountstate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (fountstate) fountstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_COLOR:
	{
	  csColor color;
	  csScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
	  partstate->SetColor (color);
	}
	break;
      case CS_TOKEN_DROPSIZE:
	{
	  float dw, dh;
	  csScanStr (params, "%f,%f", &dw, &dh);
	  fountstate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_ORIGIN:
	{
	  csVector3 origin;
	  csScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
	  fountstate->SetOrigin (origin);
	}
	break;
      case CS_TOKEN_ACCEL:
	{
	  csVector3 accel;
	  csScanStr (params, "%f,%f,%f", &accel.x, &accel.y, &accel.z);
	  fountstate->SetAcceleration (accel);
	}
	break;
      case CS_TOKEN_SPEED:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetSpeed (f);
	}
	break;
      case CS_TOKEN_OPENING:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetOpening (f);
	}
	break;
      case CS_TOKEN_AZIMUTH:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetAzimuth (f);
	}
	break;
      case CS_TOKEN_ELEVATION:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetElevation (f);
	}
	break;
      case CS_TOKEN_FALLTIME:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetFallTime (f);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = factlist->FindByName (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (partstate) partstate->DecRef ();
	    if (fountstate) fountstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          fountstate = SCF_QUERY_INTERFACE (mesh, iFountainState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = matlist->FindByName (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            mesh->DecRef ();
	    if (partstate) partstate->DecRef ();
	    if (fountstate) fountstate->DecRef ();
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
          fountstate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          csScanStr (params, "%d", &nr);
          fountstate->SetParticleCount (nr);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (fountstate) fountstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csFountainSaver::csFountainSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csFountainSaver::~csFountainSaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csFountainSaver::Initialize (iObjectRegistry* object_reg)
{
  csFountainSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csFountainSaver::WriteDown (iBase* obj, iStrVector *str)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  iFountainState *state = SCF_QUERY_INTERFACE (obj, iFountainState);
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(csStrNew(buf));

  if(partstate->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, partstate->GetMixMode());
  }
  sprintf(buf, "MATERIAL (%s)\n", partstate->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str->Push(csStrNew(buf));
  sprintf(buf, "COLOR (%g, %g, %g)\n", partstate->GetColor().red,
    partstate->GetColor().green, partstate->GetColor().blue);
  str->Push(csStrNew(buf));
  printf(buf, "NUMBER (%d)\n", state->GetParticleCount());
  str->Push(csStrNew(buf));
  sprintf(buf, "LIGHTING (%s)\n", state->GetLighting()?"true":"false");
  str->Push(csStrNew(buf));
  sprintf(buf, "ORIGIN (%g, %g, %g)\n", state->GetOrigin().x,
    state->GetOrigin().y, state->GetOrigin().z);
  str->Push(csStrNew(buf));
  float sx = 0.0, sy = 0.0;
  state->GetDropSize(sx, sy);
  sprintf(buf, "DROPSIZE (%g, %g)\n", sx, sy);
  str->Push(csStrNew(buf));
  sprintf(buf, "ACCEL (%g, %g, %g)\n", state->GetAcceleration().x,
    state->GetAcceleration().y, state->GetAcceleration().z);
  str->Push(csStrNew(buf));
  sprintf(buf, "SPEED (%g)\n", state->GetSpeed());
  str->Push(csStrNew(buf));
  sprintf(buf, "OPENING (%g)\n", state->GetOpening());
  str->Push(csStrNew(buf));
  sprintf(buf, "AZIMUTH (%g)\n", state->GetAzimuth());
  str->Push(csStrNew(buf));
  sprintf(buf, "ELEVATION (%g)\n", state->GetElevation());
  str->Push(csStrNew(buf));
  sprintf(buf, "FALLTIME (%g)\n", state->GetFallTime());
  str->Push(csStrNew(buf));

  fact->DecRef();
  partstate->DecRef();
  state->DecRef();
}

//---------------------------------------------------------------------------

