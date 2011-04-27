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

void BaseMapGen::ScanOldMaterialMaps()
{
  // Get the terraformer node
  csRef<iDocumentNodeIterator> it = rootnode->GetNodes("addon");
  while (it->HasNext())
  {
    csRef<iDocumentNode> current = it->Next();
    
    const char* plugin = GetPluginSCFID (
      current->GetAttributeValue ("plugin"));
    if ((plugin == 0)
        || (strcmp (plugin, "crystalspace.terraformer.simple.loader") != 0))
      continue;
    
    
    csRef<iDocumentNode> params = current->GetNode ("params");
    if (!params) params = current;
    csRef<iDocumentNode> namenode = params->GetNode("name");
    if (!namenode) continue;
    csString name = namenode->GetContentsValue();
    
    if ((terraformerRE != 0) && (terraformerRE->Match (name) != csrxNoError))
      continue;
    
    csPrintf ("Found terraformer %s ...\n", CS::Quote::Single (name.GetData()));
    fflush (stdout);
    
    csRef<iDocumentNode> matmap = params->GetNode ("materialmap");
    if (matmap) 
    {
      const char* imageFile = matmap->GetAttributeValue("image");
      // Set the image.
      csRef<iImage> image = LoadImage (imageFile, CS_IMGFMT_PALETTED8);
      if (!image )
      {
	Report("Failed to load material map %s!", 
	  CS::Quote::Single (imageFile));
	continue;
      }
      csRef<AlphaLayers> layers;
      layers.AttachNew (new AlphaLayers);
      layers->BuildAlphaMapsFromMatMap (image);
      terrain1Layers.Put (name, layers);
      continue;
    }
    // Check for alpha maps
    csRef<iDocumentNodeIterator> alphaMapIt = 
      params->GetNodes ("materialalphamap");
    if (alphaMapIt->HasNext())
    {
      csRef<AlphaLayers> layers;
      layers.AttachNew (new AlphaLayers);
      
      while (alphaMapIt->HasNext())
      {
	csRef<iDocumentNode> child = alphaMapIt->Next();
	if (child->GetType() != CS_NODE_ELEMENT) continue;

        const char* imageFile = child->GetAttributeValue("image");
	csRef<iImage> image = LoadImage (imageFile, CS_IMGFMT_ANY);
	if (!image.IsValid())
	{
	  Report("Failed to load alpha map %s!", CS::Quote::Single (imageFile));
	  continue;
	}
	layers->AddAlphaMap (image);
      }
      layers->AddRemainderAlpha();
      terrain1Layers.Put (name, layers);
      continue;
    }
    // else: no alpha maps

    Report("Failed to load alpha maps or a material map for %s!", 
      CS::Quote::Single (name.GetData()));

    continue;
  } // while
}

void BaseMapGen::ScanTerrain1Factories ()
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
	|| (strcmp (plugin, "crystalspace.mesh.loader.factory.terrain") != 0))
      continue;
  
    const char* name = current->GetAttributeValue("name");
  
    csRef<iDocumentNode> params = current->GetNode ("params");
    if (!params) continue;
    csRef<iDocumentNode> former = params->GetNode ("terraformer");
    if (!former) continue;
    const char* formerName = former->GetContentsValue();
    
    terrain1FactoryLayers.Put (name,
      terrain1Layers.Get (formerName, (AlphaLayers*)0));
  } // while meshfact
}

void BaseMapGen::ScanTerrain1Meshes ()
{
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
	  || (strcmp (plugin, "crystalspace.mesh.loader.terrain") != 0))
	continue;
    
      csString name = current->GetAttributeValue("name");
      if ((meshRE != 0) && (meshRE->Match (name) != csrxNoError))
	continue;
    
      csRef<iDocumentNode> params = current->GetNode ("params");
      if (!params) continue;
      csRef<iDocumentNode> material = params->GetNode ("material");
      if (!material) continue;
      csRef<iDocumentNode> factory = params->GetNode ("factory");
      if (!factory) continue;
      
      const char* factname = factory->GetContentsValue();
      AlphaLayers* alphaLayers = terrain1FactoryLayers.Get (factname,
        (AlphaLayers*)0);
      if (!alphaLayers) continue;
    
      // Get texture file name.
      const char* matname = material->GetContentsValue();
      
      csPrintf ("Found terrain %s ...\n", CS::Quote::Single (name.GetData()));
      fflush (stdout);
    
      MaterialLayer* mat = materials.Get (matname, (MaterialLayer*)0);
      if (!mat) continue;
      
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
    
      csPrintf ("Found %zu materials...\n", mlayers.GetSize());
    
      // Get the resolution.
      int basemap_w = csFindNearestPowerOf2 (alphaLayers->GetWidth());
      int basemap_h = csFindNearestPowerOf2 (alphaLayers->GetHeight());
    
      // Get the resolution from the commandline.
      csString res = cmdline->GetOption ("resolution");
      if (!res.IsEmpty())
      {
	int basemap_res = csFindNearestPowerOf2 (atoi(res));
	basemap_w = basemap_h = basemap_res;
      }
    
      const TextureInfo* texinfo = mat->texture;
      if (!texinfo) continue;
      if (texinfo->GetFileName ().IsEmpty ()) continue;
      
      csPrintf ("Basemap resolution: %dx%d\n", basemap_w, basemap_h); fflush (stdout);
      
      csRef<iImage> basemap = CreateBasemap (basemap_w, basemap_h,
        *alphaLayers, mlayers);
      if (!basemap) continue;
      
      SaveImage (basemap, texinfo->GetFileName ());
    } // while meshobj
  } // while sector
}
