/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "imball.h"
#include "igraph3d.h"
#include "qint.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)

  CS_TOKEN_DEF (NUMRIM)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (SHIFT)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csBallFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csBallLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csBallFactoryLoader)
IMPLEMENT_FACTORY (csBallLoader)

EXPORT_CLASS_TABLE (ballldr)
  EXPORT_CLASS (csBallFactoryLoader, "crystalspace.mesh.loader.factory.ball",
    "Crystal Space Ball Factory Loader")
  EXPORT_CLASS (csBallLoader, "crystalspace.mesh.loader.ball",
    "Crystal Space Ball Mesh Loader")
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

iBase* csBallFactoryLoader::Parse (const char* /*string*/, iEngine* /*engine*/)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.ball", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.ball", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.ball\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  return fact;
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

iBase* csBallLoader::Parse (const char* string, iEngine* engine)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (NUMRIM)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (SHIFT)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iBallState* ballstate = NULL;

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


