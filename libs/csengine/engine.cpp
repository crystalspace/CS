/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "qint.h"
#include "csutil/scf.h"
#include "csengine/engine.h"
#include "csengine/dumper.h"
#include "csengine/halo.h"
#include "csengine/camera.h"
#include "csengine/campos.h"
#include "csengine/keyval.h"
#include "csengine/light.h"
#include "csengine/polyplan.h"
#include "csengine/polytmap.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/thing.h"
#include "csengine/csview.h"
#include "csengine/cssprite.h"
#include "csengine/meshobj.h"
#include "csengine/cscoll.h"
#include "csengine/sector.h"
#include "csengine/covcube.h"
#include "csengine/cbufcube.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/lghtmap.h"
#include "csengine/stats.h"
#include "csengine/cspmeter.h"
#include "csengine/cbuffer.h"
#include "csengine/quadtr3d.h"
#include "csengine/lppool.h"
#include "csengine/covtree.h"
#include "csengine/particle.h"
#include "csengine/radiosty.h"
#include "csengine/region.h"
#include "csgeom/fastsqrt.h"
#include "csgeom/polypool.h"
#include "csgfxldr/csimage.h"
#include "csutil/util.h"
#include "iimage.h"
#include "ivfs.h"
#include "ihalo.h"
#include "itxtmgr.h"
#include "igraph3d.h"
#include "ievent.h"
#include "icfgfile.h"
#include "itranman.h"

//---------------------------------------------------------------------------

csPolyIt::csPolyIt (csEngine* e, csRegion* r) : engine (e), region (r)
{
  Restart ();
}

void csPolyIt::Restart ()
{
  sector_idx = -1;
  thing_idx = -1;
  polygon_idx = 0;
}

bool csPolyIt::NextSector ()
{
  sector_idx++;
  if (region)
    while (sector_idx < engine->sectors.Length ()
	  && !region->IsInRegion (GetLastSector ()))
      sector_idx++;
  if (sector_idx >= engine->sectors.Length ()) return false;
  return true;
}

csPolygon3D* csPolyIt::Fetch ()
{
  csSector* sector;
  if (sector_idx == -1)
  {
    if (!NextSector ()) return NULL;
    thing_idx = -1;
    polygon_idx = -1;
  }

  if (sector_idx >= engine->sectors.Length ()) return NULL;
  sector = (csSector*)(engine->sectors[sector_idx]);

  // Try next polygon.
  polygon_idx++;

  if (thing_idx != -1)
  {
    // We are busy scanning the things of this sector.
    // See if the current thing has the indicated polygon number.
    // If not then we try the next thing.
    while (polygon_idx >= ((csThing*)(sector->things[thing_idx]))->GetNumPolygons ())
    {
      thing_idx++;
      if (thing_idx >= sector->things.Length ())
      {
        thing_idx = -1;
	break;
      }
      polygon_idx = 0;
    }
    if (thing_idx == -1)
    {
      // There are no more things left. Go to the next sector.
      if (!NextSector ()) return NULL;
      // Initialize iterator to start of sector and recurse.
      thing_idx = -1;
      polygon_idx = -1;
      return Fetch ();
    }
  }
  else if (polygon_idx >= sector->GetNumPolygons ())
  {
    // We are not scanning things but we have no more polygons in
    // this sector. Start scanning things.
    polygon_idx = -1;
    thing_idx = 0;
    // Recurse.
    if (thing_idx < sector->things.Length ())
      return Fetch ();
    // No things. Go to next sector.
    if (!NextSector ()) return NULL;
    // Initialize iterator to start of sector and recurse.
    thing_idx = -1;
    return Fetch ();
  }

  return thing_idx != -1 ?
    ((csThing*)(sector->things[thing_idx]))->GetPolygon3D (polygon_idx) :
    sector->GetPolygon3D (polygon_idx);
}

csSector* csPolyIt::GetLastSector ()
{
  return (csSector*)(engine->sectors[sector_idx]);
}

//--------------------- csCurveIt -------------------------------------------

csCurveIt::csCurveIt (csEngine* e, csRegion* r) : engine (e), region (r)
{
  Restart ();
}

bool csCurveIt::NextSector ()
{
  sector_idx++;
  if (region)
    while (sector_idx < engine->sectors.Length ()
	  && !region->IsInRegion (GetLastSector ()))
      sector_idx++;
  if (sector_idx >= engine->sectors.Length ()) return false;
  return true;
}

void csCurveIt::Restart ()
{
  sector_idx = -1;
  thing_idx = -1;
  curve_idx = 0;
}

csCurve* csCurveIt::Fetch ()
{
  csSector* sector;
  if (sector_idx == -1)
  {
    if (!NextSector ()) return NULL;
    thing_idx = -1;
    curve_idx = -1;
  }

  if (sector_idx >= engine->sectors.Length ()) return NULL;
  sector = (csSector*)(engine->sectors[sector_idx]);

  // Try next polygon.
  curve_idx++;

  if (thing_idx != -1)
  {
    // We are busy scanning the things of this sector.
    // See if the current thing has the indicated curve number.
    // If not then we try the next thing.
    while (curve_idx >= ((csThing*)(sector->things[thing_idx]))->GetNumCurves ())
    {
      thing_idx++;
      if (thing_idx >= sector->things.Length ())
      {
        thing_idx = -1;
	break;
      }
      curve_idx = 0;
    }
    if (thing_idx == -1)
    {
      // There are no more things left. Go to the next sector.
      if (!NextSector ()) return NULL;
      // Initialize iterator to start of sector and recurse.
      thing_idx = -1;
      curve_idx = -1;
      return Fetch ();
    }
  }
  else if (curve_idx >= sector->GetNumCurves ())
  {
    // We are not scanning things but we have no more polygons in
    // this sector. Start scanning things.
    curve_idx = -1;
    thing_idx = 0;

    // Recurse.
    if (thing_idx < sector->things.Length ())
      return Fetch ();

    // No things. Go to next sector.
    if (!NextSector ()) return NULL;

    // Initialize iterator to start of sector and recurse.
    thing_idx = -1;
    return Fetch ();
  }

  return thing_idx != -1 ?
    ((csThing*)(sector->things[thing_idx]))->GetCurve (curve_idx) :
    sector->GetCurve (curve_idx);
}

csSector* csCurveIt::GetLastSector ()
{
  return (csSector*)(engine->sectors[sector_idx]);
}

//---------------------------------------------------------------------------

csLightIt::csLightIt (csEngine* e, csRegion* r) : engine (e), region (r)
{
  Restart ();
}

bool csLightIt::NextSector ()
{
  sector_idx++;
  if (region)
    while (sector_idx < engine->sectors.Length ()
	  && !region->IsInRegion (GetLastSector ()))
      sector_idx++;
  if (sector_idx >= engine->sectors.Length ()) return false;
  return true;
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
    if (!NextSector ()) return NULL;
    light_idx = -1;
  }

  if (sector_idx >= engine->sectors.Length ()) return NULL;
  sector = (csSector*)(engine->sectors[sector_idx]);

  // Try next light.
  light_idx++;

  if (light_idx >= sector->lights.Length ())
  {
    // Go to next sector.
    light_idx = -1;
    if (!NextSector ()) return NULL;
    // Initialize iterator to start of sector and recurse.
    return Fetch ();
  }

  csLight* light;
  light = (csLight*)(sector->lights[light_idx]);
  return light;
}

csSector* csLightIt::GetLastSector ()
{
  return (csSector*)(engine->sectors[sector_idx]);
}

//---------------------------------------------------------------------------

csSectorIt::csSectorIt (csSector* sector, const csVector3& pos, float radius)
{
  csSectorIt::sector = sector;
  csSectorIt::pos = pos;
  csSectorIt::radius = radius;
  recursive_it = NULL;

  Restart ();
}

