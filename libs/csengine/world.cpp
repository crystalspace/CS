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
#include "csengine/sysitf.h"
#include "csengine/world.h"
#include "csengine/halo.h"
#include "csengine/camera.h"
#include "csengine/light/light.h"
#include "csengine/light/dynlight.h"
#include "csengine/polygon/polyplan.h"
#include "csengine/polygon/polygon.h"
#include "csengine/polygon/polytext.h"
#include "csengine/objects/thingtpl.h"
#include "csengine/objects/thing.h"
#include "csengine/objects/cssprite.h"
#include "csengine/basic/cscoll.h"
#include "csengine/sector.h"
#include "csengine/library.h"
#include "csengine/texture.h"
#include "csengine/light/lghtmap.h"
#include "csengine/wirefrm.h"
#include "csengine/stats.h"
#include "csengine/config.h"
#include "csengine/scripts/primscri.h"
#include "csengine/scripts/intscri.h"
#include "csengine/scripts/csscript.h"
#include "csengine/scripts/objtrig.h"
#include "csgeom/fastsqrt.h"
#include "csobject/nameobj.h"
#include "csutil/archive.h"
#include "csutil/inifile.h"
#include "csgfxldr/csimage.h"
#include "ihalo.h"
#include "itxtmgr.h"
#include "igraph3d.h"

//---------------------------------------------------------------------------

float csWorld::shift_x;
float csWorld::shift_y;
int csWorld::frame_width;
int csWorld::frame_height;
ISystem* csWorld::isys = NULL;
csWorld* csWorld::current_world = NULL;

CSOBJTYPE_IMPL(csWorld,csObject);

csWorld::csWorld () : csObject()
{
  first_dyn_lights = NULL;
  world_file = NULL;
  wf = NULL;
  map_mode = MAP_OFF;
  start_sector = NULL;
  start_vec.x = start_vec.y = start_vec.z = 0;
  piHR = NULL;
  textures = NULL;
  CHK (cfg_engine = new csEngineConfig ());
  BuildSqrtTable ();
}

csWorld::~csWorld ()
{
  Clear ();
  CHK (delete wf);
  if (piHR) { FINAL_RELEASE (piHR); piHR = NULL; }
  CHK (delete textures);
  CHK (delete cfg_engine);
}

void csWorld::Clear ()
{
  sectors.DeleteAll ();
  libraries.DeleteAll ();
  planes.DeleteAll ();
  scripts.DeleteAll ();
  collections.DeleteAll ();
  sprite_templates.DeleteAll ();
  thing_templates.DeleteAll ();
  sprites.DeleteAll ();

  while (first_dyn_lights)
  {
    csDynLight* dyn = first_dyn_lights->GetNext ();
    CHK (delete first_dyn_lights);
    first_dyn_lights = dyn;
  }
  CHK (delete world_file); world_file = NULL;
  CHK (delete [] start_sector); start_sector = NULL;
  CHK (delete textures); textures = NULL;
  CHK (textures = new csTextureList ());
}

IConfig* csWorld::GetEngineConfigCOM ()
{
  return GetIConfigFromcsEngineConfig (cfg_engine);
}

csSpriteTemplate* csWorld::GetSpriteTemplate (const char* name, bool use_libs)
{
  // if use_libs == true we should support qualified names (like standard.bullet).
  // with the library first and then the name of the object.
  int sn = sprite_templates.Length ();
  while (sn > 0)
  {
    sn--;
    csSpriteTemplate* s = (csSpriteTemplate*)sprite_templates[sn];
    if (!strcmp (name, csNameObject::GetName(*s))) return s;
  }

  if (use_libs)
  {
    int nl = libraries.Length ();
    while (nl > 0)
    {
      nl--;
      csLibrary* lib = (csLibrary*)libraries[nl];
      csSpriteTemplate* s = (csSpriteTemplate*)lib->sprite_templates.FindByName (name);
      if (s) return s;
    }
  }

  return NULL;
}

