/*
    Copyright (C) 2006 by Kapoulkine Arseny

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
#include "imesh/terrain2.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"

#include "loader.h"



CS_PLUGIN_NAMESPACE_BEGIN(Terrain2Loader)
{
  
SCF_IMPLEMENT_FACTORY (csTerrain2FactoryLoader)
SCF_IMPLEMENT_FACTORY (csTerrain2ObjectLoader)


static const char* FACTORYERRORID = "crystalspace.mesh.loader.factory.terrain2";

csTerrain2FactoryLoader::csTerrain2FactoryLoader (iBase* parent)
 : scfImplementationType (this, parent)
{
}

csTerrain2FactoryLoader::~csTerrain2FactoryLoader ()
{
}

bool csTerrain2FactoryLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  
  return true;
}

csPtr<iBase> csTerrain2FactoryLoader::Parse (iDocumentNode* node,
  iStreamSource*, iLoaderContext* ldr_context, iBase* /*context*/)
{
  csRef<iPluginManager> pluginManager = csQueryRegistry<iPluginManager> (object_reg);

  csRef<iMeshObjectType> meshType = csLoadPlugin<iMeshObjectType> (
    object_reg, "crystalspace.mesh.object.terrain2");

  if (!meshType)
  {
    synldr->ReportError (FACTORYERRORID, node, "Cannot load mesh object type plugin");
  }

  csRef<iMeshObjectFactory> meshFactory = meshType->NewFactory ();
  csRef<iTerrainFactory> factory = scfQueryInterface<iTerrainFactory> (meshFactory);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();

  //DefaultCellValues defaults;
  csRef<iTerrainFactoryCell> defaultCell (factory->GetDefaultCell());
  defaultCell->SetSize (csVector3 (128.0f, 32.0f, 128.0f));
  defaultCell->SetGridWidth (128);
  defaultCell->SetGridHeight (128);
  defaultCell->SetMaterialMapWidth (128);
  defaultCell->SetMaterialMapHeight (128);
  defaultCell->SetMaterialPersistent (false);

  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();

    if (child->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_RENDERER:
      {
        const char* pluginname = child->GetContentsValue ();
        csRef<iTerrainRenderer> renderer = csLoadPluginCheck<iTerrainRenderer> (
          pluginManager, pluginname);

        if (!renderer)
        {
          synldr->ReportError (FACTORYERRORID, child, "Could not load %s!", pluginname);
          return 0;
        }

        factory->SetRenderer (renderer);
        break;
      }
    case XMLTOKEN_COLLIDER:
      {
        const char* pluginname = child->GetContentsValue ();
        csRef<iTerrainCollider> collider = csLoadPluginCheck<iTerrainCollider> (
          pluginManager, pluginname);

        if (!collider)
        {
          synldr->ReportError (FACTORYERRORID, child, "Could not load %s!", pluginname);
          return 0;
        }

        factory->SetCollider (collider);
        break;
      }
    case XMLTOKEN_FEEDER:
      {
        const char* pluginname = child->GetContentsValue ();
        csRef<iTerrainDataFeeder> feeder = csLoadPluginCheck<iTerrainDataFeeder> (
          pluginManager, pluginname);

        if (!feeder)
        {
          synldr->ReportError (FACTORYERRORID, child, "Could not load %s!", pluginname);
          return 0;
        }

        factory->SetFeeder (feeder);
        break;
      }
    case XMLTOKEN_CELLS:
      {
        csRef<iDocumentNodeIterator> it2 = child->GetNodes ();

        while (it2->HasNext ())
        {
          csRef<iDocumentNode> child2 = it2->Next ();

          if (child2->GetType () != CS_NODE_ELEMENT) 
            continue;

          const char* value = child2->GetValue ();
          csStringID id = xmltokens.Request (value);

          switch (id)
          {
          case XMLTOKEN_CELLDEFAULT:
            {
              if (!ParseCell (child2, ldr_context, factory, defaultCell))
                return 0;

              break;
            }
          case XMLTOKEN_CELL:
            {
              if (!ParseCell (child2, ldr_context, factory))
                return 0;

              break;
            }
          default:
            {
              synldr->ReportBadToken (child2);
              return 0;
            }
          }
        }

        break;
      }
    case XMLTOKEN_MAXLOADEDCELLS:
      {
        factory->SetMaxLoadedCells (child->GetContentsValueAsInt ());
        break;
      }
    case XMLTOKEN_AUTOPRELOAD:
      {
        bool res;

        synldr->ParseBool (child, res, false);
        factory->SetAutoPreLoad (res);
        break;
      }
    case XMLTOKEN_VIRTUALVIEWDISTANCE:
      {
        factory->SetVirtualViewDistance (child->GetContentsValueAsFloat ());
        break;
      }
    default:
      {
        synldr->ReportBadToken (child);
        return 0;
      }
    }
  }

  return csPtr<iBase> (meshFactory);
}