csSectorIt::~csSectorIt ()
{
  delete recursive_it;
}

void csSectorIt::Restart ()
{
  cur_poly = -1;
  delete recursive_it;
  recursive_it = NULL;
  has_ended = false;
}

csSector* csSectorIt::Fetch ()
{
  if (has_ended) return NULL;

  if (recursive_it)
  {
    csSector* rc = recursive_it->Fetch ();
    if (rc)
    {
      last_pos = recursive_it->GetLastPosition ();
      return rc;
    }
    delete recursive_it;
    recursive_it = NULL;
  }

  if (cur_poly == -1)
  {
    cur_poly = 0;
    last_pos = pos;
    return sector;
  }

  // @@@ This function should try to use the octree if available to
  // quickly discard lots of polygons that cannot be close enough.
  while (cur_poly < sector->GetNumPolygons ())
  {
    csPolygon3D* p = sector->GetPolygon3D (cur_poly);
    cur_poly++;
    csPortal* po = p->GetPortal ();
    if (po)
    {
      const csPlane3& wpl = p->GetPlane ()->GetWorldPlane ();
      float d = wpl.Distance (pos);
      if (d < radius && csMath3::Visible (pos, wpl))
      {
        if (!po->GetSector ()) po->CompleteSector ();
	csVector3 new_pos;
	if (po->flags.Check (CS_PORTAL_WARP))
	  new_pos = po->Warp (pos);
	else
	  new_pos = pos;
	recursive_it = new csSectorIt (po->GetSector (), new_pos, radius);
	return Fetch ();
      }
    }
  }

  // End search.
  has_ended = true;
  return NULL;
}

//---------------------------------------------------------------------------

csObjectIt::csObjectIt (csEngine* e, const csIdType& type, bool derived,
  	csSector* sector, const csVector3& pos, float radius)
{
  csObjectIt::engine = e;
  csObjectIt::type = &type;
  csObjectIt::derived = derived;
  csObjectIt::start_sector = sector;
  csObjectIt::start_pos = pos;
  csObjectIt::radius = radius;
  sectors_it = new csSectorIt (sector, pos, radius);

  Restart ();
}

csObjectIt::~csObjectIt ()
{
  delete sectors_it;
}

bool csObjectIt::CheckType (csObject* obj)
{
  if (derived) return obj->GetType () >= *type;
  else return &obj->GetType () == type;
}

bool csObjectIt::CheckType (const csIdType* ctype)
{
  if (derived) return *ctype >= *type;
  else return ctype == type;
}

void csObjectIt::Restart ()
{
  cur_type = &csDynLight::Type;
  cur_object = engine->GetFirstDynLight ();
  sectors_it->Restart ();
}

void csObjectIt::StartThings ()
{
  cur_type = &csThing::Type;
  cur_idx = 0;
}

void csObjectIt::StartStatLights ()
{
  cur_type = &csStatLight::Type;
  cur_idx = 0;
}

void csObjectIt::StartSprites ()
{
  cur_type = &csSprite::Type;
  cur_idx = 0;
}

void csObjectIt::EndSearch ()
{
  cur_type = NULL;
}

csObject* csObjectIt::Fetch ()
{
  if (!cur_type) return NULL;

  // Handle csDynLight.
  if (cur_type == &csDynLight::Type)
  {
    if (CheckType (cur_type))
    {
      if (cur_object == NULL)
        cur_type = &csSector::Type;
      else
      {
        do
	{
          csObject* rc = cur_object;
          cur_object = ((csDynLight*)cur_object)->GetNext ();
	  float r = ((csLight*)rc)->GetRadius () + radius;
	  if (csSquaredDist::PointPoint (((csLight*)rc)->GetCenter (),
		  cur_pos) <= r*r)
            return rc;
	}
	while (cur_object);
	if (cur_object == NULL)
          cur_type = &csSector::Type;
      }
    }
    else
      cur_type = &csSector::Type;
  }
  // Handle this sector first.
  if (cur_type == &csSector::Type)
  {
    cur_sector = sectors_it->Fetch ();
    if (!cur_sector)
    {
      EndSearch ();
      return NULL;
    }
    cur_pos = sectors_it->GetLastPosition ();

    if (CheckType (cur_type))
    {
      StartThings ();
      return cur_sector;
    }
    else
      StartThings ();
  }
  // Handle csThing.
  if (cur_type == &csThing::Type)
  {
    if (CheckType (cur_type))
    {
      if (cur_idx >= cur_sector->things.Length ())
        StartStatLights ();
      else
      {
        csObject* rc = (csObject*)cur_sector->things[cur_idx];
	cur_idx++;
        return rc;
      }
    }
    else
      StartStatLights ();
  }
  // Handle csLight.
  if (cur_type == &csStatLight::Type)
  {
    if (CheckType (cur_type))
    {
      if (cur_idx >= cur_sector->lights.Length ())
        StartSprites ();
      else
      {
        do
	{
          csObject* rc = (csObject*)cur_sector->lights[cur_idx];
	  cur_idx++;
	  float r = ((csLight*)rc)->GetRadius () + radius;
	  if (csSquaredDist::PointPoint (((csLight*)rc)->GetCenter (),
		  cur_pos) <= r*r)
            return rc;
	}
	while (cur_idx < cur_sector->lights.Length ());
        if (cur_idx >= cur_sector->lights.Length ())
          StartSprites ();
      }
    }
    else
      StartSprites ();
  }
  // Handle csSprite.
  if (cur_type == &csSprite::Type)
  {
    if (CheckType (cur_type))
    {
      if (cur_idx >= cur_sector->sprites.Length ())
        cur_type = &csSector::Type;
      else
      {
        csObject* rc = (csObject*)cur_sector->sprites[cur_idx];
	cur_idx++;
        return rc;
      }
    }
    else
      cur_type = &csSector::Type;
  }
  return NULL;
}

//---------------------------------------------------------------------------

int csEngine::frame_width;
int csEngine::frame_height;
iSystem* csEngine::System = NULL;
csEngine* csEngine::current_engine = NULL;
bool csEngine::use_new_radiosity = false;
int csEngine::max_process_polygons = 2000000000;
int csEngine::cur_process_polygons = 0;
bool csEngine::do_lighting_cache = true;
bool csEngine::do_not_force_relight = false;
bool csEngine::do_force_relight = false;
bool csEngine::do_not_force_revis = false;
bool csEngine::do_force_revis = false;
bool csEngine::do_rad_debug = false;

IMPLEMENT_CSOBJTYPE (csEngine,csObject);

IMPLEMENT_IBASE (csEngine)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iEngine)
  IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csEngine)

EXPORT_CLASS_TABLE (engine)
  EXPORT_CLASS_DEP (csEngine, "crystalspace.engine.core",
    "Crystal Space 3D Engine", "crystalspace.kernel., crystalspace.graphics3d.")
EXPORT_CLASS_TABLE_END

csEngine::csEngine (iBase *iParent) : csObject (), camera_positions (16, 16)
{
  CONSTRUCT_IBASE (iParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  engine_mode = CS_ENGINE_AUTODETECT;
  first_dyn_lights = NULL;
  System = NULL;
  VFS = NULL;
  G3D = NULL;
  G2D = NULL;
  textures = NULL;
  materials = NULL;
  c_buffer = NULL;
  quad3d = NULL;
  covcube = NULL;
  cbufcube = NULL;
  covtree = NULL;
  covtree_lut = NULL;
  current_camera = NULL;
  current_engine = this;
  use_pvs = false;
  use_pvs_only = false;
  freeze_pvs = false;
  library = NULL;
  engine_states = NULL;
  rad_debug = NULL;

  if (!covtree_lut)
  {
    covtree_lut = new csCovMaskLUT (16);
  }
  //covcube = new csCovcube (covtree_lut);
  cbufcube = new csCBufferCube (1024);

  SetCuller (CS_CULLER_CBUFFER);

  textures = new csTextureList ();
  materials = new csMaterialList ();

  render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory());
  lightpatch_pool = new csLightPatchPool ();

  BuildSqrtTable ();
  resize = false;
}