csThingTemplate* csWorld::GetThingTemplate (const char* name, bool use_libs)
{
  int tn = thing_templates.Length ();
  while (tn > 0)
  {
    tn--;
    csThingTemplate* s = (csThingTemplate*)thing_templates[tn];
    if (!strcmp (name, csNameObject::GetName(*s))) return s;
  }

  if (use_libs)
  {
    int nl = libraries.Length ();
    while (nl > 0)
    {
      nl--;
      csLibrary* lib = (csLibrary*)libraries[nl];
      csThingTemplate* s = (csThingTemplate*)lib->thing_templates.FindByName (name);
      if (s) return s;
    }
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

csScript* csWorld::NewScript (LanguageLayer* layer, char* name, char* params)
{
  if (!layer)
    return NULL;

  csScript* s = NULL;
  char* par = strchr (params, ':');
  if (!par)
  {
    CsPrintf (MSG_FATAL_ERROR, "Missing language qualifier for script '%s'!\n", name);
    fatal_exit (0, false);
  }

  if (scripts.FindByName (name))
  {
    CsPrintf (MSG_FATAL_ERROR, "A script with the name '%s' is already defined!\n", name);
    fatal_exit (0, false);
  }

  // All different script languages should be recognized here.
  if (!strncmp (params, "prim", (int)(par-params)))
  {
    CHK (PrimScript* sc = new PrimScript (layer));
    csNameObject::AddName(*sc,name);
    par++;
    sc->load (&par);
    s = (csScript*)sc;
  }
  else if (!strncmp (params, "int", (int)(par-params)))
  {
    CHK (IntScript* sc = new IntScript (layer));
    csNameObject::AddName(*sc,name);
    par++;
    sc->load (par);
    s = (csScript*)sc;
  }
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "Unknown script qualifier for script '%s'!\n", name);
    fatal_exit (0, false);
    return NULL;
  }

  s->prepare ();
  scripts.Push (s);

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

void csWorld::TriggerActivate (csCamera& c)
{
  csVector3 where;
  where = c.This2Other (3*VEC_FORWARD);
  csPolygon3D* p = c.GetHit (where);
  if (p)
  {
    CsPrintf (MSG_CONSOLE, "Activate polygon '%s' ", csNameObject::GetName(*p));
    csPolygonSet* ob = (csPolygonSet*)(p->GetParent ());
    CsPrintf (MSG_CONSOLE, "in set '%s'\n", csNameObject::GetName(*ob));
    csObjectTrigger::DoActivateTriggers(*ob);
  }
}

bool csWorld::Initialize (ISystem* sys, IGraphicsInfo* ginfo, csIniFile* config)
{
  ginfo->GetWidth (frame_width);
  ginfo->GetHeight (frame_height);
  shift_x = (float)(frame_width/2);
  shift_y = (float)(frame_height/2);
  isys = sys;
  current_world = this;

  CHK (textures = new csTextureList ());
  ReadConfig (config);

  if (csCamera::aspect == 0) csCamera::aspect = frame_height;
  csCamera::inv_aspect = 1./csCamera::aspect;

  StartWorld ();

  return true;
}

bool csWorld::Prepare (IGraphics3D* g3d)
{
  ITextureManager* txtmgr;
  g3d->GetTextureManager (&txtmgr);
  txtmgr->Initialize ();

  int i;
  // First register all textures to the texture manager.
  for (i = 0 ; i < textures->GetNumTextures () ; i++)
  {
    csTextureHandle* th = textures->GetTextureMM (i);
    ITextureHandle* handle;
    txtmgr->RegisterTexture (GetIImageFileFromImageFile (th->GetImageFile ()),
      &handle, th->for_3d, th->for_2d);
    th->SetTextureHandle (handle);
  }

  // Prepare all the textures.
  txtmgr->Prepare ();

  // Now precalculate some stuff for all loaded polygons.
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* s = (csSector*)sectors[i];
    s->Prepare ();
  }

  // The images are no longer needed by the 3D engine.
  txtmgr->FreeImages ();

  g3d->ClearCache ();
  ShineLights ();
  CreateLightmaps (g3d);

#if defined(OS_NEXT)
// FIXME: NextStep: Multiple Inheritence broken (IID_IHaloRasterizer)
piHR = 0;
#else
  if (!SUCCEEDED (g3d->QueryInterface(IID_IHaloRasterizer, (void**)&piHR)))
    piHR = NULL;
#endif

  return true;
}

Archive* csWorld::GetWorldFile ()
{
  if (!world_file) CHKB (world_file = new Archive ("precalc.zip"));
  return world_file;
}

Archive* csWorld::OpenWorldFile (const char* filename)
{
  CloseWorldFile ();
  CHK (world_file = new Archive (filename));
  return world_file;
}

void csWorld::CloseWorldFile ()
{
  CHK (delete world_file);
}


