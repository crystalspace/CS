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

// The 2D graphics driver used by software renderer on this platform
#define CS_SOFTWARE_2D_DRIVER "crystalspace.graphics2d.dosraw"

#if defined (CS_SYSDEF_PROVIDE_GETCWD)
#  include <dos.h>
char _getdrive ()
{
  unsigned int drive;
  _dos_getdrive (&drive);
  return (char) drive; // is this correct?
}
void _chdrive (char drive)
{
  unsigned int num_drives; // useless
  _dos_setdrive (drive, &num_drives);
}                  
#endif

#if defined (CS_SYSDEF_PROVIDE_GETCWD) || defined (CS_SYSDEF_PROVIDE_ACCESS)
#  include <unistd.h>
#  undef CS_SYSDEF_PROVIDE_GETCWD
#  undef CS_SYSDEF_PROVIDE_ACCESS
static inline char *djgpp_getcwd (char *buf, size_t size)
{
  char *out = getcwd (buf, size);
  int i;
  for (i = 0; out [i]; i++)
    if (out [i] == '/') out [i] = '\\';
  return out;
}
#  define getcwd djgpp_getcwd
#endif

#if defined (CS_SYSDEF_PROVIDE_DIR)
#  define __NEED_GENERIC_ISDIR
#endif

#define CS_LITTLE_ENDIAN

#endif // __CSOSDEFS_H__
