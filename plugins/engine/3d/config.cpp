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
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/sector.h"

SCF_IMPLEMENT_EMBEDDED_IBASE(csEngineConfig)
  SCF_IMPLEMENTS_INTERFACE(iPluginConfig)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

static const csOptionDescription
  config_options[] =
{
  { 0, "fov", "Field of Vision", CSVAR_LONG },
  { 1, "rad", "Pseudo-radiosity system", CSVAR_BOOL },
  { 2, "reflect", "Max number of reflections for radiosity", CSVAR_LONG },
  { 3, "relight", "Force/inhibit recalculation of lightmaps", CSVAR_BOOL },
  { 4, "radstep", "Enable radiosity step-by-step", CSVAR_BOOL },
  { 5, "renderloop", "Override the default render loop", CSVAR_STRING },
};
const int NUM_OPTIONS =
  (
    sizeof (config_options) /
    sizeof (config_options[0])
  );

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

bool csEngineConfig::SetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:
      csCamera::SetDefaultFOV (value->GetLong (), scfParent->G3D->GetWidth ());
      break;
    case 1:
      csSector::do_radiosity = value->GetBool ();
      break;
    case 2:
      csSector::cfg_reflections = value->GetLong ();
      break;
    case 3:
      config_relight (value->GetBool ());
      break;
    case 4:
      csEngine::do_rad_debug = value->GetBool ();
      break;
    case 5:
      scfParent->LoadDefaultRenderLoop (value->GetString ());
    default:
      return false;
  }

  return true;
}

bool csEngineConfig::GetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:   value->SetLong (csCamera::GetDefaultFOV ()); break;
    case 1:   value->SetBool (csSector::do_radiosity); break;
    case 2:   value->SetLong (csSector::cfg_reflections); break;
    case 3:   value->SetBool (config_relight ()); break;
    case 4:   value->SetBool (csEngine::do_rad_debug); break;
    case 5:   value->SetString (""); break; // @@@
    default:  return false;
  }

  return true;
}

bool csEngineConfig::GetOptionDescription (
  int idx,
  csOptionDescription *option)
{
  if (idx < 0 || idx >= NUM_OPTIONS) return false;
  *option = config_options[idx];
  return true;
}

