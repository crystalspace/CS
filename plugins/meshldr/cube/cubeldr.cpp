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
#include "plugins/meshldr/cube/cubeldr.h"
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "imcube.h"

CS_TOKEN_DEF_START
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

IMPLEMENT_IBASE (csCubeLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csCubeFactoryLoader)
IMPLEMENT_FACTORY (csCubeLoader)

EXPORT_CLASS_TABLE (cubeldr)
  EXPORT_CLASS (csCubeFactoryLoader, "crystalspace.meshldr.cubefact",
    "Crystal Space Cube Mesh Factory Loader")
  EXPORT_CLASS (csCubeLoader, "crystalspace.meshldr.cube",
    "Crystal Space Cube Mesh Loader")
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

  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.meshobj.cube", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.meshobj.cube", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.meshobj.cube\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  iCubeFactoryState* cubeLook = QUERY_INTERFACE (fact, iCubeFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
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
            fact->DecRef ();
            return NULL;
	  }
	  cubeLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	printf ("Not implemented yet!\n");
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

  return fact;
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


