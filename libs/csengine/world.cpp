/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "qint.h"
#include "csutil/scf.h"
#include "csengine/sysitf.h"
#include "csengine/world.h"
#include "csengine/dumper.h"
#include "csengine/halo.h"
#include "csengine/camera.h"
#include "csengine/light.h"
#include "csengine/dynlight.h"
#include "csengine/polyplan.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/thingtpl.h"
#include "csengine/thing.h"
#include "csengine/cssprite.h"
#include "csengine/cscoll.h"
#include "csengine/sector.h"
#include "csengine/quadcube.h"
#include "csengine/texture.h"
#include "csengine/lghtmap.h"
#include "csengine/stats.h"
#include "csengine/cspmeter.h"
#include "csengine/cbuffer.h"
#include "csengine/lppool.h"
#include "csengine/covtree.h"
#include "csgeom/fastsqrt.h"
#include "csgeom/polypool.h"
#include "csinput/csevent.h"
#include "csutil/util.h"
#include "csutil/halogen.h"
#include "iimage.h"
#include "ivfs.h"
#include "ihalo.h"
#include "itxtmgr.h"
#include "igraph3d.h"

//---------------------------------------------------------------------------

csPolyIt::csPolyIt (csWorld* w)
{
  world = w;
  sector_idx = -1;
  thing = NULL;
  polygon_idx = 0;
}

void csPolyIt::Restart ()
{
  sector_idx = -1;
  thing = NULL;
  polygon_idx = 0;
}

csPolygon3D* csPolyIt::Fetch ()
{
  csSector* sector;
  if (sector_idx == -1)
  {
    sector_idx = 0;
    thing = NULL;
    polygon_idx = -1;
  }

  if (sector_idx >= world->sectors.Length ()) return NULL;
  sector = (csSector*)(world->sectors[sector_idx]);

  // Try next polygon.
  polygon_idx++;

  if (thing)
  {
    // We are busy scanning the things of this sector.
    // See if the current thing has the indicated polygon number.
    // If not then we try the next thing.
    while (thing && polygon_idx >= thing->GetNumPolygons ())
    {
      thing = (csThing*)(thing->GetNext ());
      polygon_idx = 0;
    }
    if (!thing)
    {
      // There are no more things left. Go to the next sector.
      sector_idx++;
      if (sector_idx >= world->sectors.Length ()) return NULL;
      // Initialize iterator to start of sector and recurse.
      thing = NULL;
      polygon_idx = -1;
      return Fetch ();
    }
  }
  else if (polygon_idx >= sector->GetNumPolygons ())
  {
    // We are not scanning things but we have no more polygons in
    // this sector. Start scanning things.
    polygon_idx = -1;
    thing = sector->GetFirstThing ();
    // Recurse.
    if (thing) return Fetch ();
    // No things. Go to next sector.
    sector_idx++;
    if (sector_idx >= world->sectors.Length ()) return NULL;
    // Initialize iterator to start of sector and recurse.
    thing = NULL;
    return Fetch ();
  }

  return thing ?
    thing->GetPolygon3D (polygon_idx) : sector->GetPolygon3D (polygon_idx);
}

//---------------------------------------------------------------------------

csLightIt::csLightIt (csWorld* w)
{
  world = w;
  sector_idx = -1;
  light_idx = 0;
}

void csLightIt::Restart ()
{
  sector_idx = -1;
  light_idx = 0;
}

csLight* csLightIt::Fetch ()
{
  csSector* sector;
  if (sector_idx == -1)
  {
    sector_idx = 0;
    light_idx = -1;
  }

  if (sector_idx >= world->sectors.Length ()) return NULL;
  sector = (csSector*)(world->sectors[sector_idx]);

  // Try next light.
  light_idx++;

  if (light_idx >= sector->lights.Length ())
  {
    // Go to next sector.
    light_idx = -1;
    sector_idx++;
    if (sector_idx >= world->sectors.Length ()) return NULL;
    // Initialize iterator to start of sector and recurse.
    return Fetch ();
  }

  csLight* light;
  light = (csLight*)(sector->lights[light_idx]);
  return light;
}

//---------------------------------------------------------------------------

int csWorld::frame_width;
int csWorld::frame_height;
iSystem* csWorld::System = NULL;
csWorld* csWorld::current_world = NULL;

IMPLEMENT_CSOBJTYPE (csWorld,csObject);

IMPLEMENT_IBASE (csWorld)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iWorld)
  IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csWorld)

EXPORT_CLASS_TABLE (engine)
  EXPORT_CLASS_DEP (csWorld, "crystalspace.engine.core",
    "Crystal Space 3D Engine", "crystalspace.kernel., crystalspace.graphics3d.")
