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
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "imesh/terrfunc.h"
#include "imesh/object.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "ivaria/reporter.h"

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
  CS_TOKEN_DEF (ALLMATERIAL) // For testing purposes. Use after BLOCKS and GRID are set
CS_TOKEN_DEF_END

enum
{
  XMLTOKEN_FACTORY = 1,
  XMLTOKEN_COLOR,
  XMLTOKEN_CORRECTSEAMS,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_BLOCKS,
  XMLTOKEN_GRID,
  XMLTOKEN_HEIGHTMAP,
  XMLTOKEN_TOPLEFT,
  XMLTOKEN_SCALE,
  XMLTOKEN_DIRLIGHT,
  XMLTOKEN_LODDIST,
  XMLTOKEN_LODCOST,
  XMLTOKEN_QUADDEPTH,
  XMLTOKEN_VISTEST
};

SCF_IMPLEMENT_IBASE (csTerrFuncFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csTerrFuncLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrFuncLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
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

static void ReportError (iReporter* reporter, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (reporter)
  {
    reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

csTerrFuncFactoryLoader::csTerrFuncFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csTerrFuncFactoryLoader::~csTerrFuncFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csTerrFuncFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csTerrFuncFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csTerrFuncFactoryLoader::Parse (const char* /*string*/,
	iLoaderContext*, iBase* /* context */)
{
  iMeshObjectType* pType = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.terrfunc", iMeshObjectType);
  if (!pType)
  {
    pType = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.terrfunc",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.terrfunc\n");
    if (!pType)
      return NULL;
  }
  iMeshObjectFactory* pFactory = pType->NewFactory ();
  pType->DecRef ();
  return pFactory;
}

iBase* csTerrFuncFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  iMeshObjectType* pType = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.terrfunc", iMeshObjectType);
  if (!pType)
  {
    pType = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.terrfunc",
    	iMeshObjectType);
    if (!pType)
    {
      iReporter* reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
      ReportError (reporter,
		"crystalspace.terrfuncloader.setup.objecttype",
		"Could not load the terrfunc mesh object plugin!");
      if (reporter) reporter->DecRef ();
      return NULL;
    }
  }
  iMeshObjectFactory* pFactory = pType->NewFactory ();
  pType->DecRef ();
  return pFactory;
}

//---------------------------------------------------------------------------

csTerrFuncLoader::csTerrFuncLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
  synldr = NULL;
  reporter = NULL;
}

csTerrFuncLoader::~csTerrFuncLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (reporter);
}

bool csTerrFuncLoader::Initialize (iObjectRegistry* object_reg)
{
  csTerrFuncLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("correctseams", XMLTOKEN_CORRECTSEAMS);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("blocks", XMLTOKEN_BLOCKS);
  xmltokens.Register ("grid", XMLTOKEN_GRID);
  xmltokens.Register ("heightmap", XMLTOKEN_HEIGHTMAP);
  xmltokens.Register ("topleft", XMLTOKEN_TOPLEFT);
  xmltokens.Register ("scale", XMLTOKEN_SCALE);
  xmltokens.Register ("dirlight", XMLTOKEN_DIRLIGHT);
  xmltokens.Register ("loddist", XMLTOKEN_LODDIST);
  xmltokens.Register ("lodcost", XMLTOKEN_LODCOST);
  xmltokens.Register ("quaddepth", XMLTOKEN_QUADDEPTH);
  xmltokens.Register ("vistest", XMLTOKEN_VISTEST);

  return true;
}

