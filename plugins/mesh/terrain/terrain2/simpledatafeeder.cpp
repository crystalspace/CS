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

#include "iengine/material.h"
#include "igraphic/image.h"
#include "imap/loader.h"
#include "imesh/terrain2.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "simpledatafeeder.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
SCF_IMPLEMENT_FACTORY (csTerrainSimpleDataFeeder)

csTerrainSimpleDataFeeder::csTerrainSimpleDataFeeder (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTerrainSimpleDataFeeder::~csTerrainSimpleDataFeeder ()
{
}

csPtr<iTerrainCellFeederProperties> csTerrainSimpleDataFeeder::CreateProperties ()
{
  return csPtr<iTerrainCellFeederProperties> (new csTerrainSimpleDataFeederProperties);
}

bool csTerrainSimpleDataFeeder::PreLoad (iTerrainCell* cell)
{
  return false;
}

bool csTerrainSimpleDataFeeder::Load (iTerrainCell* cell)
{
  csTerrainSimpleDataFeederProperties* properties = 
    (csTerrainSimpleDataFeederProperties*)cell->GetFeederProperties ();

  if (!loader || !properties)
    return false;

  int width = cell->GetGridWidth ();
  int height = cell->GetGridHeight ();

  csLockedHeightData data = cell->LockHeightData (csRect(0, 0, width, height));
  
  csRef<iImage> map = loader->LoadImage (properties->heightmapSource.GetDataSafe (), 
    CS_IMGFMT_PALETTED8);

  if (!map) 
    return false;

  if (map->GetWidth () != width || map->GetHeight () != height)
  {
    map = csImageManipulate::Rescale (map, width, height);
  }
  
  const unsigned char* imagedata = (const unsigned char*)map->GetImageData ();

  float dataYScale = cell->GetSize().y;

  for (int y = 0; y < height; ++y)
  {
    float* dest_data = data.data;

    for (int x = 0; x < width; ++x)
    {
      float xd = float(x - width/2) / width;
      float yd = float(y - height/2) / height;

      *dest_data++ = *imagedata++ / 255.0f * dataYScale;
    }
    
    data.data += data.pitch;
  }

  cell->UnlockHeightData ();
  
  csRef<iImage> material = loader->LoadImage (properties->materialmapSource.GetDataSafe (),
    CS_IMGFMT_PALETTED8);

  if (!material) 
    return false;
  
  if (material->GetWidth () != cell->GetMaterialMapWidth () ||
      material->GetHeight () != cell->GetMaterialMapHeight ())
  {
    material = csImageManipulate::Rescale (material,
      cell->GetMaterialMapWidth (), cell->GetMaterialMapHeight ());
  }
  
  int mwidth = material->GetWidth ();
  int mheight = material->GetHeight ();

  csLockedMaterialMap mdata = cell->LockMaterialMap (csRect (0, 0, mwidth, mheight));

  const unsigned char* materialmap = (const unsigned char*)material->GetImageData ();
    
  for (int y = 0; y < mheight; ++y)
  {
    memcpy (mdata.data, materialmap, mwidth);
    mdata.data += mdata.pitch;
    materialmap += mwidth;
  } 
  
  cell->UnlockMaterialMap ();

  return true;
}

bool csTerrainSimpleDataFeeder::Initialize (iObjectRegistry* object_reg)
{
  this->objectReg = object_reg;

  loader = csQueryRegistry<iLoader> (objectReg);
  return true;
}

void csTerrainSimpleDataFeeder::SetParameter (const char* param, const char* value)
{
}


csTerrainSimpleDataFeederProperties::csTerrainSimpleDataFeederProperties ()
  : scfImplementationType (this)
{
}

csTerrainSimpleDataFeederProperties::csTerrainSimpleDataFeederProperties (
  csTerrainSimpleDataFeederProperties& other)
  : scfImplementationType (this), heightmapSource (other.heightmapSource),
  materialmapSource (other.materialmapSource)
{
}


void csTerrainSimpleDataFeederProperties::SetHeightmapSource (const char* source)
{
  heightmapSource = source;
}

void csTerrainSimpleDataFeederProperties::SetParameter (const char* param, const char* value)
{
  if (strcmp (param, "heightmap source") == 0)
  {
    heightmapSource = value;
  }
  else if (strcmp (param, "materialmap source") == 0)
  {
    materialmapSource = value;
  }
}

csPtr<iTerrainCellFeederProperties> csTerrainSimpleDataFeederProperties::Clone ()
{
  return csPtr<iTerrainCellFeederProperties> (new csTerrainSimpleDataFeederProperties (*this));
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
