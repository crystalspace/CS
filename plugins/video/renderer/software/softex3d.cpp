/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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

IMPLEMENT_IBASE (csDynamicTextureSoft3D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END

csDynamicTextureSoft3D::csDynamicTextureSoft3D (iSystem *iSys, iGraphics2D *iparentG2D) : csGraphics3DSoftwareCommon ()
{
  CONSTRUCT_IBASE (NULL);
  parentG2D = iparentG2D;
  (System = iSys)->IncRef ();
}

iGraphics3D *csDynamicTextureSoft3D::CreateOffScreenRenderer 
    ( int width, int height, csPixelFormat *pfmt, void *buffer, 
      RGBPixel *palette, int pal_size )
{
  G2D = parentG2D->CreateOffScreenCanvas (width, height, pfmt, buffer, 
					palette, pal_size);
  if (!G2D)
  {
    System->Printf (MSG_FATAL_ERROR, "Error opening Graphics2D texture context.\n");
    return NULL;
  }

  if (!csGraphics3DSoftwareCommon::Initialize (System))
    return NULL;

  if (!Open (NULL))
    return NULL;

  return (iGraphics3D*)this;
}
