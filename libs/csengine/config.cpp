/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
  
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
#include "csengine/engine.h"
#include "csengine/sector.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"

SCF_IMPLEMENT_EMBEDDED_IBASE (csEngineConfig)
  SCF_IMPLEMENTS_INTERFACE (iConfig)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

static const csOptionDescription config_options [] =
{
  {  0, "fov",      "Field of Vision", CSVAR_LONG },
  {  1, "rad",      "Pseudo-radiosity system", CSVAR_BOOL },
  {  2, "cosfact",  "Cosinus factor for lighting", CSVAR_FLOAT },
  {  3, "reflect",  "Max number of reflections for radiosity", CSVAR_LONG },
  {  4, "recalc",   "Force/inhibit recalculation of all cached information",
                     CSVAR_BOOL },
  {  5, "relight",  "Force/inhibit recalculation of lightmaps", CSVAR_BOOL },
  {  6, "lightqual","Lighting quality", CSVAR_LONG },
  {  7, "revis",    "Force/inhibit recalculation of visibility data",
                     CSVAR_BOOL },
  {  8, "radstep",  "Enable radiosity step-by-step", CSVAR_BOOL }
};
const int NUM_OPTIONS =
  (sizeof (config_options) / sizeof (config_options [0]));

static bool config_relight ()
{
  return csEngine::lightcache_mode == CS_ENGINE_CACHE_WRITE;
}

static void config_relight (bool flag)
{
  if (flag)
    csEngine::lightcache_mode = CS_ENGINE_CACHE_WRITE;
  else
    csEngine::lightcache_mode = CS_ENGINE_CACHE_READ;
}

static bool config_revis () { return csEngine::do_force_revis; }
static void config_revis (bool flag)
{
  csEngine::do_force_revis = flag;
}

static bool config_recalc () { return config_relight() || config_revis(); }
static void config_recalc (bool flag)
{
  config_relight (flag);
  config_revis   (flag);
}

bool csEngineConfig::SetOption (int id, csVariant* value)
{
  switch (id)
  {
    case 0:  csCamera::SetDefaultFOV (value->v.l, scfParent->G3D->GetWidth ());
             break;
    case 1:  csSector::do_radiosity = value->v.b; break;
    case 2:  csPolyTexture::cfg_cosinus_factor = value->v.f; break;
    case 3:  csSector::cfg_reflections = value->v.l; break;
    case 4:  config_recalc  (value->v.b); break;
    case 5:  config_relight (value->v.b); break;
    case 6:  csEngine::lightmap_quality = value->v.l; break;
    case 7:  config_revis   (value->v.b); break;
    case 8:  csEngine::do_rad_debug = value->v.b; break;
    default: return false;
  }
  return true;
}

bool csEngineConfig::GetOption (int id, csVariant* value)
{
  value->type = config_options[id].type;
  switch (id)
  {
    case 0:  value->v.l = csCamera::GetDefaultFOV (); break;
    case 1:  value->v.b = csSector::do_radiosity; break;
    case 2:  value->v.f = csPolyTexture::cfg_cosinus_factor; break;
    case 3:  value->v.l = csSector::cfg_reflections; break;
    case 4:  value->v.b = config_recalc (); break;
    case 5:  value->v.b = config_relight(); break;
    case 6:  value->v.l = csEngine::lightmap_quality; break;
    case 7:  value->v.b = config_revis  (); break;
    case 8:  value->v.b = csEngine::do_rad_debug; break;
    default: return false;
  }
  return true;
}

bool csEngineConfig::GetOptionDescription (
  int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS) return false;
  *option = config_options[idx];
  return true;
}

