/*
    Copyright (C) 2002 by Jorrit Tyberghein and Ryan Surkamp

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
#include "bcterldr.h"
#include "csgeom/vector3.h"
#include "csutil/parser.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "imesh/terrfunc.h"
#include "imesh/bcterr.h"
#include "imesh/object.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

// Place Token Defines Here

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (BLOCKSIZE)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MULTIPLIER)
  CS_TOKEN_DEF (LOD)
  CS_TOKEN_DEF (SYSDIST)
  CS_TOKEN_DEF (EDGE)
  CS_TOKEN_DEF (SIZE)
  CS_TOKEN_DEF (TOPLEFT)
  CS_TOKEN_DEF (HEIGHTMAP)
  CS_TOKEN_DEF (GROUPMATERIAL)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csBCTerrFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBCTerrLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBCTerrLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csBCTerrFactoryLoader)
SCF_IMPLEMENT_FACTORY (csBCTerrLoader)

SCF_EXPORT_CLASS_TABLE (bcterrldr)
  SCF_EXPORT_CLASS (csBCTerrFactoryLoader,
    "crystalspace.mesh.loader.factory.bcterr",
    "Crystal Space Bezier Curve Terrain Factory Loader")
  SCF_EXPORT_CLASS (csBCTerrLoader, "crystalspace.mesh.loader.bcterr",
    "Crystal Space Bezier Curve Terrain Loader")
SCF_EXPORT_CLASS_TABLE_END

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
// csBCTerrFactoryLoader
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------


csBCTerrFactoryLoader::csBCTerrFactoryLoader (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csBCTerrFactoryLoader::~csBCTerrFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csBCTerrFactoryLoader::Initialize (iObjectRegistry *object_reg)
{
  //printf("Terrain factory loader: Initialize\n");
  csBCTerrFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Init Factory Loader");
  return true;
}

iBase* csBCTerrFactoryLoader::Parse (const char* pString,
  iLoaderContext* ldr_context, iBase* /* context */)
{
  //printf("Terrain factory loader: Parsing\n");
  iMeshObjectType* pType = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
    "crystalspace.mesh.object.bcterr", iMeshObjectType);
  if (!pType)
  {
    pType = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.bcterr",
    iMeshObjectType);
    printf ("Loading plugin: crystalspace.mesh.object.bcterr\n");
    if (!pType)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Couldn't Load Object Type");
      return NULL;
    }
  }
  iMeshObjectFactory* pFactory = pType->NewFactory ();
  pType->DecRef ();
  iBCTerrFactoryState* iState = NULL;
  iState = SCF_QUERY_INTERFACE (pFactory, iBCTerrFactoryState);
  if (!iState)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Couldn't Get State");
    pFactory->DecRef ();
    return NULL;
  }
  // new loading stuff here
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (BLOCKSIZE)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MULTIPLIER)
    CS_TOKEN_TABLE (LOD)
    CS_TOKEN_TABLE (SYSDIST)
    CS_TOKEN_TABLE (EDGE)
  CS_TOKEN_TABLE_END

  char *pName;
  long cmd;
  char *pParams;
  char pStr[255];
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Entering Loop");
  csParser* parser = ldr_context->GetParser ();
  char* pBuf = (char*)pString;
  while ((cmd = parser->GetObject (&pBuf, commands, &pName, &pParams)) > 0)
  {
    if (!pParams)
    {
      // @@@ Error handling!
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_BLOCKSIZE:
      {
        float x, z;
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Blocksize");
        csScanStr (pParams, "%f,%f", &x, &z);
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","After Scan b4 Call");
        iState->SetBlockSize (x, z);				
      }
      break;
      case CS_TOKEN_MATERIAL:
      {
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Material");
        csScanStr (pParams, "%s", pStr);
        iMaterialWrapper* mat = NULL;
        mat = ldr_context->FindMaterial (pStr);
        if (!mat)
        {
          // @@@ Error handling!
          csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Couldn't Load Material");
          return NULL;
        }
        /*if ( !mat->GetMaterialHandle () )
        {
          csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Couldn't Get Material Handle");
          return NULL;
        }*/
        iState->SetDefaultMaterial (mat);
      }
      break;
      case CS_TOKEN_MULTIPLIER:
      {
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Multiplier");
        float n;
        csScanStr (pParams, "%f", &n);
        iState->SetMultiplier (n);
      }
      break;
      case CS_TOKEN_LOD:
      {
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","AddLOD");
        float distance;
        int inc;
        csScanStr (pParams, "%f,%d", &distance, &inc);
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","AddLOD call");
        iState->AddLOD (distance, inc);
      }
      break;
      case CS_TOKEN_SYSDIST:
      {
        float d;
        csScanStr (pParams, "%f", &d);
        iState->SetSystemDistance (d);
      }
      break;
      case CS_TOKEN_EDGE:
      {
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Edge");
        int edge_res;
        csScanStr (pParams, "%d", &edge_res);
        iState->SetMaxEdgeResolution (edge_res);
      }
      break;
    }
  }
  iState->DecRef ();
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Factory Loader","Created  Factory ");
  return pFactory;
}


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
// csBCTerrLoader
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