EXPORT_CLASS_TABLE_END

csWorld::csWorld (iBase *iParent) : csObject (), start_vec (0, 0, 0)
{
  CONSTRUCT_IBASE (iParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  do_lighting_cache = true;
  first_dyn_lights = NULL;
  start_sector = NULL;
  System = NULL;
  VFS = NULL;
  G3D = NULL;
  textures = NULL;
  c_buffer = NULL;
  quadtree = NULL;
  quadcube = NULL;
  covtree = NULL;
  covtree_lut = NULL;
  current_camera = NULL;
  current_world = this;

  CHK (quadcube = new csQuadcube (8));
  CHK (textures = new csTextureList ());

  CHK (render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory()));
  CHK (lightpatch_pool = new csLightPatchPool ());

  BuildSqrtTable ();
}

// @@@ Hack
csCamera* camera_hack = NULL;

csWorld::~csWorld ()
{
  Clear ();
  if (G3D) G3D->DecRef ();
  if (VFS) VFS->DecRef ();
  CHK (delete textures);
  CHK (delete render_pol2d_pool);
  CHK (delete lightpatch_pool);
  CHK (delete quadcube);

  // @@@ temp hack
  CHK (delete camera_hack);
  camera_hack = NULL;
}

bool csWorld::Initialize (iSystem* sys)
{
  System = sys;

  if (!(G3D = QUERY_PLUGIN (sys, iGraphics3D)))
    return false;

  if (!(VFS = QUERY_PLUGIN (sys, iVFS)))
  {
    G3D->DecRef ();
    return false;
  }

  // Tell system driver that we want to handle broadcast events
  if (!System->CallOnEvents (this, CSMASK_Broadcast))
    return false;

  ReadConfig ();

  return true;
}

// Handle some system-driver broadcasts
bool csWorld::HandleEvent (csEvent &Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
      {
        frame_width = G3D->GetWidth ();
        frame_height = G3D->GetHeight ();
        if (csCamera::default_aspect == 0)
          csCamera::default_aspect = frame_height;
        csCamera::default_inv_aspect = 1./csCamera::default_aspect;

        // @@@ Ugly hack to always have a camera in current_camera.
        // This is needed for the lighting routines.
        if (!current_camera)
        {
          CHK (current_camera = new csCamera ());
          camera_hack = current_camera;
        }

        StartWorld ();

        return true;
      } /* endif */
    } /* endswitch */

  return false;
}

void csWorld::Clear ()
{
  halos.DeleteAll ();
  collections.DeleteAll ();
  sprites.DeleteAll ();
  sprite_templates.DeleteAll ();
  thing_templates.DeleteAll ();
  sectors.DeleteAll ();
  planes.DeleteAll ();
  CLights::DeleteAll ();

  while (first_dyn_lights)
  {
    csDynLight* dyn = first_dyn_lights->GetNext ();
    CHK (delete first_dyn_lights);
    first_dyn_lights = dyn;
  }
  CHK (delete [] start_sector); start_sector = NULL;
  CHK (delete textures); textures = NULL;
  CHK (textures = new csTextureList ());
  CHK (delete c_buffer); c_buffer = NULL;
  CHK (delete quadtree); quadtree = NULL;
  CHK (delete covtree); covtree = NULL;
  CHK (delete covtree_lut); covtree_lut = NULL;
  CHK (delete render_pol2d_pool);
  CHK (render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory()));
  CHK (delete lightpatch_pool);
  CHK (lightpatch_pool = new csLightPatchPool ());
}

void csWorld::EnableLightingCache (bool en)
{
  do_lighting_cache = en;
  if (!do_lighting_cache) csPolygon3D::do_force_recalc = true;
}

void csWorld::EnableCBuffer (bool en)
{
  if (en)
  {
    CHK (delete quadtree); quadtree = NULL;
    CHK (delete covtree); covtree = NULL;
    if (c_buffer) return;
    CHK (c_buffer = new csCBuffer (0, frame_width-1, frame_height));
  }
  else
  {
    CHK (delete c_buffer);
    c_buffer = NULL;
  }
}

void csWorld::EnableQuadtree (bool en)
{
  if (en)
  {
    CHK (delete c_buffer); c_buffer = NULL;
    CHK (delete covtree); covtree = NULL;
    if (quadtree) return;
    csBox box (0, 0, frame_width, frame_height);
    CHK (quadtree = new csQuadtree (box, 8));
  }
  else
  {
    CHK (delete quadtree);
    quadtree = NULL;
  }
}