void csWorld::ShineLights ()
{
  if (!csPolygon3D::do_not_force_recalc)
  {
    // If recalculation is not forced then we check if the 'precalc_info'
    // file exists in the archive. If not then we will recalculate in any
    // case. If the file exists but is not valid (some flags are different)
    // then we recalculate again.
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
    bool force = false;
    char* reason = NULL;

    Archive* ar = GetWorldFile ();
    char* data = ar->read ("precalc_info");
    if (data)
    {
      char* p1, * p2;
      int i;
      float f;
      p1 = data; p2 = strchr (p1, '='); sscanf (p2+1, "%d", &i); if (i != current.lm_version) { force = true; reason = "lightmap format changed"; }
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
      }}}}}}}}}}
      CHK (delete [] data);
    }
    else { force = true; reason = "no 'precalc_info' found"; }

    if (force)
    {
      CHK (data = new char [5000]);
      sprintf (data, "LMVERSION=%d\nNORMALLIGHTLEVEL=%d\nAMBIENT_WHITE=%d\nAMBIENT_RED=%d\nAMBIENT_GREEN=%d\n\
AMBIENT_BLUE=%d\nREFLECT=%d\nRADIOSITY=%d\nACCURATE_THINGS=%d\nCOSINUS_FACTOR=%f\nLIGHTMAP_SIZE=%d\n",
        current.lm_version, current.normal_light_level, current.ambient_white, current.ambient_red, current.ambient_green,
        current.ambient_blue, current.reflect, current.radiosity, current.accurate_things, current.cosinus_factor,
        current.lightmap_size);
      void* entry = ar->new_file ("precalc_info", strlen (data));
      ar->write (entry, data, strlen (data));
      CHK (delete [] data);
      CsPrintf (MSG_INITIALIZATION, "Lightmap data in archive is not up to date (reason: %s).\n", reason);
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

  int sn, total, last_printed=0, complete;
  total = sn = sectors.Length ();
  CsPrintf (MSG_INITIALIZATION, "Initializing lightmaps (%d sectors total):\n  ",total);
  CsPrintf (MSG_TICKER, "begin");

  for(last_printed=0;sn-->0;)
  {
    csSector* s = (csSector*)sectors[sn];
    s->InitLightmaps ();
    complete=100-100*sn/total;
    for(;last_printed<complete;last_printed+=2)
    {
      if(last_printed%10)
        CsPrintf(MSG_INITIALIZATION, ".");
      else
        CsPrintf(MSG_INITIALIZATION, "%d%%",last_printed);
    }
  }

  total = sn = sectors.Length ();

  CsPrintf(MSG_INITIALIZATION, "100%%\n");
  CsPrintf(MSG_INITIALIZATION, "Shining lights (%d sectors total):\n  ",total);
  for(last_printed=0;sn-->0;)
  {
    csSector* s = (csSector*)sectors[sn];
    s->ShineLights ();

    complete=100-100*sn/total;
    for(;last_printed<complete;last_printed+=2)
    {
      if(last_printed%10)
        CsPrintf(MSG_INITIALIZATION, ".");
      else
        CsPrintf(MSG_INITIALIZATION, "%d%%",last_printed);
    }
  }

  total = sn = sectors.Length ();

  CsPrintf(MSG_INITIALIZATION, "100%%\n");
  CsPrintf(MSG_INITIALIZATION, "Caching lightmaps (%d sectors total):\n  ",total);
  for(last_printed=0;sn-->0;)
  {
    csSector* s = (csSector*)sectors[sn];
    s->CacheLightmaps ();
    complete=100-100*sn/total;
    for(;last_printed<complete;last_printed+=2)
    {
      if(last_printed%10)
        CsPrintf(MSG_INITIALIZATION, ".");
      else
        CsPrintf(MSG_INITIALIZATION, "%d%%",last_printed);
    }
  }
  CsPrintf (MSG_INITIALIZATION,"100%%\n");
  CsPrintf (MSG_TICKER, "end");

  csPolygonSet::current_light_frame_number++;

  Archive* ar = GetWorldFile ();
  if (ar)
    if (!ar->write_archive ())
      CsPrintf (MSG_WARNING, "WARNING: error updating lighttable cache in archive %s!\n", ar->GetFilename ());
}

void csWorld::CreateLightmaps (IGraphics3D* g3d)
{
  int sn = sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)sectors[sn];
    s->CreateLightmaps (g3d);
  }
}

void csWorld::StartWorld ()
{
  Clear ();
}

