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
#include "iutil/document.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
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
  CS_TOKEN_DEF (CORRECTSEAMS)
  CS_TOKEN_DEF (C)
  CS_TOKEN_DEF (H)
  CS_TOKEN_DEF (BC)
  CS_TOKEN_DEF (BH)
  CS_TOKEN_DEF (BUILD)
  CS_TOKEN_DEF (FLATTEN)
  CS_TOKEN_DEF (DOFLATTEN)
  CS_TOKEN_DEF (SYSINC)
CS_TOKEN_DEF_END

enum
{
  XMLTOKEN_FACTORY = 1,
  XMLTOKEN_BLOCKSIZE,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MULTIPLIER,
  XMLTOKEN_LOD,
  XMLTOKEN_SYSDIST,
  XMLTOKEN_EDGE,
  XMLTOKEN_SIZE,
  XMLTOKEN_TOPLEFT,
  XMLTOKEN_HEIGHTMAP,
  XMLTOKEN_CORRECTSEAMS,
  XMLTOKEN_C,
  XMLTOKEN_H,
  XMLTOKEN_BC,
  XMLTOKEN_BH,
  XMLTOKEN_BUILD,
  XMLTOKEN_FLATTEN,
  XMLTOKEN_DOFLATTEN,
  XMLTOKEN_SYSINC
};

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
  synldr = NULL;
}

csBCTerrFactoryLoader::~csBCTerrFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
}