void csWorld::EnableCovtree (bool en)
{
  if (en)
  {
    CHK (delete quadtree); quadtree = NULL;
    //@@@@@@@@@@@@CHK (delete c_buffer); c_buffer = NULL;
    if (covtree) return;
    csBox box (0, 0, frame_width, frame_height);
    if (!covtree_lut)
    {
      CHK (covtree_lut = new csCovMaskLUT (16));
    }
    CHK (covtree = new csCoverageMaskTree (covtree_lut, box));
  }
  else
  {
    CHK (delete covtree);
    covtree = NULL;
  }
}

csSpriteTemplate* csWorld::GetSpriteTemplate (const char* name)
{
  int sn = sprite_templates.Length ();
  while (sn > 0)
  {
    sn--;
    csSpriteTemplate* s = (csSpriteTemplate*)sprite_templates[sn];
    if (!strcmp (name, s->GetName ())) return s;
  }

  return NULL;
}

csThingTemplate* csWorld::GetThingTemplate (const char* name)
{
  int tn = thing_templates.Length ();
  while (tn > 0)
  {
    tn--;
    csThingTemplate* s = (csThingTemplate*)thing_templates[tn];
    if (!strcmp (name, s->GetName ())) return s;
  }
  return NULL;
}

csSector* csWorld::NewSector ()
{
  CHK (csSector* s = new csSector());
  s->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);
  sectors.Push (s);
  return s;
}

csThing* csWorld::GetThing (const char* name)
{
  int i = sectors.Length ();
  while (i > 0)
  {
    i--;
    csSector* s = (csSector*)sectors[i];
    csThing* t = s->GetThing (name);
    if (t) return t;
  }
  return NULL;
}

void csWorld::PrepareTextures ()
{
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  txtmgr->Initialize ();

  // First register all textures to the texture manager.
  for (int i = 0 ; i < textures->GetNumTextures () ; i++)
  {
    csTextureHandle* th = textures->GetTextureMM (i);
    iImageFile *image = th->GetImageFile ();

    // Now we check the size of the loaded image. Having an image, that
    // is not a power of two will result in strange errors while
    // rendering. It is by far better to check the format of all textures
    // already while loading them.
    if (th->for_3d)
    {
      int Width  = image->GetWidth ();
      int Height = image->GetHeight ();

      if (!IsPowerOf2(Width) || !IsPowerOf2(Height))
        CsPrintf (MSG_WARNING,
          "Inefficient texture image '%s' dimenstions!\n"
          "The width (%d) and height (%d) should be a power of two.\n",
          th->GetName (), Width, Height);
    }

    iTextureHandle* handle = txtmgr->RegisterTexture (th->GetImageFile (),
      th->for_3d, th->for_2d);
    th->SetTextureHandle (handle);
  }

  // Prepare all the textures.
  txtmgr->Prepare ();
}

void csWorld::PrepareSectors()
{
  // Now precalculate some stuff for all loaded polygons.
  for (int i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* s = (csSector*)sectors[i];
    s->Prepare ();
  }
}

bool csWorld::Prepare ()
{
  PrepareTextures ();
  PrepareSectors ();

  // The images are no longer needed by the 3D engine.
  iTextureManager *txtmgr = G3D->GetTextureManager ();
  txtmgr->FreeImages ();

  G3D->ClearCache ();

  // Prepare lightmaps if we have any sectors
  if (sectors.Length ())
    ShineLights ();

  CheckConsistency ();

  return true;
}

