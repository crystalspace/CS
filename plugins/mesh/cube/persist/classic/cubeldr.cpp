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
#include "cubeldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "imesh/cube.h"
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

  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (SHIFT)
  CS_TOKEN_DEF (SIZE)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csCubeFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeFactoryLoader::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csCubeFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugIn)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeFactorySaver::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csCubeLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeLoader::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csCubeSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugIn)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCubeSaver::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csCubeFactoryLoader)
SCF_IMPLEMENT_FACTORY (csCubeFactorySaver)
SCF_IMPLEMENT_FACTORY (csCubeLoader)
SCF_IMPLEMENT_FACTORY (csCubeSaver)

SCF_EXPORT_CLASS_TABLE (cubeldr)
  SCF_EXPORT_CLASS (csCubeFactoryLoader,
    "crystalspace.mesh.loader.factory.cube",
    "Crystal Space Cube Mesh Factory Loader")
  SCF_EXPORT_CLASS (csCubeFactorySaver, "crystalspace.mesh.saver.factory.cube",
    "Crystal Space Cube Mesh Factory Saver")
  SCF_EXPORT_CLASS (csCubeLoader, "crystalspace.mesh.loader.cube",
    "Crystal Space Cube Mesh Loader")
  SCF_EXPORT_CLASS (csCubeSaver, "crystalspace.mesh.saver.cube",
    "Crystal Space Cube Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csCubeFactoryLoader::csCubeFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csCubeFactoryLoader::~csCubeFactoryLoader ()
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

iBase* csCubeFactoryLoader::Parse (const char* string, iEngine* engine,
	iBase* /* context */)
{
  // @@@ Implement MIXMODE
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (SHIFT)
    CS_TOKEN_TABLE (SIZE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (sys,
  	"crystalspace.mesh.object.cube", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (sys, "crystalspace.mesh.object.cube",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.cube\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  iCubeFactoryState* cubeLook = SCF_QUERY_INTERFACE (fact, iCubeFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      cubeLook->DecRef ();
      fact->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
	    cubeLook->DecRef ();
            fact->DecRef ();
            return NULL;
	  }
	  cubeLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        cubeLook->SetMixMode (ParseMixmode (params));
	break;
      case CS_TOKEN_SHIFT:
	{
	  float shiftx, shifty, shiftz;
	  csScanStr (params, "%f,%f,%f", &shiftx, &shifty, &shiftz);
	  cubeLook->SetShift (shiftx, shifty, shiftz);
	}
	break;
      case CS_TOKEN_SIZE:
	{
	  float sizex, sizey, sizez;
	  csScanStr (params, "%f,%f,%f", &sizex, &sizey, &sizez);
	  cubeLook->SetSize (sizex, sizey, sizez);
	}
	break;
    }
  }

  cubeLook->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csCubeFactorySaver::csCubeFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csCubeFactorySaver::~csCubeFactorySaver ()
{
}

#define MAXLINE 100 /* max number of chars per line... */

static void WriteMixmode(iStrVector *str, UInt mixmode)
{
  str->Push(csStrNew("  MIXMODE ("));
  if (mixmode&CS_FX_COPY) str->Push(csStrNew(" COPY ()"));
  if (mixmode&CS_FX_ADD) str->Push(csStrNew(" ADD ()"));
  if (mixmode&CS_FX_MULTIPLY) str->Push(csStrNew(" MULTIPLY ()"));
  if (mixmode&CS_FX_MULTIPLY2) str->Push(csStrNew(" MULTIPLY2 ()"));
  if (mixmode&CS_FX_KEYCOLOR) str->Push(csStrNew(" KEYCOLOR ()"));
  if (mixmode&CS_FX_TRANSPARENT) str->Push(csStrNew(" TRANSPARENT ()"));
  if (mixmode&CS_FX_ALPHA)
  {
    char buf[MAXLINE];
    sprintf (buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(csStrNew(buf));
  }
  str->Push (csStrNew(")"));
}

void csCubeFactorySaver::WriteDown (iBase* obj, iStrVector * str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iCubeFactoryState *cubelook = SCF_QUERY_INTERFACE(obj, iCubeFactoryState);
  if(!cubelook)
  {
    printf("Error: non-cubefactorystate given to %s.\n",
      fact->QueryDescription () );
    fact->DecRef();
    return;
  }

  if(cubelook->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, cubelook->GetMixMode());
  }

  char buf[MAXLINE];
  sprintf(buf, "MATERIAL (%s)\n", cubelook->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str->Push(csStrNew(buf));
  sprintf(buf, "SIZE (%g, %g, %g)\n", cubelook->GetSizeX (),
    cubelook->GetSizeY (), cubelook->GetSizeZ ());
  str->Push(csStrNew(buf));
  sprintf(buf, "SHIFT (%g, %g, %g)\n", cubelook->GetShiftX (),
    cubelook->GetShiftY (), cubelook->GetShiftZ ());
  str->Push(csStrNew(buf));

  cubelook->DecRef();
  fact->DecRef();
}

//---------------------------------------------------------------------------

csCubeLoader::csCubeLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csCubeLoader::~csCubeLoader ()
{
}

iBase* csCubeLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  iMeshObject* mesh = 0;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
        csScanStr (params, "%s", str);
	iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	if (!fact)
	{
	  // @@@ Error handling!
	  return NULL;
	}
	mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	imeshwrap->SetFactory (fact);
	break;
    }
  }

  return mesh;
}

//---------------------------------------------------------------------------

csCubeSaver::csCubeSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csCubeSaver::~csCubeSaver ()
{
}

void csCubeSaver::WriteDown (iBase* /*obj*/, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(csStrNew(buf));
  fact->DecRef();
}
