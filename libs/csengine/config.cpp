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
#include "csengine/config.h"
#include "csengine/world.h"
#include "csengine/sector.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"

//---------------------------------------------------------------------------

IMPLEMENT_UNKNOWN_NODELETE (csEngineConfig)

BEGIN_INTERFACE_TABLE (csEngineConfig)
	IMPLEMENTS_COMPOSITE_INTERFACE (EngineConfig)
END_INTERFACE_TABLE()

csOptionDescription csEngineConfig::config_options[] =
{
  { 0, "fov", "Field of Vision", CSVAR_LONG },
  { 1, "rad", "Pseudo-radiosity system", CSVAR_BOOL },
  { 2, "accthg", "Accurate shadows for things", CSVAR_BOOL },
  { 3, "cosfact", "Cosinus factor for lighting", CSVAR_FLOAT },
  { 4, "reflect", "Max number of reflections for radiosity", CSVAR_LONG },
  { 5, "recalc", "Force recalculation of lightmaps", CSVAR_CMD },
  { 6, "inhrecalc", "Inhibit automatic recalculation of lightmaps", CSVAR_CMD },
  { 7, "cache", "Enabling caching of generated lightmaps", CSVAR_BOOL },
};

#define NUM_OPTIONS 8

bool csEngineConfig::SetOption (int id, csVariant* value)
{
  switch (id)
  {
    case 0: csCamera::SetDefaultFOV (value->v.lVal); break;
    case 1: csSector::do_radiosity = value->v.bVal; break;
    case 2: csPolyTexture::do_accurate_things = value->v.bVal; break;
    case 3: csPolyTexture::cfg_cosinus_factor = value->v.fVal; break;
    case 4: csSector::cfg_reflections = value->v.lVal; break;
    case 5: csPolygon3D::do_force_recalc = value->v.bVal; break;
    case 6: csPolygon3D::do_not_force_recalc = value->v.bVal; break;
    case 7: csPolygon3D::do_cache_lightmaps = value->v.bVal; break;
    default: return false;
  }
  return true;
}

bool csEngineConfig::GetOption (int id, csVariant* value)
{
  value->type = config_options[id].type;
  switch (id)
  {
    case 0: value->v.lVal = csCamera::GetDefaultFOV (); break;
    case 1: value->v.bVal = csSector::do_radiosity; break;
    case 2: value->v.bVal = csPolyTexture::do_accurate_things; break;
    case 3: value->v.fVal = csPolyTexture::cfg_cosinus_factor; break;
    case 4: value->v.lVal = csSector::cfg_reflections; break;
    case 5: value->v.bVal = csPolygon3D::do_force_recalc; break;
    case 6: value->v.bVal = csPolygon3D::do_not_force_recalc; break;
    case 7: value->v.bVal = csPolygon3D::do_cache_lightmaps; break;
    default: return false;
  }
  return true;
}

int csEngineConfig::GetNumberOptions ()
{
  return NUM_OPTIONS;
}

bool csEngineConfig::GetOptionDescription (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS) return false;
  *option = config_options[idx];
  return true;
}

IMPLEMENT_COMPOSITE_UNKNOWN (csEngineConfig, EngineConfig)

STDMETHODIMP IEngineConfig::SetOption (int id, csVariant* value)
{
  METHOD_PROLOGUE (csEngineConfig, EngineConfig);
  if (!pThis->SetOption (id, value)) return E_FAIL;
  return S_OK;
}

STDMETHODIMP IEngineConfig::GetOption (int id, csVariant* value)
{
  METHOD_PROLOGUE (csEngineConfig, EngineConfig);
  if (!pThis->GetOption (id, value)) return E_FAIL;
  return S_OK;
}

STDMETHODIMP IEngineConfig::GetNumberOptions (int& num)
{
  METHOD_PROLOGUE (csEngineConfig, EngineConfig);
  num = pThis->GetNumberOptions ();
  return S_OK;
}

STDMETHODIMP IEngineConfig::GetOptionDescription (int idx, csOptionDescription* option)
{
  METHOD_PROLOGUE (csEngineConfig, EngineConfig);
  if (!pThis->GetOptionDescription (idx, option)) return E_FAIL;
  return S_OK;
}

//---------------------------------------------------------------------------