iBase* csTerrFuncLoader::Parse (const char* pString,
	iLoaderContext* ldr_context, iBase* /* context */)
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
    CS_TOKEN_TABLE (ALLMATERIAL)
  CS_TOKEN_TABLE_END

  char *pName;
  long cmd;
  char *pParams;
  char pStr[255];

  iMeshObject* mesh = NULL;
  iTerrFuncState* terrstate = NULL;

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
	  mesh = iFactory->GetMeshObjectFactory()->NewInstance();
          terrstate = SCF_QUERY_INTERFACE (mesh, iTerrFuncState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
	  int i;
          csScanStr (pParams, "%d,%s", &i, pStr);
          iMaterialWrapper* mat = ldr_context->FindMaterial (pStr);
	  if (!mat)
	  {
            // @@@ Error handling!
            printf("Terrain func loader: Can't find requested material '%s'\n",pStr);
            return NULL;
	  }
	  terrstate->SetMaterial (i, mat);
	}
        break;
	  case CS_TOKEN_ALLMATERIAL:
	  {
		int i, j = terrstate->GetXResolution();
		j = j * j;
		csScanStr( pParams, "%s", pStr);
		iMaterialWrapper *mat = ldr_context->FindMaterial (pStr);
		if (!mat)
		{
		  printf("Terrain func loader: Cant find requested material '%s'\n",pStr);
		  return NULL;
		}
		for (i = 0; i < j; i++) terrstate->SetMaterial(i,mat);
	  }
	  break;
      case CS_TOKEN_GROUPMATERIAL:
	{
	  int rangeStart, rangeEnd;
	  csScanStr (pParams, "%s,%d,%d", pStr, &rangeStart, &rangeEnd);
	  terrstate->LoadMaterialGroup (ldr_context, pStr,
	  	rangeStart, rangeEnd);
	}
    	break;
      case CS_TOKEN_CORRECTSEAMS:
	{
	  int tw, th;
	  csScanStr (pParams, "%d,%d", &tw, &th);
          terrstate->CorrectSeams (tw, th);
        }
        break;
      case CS_TOKEN_BLOCKS:
	{
	  int bx, by;
	  csScanStr (pParams, "%d,%d", &bx, &by);
          terrstate->SetResolution (bx, by);
        }
        break;
      case CS_TOKEN_GRID:
	{
	  int bx, by;
	  csScanStr (pParams, "%d,%d", &bx, &by);
          terrstate->SetGridResolution (bx, by);
        }
        break;
      case CS_TOKEN_TOPLEFT:
	{
	  csVector3 tl;
	  csScanStr (pParams, "%f,%f,%f", &tl.x, &tl.y, &tl.z);
          terrstate->SetTopLeftCorner (tl);
        }
        break;
      case CS_TOKEN_SCALE:
	{
	  csVector3 s;
	  csScanStr (pParams, "%f,%f,%f", &s.x, &s.y, &s.z);
          terrstate->SetScale (s);
        }
        break;
      case CS_TOKEN_COLOR:
        {
	  csColor col;
	  csScanStr (pParams, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  terrstate->SetColor (col);
	}
	break;
      case CS_TOKEN_DIRLIGHT:
        {
	  csVector3 pos;
	  csColor col;
	  csScanStr (pParams, "%f,%f,%f:%f,%f,%f", &pos.x, &pos.y, &pos.z,
	  	&col.red, &col.green, &col.blue);
	  terrstate->SetDirLight (pos, col);
	}
	break;
      case CS_TOKEN_LODDIST:
        {
	  int lod;
	  float dist;
	  csScanStr (pParams, "%d,%f", &lod, &dist);
	  terrstate->SetLODDistance (lod, dist);
	}
	break;
      case CS_TOKEN_LODCOST:
        {
	  int lod;
	  float cost;
	  csScanStr (pParams, "%d,%f", &lod, &cost);
	  terrstate->SetMaximumLODCost (lod, cost);
	}
	break;
      case CS_TOKEN_QUADDEPTH:
        {
	  int qd;
	  csScanStr (pParams, "%d", &qd);
	  terrstate->SetQuadDepth (qd);
	}
        break;
      case CS_TOKEN_VISTEST:
        {
	  bool vt;
	  csScanStr (pParams, "%b", &vt);
	  terrstate->SetVisTesting (vt);
	}
        break;
      case CS_TOKEN_HEIGHTMAP:
        {
	  float hscale, hshift;
	  csScanStr (pParams, "%s,%f,%f\n", pStr, &hscale, &hshift);
	  iVFS* vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
	  if (!vfs)
	  {
	    // @@@ Use reporter!
	    printf ("No VFS!\n");
	    return NULL;
	  }
	  iImageIO* loader = CS_QUERY_REGISTRY (object_reg, iImageIO);
	  if (!loader)
	  {
	    vfs->DecRef ();
	    // @@@ Use reporter!
	    printf ("No image loader!\n");
	    return NULL;
	  }

	  iDataBuffer* buf = vfs->ReadFile (pStr);
	  vfs->DecRef ();
	  if (!buf || !buf->GetSize ())
	  {
	    loader->DecRef ();
	    // @@@ Use reporter!
	    printf ("Can't open file '%s' in vfs!\n", pStr);
	    return NULL;
	  }
	  iImage* ifile = loader->Load (buf->GetUint8 (), buf->GetSize (),
	  	CS_IMGFMT_TRUECOLOR);
	  loader->DecRef ();
	  if (!ifile)
	  {
	    buf->DecRef ();
	    // @@@ Use reporter!
	    printf ("Error loading image '%s'!\n", pStr);
	    return NULL;
	  }
	  terrstate->SetHeightMap (ifile, hscale, hshift);
	  ifile->DecRef ();
	  buf->DecRef ();
	}
	break;
    }
  }
  terrstate->DecRef ();

  return mesh;
}

