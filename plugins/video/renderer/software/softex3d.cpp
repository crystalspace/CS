/*
    Copyright (C) 2000 by Jorrit Tyberghein
  if (texman->pfmt.PixelBytes != 1)
  {    Written by Samuel Humphreys

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
#include "softex3d.h"
#include "isystem.h"
#include "igraph2d.h"
#include "soft_txt.h"

DECLARE_FACTORY (csDynamicTextureSoft3D)

IMPLEMENT_IBASE (csDynamicTextureSoft3D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END

csDynamicTextureSoft3D::csDynamicTextureSoft3D (iBase *iParent)
  : csGraphics3DSoftwareCommon ()
{
  CONSTRUCT_IBASE (iParent);
  tex_mm = NULL;
}

bool csDynamicTextureSoft3D::Initialize (iSystem *iSys)
{
  // This gets IncRef() when csGraphics3DSoftwareCommon::Initialize (System)
  // gets called below, after the G2D driver is created
  (System = iSys)->IncRef();
  return true;
}

void csDynamicTextureSoft3D::SetTarget (csTextureMMSoftware *tex_mm)
{
  this->tex_mm = tex_mm;
}


iGraphics3D *csDynamicTextureSoft3D::CreateOffScreenRenderer 
    ( iGraphics3D *parent_g3d, int width, int height, csPixelFormat *ipfmt, 
      void *buffer, RGBPixel *palette, int pal_size)
{
  iGraphics2D *parent_g2d = parent_g3d->GetDriver2D ();
  G2D = parent_g2d->CreateOffScreenCanvas (width, height, ipfmt, buffer, 
					palette, pal_size, tex_mm != NULL);
  if (!G2D)
  {
    System->Printf (MSG_FATAL_ERROR, "Error opening Graphics2D texture context.\n");
    return NULL;
  }

  // Presence of tex_mm currently indicates merely whether we are going to
  // share resources or not.

  if (tex_mm)
  {
    SharedInitialize ((csGraphics3DSoftwareCommon*)parent_g3d);
    if (!Open (NULL))
      return NULL;
    SharedOpen ((csGraphics3DSoftwareCommon*)parent_g3d);
//      if (pfmt.PixelBytes == 4)
//        pixel_shift = 2;
//      else if (pfmt.PixelBytes == 2)
//        pixel_shift = 1;
//      else
//        pixel_shift = 0;

//  #ifdef TOP8BITS_R8G8B8_USED
//      if (pfmt.PixelBytes == 4)
//        pixel_adjust = (pfmt.RedShift && pfmt.GreenShift && pfmt.BlueShift) ? 8 : 0;
//  #endif
//      alpha_mask = 0;
//      alpha_mask |= 1 << (pfmt.RedShift);
//      alpha_mask |= 1 << (pfmt.GreenShift);
//      alpha_mask |= 1 << (pfmt.BlueShift);
//      alpha_mask = ~alpha_mask;
    G2D->Open (NULL);
  }
  else
  {
    NewInitialize ();

    if (!Open (NULL) || !NewOpen (NULL))
      return NULL;
  }
  return (iGraphics3D*)this;
}

void csDynamicTextureSoft3D::Print (csRect *area)
{
  csGraphics3DSoftwareCommon::Print (area);
  if (tex_mm && pfmt.PixelBytes != 1)
  {
    // This might break Carmak/AndyZ texture cache optimisation
    tex_mm->RePrepareDynamicTexture ();
  }
}
