/*
    PNG image file format support for CrystalSpace 3D library
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __PNGSAVE_H__
#define __PNGSAVE_H__

#include "igraphic/image.h"

/**
 * This function will save an iImage into a .PNG format file.
 * It is useful for doing debugging, screen shots and so on.
 * Note that DO_PNG should be defined in order for this function
 * to be present in graphics library.
 */
extern bool csSavePNG (const char *FileName, iImage *Image);

#endif // __PNGSAVE_H__