iBase* csTerrFuncLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase* /* context */)
{
  csRef<iMeshObject> mesh;
  csRef<iTerrFuncState> terrstate;

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
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return NULL;
	  }
	  mesh = iFactory->GetMeshObjectFactory()->NewInstance();
          terrstate = SCF_QUERY_INTERFACE (mesh, iTerrFuncState);
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
	  csRef<iDocumentAttribute> attr;
	  attr = child->GetAttribute ("index");
	  if (attr != NULL)
	  {
	    // Set a single material for one index.
            iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	    if (!mat)
	    {
      	      synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
              return NULL;
	    }
	    terrstate->SetMaterial (attr->GetValueAsInt (), mat);
	  }
	  else
	  {
	    attr = child->GetAttribute ("start");
	    if (attr != NULL)
	    {
	      // Set a range of materials.
	      int start = attr->GetValueAsInt ();
	      attr = child->GetAttribute ("end");
	      if (attr == NULL)
	      {
      	        synldr->ReportError (
	    	  "crystalspace.terrfuncloader.parse",
		  child, "'end' attribute missing!");
                return NULL;
	      }
	      int end = attr->GetValueAsInt ();
	      terrstate->LoadMaterialGroup (ldr_context, matname, start, end);
	    }
	    else
	    {
	      // Set all materials.
	      int i, j = terrstate->GetXResolution();
	      j = j * j;
              iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	      if (!mat)
	      {
      	        synldr->ReportError (
	    	  "crystalspace.terrfuncloader.parse.unknownmaterial",
		  child, "Couldn't find material '%s'!", matname);
                return NULL;
	      }
	      for (i = 0 ; i < j ; i++) terrstate->SetMaterial (i, mat);
	    }
	  }
	}
	break;
      case XMLTOKEN_CORRECTSEAMS:
        terrstate->CorrectSeams (child->GetAttributeValueAsInt ("w"),
	  	child->GetAttributeValueAsInt ("h"));
        break;
      case XMLTOKEN_BLOCKS:
        terrstate->SetResolution (child->GetAttributeValueAsInt ("x"),
		child->GetAttributeValueAsInt ("y"));
        break;
      case XMLTOKEN_GRID:
        terrstate->SetGridResolution (child->GetAttributeValueAsInt ("x"),
		child->GetAttributeValueAsInt ("y"));
        break;
      case XMLTOKEN_TOPLEFT:
	{
	  csVector3 tl;
	  if (!synldr->ParseVector (child, tl))
	    return NULL;
          terrstate->SetTopLeftCorner (tl);
        }
        break;
      case XMLTOKEN_SCALE:
	{
	  csVector3 s;
	  if (!synldr->ParseVector (child, s))
	    return NULL;
          terrstate->SetScale (s);
        }
        break;
      case XMLTOKEN_COLOR:
        {
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return NULL;
	  terrstate->SetColor (col);
	}
	break;
      case XMLTOKEN_DIRLIGHT:
        {
	  csRef<iDocumentNode> posnode = child->GetNode ("position");
	  if (posnode == NULL)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "<position> missing in <dirlight>!");
	    return NULL;
	  }
	  csVector3 pos;
	  pos.x = posnode->GetAttributeValueAsFloat ("x");
	  pos.y = posnode->GetAttributeValueAsFloat ("y");
	  pos.z = posnode->GetAttributeValueAsFloat ("z");
	  csRef<iDocumentNode> colnode = child->GetNode ("color");
	  csColor col (1, 1, 1);
	  if (colnode != NULL)
	  {
	    col.red = colnode->GetAttributeValueAsFloat ("red");
	    col.green = colnode->GetAttributeValueAsFloat ("green");
	    col.blue = colnode->GetAttributeValueAsFloat ("blue");
	  }
	  terrstate->SetDirLight (pos, col);
	}
	break;
      case XMLTOKEN_LODDIST:
	terrstate->SetLODDistance (child->GetAttributeValueAsInt ("level"),
	  child->GetAttributeValueAsFloat ("distance"));
	break;
      case XMLTOKEN_LODCOST:
	terrstate->SetMaximumLODCost (child->GetAttributeValueAsInt ("level"),
	  child->GetAttributeValueAsFloat ("cost"));
	break;
      case XMLTOKEN_QUADDEPTH:
	terrstate->SetQuadDepth (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_VISTEST:
        {
	  bool vt;
	  if (!synldr->ParseBool (child, vt, true))
	    return NULL;
	  terrstate->SetVisTesting (vt);
	}
        break;
      case XMLTOKEN_HEIGHTMAP:
        {
	  float hscale = 1, hshift = 0;
	  csRef<iDocumentNode> imgnode = child->GetNode ("image");
	  if (!imgnode)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "<image> missing in <heightmap>!");
	    return NULL;
	  }
	  const char* imgname = imgnode->GetContentsValue ();
	  csRef<iDocumentNode> scalenode = child->GetNode ("scale");
	  if (scalenode) hscale = scalenode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> shiftnode = child->GetNode ("shift");
	  if (shiftnode) hshift = shiftnode->GetContentsValueAsFloat ();

	  csRef<iVFS> vfs (CS_QUERY_REGISTRY (object_reg, iVFS));
	  if (!vfs)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "VFS is missing!");
	    return NULL;
	  }
	  csRef<iImageIO> loader (CS_QUERY_REGISTRY (object_reg, iImageIO));
	  if (!loader)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "Image loader is missing!");
	    return NULL;
	  }

	  csRef<iDataBuffer> buf (vfs->ReadFile (imgname));
	  if (!buf || !buf->GetSize ())
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "Cannot read file '%s' on VFS!", imgname);
	    return NULL;
	  }
	  csRef<iImage> ifile;
	  ifile = csPtr<iImage> (loader->Load (buf->GetUint8 (), buf->GetSize (),
	  	CS_IMGFMT_TRUECOLOR));
	  if (!ifile)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "Error reading image '%s'!", imgname);
	    return NULL;
	  }
	  terrstate->SetHeightMap (ifile, hscale, hshift);
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }

  // Incref to avoid smart pointer from cleaning up.
  if (mesh) mesh->IncRef ();
  return mesh;
}


//---------------------------------------------------------------------------