// @@@ Hack
csCamera* camera_hack = NULL;

csEngine::~csEngine ()
{
  Clear ();
  if (G3D) G3D->DecRef ();
  if (VFS) VFS->DecRef ();
  if (System) System->DecRef ();
  delete textures;
  delete materials;
  delete render_pol2d_pool;
  delete lightpatch_pool;
  delete covcube;
  delete cbufcube;
  delete covtree_lut;
  delete rad_debug;
  delete c_buffer;
  delete covtree;
  delete quad3d;
  
  // @@@ temp hack
  delete camera_hack;
  camera_hack = NULL;
}

bool csEngine::Initialize (iSystem* sys)
{
#if defined(JORRIT_DEBUG)
  printf ("csPolygon3D %ld\n", (long)sizeof (csPolygon3D));
  printf ("csPolyPlane %ld\n", (long)sizeof (csPolyPlane));
  printf ("csPolyTxtPlane %ld\n", (long)sizeof (csPolyTxtPlane));
  printf ("csPolyTexture %ld\n", (long)sizeof (csPolyTexture));
  printf ("csPolygonSet %ld\n", (long)sizeof (csPolygonSet));
  printf ("csLightMapped %ld\n", (long)sizeof (csLightMapped));
  printf ("csGouraudShaded %ld\n", (long)sizeof (csGouraudShaded));
  printf ("csLightMap %ld\n", (long)sizeof (csLightMap));
#endif

  (System = sys)->IncRef ();

  if (!(G3D = QUERY_PLUGIN_ID (sys, CS_FUNCID_VIDEO, iGraphics3D)))
    return false;

  if (!(VFS = QUERY_PLUGIN_ID (sys, CS_FUNCID_VFS, iVFS)))
    return false;

  G2D = G3D->GetDriver2D ();

  // Tell system driver that we want to handle broadcast events
  if (!System->CallOnEvents (this, CSMASK_Broadcast))
    return false;

  ReadConfig ();

  return true;
}

// Handle some system-driver broadcasts
bool csEngine::HandleEvent (iEvent &Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
      {
        csGraphics3DCaps *caps = G3D->GetCaps ();
        fogmethod = caps->fog;
        NeedPO2Maps = caps->NeedsPO2Maps;
        MaxAspectRatio = caps->MaxAspectRatio;

        frame_width = G3D->GetWidth ();
        frame_height = G3D->GetHeight ();
        if (csCamera::GetDefaultFOV () == 0)
          csCamera::SetDefaultFOV (frame_height, frame_width);

        // @@@ Ugly hack to always have a camera in current_camera.
        // This is needed for the lighting routines.
        if (!current_camera)
        {
          current_camera = new csCamera ();
          camera_hack = current_camera;
        }

        // Allow context resizing since we handle cscmdContextResize
        G2D->AllowCanvasResize (true);

        StartEngine ();

        return true;
      }
      case cscmdSystemClose:
      {
        // We must free all material and texture handles since after
        // G3D->Close() they all become invalid, no matter whenever
        // we did or didn't an IncRef on them.
        Clear ();
        return true;
      }
      case cscmdContextResize:
      {
	if (engine_states)
	  engine_states->Resize ((iGraphics2D*) Event.Command.Info);
	else
	  if (((iGraphics2D*) Event.Command.Info) == G2D)
	    resize = true;
	return false;
      }
      case cscmdContextClose:
      {
	if (engine_states)
	{
	  engine_states->Close ((iGraphics2D*) Event.Command.Info);
	  if (!engine_states->Length())
	  {
	    delete engine_states;
	    engine_states = NULL;
	  }
	}
	return false;
      }
    } /* endswitch */

  return false;
}

void csEngine::Clear ()
{
  if (G3D) G3D->ClearCache ();
  halos.DeleteAll ();
  collections.DeleteAll ();
  sprites.DeleteAll ();
  things.DeleteAll ();
  skies.DeleteAll ();
  sprite_templates.DeleteAll ();
  meshobj_factories.DeleteAll ();
  curve_templates.DeleteAll ();
  thing_templates.DeleteAll ();
  sectors.DeleteAll ();
  camera_positions.DeleteAll ();
  int i;
  for (i = 0 ; i < planes.Length () ; i++)
  {
    csPolyTxtPlane* p = (csPolyTxtPlane*)planes[i];
    planes[i] = NULL;
    p->DecRef ();
  }
  planes.DeleteAll ();

  while (first_dyn_lights)
  {
    csDynLight* dyn = first_dyn_lights->GetNext ();
    delete first_dyn_lights;
    first_dyn_lights = dyn;
  }
  delete textures; 
  textures = NULL;
  textures = new csTextureList ();
  delete materials; 
  materials = NULL;
  materials = new csMaterialList ();

  // Delete engine states and their references to cullers before cullers are
  // deleted in SetCuller below.
  if (engine_states)
  {
    engine_states->DeleteAll ();
    delete engine_states;
    engine_states = NULL;
  }

  SetCuller (CS_CULLER_CBUFFER);
  delete render_pol2d_pool;
  render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory());
  delete lightpatch_pool;
  lightpatch_pool = new csLightPatchPool ();
  use_pvs = false;

  // Clear all object libraries
  library = NULL;
  libraries.DeleteAll ();

  // Clear all regions.
  region = NULL;
  regions.DeleteAll ();

  tr_manager.Initialize ();
}

void csEngine::ResolveEngineMode ()
{
  if (engine_mode == CS_ENGINE_AUTODETECT)
  {
    // Here we do some heuristic. We scan all sectors and see if there
    // are some that have big octrees. If so we select CS_ENGINE_FRONT2BACK.
    // If not we select CS_ENGINE_BACK2FRONT.
    // @@@ It might be an interesting option to try to find a good engine
    // mode for every sector and switch dynamically based on the current
    // sector (only switch based on the position of the camera, not switch
    // based on the sector we are currently rendering).
    int switch_f2b = 0;
    for (int i = 0 ; i < sectors.Length () ; i++)
    {
      csSector* s = (csSector*)sectors[i];
      csPolygonTree* ptree = s->GetStaticTree ();
      if (ptree)
      {
        csOctree* otree = (csOctree*)ptree;
	int num_nodes = otree->GetRoot ()->CountChildren ();
	if (num_nodes > 30) switch_f2b += 10;
	else if (num_nodes > 15) switch_f2b += 5;
      }
      if (switch_f2b >= 10) break;
    }
    if (switch_f2b >= 10)
    {
      engine_mode = CS_ENGINE_FRONT2BACK;
      CsPrintf (MSG_CONSOLE, "Engine is using front2back mode\n");
    }
    else
    {
      engine_mode = CS_ENGINE_BACK2FRONT;
      CsPrintf (MSG_CONSOLE, "Engine is using back2front mode\n");
    }
  }
}

void csEngine::EnableLightingCache (bool en)
{
  do_lighting_cache = en;
  if (!do_lighting_cache) do_force_relight = true;
  else do_force_relight = false;
}

void csEngine::SetCuller (int culler)
{
  delete c_buffer; c_buffer = NULL;
  delete covtree; covtree = NULL;
  delete quad3d; quad3d = NULL;
  switch (culler)
  {
    case CS_CULLER_CBUFFER:
      c_buffer = new csCBuffer (0, frame_width-1, frame_height);
      break;
    case CS_CULLER_QUAD3D:
    {
      csVector3 corners[4];
      corners[0].Set (-1, -1, 1);
      corners[1].Set (1, -1, 1);
      corners[2].Set (1, 1, 1);
      corners[3].Set (-1, 1, 1);
      csBox3 bbox;
      quad3d = new csQuadTree3D (csVector3 (0, 0, 0),
    	  corners, bbox, 5);
      break;
    }
    case CS_CULLER_COVTREE:
    {
      csBox2 box (0, 0, frame_width, frame_height);
      if (!covtree_lut)
      {
        covtree_lut = new csCovMaskLUT (16);
      }
      covtree = new csCoverageMaskTree (covtree_lut, box);
      break;
    }
  }
}

