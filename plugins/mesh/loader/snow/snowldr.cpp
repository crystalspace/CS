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
#include "snowldr.h"
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "impartic.h"
#include "imsnow.h"
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

  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FALLSPEED)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (BOX)
  CS_TOKEN_DEF (SWIRL)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csSnowFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSnowLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSnowFactoryLoader)
IMPLEMENT_FACTORY (csSnowLoader)

EXPORT_CLASS_TABLE (snowldr)
  EXPORT_CLASS (csSnowFactoryLoader, "crystalspace.mesh.loader.factory.snow",
    "Crystal Space Snow Factory Loader")
  EXPORT_CLASS (csSnowLoader, "crystalspace.mesh.loader.snow",
    "Crystal Space Snow Mesh Loader")
EXPORT_CLASS_TABLE_END

csSnowFactoryLoader::csSnowFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSnowFactoryLoader::~csSnowFactoryLoader ()
{
}

bool csSnowFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csSnowFactoryLoader::Parse (const char* /*string*/, iEngine* /*engine*/)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.snow", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.snow", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.snow\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csSnowLoader::csSnowLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSnowLoader::~csSnowLoader ()
{
}

bool csSnowLoader::Initialize (iSystem* system)
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

iBase* csSnowLoader::Parse (const char* string, iEngine* engine)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (FALLSPEED)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (SWIRL)
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
  iSnowState* snowstate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (snowstate) snowstate->DecRef ();
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
	  snowstate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_BOX:
	{
	  csVector3 minbox, maxbox;
	  ScanStr (params, "%f,%f,%f,%f,%f,%f",
	      	&minbox.x, &minbox.y, &minbox.z,
	      	&maxbox.x, &maxbox.y, &maxbox.z);
	  snowstate->SetBox (minbox, maxbox);
	}
	break;
      case CS_TOKEN_FALLSPEED:
	{
	  csVector3 s;
	  ScanStr (params, "%f,%f,%f", &s.x, &s.y, &s.z);
	  snowstate->SetFallSpeed (s);
	}
	break;
      case CS_TOKEN_SWIRL:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  snowstate->SetSwirl (f);
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
	    if (snowstate) snowstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = QUERY_INTERFACE (mesh, iParticleState);
          snowstate = QUERY_INTERFACE (mesh, iSnowState);
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
	    if (snowstate) snowstate->DecRef ();
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
          snowstate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          ScanStr (params, "%d", &nr);
          snowstate->SetNumberParticles (nr);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (snowstate) snowstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


