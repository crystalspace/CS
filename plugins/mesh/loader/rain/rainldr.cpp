/*
    Copyright (C) 2000 by Jorrit Tyberghein
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
#include "isys/system.h"
#include "imesh/partsys.h"
#include "imesh/rain.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iobject/object.h"
#include "iengine/material.h"
#include "csengine/material.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
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

IMPLEMENT_IBASE (csRainFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csRainFactorySaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csRainLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csRainSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csRainFactoryLoader)
IMPLEMENT_FACTORY (csRainFactorySaver)
IMPLEMENT_FACTORY (csRainLoader)
IMPLEMENT_FACTORY (csRainSaver)

EXPORT_CLASS_TABLE (rainldr)
  EXPORT_CLASS (csRainFactoryLoader, "crystalspace.mesh.loader.factory.rain",
    "Crystal Space Rain Factory Loader")
  EXPORT_CLASS (csRainFactorySaver, "crystalspace.mesh.saver.factory.rain",
    "Crystal Space Rain Factory Saver")
  EXPORT_CLASS (csRainLoader, "crystalspace.mesh.loader.rain",
    "Crystal Space Rain Mesh Loader")
  EXPORT_CLASS (csRainSaver, "crystalspace.mesh.saver.rain",
    "Crystal Space Rain Mesh Saver")
EXPORT_CLASS_TABLE_END

csRainFactoryLoader::csRainFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csRainFactoryLoader::~csRainFactoryLoader ()
{
}

bool csRainFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csRainFactoryLoader::Parse (const char* /*string*/, iEngine* /*engine*/)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.rain", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.rain", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.rain\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------
csRainFactorySaver::csRainFactorySaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csRainFactorySaver::~csRainFactorySaver ()
{
}

bool csRainFactorySaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

static void WriteMixmode(iStrVector *str, UInt mixmode)
{
  str->Push(strnew("  MIXMODE ("));
  if(mixmode&CS_FX_COPY) str->Push(strnew(" COPY ()"));
  if(mixmode&CS_FX_ADD) str->Push(strnew(" ADD ()"));
  if(mixmode&CS_FX_MULTIPLY) str->Push(strnew(" MULTIPLY ()"));
  if(mixmode&CS_FX_MULTIPLY2) str->Push(strnew(" MULTIPLY2 ()"));
  if(mixmode&CS_FX_KEYCOLOR) str->Push(strnew(" KEYCOLOR ()"));
  if(mixmode&CS_FX_TRANSPARENT) str->Push(strnew(" TRANSPARENT ()"));
  if(mixmode&CS_FX_ALPHA) {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(strnew(buf));
  }
  str->Push(strnew(")"));
}

void csRainFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/,
  iEngine* /*engine*/)
{
  // nothing to do
}
//---------------------------------------------------------------------------

csRainLoader::csRainLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csRainLoader::~csRainLoader ()
{
}

bool csRainLoader::Initialize (iSystem* system)
{
  sys = system;
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
	int ialpha;
        ScanStr (params, "%f", &alpha);
	ialpha = QInt (alpha * 255.99);
	Mixmode |= CS_FX_SETALPHA(ialpha);
	break;
      case CS_TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing the modes!\n", csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

iBase* csRainLoader::Parse (const char* string, iEngine* engine)
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
	  ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
	  partstate->SetColor (color);
	}
	break;
      case CS_TOKEN_DROPSIZE:
	{
	  float dw, dh;
	  ScanStr (params, "%f,%f", &dw, &dh);
	  rainstate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_BOX:
	{
	  csVector3 minbox, maxbox;
	  ScanStr (params, "%f,%f,%f,%f,%f,%f",
	      	&minbox.x, &minbox.y, &minbox.z,
	      	&maxbox.x, &maxbox.y, &maxbox.z);
	  rainstate->SetBox (minbox, maxbox);
	}
	break;
      case CS_TOKEN_FALLSPEED:
	{
	  csVector3 s;
	  ScanStr (params, "%f,%f,%f", &s.x, &s.y, &s.z);
	  rainstate->SetFallSpeed (s);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          ScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (partstate) partstate->DecRef ();
	    if (rainstate) rainstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = QUERY_INTERFACE (mesh, iParticleState);
          rainstate = QUERY_INTERFACE (mesh, iRainState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          ScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
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
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          rainstate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          ScanStr (params, "%d", &nr);
          rainstate->SetNumberParticles (nr);
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
  CONSTRUCT_IBASE (pParent);
}

csRainSaver::~csRainSaver ()
{
}

bool csRainSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

void csRainSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = QUERY_INTERFACE (obj, iParticleState);
  iRainState *state = QUERY_INTERFACE (obj, iRainState);
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(strnew(buf));

  if(partstate->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, partstate->GetMixMode());
  }

  sprintf(buf, "MATERIAL (%s)\n", partstate->GetMaterialWrapper()->
    GetPrivateObject()->GetName());
  str->Push(strnew(buf));
  sprintf(buf, "COLOR (%g, %g, %g)\n", partstate->GetColor().red,
    partstate->GetColor().green, partstate->GetColor().blue);
  str->Push(strnew(buf));
  printf(buf, "NUMBER (%d)\n", state->GetNumberParticles());
  str->Push(strnew(buf));
  sprintf(buf, "LIGHTING (%s)\n", state->GetLighting()?"true":"false");
  str->Push(strnew(buf));
  float sx = 0.0, sy = 0.0;
  state->GetDropSize(sx, sy);
  sprintf(buf, "DROPSIZE (%g, %g)\n", sx, sy);
  str->Push(strnew(buf));
  printf(buf, "FALLSPEED (%g, %g, %g)\n", state->GetFallSpeed().x, 
    state->GetFallSpeed().y, state->GetFallSpeed().z);
  str->Push(strnew(buf));
  csVector3 minbox, maxbox;
  state->GetBox(minbox, maxbox);
  printf(buf, "BOX (%g,%g,%g, %g,%g,%g)\n", minbox.x, minbox.y, minbox.z, 
    maxbox.x, maxbox.y, maxbox.z);

  fact->DecRef();
  partstate->DecRef();
  state->DecRef();
}


//---------------------------------------------------------------------------
