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

void csTerrainSimpleDataFeeder::PreLoad(iTerrainCell* cell)
{
  printf("preload!\n");
}

void csTerrainSimpleDataFeeder::Load(iTerrainCell* cell)
{
  printf("load!\n");

  int width = cell->GetGridWidth();
  int height = cell->GetGridHeight();

  csLockedHeightData data = cell->LockHeightData(csRect(0, 0, width, height));

  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
    {
      float xd = float(x - width/2) / width;
      float yd = float(y - height/2) / height;

      data.data[y * data.pitch + x] = sqrtf(2 - xd*xd - yd*yd);
    }

  cell->UnlockHeightData();
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
