/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSOSDEFS_H__
#define __CSOSDEFS_H__

extern char* get_software_2d_driver ();
#define SOFTWARE_2D_DRIVER get_software_2d_driver ()

// The 2D graphics drivers used by software renderer on this platform
#define SOFTWARE_2D_DRIVER_SVGA	"crystalspace.graphics2d.svgalib"
#define SOFTWARE_2D_DRIVER_GGI	"crystalspace.graphics2d.ggi"
#define SOFTWARE_2D_DRIVER_XLIB	"crystalspace.graphics2d.xlib"

// The 2D graphics driver used by OpenGL renderer
#define OPENGL_2D_DRIVER "crystalspace.graphics2d.glx"

// The 2D graphics driver used by Glide renderer
#define GLIDE_2D_DRIVER	"crystalspace.graphics2d.glidex"

// The sound driver
#define SOUND_DRIVER "crystalspace.sound.driver.oss"

#if defined (SYSDEF_DIR)
#  define __NEED_GENERIC_ISDIR
#endif

#endif // __CSOSDEFS_H__