template<typename IProp>
bool csTerrain2FactoryLoader::ParseParams (IProp* props, 
                                           iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it2 = node->GetNodes ();

  while (it2->HasNext ())
  {
    csRef<iDocumentNode> child2 = it2->Next ();

    if (child2->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child2->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
    case XMLTOKEN_PARAM:
      {
        const char* name = child2->GetAttributeValue ("name");
        const char* value = child2->GetContentsValue ();
	props->SetParameter (name, value ? value : "");
        break;
      }
    default:
      {
        synldr->ReportBadToken (child2);
        return false;
      }
    }
  }    

  return true;
}

bool csTerrain2FactoryLoader::ParseRenderParams (iTerrainCellRenderProperties* props,
						 iLoaderContext* ldr_context,
                                                 iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it2 = node->GetNodes ();

  while (it2->HasNext ())
  {
    csRef<iDocumentNode> child2 = it2->Next ();

    if (child2->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child2->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
    case XMLTOKEN_SHADERVAR:
      {
        csRef<csShaderVariable> sv;
        sv.AttachNew (new csShaderVariable);
        if (!synldr->ParseShaderVar (ldr_context, child2, *sv)) return false;
	props->AddVariable (sv);
        break;
      }
    case XMLTOKEN_PARAM:
      {
        const char* name = child2->GetAttributeValue ("name");
        const char* value = child2->GetContentsValue ();
	props->SetParameter (name, value ? value : "");
        break;
      }
    default:
      {
        synldr->ReportBadToken (child2);
        return false;
      }
    }
  }    

  return true;
}

bool csTerrain2FactoryLoader::ParseFeederParams (iTerrainCellFeederProperties* props,
						 iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it2 = node->GetNodes ();

  while (it2->HasNext ())
  {
    csRef<iDocumentNode> child2 = it2->Next ();

    if (child2->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child2->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
    case XMLTOKEN_PARAM:
      {
        const char* name = child2->GetAttributeValue ("name");
        const char* value = child2->GetContentsValue ();
	props->SetParameter (name, value ? value : "");
        break;
      }
    case XMLTOKEN_ALPHAMAP:
      {
        const char* name = child2->GetAttributeValue ("material");
        const char* value = child2->GetContentsValue ();
	props->AddAlphaMap (name, value);
	break;
      }
    default:
      {
        synldr->ReportBadToken (child2);
        return false;
      }
    }
  }    

  return true;
}

