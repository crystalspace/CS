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

IMPLEMENT_IBASE (csBallFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csBallFactorySaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csBallLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csBallSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csBallFactoryLoader)
IMPLEMENT_FACTORY (csBallFactorySaver)
IMPLEMENT_FACTORY (csBallLoader)
IMPLEMENT_FACTORY (csBallSaver)

EXPORT_CLASS_TABLE (ballldr)
  EXPORT_CLASS (csBallFactoryLoader, "crystalspace.mesh.loader.factory.ball",
    "Crystal Space Ball Factory Loader")
  EXPORT_CLASS (csBallFactorySaver, "crystalspace.mesh.saver.factory.ball",
    "Crystal Space Ball Factory Saver")
  EXPORT_CLASS (csBallLoader, "crystalspace.mesh.loader.ball",
    "Crystal Space Ball Mesh Loader")
  EXPORT_CLASS (csBallSaver, "crystalspace.mesh.saver.ball",
    "Crystal Space Ball Mesh Saver")
EXPORT_CLASS_TABLE_END

csBallFactoryLoader::csBallFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csBallFactoryLoader::~csBallFactoryLoader ()
{
}

bool csBallFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csBallFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys,
  	"crystalspace.mesh.object.ball", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.ball",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.ball\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csBallFactorySaver::csBallFactorySaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csBallFactorySaver::~csBallFactorySaver ()
{
}

bool csBallFactorySaver::Initialize (iSystem* system)
{
  sys = system;
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
  CONSTRUCT_IBASE (pParent);
}

csBallLoader::~csBallLoader ()
{
}

bool csBallLoader::Initialize (iSystem* system)
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
        ScanStr (params, "%f", &alpha);
	Mixmode |= CS_FX_SETALPHA(alpha);
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
  iMeshWrapper* imeshwrap = QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

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
      case CS_TOKEN_REVERSED:
	{
	  bool r;
	  ScanStr (params, "%b", &r);
	  ballstate->SetReversed (r);
	}
	break;
      case CS_TOKEN_TOPONLY:
	{
	  bool r;
	  ScanStr (params, "%b", &r);
	  ballstate->SetTopOnly (r);
	}
	break;
      case CS_TOKEN_CYLINDRICAL:
	{
	  bool r;
	  ScanStr (params, "%b", &r);
	  ballstate->SetCylindricalMapping (r);
	}
	break;
      case CS_TOKEN_LIGHTING:
	{
	  bool r;
	  ScanStr (params, "%b", &r);
	  ballstate->SetLighting (r);
	}
	break;
      case CS_TOKEN_COLOR:
	{
	  csColor col;
	  ScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  ballstate->SetColor (col);
	}
	break;
      case CS_TOKEN_RADIUS:
	{
	  float x, y, z;
	  ScanStr (params, "%f,%f,%f", &x, &y, &z);
	  ballstate->SetRadius (x, y, z);
	}
	break;
      case CS_TOKEN_SHIFT:
	{
	  float x, y, z;
	  ScanStr (params, "%f,%f,%f", &x, &y, &z);
	  ballstate->SetShift (x, y, z);
	}
	break;
      case CS_TOKEN_NUMRIM:
	{
	  int f;
	  ScanStr (params, "%d", &f);
	  ballstate->SetRimVertices (f);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          ScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
          ballstate = QUERY_INTERFACE (mesh, iBallState);
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
            return NULL;
	  }
	  ballstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        ballstate->SetMixMode (ParseMixmode (params));
	break;
    }
  }

  if (ballstate) ballstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csBallSaver::csBallSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csBallSaver::~csBallSaver ()
{
}

bool csBallSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

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

void csBallSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  iMeshObject *mesh = QUERY_INTERFACE(obj, iMeshObject);
  if(!mesh)
  {
    printf("Error: non-mesh given to %s.\n", 
      fact->QueryDescription () );
    fact->DecRef();
    return;
  }
  iBallState *state = QUERY_INTERFACE(obj, iBallState);
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
  str->Push(strnew(buf));
  if(state->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, state->GetMixMode());
  }

  // Mesh information
  float x=0, y=0, z=0;
  state->GetRadius(x, y, z);
  sprintf(buf, "RADIUS (%g, %g, %g)\n", x, y, z);
  str->Push(strnew(buf));
  sprintf(buf, "SHIFT (%g, %g, %g)\n", state->GetShift ().x, 
    state->GetShift ().y, state->GetShift ().z);
  str->Push(strnew(buf));
  sprintf(buf, "NUMRIM (%d)\n", state->GetRimVertices());
  str->Push(strnew(buf));
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str->Push(strnew(buf));
  if (!state->IsLighting ())
    str->Push (strnew ("LIGHTING (no)\n"));
  if (state->IsReversed ())
    str->Push (strnew ("REVERSED (yes)\n"));
  if (state->IsTopOnly ())
    str->Push (strnew ("TOPONLY (yes)\n"));
  if (state->IsCylindricalMapping ())
    str->Push (strnew ("CYLINDRICAL (yes)\n"));
  csColor col = state->GetColor ();
  sprintf(buf, "COLOR (%g,%g,%g)\n", col.red, col.green, col.blue);
  str->Push(strnew(buf));

  fact->DecRef();
  mesh->DecRef();
  state->DecRef();

}

//---------------------------------------------------------------------------