void csWorld::ShineLights ()
{
  tr_manager.NewFrame ();

  if (!csPolygon3D::do_not_force_recalc)
  {
    // If recalculation is not forced then we check if the 'precalc_info'
    // file exists on the VFS. If not then we will recalculate in any case.
    // If the file exists but is not valid (some flags are different) then
    // we recalculate again.
    // If we recalculate then we also update this 'precalc_info' file with
    // the new settings.
    struct PrecalcInfo
    {
      int lm_version;           // This number identifies a version of the lightmap format.
                                // If different then the format is different and we need
                                // to recalculate.
      int normal_light_level;   // Normal light level (unlighted level).
      int ambient_white;
      int ambient_red;
      int ambient_green;
      int ambient_blue;
      int reflect;
      int radiosity;
      int accurate_things;
      float cosinus_factor;
      int lightmap_size;
      int lightmap_highqual;
    };
    PrecalcInfo current;
    memset(&current, 0, sizeof(current)); //03/05/1999 Thomas Hieber: initialize current to something.
    current.lm_version = 1;
    current.normal_light_level = NORMAL_LIGHT_LEVEL;
    //@@@
    //current.ambient_white = Textures::ambient_white;
    //current.ambient_red = Textures::ambient_red;
    //current.ambient_green = Textures::ambient_green;
    //current.ambient_blue = Textures::ambient_blue;
    current.reflect = csSector::cfg_reflections;
    current.radiosity = (int)csSector::do_radiosity;
    current.accurate_things = csPolyTexture::do_accurate_things;
    current.cosinus_factor = csPolyTexture::cfg_cosinus_factor;
    current.lightmap_size = csPolygon3D::def_mipmap_size;
    current.lightmap_highqual = (int)csPolygon3D::do_lightmap_highqual;
    bool force = false;
    char* reason = NULL;

    size_t size;
    char *data = VFS->ReadFile ("precalc_info", size);
    if (data)
    {
      char* p1, * p2;
      int i;
      float f;
      p1 = data; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i);
      if (i != current.lm_version) { force = true; reason = "lightmap format changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.normal_light_level) { force = true; reason = "normal light level changed"; }
      //@@@
      //else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.ambient_white) { force = true; reason = "ambient white level changed"; }
      //else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.ambient_red) { force = true; reason = "ambient red level changed"; }
      //else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.ambient_green) { force = true; reason = "ambient green level changed"; }
      //else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.ambient_blue) { force = true; reason = "ambient blue level changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (false) { force = true; reason = "ambient white level changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (false) { force = true; reason = "ambient red level changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (false) { force = true; reason = "ambient green level changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (false) { force = true; reason = "ambient blue level changed"; }

      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.reflect) { force = true; reason = "reflection value changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.radiosity) { force = true; reason = "radiosity value changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.accurate_things) { force = true; reason = "'accurate things' flag changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%f", &f); if (ABS (f-current.cosinus_factor) > SMALL_EPSILON) { force = true; reason = "cosinus factor changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.lightmap_size) { force = true; reason = "lightmap size changed"; }
      else { p1 = p2+1; p2 = strchr (p1, '='); if (p2) sscanf (p2+1, "%d", &i); else i = -1; if (i != current.lightmap_highqual) { force = true; reason = "lightmap quality setting changed"; }
      }}}}}}}}}}}
      CHK (delete [] data);
    }
    else
    {
      force = true;
      reason = "no 'precalc_info' found";
    }

    if (force)
    {
      char data [1000];
      sprintf (data,
        "LMVERSION=%d\n"        "NORMALLIGHTLEVEL=%d\n"
        "AMBIENT_WHITE=%d\n"    "AMBIENT_RED=%d\n"
        "AMBIENT_GREEN=%d\n"    "AMBIENT_BLUE=%d\n"
        "REFLECT=%d\n"          "RADIOSITY=%d\n"
        "ACCURATE_THINGS=%d\n"  "COSINUS_FACTOR=%f\n"
        "LIGHTMAP_SIZE=%d\n"    "LIGHTMAP_HIGHQUAL=%d\n",
        current.lm_version, current.normal_light_level, current.ambient_white, current.ambient_red, current.ambient_green,
        current.ambient_blue, current.reflect, current.radiosity, current.accurate_things, current.cosinus_factor,
        current.lightmap_size, current.lightmap_highqual);
      VFS->WriteFile ("precalc_info", data, strlen (data));
      CsPrintf (MSG_INITIALIZATION, "Lightmap data is not up to date (reason: %s).\n", reason);
      csPolygon3D::do_force_recalc = true;
    }
  }

  if (csPolygon3D::do_force_recalc)
  {
    CsPrintf (MSG_INITIALIZATION, "Recalculation of lightmaps forced.\n");
    CsPrintf (MSG_INITIALIZATION, "  Pseudo-radiosity system %s.\n", csSector::do_radiosity ? "enabled" : "disabled");
    CsPrintf (MSG_INITIALIZATION, "  Maximum number of visits per sector = %d.\n", csSector::cfg_reflections);
  }
  else
  {
    // If no recalculation is forced we set these variables to default to
    // make sure that we don't do too many unneeded calculations.
    csSector::do_radiosity = false;
    csSector::cfg_reflections = 1;
  }

  csPolyIt* pit = NewPolyIterator ();
  csLightIt* lit = NewLightIterator ();

  // Set lumel size for 'High Quality Mode'
  // and reinit all lightmaps.
  // This loop also counts all polygons.
  csPolygon3D* p;
  int polygon_count = 0;
  if (csPolygon3D::do_lightmap_highqual && csPolygon3D::do_force_recalc)
    csPolygon3D::def_mipmap_size /= 2;
  pit->Restart ();
  while ((p = pit->Fetch ()) != NULL)
  {
    if (csPolygon3D::do_lightmap_highqual && csPolygon3D::do_force_recalc)
      p->UpdateLightMapSize ();
    polygon_count++;
  }

  // Count number of lights to process.
  csLight* l;
  int light_count = 0;
  lit->Restart ();
  while (lit->Fetch ()) light_count++;

  int sn = 0;
  int num_sectors = sectors.Length ();
  csProgressMeter meter;
  CsPrintf (MSG_INITIALIZATION, "Initializing lightmaps (%d sectors):\n  ", num_sectors);

  meter.SetTotal (num_sectors);
  for (sn = 0; sn < num_sectors ; sn++)
  {
    csSector* s = (csSector*)sectors [sn];
    s->InitLightMaps ();
    meter.Step();
  }

  time_t start, stop;
  start = System->GetTime ();
  meter.SetTotal (light_count);
  CsPrintf (MSG_INITIALIZATION, "\nShining lights (%d lights):\n  ", light_count);
  lit->Restart ();
  while ((l = lit->Fetch ()) != NULL)
  {
    ((csStatLight*)l)->CalculateLighting ();
    meter.Step();
  }
  stop = System->GetTime ();
  CsPrintf (MSG_INITIALIZATION, "\n(%f seconds)", (float)(stop-start)/1000.);

  // Restore lumel size from 'High Quality Mode'
  // and remap all lightmaps.
  if (csPolygon3D::do_lightmap_highqual && csPolygon3D::do_force_recalc)
  {
    CsPrintf (MSG_INITIALIZATION, "\nScaling lightmaps (%d maps):\n  ", polygon_count);
    csPolygon3D::def_mipmap_size *= 2;
    meter.SetTotal (polygon_count);
    pit->Restart ();
    while ((p = pit->Fetch ()) != NULL)
    {
      p->ScaleLightMaps ();
      meter.Step();
    }
  }

  meter.SetTotal (num_sectors);
  CsPrintf (MSG_INITIALIZATION, "\nCaching lightmaps (%d sectors):\n  ", num_sectors);
  for (sn = 0; sn < num_sectors ; sn++)
  {
    csSector* s = (csSector*)sectors[sn];
    s->CacheLightMaps ();
    meter.Step();
  }

  CsPrintf (MSG_INITIALIZATION, "\nPreparing lightmaps (%d maps):\n  ", polygon_count);
  meter.SetTotal (polygon_count);
  pit->Restart ();
  while ((p = pit->Fetch ()) != NULL)
  {
    p->CreateLightMaps (G3D);
    meter.Step();
  }

  csPolygonSet::current_light_frame_number++;

  CsPrintf (MSG_INITIALIZATION, "\nUpdating VFS...\n");
  if (!VFS->Sync ())
    CsPrintf (MSG_WARNING, "WARNING: error updating lighttable cache!\n");
  CsPrintf (MSG_INITIALIZATION, "DONE!\n");

  CHK (delete pit);
  CHK (delete lit);
}

