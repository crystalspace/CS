/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "csengine/csspr2d.h"
#include "csengine/world.h"
#include "csengine/sector.h"

//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite2D, csSprite)

csSprite2D::csSprite2D () : csSprite (), position (0, 0, 0)
{
  cstxt = NULL;
}

csSprite2D::~csSprite2D ()
{
}

void csSprite2D::UpdatePolyTreeBBox ()
{
}

void csSprite2D::UpdateLighting (csLight** /*lights*/, int /*num_lights*/)
{
}

void csSprite2D::Draw (csRenderView& rview)
{
  if (!cstxt)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error! Trying to draw a 2D sprite with no texture!\n");
    fatal_exit (0, false);
  }

  rview.g3d->SetZBufMode (CS_ZBUF_USE);

  static G3DPolygonDPFX g3dpolyfx;
  g3dpolyfx.num = vertices.GetLimit ();
  g3dpolyfx.txt_handle = cstxt->GetTextureHandle ();
  g3dpolyfx.inv_aspect = rview.inv_aspect;
}


