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
#include "plugins/meshldr/fountain/fountldr.h"
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "impartic.h"
#include "imfount.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ACCEL)
  CS_TOKEN_DEF (AZIMUTH)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (ELEVATION)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FALLTIME)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (OPENING)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (SPEED)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csFountainFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csFountainLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csFountainFactoryLoader)
IMPLEMENT_FACTORY (csFountainLoader)

EXPORT_CLASS_TABLE (fountldr)
  EXPORT_CLASS (csFountainFactoryLoader, "crystalspace.meshldr.fountfact",
    "Crystal Space Fountain Factory Loader")
  EXPORT_CLASS (csFountainLoader, "crystalspace.meshldr.fount",
    "Crystal Space Fountain Mesh Loader")
EXPORT_CLASS_TABLE_END

csFountainFactoryLoader::csFountainFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csFountainFactoryLoader::~csFountainFactoryLoader ()
{
}

bool csFountainFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csFountainFactoryLoader::Parse (const char* /*string*/, iEngine* /*engine*/)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.meshobj.fountain", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.meshobj.fountain", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.meshobj.fountain\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  return fact;
}

//---------------------------------------------------------------------------

csFountainLoader::csFountainLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csFountainLoader::~csFountainLoader ()
{
}

bool csFountainLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csFountainLoader::Parse (const char* string, iEngine* engine)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (ACCEL)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (FALLTIME)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (OPENING)
    CS_TOKEN_TABLE (AZIMUTH)
    CS_TOKEN_TABLE (ELEVATION)
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
  iFountainState* fountstate = NULL;

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
	  fountstate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_ORIGIN:
	{
	  csVector3 origin;
	  ScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
	  fountstate->SetOrigin (origin);
	}
	break;
      case CS_TOKEN_ACCEL:
	{
	  csVector3 accel;
	  ScanStr (params, "%f,%f,%f", &accel.x, &accel.y, &accel.z);
	  fountstate->SetAcceleration (accel);
	}
	break;
      case CS_TOKEN_SPEED:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  fountstate->SetSpeed (f);
	}
	break;
      case CS_TOKEN_OPENING:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  fountstate->SetOpening (f);
	}
	break;
      case CS_TOKEN_AZIMUTH:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  fountstate->SetAzimuth (f);
	}
	break;
      case CS_TOKEN_ELEVATION:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  fountstate->SetElevation (f);
	}
	break;
      case CS_TOKEN_FALLTIME:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  fountstate->SetFallTime (f);
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
          partstate = QUERY_INTERFACE (mesh, iParticleState);
          fountstate = QUERY_INTERFACE (mesh, iFountainState);
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
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	printf ("Not implemented yet!\n");
	break;
      case CS_TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          fountstate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          ScanStr (params, "%d", &nr);
          fountstate->SetNumberParticles (nr);
        }
        break;
    }
  }

  return mesh;
}

//---------------------------------------------------------------------------


