/*
    Copyright (C) 2007 by Jelle Hellemans aka sueastside

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

#include "basemapgen.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to basemapgen
BaseMapGen *basemapgen;

BaseMapGen::BaseMapGen (iObjectRegistry* object_reg)
{
  BaseMapGen::object_reg = object_reg;
}

BaseMapGen::~BaseMapGen ()
{
}

bool BaseMapGen::Initialize ()
{
  if (0 != 0)
  {
    Report("Not enough chickens sacrificed for cthulhu!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
  {
    Report("Can't initialize plugins!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    OnCommandLineHelp();
    return false;
  }

  cmdline = csQueryRegistry<iCommandLineParser> (object_reg);
  if (!cmdline)
  {
    Report("No iCommandLineParser plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report("Error opening system!");
    return false;
  }

  return true;
}

void BaseMapGen::Report(const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV(object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.application.basemapgen", msg, arg);
  va_end (arg);
}

void BaseMapGen::OnCommandLineHelp()
{
  csPrintf ("\n");
  csPrintf ("Usage: basemapgen path world [OPTIONS]\n");
  csPrintf (" Example: basemapgen /this/data/terrain world -terrainname=Terrain\n");
  csPrintf ("\n");
  csPrintf ("<path>                    The path to the worldfile        (default /this)\n");
  csPrintf ("<world>                   Name of the world file           (default 'world')\n");
  csPrintf ("-terrainname=<name>       Name of the terrain mesh object  (default 'Terrain')\n");
  csPrintf ("-terraformername=<name>   Name of the terraformer          (default 'simple')\n");
  csPrintf ("-resolution=<pixels>      The resolution for the basemap   (default basemap resolution)\n");
}

bool BaseMapGen::LoadMap ()
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iConfigManager> cfgmgr = csQueryRegistry<iConfigManager> (object_reg);

  csRef<iConfigFile> cfgfile = cfgmgr->GetDynamicDomain ();
  csPrintf ("Loading custom VFS mounts from:\n%s\n", cfgfile->GetFileName ());
  vfs->LoadMountsFromFile(cfgfile);
  
  csString path = cmdline->GetName(0);
  csString world = cmdline->GetName(1);

  if (path.IsEmpty()) path = "/this";
  if (world.IsEmpty()) world = "world";

  csStringArray paths;
  paths.Push ("/lev/");
  if (!vfs->ChDirAuto(path.GetData(), &paths, 0, "world"))
  {
    Report("Error setting directory '%s'!", path.GetData ());
    return false;
  }

  csRef<iFile> buf = vfs->Open(world.GetData(), VFS_FILE_READ);
  if (!buf)
  {
    Report("Failed to open file '%s'!", world.GetData());
    return false;
  }

  csRef<iDocument> doc;
  csRef<iDocumentSystem> docsys;

  if (!docsys) docsys = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  doc = docsys->CreateDocument ();
  const char* error = doc->Parse (buf, true);
  if (error != 0)
    return false;

  if (doc)
  {
    rootnode = doc->GetRoot()->GetNode("world");
    if (!rootnode)
      return false;
  }

  return true;
}

void BaseMapGen::AddMaterialLayer (csArray<MaterialLayer>& mat_layers, iDocumentNode* materialnode)
{
  MaterialLayer tl;
  csRef<iDocumentNodeIterator> it;

  // Set the texturename.
  csRef<iDocumentNode> texnode = materialnode->GetNode("texture");
  if(!texnode) return;
  tl.texture_name = texnode->GetContentsValue();

  // Set the texture scale.
  it = materialnode->GetNodes("shadervar");
  csVector2 texscale(32,32);
  while (it->HasNext())
  {
    csRef<iDocumentNode> current = it->Next();
    csString name = current->GetAttributeValue("name");
    if ( name.Compare("texture scale"))
    {
      csString scalestr = current->GetContentsValue();
      size_t pos = scalestr.FindFirst(",",0);
      csString firststr = scalestr.Slice(0,pos);
      csString secondstr = scalestr.Slice(pos+1, scalestr.Length());
      int first = atoi(firststr.GetData());
      int second = atoi(secondstr.GetData());
      texscale = csVector2(first, second);
      break;
    }
  } // while
  tl.texture_scale = texscale;

  // Set the texturefile.
  tl.texture_file = GetTextureFile(tl.texture_name);

  // Set the image.
  tl.image = LoadImage(tl.texture_file, CS_IMGFMT_TRUECOLOR);
   
  // Push it on the array.
  mat_layers.Push(tl);
}

csRef<iDocumentNode> BaseMapGen::GetTerrainNode ()
{
  csRef<iDocumentNode> terrainnode;

  csString terrainname = cmdline->GetOption("terrainname");
  if (terrainname.IsEmpty()) terrainname = "Terrain";

  csPrintf ("Trying to load terrain '%s' ...\n", terrainname.GetData());
  fflush (stdout);

   // Get the terrain node
  csRef<iDocumentNodeIterator> sectors = rootnode->GetNodes("sector");
  while (sectors->HasNext())
  {
    csRef<iDocumentNode> sector = sectors->Next();
    csRef<iDocumentNodeIterator> it = sector->GetNodes("meshobj");
    while (it->HasNext())
    {
      csRef<iDocumentNode> current = it->Next();
      csString name = current->GetAttributeValue("name");
      if (name.Compare(terrainname))
      {
        terrainnode = current;
        break;
      }
    } // while meshobj
  } // while sector

  if (!terrainnode.IsValid())
  {
    Report("Couldn't find terrain node!");
  }

  return terrainnode;
}

csString BaseMapGen::GetTextureFile (const csString& texturename)
{
  csRef<iDocumentNode> textures = rootnode->GetNode("textures");
  csRef<iDocumentNodeIterator> it = textures->GetNodes("texture");
  while (it->HasNext())
  {
    csRef<iDocumentNode> current = it->Next();
    csString name = current->GetAttributeValue("name");
    if (name.Compare(texturename))
    {
      csRef<iDocumentNode> file = current->GetNode("file");
      csString filename = file->GetContentsValue();
      return filename;
    }
  } // while


  Report("Texture '%s' not found!", texturename.GetData());

  return "";
}

csRef<iDocumentNode> BaseMapGen::GetMaterialNode (const csString& materialname)
{
  csRef<iDocumentNode> material;
  csRef<iDocumentNode> mats = rootnode->GetNode ("materials");
  if (!mats) return material;

  csRef<iDocumentNodeIterator> it_mats = mats->GetNodes("material");
  while (it_mats->HasNext())
  {
    csRef<iDocumentNode> mat = it_mats->Next();
    csString matname = mat->GetAttributeValue("name");
    if (materialname.Compare(matname))
    {
      material = mat;
      return material;
    }
  } // while

  return material;
}

csRefArray<iDocumentNode> BaseMapGen::GetMaterialNodes ()
{
  csRefArray<iDocumentNode> materials;
  csRef<iDocumentNode> terrainnode = GetTerrainNode();
  if (!terrainnode) return materials;

  // Get the materialpalette.
  csRef<iDocumentNode> params = terrainnode->GetNode ("params");
  if (!params) return materials;
  csRef<iDocumentNode> materialpalette = params->GetNode ("materialpalette");
  if (!materialpalette) return materials;
  csRef<iDocumentNode> mats = rootnode->GetNode ("materials");
  if (!materialpalette) return materials;

  // Get the materials.
  csRef<iDocumentNodeIterator> it = materialpalette->GetNodes("material");
  while (it->HasNext())
  {
    csRef<iDocumentNode> pal = it->Next();
    csString matname = pal->GetContentsValue();
    csRef<iDocumentNode> matnode = GetMaterialNode(matname);
    if (matnode.IsValid())
      materials.Push(matnode);
  } // while

  csPrintf ("Found %d materials...\n", materials.GetSize()); fflush (stdout);

  return materials;
}

bool BaseMapGen::CopyAlphaMapsToLayers (const csRefArray<iImage>& alphaMaps,
                                        int& matmap_w, int& matmap_h, 
                                        csArray<MaterialLayer>& mat_layers)
{
  matmap_w = alphaMaps[0]->GetWidth();
  matmap_h = alphaMaps[0]->GetHeight();
  for (size_t l = 1; l < alphaMaps.GetSize(); l++)
  {
    if ((alphaMaps[l]->GetWidth() != matmap_w)
      || (alphaMaps[l]->GetHeight() != matmap_h))
    {
      Report ("Alpha maps don't match in size.");
      return false;
    }
  }

  csArray<int> total;
  size_t numPix = matmap_w * matmap_h;
  total.SetSize (numPix, 0);
  for (size_t l = 0; l < mat_layers.GetSize(); l++)
  {
    if (l > alphaMaps.GetSize()) break;
    mat_layers[l].alphaMap = (uint8*)cs_malloc (numPix);
    uint8* layerMap = mat_layers[l].alphaMap;

    if (l == alphaMaps.GetSize())
    {
      // The last alpha map is 255 minus the sum of all other alpha maps
      for (size_t n = 0; n < numPix; n++)
      {
        *layerMap++ = csClamp (255 - total[n], 255, 0);
      }
    }
    else
    {
      uint8* map = (uint8*)alphaMaps[l]->GetImageData();
      for (size_t n = 0; n < numPix; n++)
      {
        uint8 v = *map++;
        *layerMap++ = v;
        total[n] += v;
      }
    }
  }
  return true;
}

bool BaseMapGen::GetMaterialMaps (csArray<MaterialLayer>& mat_layers,
                                  int& matmap_w, int& matmap_h)
{
  csString terrname = cmdline->GetOption("terraformername");
  if (terrname.IsEmpty()) terrname = "simple";

  csPrintf ("Trying to obtain material masks for '%s' ...\n", terrname.GetData());
  fflush (stdout);

  // Get the terraformer node
  csRef<iDocumentNodeIterator> it = rootnode->GetNodes("addon");
  while (it->HasNext())
  {
    csRef<iDocumentNode> current = it->Next();
    csRef<iDocumentNode> params = current->GetNode ("params");
    if (!params) params = current;
    csRef<iDocumentNode> namenode = params->GetNode("name");
    if (!namenode) continue;
    csString name = namenode->GetContentsValue();
    if ( name.Compare(terrname))
    {
      csRef<iDocumentNode> matmap = params->GetNode ("materialmap");
      if (matmap) 
      {
        // Set the image.
        ImageMap imagemap;
        imagemap.texture_file = matmap->GetAttributeValue("image");
        imagemap.image = LoadImage(imagemap.texture_file, CS_IMGFMT_PALETTED8);
        if (!imagemap.image)
        {
          Report("Failed to load material map '%s'!", 
            imagemap.texture_file.GetData());
          return false;
        }
        matmap_w = imagemap.image->GetWidth ();
        matmap_h = imagemap.image->GetHeight ();
        BuildAlphaMapsFromMatMap (imagemap, mat_layers);
        return true;
      }
      // Check for alpha maps
      csRef<iDocumentNodeIterator> alphaMapIt = 
        params->GetNodes ("materialalphamap");
      if (alphaMapIt->HasNext())
      {
        csRefArray<iImage> alphaMaps;
        while (alphaMapIt->HasNext())
        {
          csRef<iDocumentNode> child = alphaMapIt->Next();
          if (child->GetType() != CS_NODE_ELEMENT) continue;

          ImageMap imagemap;
          imagemap.texture_file = child->GetAttributeValue("image");
          imagemap.image = LoadImage(imagemap.texture_file, CS_IMGFMT_PALETTED8);
          if (!imagemap.image)
          {
            Report("Failed to load alpha map '%s'!", 
              imagemap.texture_file.GetData());
            return false;
          }
          alphaMaps.Push (imagemap.image);
        }
        if (alphaMaps.GetSize() > 0)
          return CopyAlphaMapsToLayers (alphaMaps, matmap_w, matmap_h,
            mat_layers);
      }
      // else: no alpha maps

      Report("Failed to load alpha maps or a material map for '%s'!", 
        terrname.GetData());

      return false;
    }
  } // while

  Report("Couldn't find terraformer node for '%s'!", terrname.GetData());

  return false;
}

BaseMapGen::ImageMap BaseMapGen::GetBaseMap ()
{
  ImageMap imagemap;

  csRef<iDocumentNode> terrainnode = GetTerrainNode();
  if (!terrainnode) return imagemap;


  csRef<iDocumentNode> params = terrainnode->GetNode ("params");
  if (!params) return imagemap;
  csRef<iDocumentNode> material = params->GetNode ("material");
  if (!material) return imagemap;

  // Get texture file name.
  csString matname = material->GetContentsValue();
  csRef<iDocumentNode> matnode = GetMaterialNode(matname);
  if (!matnode) return imagemap;
  csRef<iDocumentNode> tex = matnode->GetNode ("texture");
  if (!tex) return imagemap;
  csString texname = tex->GetContentsValue();
  imagemap.texture_file = GetTextureFile(texname);

  return imagemap;
}

csRef<iImage> BaseMapGen::LoadImage (const csString& filename, int format)
{
  csPrintf ("Trying to load '%s'... \t", filename.GetData());
  fflush (stdout);
  csRef<iImage> image;
  csRef<iImageIO> imageio = csQueryRegistry<iImageIO> (object_reg);
  csRef<iVFS> VFS = csQueryRegistry<iVFS> (object_reg);

  csRef<iDataBuffer> buf = VFS->ReadFile (filename.GetData(), false);
  if (!buf)
  {
    Report ("Failed to load image file '%s'!", filename.GetData());
    return image;
  }
  image = imageio->Load (buf, format);
  image.IsValid() ? csPrintf ("success.\n") : csPrintf ("failed.\n");

  return image;
}

void BaseMapGen::SaveImage (BaseMapGen::ImageMap image)
{
  csRef<iImageIO> imageio = csQueryRegistry<iImageIO> (object_reg);
  csRef<iVFS> VFS = csQueryRegistry<iVFS> (object_reg);

  csRef<iDataBuffer> db = imageio->Save (image.image, "image/png", "progressive");
  if (db)
  {
    if (!VFS->WriteFile (image.texture_file.GetData (), (const char*)db->GetData (), db->GetSize ()))
    {
      Report("Failed to write file '%s'!", image.texture_file.GetData ());
      return;
    }
  }
  else
  {
    Report("Failed to save png image for basemap!");
    return;
  }
}

void BaseMapGen::DrawProgress (int percent)
{
  const uint progTotal = 65;
  uint numDone = (progTotal*percent) / 100;

  csPrintf (CS_ANSI_CLEAR_LINE);
  csPrintf (CS_ANSI_CURSOR_BWD(70));
  csPrintf (CS_ANSI_CURSOR_UP(1));

  printf("[");
  uint x;
  for (x = 0 ; x < numDone ; x++)
    printf("=");

  for (; x < progTotal; x++)
    printf(" ");

  csPrintf("] %d%% \n", percent);

  fflush (stdout);

}

static csColor GetPixelWrap (iImage* img, int img_w, int img_h, 
                             int x, int y)
{
  // Wrap around the texture coordinates.
  x = x % img_w;
  y = y % img_h;

  // Get the pixel.
  csRGBpixel* px = (csRGBpixel*)img->GetImageData() + x + (y * img_h);
  return csColor (px->red, px->green, px->blue);
}

csColor BaseMapGen::GetPixel (const MaterialLayer& material, 
                              float coord_x, float coord_y)
{
  const int img_w = material.image->GetWidth();
  const int img_h = material.image->GetHeight();

  // Scale the texture coordinates.
  coord_x *= material.texture_scale.x;
  coord_y *= material.texture_scale.y;

  // Calculate the material coordinates.
  float matcoord_x_f = (coord_x * img_w);
  float matcoord_y_f = (coord_y * img_w);
  int matcoord_x = int (matcoord_x_f);
  int matcoord_y = int (matcoord_y_f);

  // Bilinearly filter from material.
  csColor p00 (GetPixelWrap (material.image, img_w, img_h,
    matcoord_x, matcoord_y));
  csColor p01 (GetPixelWrap (material.image, img_w, img_h,
    matcoord_x, matcoord_y+1));
  csColor p11 (GetPixelWrap (material.image, img_w, img_h,
    matcoord_x+1, matcoord_y+1));
  csColor p10 (GetPixelWrap (material.image, img_w, img_h,
    matcoord_x+1, matcoord_y));

  float f1 = matcoord_x_f - matcoord_x;
  float f2 = matcoord_y_f - matcoord_y;

  return csLerp (csLerp (p00, p10, f1), 
    csLerp (p01, p11, f1), f2);
}

void BaseMapGen::CreateBasemap (int basemap_w, int basemap_h, 
                                ImageMap& basetexture, 
                                int matmap_w, int matmap_h, 
                                const csArray<MaterialLayer>& mat_layers)
{
  csPrintf ("Creating base texturemap... \n\n"); fflush (stdout);
  // Block: to measure time
  {
    CS::MeasureTime lTimeMeasurer ("Time taken");

    basetexture.image.AttachNew (new csImageMemory (basemap_w, basemap_h));
    // Get image data.
    csRGBpixel* bm_dst = (csRGBpixel*)(basetexture.image->GetImageData());

    int layercoord_x, layercoord_y;
    float coord_x, coord_y;

    float inv_basemap_w = 1.0f / basemap_w;
    float inv_basemap_h = 1.0f / basemap_h;
    float basemap_x_to_matmap = inv_basemap_w * matmap_w;
    float basemap_y_to_matmap = inv_basemap_h * matmap_h;

    for (int y = 0 ; y < basemap_h ; y++)
    {
      // Draw progress.
      uint percent = uint ((y*100) * inv_basemap_h);
      DrawProgress(percent);

      for (int x = 0 ; x < basemap_w ; x++)
      {
        // Get the layer index.
        layercoord_x =  (int) (x * basemap_x_to_matmap);
        layercoord_y =  (int) (y * basemap_y_to_matmap);

        // Calculate the destination coordinates.
        coord_x    = x * inv_basemap_w;
        coord_y    = y * inv_basemap_h;

        csColor col (0, 0, 0);
        size_t layerPos = layercoord_x + (layercoord_y * matmap_w);
        for (size_t l = 0; l < mat_layers.GetSize(); l++)
        {
          const MaterialLayer& layer = mat_layers[l];
          if (!layer.image.IsValid()) continue;
          if (!layer.alphaMap) continue;
          if (!layer.alphaMap[layerPos]) continue;
          float a = layer.alphaMap[layerPos] * (1.0f/255.0f);
          // Blend material colors.
          col += GetPixel (layer, coord_x, coord_y) * a;
        }

        csRGBpixel mat_dst (int (col.red), int (col.green), int (col.blue));

        // Set the basemap pixel.
        bm_dst->Set (mat_dst.red, mat_dst.green, mat_dst.blue);

        // Increase the pointer.
        bm_dst++;

      } // for y
    } // for x

  } // Block to measure time

}

void BaseMapGen::BuildAlphaMapsFromMatMap (const ImageMap& matmap,
                                           csArray<MaterialLayer>& mat_layers)
{
  int matmap_w = matmap.image->GetWidth ();
  int matmap_h = matmap.image->GetHeight ();
  size_t numPix = matmap_w * matmap_h;

  uint8* matmapPtr = (uint8*)(matmap.image->GetImageData ());
  for (size_t n = 0; n < numPix; n++)
  {
    unsigned int layerNum = *matmapPtr++;

    if (layerNum > mat_layers.GetSize()) continue;
    MaterialLayer& layer = mat_layers[layerNum];

    if (layer.alphaMap == 0)
    {
      layer.alphaMap = (uint8*)cs_calloc (numPix, 1);
    }
    layer.alphaMap[n] = 255;
  }
}

/* Get mipmap for an image, using precomputed mipmaps as far as
   possible. */