bool csWorld::CheckConsistency ()
{
  csPolyIt* pit = NewPolyIterator ();
  bool error = false;

  csPolygon3D* p;

  CsPrintf (MSG_INITIALIZATION, "Validating world...\n");
  pit->Restart ();
  while ((p = pit->Fetch ()) != NULL)
  {
    if (p->GetNumVertices () < 3)
    {
      CsPrintf (MSG_WARNING, "  Polygon with only %d vertices! (id=%d)\n", p->GetNumVertices (), p->GetID ());
      CsPrintf (MSG_DEBUG_0, "============ Polygon with only %d vertices (id=%d)!\n", p->GetNumVertices (), p->GetID ());
      Dumper::dump (p);
      error = true;
    }
    else if (p->GetNumVertices () > 3)
    {
      csVector3 normal;
      float D;
      csMath3::CalcPlane (p->Vobj (0), p->Vobj (1), p->Vobj (2), normal, D);
      csPlane pl (normal, D);
      int i;
      for (i = 3 ; i < p->GetNumVertices () ; i++)
      {
        if (ABS (pl.Classify (p->Vobj (i))) > EPSILON)
	{
          CsPrintf (MSG_WARNING, "  Non-coplanar polygon! (id=%d)\n", p->GetID ());
          CsPrintf (MSG_DEBUG_0, "============ Non-coplanar polygon (id=%d)!\n", p->GetID ());
          Dumper::dump (p);
          error = true;
	  break;
	}
      }
    }
  }

  CHK (delete pit);
  CsPrintf (MSG_INITIALIZATION, "DONE\n");
  return error;
}

void csWorld::StartWorld ()
{
  Clear ();
}

