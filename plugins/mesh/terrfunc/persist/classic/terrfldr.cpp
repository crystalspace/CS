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
#include "isys/vfs.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "imesh/terrfunc.h"
#include "imesh/object.h"
#include "iengine/mesh.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (CORRECTSEAMS)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (GROUPMATERIAL)
  CS_TOKEN_DEF (BLOCKS)
  CS_TOKEN_DEF (GRID)
  CS_TOKEN_DEF (HEIGHTMAP)
  CS_TOKEN_DEF (TOPLEFT)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (DIRLIGHT)
  CS_TOKEN_DEF (LODDIST)
  CS_TOKEN_DEF (LODCOST)
  CS_TOKEN_DEF (QUADDEPTH)
  CS_TOKEN_DEF (VISTEST)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csTerrFuncFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncFactoryLoader::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csTerrFuncLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncLoader::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTerrFuncFactoryLoader)
SCF_IMPLEMENT_FACTORY (csTerrFuncLoader)

SCF_EXPORT_CLASS_TABLE (terrfldr)
  SCF_EXPORT_CLASS (csTerrFuncFactoryLoader,
    "crystalspace.mesh.loader.factory.terrfunc",
    "Crystal Space Function Terrain Factory Loader")
  SCF_EXPORT_CLASS (csTerrFuncLoader, "crystalspace.mesh.loader.terrfunc",
    "Crystal Space Function Terrain Loader")
SCF_EXPORT_CLASS_TABLE_END

csTerrFuncFactoryLoader::csTerrFuncFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csTerrFuncFactoryLoader::~csTerrFuncFactoryLoader ()
{
}

iBase* csTerrFuncFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* pType = CS_QUERY_PLUGIN_CLASS (pSystem,
  	"crystalspace.mesh.object.terrfunc", "MeshObj",
	iMeshObjectType);
  if (!pType)
  {
    pType = CS_LOAD_PLUGIN (pSystem, "crystalspace.mesh.object.terrfunc",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.terrfunc\n");
  }
  iMeshObjectFactory* pFactory = pType->NewFactory ();
  return pFactory;
}

//---------------------------------------------------------------------------

