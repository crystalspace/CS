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
}

csTerrFuncFactoryLoader::~csTerrFuncFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csTerrFuncFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csTerrFuncFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csTerrFuncFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> pType (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.terrfunc", iMeshObjectType));
  if (!pType)
  {
    pType = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.terrfunc",
    	iMeshObjectType);
    if (!pType)
    {
      csRef<iReporter> reporter (CS_QUERY_REGISTRY (object_reg, iReporter));
      ReportError (reporter,
		"crystalspace.terrfuncloader.setup.objecttype",
		"Could not load the terrfunc mesh object plugin!");
      return 0;
    }
  }
  csRef<iMeshObjectFactory> pFactory (pType->NewFactory ());
  return csPtr<iBase> (pFactory);
}

//---------------------------------------------------------------------------

csTerrFuncLoader::csTerrFuncLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csTerrFuncLoader::~csTerrFuncLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csTerrFuncLoader::Initialize (iObjectRegistry* object_reg)
{
  csTerrFuncLoader::object_reg = object_reg;
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

csPtr<iBase> csTerrFuncLoader::Parse (iDocumentNode* node,
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
	    return 0;
	  }
	  mesh = iFactory->GetMeshObjectFactory()->NewInstance();
          terrstate = SCF_QUERY_INTERFACE (mesh, iTerrFuncState);
	  if (!terrstate)
	  {
      	    synldr->ReportError (
		"crystalspace.terrfunc.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a terrfunc factory!",
		factname);
	    return 0;
	  }
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
	  csRef<iDocumentAttribute> attr;
	  attr = child->GetAttribute ("index");
	  if (attr != 0)
	  {
	    // Set a single material for one index.
            iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	    if (!mat)
	    {
      	      synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
              return 0;
	    }
	    terrstate->SetMaterial (attr->GetValueAsInt (), mat);
	  }
	  else
	  {
	    attr = child->GetAttribute ("start");
	    if (attr != 0)
	    {
	      // Set a range of materials.
	      int start = attr->GetValueAsInt ();
	      attr = child->GetAttribute ("end");
	      if (attr == 0)
	      {
      	        synldr->ReportError (
	    	  "crystalspace.terrfuncloader.parse",
		  child, "'end' attribute missing!");
                return 0;
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
                return 0;
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
	    return 0;
          terrstate->SetTopLeftCorner (tl);
        }
        break;
      case XMLTOKEN_SCALE:
	{
	  csVector3 s;
	  if (!synldr->ParseVector (child, s))
	    return 0;
          terrstate->SetScale (s);
        }
        break;
      case XMLTOKEN_COLOR:
        {
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  terrstate->SetColor (col);
	}
	break;
      case XMLTOKEN_DIRLIGHT:
        {
	  csRef<iDocumentNode> posnode = child->GetNode ("position");
	  if (posnode == 0)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "<position> missing in <dirlight>!");
	    return 0;
	  }
	  csVector3 pos;
	  pos.x = posnode->GetAttributeValueAsFloat ("x");
	  pos.y = posnode->GetAttributeValueAsFloat ("y");
	  pos.z = posnode->GetAttributeValueAsFloat ("z");
	  csRef<iDocumentNode> colnode = child->GetNode ("color");
	  csColor col (1, 1, 1);
	  if (colnode != 0)
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
	    return 0;
	  terrstate->SetVisTesting (vt);
	}
        break;
      case XMLTOKEN_HEIGHTMAP:
        {
	  float hscale = 1, hshift = 0;
	  bool flipx = false, flipy = false;
	  csRef<iDocumentNode> imgnode = child->GetNode ("image");
	  if (!imgnode)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "<image> missing in <heightmap>!");
	    return 0;
	  }
	  const char* imgname = imgnode->GetContentsValue ();
	  csRef<iDocumentNode> scalenode = child->GetNode ("scale");
	  if (scalenode) hscale = scalenode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> shiftnode = child->GetNode ("shift");
	  if (shiftnode) hshift = shiftnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> flipxnode = child->GetNode ("flipx");
	  if(flipxnode)
	    if(!synldr->ParseBool(flipxnode, flipx, true))
	    {
	      synldr->ReportError (
	        "crystalspace.maploader.parse.heightgen", flipxnode,
                "bad flipx argument.");
	      return 0;
	    }
	  csRef<iDocumentNode> flipynode = child->GetNode ("flipy");
	  if(flipynode)
	    if(!synldr->ParseBool(flipynode, flipy, true))
	    {
	      synldr->ReportError (
	        "crystalspace.maploader.parse.heightgen", flipynode,
                "bad flipy argument.");
	      return 0;
	    }

	  csRef<iVFS> vfs (CS_QUERY_REGISTRY (object_reg, iVFS));
	  if (!vfs)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "VFS is missing!");
	    return 0;
	  }
	  csRef<iImageIO> loader (CS_QUERY_REGISTRY (object_reg, iImageIO));
	  if (!loader)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "Image loader is missing!");
	    return 0;
	  }

	  csRef<iDataBuffer> buf (vfs->ReadFile (imgname));
	  if (!buf || !buf->GetSize ())
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "Cannot read file '%s' on VFS!", imgname);
	    return 0;
	  }
	  csRef<iImage> ifile (loader->Load (buf->GetUint8 (), buf->GetSize (),
	  	CS_IMGFMT_TRUECOLOR));
	  if (!ifile)
	  {
      	    synldr->ReportError (
	    	"crystalspace.terrfuncloader.parse",
		child, "Error reading image '%s'!", imgname);
	    return 0;
	  }
	  terrstate->SetHeightMap (ifile, hscale, hshift, flipx, flipy);
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}


//---------------------------------------------------------------------------


