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
#include "spirldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "imesh/partsys.h"
#include "imesh/spiral.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iobject/object.h"
#include "iengine/material.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)

  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (SOURCE)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csSpiralFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSpiralFactorySaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSpiralLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSpiralSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSpiralFactoryLoader)
IMPLEMENT_FACTORY (csSpiralFactorySaver)
IMPLEMENT_FACTORY (csSpiralLoader)
IMPLEMENT_FACTORY (csSpiralSaver)

EXPORT_CLASS_TABLE (spirldr)
  EXPORT_CLASS (csSpiralFactoryLoader,
  	"crystalspace.mesh.loader.factory.spiral",
	"Crystal Space Spiral Factory Loader")
  EXPORT_CLASS (csSpiralFactorySaver, "crystalspace.mesh.saver.factory.spiral",
    "Crystal Space Spiral Factory Saver")
  EXPORT_CLASS (csSpiralLoader, "crystalspace.mesh.loader.spiral",
    "Crystal Space Spiral Mesh Loader")
  EXPORT_CLASS (csSpiralSaver, "crystalspace.mesh.saver.spiral",
    "Crystal Space Spiral Mesh Saver")
EXPORT_CLASS_TABLE_END

csSpiralFactoryLoader::csSpiralFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSpiralFactoryLoader::~csSpiralFactoryLoader ()
{
}

bool csSpiralFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csSpiralFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys,
  	"crystalspace.mesh.object.spiral", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.spiral",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.spiral\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------
csSpiralFactorySaver::csSpiralFactorySaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSpiralFactorySaver::~csSpiralFactorySaver ()
{
}

bool csSpiralFactorySaver::Initialize (iSystem* system)
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
  if(mixmode&CS_FX_ALPHA)
  {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(strnew(buf));
  }
  str->Push(strnew(")"));
}

void csSpiralFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/,
  iEngine* /*engine*/)
{
  // nothing to do
}
//---------------------------------------------------------------------------
csSpiralLoader::csSpiralLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSpiralLoader::~csSpiralLoader ()
{
}

bool csSpiralLoader::Initialize (iSystem* system)
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
    printf ("Token '%s' not found while parsing the modes!\n",
    	csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

iBase* csSpiralLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (SOURCE)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshWrapper* imeshwrap = QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  iMeshObject* mesh = NULL;
  iParticleState* partstate = NULL;
  iSpiralState* spiralstate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (spiralstate) spiralstate->DecRef ();
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
      case CS_TOKEN_SOURCE:
	{
	  csVector3 s;
	  ScanStr (params, "%f,%f,%f", &s.x, &s.y, &s.z);
	  spiralstate->SetSource (s);
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
	    if (spiralstate) spiralstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
          partstate = QUERY_INTERFACE (mesh, iParticleState);
          spiralstate = QUERY_INTERFACE (mesh, iSpiralState);
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
	    if (spiralstate) spiralstate->DecRef ();
            return NULL;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        partstate->SetMixMode (ParseMixmode (params));
	break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          ScanStr (params, "%d", &nr);
          spiralstate->SetNumberParticles (nr);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (spiralstate) spiralstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csSpiralSaver::csSpiralSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSpiralSaver::~csSpiralSaver ()
{
}

bool csSpiralSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

void csSpiralSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = QUERY_INTERFACE (obj, iParticleState);
  iSpiralState *state = QUERY_INTERFACE (obj, iSpiralState);
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
    QueryObject ()->GetName());
  str->Push(strnew(buf));
  sprintf(buf, "COLOR (%g, %g, %g)\n", partstate->GetColor().red,
    partstate->GetColor().green, partstate->GetColor().blue);
  str->Push(strnew(buf));
  printf(buf, "NUMBER (%d)\n", state->GetNumberParticles());
  str->Push(strnew(buf));
  printf(buf, "SOURCE (%g, %g, %g)\n", state->GetSource().x,
    state->GetSource().y, state->GetSource().z);
  str->Push(strnew(buf));

  fact->DecRef();
  partstate->DecRef();
  state->DecRef();
}

//---------------------------------------------------------------------------
