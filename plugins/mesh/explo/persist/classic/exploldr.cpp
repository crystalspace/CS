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
#include "exploldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/plugin.h"
#include "imesh/partsys.h"
#include "imesh/explode.h"
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

  CS_TOKEN_DEF (CENTER)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FADE)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (NRSIDES)
  CS_TOKEN_DEF (PARTRADIUS)
  CS_TOKEN_DEF (PUSH)
  CS_TOKEN_DEF (SPREADPOS)
  CS_TOKEN_DEF (SPREADSPEED)
  CS_TOKEN_DEF (SPREADACCEL)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csExplosionFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csExplosionFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csExplosionLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csExplosionSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csExplosionFactoryLoader)
SCF_IMPLEMENT_FACTORY (csExplosionFactorySaver)
SCF_IMPLEMENT_FACTORY (csExplosionLoader)
SCF_IMPLEMENT_FACTORY (csExplosionSaver)

SCF_EXPORT_CLASS_TABLE (exploldr)
  SCF_EXPORT_CLASS (csExplosionFactoryLoader, 
    "crystalspace.mesh.loader.factory.explosion",
    "Crystal Space Explosion Factory Loader")
  SCF_EXPORT_CLASS (csExplosionFactorySaver, 
    "crystalspace.mesh.saver.factory.explosion",
    "Crystal Space Explosion Factory Saver")
  SCF_EXPORT_CLASS (csExplosionLoader, "crystalspace.mesh.loader.explosion",
    "Crystal Space Explosion Mesh Loader")
  SCF_EXPORT_CLASS (csExplosionSaver, "crystalspace.mesh.saver.explosion",
    "Crystal Space Explosion Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csExplosionFactoryLoader::csExplosionFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionFactoryLoader::~csExplosionFactoryLoader ()
{
}

bool csExplosionFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csExplosionFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csExplosionFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.explosion", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.explosion",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.explosion\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csExplosionFactorySaver::csExplosionFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionFactorySaver::~csExplosionFactorySaver ()
{
}

bool csExplosionFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csExplosionFactorySaver::object_reg = object_reg;
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

void csExplosionFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/,
  iEngine* /*engine*/)
{
  // nothing to do
}


//---------------------------------------------------------------------------

csExplosionLoader::csExplosionLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionLoader::~csExplosionLoader ()
{
}

bool csExplosionLoader::Initialize (iObjectRegistry* object_reg)
{
  csExplosionLoader::object_reg = object_reg;
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

iBase* csExplosionLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (CENTER)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (FADE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (NRSIDES)
    CS_TOKEN_TABLE (PARTRADIUS)
    CS_TOKEN_TABLE (PUSH)
    CS_TOKEN_TABLE (SPREADPOS)
    CS_TOKEN_TABLE (SPREADSPEED)
    CS_TOKEN_TABLE (SPREADACCEL)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  iMeshObject* mesh = NULL;
  iParticleState* partstate = NULL;
  iExplosionState* explostate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (explostate) explostate->DecRef ();
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
      case CS_TOKEN_CENTER:
	{
	  csVector3 center;
	  csScanStr (params, "%f,%f,%f", &center.x, &center.y, &center.z);
	  explostate->SetCenter (center);
	}
	break;
      case CS_TOKEN_PUSH:
	{
	  csVector3 push;
	  csScanStr (params, "%f,%f,%f", &push.x, &push.y, &push.z);
	  explostate->SetPush (push);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->GetMeshFactories ()
	  	->FindByName (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (partstate) partstate->DecRef ();
	    if (explostate) explostate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          explostate = SCF_QUERY_INTERFACE (mesh, iExplosionState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->GetMaterialList ()->
	  	FindByName (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            mesh->DecRef ();
	    if (partstate) partstate->DecRef ();
	    if (explostate) explostate->DecRef ();
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
          explostate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          csScanStr (params, "%d", &nr);
          explostate->SetParticleCount (nr);
        }
        break;
      case CS_TOKEN_NRSIDES:
        {
          int nr;
          csScanStr (params, "%d", &nr);
          explostate->SetNrSides (nr);
        }
        break;
      case CS_TOKEN_FADE:
        {
          int f;
          csScanStr (params, "%d", &f);
          explostate->SetFadeSprites (f);
        }
        break;
      case CS_TOKEN_PARTRADIUS:
        {
          float f;
          csScanStr (params, "%f", &f);
          explostate->SetPartRadius (f);
        }
        break;
      case CS_TOKEN_SPREADPOS:
        {
          float f;
          csScanStr (params, "%f", &f);
          explostate->SetSpreadPos (f);
        }
        break;
      case CS_TOKEN_SPREADSPEED:
        {
          float f;
          csScanStr (params, "%f", &f);
          explostate->SetSpreadSpeed (f);
        }
        break;
      case CS_TOKEN_SPREADACCEL:
        {
          float f;
          csScanStr (params, "%f", &f);
          explostate->SetSpreadAcceleration (f);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (explostate) explostate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csExplosionSaver::csExplosionSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionSaver::~csExplosionSaver ()
{
}

bool csExplosionSaver::Initialize (iObjectRegistry* object_reg)
{
  csExplosionSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csExplosionSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  iExplosionState *explostate = SCF_QUERY_INTERFACE (obj, iExplosionState);
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

  sprintf(buf, "CENTER (%g, %g, %g)\n", explostate->GetCenter().x,
    explostate->GetCenter().y, explostate->GetCenter().z);
  str->Push(csStrNew(buf));
  sprintf(buf, "PUSH (%g, %g, %g)\n", explostate->GetPush().x,
    explostate->GetPush().y, explostate->GetPush().z);
  str->Push(csStrNew(buf));
  sprintf(buf, "SPREADPOS (%g)\n", explostate->GetSpreadPos());
  str->Push(csStrNew(buf));
  sprintf(buf, "SPREADSPEED (%g)\n", explostate->GetSpreadSpeed());
  str->Push(csStrNew(buf));
  sprintf(buf, "SPREADACCEL (%g)\n", explostate->GetSpreadAcceleration());
  str->Push(csStrNew(buf));
  sprintf(buf, "NUMBER (%d)\n", explostate->GetParticleCount());
  str->Push(csStrNew(buf));
  sprintf(buf, "NRSIDES (%d)\n", explostate->GetNrSides());
  str->Push(csStrNew(buf));
  sprintf(buf, "PARTRADIUS (%g)\n", explostate->GetPartRadius());
  str->Push(csStrNew(buf));
  sprintf(buf, "LIGHTING (%s)\n", explostate->GetLighting()?"true":"false");
  str->Push(csStrNew(buf));
  csTicks fade_time = 0;
  if(explostate->GetFadeSprites(fade_time))
  {
    sprintf(buf, "FADE (%d)\n", (int)fade_time);
    str->Push(csStrNew(buf));
  }

  fact->DecRef();
  partstate->DecRef();
  explostate->DecRef();
}

//---------------------------------------------------------------------------

