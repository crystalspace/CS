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

#include "loader.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csTerrainFactoryLoader)
SCF_IMPLEMENT_FACTORY (csTerrainObjectLoader)


static const char* FACTORYERRORID = "crystalspace.mesh.loader.factory.terrainimproved";
static const char* ERRORID = "crystalspace.mesh.loader.terrainimproved";

csTerrainFactoryLoader::csTerrainFactoryLoader (iBase* parent)
 : scfImplementationType (this, parent)
{
}

csTerrainFactoryLoader::~csTerrainFactoryLoader ()
{
}

bool csTerrainFactoryLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  
  return true;
}

csPtr<iBase> csTerrainFactoryLoader::Parse (iDocumentNode* node,
  iStreamSource*, iLoaderContext* ldr_context, iBase* /*context*/)
{
  csRef<iPluginManager> pluginManager = csQueryRegistry<iPluginManager> (object_reg);

  csRef<iMeshObjectType> meshType = csLoadPlugin<iMeshObjectType> (
    object_reg, "crystalspace.mesh.object.terrainimproved");

  if (!meshType)
  {
    synldr->ReportError (FACTORYERRORID, node, "Cannot load mesh object type plugin");
  }

  csRef<iMeshObjectFactory> meshFactory = meshType->NewFactory ();
  csRef<iTerrainFactory> factory = scfQueryInterface<iTerrainFactory> (meshFactory);

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

bool csTerrainFactoryLoader::ParseParams (csArray<ParamPair>& pairs, iDocumentNode* node)
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
        ParamPair p;
        p.name = child2->GetAttributeValue ("name");
        p.value = child2->GetContentsValue ();

        pairs.Push (p);
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

bool csTerrainFactoryLoader::ParseCell (iDocumentNode *node, 
  iLoaderContext *ldr_ctx, iTerrainFactory *fact)
{
  csArray<ParamPair> renderParams, collParams, feederParams;

  csString name;
  csVector3 size;
  csVector2 position;
  unsigned int gridWidth, gridHeight, materialmapWidth, materialmapHeight;
  bool materialmapPersist;
  csRef<iMaterialWrapper> baseMaterial;

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
      name = child->GetContentsValue ();
      break;
    case XMLTOKEN_SIZE:
      synldr->ParseVector (child, size);
      break;
    case XMLTOKEN_POSITION:
      synldr->ParseVector (child, position);
      break;
    case XMLTOKEN_GRIDSIZE:
      gridWidth = child->GetAttributeValueAsInt ("width");
      gridHeight = child->GetAttributeValueAsInt ("height");
      break;
    case XMLTOKEN_MATERIALMAPSIZE:
      materialmapWidth = child->GetAttributeValueAsInt ("width");
      materialmapHeight = child->GetAttributeValueAsInt ("height");
      break;
    case XMLTOKEN_BASEMATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        baseMaterial = ldr_ctx->FindMaterial (matname);
        
        if (!baseMaterial)
        {
          synldr->ReportError (
            "crystalspace.terrain.object.loader.basematerial",
            child, "Couldn't find material '%s'!", matname);
          return false;
        }
        break;
      }
    case XMLTOKEN_MATERIALMAPPERSISTENT:
      synldr->ParseBool (child, materialmapPersist, false);
      break;
    case XMLTOKEN_RENDERPROPERTIES:
      {
        if (!ParseParams (renderParams, child))
          return false;

        break;
      }
    case XMLTOKEN_COLLIDERPROPERTIES:
      {
        if (!ParseParams (collParams, child))
          return false;

        break;
      }
    case XMLTOKEN_FEEDERPROPERTIES:
      {
        if (!ParseParams (feederParams, child))
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

  // Time to build the cell
  iTerrainFactoryCell* cell = fact->AddCell (name.GetDataSafe (), gridWidth, gridHeight, 
    materialmapWidth, materialmapHeight, materialmapPersist, position, size);
  
  cell->SetBaseMaterial (baseMaterial);

  iTerrainCellRenderProperties* renderProperties = cell->GetRenderProperties ();
  for (size_t i = 0; i < renderParams.GetSize (); ++i)
  {
    renderProperties->SetParameter (renderParams[i].name.GetDataSafe (), renderParams[i].value.GetDataSafe ());
  }

  iTerrainCellCollisionProperties* colliderProperties = cell->GetCollisionProperties ();
  for (size_t i = 0; i < collParams.GetSize (); ++i)
  {
    colliderProperties->SetParameter (collParams[i].name.GetDataSafe (), collParams[i].value.GetDataSafe ());
  }

  iTerrainCellFeederProperties* feederProperties = cell->GetFeederProperties ();
  for (size_t i = 0; i < feederParams.GetSize (); ++i)
  {
    feederProperties->SetParameter (feederParams[i].name.GetDataSafe (), feederParams[i].value.GetDataSafe ());
  }

  return true;
}


csTerrainObjectLoader::csTerrainObjectLoader (iBase* parent)
 : scfImplementationType (this, parent)
{
}

csTerrainObjectLoader::~csTerrainObjectLoader ()
{
}

bool csTerrainObjectLoader::Initialize (iObjectRegistry* objreg)
{
  object_reg = objreg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);

  return true;
}

csPtr<iBase> csTerrainObjectLoader::Parse (iDocumentNode* node, 
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
        if (!fact)
        {
          synldr->ReportError ("crystalspace.terrain.object.loader",
            child, "Couldn't find factory '%s'!", factname);
          return 0;
        }
        mesh = fact->GetMeshObjectFactory ()->NewInstance ();
        terrain = scfQueryInterface<iTerrainSystem> (mesh);
            
        if (!terrain)
        {
          synldr->ReportError (
                    "crystalspace.terrain.parse.badfactory", child,
                    "Factory '%s' doesn't appear to be a terrain factory!",
                    factname);
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
                  child, "Couldn't find material '%s'!", matname);
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
      default:
        synldr->ReportBadToken (child);
    }
  }

  return csPtr<iBase>(mesh);
}