void csWorld::Draw (IGraphics3D* g3d, csCamera* c, csClipper* view)
{
  Stats::polygons_considered = 0;
  Stats::polygons_drawn = 0;
  Stats::portals_drawn = 0;

  IGraphics2D* g2d;
  g3d->Get2dDriver (&g2d);
  csRenderView rview (*c, view, g3d, g2d);
  rview.clip_plane.Set (0, 0, 1, -1);   //@@@CHECK!!!

  if (map_mode != MAP_OFF) wf->GetWireframe ()->Clear ();
  csSector* s = c->GetSector ();
  s->Draw (rview);

  // draw all halos on the screen
  IHaloRasterizer* piHR = NULL;
  bool supports_halos;

#if defined(OS_NEXT)
// FIXME: NextStep: Multiple Inheritence broken (IID_IHaloRasterizer)
supports_halos = false;
#else
  supports_halos = g3d->QueryInterface (IID_IHaloRasterizer, (void**)&piHR) == S_OK ? true : false;
#endif
  
  csHaloInformation* pinfo;
  
  for (int cntHalos = 0; cntHalos <halos.Length(); cntHalos++)
  {
    HRESULT hres = S_FALSE;

    pinfo = (csHaloInformation*)halos.Get(cntHalos);

    float hintensity = pinfo->pLight->GetHaloIntensity ();

    if (pinfo->pLight->GetReferenceCount () == 0)
    {      
      hintensity -= .15f;

      // this halo is completely invisible. kill it.
      if (hintensity <= 0)
      {
        halos.Delete(cntHalos);
        pinfo->pLight->SetHaloInQueue (false);
        
        piHR->DestroyHalo(pinfo->haloinfo);
        delete pinfo;

        cntHalos--;
        continue;
      }

      pinfo->pLight->SetHaloIntensity (hintensity);
    }
    else
    {

      if (hintensity < pinfo->pLight->GetHaloMaxIntensity ())
        hintensity += .15f;
      
      if (hintensity > pinfo->pLight->GetHaloMaxIntensity ())
        hintensity = pinfo->pLight->GetHaloMaxIntensity ();

      pinfo->pLight->SetHaloIntensity (hintensity);
    }
  
    if (supports_halos)
    {
      // project the halo.
      pinfo->v = rview.World2Camera (pinfo->pLight->GetCenter ());
      
      if (pinfo->v.z > SMALL_Z)
      {
        float iz = csCamera::aspect/pinfo->v.z;
        pinfo->v.x = pinfo->v.x * iz + shift_x;
        pinfo->v.y = frame_height - 1 - (pinfo->v.y * iz + shift_y);
        
        pinfo->intensity = pinfo->pLight->GetHaloIntensity ();

        hres = piHR->DrawHalo(&pinfo->v, pinfo->intensity, pinfo->haloinfo);
      }
    }

    // was this halo actually drawn?
    if (hres == S_FALSE)
      pinfo->pLight->RemoveReference ();
  }   
  if (piHR) piHR->Release();

  // DAN: this is commneted out until we figure out where light selection logic should go.
/*  if (g3d->do_light_frust && g3d->selected_light)
    g3d->selected_light->dump_frustrum (*c); */
}

void csWorld::AddHalo (csHaloInformation* pinfo)
{
  pinfo->pLight->AddReference ();
  pinfo->pLight->SetHaloInQueue (true);
  halos.Push(pinfo);
}

bool csWorld::HasHalo (csLight* pLight)
{
  for (int i=0; i < halos.Length(); i++)
  {
    if (((csHaloInformation*)halos[i])->pLight == pLight)
      return true;
  }

  return false;
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

void csWorld::AdvanceSpriteFrames (long current_time)
{
  int i;
  for (i = 0 ; i < sprites.Length () ; i++)
  {
    csSprite3D* s = (csSprite3D*)sprites[i];
    s->NextFrame (current_time);
  }
}

void csWorld::ReadConfig (csIniFile* config)
{
  if (!config) return;
  csPolygon3D::def_mipmap_size = config->GetInt ("TextureMapper", "LIGHTMAP_SIZE", 16);
  csLight::ambient_red = config->GetInt ("World", "AMBIENT_RED", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_green = config->GetInt ("World", "AMBIENT_GREEN", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_blue = config->GetInt ("World", "AMBIENT_BLUE", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_white = config->GetInt ("World", "AMBIENT_WHITE", DEFAULT_LIGHT_LEVEL);
  csLight::ambient_red += csLight::ambient_white;
  csLight::ambient_green += csLight::ambient_white;
  csLight::ambient_blue += csLight::ambient_white;
  csSector::cfg_reflections = config->GetInt ("Lighting", "REFLECT", csSector::cfg_reflections);
  csSector::do_radiosity = config->GetYesNo ("Lighting", "RADIOSITY", csSector::do_radiosity);
  csPolyTexture::do_accurate_things = config->GetYesNo ("Lighting", "ACCURATE_THINGS", csPolyTexture::do_accurate_things);
  csPolyTexture::cfg_cosinus_factor = config->GetFloat ("Lighting", "COSINUS_FACTOR", csPolyTexture::cfg_cosinus_factor);
  //@@@
  //Textures::Gamma = config->GetFloat ("TextureMapper", "GAMMA", 1.0);
}

void csWorld::SetHilight (csPolygon3D* hi)
{
  csPolygon3D::hilight = hi;
}

csPolygon3D* csWorld::GetHilight ()
{
  return csPolygon3D::hilight;
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

//---------------------------------------------------------------------------

