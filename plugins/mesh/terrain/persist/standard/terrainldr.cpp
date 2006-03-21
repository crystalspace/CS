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
#include "csutil/stringarray.h"
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
#include "iutil/object.h"
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
  iStreamSource*, iLoaderContext* /*ldr_context*/, iBase* /*context*/)
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
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csTerrainFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  
  if (obj)
  {
    csRef<iTerrainFactoryState> tfact = 
      SCF_QUERY_INTERFACE (obj, iTerrainFactoryState);
    csRef<iMeshObjectFactory> meshfact = 
      SCF_QUERY_INTERFACE (obj, iMeshObjectFactory);
    if (!tfact) return false;
    if (!meshfact) return false;
    
    // Write plugin
    csRef<iDocumentNode> pluginNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    pluginNode->SetValue("plugin");
    
    csRef<iFactory> factory = 
      SCF_QUERY_INTERFACE(meshfact->GetMeshObjectType(), iFactory);
    const char* pluginname = factory->QueryClassID();
    if (!(pluginname && *pluginname)) return false;
    
    pluginNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue (pluginname);
    
    // Write terraformer
    csRef<iDocumentNode> terraFormerNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    terraFormerNode->SetValue("terraformer");
    
    const char* terraformer = tfact->GetTerraFormer ()->QueryObject ()->GetName ();
    terraFormerNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue (terraformer);
    
    // Write sampleregion
    csRef<iDocumentNode> sampleRegionNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    sampleRegionNode->SetValue("sampleregion");
    
    csBox2 box = tfact->GetSamplerRegion ();
    csBox3 box3 (box.MinX(), box.MinY(), 0, box.MaxX(), box.MaxY(), 0);
    synldr->WriteBox (sampleRegionNode, &box3);
  }
  
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
  iStreamSource*, iLoaderContext* ldr_context, iBase* /*context*/)
{
  csRef<iMeshObject> mesh;
  csRef<iTerrainObjectState> state;
  bool palette_set = false;
  bool material_map_set = false;

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
        synldr->ReportError ("crystalspace.terrain.factory.loader",
            child, "Materialmaps are now handled by the Formers!");
        return 0;
        break;
      }
      case XMLTOKEN_MATERIALALPHAMAP:
      {
        synldr->ReportError ("crystalspace.terrain.factory.loader",
            child, "Alphamaps are now handled by the Formers!");
        return 0;
        break;
      }
      case XMLTOKEN_LODVALUE:
      {
//@@@
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
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csTerrainObjectSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  
  if (obj)
  {
    csRef<iTerrainObjectState> tmesh = 
      SCF_QUERY_INTERFACE (obj, iTerrainObjectState);
    csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);
    if (!tmesh) return false;
    if (!mesh) return false;

    //Writedown Factory tag
    iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper ();
    if (fact)
    {
      const char* factname = fact->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        factNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(factname);
      }    
    }

    //Writedown castshadow tag
    if (!tmesh->GetCastShadows())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)->SetValue("castshadows");

    //Writedown staticlighting tag
    if (!tmesh->GetStaticLighting())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)
        ->SetValue("staticlighting");

    //Writedown Color tag
    csColor col;
    if (mesh->GetColor(col))
    {
      csRef<iDocumentNode> colorNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      colorNode->SetValue("color");
      synldr->WriteColor(colorNode, &col);
    }

    //Writedown Material tag
    iMaterialWrapper* mat = mesh->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }
    
    // Write materialpalette
    csArray<iMaterialWrapper*> matpalette = tmesh->GetMaterialPalette ();
    csRef<iDocumentNode> matpaletteNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    matpaletteNode->SetValue ("materialpalette");
    for (size_t i = 0; i < matpalette.GetSize(); i++)
    {
      if (matpalette[i])
      {
        const char* matname = matpalette[i]->QueryObject ()->GetName ();;
        
        csRef<iDocumentNode> matNode =
          matpaletteNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }
    }
    
    // Write lodvalues
    csStringArray lodparams;
    lodparams.Push ("splatting distance");
    lodparams.Push ("block resolution");
    lodparams.Push ("block split distance");
    lodparams.Push ("minimum block size");
    lodparams.Push ("cd resolution");
    lodparams.Push ("cd lod cost");
    lodparams.Push ("lightmap resolution");
    
    for (size_t i = 0; i < lodparams.GetSize(); i++)
    {
      float value = tmesh->GetLODValue (lodparams[i]);
      
      csRef<iDocumentNode> lodvalueNode =
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      lodvalueNode->SetValue("lodvalue");
      
      lodvalueNode->SetAttribute ("name", lodparams[i]);
      
      csRef<iDocumentNode> lodvalueValueNode = 
        lodvalueNode->CreateNodeBefore(CS_NODE_TEXT, 0);
      lodvalueValueNode->SetValueAsFloat(value);
    }
    
    // Write materialmap
    int matmapW, matmapH;
    bool raw;
    const char* matmapFile = tmesh->GetMaterialMapFile (matmapW, matmapH, raw);
    if (matmapFile)
    {
      csRef<iDocumentNode> matmapNode =
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      matmapNode->SetValue("materialmap");
      if (raw)
      {
        matmapNode->SetAttribute ("raw", matmapFile);
        matmapNode->SetAttributeAsInt ("width", matmapW);
        matmapNode->SetAttributeAsInt ("height", matmapH);
      }
      else
      {
        matmapNode->SetAttribute ("image", matmapFile);
      }
    }
  }
  
  return true;
}