void csEngine::PrepareTextures ()
{
  int i;

  iTextureManager* txtmgr = G3D->GetTextureManager ();
  txtmgr->ResetPalette ();

  // First register all textures to the texture manager.
  for (i = 0; i < textures->Length (); i++)
  {
    csTextureWrapper *csth = textures->Get (i);
    if (!csth->GetTextureHandle ())
      csth->Register (txtmgr);
  }

  // Prepare all the textures.
  txtmgr->PrepareTextures ();

  // Then register all materials to the texture manager.
  for (i = 0; i < materials->Length (); i++)
  {
    csMaterialWrapper *csmh = materials->Get (i);
    if (!csmh->GetMaterialHandle ())
      csmh->Register (txtmgr);
  }

  // Prepare all the materials.
  txtmgr->PrepareMaterials ();
}

void csEngine::PrepareSectors()
{
  // Now precalculate some stuff for all loaded polygons.
  for (int i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* s = (csSector*)sectors[i];
    s->Prepare (s);
  }
}

// If a STAT_BSP level the loader call to UpdateMove is redundant when called
// before csEngine::Prepare().
void csEngine::PrepareSprites ()
{
  int i;
  for (i = 0 ; i < sprites.Length () ; i++)
  {
    csSprite* sp = (csSprite*)sprites[i];
    sp->GetMovable ().UpdateMove ();
  }
}

bool csEngine::Prepare ()
{
  PrepareTextures ();
  PrepareSectors ();
  PrepareSprites ();
  // The images are no longer needed by the 3D engine.
  iTextureManager *txtmgr = G3D->GetTextureManager ();
  txtmgr->FreeImages ();

  G3D->ClearCache ();

  // Prepare lightmaps if we have any sectors
  if (sectors.Length ())
    ShineLights ();

  CheckConsistency ();

  long memory = 0;
  long tree_memory = 0;
  for (int i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* s = (csSector*)sectors[i];
    memory += Dumper::Memory (s, 0);
    if (s->GetStaticTree ())
    {
      csOctree* otree = (csOctree*)(s->GetStaticTree ());
      tree_memory += Dumper::Memory (otree, 0);
    }
  }
  CsPrintf (MSG_INITIALIZATION, "World geometry is using %ld bytes.\n", memory);
  CsPrintf (MSG_INITIALIZATION, "Octree/BSP trees are using %ld bytes.\n", tree_memory);
  CsPrintf (MSG_INITIALIZATION, "Textures are using %ld texels.\n",
  	Dumper::TotalTexels (textures));

  return true;
}

void csEngine::ShineLights (csRegion* region)
{
  tr_manager.NewFrame ();

  if (!do_not_force_relight)
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
      int ambient_red;
      int ambient_green;
      int ambient_blue;
      int reflect;
      int radiosity;
      float cosinus_factor;
      int lightmap_size;
    };
    PrecalcInfo current;
    memset (&current, 0, sizeof (current));
    current.lm_version = 1;
    current.normal_light_level = NORMAL_LIGHT_LEVEL;
    current.ambient_red = csLight::ambient_red;
    current.ambient_green = csLight::ambient_green;
    current.ambient_blue = csLight::ambient_blue;
    current.reflect = csSector::cfg_reflections;
    current.radiosity = (int)csSector::do_radiosity;
    current.cosinus_factor = csPolyTexture::cfg_cosinus_factor;
    current.lightmap_size = csLightMap::lightcell_size;
    char *reason = NULL;

    iDataBuffer *data = VFS->ReadFile ("precalc_info");
    if (!data)
      reason = "no 'precalc_info' found";
    else
    {
      char *input = **data;
      while (*input)
      {
        char *keyword = input + strspn (input, " \t");
        char *endkw = strchr (input, '=');
        if (!endkw) break;
        *endkw++ = 0;
        input = strchr (endkw, '\n');
        if (input) *input++ = 0;
        float xf;
        sscanf (endkw, "%f", &xf);
        int xi = QRound (xf);

#define CHECK(keyw,cond,reas) \
        else if (!strcmp (keyword, keyw)) \
        { if (cond) { reason = reas " changed"; break; } }

        if (false) {}
        CHECK ("LMVERSION", xi != current.lm_version, "lightmap format")
        CHECK ("LIGHTMAP_SIZE", xi != current.lightmap_size, "lightmap size")

#undef CHECK
      }
    }
    if (data) data->DecRef ();

    if (reason)
    {
      char data [500];
      sprintf (data,
        "LMVERSION=%d\n"
        "NORMALLIGHTLEVEL=%d\n"
        "AMBIENT_RED=%d\n"
        "AMBIENT_GREEN=%d\n"
        "AMBIENT_BLUE=%d\n"
        "REFLECT=%d\n"
        "RADIOSITY=%d\n"
        "COSINUS_FACTOR=%g\n"
        "LIGHTMAP_SIZE=%d\n",
        current.lm_version, current.normal_light_level, current.ambient_red,
        current.ambient_green, current.ambient_blue, current.reflect,
        current.radiosity, current.cosinus_factor,
        current.lightmap_size);
      VFS->WriteFile ("precalc_info", data, strlen (data));
      CsPrintf (MSG_INITIALIZATION, "Lightmap data is not up to date (reason: %s).\n", reason);
      do_force_relight = true;
    }
  }

  if (do_force_relight)
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

  csPolyIt* pit = NewPolyIterator (region);
  csLightIt* lit = NewLightIterator (region);

  // Reinit all lightmaps. This loop also counts all polygons.
  csPolygon3D* p;
  int polygon_count = 0;
  pit->Restart ();
  while ((p = pit->Fetch ()) != NULL)
    polygon_count++;

  // Count number of lights to process.
  csLight* l;
  int light_count = 0;
  lit->Restart ();
  while (lit->Fetch ()) light_count++;

  int sn = 0;
  int num_sectors = sectors.Length ();
  csProgressMeter meter (System);

  CsPrintf (MSG_INITIALIZATION, "Initializing lightmaps (%d sectors):\n  ", num_sectors);
  meter.SetTotal (num_sectors);
  meter.Restart ();
  for (sn = 0; sn < num_sectors ; sn++)
  {
    csSector* s = (csSector*)sectors [sn];
    if (!region || region->IsInRegion (s))
      s->InitLightMaps ();
    meter.Step();
  }

  cs_time start, stop;
  start = System->GetTime ();
  CsPrintf (MSG_INITIALIZATION, "\nShining lights (%d lights):\n  ", light_count);
  meter.SetTotal (light_count);
  meter.Restart ();
  lit->Restart ();
  while ((l = lit->Fetch ()) != NULL)
  {
    ((csStatLight*)l)->CalculateLighting ();
    meter.Step();
  }
  stop = System->GetTime ();
  CsPrintf (MSG_INITIALIZATION, "\n(%.4f seconds)", (float)(stop-start)/1000.);

  // Render radiosity
  if (use_new_radiosity && !do_not_force_relight && do_force_relight)
  {
    start = System->GetTime ();
    csRadiosity *rad = new csRadiosity(this);
    if (do_rad_debug)
    {
      rad_debug = rad;
    }
    else
    {
      rad->DoRadiosity();
      delete rad;
    }
    stop = System->GetTime ();
    CsPrintf (MSG_INITIALIZATION, "(%.4f seconds)", (float)(stop-start)/1000.);
  }

  CsPrintf (MSG_INITIALIZATION, "\nCaching lightmaps (%d sectors):\n  ", num_sectors);
  meter.SetTotal (num_sectors);
  meter.Restart ();
  for (sn = 0; sn < num_sectors ; sn++)
  {
    csSector* s = (csSector*)sectors[sn];
    if (!region || region->IsInRegion (s))
      s->CacheLightMaps ();
    meter.Step();
  }

  CsPrintf (MSG_INITIALIZATION, "\nPreparing lightmaps (%d maps):\n  ", polygon_count);
  meter.SetTotal (polygon_count);
  meter.Restart ();
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

  delete pit;
  delete lit;
}