bool csBCTerrFactoryLoader::Initialize (iObjectRegistry *object_reg)
{
  csBCTerrFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("blocksize", XMLTOKEN_BLOCKSIZE);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("multiplier", XMLTOKEN_MULTIPLIER);
  xmltokens.Register ("lod", XMLTOKEN_LOD);
  xmltokens.Register ("sysdist", XMLTOKEN_SYSDIST);
  xmltokens.Register ("edge", XMLTOKEN_EDGE);
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
        float d, s;
        csScanStr (pParams, "%f, %f", &s, &d);
        iState->SetSystemDistance (s, d);
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

iBase* csBCTerrFactoryLoader::Parse (iDocumentNode* node,
  iLoaderContext* ldr_context, iBase* /* context */)
{
  iMeshObjectType* pType = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
    "crystalspace.mesh.object.bcterr", iMeshObjectType);
  if (!pType)
  {
    pType = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.bcterr",
    iMeshObjectType);
    if (!pType)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "Factory Loader",
      	"Couldn't Load Object Type");
      return NULL;
    }
  }

  csRef<iMeshObjectFactory> pFactory (pType->NewFactory ());
  pType->DecRef ();
  csRef<iBCTerrFactoryState> iState;
  iState = SCF_QUERY_INTERFACE (pFactory, iBCTerrFactoryState);
  if (!iState)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
    	"Factory Loader", "Couldn't Get State");
    return NULL;
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_BLOCKSIZE:
        {
          float x, z;
	  x = child->GetAttributeValueAsFloat ("x");
	  z = child->GetAttributeValueAsFloat ("y");
          iState->SetBlockSize (x, z);				
        }
        break;
      case XMLTOKEN_MATERIAL:
        {
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
          if (!mat)
          {
      	    synldr->ReportError ("crystalspace.bcterrldr.parse.unknownmaterial",
		  child, "Couldn't find material '%s'!", matname);
          }
          iState->SetDefaultMaterial (mat);
        }
        break;
      case XMLTOKEN_MULTIPLIER:
        iState->SetMultiplier (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_LOD:
        {
	  csRef<iDocumentNode> distnode = child->GetNode ("distance");
	  if (!distnode)
	  {
	    synldr->ReportError ("crystalspace.bcterrldr.parse",
	    	child, "Could not find 'distance' node!");
	    return NULL;
	  }
          float distance = distnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> incnode = child->GetNode ("inc");
	  if (!incnode)
	  {
	    synldr->ReportError ("crystalspace.bcterrldr.parse",
	    	child, "Could not find 'inc' node!");
	    return NULL;
	  }
          int inc = incnode->GetContentsValueAsInt ();
          iState->AddLOD (distance, inc);
        }
        break;
      case XMLTOKEN_SYSDIST:
        {
	  csRef<iDocumentNode> distnode = child->GetNode ("distance");
	  if (!distnode)
	  {
	    synldr->ReportError ("crystalspace.bcterrldr.parse",
	    	child, "Could not find 'distance' node!");
	    return NULL;
	  }
          float d = distnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> startnode = child->GetNode ("start");
	  if (!startnode)
	  {
	    synldr->ReportError ("crystalspace.bcterrldr.parse",
	    	child, "Could not find 'start' node!");
	    return NULL;
	  }
          float s = startnode->GetContentsValueAsFloat ();
          iState->SetSystemDistance (s, d);
        }
        break;
      case XMLTOKEN_EDGE:
        iState->SetMaxEdgeResolution (child->GetContentsValueAsInt ());
        break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }
  // To avoid smart pointer release.
  if (pFactory) pFactory->IncRef ();
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

  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("size", XMLTOKEN_SIZE);
  xmltokens.Register ("topleft", XMLTOKEN_TOPLEFT);
  xmltokens.Register ("heightmap", XMLTOKEN_HEIGHTMAP);
  xmltokens.Register ("correctseams", XMLTOKEN_CORRECTSEAMS);
  xmltokens.Register ("c", XMLTOKEN_C);
  xmltokens.Register ("h", XMLTOKEN_H);
  xmltokens.Register ("bc", XMLTOKEN_BC);
  xmltokens.Register ("bh", XMLTOKEN_BH);
  xmltokens.Register ("build", XMLTOKEN_BUILD);
  xmltokens.Register ("flatten", XMLTOKEN_FLATTEN);
  xmltokens.Register ("doflatten", XMLTOKEN_DOFLATTEN);
  xmltokens.Register ("sysinc", XMLTOKEN_SYSINC);
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
    CS_TOKEN_TABLE (CORRECTSEAMS)    
    CS_TOKEN_TABLE (C)
    CS_TOKEN_TABLE (H)
    CS_TOKEN_TABLE (BC)
    CS_TOKEN_TABLE (BH)
    CS_TOKEN_TABLE (BUILD)
    CS_TOKEN_TABLE (FLATTEN)
    CS_TOKEN_TABLE (DOFLATTEN)
    CS_TOKEN_TABLE (SYSINC)
  CS_TOKEN_TABLE_END
  
  char *pName;
  long cmd;
  char *pParams;
  char pStr[255];

  iMeshObject* iTerrObj = NULL;
  iBCTerrState* iState = NULL;
  iTerrFuncState* iTerrFunc = NULL;
  int group_iter = 0;
  int cp_iter = 0;
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
        iTerrFunc = SCF_QUERY_INTERFACE (iTerrObj, iTerrFuncState);
      }
      break;
      case CS_TOKEN_SIZE:
      {
        int x, z;
        csScanStr (pParams, "%d,%d", &x, &z);
        if (iState)
        {
          iState->SetSize (x, z);
          iState->PreBuild ();
        }
      }
      break;
      case CS_TOKEN_SYSINC:
      {
          int i;
          csScanStr (pParams, "%d", &i);
          if (iState)
            iState->SetSystemInc (i);

      }
      break;
      case CS_TOKEN_BUILD:
      {
        if (iState)
          iState->Build ();
      }
      break;
      case CS_TOKEN_FLATTEN:
      {
        float t, r, d, l;
        csScanStr (pParams, "%f, %f, %f, %f", &t, &d, &l, &r);
        if (iState)
          iState->SetFlattenHeight (t, d,l,r);
      }
      break;
      case CS_TOKEN_DOFLATTEN:
      {
        bool t, r, d, l;
        csScanStr (pParams, "%b, %b, %b, %b", &t, &d, &l, &r);
        if (iState)
          iState->DoFlatten (t, d,l,r);
      }
      break;
      case CS_TOKEN_C:
      {
        csVector3 cp;
        csScanStr (pParams, "%f, %f, %f", &cp.x, &cp.y, &cp.z);
        if (iState)
        {
          iState->SetControlPoint (cp, cp_iter);
          cp_iter++;
        }
      }
      break;
      case CS_TOKEN_H:
      {
        float h;
        csScanStr (pParams, "%f", &h);
        if (iState)
        {
          iState->SetControlPointHeight (h, cp_iter);
          cp_iter++;
        }
      }
      break;
      case CS_TOKEN_BC:
      {
        csVector3 cp;
        int x, z;
        csScanStr (pParams, "%d, %d, %f, %f, %f", &x, &z, &cp.x, &cp.y, &cp.z);
        if (iState)
          iState->SetControlPoint (cp, x, z);
      }
      break;
      case CS_TOKEN_BH:
      {
        float h;
        int x, z;
        csScanStr (pParams, "%d, %d, %f", &x, &z, &h );
        if (iState)
          iState->SetControlPointHeight (h, x, z);
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

        csRef<iDataBuffer> buf (vfs->ReadFile (pStr));
        if (!buf || !buf->GetSize ())
        {
          printf ("Can't open file '%s' in vfs!\n", pStr);
          exit (0);
        }
        csRef<iImage> ifile (loader->Load (buf->GetUint8 (), buf->GetSize (),
          CS_IMGFMT_TRUECOLOR));
        if (!ifile)
        {
          printf ("Error loading image '%s'!\n", pStr);
          exit (0);
        }
        //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"BC Loader","Set HeightMap");
        iState->SetHeightMap (ifile);
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
      case CS_TOKEN_CORRECTSEAMS:
      {
        int x, y;
        csScanStr (pParams, "%d,%d", &x, &y, pStr);
        if (iTerrFunc)
        {
          iTerrFunc->CorrectSeams (x, y);
        }
      }
      break;
      case CS_TOKEN_GROUPMATERIAL:
      {
        int i, rangeStart, rangeEnd;
        bool done;
        char pMatName[256];
        done = false;
        i = 0;
	csScanStr (pParams, "%s,%d,%d", pStr, &rangeStart, &rangeEnd);
        for (i = rangeStart; i <= rangeEnd; i++) 
        {
          sprintf (pMatName, pStr, i);
          iMaterialWrapper* mat = ldr_context->FindMaterial (pMatName);
          if (mat)
          {
            if (iState)
              iState->SetBlockMaterialNum (group_iter, mat);
            group_iter++;
          }
        }
      }
      break;
    }
  }
  iState->DecRef ();
  return iTerrObj;
}

