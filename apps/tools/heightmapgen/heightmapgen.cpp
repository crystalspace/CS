/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#include "heightmapgen.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to heightmapgen
HeightMapGen *heightmapgen;

HeightMapGen::HeightMapGen (iObjectRegistry* object_reg)
{
  HeightMapGen::object_reg = object_reg;
}

HeightMapGen::~HeightMapGen ()
{
}

bool HeightMapGen::LoadMap ()
{
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  csStringArray paths;
  paths.Push ("/lev/");
  csString map = cfgmgr->GetStr ("HeightMapGen.WorldInputPath", "/this");
  csString world = cfgmgr->GetStr ("HeightMapGen.WorldInput", "world");
  if (!VFS->ChDirAuto (map, &paths, 0, world))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"Error setting directory '%s'!", map.GetData ());
    return false;
  }

  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile (world))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"Couldn't load level '%s' on '%s'!", world.GetData (), map.GetData ());
    return false;
  }
  engine->Prepare ();

  return true;
}

bool HeightMapGen::Initialize ()
{
  csDebuggingGraph::SetupGraph (object_reg);

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_PLUGIN("crystalspace.graphics3d.null", iGraphics3D),
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_PLUGIN("crystalspace.collisiondetection.opcode",
		iCollideSystem),
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"Can't initialize plugins!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (!vc)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"Error opening system!");
    return false;
  }

  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  const char* configfile = cmdline->GetName (0);
  if (!configfile)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"Expected name of a config file as a parameter!");
    return false;
  }
  config.AddConfig (object_reg, configfile);
  cfgmgr = (iConfigFile*)config;

  if (!LoadMap ()) return false;
  return true;
}

static csRef<iDocumentNode> CreateNode (iDocumentNode* parent,
	const char* name)
{
  csRef<iDocumentNode> node = parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  node->SetValue (name);
  return node;
}

static csRef<iDocumentNode> CreateNode (iDocumentNode* parent,
	const char* name, const char* value)
{
  csRef<iDocumentNode> node = parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  node->SetValue (name);
  csRef<iDocumentNode> node_value = node->CreateNodeBefore (
      	CS_NODE_TEXT, 0);
  node_value->SetValue (value);
  return node;
}

void HeightMapGen::CreateHeightmap (int heightmap_res, iCollideSystem* cdsys, 
                                     iSector* sector, csRGBpixel* hmap_dst, 
                                     float* height_dst, 
                                     const csBox3& box)
{
  csPrintf ("Creating heightmap...\n"); fflush (stdout);

  float dx = (box.MaxX () - box.MinX () - 0.2) / float (heightmap_res-1);
  float dz = (box.MaxZ () - box.MinZ () - 0.2) / float (heightmap_res-1);
  for (int z = 0 ; z < heightmap_res ; z++)
  {
    for (int x = 0 ; x < heightmap_res ; x++)
    {
      csVector3 start, end;
      start.x = box.MinX () + (float)x * dx + 0.1;
      start.y = box.MaxY () + 10.0;
      start.z = box.MinZ () + (heightmap_res-z-1) * dz + 0.1;
      end = start;
      end.y = box.MinY () - 10.0;
      csVector3 isect;
      //mesh->HitBeamObject (start, end, isect, 0, 0);
      csIntersectingTriangle closest_tri;
      csColliderHelper::TraceBeam (cdsys, sector, start, end,
      	false, closest_tri, isect);
      float y = (isect.y - box.MinY ()) / (box.MaxY () - box.MinY ());
      if (y < 0) y = 0;
      else if (y > 0.9999f) y = 0.9999f;
      *height_dst++ = y;
      y *= 256.0;
      hmap_dst->Set (int (y), int (y), int (y));
      hmap_dst++;
    }
  }
}

