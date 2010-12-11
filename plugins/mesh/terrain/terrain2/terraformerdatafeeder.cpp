/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#include "csgeom/csrect.h"
#include "csgfx/imagemanipulate.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "csutil/scanstr.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "igraphic/image.h"
#include "imap/loader.h"
#include "imesh/terrain2.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "terraformerdatafeeder.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{
struct TerraFormerFeederProperties : public 
  scfImplementation1<TerraFormerFeederProperties,
                     iTerrainCellFeederProperties>
{
  TerraFormerFeederProperties (iObjectRegistry* objectReg)
    : scfImplementationType (this), objectReg (objectReg)
  {
  }

  TerraFormerFeederProperties (TerraFormerFeederProperties& other)
    : scfImplementationType (this), objectReg (other.objectReg), 
    terraFormer (other.terraFormer), samplerRegion (other.samplerRegion)
  {
  }

  void SetHeightmapSource (const char* source, const char* format) 
  {
    terraFormer = csQueryRegistryTagInterface<iTerraFormer> (objectReg, source);
  }

  void SetNormalMapSource (const char* source)
  {}

  void SetMaterialMapSource (const char* source)
  {}

  void SetHeightOffset (float offset) 
  {}

  void AddAlphaMap (const char* material, const char* alphaMapSource)
  {}

  void SetParameter (const char* param, const char* value) 
  {
    if (strcmp (param, "terraformer") == 0)
    {
      SetHeightmapSource (value, 0); 
    }
    else if (strcmp (param, "sampleregion min") == 0)
    {
      float x = 0, y = 0;

      csScanStr (value, "%f %f", &x, &y);

      samplerRegion.SetMin (0, x);
      samplerRegion.SetMin (1, y);
    }
    else if (strcmp (param, "sampleregion max") == 0)
    {
      float x = 1.0f, y = 1.0f;

      csScanStr (value, "%f %f", &x, &y);

      samplerRegion.SetMax (0, x);
      samplerRegion.SetMax (1, y);
    }
  }

  size_t GetParameterCount() { return 3; }

  const char* GetParameterName (size_t index)
  {
    switch (index)
    {
      case 0: return "terraformer";
      case 1: return "sampleregion min";
      case 2: return "sampleregion max";
      default: return 0;
    }
  }

  const char* GetParameterValue (size_t index)
  { return GetParameterValue (GetParameterName (index)); }

  const char* GetParameterValue (const char* name)
  {
    // @@@ Not nice
    static char scratch[32];

    if (strcasecmp (name, "terraformer") == 0)
    {
      csRef<iObjectRegistryIterator> it (objectReg->Get());
      while (it->HasNext())
      {
	iBase* obj = it->Next();
	if (obj == (iBase*)terraFormer)
	  return it->GetCurrentTag();
      }
      return 0;
    }
    else if (strcasecmp (name, "sampleregion min") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%f %f",
	samplerRegion.MinX(), samplerRegion.MinY());
      return scratch;
    }
    else if (strcasecmp (name, "sampleregion max") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%f %f",
	samplerRegion.MaxX(), samplerRegion.MaxY());
      return scratch;
    }
    return 0;
  }

  size_t GetAlphaMapCount() { return 0; }

  const char* GetAlphaMapMaterial (size_t index) { return 0; }

  const char* GetAlphaMapSource (size_t index) { return 0; }
  const char* GetAlphaMapSource (const char* material) { return 0; }

  virtual void SetHeightmapSmooth (bool doSmooth)
  {}
  virtual bool GetHeightmapSmooth () const
  { return false; }

  
  csPtr<iTerrainCellFeederProperties> Clone ()
  {
    return csPtr<iTerrainCellFeederProperties> (
      new TerraFormerFeederProperties (*this));  
  }

  iObjectRegistry* objectReg;
  csRef<iTerraFormer> terraFormer;
  csBox2 samplerRegion;
};

SCF_IMPLEMENT_FACTORY (csTerrainTerraFormerDataFeeder)

csTerrainTerraFormerDataFeeder::csTerrainTerraFormerDataFeeder (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTerrainTerraFormerDataFeeder::~csTerrainTerraFormerDataFeeder ()
{
}

csPtr<iTerrainCellFeederProperties> csTerrainTerraFormerDataFeeder::CreateProperties ()
{
  return csPtr<iTerrainCellFeederProperties> (
    new TerraFormerFeederProperties (object_reg));  
}

bool csTerrainTerraFormerDataFeeder::PreLoad (iTerrainCell* cell)
{
  return false;
}

bool csTerrainTerraFormerDataFeeder::Load (iTerrainCell* cell)
{
  TerraFormerFeederProperties* properties = 
    (TerraFormerFeederProperties*)cell->GetFeederProperties ();

  if (!properties || !properties->terraFormer)
    return false;

  int width = cell->GetGridWidth ();
  int height = cell->GetGridHeight ();

  csRef<iTerraSampler> height_sampler = properties->terraFormer->GetSampler (
    properties->samplerRegion, width, height);

  if (!height_sampler) 
    return false;

  const float* height_data = height_sampler->SampleFloat(heightsString);

  csLockedHeightData data = cell->LockHeightData (csRect(0, 0, width, height));

  for (int y = 0; y < height; ++y)
  {
    memcpy (data.data, height_data, width * sizeof(float));

    data.data += data.pitch;
    height_data += width;
  }

  cell->UnlockHeightData ();
  cell->RecalculateNormalData ();

  int mwidth = cell->GetMaterialMapWidth ();
  int mheight = cell->GetMaterialMapHeight ();

  csRef<iTerraSampler> material_sampler = properties->terraFormer->GetSampler (
    properties->samplerRegion, mwidth, mheight);

  if (!material_sampler) 
    return false;

  const int* material_data = material_sampler->SampleInteger(materialmapString);

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

  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");

  heightsString = strings->Request ("heights");
  materialmapString = strings->Request("materialmap");

  return true;
}

void csTerrainTerraFormerDataFeeder::SetParameter (const char* param, const char* value)
{
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