csTerrFuncLoader::csTerrFuncLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csTerrFuncLoader::~csTerrFuncLoader ()
{
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
    CS_TOKEN_TABLE (HEIGHTMAP)
    CS_TOKEN_TABLE (TOPLEFT)
    CS_TOKEN_TABLE (SCALE)
    CS_TOKEN_TABLE (DIRLIGHT)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (LODDIST)
    CS_TOKEN_TABLE (LODCOST)
    CS_TOKEN_TABLE (CORRECTSEAMS)
    CS_TOKEN_TABLE (QUADDEPTH)
    CS_TOKEN_TABLE (VISTEST)
  CS_TOKEN_TABLE_END

  char *pName;
  long cmd;
  char *pParams;
  char pStr[255];

  iMeshObject* iTerrObj = NULL;
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
          csScanStr (pParams, "%s", pStr);
	  iMeshFactoryWrapper* iFactory = iEngine->FindMeshFactory (pStr);
	  if (!iFactory)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  iTerrObj = iFactory->GetMeshObjectFactory()->NewInstance();
          iTerrainState = SCF_QUERY_INTERFACE (iTerrObj, iTerrFuncState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
	  int i;
          csScanStr (pParams, "%d,%s", &i, pStr);
          iMaterialWrapper* mat = iEngine->FindMaterial (pStr);
	  if (!mat)
	  {
            // @@@ Error handling!
            return NULL;
	  }
	  iTerrainState->SetMaterial (i, mat);
	}
        break;
      case CS_TOKEN_GROUPMATERIAL:
	{
	  int rangeStart, rangeEnd;
	  csScanStr (pParams, "%s,%d,%d", pStr, &rangeStart, &rangeEnd);
	  iTerrainState->LoadMaterialGroup (iEngine, pStr,
	  	rangeStart, rangeEnd);
	}
    	break;
      case CS_TOKEN_CORRECTSEAMS:
	{
	  int tw, th;
	  csScanStr (pParams, "%d,%d", &tw, &th);
          iTerrainState->CorrectSeams (tw, th);
        }
        break;
      case CS_TOKEN_BLOCKS:
	{
	  int bx, by;
	  csScanStr (pParams, "%d,%d", &bx, &by);
          iTerrainState->SetResolution (bx, by);
        }
        break;
      case CS_TOKEN_GRID:
	{
	  int bx, by;
	  csScanStr (pParams, "%d,%d", &bx, &by);
          iTerrainState->SetGridResolution (bx, by);
        }
        break;
      case CS_TOKEN_TOPLEFT:
	{
	  csVector3 tl;
	  csScanStr (pParams, "%f,%f,%f", &tl.x, &tl.y, &tl.z);
          iTerrainState->SetTopLeftCorner (tl);
        }
        break;
      case CS_TOKEN_SCALE:
	{
	  csVector3 s;
	  csScanStr (pParams, "%f,%f,%f", &s.x, &s.y, &s.z);
          iTerrainState->SetScale (s);
        }
        break;
      case CS_TOKEN_COLOR:
        {
	  csColor col;
	  csScanStr (pParams, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  iTerrainState->SetColor (col);
	}
	break;
      case CS_TOKEN_DIRLIGHT:
        {
	  csVector3 pos;
	  csColor col;
	  csScanStr (pParams, "%f,%f,%f:%f,%f,%f", &pos.x, &pos.y, &pos.z,
	  	&col.red, &col.green, &col.blue);
	  iTerrainState->SetDirLight (pos, col);
	}
	break;
      case CS_TOKEN_LODDIST:
        {
	  int lod;
	  float dist;
	  csScanStr (pParams, "%d,%f", &lod, &dist);
	  iTerrainState->SetLODDistance (lod, dist);
	}
	break;
      case CS_TOKEN_LODCOST:
        {
	  int lod;
	  float cost;
	  csScanStr (pParams, "%d,%f", &lod, &cost);
	  iTerrainState->SetMaximumLODCost (lod, cost);
	}
	break;
      case CS_TOKEN_QUADDEPTH:
        {
	  int qd;
	  csScanStr (pParams, "%d", &qd);
	  iTerrainState->SetQuadDepth (qd);
	}
        break;
      case CS_TOKEN_VISTEST:
        {
	  bool vt;
	  csScanStr (pParams, "%b", &vt);
	  iTerrainState->SetVisTesting (vt);
	}
        break;
      case CS_TOKEN_HEIGHTMAP:
        {
	  float hscale, hshift;
	  csScanStr (pParams, "%s,%f,%f\n", pStr, &hscale, &hshift);
	  iVFS* vfs = CS_QUERY_PLUGIN (pSystem, iVFS);
	  if (!vfs)
	  {
	    printf ("No VFS!\n");
	    exit (0);
	  }
	  iImageIO* loader = CS_QUERY_PLUGIN_ID (pSystem,
	  	CS_FUNCID_IMGLOADER, iImageIO);
	  if (!loader)
	  {
	    printf ("No image loader!\n");
	    exit (0);
	  }

	  iDataBuffer* buf = vfs->ReadFile (pStr);
	  if (!buf || !buf->GetSize ())
	  {
	    printf ("Can't open file '%s' in vfs!\n", pStr);
	    exit (0);
	  }
	  iImage* ifile = loader->Load (buf->GetUint8 (), buf->GetSize (),
	  	CS_IMGFMT_TRUECOLOR);
	  if (!ifile)
	  {
	    printf ("Error loading image '%s'!\n", pStr);
	    exit (0);
	  }
	  iTerrainState->SetHeightMap (ifile, hscale, hshift);
	  ifile->DecRef ();
	  buf->DecRef ();
	  loader->DecRef ();
	  vfs->DecRef ();
	}
	break;
    }
  }
  iTerrainState->DecRef ();

  return iTerrObj;
}

//---------------------------------------------------------------------------


