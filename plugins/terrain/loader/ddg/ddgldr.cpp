/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Portions written by Richard D Shank

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
#include "ddgldr.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "iterrain/ddg.h"
#include "iterrain/object.h"
#include "iengine/terrain.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (HEIGHTMAP)
  CS_TOKEN_DEF (LOD)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (GROUPMATERIAL)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csDDGFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csDDGLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csDDGFactoryLoader)
IMPLEMENT_FACTORY (csDDGLoader)

EXPORT_CLASS_TABLE (ddgldr)
  EXPORT_CLASS (csDDGFactoryLoader, "crystalspace.terrain.loader.factory.ddg",
    "Crystal Space DDG Terrain Factory Loader")
  EXPORT_CLASS (csDDGLoader, "crystalspace.terrain.loader.ddg",
    "Crystal Space DDG Terrain Loader")
EXPORT_CLASS_TABLE_END

csDDGFactoryLoader::csDDGFactoryLoader( iBase* pParent )
{
  CONSTRUCT_IBASE (pParent);
}

csDDGFactoryLoader::~csDDGFactoryLoader()
{
}

bool csDDGFactoryLoader::Initialize( iSystem* pSys )
{
  pSystem = pSys;
  return true;
}

iBase* csDDGFactoryLoader::Parse( const char* /*string*/, iEngine* /*engine*/, iBase* /* context */)
{
  iTerrainObjectType* pType = QUERY_PLUGIN_CLASS( pSystem, "crystalspace.terrain.object.ddg", "TerrainObj", iTerrainObjectType);
  if (!pType)
  {
    pType = LOAD_PLUGIN( pSystem, "crystalspace.terrain.object.ddg", "TerrainObj", iTerrainObjectType);
    printf ("Load TYPE plugin crystalspace.terrain.object.ddg\n");
  }
  iTerrainObjectFactory* pFactory = pType->NewFactory ();
  return pFactory;
}

//---------------------------------------------------------------------------

csDDGLoader::csDDGLoader( iBase* pParent )
{
  CONSTRUCT_IBASE( pParent );
}

csDDGLoader::~csDDGLoader()
{
}

bool csDDGLoader::Initialize( iSystem* pSys )
{
  pSystem = pSys;
  return true;
}

iBase* csDDGLoader::Parse( const char* pString, iEngine *iEngine, iBase* /* context */)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (HEIGHTMAP)
    CS_TOKEN_TABLE (LOD)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (GROUPMATERIAL)
  CS_TOKEN_TABLE_END

  char *pName;
  long cmd;
  char *pParams;
  char pStr[255];

  iTerrainObject* iTerrObj = NULL;
  iDDGState* iTerrainState = NULL;

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
          ScanStr( pParams, "%s", pStr);
	  iTerrainFactoryWrapper* iFactory = iEngine->FindTerrainFactory( pStr );
	  if (!iFactory)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  iTerrObj = iFactory->GetTerrainObjectFactory()->NewInstance();
          iTerrainState = QUERY_INTERFACE( iTerrObj, iDDGState );
          iTerrainState->SetEngine( iEngine );
	}
	break;
      case CS_TOKEN_GROUPMATERIAL:
	{
	  int rangeStart, rangeEnd;
	  ScanStr( pParams, "%s,%d,%d", pStr, &rangeStart, &rangeEnd );
	  iTerrainState->LoadMaterialGroup( pStr, rangeStart, rangeEnd );
	}
    	break;
      case CS_TOKEN_HEIGHTMAP:
	{
          ScanStr( pParams, "%s", pStr);
	  iTerrainState->LoadHeightMap( pStr );
	}
	break;  
      case CS_TOKEN_LOD:
        {
          int detailLevel;
          ScanStr( pParams, "%d", &detailLevel );
          iTerrainState->SetLOD( detailLevel );
        }
        break;
      case CS_TOKEN_MATERIAL:
	{
          ScanStr( pParams, "%s", pStr);
	  iTerrainState->LoadMaterial( pStr );
	}
        break;
    }
  }
  iTerrainState->DecRef();

  return iTerrObj;
}

//---------------------------------------------------------------------------


