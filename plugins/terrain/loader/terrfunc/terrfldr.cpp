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
#include "terrfldr.h"
#include "csgeom/vector3.h"
#include "csutil/parser.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "iterrain/terrfunc.h"
#include "iterrain/object.h"
#include "iengine/terrain.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (GROUPMATERIAL)
  CS_TOKEN_DEF (BLOCKS)
  CS_TOKEN_DEF (GRID)
  CS_TOKEN_DEF (TOPLEFT)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (DIRLIGHT)
  CS_TOKEN_DEF (LODDIST)
  CS_TOKEN_DEF (LODCOST)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csTerrFuncFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csTerrFuncLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csTerrFuncFactoryLoader)
IMPLEMENT_FACTORY (csTerrFuncLoader)

EXPORT_CLASS_TABLE (terrfldr)
  EXPORT_CLASS (csTerrFuncFactoryLoader,
    "crystalspace.terrain.loader.factory.terrfunc",
    "Crystal Space Function Terrain Factory Loader")
  EXPORT_CLASS (csTerrFuncLoader, "crystalspace.terrain.loader.terrfunc",
    "Crystal Space Function Terrain Loader")
EXPORT_CLASS_TABLE_END

csTerrFuncFactoryLoader::csTerrFuncFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csTerrFuncFactoryLoader::~csTerrFuncFactoryLoader ()
{
}

bool csTerrFuncFactoryLoader::Initialize (iSystem* pSys)
{
  pSystem = pSys;
  return true;
}

iBase* csTerrFuncFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iTerrainObjectType* pType = QUERY_PLUGIN_CLASS (pSystem,
  	"crystalspace.terrain.object.terrfunc", "TerrainObj",
	iTerrainObjectType);
  if (!pType)
  {
    pType = LOAD_PLUGIN (pSystem, "crystalspace.terrain.object.terrfunc",
    	"TerrainObj", iTerrainObjectType);
    printf ("Load TYPE plugin crystalspace.terrain.object.terrfunc\n");
  }
  iTerrainObjectFactory* pFactory = pType->NewFactory ();
  return pFactory;
}

//---------------------------------------------------------------------------

csTerrFuncLoader::csTerrFuncLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csTerrFuncLoader::~csTerrFuncLoader ()
{
}

bool csTerrFuncLoader::Initialize (iSystem* pSys)
{
  pSystem = pSys;
  return true;
}

iBase* csTerrFuncLoader::Parse (const char* pString, iEngine *iEngine,
	iBase* /* context */)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (GROUPMATERIAL)
    CS_TOKEN_TABLE (BLOCKS)
    CS_TOKEN_TABLE (GRID)
    CS_TOKEN_TABLE (TOPLEFT)
    CS_TOKEN_TABLE (SCALE)
    CS_TOKEN_TABLE (DIRLIGHT)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (LODDIST)
    CS_TOKEN_TABLE (LODCOST)
  CS_TOKEN_TABLE_END

  char *pName;
  long cmd;
  char *pParams;
  char pStr[255];

  iTerrainObject* iTerrObj = NULL;
  iTerrFuncState* iTerrainState = NULL;

  char* pBuf = (char*)pString;
  while ((cmd = csGetObject (&pBuf, commands, &pName, &pParams)) > 0)
  {
    if (!pParams)
    {
      // @@@ Error handling!
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
	{
          ScanStr (pParams, "%s", pStr);
	  iTerrainFactoryWrapper* iFactory = iEngine->FindTerrainFactory (pStr);
	  if (!iFactory)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  iTerrObj = iFactory->GetTerrainObjectFactory()->NewInstance();
          iTerrainState = QUERY_INTERFACE (iTerrObj, iTerrFuncState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
	  int i;
          ScanStr (pParams, "%d,%s", &i, pStr);
          iMaterialWrapper* mat = iEngine->FindMaterial (pStr);
	  if (!mat)
	  {
            // @@@ Error handling!
            return NULL;
	  }
	  iTerrObj->SetMaterial (i, mat);
	}
        break;
      case CS_TOKEN_GROUPMATERIAL:
	{
	  int rangeStart, rangeEnd;
	  ScanStr (pParams, "%s,%d,%d", pStr, &rangeStart, &rangeEnd);
	  iTerrainState->LoadMaterialGroup (iEngine, pStr,
	  	rangeStart, rangeEnd);
	}
    	break;
      case CS_TOKEN_BLOCKS:
	{
	  int bx, by;
	  ScanStr (pParams, "%d,%d", &bx, &by);
          iTerrainState->SetResolution (bx, by);
        }
        break;
      case CS_TOKEN_GRID:
	{
	  int bx, by;
	  ScanStr (pParams, "%d,%d", &bx, &by);
          iTerrainState->SetGridResolution (bx, by);
        }
        break;
      case CS_TOKEN_TOPLEFT:
	{
	  csVector3 tl;
	  ScanStr (pParams, "%f,%f,%f", &tl.x, &tl.y, &tl.z);
          iTerrainState->SetTopLeftCorner (tl);
        }
        break;
      case CS_TOKEN_SCALE:
	{
	  csVector3 s;
	  ScanStr (pParams, "%f,%f,%f", &s.x, &s.y, &s.z);
          iTerrainState->SetScale (s);
        }
        break;
      case CS_TOKEN_COLOR:
        {
	  csColor col;
	  ScanStr (pParams, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  iTerrainState->SetColor (col);
	}
	break;
      case CS_TOKEN_DIRLIGHT:
        {
	  csVector3 pos;
	  csColor col;
	  ScanStr (pParams, "%f,%f,%f:%f,%f,%f", &pos.x, &pos.y, &pos.z,
	  	&col.red, &col.green, &col.blue);
	  iTerrObj->SetDirLight (pos, col);
	}
	break;
      case CS_TOKEN_LODDIST:
        {
	  int lod;
	  float dist;
	  ScanStr (pParams, "%d,%f", &lod, &dist);
	  iTerrainState->SetLODDistance (lod, dist);
	}
	break;
      case CS_TOKEN_LODCOST:
        {
	  int lod;
	  float cost;
	  ScanStr (pParams, "%d,%f", &lod, &cost);
	  iTerrainState->SetMaximumLODCost (lod, cost);
	}
	break;
    }
  }
  iTerrainState->DecRef ();

  return iTerrObj;
}

//---------------------------------------------------------------------------


