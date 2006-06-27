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

  csRef<iLoader> loader = csQueryRegistry<iLoader> (object_reg);
  
  csRef<iImage> map = loader->LoadImage (heightmap_source,
  CS_IMGFMT_PALETTED8);

  if (map->GetWidth () != width || map->GetHeight () != height)
  {
    map = csImageManipulate::Rescale (map, width, height);
  }
  
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
    {
      float xd = float(x - width/2) / width;
      float yd = float(y - height/2) / height;

      data.data[y * data.pitch + x] = ((unsigned char*)map->GetImageData ())
      [y * map->GetWidth () + x] / 255.0f * cell->GetSize().z;
    }

  cell->UnlockHeightData ();
  
  csRef<iImage> material = loader->LoadImage (mmap_source,
    CS_IMGFMT_PALETTED8);
  
  if (material->GetWidth () != cell->GetMaterialMapWidth () ||
    material->GetHeight () != cell->GetMaterialMapHeight ())
    material = csImageManipulate::Rescale (material,
      cell->GetMaterialMapWidth (), cell->GetMaterialMapHeight ());
  
  int mwidth = material->GetWidth ();
  int mheight = material->GetHeight ();

  csLockedMaterialMap mdata = cell->LockMaterialMap (csRect (0, 0, mwidth,
    mheight));
  
  for (int y = 0; y < mheight; ++y)
    memcpy(mdata.data + mdata.pitch * y, (const unsigned char*)material->
      GetImageData() + mwidth * y, mwidth);
   
  cell->UnlockMaterialMap();
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
