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
#include "fireldr.h"
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "impartic.h"
#include "imfire.h"
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

  CS_TOKEN_DEF (COLORSCALE)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (SWIRL)
  CS_TOKEN_DEF (TOTALTIME)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csFireFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csFireLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csFireFactoryLoader)
IMPLEMENT_FACTORY (csFireLoader)

EXPORT_CLASS_TABLE (fireldr)
  EXPORT_CLASS (csFireFactoryLoader, "crystalspace.mesh.loader.factory.fire",
    "Crystal Space Fire Factory Loader")
  EXPORT_CLASS (csFireLoader, "crystalspace.mesh.loader.fire",
    "Crystal Space Fire Mesh Loader")
EXPORT_CLASS_TABLE_END

csFireFactoryLoader::csFireFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csFireFactoryLoader::~csFireFactoryLoader ()
{
}

bool csFireFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csFireFactoryLoader::Parse (const char* /*string*/, iEngine* /*engine*/)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.fire", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.fire", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.fire\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csFireLoader::csFireLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csFireLoader::~csFireLoader ()
{
}

bool csFireLoader::Initialize (iSystem* system)
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

iBase* csFireLoader::Parse (const char* string, iEngine* engine)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (SWIRL)
    CS_TOKEN_TABLE (TOTALTIME)
    CS_TOKEN_TABLE (COLORSCALE)
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
  iFireState* firestate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (firestate) firestate->DecRef ();
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
	  firestate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_ORIGIN:
	{
	  csVector3 origin;
	  ScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
	  firestate->SetOrigin (origin);
	}
	break;
      case CS_TOKEN_SWIRL:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  firestate->SetSwirl (f);
	}
	break;
      case CS_TOKEN_COLORSCALE:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  firestate->SetColorScale (f);
	}
	break;
      case CS_TOKEN_TOTALTIME:
	{
	  float f;
	  ScanStr (params, "%f", &f);
	  firestate->SetTotalTime (f);
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
	    if (firestate) firestate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = QUERY_INTERFACE (mesh, iParticleState);
          firestate = QUERY_INTERFACE (mesh, iFireState);
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
	    if (firestate) firestate->DecRef ();
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
          firestate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          ScanStr (params, "%d", &nr);
          firestate->SetNumberParticles (nr);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (firestate) firestate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


