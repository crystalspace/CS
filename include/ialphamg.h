/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Jorrit Tyberghein.

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

#ifndef IALPHAMG_H
#define IALPHAMG_H

#include "cscom/com.h"

interface IImageFile;
interface IAlphaMapHandle;

extern const GUID IID_IAlphaMapManager;

/**
 * This is the standard texture manager interface.
 * A 3D rasterizer will have to implement a subclass of this one and
 * return a pointer to it in Graphics3D.
 * This class is responsible for receiving all textures
 * from the 3D engine, converting them to an internal format if
 * needed, calculating a palette if needed, and calculating all
 * lookup tables related to the textures. Mipmap creation is
 * also done in this class.
 */
interface IAlphaMapManager : public IUnknown
{
public:
};

#endif      // IALPHAMG_H