void csEngine::InvalidateLightmaps ()
{
  csPolyIt* pit = NewPolyIterator ();
  csPolygon3D* p;
  while ((p = pit->Fetch ()) != NULL)
  {
    csPolyTexLightMap* lmi = p->GetLightMapInfo ();
    if (lmi && lmi->GetLightMap ())
      ((csLightMap*)lmi->GetLightMap ())->MakeDirtyDynamicLights ();
  }
  delete pit;
}

bool csEngine::CheckConsistency ()
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
      csVector3 poly[3];
      int i, j;
      poly[0] = p->Vobj (0);
      j = 1;
      for (i = 1 ; i < p->GetNumVertices () ; i++)
        if (!((poly[j-1] - p->Vobj (i)) < SMALL_EPSILON))
	{
	  poly[j] = p->Vobj (i);
	  j++;
	  if (j > 2) break;
	}
      if (j > 2)
      {
        csVector3 normal;
        float D;
        csMath3::CalcPlane (poly[0], poly[1], poly[2], normal, D);
        csPlane3 pl (normal, D);
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
  }

  delete pit;
  CsPrintf (MSG_INITIALIZATION, "DONE\n");
  return error;
}

void csEngine::StartEngine ()
{
  Clear ();
}

void csEngine::StartDraw (csCamera* c, csClipper* view, csRenderView& rview)
{
  Stats::polygons_considered = 0;
  Stats::polygons_drawn = 0;
  Stats::portals_drawn = 0;
  Stats::polygons_rejected = 0;
  Stats::polygons_accepted = 0;

  // Make sure we select an engine mode.
  ResolveEngineMode ();

  current_camera = c;
  rview.engine = this;

  // This flag is set in HandleEvent on a cscmdContextResize event
  if (resize)
  {
    resize = false;
    Resize ();
  }

  top_clipper = view;

  rview.clip_plane.Set (0, 0, 1, -1);   //@@@CHECK!!!

  // Calculate frustum for screen dimensions (at z=1).
  float leftx = - c->GetShiftX () * c->GetInvFOV ();
  float rightx = (frame_width - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = - c->GetShiftY () * c->GetInvFOV ();
  float boty = (frame_height - c->GetShiftY ()) * c->GetInvFOV ();
  rview.SetFrustum (leftx, rightx, topy, boty);

  // Initialize our transformation manager so that we will get
  // new transformed coordinates.
  tr_manager.NewFrame ();

  // Initialize the 2D/3D culler.
  if (engine_mode == CS_ENGINE_FRONT2BACK)
  {
    if (c_buffer)
    {
      c_buffer->Initialize ();
      c_buffer->InsertPolygon (view->GetClipPoly (), view->GetNumVertices (), true);
    }
    else if (quad3d)
    {
      csVector3 corners[4];
      c->InvPerspective (csVector2 (0, 0), 1, corners[0]);
      corners[0] = c->Camera2World (corners[0]);
      c->InvPerspective (csVector2 (frame_width-1, 0), 1, corners[1]);
      corners[1] = c->Camera2World (corners[1]);
      c->InvPerspective (csVector2 (frame_width-1, frame_height-1), 1, corners[2]);
      corners[2] = c->Camera2World (corners[2]);
      c->InvPerspective (csVector2 (0, frame_height-1), 1, corners[3]);
      corners[3] = c->Camera2World (corners[3]);
      quad3d->SetMainFrustum (c->GetOrigin (), corners);
      quad3d->MakeEmpty ();
    }
    else if (covtree)
    {
      covtree->MakeEmpty ();
      covtree->UpdatePolygonInverted (view->GetClipPoly (), view->GetNumVertices ());
    }
  }

  cur_process_polygons = 0;
}

void csEngine::Draw (csCamera* c, csClipper* view)
{
  csRenderView rview (*c, view, G3D, G2D);
  StartDraw (c, view, rview);
  rview.callback = NULL;

  csSector* s = c->GetSector ();
  s->Draw (rview);

  // draw all halos on the screen
  cs_time Elapsed, Current;
  System->GetElapsedTime (Elapsed, Current);
  for (int halo = halos.Length () - 1; halo >= 0; halo--)
    if (!halos.Get (halo)->Process (Elapsed, *this))
      halos.Delete (halo);
}

void csEngine::DrawFunc (csCamera* c, csClipper* view,
  csDrawFunc* callback, void* callback_data)
{
  csRenderView rview (*c, view, G3D, G2D);
  StartDraw (c, view, rview);

  rview.callback = callback;
  rview.callback_data = callback_data;

  csSector* s = c->GetSector ();
  s->Draw (rview);
}

void csEngine::AddHalo (csLight* Light)
{
  if (!Light->GetHalo () || Light->flags.Check (CS_LIGHT_ACTIVEHALO))
    return;

  // Transform light pos into camera space and see if it is directly visible
  csVector3 v = current_camera->World2Camera (Light->GetCenter ());

  // Check if light is behind us
  if (v.z <= SMALL_Z)
    return;

  // Project X,Y into screen plane
  float iz = current_camera->GetFOV () / v.z;
  v.x = v.x * iz + current_camera->GetShiftX ();
  v.y = frame_height - 1 - (v.y * iz + current_camera->GetShiftY ());

  // If halo is not inside visible region, return
  if (!top_clipper->IsInside (csVector2 (v.x, v.y)))
    return;

  // Check if light is not obscured by anything
  float zv = G3D->GetZBuffValue (QRound (v.x), QRound (v.y));
  if (v.z > zv)
    return;

  // Halo size is 1/4 of the screen height; also we make sure its odd
  int hs = (frame_height / 4) | 1;
  unsigned char *Alpha = Light->GetHalo ()->Generate (hs);

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

void csEngine::RemoveHalo (csLight* Light)
{
  int idx = halos.FindKey (Light);
  if(idx!=-1)
    halos.Delete (idx);
}

csStatLight* csEngine::FindLight (float x, float y, float z, float dist)
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

csStatLight* csEngine::FindLight (CS_ID id)
{
  csStatLight* l;
  int sn = sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)sectors[sn];
    l = s->FindLight (id);
    if (l) return l;
  }
  return NULL;
}

csStatLight* csEngine::FindLight (const char* name, bool /*regionOnly*/)
{
  //@@@### regionOnly
  csStatLight* l;
  int sn = sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)sectors[sn];
    l = (csStatLight*)s->lights.FindByName (name);
    if (l) return l;
  }
  return NULL;
}

void csEngine::AddDynLight (csDynLight* dyn)
{
  dyn->SetNext (first_dyn_lights);
  dyn->SetPrev (NULL);
  if (first_dyn_lights) first_dyn_lights->SetPrev (dyn);
  first_dyn_lights = dyn;
}

void csEngine::RemoveDynLight (csDynLight* dyn)
{
  if (dyn->GetNext ()) dyn->GetNext ()->SetPrev (dyn->GetPrev ());
  if (dyn->GetPrev ()) dyn->GetPrev ()->SetNext (dyn->GetNext ());
  else if (dyn == first_dyn_lights) first_dyn_lights = dyn->GetNext ();
  dyn->SetNext (NULL);
  dyn->SetPrev (NULL);
}