bool csTerrain2FactoryLoader::ParseCell (iDocumentNode *node, 
  iLoaderContext *ldr_ctx, iTerrainFactory *fact,
  iTerrainFactoryCell* _cell)
{
  csRef<iTerrainFactoryCell> cell (_cell);
  if (!cell.IsValid()) cell = fact->AddCell ();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();

    if (child->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_NAME:
      cell->SetName (child->GetContentsValue ());
      break;
    case XMLTOKEN_SIZE:
      {
	csVector3 size;
	synldr->ParseVector (child, size);
	cell->SetSize (size);
      }
      break;
    case XMLTOKEN_POSITION:
      {
	csVector2 position;
	synldr->ParseVector (child, position);
	cell->SetPosition (position);
      }
      break;
    case XMLTOKEN_GRIDSIZE:
      cell->SetGridWidth (child->GetAttributeValueAsInt ("width"));
      cell->SetGridHeight (child->GetAttributeValueAsInt ("height"));
      break;
    case XMLTOKEN_MATERIALMAPSIZE:
      cell->SetMaterialMapWidth (child->GetAttributeValueAsInt ("width"));
      cell->SetMaterialMapHeight (child->GetAttributeValueAsInt ("height"));
      break;
    case XMLTOKEN_BASEMATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        csRef<iMaterialWrapper> baseMaterial = ldr_ctx->FindMaterial (matname);

        if (!baseMaterial)
        {
          synldr->ReportError (
            "crystalspace.terrain.object.loader.basematerial",
            child, "Couldn't find material %s!", CS::Quote::Single (matname));
          return false;
        }

        cell->SetBaseMaterial (baseMaterial);
        break;
      }
    case XMLTOKEN_ALPHASPLATMATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        csRef<iMaterialWrapper> alphaSplatMaterial = ldr_ctx->FindMaterial (matname);

        if (!alphaSplatMaterial)
        {
          synldr->ReportError (
            "crystalspace.terrain.object.loader.basematerial",
            child, "Couldn't find material %s!", CS::Quote::Single (matname));
          return false;
        }

        cell->SetAlphaSplatMaterial (alphaSplatMaterial);
        break;
      }
    case XMLTOKEN_MATERIALMAPPERSISTENT:
      {
	bool materialmapPersist;
	synldr->ParseBool (child, materialmapPersist, false);
	cell->SetMaterialPersistent (materialmapPersist);
      }
      break;
    case XMLTOKEN_RENDERPROPERTIES:
      {
	if (!ParseRenderParams (cell->GetRenderProperties(), ldr_ctx, child))
          return false;

        break;
      }
    case XMLTOKEN_COLLIDERPROPERTIES:
      {
	if (!ParseParams (cell->GetCollisionProperties(), child))
          return false;

        break;
      }
    case XMLTOKEN_FEEDERPROPERTIES:
      {
	if (!ParseFeederParams (cell->GetFeederProperties(), child))
          return false;

        break;
      }
    default:
      {
        synldr->ReportBadToken (child);
        return false;
      }
    }
  }

  return true;
}


csTerrain2ObjectLoader::csTerrain2ObjectLoader (iBase* parent)
 : scfImplementationType (this, parent)
{
}

csTerrain2ObjectLoader::~csTerrain2ObjectLoader ()
{
}

bool csTerrain2ObjectLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);

  return true;
}

csPtr<iBase> csTerrain2ObjectLoader::Parse (iDocumentNode* node, 
  iStreamSource*, iLoaderContext* ldr_context, iBase* /*context*/)
{
  csRef<iMeshObject> mesh;
  csRef<iTerrainSystem> terrain;

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

        if(!fact)
        {
          synldr->ReportError ("crystalspace.terrain.object.loader",
            child, "Couldn't find factory %s!", CS::Quote::Single (factname));
          return 0;
        }

        mesh = fact->GetMeshObjectFactory ()->NewInstance ();
        terrain = scfQueryInterface<iTerrainSystem> (mesh);
            
        if (!terrain)
        {
          synldr->ReportError (
                    "crystalspace.terrain.parse.badfactory", child,
                    "Factory %s doesn't appear to be a terrain factory!",
                    CS::Quote::Single (factname));
          return 0;
        }
            
        break;
      }
      case XMLTOKEN_MATERIALPALETTE:
      {
        csRefArray<iMaterialWrapper> pal;
  
        csRef<iDocumentNodeIterator> it = child->GetNodes ();
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
                  child, "Couldn't find material %s!", CS::Quote::Single (matname));
                return 0;
              }
              pal.Push (mat);
              break;
            }
            default:
              synldr->ReportBadToken (child);
          }
        }

        terrain->SetMaterialPalette (pal);
        break;
      }
      case XMLTOKEN_CELLS:
	{
	  csRef<iDocumentNodeIterator> it2 = child->GetNodes ();

	  while (it2->HasNext ())
	  {
	    csRef<iDocumentNode> child2 = it2->Next ();

	    if (child2->GetType () != CS_NODE_ELEMENT) 
	      continue;

	    const char* value = child2->GetValue ();
	    csStringID id = xmltokens.Request (value);

	    switch (id)
	    {
	    case XMLTOKEN_CELL:
	      {
		if (!ParseCell (child2, ldr_context, terrain))
		  return 0;

		break;
	      }
	    default:
	      {
		synldr->ReportBadToken (child2);
		return 0;
	      }
	    }
	  }

	  break;
	}
      default:
        synldr->ReportBadToken (child);
    }
  }

  return csPtr<iBase>(mesh);
}

