/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "surfldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "imesh/surf.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"

CS_IMPLEMENT_PLUGIN

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
  CS_TOKEN_DEF (RESOLUTION)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (TOPLEFT)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MIXMODE)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csSurfFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSurfFactoryLoader::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSurfFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugIn)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSurfFactorySaver::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSurfLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSurfLoader::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSurfSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugIn)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSurfSaver::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSurfFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSurfFactorySaver)
SCF_IMPLEMENT_FACTORY (csSurfLoader)
SCF_IMPLEMENT_FACTORY (csSurfSaver)

SCF_EXPORT_CLASS_TABLE (surfldr)
  SCF_EXPORT_CLASS (csSurfFactoryLoader,
    "crystalspace.mesh.loader.factory.surface",
    "Crystal Space Surface Factory Loader")
  SCF_EXPORT_CLASS (csSurfFactorySaver,
    "crystalspace.mesh.saver.factory.surface",
    "Crystal Space Surface Factory Saver")
  SCF_EXPORT_CLASS (csSurfLoader,
    "crystalspace.mesh.loader.surface",
    "Crystal Space Surface Mesh Loader")
  SCF_EXPORT_CLASS (csSurfSaver,
    "crystalspace.mesh.saver.surface",
    "Crystal Space Surface Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csSurfFactoryLoader::csSurfFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csSurfFactoryLoader::~csSurfFactoryLoader ()
{
}

iBase* csSurfFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (sys,
  	"crystalspace.mesh.object.surface", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (sys, "crystalspace.mesh.object.surface",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.surface\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csSurfFactorySaver::csSurfFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csSurfFactorySaver::~csSurfFactorySaver ()
{
}

#define MAXLINE 100 /* max number of chars per line... */

void csSurfFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/,
  iEngine* /*engine*/)
{
  // no params
}

//---------------------------------------------------------------------------

csSurfLoader::csSurfLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csSurfLoader::~csSurfLoader ()
{
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
        csScanStr (params, "%f", &alpha);
	Mixmode |= CS_FX_SETALPHA(alpha);
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

iBase* csSurfLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (TOPLEFT)
    CS_TOKEN_TABLE (SCALE)
    CS_TOKEN_TABLE (RESOLUTION)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (COLOR)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iSurfaceState* surfstate = NULL;
  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (surfstate) surfstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_LIGHTING:
	{
	  bool r;
	  csScanStr (params, "%b", &r);
	  surfstate->SetLighting (r);
	}
	break;
      case CS_TOKEN_COLOR:
	{
	  csColor col;
	  csScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  surfstate->SetColor (col);
	}
	break;
      case CS_TOKEN_TOPLEFT:
	{
	  csVector3 tl;
	  csScanStr (params, "%f,%f,%f", &tl.x, &tl.y, &tl.z);
	  surfstate->SetTopLeftCorner (tl);
	}
	break;
      case CS_TOKEN_SCALE:
	{
	  float x, y;
	  csScanStr (params, "%f,%f", &x, &y);
	  surfstate->SetScale (x, y);
	}
	break;
      case CS_TOKEN_RESOLUTION:
	{
	  int x, y;
	  csScanStr (params, "%d,%d", &x, &y);
	  surfstate->SetResolution (x, y);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
          surfstate = SCF_QUERY_INTERFACE (mesh, iSurfaceState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            mesh->DecRef ();
            return NULL;
	  }
	  surfstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        surfstate->SetMixMode (ParseMixmode (params));
	break;
    }
  }

  if (surfstate) surfstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csSurfSaver::csSurfSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csSurfSaver::~csSurfSaver ()
{
}

static void WriteMixmode(iStrVector *str, UInt mixmode)
{
  str->Push(csStrNew("  MIXMODE ("));
  if(mixmode&CS_FX_COPY) str->Push(csStrNew(" COPY ()"));
  if(mixmode&CS_FX_ADD) str->Push(csStrNew(" ADD ()"));
  if(mixmode&CS_FX_MULTIPLY) str->Push(csStrNew(" MULTIPLY ()"));
  if(mixmode&CS_FX_MULTIPLY2) str->Push(csStrNew(" MULTIPLY2 ()"));
  if(mixmode&CS_FX_KEYCOLOR) str->Push(csStrNew(" KEYCOLOR ()"));
  if(mixmode&CS_FX_TRANSPARENT) str->Push(csStrNew(" TRANSPARENT ()"));
  if(mixmode&CS_FX_ALPHA) {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(csStrNew(buf));
  }
  str->Push(csStrNew(")"));
}

void csSurfSaver::WriteDown (iBase* obj, iStrVector *str,
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
  iSurfaceState *state = SCF_QUERY_INTERFACE(obj, iSurfaceState);
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
  csVector3 tl = state->GetTopLeftCorner ();
  sprintf(buf, "TOPLEFT (%g, %g, %g)\n", tl.x, tl.y, tl.z);
  str->Push(csStrNew(buf));
  sprintf(buf, "SCALE (%g, %g)\n", state->GetXScale (), state->GetYScale ());
  str->Push(csStrNew(buf));
  sprintf(buf, "RESOLUTION (%d, %d)\n",
  	state->GetXResolution (), state->GetYResolution ());
  str->Push(csStrNew(buf));
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    QueryObject()->GetName());
  str->Push(csStrNew(buf));
  if (!state->IsLighting ())
    str->Push (csStrNew ("LIGHTING (no)\n"));
  csColor col = state->GetColor ();
  sprintf(buf, "COLOR (%g,%g,%g)\n", col.red, col.green, col.blue);
  str->Push(csStrNew(buf));

  fact->DecRef();
  mesh->DecRef();
  state->DecRef();

}