void csWorld::Draw (csCamera* c, csClipper* view)
{
  Stats::polygons_considered = 0;
  Stats::polygons_drawn = 0;
  Stats::portals_drawn = 0;
  Stats::polygons_rejected = 0;
  Stats::polygons_accepted = 0;

  current_camera = c;
  top_clipper = view;

  iGraphics2D *G2D = G3D->GetDriver2D ();
  csRenderView rview (*c, view, G3D, G2D);
  rview.clip_plane.Set (0, 0, 1, -1);   //@@@CHECK!!!
  rview.callback = NULL;

  tr_manager.NewFrame ();

//@@@@@@
extern bool stop_processing;
stop_processing = false;

  if (c_buffer)
  {
    c_buffer->Initialize ();
    csVector2 verts[50];	// @@@ BAD! Hardcoded!
    int i, num;
    num = view->GetNumVertices ();
    for (i = 0 ; i < num ; i++) verts[i] = view->GetVertex (i);
    c_buffer->InsertPolygon (verts, num, true);
  }
  if (quadtree)
    quadtree->MakeEmpty ();
  if (covtree)
  {
  //@@@@@@@@@@@@
  covtree->MakeInvalid ();
    covtree->MakeEmpty ();
  }

  csSector* s = c->GetSector ();
  s->Draw (rview);

  // draw all halos on the screen
  for (int halo = halos.Length () - 1; halo >= 0; halo--)
    if (!ProcessHalo (halos.Get (halo)))
      halos.Delete (halo);
}

void csWorld::DrawFunc (csCamera* c, csClipper* view,
	csDrawFunc* callback, void* callback_data)
{
  iGraphics2D* G2D = G3D->GetDriver2D ();
  csRenderView rview (*c, view, G3D, G2D);
  rview.clip_plane.Set (0, 0, 1, -1);   //@@@CHECK!!!
  rview.callback = callback;
  rview.callback_data = callback_data;

  tr_manager.NewFrame ();

  if (c_buffer) c_buffer->Initialize ();
  if (quadtree) quadtree->MakeEmpty ();
  if (covtree) covtree->MakeEmpty ();

  csSector* s = c->GetSector ();
  s->Draw (rview);
}

void csWorld::AddHalo (csLight* Light)
{
  if (!Light->CheckFlags (CS_LIGHT_HALO)
   || Light->GetHaloInQueue ())
    return;

  // Transform light pos into camera space and see if it is directly visible
  csVector3 v = current_camera->World2Camera (Light->GetCenter ());

  // Check if light is behind us
  if (v.z <= SMALL_Z)
    return;

  // Project X,Y into screen plane
  float iz = current_camera->aspect / v.z;
  v.x = v.x * iz + current_camera->shift_x;
  v.y = frame_height - 1 - (v.y * iz + current_camera->shift_y);

  // If halo is not inside visible region, return
  if (!top_clipper->IsInside (v.x, v.y))
    return;

  // Check if light is not obscured by anything
  float zv = G3D->GetZbuffValue (QRound (v.x), QRound (v.y));
  if (v.z > zv)
    return;

  // Halo size is 1/4 of the screen height; also we make sure its odd
  int hs = (frame_height / 4) | 1;
  float hi, hc;
  Light->GetHaloType (hi, hc);
  unsigned char *Alpha = GenerateHalo (hs, hi, hc);

  // Okay, put the light into the queue: first we generate the alphamap
  iHalo *handle = G3D->CreateHalo (Light->GetColor ().red,
    Light->GetColor ().green, Light->GetColor ().blue, Alpha, hs, hs);

  // We don't need alpha map anymore
  delete [] Alpha;

  // Does 3D rasterizer support halos?
  if (!handle)
    return;

  halos.Push (new csLightHalo (Light, handle));
}

bool csWorld::HasHalo (csLight* Light)
{
  return halos.FindKey (Light) >= 0;
}

#define HALO_INTENSITY_STEP	0.15f