void csEngine::NextFrame (cs_time current_time)
{
  int i;
  for (i = 0 ; i < sprites.Length () ; i++)
  {
    csSprite* sp = (csSprite*)sprites[i];
    sp->NextFrame (current_time);
  }

  // Delete particle systems that self-destructed now.
  i = sprites.Length ()-1;
  while (i >= 0)
  {
    csSprite* sp = (csSprite*)sprites[i];
    if (sp->WantToDie ())
      delete sp;
    i--;
  }
}

void csEngine::ReadConfig ()
{
  if (!System) return;
  iConfigFile *Config = System->GetConfig ();
  csLightMap::SetLightCellSize (Config->GetInt ("Lighting", "LightmapSize", 16));

  csLight::ambient_red = Config->GetInt ("Lighting", "Ambient.Red", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_green = Config->GetInt ("Lighting", "Ambient.Green", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_blue = Config->GetInt ("Lighting", "Ambient.Blue", DEFAULT_LIGHT_LEVEL);
  int ambient_white = Config->GetInt ("Lighting", "Ambient.White", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_red += ambient_white;
  csLight::ambient_green += ambient_white;
  csLight::ambient_blue += ambient_white;

  // Do not allow too black environments since software renderer hates it
  if (csLight::ambient_red + csLight::ambient_green + csLight::ambient_blue < 6)
    csLight::ambient_red = csLight::ambient_green = csLight::ambient_blue = 2;

  csSector::cfg_reflections = Config->GetInt ("Lighting", "Reflections", csSector::cfg_reflections);
  csPolyTexture::cfg_cosinus_factor = Config->GetFloat ("Lighting", "CosinusFactor", csPolyTexture::cfg_cosinus_factor);
  csSprite3D::global_lighting_quality = Config->GetInt ("Lighting", "LightingQual", csSprite3D::global_lighting_quality);
  csSector::do_radiosity = Config->GetYesNo ("Lighting", "Radiosity", csSector::do_radiosity);

  // radiosity options
  csEngine::use_new_radiosity = Config->GetYesNo ("Lighting",
    "Radiosity.Enable", csEngine::use_new_radiosity);
  csRadiosity::do_static_specular = Config->GetYesNo ("Lighting",
    "Radiosity.DoStaticSpecular", csRadiosity::do_static_specular);
  csRadiosity::static_specular_amount = Config->GetFloat ("Lighting",
    "Radiosity.StaticSpecularAmount", csRadiosity::static_specular_amount);
  csRadiosity::static_specular_tightness = Config->GetInt ("Lighting",
    "Radiosity.StaticSpecularTightness", csRadiosity::static_specular_tightness);
  csRadiosity::colour_bleed = Config->GetFloat ("Lighting",
    "Radiosity.ColourBleed", csRadiosity::colour_bleed);
  csRadiosity::stop_priority = Config->GetFloat ("Lighting",
    "Radiosity.StopPriority", csRadiosity::stop_priority);
  csRadiosity::stop_improvement = Config->GetFloat ("Lighting",
    "Radiosity.StopImprovement", csRadiosity::stop_improvement);
  csRadiosity::stop_iterations = Config->GetInt ("Lighting",
    "Radiosity.StopIterations", csRadiosity::stop_iterations);
  csRadiosity::source_patch_size = Config->GetInt ("Lighting",
    "Radiosity.SourcePatchSize", csRadiosity::source_patch_size);
}

void csEngine::UnlinkSprite (csSprite* sprite)
{
  sprite->GetMovable ().ClearSectors ();
  int idx = sprites.Find (sprite);
  if (idx == -1) return;
  sprites[idx] = NULL;
  sprites.Delete (idx);
}

void csEngine::RemoveSprite (csSprite* sprite)
{
  sprite->GetMovable ().ClearSectors ();
  int idx = sprites.Find (sprite);
  if (idx == -1) return;
  sprites[idx] = NULL;
  sprites.Delete (idx);
  delete sprite;
}

void csEngine::UnlinkThing (csThing* thing)
{
  thing->GetMovable ().ClearSectors ();
  int idx = things.Find (thing);
  if (idx == -1) return;
  things[idx] = NULL;
  things.Delete (idx);
}

void csEngine::RemoveThing (csThing* thing)
{
  thing->GetMovable ().ClearSectors ();
  int idx = things.Find (thing);
  if (idx == -1) return;
  things[idx] = NULL;
  things.Delete (idx);
  delete thing;
}

void csEngine::UnlinkSky (csThing* thing)
{
  thing->GetMovable ().ClearSectors ();
  int idx = skies.Find (thing);
  if (idx == -1) return;
  skies[idx] = NULL;
  skies.Delete (idx);
}

void csEngine::RemoveSky (csThing* thing)
{
  thing->GetMovable ().ClearSectors ();
  int idx = skies.Find (thing);
  if (idx == -1) return;
  skies[idx] = NULL;
  skies.Delete (idx);
  delete thing;
}

void csEngine::UnlinkCollection (csCollection* collection)
{
  collection->GetMovable ().ClearSectors ();
  int idx = collections.Find (collection);
  if (idx == -1) return;
  collections[idx] = NULL;
  collections.Delete (idx);
}

void csEngine::RemoveCollection (csCollection* collection)
{
  collection->GetMovable ().ClearSectors ();
  int idx = collections.Find (collection);
  if (idx == -1) return;
  collections[idx] = NULL;
  collections.Delete (idx);
  delete collection;
}

struct LightAndDist
{
  csLight* light;
  float sqdist;
};

// csLightArray is a subclass of csCleanable which is registered
// to csEngine.cleanup.
class csLightArray : public csBase
{
public:
  LightAndDist* array;
  // Size is the physical size of the array. num_lights is the number of lights in it.
  int size, num_lights;

  csLightArray () : array (NULL), size (0), num_lights (0) { }
  virtual ~csLightArray () { delete [] array; }
  void Reset () { num_lights = 0; }
  void AddLight (csLight* l, float sqdist)
  {
    if (num_lights >= size)
    {
      LightAndDist* new_array;
      new_array = new LightAndDist [size+5];
      if (array)
      {
        memcpy (new_array, array, sizeof (LightAndDist)*num_lights);
        delete [] array;
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

int csEngine::GetNearbyLights (csSector* sector, const csVector3& pos, ULong flags,
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
  // in csEngine so that it will be freed later.

  static csLightArray* light_array = NULL;
  if (!light_array)
  {
    light_array = new csLightArray ();
    csEngine::current_engine->cleanup.Push (light_array);
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

csSectorIt* csEngine::GetNearbySectors (csSector* sector,
	const csVector3& pos, float radius)
{
  csSectorIt* it = new csSectorIt (sector, pos, radius);
  return it;
}

csObjectIt* csEngine::GetNearbyObjects (const csIdType& type, bool derived,
  	csSector* sector, const csVector3& pos, float radius)
{
  csObjectIt* it = new csObjectIt (this, type, derived, sector, pos, radius);
  return it;
}

int csEngine::GetTextureFormat ()
{
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  return txtmgr->GetTextureFormat ();
}

void csEngine::SelectRegion (const char *iName)
{
  if (iName == NULL)
  {
    region = NULL; // Default engine region.
    return;
  }

  region = (csRegion*)regions.FindByName (iName);
  if (!region)
  {
    region = new csRegion (this);
    region->SetName (iName);
    regions.Push (region);
  }
}

iRegion* csEngine::GetCurrentRegion ()
{
  return &region->scfiRegion;
}

void csEngine::AddToCurrentRegion (csObject* obj)
{
  if (region)
  {
    region->AddToRegion (obj);
  }
}

void csEngine::SelectLibrary (const char *iName)
{
  library = libraries.FindByName (iName);
  if (!library)
  {
    library = new csObjectNoDel ();
    library->SetName (iName);
    libraries.Push (library);
  }
}

bool csEngine::DeleteLibrary (const char *iName)
{
  csObject *lib = libraries.FindByName (iName);
  if (!lib) return false;

#define DELETE_ALL_OBJECTS(vector,type)				\
  { for (csObjIterator iter = lib->GetIterator (type::Type);	\
         !iter.IsFinished (); ++iter)				\
    {								\
      type &x = (type&)*iter;					\
      int idx = vector.Find (&x);					\
      if (idx >= 0) vector.Delete (idx);				\
  } }

  DELETE_ALL_OBJECTS (collections, csCollection)
  DELETE_ALL_OBJECTS (sprites, csSprite)
  DELETE_ALL_OBJECTS (sprite_templates, csSpriteTemplate)
  DELETE_ALL_OBJECTS (meshobj_factories, csMeshFactoryWrapper)
  DELETE_ALL_OBJECTS (curve_templates, csCurveTemplate)
  DELETE_ALL_OBJECTS (thing_templates, csThing)
  DELETE_ALL_OBJECTS (sectors, csSector)
  DELETE_ALL_OBJECTS ((*textures), csTextureWrapper)
  DELETE_ALL_OBJECTS ((*materials), csMaterialWrapper)

#undef DELETE_ALL_OBJECTS
#define DELETE_ALL_OBJECTS(vector,type)				\
  for (csObjIterator iter = lib->GetIterator (type::Type);	\
       !iter.IsFinished (); ++iter)				\
  {								\
    type &x = (type&)*iter;					\
    int idx = vector.Find (&x);					\
    if (idx >= 0) { x.DecRef (); vector [idx] = 0; }		\
  }

  DELETE_ALL_OBJECTS (planes, csPolyTxtPlane)

/*
@@todo: move all dynamic lights to a vector
  while (first_dyn_lights)
  {
    csDynLight *dyn = first_dyn_lights->GetNext ();
    delete first_dyn_lights;
    first_dyn_lights = dyn;
  }
*/

  return true;
}

void csEngine::DeleteAll ()
{
  Clear ();
}

iTextureWrapper* csEngine::CreateTexture (const char *iName, const char *iFileName,
  csColor *iTransp, int iFlags)
{
  // First of all, load the image file
  iDataBuffer *data = VFS->ReadFile (iFileName);
  if (!data || !data->GetSize ())
  {
    if (data) data->DecRef ();
    CsPrintf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n",
      iFileName);
    return NULL;
  }

  iImage *ifile = csImageLoader::Load (data->GetUint8 (), data->GetSize (),
    CS_IMGFMT_TRUECOLOR);// GetTextureFormat ());
  data->DecRef ();

  if (!ifile)
  {
    CsPrintf (MSG_WARNING, "Unknown image file format: \"%s\"\n", iFileName);
    return NULL;
  }

  iDataBuffer *xname = VFS->ExpandPath (iFileName);
  ifile->SetName (**xname);
  xname->DecRef ();

  // Okay, now create the respective texture handle object
  csTextureWrapper *tex = GetTextures ()->FindByName (iName);
  if (tex)
    tex->SetImageFile (ifile);
  else
    tex = GetTextures ()->NewTexture (ifile);

  tex->flags = iFlags;
  tex->SetName (iName);

  // dereference image pointer since tex already incremented it
  ifile->DecRef ();

  if (iTransp)
    tex->SetKeyColor (QInt (iTransp->red * 255.2),
      QInt (iTransp->green * 255.2), QInt (iTransp->blue * 255.2));

  return &tex->scfiTextureWrapper;
}

iMaterialWrapper* csEngine::CreateMaterial (const char *iName, iTextureWrapper* texture)
{
  csMaterial* mat =
    new csMaterial (((csTextureWrapper::TextureWrapper*)texture)->scfParent);
  csMaterialWrapper* wrapper = materials->NewMaterial (mat);
  wrapper->SetName (iName);
  return &wrapper->scfiMaterialWrapper;
}

bool csEngine::CreateCamera (const char *iName, const char *iSector,
  const csVector3 &iPos, const csVector3 &iForward, const csVector3 &iUpward)
{
  csCameraPosition *cp = (csCameraPosition *)camera_positions.FindByName (iName);
  if (cp)
    cp->Set (iSector, iPos, iForward, iUpward);
  else
    camera_positions.Push (new csCameraPosition (iName, iSector,
      iPos, iForward, iUpward));

  return true;
}

bool csEngine::CreateKey (const char *iName, const char *iValue)
{
  ObjAdd (new csKeyValuePair (iName, iValue));
  return true;
}

bool csEngine::CreatePlane (const char *iName, const csVector3 &iOrigin,
  const csMatrix3 &iMatrix)
{
  csPolyTxtPlane *ppl = new csPolyTxtPlane ();
  ppl->SetName (iName);
  ppl->SetTextureSpace (iMatrix, iOrigin);
  planes.Push (ppl);
  return true;
}

csSector* csEngine::CreateCsSector (const char *iName)
{
  csSector* sector = new csSector (this);
  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);
  sector->SetName (iName);
  sectors.Push (sector);
  return sector;
}

iSector* csEngine::CreateSector (const char *iName)
{
  csSector* sector = CreateCsSector (iName);
  iSector *s = QUERY_INTERFACE (sector, iSector);
  sector->DecRef ();
  return s;
}

iThing *csEngine::CreateThing (const char *iName, iSector *iParent)
{
  csThing *thing = new csThing (this);
  thing->SetName (iName);
  csSector *sector = iParent->GetPrivateObject ();
  thing->GetMovable ().SetSector (sector);
  things.Push (thing);
  iThing *p = QUERY_INTERFACE (thing, iThing);
  thing->DecRef ();
  return p;
}

iSector *csEngine::GetSector (int iIndex)
{
  return &((csSector *)sectors.Get (iIndex))->scfiSector;
}

csObject* csEngine::FindObjectInRegion (csRegion* region,
	csNamedObjVector& vector, const char* name)
{
  for (int i = vector.Length () - 1; i >= 0; i--)
  {
    csObject* o = (csObject *)vector[i];
    if (!region->IsInRegion (o)) continue;
    const char* oname = o->GetName ();
    if (name == oname || (name && oname && !strcmp (oname, name)))
      return o;
  }
  return NULL;
}

iSector *csEngine::FindSector (const char *iName, bool regionOnly)
{
  csSector* sec;
  if (regionOnly && region)
    sec = (csSector*)FindObjectInRegion (region, sectors, iName);
  else
    sec = (csSector *)sectors.FindByName (iName);
  return sec ? &sec->scfiSector : NULL;
}

iThing *csEngine::FindThing (const char *iName, bool regionOnly)
{
  csThing* thing;
  if (regionOnly && region)
    thing = (csThing*)FindObjectInRegion (region, things, iName);
  else
    thing = (csThing*)things.FindByName (iName);
  return thing ? &thing->scfiThing : NULL;
}

iThing *csEngine::FindSky (const char *iName, bool regionOnly)
{
  csThing* thing;
  if (regionOnly && region)
    thing = (csThing*)FindObjectInRegion (region, skies, iName);
  else
    thing = (csThing*)skies.FindByName (iName);
  return thing ? &thing->scfiThing : NULL;
}

iThing *csEngine::FindThingTemplate (const char *iName, bool regionOnly)
{
  csThing* thing;
  if (regionOnly && region)
    thing = (csThing*)FindObjectInRegion (region, thing_templates, iName);
  else
    thing = (csThing*)thing_templates.FindByName (iName);
  return thing ? &thing->scfiThing : NULL;
}

iSprite *csEngine::FindSprite (const char *iName, bool regionOnly)
{
  csSprite* sprite;
  if (regionOnly && region)
    sprite = (csSprite*)FindObjectInRegion (region, sprites, iName);
  else
    sprite = (csSprite*)sprites.FindByName (iName);
  return sprite ? &sprite->scfiSprite : NULL;
}

iMeshFactoryWrapper *csEngine::FindMeshFactory (const char *iName, bool regionOnly)
{
  csMeshFactoryWrapper* fact;
  if (regionOnly && region)
    fact = (csMeshFactoryWrapper*)FindObjectInRegion (region, meshobj_factories, iName);
  else
    fact = (csMeshFactoryWrapper*)meshobj_factories.FindByName (iName);
  return fact ? &fact->scfiMeshFactoryWrapper : NULL;
}

iSpriteTemplate *csEngine::FindSpriteTemplate (const char *iName, bool regionOnly)
{
  csSpriteTemplate* sprite;
  if (regionOnly && region)
    sprite = (csSpriteTemplate*)FindObjectInRegion (region, sprite_templates, iName);
  else
    sprite = (csSpriteTemplate*)sprite_templates.FindByName (iName);
  return sprite ? &sprite->scfiSpriteTemplate : NULL;
}

iMaterialWrapper* csEngine::FindMaterial (const char* iName, bool regionOnly)
{
  csMaterialWrapper* wr;
  if (regionOnly && region)
    wr = (csMaterialWrapper*)FindObjectInRegion (region, *materials, iName);
  else
    wr = materials->FindByName (iName);
  return wr ? &wr->scfiMaterialWrapper : NULL;
}

csMaterialWrapper* csEngine::FindCsMaterial (const char* iName, bool regionOnly)
{
  csMaterialWrapper* wr;
  if (regionOnly && region)
    wr = (csMaterialWrapper*)FindObjectInRegion (region, *materials, iName);
  else
    wr = materials->FindByName (iName);
  return wr;
}

iTextureWrapper* csEngine::FindTexture (const char* iName, bool regionOnly)
{
  csTextureWrapper* wr;
  if (regionOnly && region)
    wr = (csTextureWrapper*)FindObjectInRegion (region, *textures, iName);
  else
    wr = textures->FindByName (iName);
  return &wr->scfiTextureWrapper;
}

csTextureWrapper* csEngine::FindCsTexture (const char* iName, bool regionOnly)
{
  csTextureWrapper* wr;
  if (regionOnly && region)
    wr = (csTextureWrapper*)FindObjectInRegion (region, *textures, iName);
  else
    wr = textures->FindByName (iName);
  return wr;
}

iCameraPosition* csEngine::FindCameraPosition (const char* iName, bool regionOnly)
{
  csCameraPosition* wr;
  if (regionOnly && region)
    wr = (csCameraPosition*)FindObjectInRegion (region, camera_positions, iName);
  else
    wr = (csCameraPosition*)camera_positions.FindByName (iName);
  return &wr->scfiCameraPosition;
}

iView* csEngine::CreateView (iGraphics3D* g3d)
{
  csView* view = new csView (this, g3d);
  iView* iview = QUERY_INTERFACE (view, iView);
  return iview;
}

iStatLight* csEngine::CreateLight (const csVector3& pos, float radius,
  	const csColor& color, bool pseudoDyn)
{
  csStatLight* light = new csStatLight (pos.x, pos.y, pos.z, radius,
  	color.red, color.green, color.blue, pseudoDyn);
  return QUERY_INTERFACE (light, iStatLight);
}

iDynLight* csEngine::CreateDynLight (const csVector3& pos, float radius,
  	const csColor& color)
{
  csDynLight* light = new csDynLight (pos.x, pos.y, pos.z, radius,
  	color.red, color.green, color.blue);
  AddDynLight (light);
  return QUERY_INTERFACE (light, iDynLight);
}

void csEngine::RemoveDynLight (iDynLight* light)
{
  RemoveDynLight (light->GetPrivateObject ());
}

iTransformationManager* csEngine::GetTransformationManager ()
{
  return QUERY_INTERFACE (&tr_manager, iTransformationManager);
}

//----------------Begin-Multi-Context-Support------------------------------

void csEngine::Resize ()
{
  frame_width = G3D->GetWidth ();
  frame_height = G3D->GetHeight ();
  // Reset the culler.
  SetCuller (GetCuller ());
}

csEngine::csEngineState::csEngineState (csEngine *e)
{
  engine    = e;
  c_buffer = e->c_buffer;
  cbufcube = e->cbufcube;
  covtree  = e->covtree;
  quad3d   = e->quad3d;
  G2D      = e->G2D;
  G3D      = e->G3D;
  resize   = false;
}

csEngine::csEngineState::~csEngineState ()
{
  if (engine->G2D == G2D)
  {
    engine->G3D->DecRef ();
    engine->G3D      = NULL;
    engine->G2D      = NULL;
    engine->c_buffer = NULL;
    engine->quad3d   = NULL;
    engine->covtree  = NULL;
    engine->cbufcube = NULL;
  }
  delete c_buffer;
  delete cbufcube;
  delete quad3d;
  delete covtree;
}

void csEngine::csEngineState::Activate ()
{
  engine->c_buffer     = c_buffer;
  engine->cbufcube     = cbufcube;
  engine->quad3d       = quad3d;
  engine->covtree      = covtree;
  engine->frame_width  = G3D->GetWidth ();
  engine->frame_height = G3D->GetHeight ();

  if (resize)
  {
    engine->Resize ();

    c_buffer = engine->c_buffer;
    cbufcube = engine->cbufcube;
    quad3d   = engine->quad3d;
    covtree  = engine->covtree;
    resize   = false;
  }
}

void csEngine::csEngineStateVector::Close (iGraphics2D *g2d)
{
  // Hack-- with the glide back buffer implementations of procedural textures
  // circumstances are that many G3D can be associated with one G2D.
  // It is impossible to tell which is which so destroy them all, and rely on
  // regeneration the next time the context is set to the surviving G3Ds associated
  // with this G2D
  for (int i = 0; i < Length (); i++)
    if (((csEngineState*)root [i])->G2D == g2d)
    {
      //Delete (i);
      break;
    }
}

void csEngine::csEngineStateVector::Resize (iGraphics2D *g2d)
{
  // With the glide back buffer implementation of procedural textures
  // circumstances are that many G3D can be associated with one G2D, so
  // we test for width and height also.
  for (int i = 0; i < Length (); i++)
  {
    csEngineState *state = (csEngineState*)root [i];
    if (state->G2D == g2d)
      if ((state->G2D->GetWidth() == state->G3D->GetWidth ()) &&
	  (state->G2D->GetHeight() == state->G3D->GetHeight ()))
	  ((csEngineState*)root [i])->resize = true;
  }
}

void csEngine::SetContext (iGraphics3D* g3d)
{
  G2D = g3d->GetDriver2D ();
  if (g3d != G3D)
  {
    if (G3D) G3D->DecRef ();
    G3D = g3d;
    if (!engine_states)
    {
      engine_states = new csEngineStateVector();
      Resize ();
      engine_states->Push (new csEngineState (this));
    }
    else
    {
      int idg3d = engine_states->FindKey (g3d);
      if (idg3d < 0)
      {
	int c = GetCuller ();
	// Null out the culler which belongs to another state so its not deleted.
	c_buffer = NULL;
	cbufcube = NULL;
	covtree  = NULL;
	quad3d   = NULL;
	frame_width = G3D->GetWidth ();
	frame_height = G3D->GetHeight ();
	cbufcube = new csCBufferCube (1024);
	SetCuller (c);
	engine_states->Push (new csEngineState (this));
      }
      else
      {
	csEngineState *state = (csEngineState *)engine_states->Get (idg3d);
	state->Activate ();
      }
    }
    G3D->IncRef ();
  }
}

//-------------------End-Multi-Context-Support--------------------------------
