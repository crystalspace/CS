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
#include "exploldr.h"
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "impartic.h"
#include "imexplo.h"
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

IMPLEMENT_IBASE (csExplosionFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csExplosionLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csExplosionFactoryLoader)
IMPLEMENT_FACTORY (csExplosionLoader)

EXPORT_CLASS_TABLE (exploldr)
  EXPORT_CLASS (csExplosionFactoryLoader, "crystalspace.mesh.loader.factory.explosion",
    "Crystal Space Explosion Factory Loader")
  EXPORT_CLASS (csExplosionLoader, "crystalspace.mesh.loader.explosion",
    "Crystal Space Explosion Mesh Loader")
EXPORT_CLASS_TABLE_END

csExplosionFactoryLoader::csExplosionFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csExplosionFactoryLoader::~csExplosionFactoryLoader ()
{
}

bool csExplosionFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csExplosionFactoryLoader::Parse (const char* /*string*/, iEngine* /*engine*/)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.explosion", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.explosion", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.explosion\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  return fact;
}

//---------------------------------------------------------------------------

csExplosionLoader::csExplosionLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csExplosionLoader::~csExplosionLoader ()
{
}

bool csExplosionLoader::Initialize (iSystem* system)
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

iBase* csExplosionLoader::Parse (const char* string, iEngine* engine)
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
	  ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
	  partstate->SetColor (color);
	}
	break;
      case CS_TOKEN_CENTER:
	{
	  csVector3 center;
	  ScanStr (params, "%f,%f,%f", &center.x, &center.y, &center.z);
	  explostate->SetCenter (center);
	}
	break;
      case CS_TOKEN_PUSH:
	{
	  csVector3 push;
	  ScanStr (params, "%f,%f,%f", &push.x, &push.y, &push.z);
	  explostate->SetPush (push);
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
	    if (explostate) explostate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = QUERY_INTERFACE (mesh, iParticleState);
          explostate = QUERY_INTERFACE (mesh, iExplosionState);
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
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          explostate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          ScanStr (params, "%d", &nr);
          explostate->SetNumberParticles (nr);
        }
        break;
      case CS_TOKEN_NRSIDES:
        {
          int nr;
          ScanStr (params, "%d", &nr);
          explostate->SetNrSides (nr);
        }
        break;
      case CS_TOKEN_FADE:
        {
          int f;
          ScanStr (params, "%d", &f);
          explostate->SetFadeSprites (f);
        }
        break;
      case CS_TOKEN_PARTRADIUS:
        {
          float f;
          ScanStr (params, "%f", &f);
          explostate->SetPartRadius (f);
        }
        break;
      case CS_TOKEN_SPREADPOS:
        {
          float f;
          ScanStr (params, "%f", &f);
          explostate->SetSpreadPos (f);
        }
        break;
      case CS_TOKEN_SPREADSPEED:
        {
          float f;
          ScanStr (params, "%f", &f);
          explostate->SetSpreadSpeed (f);
        }
        break;
      case CS_TOKEN_SPREADACCEL:
        {
          float f;
          ScanStr (params, "%f", &f);
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


