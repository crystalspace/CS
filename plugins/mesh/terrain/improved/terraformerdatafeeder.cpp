/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#include "iterrain/terraincell.h"

#include "csgeom/csrect.h"

#include "igraphic/image.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "csutil/refarr.h"
#include "csutil/dirtyaccessarray.h"

#include "csgfx/imagemanipulate.h"

#include "iengine/material.h"

#include "iengine/engine.h"

#include "imap/loader.h"

#include "terraformerdatafeeder.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainTerraFormerDataFeeder)

csTerrainTerraFormerDataFeeder::csTerrainTerraFormerDataFeeder (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTerrainTerraFormerDataFeeder::~csTerrainTerraFormerDataFeeder ()
{
}

bool csTerrainTerraFormerDataFeeder::PreLoad (iTerrainCell* cell)
{
  return true;
}

bool csTerrainTerraFormerDataFeeder::Load (iTerrainCell* cell)
{
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
	object_reg, "crystalspace.shared.stringset", iStringSet);

  int width = cell->GetGridWidth ();
  int height = cell->GetGridHeight ();

  csRef<iTerraSampler> height_sampler = former->GetSampler (sample_region, 
	width, height);

  if (!height_sampler) return false;

  const float* height_data = height_sampler->SampleFloat(
    strings->Request("heights"));

  csLockedHeightData data = cell->LockHeightData (csRect(0, 0, width, height));

  for (int y = 0; y < height; ++y)
  {
    memcpy (data.data, height_data, width * sizeof(float));

    data.data += data.pitch;
    height_data += width;
  }

  cell->UnlockHeightData ();
  
  int mwidth = cell->GetMaterialMapWidth ();
  int mheight = cell->GetMaterialMapHeight ();

  csRef<iTerraSampler> material_sampler = former->GetSampler (sample_region, 
	mwidth, mheight);

  if (!material_sampler) return false;

  const int* material_data = material_sampler->SampleInteger(
    strings->Request("materialmap"));

  csLockedMaterialMap mdata = cell->LockMaterialMap (csRect (0, 0, mwidth,
    mheight));

  for (int y = 0; y < mheight; ++y)
  {
    unsigned char* dest_data = mdata.data;

    for (int x = 0; x < mwidth; ++x)
    {
      *dest_data++ = (unsigned char)*material_data++;
    }

    mdata.data += mdata.pitch;
  } 
  
  cell->UnlockMaterialMap ();

  return true;
}

bool csTerrainTerraFormerDataFeeder::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  return true;
}

void csTerrainTerraFormerDataFeeder::SetParam(const char* param, const char* value)
{
  if (!strcmp (param, "terraformer"))
  {
    former = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, value, iTerraFormer);
  }
  else if (!strcmp (param, "sampleregion min"))
  {
    float x, y;

    x = (float)strtod (value, (char**)&value);
    y = (float)strtod (value, (char**)&value);

    sample_region.SetMin (0, x);
	sample_region.SetMin (1, y);
  }
  else if (!strcmp (param, "sampleregion max"))
  {
    float x, y;

    x = (float)strtod (value, (char**)&value);
    y = (float)strtod (value, (char**)&value);
    
	sample_region.SetMax (0, x);
	sample_region.SetMax (1, y);
  }
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
