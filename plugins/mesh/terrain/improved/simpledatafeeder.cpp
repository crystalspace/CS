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

#include "iengine/material.h"

#include "iengine/engine.h"

#include "imap/loader.h"

#include "simpledatafeeder.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainSimpleDataFeeder)

csTerrainSimpleDataFeeder::csTerrainSimpleDataFeeder (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTerrainSimpleDataFeeder::~csTerrainSimpleDataFeeder ()
{
}

void csTerrainSimpleDataFeeder::PreLoad (iTerrainCell* cell)
{
  printf("preload!\n");
}

void csTerrainSimpleDataFeeder::Load (iTerrainCell* cell)
{
  printf("load!\n");

  int width = cell->GetGridWidth ();
  int height = cell->GetGridHeight ();

  csLockedHeightData data = cell->LockHeightData (csRect(0, 0, width, height));

  csRef<iLoader> loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  
  csRef<iImage> map = loader->LoadImage (heightmap_source,
  CS_IMGFMT_PALETTED8);
  
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
    {
      float xd = float(x - width/2) / width;
      float yd = float(y - height/2) / height;

      data.data[y * data.pitch + x] = ((unsigned char*)map->GetImageData ())
      [y * map->GetWidth () + x] / 255.0f * cell->GetSize().z;
    }

  cell->UnlockHeightData ();
  
  int mwidth = cell->GetMaterialMapWidth ();
  int mheight = cell->GetMaterialMapHeight ();
  
  csRef<iImage> material = loader->LoadImage (mmap_source,
  CS_IMGFMT_TRUECOLOR);
  
  csDirtyAccessArray<unsigned char> mdata;
  mdata.SetSize (mwidth * mheight);
  
  for (int i = 0; i < 3; ++i)
  {
    for (int y = 0; y < mheight; ++y)
      for (int x = 0; x < mwidth; ++x)
        mdata[y * mwidth + x] = ((unsigned char*)material->GetImageData ())
        [y * mwidth * 4 + x * 4 + i];
        
    cell->SetMaterialMask (i, mdata.GetArray (), mwidth, mheight);
  }
}

bool csTerrainSimpleDataFeeder::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  return true;
}

void csTerrainSimpleDataFeeder::SetParam(const char* param, const char* value)
{
  if (!strcmp(param, "heightmap source"))
  {
    heightmap_source = value;
  }
  else if (!strcmp(param, "materialmap source"))
  {
    mmap_source = value;
  }
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