bool csWorld::ProcessHalo (csLightHalo *Halo)
{
  // Whenever the center of halo (the light) is directly visible
  bool halo_vis = false;
  // Whenever at least a piece of halo is visible
  bool draw_halo = false;
  // top-left coordinates of halo rectangle
  float xtl = 0, ytl = 0;

  // Project the halo.
  csVector3 v = current_camera->World2Camera (Halo->Light->GetCenter ());
  // The clipped halo polygon
  csVector2 HaloClip [32];
  // Number of vertices in HaloClip array
  int HaloVCount = 32;

  if (v.z > SMALL_Z)
  {
    float iz = current_camera->aspect / v.z;
    v.x = v.x * iz + current_camera->shift_x;
    v.y = frame_height - 1 - (v.y * iz + current_camera->shift_y);

    if (top_clipper->IsInside (v.x, v.y))
    {
      float zv = G3D->GetZbuffValue (QRound (v.x), QRound (v.y));
      halo_vis = (v.z <= zv);
    }

    // Create a rectangle containing the halo and clip it against screen
    int hw = Halo->Handle->GetWidth ();
    int hh = Halo->Handle->GetHeight ();
    float hw2 = float (hw) / 2.0;
    float hh2 = float (hh) / 2.0;
    csVector2 HaloPoly [4] =
    {
      csVector2 (v.x - hw2, v.y - hh2),
      csVector2 (v.x - hw2, v.y + hh2),
      csVector2 (v.x + hw2, v.y + hh2),
      csVector2 (v.x + hw2, v.y - hh2)
    };
    // Clip the halo against clipper
    if (top_clipper->Clip (HaloPoly, HaloClip, 4, HaloVCount))
    {
      xtl = HaloPoly [0].x;
      ytl = HaloPoly [0].y;
      draw_halo = true;
    }
  }

  float hintensity = Halo->Light->GetHaloIntensity ();
  if (halo_vis)
  {
    float maxintensity = Halo->Light->GetHaloMaxIntensity ();
    if (hintensity < maxintensity - HALO_INTENSITY_STEP)
      hintensity += HALO_INTENSITY_STEP;
    else
      hintensity = maxintensity;
  }
  else
  {
    hintensity -= HALO_INTENSITY_STEP;

    // this halo is completely invisible. kill it.
    if (hintensity <= 0)
      return false;
  }
  Halo->Light->SetHaloIntensity (hintensity);

  if (draw_halo)
    Halo->Handle->Draw (xtl, ytl, -1, -1, hintensity, HaloClip, HaloVCount);
  return true;
}

csStatLight* csWorld::FindLight (float x, float y, float z, float dist)
{
  csStatLight* l;
  int sn = sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)sectors[sn];
    l = s->FindLight (x, y, z, dist);
    if (l) return l;
  }
  return NULL;
}

void csWorld::AddDynLight (csDynLight* dyn)
{
  dyn->SetNext (first_dyn_lights);
  dyn->SetPrev (NULL);
  if (first_dyn_lights) first_dyn_lights->SetPrev (dyn);
  first_dyn_lights = dyn;
}

void csWorld::RemoveDynLight (csDynLight* dyn)
{
  if (dyn->GetNext ()) dyn->GetNext ()->SetPrev (dyn->GetPrev ());
  if (dyn->GetPrev ()) dyn->GetPrev ()->SetNext (dyn->GetNext ());
  else if (dyn == first_dyn_lights) first_dyn_lights = dyn->GetNext ();
  dyn->SetNext (NULL);
  dyn->SetPrev (NULL);
}

void csWorld::AdvanceSpriteFrames (time_t current_time)
{
  int i;
  for (i = 0 ; i < sprites.Length () ; i++)
  {
    csSprite3D* s = (csSprite3D*)sprites[i];
    s->NextFrame (current_time);
  }
}

