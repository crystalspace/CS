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
#include "cubeldr.h"
#include "imesh/imeshobj.h"
#include "iengine/imeshobj.h"
#include "iengine/iengine.h"
#include "isys/isystem.h"
#include "imesh/imcube.h"
#include "ivideo/igraph3d.h"
#include "qint.h"
#include "iutil/istrvec.h"
#include "csutil/util.h"
#include "iobject/iobject.h"
#include "iengine/imater.h"
#include "csengine/material.h"

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

IMPLEMENT_IBASE (csCubeFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csCubeFactorySaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csCubeLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csCubeSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csCubeFactoryLoader)
IMPLEMENT_FACTORY (csCubeFactorySaver)
IMPLEMENT_FACTORY (csCubeLoader)
IMPLEMENT_FACTORY (csCubeSaver)

EXPORT_CLASS_TABLE (cubeldr)
  EXPORT_CLASS (csCubeFactoryLoader, "crystalspace.mesh.loader.factory.cube",
    "Crystal Space Cube Mesh Factory Loader")
  EXPORT_CLASS (csCubeFactorySaver, "crystalspace.mesh.saver.factory.cube",
    "Crystal Space Cube Mesh Factory Saver")
  EXPORT_CLASS (csCubeLoader, "crystalspace.mesh.loader.cube",
    "Crystal Space Cube Mesh Loader")
  EXPORT_CLASS (csCubeSaver, "crystalspace.mesh.saver.cube",
    "Crystal Space Cube Mesh Saver")
EXPORT_CLASS_TABLE_END

csCubeFactoryLoader::csCubeFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csCubeFactoryLoader::~csCubeFactoryLoader ()
{
}

bool csCubeFactoryLoader::Initialize (iSystem* system)
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

iBase* csCubeFactoryLoader::Parse (const char* string, iEngine* engine)
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

  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.cube", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.cube", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.cube\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  iCubeFactoryState* cubeLook = QUERY_INTERFACE (fact, iCubeFactoryState);

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
          ScanStr (params, "%s", str);
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
	  ScanStr (params, "%f,%f,%f", &shiftx, &shifty, &shiftz);
	  cubeLook->SetShift (shiftx, shifty, shiftz);
	}
	break;
      case CS_TOKEN_SIZE:
	{
	  float sizex, sizey, sizez;
	  ScanStr (params, "%f,%f,%f", &sizex, &sizey, &sizez);
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
  CONSTRUCT_IBASE (pParent);
}

csCubeFactorySaver::~csCubeFactorySaver ()
{
}

bool csCubeFactorySaver::Initialize (iSystem* system)
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

void csCubeFactorySaver::WriteDown (iBase* obj, iStrVector * str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  iCubeFactoryState *cubelook = QUERY_INTERFACE(obj, iCubeFactoryState);
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
    GetPrivateObject()->GetName());
  str->Push(strnew(buf));
  sprintf(buf, "SIZE (%g, %g, %g)\n", cubelook->GetSizeX (),
    cubelook->GetSizeY (), cubelook->GetSizeZ ());
  str->Push(strnew(buf));
  sprintf(buf, "SHIFT (%g, %g, %g)\n", cubelook->GetShiftX (),
    cubelook->GetShiftY (), cubelook->GetShiftZ ());
  str->Push(strnew(buf));
  
  cubelook->DecRef();
  fact->DecRef();
}

//---------------------------------------------------------------------------

csCubeLoader::csCubeLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csCubeLoader::~csCubeLoader ()
{
}

bool csCubeLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csCubeLoader::Parse (const char* string, iEngine* engine)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

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
        ScanStr (params, "%s", str);
	iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	if (!fact)
	{
	  // @@@ Error handling!
	  return NULL;
	}
	mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	break;
    }
  }

  return mesh;
}

//---------------------------------------------------------------------------

csCubeSaver::csCubeSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csCubeSaver::~csCubeSaver ()
{
}

bool csCubeSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

void csCubeSaver::WriteDown (iBase* /*obj*/, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(strnew(buf));
  fact->DecRef();
}

//---------------------------------------------------------------------------