csBCTerrLoader::csBCTerrLoader (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  plugin_mgr = NULL;
}

csBCTerrLoader::~csBCTerrLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csBCTerrLoader::Initialize (iObjectRegistry* object_reg)
{
  //printf("Terrain BC loader: Initializing\n");
  csBCTerrLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"Terrain Loader","Init BC Terrain Loader");
  return true;
}

iBase* csBCTerrLoader::Parse (const char* pString,
  iLoaderContext* ldr_context, iBase* /* context */)
{
  //printf("Terrain BC loader: Parse\n");
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (SIZE)
    CS_TOKEN_TABLE (TOPLEFT)
    CS_TOKEN_TABLE (HEIGHTMAP)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (GROUPMATERIAL)
  CS_TOKEN_TABLE_END
  
  char *pName;
  long cmd;
  char *pParams;
  char pStr[255];

  iMeshObject* iTerrObj = NULL;
  iBCTerrState* iState = NULL;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Loader","Parse");
  csParser* parser = ldr_context->GetParser ();
  char* pBuf = (char*)pString;
  while ((cmd = parser->GetObject (&pBuf, commands, &pName, &pParams)) > 0)
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
        iMeshFactoryWrapper* iFactory = ldr_context->FindMeshFactory (pStr);
        if (!iFactory)
        {
          // @@@ Error handling!
          return NULL;
        }
        iTerrObj = iFactory->GetMeshObjectFactory()->NewInstance();
        iState = SCF_QUERY_INTERFACE (iTerrObj, iBCTerrState);
      }
      break;
      case CS_TOKEN_SIZE:
      {
        int x, z;
        csScanStr (pParams, "%d,%d", &x, &z);
        if (iState)
        {
          iState->SetSize (x, z);
        }
      }
      break;
      case CS_TOKEN_TOPLEFT:
      {
        csVector3 topleft;
        csScanStr (pParams, "%f, %f, %f", &topleft.x, &topleft.y, &topleft.z);
        if (iState)
        {
          iState->SetTopLeftCorner (topleft);
        }
      }
      break;
      case CS_TOKEN_HEIGHTMAP:
      {
        csScanStr (pParams, "%s", pStr);
        iVFS* vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
        if (!vfs)
        {
          printf ("No VFS!\n");
          exit (0);
        }
        iImageIO* loader = CS_QUERY_REGISTRY (object_reg, iImageIO);
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
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Loader","Set HeightMap");
        iState->SetHeightMap (ifile);
        ifile->DecRef ();
        buf->DecRef ();
        vfs->DecRef ();
        loader->DecRef ();
      }
      break;
      case CS_TOKEN_MATERIAL:
      {
        int x, z;
        csScanStr (pParams, "%d,%d,%s", &x, &z, pStr);
        iMaterialWrapper* mat = ldr_context->FindMaterial (pStr);
        if (mat && iState)
        {
          iState->SetBlockMaterial (x, z, mat);					
        }
        
      }
      break;
      case CS_TOKEN_GROUPMATERIAL:
      {
        int i, rangeStart, rangeEnd, iter;
        bool done;
        char pMatName[256];
        done = false;
        i = 0;
        iter = 0;
	csScanStr (pParams, "%s,%d,%d", pStr, &rangeStart, &rangeEnd);
        for (i = rangeStart; i <= rangeEnd; i++) 
        {
          sprintf (pMatName, pStr, i);
          iMaterialWrapper* mat = ldr_context->FindMaterial (pMatName);
          if (mat)
          {
            if (iState)
              iState->SetBlockMaterialNum (iter, mat);
            iter++;
          }
        }
      }
      break;
    }
  }
  iState->DecRef ();
  return iTerrObj;
}