iBase* csBCTerrLoader::Parse (iDocumentNode* node,
  iLoaderContext* ldr_context, iBase* /* context */)
{
  csRef<iMeshObject> iTerrObj;
  csRef<iBCTerrState> iState;
  csRef<iTerrFuncState> iTerrFunc;
  int group_iter = 0;
  int cp_iter = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FACTORY:
        {
          const char* factname = child->GetContentsValue ();
          iMeshFactoryWrapper* iFactory = ldr_context->FindMeshFactory (
	  	factname);
          if (!iFactory)
          {
      	    synldr->ReportError ("crystalspace.bcterrldr.parse.unknownfactory",
		    child, "Couldn't find factory '%s'!", factname);
            return NULL;
          }
          iTerrObj = iFactory->GetMeshObjectFactory()->NewInstance();
          iState = SCF_QUERY_INTERFACE (iTerrObj, iBCTerrState);
          iTerrFunc = SCF_QUERY_INTERFACE (iTerrObj, iTerrFuncState);
        }
        break;
      case XMLTOKEN_SIZE:
        {
	  int x = child->GetAttributeValueAsInt ("w");
	  int z = child->GetAttributeValueAsInt ("h");
          iState->SetSize (x, z);
          iState->PreBuild ();
        }
        break;
      case XMLTOKEN_SYSINC:
        iState->SetSystemInc (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_BUILD:
        iState->Build ();
        break;
      case XMLTOKEN_FLATTEN:
        {
          float t = 0, r = 1, d = 1, l = 0;
	  csRef<iDocumentNode> upnode = child->GetNode ("up");
	  if (upnode) t = upnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> downnode = child->GetNode ("down");
	  if (downnode) d = downnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> leftnode = child->GetNode ("left");
	  if (leftnode) l = leftnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> rightnode = child->GetNode ("right");
	  if (rightnode) r = rightnode->GetContentsValueAsFloat ();
          iState->SetFlattenHeight (t, d,l,r);
        }
        break;
      case XMLTOKEN_DOFLATTEN:
        {
          bool t = false, r = false, d = false, l = false;
	  csRef<iDocumentNode> upnode = child->GetNode ("up");
	  if (upnode) if (!synldr->ParseBool (upnode, t, true)) return NULL;
	  csRef<iDocumentNode> downnode = child->GetNode ("down");
	  if (downnode) if (!synldr->ParseBool (downnode, d, true)) return NULL;
	  csRef<iDocumentNode> leftnode = child->GetNode ("left");
	  if (leftnode) if (!synldr->ParseBool (leftnode, l, true)) return NULL;
	  csRef<iDocumentNode> rightnode = child->GetNode ("right");
	  if (rightnode) if (!synldr->ParseBool (rightnode, r, true)) return NULL;
          iState->DoFlatten (t, d,l,r);
        }
        break;
      case XMLTOKEN_C:
        {
          csVector3 cp;
	  if (!synldr->ParseVector (child, cp))
	    return NULL;
          iState->SetControlPoint (cp, cp_iter);
          cp_iter++;
        }
        break;
      case XMLTOKEN_H:
        iState->SetControlPointHeight (child->GetContentsValueAsFloat (),
		cp_iter);
        cp_iter++;
        break;
      case XMLTOKEN_BC:
        {
          csVector3 cp;
          int x, z;
	  cp.x = child->GetAttributeValueAsFloat ("x");
	  cp.y = child->GetAttributeValueAsFloat ("y");
	  cp.z = child->GetAttributeValueAsFloat ("z");
	  x = child->GetAttributeValueAsInt ("cpx");
	  z = child->GetAttributeValueAsInt ("cpy");
          iState->SetControlPoint (cp, x, z);
        }
        break;
      case XMLTOKEN_BH:
        {
          float h;
          int x, z;
	  h = child->GetAttributeValueAsFloat ("height");
	  x = child->GetAttributeValueAsInt ("cpx");
	  z = child->GetAttributeValueAsInt ("cpy");
          iState->SetControlPointHeight (h, x, z);
        }
        break;
      case XMLTOKEN_TOPLEFT:
        {
          csVector3 topleft;
	  if (!synldr->ParseVector (child, topleft))
	    return NULL;
          iState->SetTopLeftCorner (topleft);
        }
        break;
      case XMLTOKEN_HEIGHTMAP:
        {
	  const char* mapname = child->GetContentsValue ();
          csRef<iVFS> vfs (CS_QUERY_REGISTRY (object_reg, iVFS));
          csRef<iImageIO> loader (CS_QUERY_REGISTRY (object_reg, iImageIO));
          csRef<iDataBuffer> buf (vfs->ReadFile (mapname));
          if (!buf || !buf->GetSize ())
          {
	    synldr->ReportError ("crystalspace.bcterrldr.parse",
	      child, "Can't open file '%s' in vfs!", mapname);
	    return NULL;
          }
          csRef<iImage> ifile (loader->Load (buf->GetUint8 (), buf->GetSize (),
            		CS_IMGFMT_TRUECOLOR));
          if (!ifile)
          {
	    synldr->ReportError ("crystalspace.bcterrldr.parse",
	      child, "Can't load image '%s'!", mapname);
	    return NULL;
          }
          iState->SetHeightMap (ifile);
        }
        break;
      case XMLTOKEN_MATERIAL:
        {
	  const char* matname = child->GetContentsValue ();
	  csRef<iDocumentAttribute> attr;
	  attr = child->GetAttribute ("start");
	  if (attr != NULL)
	  {
	    int start = attr->GetValueAsInt ();
	    attr = child->GetAttribute ("end");
	    if (attr == NULL)
	    {
      	      synldr->ReportError (
	    	  "crystalspace.bcterrloader.parse",
		  child, "'end' attribute missing!");
              return NULL;
	    }
	    int end = attr->GetValueAsInt ();
	    int i = 0;
            for (i = start ; i <= end ; i++) 
            {
	      char matn[1024];
              sprintf (matn, matname, i);
	      iMaterialWrapper* mat = ldr_context->FindMaterial (matn);
	      if (mat)
              {
                iState->SetBlockMaterialNum (group_iter, mat);
                group_iter++;
              }
            }
	  }
	  else
	  {
            iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	    if (!mat)
	    {
	      synldr->ReportError ("crystalspace.bcterrldr.parse",
	        child, "Can't find material '%s'!", matname);
	      return NULL;
	    }
            int x, z;
	    x = child->GetAttributeValueAsInt ("x");
	    z = child->GetAttributeValueAsInt ("y");
            iState->SetBlockMaterial (x, z, mat);
	  }
        }
        break;
      case XMLTOKEN_CORRECTSEAMS:
        {
          int x, y;
	  x = child->GetAttributeValueAsInt ("w");
	  y = child->GetAttributeValueAsInt ("h");
          iTerrFunc->CorrectSeams (x, y);
        }
        break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }
  // To avoid smart pointer from releasing.
  if (iTerrObj) iTerrObj->IncRef ();
  return iTerrObj;
}


