/*
    Copyright (C) 1998, 2000 by Jorrit Tyberghein
  
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

#ifndef __IENGINE_ITEXTURE_H__
#define __IENGINE_ITEXTURE_H__

#include "csutil/scf.h"
#include "cstypes.h"

SCF_VERSION (iTextureWrapper, 0, 0, 1);

/**
 * This class represents a texture wrapper which holds
 * the mapping between a texture in the engine and a texture
 * in the 3D rasterizer.
 */
struct iTextureWrapper : public iBase
{
};

#endif // __IENGINE_ITEXTURE_H__