void HeightMapGen::CreateBasemap (int heightmap_res, csRGBpixel* basemap_dst, 
                                   uint8* matmap_dst, const float* height_dst,
                                   const csArray<TextureLayer>& txt_layers)
{
  csPrintf ("Creating base texturemap...\n"); fflush (stdout);
  for (int z = 0 ; z < heightmap_res ; z++)
  {
    csRGBpixel* bm_dst = basemap_dst + (heightmap_res-z-1) * heightmap_res;
    uint8* mm_dst = matmap_dst + (heightmap_res-z-1) * heightmap_res;
    for (int x = 0 ; x < heightmap_res ; x++)
    {
      float y = *height_dst++;
      (void)y;

      int layer;
      //for (layer = 0 ; layer < num_texture_layers ; layer++)
      //{
        //if (y >= txt_layers[layer].min_height
		//&& y <= txt_layers[layer].max_height)
	  //break;
      //}
      //*mm_dst++ = layer;
      layer = *mm_dst++;
      bm_dst->Set (
      	int (txt_layers[layer].average.red * 255.0),
      	int (txt_layers[layer].average.green * 255.0),
      	int (txt_layers[layer].average.blue * 255.0)
      	);
      bm_dst++;
    }
  }
}

void HeightMapGen::Start ()
{
  csRef<iImageIO> imageio = CS_QUERY_REGISTRY (object_reg, iImageIO);
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  csString meshname = cfgmgr->GetStr ("HeightMapGen.MeshInput", "");
  iMeshWrapper* mesh = engine->FindMeshObject (meshname);
  if (!mesh)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
    	"Can't find mesh '%s'!", meshname.GetData ());
    return;
  }

  csBox3 box;
  //mesh->GetMeshObject ()->GetObjectModel ()->GetObjectBoundingBox (box);
  mesh->GetWorldBoundingBox (box);
  csPrintf ("box=%g,%g,%g   %g,%g,%g\n",
  	box.MinX (), box.MinY (), box.MinZ (),
  	box.MaxX (), box.MaxY (), box.MaxZ ());

  csRef<iCollideSystem> cdsys = CS_QUERY_REGISTRY (object_reg, iCollideSystem);
  csColliderHelper::InitializeCollisionWrapper (cdsys, mesh);
  iSector* sector = mesh->GetMovable ()->GetSectors ()->Get (0);

  // Heightmap resolution.
  int heightmap_res = cfgmgr->GetInt ("HeightMapGen.HeightmapResolution", 513);
  int num_texture_layers = cfgmgr->GetInt ("HeightMapGen.NumTextureLayers", 3);
  csArray<TextureLayer> txt_layers;
  txt_layers.SetLength (num_texture_layers);
  int i;
  for (i = 0 ; i < num_texture_layers ; i++)
  {
    csString str;
    str.Format ("HeightMapGen.Layer%d.", i);
    txt_layers[i].min_height = cfgmgr->GetInt ((str+"MinHeight").GetData ());
    txt_layers[i].max_height = cfgmgr->GetInt ((str+"MaxHeight").GetData ());
    txt_layers[i].texture_name = cfgmgr->GetStr (
    	(str+"TextureName").GetData (), "");
    txt_layers[i].texture_file = cfgmgr->GetStr (
    	(str+"TextureFile").GetData (), "");
  }

  csRef<csImageMemory> heightmap_img;
  heightmap_img.AttachNew (new csImageMemory (
  	heightmap_res, heightmap_res));
  csRef<csImageMemory> basetexture_img;
  basetexture_img.AttachNew (new csImageMemory (
  	heightmap_res, heightmap_res));

  csString materialmap_input = cfgmgr->GetStr (
  	"HeightMapGen.MaterialMapInput", "");
  csRef<iImage> materialmap_img;
  {
    csRef<iDataBuffer> buf = VFS->ReadFile (materialmap_input, false);
    materialmap_img = imageio->Load (buf, CS_IMGFMT_PALETTED8);
  }

  //csRef<csImageMemory> materialmap_img;
  //materialmap_img.AttachNew (new csImageMemory (
  	//heightmap_res, heightmap_res, CS_IMGFMT_PALETTED8));
  //materialmap_img->GetPalette ()[0].Set (0, 0, 0);
  //materialmap_img->GetPalette ()[1].Set (255, 255, 255);
  //materialmap_img->GetPalette ()[2].Set (255, 0, 0);
  //materialmap_img->GetPalette ()[3].Set (0, 255, 0);
  //materialmap_img->GetPalette ()[4].Set (0, 0, 255);
  //materialmap_img->GetPalette ()[5].Set (255, 0, 255);
  //materialmap_img->GetPalette ()[6].Set (255, 255, 0);
  //materialmap_img->GetPalette ()[7].Set (0, 255, 255);

  for (i = 0 ; i < num_texture_layers ; i++)
  {
    csRef<iDataBuffer> buf = VFS->ReadFile (txt_layers[i].texture_file, false);
    if (!buf)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"Failed to read texture file '%s'!", txt_layers[i].texture_file.GetData());
      return;
    }
    txt_layers[i].image = imageio->Load (buf, CS_IMGFMT_TRUECOLOR);
    csRGBpixel* img = (csRGBpixel*)(txt_layers[i].image->GetImageData ());
    int w = txt_layers[i].image->GetWidth ();
    int h = txt_layers[i].image->GetHeight ();
    long r = 0, g = 0, b = 0;
    int x, y;
    for (y = 0 ; y < h ; y++)
      for (x = 0 ; x < w ; x++)
      {
        r += img->red;
        g += img->green;
        b += img->blue;
	img++;
      }
    txt_layers[i].average.Set (
    	float (r / (w*h)) / 255.0,
    	float (g / (w*h)) / 255.0,
    	float (b / (w*h)) / 255.0);
    csPrintf ("Average color for %s: %g,%g,%g\n",
    	txt_layers[i].texture_file.GetData(),
	txt_layers[i].average.red,
	txt_layers[i].average.green,
	txt_layers[i].average.blue);
    fflush (stdout);
  }

  csRGBpixel* hmap_dst = (csRGBpixel*)(heightmap_img->GetImageData ());
  float* height = new float[heightmap_res*heightmap_res];
  float* height_dst = height;

  // Create the heightmap first.
  CreateHeightmap (heightmap_res, cdsys, sector, hmap_dst, height_dst, box);

  // Now create the base texture and material map.
  csRGBpixel* basemap_dst = (csRGBpixel*)(basetexture_img->GetImageData ());
  uint8* matmap_dst = (uint8*)(materialmap_img->GetImageData ());
  CreateBasemap (heightmap_res, basemap_dst, matmap_dst, height_dst, 
    txt_layers);

  delete[] height;

  csString heightmap_output_file = cfgmgr->GetStr (
    	"HeightMapGen.HeightMapOutput", "");
  csString basemap_name = cfgmgr->GetStr (
    	"HeightMapGen.BaseMapName", "basemap");
  csString basemap_output_file = cfgmgr->GetStr (
    	"HeightMapGen.BaseMapOutput", "");
  csString terrain_meshfactname = cfgmgr->GetStr (
    	"HeightMapGen.TerrainFactoryName", "TerrainFact");
  csString terrain_meshname = cfgmgr->GetStr (
    	"HeightMapGen.TerrainObjectName", "Terrain");
  csString addon_name = cfgmgr->GetStr (
    	"HeightMapGen.TerrainAddonName", "simple");
  csString sector_name = cfgmgr->GetStr (
    	"HeightMapGen.SectorName", "room");
  csString world_name = cfgmgr->GetStr (
  	"HeightMapGen.WorldOutput", "/this/world");

  csPrintf ("Writing images...\n"); fflush (stdout);
  csRef<iDataBuffer> db = imageio->Save (heightmap_img,
    	"image/png", "progressive");
  if (db)
  {
    if (!VFS->WriteFile (heightmap_output_file,
        (const char*)db->GetData (), db->GetSize ()))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"Failed to write file '%s'!", heightmap_output_file.GetData ());
      return;
    }
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"Failed to save png image for heightmap!");
    return;
  }

  csRef<iDataBuffer> db_base = imageio->Save (basetexture_img,
    	"image/png", "progressive");
  if (db_base)
  {
    if (!VFS->WriteFile (basemap_output_file,
    	(const char*)db_base->GetData (), db_base->GetSize ()))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"Failed to write file '%s'!", basemap_output_file.GetData ());
      return;
    }
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.heightmapgen",
	"Failed to save png image for basemap!");
    return;
  }

  csPrintf ("Writing world...\n"); fflush (stdout);
  csRef<iDocumentSystem> docsys;
  docsys.AttachNew (new csTinyDocumentSystem ());
  csRef<iDocument> doc = docsys->CreateDocument ();
  csRef<iDocumentNode> root = doc->CreateRoot ();
  csRef<iDocumentNode> world = CreateNode (root, "world");

  //========================================================================
  // Textures
  //========================================================================
  {
    csRef<iDocumentNode> node_textures = CreateNode (world, "textures");
    for (i = 0 ; i < num_texture_layers ; i++)
    {
      csRef<iDocumentNode> node_texture = CreateNode (node_textures, "texture");
      node_texture->SetAttribute ("name", txt_layers[i].texture_name);
      CreateNode (node_texture, "file", txt_layers[i].texture_file);
    }
    csRef<iDocumentNode> node_texture = CreateNode (node_textures, "texture");
    node_texture->SetAttribute ("name", basemap_name);
    CreateNode (node_texture, "file", basemap_output_file);
  }
  //========================================================================
  // Shaders
  //========================================================================
  {
    csRef<iDocumentNode> node_shaders = CreateNode (world, "shaders");
    csRef<iDocumentNode> node_shader1 = CreateNode (node_shaders, "shader");
    CreateNode (node_shader1, "file", "/shader/terrain_fixed_base.xml");
    csRef<iDocumentNode> node_shader2 = CreateNode (node_shaders, "shader");
    CreateNode (node_shader2, "file", "/shader/terrain_fixed_splatting.xml");
  }
  //========================================================================
  // Materials
  //========================================================================
  {
    csRef<iDocumentNode> node_materials = CreateNode (world, "materials");
    for (i = 0 ; i < num_texture_layers ; i++)
    {
      csRef<iDocumentNode> node_material = CreateNode (node_materials,
      	"material");
      node_material->SetAttribute ("name", txt_layers[i].texture_name);
      CreateNode (node_material, "texture", txt_layers[i].texture_name);
      csRef<iDocumentNode> node_shadervar = CreateNode (node_material,
      	"shadervar", "16,16");
      node_shadervar->SetAttribute ("name", "texture scale");
      node_shadervar->SetAttribute ("type", "vector2");
      csRef<iDocumentNode> node_shader = CreateNode (node_material,
      	"shader", "terrain_fixed_splatting");
      node_shader->SetAttribute ("type", "terrain splat");
    }
    csRef<iDocumentNode> node_material = CreateNode (node_materials,
      	"material");
    node_material->SetAttribute ("name", basemap_name);
    CreateNode (node_material, "texture", basemap_name);
    csRef<iDocumentNode> node_shader = CreateNode (node_material,
      	"shader", "terrain_fixed_base");
    node_shader->SetAttribute ("type", "ambient");
  }
  //========================================================================
  // Renderloop addon.
  //========================================================================
  {
    csRef<iDocumentNode> node_addon = CreateNode (world, "addon");
    CreateNode (node_addon, "plugin", "crystalspace.renderloop.loop.loader");
    csRef<iDocumentNode> node_params = CreateNode (node_addon, "params");
    CreateNode (node_params, "name", "TerrainLoop");
    csRef<iDocumentNode> node_steps = CreateNode (node_params, "steps");

    csRef<iDocumentNode> node_step_amb = CreateNode (node_steps, "step");
    node_step_amb->SetAttribute ("plugin",
    	"crystalspace.renderloop.step.generic");
    CreateNode (node_step_amb, "shadertype", "ambient");
    CreateNode (node_step_amb, "zoffset", "yes");
    CreateNode (node_step_amb, "portaltraversal");
    CreateNode (node_step_amb, "zuse");

    csRef<iDocumentNode> node_step_splat = CreateNode (node_steps, "step");
    node_step_splat->SetAttribute ("plugin",
    	"crystalspace.renderloop.step.generic");
    CreateNode (node_step_splat, "shadertype", "terrain splat");
    CreateNode (node_step_splat, "zoffset", "no");
    CreateNode (node_step_splat, "zuse");

    csRef<iDocumentNode> node_step_geom = CreateNode (node_steps, "step");
    node_step_geom->SetAttribute ("plugin",
    	"crystalspace.renderloop.step.generic");
    CreateNode (node_step_geom, "shadertype", "standard");
    CreateNode (node_step_geom, "zoffset", "no");
    CreateNode (node_step_geom, "zuse");
  }
  //========================================================================
  // Settings
  //========================================================================
  {
    csRef<iDocumentNode> node_settings = CreateNode (world, "settings");
    CreateNode (node_settings, "clearzbuf", "yes");
    CreateNode (node_settings, "clearscreen", "yes");
  }
  //========================================================================
  // Start
  //========================================================================
  {
    csRef<iDocumentNode> node_start = CreateNode (world, "start");
    CreateNode (node_start, "sector", sector_name);
    csRef<iDocumentNode> node_pos = CreateNode (node_start, "position");
    node_pos->SetAttributeAsInt ("x",
    	cfgmgr->GetInt("HeightMapGen.WorldStart.x",0));
    node_pos->SetAttributeAsInt ("y",
    	cfgmgr->GetInt("HeightMapGen.WorldStart.y",0));
    node_pos->SetAttributeAsInt ("z",
    	cfgmgr->GetInt("HeightMapGen.WorldStart.z",0));
  }
  //========================================================================
  // TerraFormer addon.
  //========================================================================
  {
    csRef<iDocumentNode> node_addon = CreateNode (world, "addon");
    CreateNode (node_addon, "plugin", "crystalspace.terraformer.simple.loader");
    csRef<iDocumentNode> node_params = CreateNode (node_addon, "params");
    CreateNode (node_params, "name", addon_name);
    CreateNode (node_params, "heightmap", heightmap_output_file);
    csRef<iDocumentNode> node_scale = CreateNode (node_params, "scale");
    node_scale->SetAttributeAsFloat ("x", (box.MaxX ()-box.MinX ())/2.0f);
    node_scale->SetAttributeAsFloat ("y", box.MaxY ()-box.MinY ());
    node_scale->SetAttributeAsFloat ("z", (box.MaxZ ()-box.MinZ ())/2.0f);
  }
  //========================================================================
  // Terrain factory.
  //========================================================================
  {
    csRef<iDocumentNode> node_fact = CreateNode (world, "meshfact");
    node_fact->SetAttribute ("name", terrain_meshfactname);
    CreateNode (node_fact, "plugin",
    	"crystalspace.mesh.loader.factory.terrain");
    csRef<iDocumentNode> node_params = CreateNode (node_fact, "params");
    CreateNode (node_params, "plugin",
    	"crystalspace.mesh.object.terrain.bruteblock");
    CreateNode (node_params, "terraformer", addon_name);
    csRef<iDocumentNode> node_sample = CreateNode (node_params, "sampleregion");
    csRef<iDocumentNode> node_min = CreateNode (node_sample, "min");
    node_min->SetAttributeAsFloat ("x", box.MinX ());
    node_min->SetAttributeAsFloat ("y", box.MinZ ());
    csRef<iDocumentNode> node_max = CreateNode (node_sample, "max");
    node_max->SetAttributeAsFloat ("x", box.MaxX ());
    node_max->SetAttributeAsFloat ("y", box.MaxZ ());
  }
  //========================================================================
  // Sector.
  //========================================================================
  {
    csRef<iDocumentNode> node_sector = CreateNode (world, "sector");
    node_sector->SetAttribute ("name", sector_name);
    CreateNode (node_sector, "renderloop", "TerrainLoop");
    csRef<iDocumentNode> node_obj = CreateNode (node_sector, "meshobj");
    node_obj->SetAttribute ("name", terrain_meshname);
    CreateNode (node_obj, "plugin", "crystalspace.mesh.loader.terrain");
    csRef<iDocumentNode> node_params = CreateNode (node_obj, "params");
    CreateNode (node_params, "factory", terrain_meshfactname);
    CreateNode (node_params, "material", basemap_name);
    csRef<iDocumentNode> node_pal = CreateNode (node_params, "materialpalette");
    for (i = 0 ; i < num_texture_layers ; i++)
    {
      CreateNode (node_pal, "material", txt_layers[i].texture_name);
    }
    CreateNode (node_params, "lodvalue", cfgmgr->GetStr (
    	"HeightMapGen.LodSplattingDistance", "250"))
    	->SetAttribute ("name", "splatting distance");
    CreateNode (node_params, "lodvalue", cfgmgr->GetStr (
    	"HeightMapGen.LodBlockResolution", "16"))
    	->SetAttribute ("name", "block resolution");
    CreateNode (node_params, "lodvalue", cfgmgr->GetStr (
    	"HeightMapGen.LodBlockSplitDistance", "8"))
    	->SetAttribute ("name", "block split distance");
    CreateNode (node_params, "lodvalue", cfgmgr->GetStr (
    	"HeightMapGen.LodMinimumBlockSize", "32"))
    	->SetAttribute ("name", "minimum block size");
    CreateNode (node_params, "lodvalue", cfgmgr->GetStr (
    	"HeightMapGen.LodCDResolution", "256"))
    	->SetAttribute ("name", "cd resolution");
    CreateNode (node_params, "lodvalue", cfgmgr->GetStr (
    	"HeightMapGen.LodLMResolution", "257"))
    	->SetAttribute ("name", "lightmap resolution");
    CreateNode (node_params, "materialmap")
    	->SetAttribute ("image", materialmap_input);
    CreateNode (node_params, "staticlighting",
    	cfgmgr->GetStr ("HeightMapGen.StaticLighting", "yes"));
    CreateNode (node_params, "castshadows",
    	cfgmgr->GetStr ("HeightMapGen.CastShadows", "yes"));

    csRef<iDocumentNode> node_move = CreateNode (node_obj, "move");
    csRef<iDocumentNode> node_v = CreateNode (node_move, "v");
    node_v->SetAttributeAsFloat ("x", (box.MaxX ()+box.MinX ()));
    node_v->SetAttributeAsFloat ("y", box.MinY ());
    node_v->SetAttributeAsFloat ("z", (box.MaxZ ()+box.MinZ ()));

#if 0
    csRef<iDocumentNode> node_light = CreateNode (node_sector, "light");
    node_light->SetAttribute ("name", "sun");
    csRef<iDocumentNode> node_center = CreateNode (node_light, "center");
    node_center->SetAttribute ("x", "0");
    node_center->SetAttribute ("y", "10000");
    node_center->SetAttribute ("z", "0");
    csRef<iDocumentNode> node_color = CreateNode (node_light, "color");
    node_color->SetAttribute ("red", "1");
    node_color->SetAttribute ("green", "1");
    node_color->SetAttribute ("blue", "1");
    CreateNode (node_light, "radius", "100000");
#endif
  }

  const char* err = doc->Write (VFS, world_name);
  if (err != 0) csPrintf ("%s\n", err);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  heightmapgen = new HeightMapGen (object_reg);

  if (heightmapgen->Initialize ())
    heightmapgen->Start ();

  delete heightmapgen;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}
