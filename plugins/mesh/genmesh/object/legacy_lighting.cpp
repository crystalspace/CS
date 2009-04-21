/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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

#include "iengine/engine.h"
#include "iengine/movable.h"
#include "iengine/sector.h"

#include "csgeom/math3d.h"

#include "genmesh.h"

CS_PLUGIN_NAMESPACE_BEGIN(Genmesh)
{

csGenmeshMeshObject::LegacyLightingData::LegacyLightingData ()
 : lit_mesh_colors (0), num_lit_mesh_colors (0)
{
}

csGenmeshMeshObject::LegacyLightingData::~LegacyLightingData ()
{
  delete[] lit_mesh_colors;
}

void csGenmeshMeshObject::LegacyLightingData::SetColorNum (int num)
{
  num_lit_mesh_colors = num;
  delete[] lit_mesh_colors;
  lit_mesh_colors = new csColor4 [num_lit_mesh_colors];
}

void csGenmeshMeshObject::LegacyLightingData::Free ()
{
  delete[] lit_mesh_colors;
  lit_mesh_colors = 0;
}

void csGenmeshMeshObject::LegacyLightingData::Clear (const csColor4& base_color)
{
  //csColor amb;
  //factory->engine->GetAmbientLight (amb);
  int i;
  for (i = 0 ; i < num_lit_mesh_colors ; i++)
  {
    lit_mesh_colors[i].Set (base_color);
  }
}


}
CS_PLUGIN_NAMESPACE_END(Genmesh)