void csWorld::ReadConfig ()
{
  if (!System) return;
  csPolygon3D::def_mipmap_size = System->ConfigGetInt ("Lighting", "LIGHTMAP_SIZE", 16);
  csPolygon3D::do_lightmap_highqual = System->ConfigGetYesNo ("Lighting", "LIGHTMAP_HIGHQUAL", true);
  csLight::ambient_red = System->ConfigGetInt ("World", "AMBIENT_RED", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_green = System->ConfigGetInt ("World", "AMBIENT_GREEN", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_blue = System->ConfigGetInt ("World", "AMBIENT_BLUE", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_white = System->ConfigGetInt ("World", "AMBIENT_WHITE", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_red += csLight::ambient_white;
  csLight::ambient_green += csLight::ambient_white;
  csLight::ambient_blue += csLight::ambient_white;
  csSector::cfg_reflections = System->ConfigGetInt ("Lighting", "REFLECT", csSector::cfg_reflections);
  csSector::do_radiosity = System->ConfigGetYesNo ("Lighting", "RADIOSITY", csSector::do_radiosity);
  csPolyTexture::do_accurate_things = System->ConfigGetYesNo ("Lighting", "ACCURATE_THINGS", csPolyTexture::do_accurate_things);
  csPolyTexture::cfg_cosinus_factor = System->ConfigGetFloat ("Lighting", "COSINUS_FACTOR", csPolyTexture::cfg_cosinus_factor);
  csSprite3D::do_quality_lighting = System->ConfigGetYesNo ("Lighting", "SPRITE_HIGHQUAL", csSprite3D::do_quality_lighting);
  //@@@
  //Textures::Gamma = System->ConfigGetFloat ("TextureMapper", "GAMMA", 1.0);
}

void csWorld::UnlinkSprite (csSprite3D* sprite)
{
  sprite->RemoveFromSectors ();
  int idx = sprites.Find (sprite);
  if (idx == -1) return;
  sprites[idx] = NULL;
  sprites.Delete (idx);
}

void csWorld::RemoveSprite (csSprite3D* sprite)
{
  sprite->RemoveFromSectors ();
  int idx = sprites.Find (sprite);
  if (idx == -1) return;
  sprites.Delete (idx);
}

struct LightAndDist
{
  csLight* light;
  float sqdist;
};

// csLightArray is a subclass of csCleanable which is registered
// to csWorld.cleanup.
class csLightArray : public csBase
{
public:
  LightAndDist* array;
  // Size is the physical size of the array. num_lights is the number of lights in it.
  int size, num_lights;

  csLightArray () : array (NULL), size (0), num_lights (0) { }
  virtual ~csLightArray () { CHK (delete [] array); }
  void Reset () { num_lights = 0; }
  void AddLight (csLight* l, float sqdist)
  {
    if (num_lights >= size)
    {
      LightAndDist* new_array;
      CHK (new_array = new LightAndDist [size+5]);
      if (array)
      {
        memcpy (new_array, array, sizeof (LightAndDist)*num_lights);
        CHK (delete [] array);
      }
      array = new_array;
      size += 5;
    }
    array[num_lights].light = l;
    array[num_lights++].sqdist = sqdist;
  };
  csLight* GetLight (int i) { return array[i].light; }
};

int compare_light (const void* p1, const void* p2)
{
  LightAndDist* sp1 = (LightAndDist*)p1;
  LightAndDist* sp2 = (LightAndDist*)p2;
  float z1 = sp1->sqdist;
  float z2 = sp2->sqdist;
  if (z1 < z2) return -1;
  else if (z1 > z2) return 1;
  return 0;
}

int csWorld::GetNearbyLights (csSector* sector, const csVector3& pos, ULong flags,
  	csLight** lights, int max_num_lights)
{
  int i;
  float sqdist;

  // This is a static light array which is adapted to the
  // right size everytime it is used. In the beginning it means
  // that this array will grow a lot but finally it will
  // stabilize to a maximum size (not big). The advantage of
  // this approach is that we don't have a static array which can
  // overflow. And we don't have to do allocation every time we
  // come here. We register this memory to the 'cleanup' array
  // in csWorld so that it will be freed later.

  static csLightArray* light_array = NULL;
  if (!light_array)
  {
    CHK (light_array = new csLightArray ());
    csWorld::current_world->cleanup.Push (light_array);
  }
  light_array->Reset ();

  // Add all dynamic lights to the array (if CS_NLIGHT_DYNAMIC is set).
  if (flags & CS_NLIGHT_DYNAMIC)
  {
    csDynLight* dl = first_dyn_lights;
    while (dl)
    {
      if (dl->GetSector () == sector)
      {
        sqdist = csSquaredDist::PointPoint (pos, dl->GetCenter ());
        if (sqdist < dl->GetSquaredRadius ()) light_array->AddLight (dl, sqdist);
      }
      dl = dl->GetNext ();
    }
  }

  // Add all static lights to the array (if CS_NLIGHT_STATIC is set).
  if (flags & CS_NLIGHT_STATIC)
  {
    for (i = 0 ; i < sector->lights.Length () ; i++)
    {
      csStatLight* sl = (csStatLight*)sector->lights[i];
      sqdist = csSquaredDist::PointPoint (pos, sl->GetCenter ());
      if (sqdist < sl->GetSquaredRadius ()) light_array->AddLight (sl, sqdist);
    }
  }

  if (light_array->num_lights <= max_num_lights)
  {
    // The number of lights that we found is smaller than what fits
    // in the array given us by the user. So we just copy them all
    // and don't need to sort.
    for (i = 0 ; i < light_array->num_lights ; i++)
      lights[i] = light_array->GetLight (i);
    return light_array->num_lights;
  }
  else
  {
    // We found more lights than we can put in the given array
    // so we sort the lights and then return the nearest.
    qsort (light_array->array, light_array->num_lights, sizeof (LightAndDist), compare_light);
    for (i = 0 ; i < max_num_lights; i++)
      lights[i] = light_array->GetLight (i);
    return max_num_lights;
  }
}