static csRef<iImage> GetImageMip (iImage* img, uint mip)
{
  if (mip == 0) return img;
  csRef<iImage> imgToMip (img);
  uint hasMips = img->HasMipmaps();
  if (mip <= hasMips) return img->GetMipmap (mip);
  imgToMip = img->GetMipmap (hasMips);
  mip -= hasMips;
  if (mip == 0) return imgToMip;
  return csImageManipulate::Mipmap (imgToMip, mip);
}

void BaseMapGen::Start ()
{
  // Load the world.
  if (!LoadMap ())
  {
    Report("Error opening world file!");
    return;
  }

  // Iterate the materials on the terrain.
  csArray<MaterialLayer> mat_layers;
  csRefArray<iDocumentNode> materials = GetMaterialNodes();
  for (unsigned int i = 0 ; i < materials.GetSize() ; i++)
  {
    AddMaterialLayer (mat_layers, materials.Get(i));
  }

  // Get the materialmaps.
  int matmap_w, matmap_h;
  if (!GetMaterialMaps (mat_layers, matmap_w, matmap_h))
  {
    return;
  }

  // Get the resolution.
  int basemap_w = csFindNearestPowerOf2 (matmap_w);
  int basemap_h = csFindNearestPowerOf2 (matmap_h);

  // Get the resolution from the commandline.
  csString res = cmdline->GetOption("resolution");
  if (!res.IsEmpty())
  {
    int basemap_res = csFindNearestPowerOf2(atoi(res));
    basemap_w = basemap_h = basemap_res;
  }

  csPrintf ("Basemap resolution: %dx%d\n", basemap_w, basemap_h);

  /* Mipmap the materials to the highest mipmap needed below the required 
   * resolution. This mipmap is then upsampled. */
  for (unsigned int i = 0 ; i < mat_layers.GetSize() ; i++)
  {
    float layer_needed_x = float (basemap_w) / mat_layers[i].texture_scale.x;
    float layer_needed_y = float (basemap_w) / mat_layers[i].texture_scale.y;
    int mip_x = csFindNearestPowerOf2 (
      int (ceil (mat_layers[i].image->GetWidth() / layer_needed_x)));
    int mip_y = csFindNearestPowerOf2 (
      int (ceil (mat_layers[i].image->GetHeight() / layer_needed_y)));
    int mip = csMax (
      csClamp (csLog2 (mip_x), csLog2 (mat_layers[i].image->GetWidth()), 0),
      csClamp (csLog2 (mip_y), csLog2 (mat_layers[i].image->GetHeight()), 0));
    mat_layers[i].image = GetImageMip (mat_layers[i].image, mip);
  }

  // Create the basemap.
  ImageMap basetexture (GetBaseMap());
  CreateBasemap (basemap_w, basemap_h, basetexture, 
    matmap_w, matmap_h, mat_layers);

  // Save the basemap.
  csPrintf ("Saving %zu KB of data.\n", 
    csImageTools::ComputeDataSize(basetexture.image)/1024);
  SaveImage (basetexture);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  basemapgen = new BaseMapGen (object_reg);

  if (basemapgen->Initialize ())
    basemapgen->Start ();

  delete basemapgen;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}
