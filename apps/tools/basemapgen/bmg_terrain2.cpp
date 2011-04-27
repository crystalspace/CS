/*
    Copyright (C) 2007 by Jelle Hellemans aka sueastside
              (C) 2008 by Frank Richter

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

#include "crystalspace.h"

#include "basemapgen.h"

#include "textureinfo.h"

bool BaseMapGen::Terrain2Cell::Parse (iDocumentNode* node, bool isDefault)
{
  if (!isDefault)
    cellNode = node;
  {
    csRef<iDocumentNode> name = node->GetNode ("name");
    if (name.IsValid())
      this->name = name->GetContentsValue();
  }
  {
    csRef<iDocumentNode> basematerial = node->GetNode ("basematerial");
    if (basematerial.IsValid())
      this->baseMaterial = basematerial->GetContentsValue();
  }
  {
    csRef<iDocumentNode> renderProperties = node->GetNode ("renderproperties");
    if (isDefault)
      this->defRenderPropertiesNode = renderProperties;
    else
      this->renderPropertiesNode = renderProperties;
  }
  {
    csRef<iDocumentNode> feederProperties = node->GetNode ("feederproperties");
    if (feederProperties.IsValid())
    {
      csRef<iDocumentNodeIterator> paramsIt (feederProperties->GetNodes ("param"));
      while (paramsIt->HasNext())
      {
	csRef<iDocumentNode> paramNode = paramsIt->Next();
	
	const char* paramName = paramNode->GetAttributeValue ("name");
	if (paramName == 0) continue;
	
	if (strcmp (paramName, "materialmap source") == 0)
	{
	  const char* matMapSrc = paramNode->GetContentsValue();
	  materialMapLayers.AttachNew (new AlphaLayers);
	  csRef<iImage> matMapImg = basemapgen->LoadImage (matMapSrc, CS_IMGFMT_PALETTED8);
	  materialMapLayers->BuildAlphaMapsFromMatMap (matMapImg);
	}
      }
      
      csRef<iDocumentNodeIterator> alphaMapIt (feederProperties->GetNodes ("alphamap"));
      while (alphaMapIt->HasNext())
      {
	csRef<iDocumentNode> alphaMapNode = alphaMapIt->Next();
	
	const char* material = alphaMapNode->GetAttributeValue ("material");
	if (material == 0) continue;
        const char* alphaMapSrc = alphaMapNode->GetContentsValue();
        	
        csRef<iImage> alphaMapImg = basemapgen->LoadImage (alphaMapSrc,
          CS_IMGFMT_ANY | CS_IMGFMT_ALPHA);
        if (!alphaLayers.IsValid())
          alphaLayers.AttachNew (new AlphaLayers);
        alphaLayers->AddAlphaMap (alphaMapImg);
        alphaMaterials.Push (basemapgen->materials.Get (material,
          (MaterialLayer*)0));
      }
    }
  }
  return true;
}
    
void BaseMapGen::Terrain2Cell::ApplyMaterialMap (const MaterialLayers& matMap)
{
  alphaLayers = materialMapLayers;
  alphaMaterials = matMap;
}

void BaseMapGen::ScanTerrain2Factories ()
{
   // Get the terrain node
  csRef<iDocumentNodeIterator> meshfacts = rootnode->GetNodes("meshfact");
  while (meshfacts->HasNext())
  {
    csRef<iDocumentNode> current = meshfacts->Next();
    
    csRef<iDocumentNode> pluginNode = current->GetNode ("plugin");
    if (!pluginNode.IsValid()) continue;
    const char* plugin = GetPluginSCFID (pluginNode->GetContentsValue());
    if ((plugin == 0)
	|| (strcmp (plugin, "crystalspace.mesh.loader.factory.terrain2") != 0))
      continue;
  
    const char* name = current->GetAttributeValue("name");
  
    csRef<iDocumentNode> params = current->GetNode ("params");
    if (!params) continue;
    csRef<iDocumentNode> cells = params->GetNode ("cells");
    if (!cells) continue;
    
    csRef<Terrain2Factory> factory;
    factory.AttachNew (new Terrain2Factory);
    
    Terrain2Cell defaultCell;
    csRef<iDocumentNode> celldefault = cells->GetNode ("celldefault");
    if (celldefault)
    {
      if (!defaultCell.Parse (celldefault))
        continue;
    }
    
    csRef<iDocumentNodeIterator> cellsIt (cells->GetNodes ("cell"));
    while (cellsIt->HasNext())
    {
      csRef<iDocumentNode> cellNode = cellsIt->Next();
      csRef<Terrain2Cell> cell;
      cell.AttachNew (new Terrain2Cell (defaultCell));
      if (!cell->Parse (cellNode))
        continue;
      factory->cells.Put (cell->name, cell);
    }
    
    terrain2Factories.Put (name, factory);
  } // while meshfact
}


void BaseMapGen::ScanTerrain2Meshes ()
{
  csHash<uint, csString> baseMapWriteCounts;

   // Get the terrain node
  csRef<iDocumentNodeIterator> sectors = rootnode->GetNodes("sector");
  while (sectors->HasNext())
  {
    csRef<iDocumentNode> sector = sectors->Next();
    csRef<iDocumentNodeIterator> it = sector->GetNodes("meshobj");
    while (it->HasNext())
    {
      csRef<iDocumentNode> current = it->Next();
    
      csRef<iDocumentNode> pluginNode = current->GetNode ("plugin");
      if (!pluginNode.IsValid()) continue;
      const char* plugin = GetPluginSCFID (pluginNode->GetContentsValue());
      if ((plugin == 0)
	  || (strcmp (plugin, "crystalspace.mesh.loader.terrain2") != 0))
	continue;
    
      csString name = current->GetAttributeValue("name");
      if ((meshRE != 0) && (meshRE->Match (name) != csrxNoError))
	continue;
    
      csRef<iDocumentNode> params = current->GetNode ("params");
      if (!params) continue;
      csRef<iDocumentNode> factoryNode = params->GetNode ("factory");
      if (!factoryNode) continue;
      
      const char* factname = factoryNode->GetContentsValue();
      Terrain2Factory* factory = terrain2Factories.Get (factname,
        (Terrain2Factory*)0);
      if (!factory) continue;
    
      csPrintf ("Found terrain %s ...\n", CS::Quote::Single (name.GetData()));
      fflush (stdout);
      
      // Get the materialpalette.
      csRef<iDocumentNode> materialpalette = params->GetNode ("materialpalette");
      if (!materialpalette) continue;
    
      MaterialLayers mlayers;
      // Get the materials.
      csRef<iDocumentNodeIterator> it = materialpalette->GetNodes("material");
      while (it->HasNext())
      {
	csRef<iDocumentNode> pal = it->Next();
	const char* matname = pal->GetContentsValue();
	MaterialLayer* layer = materials.Get (matname, (MaterialLayer*)0);
	mlayers.Push (layer);
      } // while
    
      if (mlayers.GetSize() > 0)
        csPrintf ("Found %zu materials in materialpalette...\n", mlayers.GetSize());
        
      csRef<iDocumentNode> cells = params->GetNode ("cells");
      if (!cells) continue;
      
      csRef<iDocumentNodeIterator> cellsIt (cells->GetNodes ("cell"));
      while (cellsIt->HasNext())
      {
	csRef<iDocumentNode> cellNode = cellsIt->Next();
	
	csRef<iDocumentNode> nameNode = cellNode->GetNode ("name");
	if (!nameNode) continue;
	const char* cellName = nameNode->GetContentsValue();
	
	Terrain2Cell* factoryCell = factory->cells.Get (cellName,
	  (Terrain2Cell*)0);
	if (!factoryCell) continue;
	
	Terrain2Cell cell (*factoryCell);
	if (!cell.Parse (cellNode)) continue;
	
	MaterialLayer* mat = materials.Get (cell.baseMaterial, (MaterialLayer*)0);
	if (!mat) continue;
	
	if (!cell.alphaLayers.IsValid())
	  cell.ApplyMaterialMap (mlayers);

	// Get the resolution.
	int basemap_w = csFindNearestPowerOf2 (cell.alphaLayers->GetWidth());
	int basemap_h = csFindNearestPowerOf2 (cell.alphaLayers->GetHeight());
      
	// Get the resolution from the commandline.
	csString res = cmdline->GetOption ("resolution");
	if (!res.IsEmpty())
	{
	  int basemap_res = csFindNearestPowerOf2 (atoi(res));
	  basemap_w = basemap_h = basemap_res;
	}
      
	const TextureInfo* texinfo = mat->texture;
	if (!texinfo) continue;
	if (texinfo->GetFileName ().IsEmpty()) continue;
	
	csPrintf ("Basemap resolution: %dx%d\n", basemap_w, basemap_h); fflush (stdout);
	
	csRef<iImage> basemap = CreateBasemap (basemap_w, basemap_h,
	  *(cell.alphaLayers), cell.alphaMaterials);
	if (!basemap) continue;
	
	SaveImage (basemap, texinfo->GetFileName());
	SetTextureClassNode (texinfo->GetDocumentNode(), "nosharpen");
	SetTextureFlag (texinfo->GetDocumentNode(), "clamp");
	
	csRef<iDocumentNode> renderPropertiesNode (cell.renderPropertiesNode);
	if (!renderPropertiesNode)
	{
	  renderPropertiesNode = cell.cellNode->CreateNodeBefore (CS_NODE_ELEMENT);
	  renderPropertiesNode->SetValue ("renderproperties");
	}
	SetShaderVarNode (renderPropertiesNode, "basemap scale", "vector4",
			  csString().Format ("%.9g,%.9g,%.9g,%.9g",
					     float (basemap_w-1) / basemap_w,
					     float (basemap_h-1) / basemap_h,
					     0.5 / basemap_w,
					     0.5 / basemap_h));
	
	baseMapWriteCounts.GetOrCreate (texinfo->GetFileName(), 0)++;
      }
    } // while meshobj
  } // while sector
  
  csHash<uint, csString>::GlobalIterator countIt (baseMapWriteCounts.GetIterator());
  while (countIt.HasNext())
  {
    csString key;
    uint count = countIt.Next (key);
    if (count > 1)
      csPrintf ("Wrote %u times to texture %s - \n"
        "that indicates this basemap is used for multiple cells. Is that right?\n",
        count, key.GetData());
  }
}