bool csTerrain2ObjectLoader::ParseCell (iDocumentNode* node, 
					iLoaderContext* ldr_ctx,
					iTerrainSystem* terrain)
{
  csRef<iDocumentNode> nameNode = node->GetNode ("name");
  if (!nameNode.IsValid())
  {
    synldr->ReportError (
      "crystalspace.terrain.object.loader.cell",
      node, "<cell> without name");
    return false;
  }

  const char* cellName = nameNode->GetContentsValue();
  if (cellName == 0)
  {
    synldr->ReportError (
      "crystalspace.terrain.object.loader.cell",
      node, "Empty cell name");
    return false;
  }

  iTerrainCell* cell = terrain->GetCell (cellName);
  if (cell == 0)
  {
    synldr->ReportError (
      "crystalspace.terrain.object.loader.cell",
      node, "Invalid cell name %s", CS::Quote::Single (cellName));
    return false;
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();

    if (child->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_NAME:
      // Skip
      break;
    case XMLTOKEN_RENDERPROPERTIES:
      {
	if (!ParseRenderParams (cell->GetRenderProperties(), ldr_ctx, child))
          return false;

        break;
      }
    case XMLTOKEN_COLLIDERPROPERTIES:
      {
	if (!ParseParams (cell->GetCollisionProperties(), child))
          return false;

        break;
      }
    case XMLTOKEN_FEEDERPROPERTIES:
      {
	if (!ParseFeederParams (cell->GetFeederProperties(), child))
          return false;

        break;
      }
    default:
      {
        synldr->ReportBadToken (child);
        return false;
      }
    }
  }

  return true;
}

template<typename IProp>
bool csTerrain2ObjectLoader::ParseParams (IProp* props, 
					  iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it2 = node->GetNodes ();

  while (it2->HasNext ())
  {
    csRef<iDocumentNode> child2 = it2->Next ();

    if (child2->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child2->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
    case XMLTOKEN_PARAM:
      {
        const char* name = child2->GetAttributeValue ("name");
        const char* value = child2->GetContentsValue ();
	props->SetParameter (name, value ? value : "");
        break;
      }
    default:
      {
        synldr->ReportBadToken (child2);
        return false;
      }
    }
  }    

  return true;
}

bool csTerrain2ObjectLoader::ParseRenderParams (iTerrainCellRenderProperties* props,
						iLoaderContext* ldr_context,
						iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it2 = node->GetNodes ();

  while (it2->HasNext ())
  {
    csRef<iDocumentNode> child2 = it2->Next ();

    if (child2->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child2->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
    case XMLTOKEN_SHADERVAR:
      {
        csRef<csShaderVariable> sv;
        sv.AttachNew (new csShaderVariable);
        if (!synldr->ParseShaderVar (ldr_context, child2, *sv)) return false;
	props->AddVariable (sv);
        break;
      }
    case XMLTOKEN_PARAM:
      {
        const char* name = child2->GetAttributeValue ("name");
        const char* value = child2->GetContentsValue ();
	props->SetParameter (name, value ? value : "");
        break;
      }
    default:
      {
        synldr->ReportBadToken (child2);
        return false;
      }
    }
  }    

  return true;
}

bool csTerrain2ObjectLoader::ParseFeederParams (iTerrainCellFeederProperties* props,
						iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it2 = node->GetNodes ();

  while (it2->HasNext ())
  {
    csRef<iDocumentNode> child2 = it2->Next ();

    if (child2->GetType () != CS_NODE_ELEMENT) 
      continue;

    const char* value = child2->GetValue ();
    csStringID id = xmltokens.Request (value);

    switch (id)
    {
    case XMLTOKEN_PARAM:
      {
        const char* name = child2->GetAttributeValue ("name");
        const char* value = child2->GetContentsValue ();
	props->SetParameter (name, value ? value : "");
        break;
      }
    case XMLTOKEN_ALPHAMAP:
      {
        const char* name = child2->GetAttributeValue ("material");
        const char* value = child2->GetContentsValue ();
	props->AddAlphaMap (name, value);
	break;
      }
    default:
      {
        synldr->ReportBadToken (child2);
        return false;
      }
    }
  }    

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2Loader)
