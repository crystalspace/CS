/*
    Copyright (C) 2003 by Jorrit Tyberghein, Daniel Duhprey

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

#include "csutil/cscolor.h"
#include "csutil/sysfunc.h"
#include "csutil/refarr.h"

#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "imap/loader.h"
#include "imesh/object.h"
#include "imesh/terrain.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/terraform.h"

#include "terrainldr.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_PLUGIN = 1,
  XMLTOKEN_TERRAFORMER,
  XMLTOKEN_SAMPLEREGION,
  XMLTOKEN_COLOR,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MATERIALPALETTE,
  XMLTOKEN_MATERIALMAP,
  XMLTOKEN_MATERIALALPHAMAP,
  XMLTOKEN_LODVALUE,
  XMLTOKEN_STATICLIGHTING,
  XMLTOKEN_CASTSHADOWS
};

SCF_IMPLEMENT_IBASE (csTerrainFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTerrainFactoryLoader)

csTerrainFactoryLoader::csTerrainFactoryLoader (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csTerrainFactoryLoader::~csTerrainFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csTerrainFactoryLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  xmltokens.Register ("plugin", XMLTOKEN_PLUGIN);
  xmltokens.Register ("terraformer", XMLTOKEN_TERRAFORMER);
  xmltokens.Register ("sampleregion", XMLTOKEN_SAMPLEREGION);
  return true;
}

csPtr<iBase> csTerrainFactoryLoader::Parse (iDocumentNode* node,
  iStreamSource*, iLoaderContext* ldr_context, iBase* context)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg,
    iPluginManager);

  csRef<iMeshObjectFactory> fact;
  csRef<iTerrainFactoryState> state;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_PLUGIN:
      {
        const char* pluginname = child->GetContentsValue ();
        csRef<iMeshObjectType> type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
          pluginname, iMeshObjectType);
        if (!type)
        {
          type = CS_LOAD_PLUGIN (plugin_mgr, 
            pluginname, iMeshObjectType);
        }
        if (!type)
        {
          synldr->ReportError ("crystalspace.terrain.loader.factory",
            node, "Could not load %s!", pluginname);
          return 0;
        }
        fact = type->NewFactory ();
        if (!fact)
        {
          synldr->ReportError ("crystalspace.terrain.loader.factory",
            node, "Could not create a factory from %s", pluginname);
        }
        state = SCF_QUERY_INTERFACE (fact, iTerrainFactoryState);
        if (!state)
        {
          synldr->ReportError ("crystalspace.terrain.loader.factory",
            node, "Could not query iTerrainFactoryState from %s", pluginname);
        }
        break;
      }
      case XMLTOKEN_TERRAFORMER:
      {
        const char* name = child->GetContentsValue ();
        csRef<iTerraFormer> form = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
	  name, iTerraFormer);
	if (form == 0) 
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
            child, "Unable to find TerraFormer %s", name);
          return 0;
	}
        state->SetTerraFormer (form);
        break;
      }
      case XMLTOKEN_SAMPLEREGION:
      {
        csBox3 box;
        if (!synldr->ParseBox (child, box)) 
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
            child, "Unable to parse sampleregion");
          return 0;
	}
        state->SetSamplerRegion (csBox2(box.MinX(), box.MinY(), 
		                        box.MaxX(), box.MaxY()));
        break;
      }
      default:
        synldr->ReportError ("crystalspace.terrain.factory.loader",
          child, "Unknown token!");
    }
  }
  
  return csPtr<iBase> (fact);
}

SCF_IMPLEMENT_IBASE (csTerrainFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTerrainFactorySaver)

csTerrainFactorySaver::csTerrainFactorySaver (iBase* parent)
{ 
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csTerrainFactorySaver::~csTerrainFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csTerrainFactorySaver::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  return true;
}
//TBD
bool csTerrainFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for terrain mesh");
  paramsNode=0;
  
  return true;
}

SCF_IMPLEMENT_IBASE (csTerrainObjectLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainObjectLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTerrainObjectLoader)

csTerrainObjectLoader::csTerrainObjectLoader (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csTerrainObjectLoader::~csTerrainObjectLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csTerrainObjectLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("materialpalette", XMLTOKEN_MATERIALPALETTE);
  xmltokens.Register ("materialmap", XMLTOKEN_MATERIALMAP);
  xmltokens.Register ("materialalphamap", XMLTOKEN_MATERIALALPHAMAP);
  xmltokens.Register ("lodvalue", XMLTOKEN_LODVALUE);
  xmltokens.Register ("staticlighting", XMLTOKEN_STATICLIGHTING);
  xmltokens.Register ("castshadows", XMLTOKEN_CASTSHADOWS);
  return true;
}

csPtr<iBase> csTerrainObjectLoader::Parse (iDocumentNode* node, 
  iStreamSource*, iLoaderContext* ldr_context, iBase* context)
{
  csRef<iMeshObject> mesh;
  csRef<iTerrainObjectState> state;
  bool palette_set = false;
  bool material_map_set = false;
  bool one_material_map_used = false;

  csRefArray<iImage> alphamaps_ref;
  csArray<iImage*> alphamaps;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FACTORY:
      {
        const char* factname = child->GetContentsValue ();
        csRef<iMeshFactoryWrapper> fact = ldr_context->FindMeshFactory (
          factname);
        if (!fact)
        {
          synldr->ReportError ("crystalspace.terrain.object.loader",
            child, "Couldn't find factory '%s'!", factname);
          return 0;
        }
        mesh = fact->GetMeshObjectFactory ()->NewInstance ();
        state = SCF_QUERY_INTERFACE (mesh, iTerrainObjectState);
	if (!state)
	{
      	  synldr->ReportError (
		"crystalspace.terrain.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a terrain factory!",
		factname);
	  return 0;
	}
        break;
      }
      case XMLTOKEN_COLOR:
      {
        csColor c;
        if (!synldr->ParseColor (child, c))
        {
          synldr->ReportError ("crystalspace.terrain.object.loader",
            child, "Error reading color value!");
          return 0;
        }
        mesh->SetColor (c);
        break;
      }
      case XMLTOKEN_MATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        csRef<iMaterialWrapper> mat = ldr_context->FindMaterial (matname);
        if (!mat)
        {
          synldr->ReportError ("crystalspace.terrain.object.loader",
            child, "Couldn't find material '%s'!", matname);
          return 0;
        }
        mesh->SetMaterialWrapper (mat);
        break;
      }
      case XMLTOKEN_MATERIALPALETTE:
      {
        csArray<iMaterialWrapper*> pal;
        if (!ParseMaterialPalette (child, ldr_context, pal))
        {
          synldr->ReportError ("crystalspace.terrain.object.loader",
            child, "Error parsing material palette!");
          return 0;
        }
        state->SetMaterialPalette (pal);
	palette_set = true;
        break;
      }
      case XMLTOKEN_MATERIALMAP:
      {
        if (!palette_set)
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "First set a material palette before <materialmap>!");
          return 0;
	}
	if (alphamaps.Length () > 0)
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "You can't combine material alhpamaps with materialmap!");
          return 0;
	}
	material_map_set = true;
	one_material_map_used = true;
        const char* imagefile = child->GetAttributeValue ("image");
        const char *arrayfile = child->GetAttributeValue ("raw");
        int width = child->GetAttributeValueAsInt ("width");
        int height = child->GetAttributeValueAsInt ("height");
        if (imagefile != 0)
        {
	  csRef<iLoader> loader = CS_QUERY_REGISTRY (object_reg, iLoader);
          csRef<iImage> map = loader->LoadImage(imagefile,CS_IMGFMT_PALETTED8);
          if (map == 0) 
          {
            synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "Error reading in image file for heightmap '%s'", 
              imagefile);
            return 0;
          }
          state->SetMaterialMap (map);
        }
        else if (arrayfile != 0 && width != 0 && height != 0) 
        {
          csRef<iFile> file = vfs->Open (arrayfile, VFS_FILE_READ);
          if (file == 0) 
          {
            synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "Error reading in raw file for heightmap '%s'", 
              arrayfile);
            return 0;
          }
          csArray<char> array;
          int index = 0;
          while (!file->AtEOF())
          {
            file->Read (&array.GetExtend (index++), sizeof (char));
          }
          state->SetMaterialMap (array, width, height);
        }
        else
        {
          synldr->ReportError ("crystalpace.terrain.object.loader",
            child, "No image or raw file specified for material map");
          return 0;
        }
        break;
      }
      case XMLTOKEN_MATERIALALPHAMAP:
      {
        if (!palette_set)
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "First set a material palette before <materialmap>!");
          return 0;
	}
	if (one_material_map_used)
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "You are already using materialmap. Can't add alpha maps now!");
          return 0;
	}
	material_map_set = true;
        const char* imagefile = child->GetAttributeValue ("image");
        if (imagefile != 0)
        {
	  csRef<iLoader> loader = CS_QUERY_REGISTRY (object_reg, iLoader);
          csRef<iImage> map = loader->LoadImage(imagefile,CS_IMGFMT_PALETTED8);
          if (map == 0) 
          {
            synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "Error reading in image file for heightmap '%s'", 
              imagefile);
            return 0;
          }
	  alphamaps_ref.Push (map);
	  alphamaps.Push (map);
        }
        else
        {
          synldr->ReportError ("crystalpace.terrain.object.loader",
            child, "No image file specified for materialraw  p");
          return 0;
        }
        break;
      }
      case XMLTOKEN_LODVALUE:
      {
        if (material_map_set)
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "<lodvalue> must be set before <materialmap>!");
          return 0;
	}
	const char* name = child->GetAttributeValue ("name");
	if (name == 0)
	{
          synldr->ReportError ("crystalspace.terrain.factory.loader",
              child, "<lodvalue> has no 'name' attribute");
          return 0;
	}
        float val = child->GetContentsValueAsFloat ();
	state->SetLODValue (name, val);
	break;
      }
      case XMLTOKEN_STATICLIGHTING:
	{
	  bool staticLighting;
	  if (!synldr->ParseBool (child, staticLighting, true))
	    return 0;
	  state->SetStaticLighting (staticLighting);
	}
	break;
      case XMLTOKEN_CASTSHADOWS:
	{
	  bool castShadows;
	  if (!synldr->ParseBool (child, castShadows, true))
	    return 0;
	  state->SetCastShadows (castShadows);
	}
	break;
      default:
        synldr->ReportError ("crystalspace.terrain.object.loader",
          child, "Unknown token");
    }
  }

  if (alphamaps.Length () > 0)
  {
    state->SetMaterialAlphaMaps (alphamaps);
  }

  return csPtr<iBase>(mesh);
}

bool csTerrainObjectLoader::ParseMaterialPalette (iDocumentNode *node,
       iLoaderContext *ldr_context, csArray<iMaterialWrapper*>& palette)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        csRef<iMaterialWrapper> mat = ldr_context->FindMaterial (matname);
        if (!mat)
        {
          synldr->ReportError (
            "crystalspace.terrain.object.loader.materialpalette",
            child, "Couldn't find material '%s'!", matname);
          return false;
        }
        palette.Push (mat);
        break;
      }
      default:
        synldr->ReportError (
          "crystalspace.terrain.object.loader.materialpalette",
          child, "Unknown token in materials list!");
    }
  }
  return true;
}

SCF_IMPLEMENT_IBASE (csTerrainObjectSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTerrainObjectSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTerrainObjectSaver)

csTerrainObjectSaver::csTerrainObjectSaver (iBase* parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csTerrainObjectSaver::~csTerrainObjectSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csTerrainObjectSaver::Initialize (iObjectRegistry *objreg)
{
  object_reg = objreg;
  return true;
}
//TBD
bool csTerrainObjectSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for terrain mesh");
  paramsNode=0;
  
  return true;
}

