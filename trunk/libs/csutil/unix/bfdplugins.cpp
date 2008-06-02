/*
    Load plugin meta data from object file headers.
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Eric Sunshine
	      (C) 2003 by Frank Richter
	      (C) 2003 by Mat Sutcliffe

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

/* Note: The section name must be of the same value as SECTION_TAG_NAME 
    in unix.jam */
#define SECTION_TAG_NAME      ".crystalspace"

#include <bfd.h>

char* csExtractMetadata (const char* fullPath, const char*& errMsg)
{
  char* buf = 0;
  bfd *abfd = bfd_openr (fullPath, 0);
  if (abfd)
  {
    if (bfd_check_format (abfd, bfd_object))
    {
      asection *sect = bfd_get_section_by_name (abfd, SECTION_TAG_NAME);
      if (sect)
      {
        int size = bfd_section_size (abfd, sect);
        buf = new char[size + 1];
        if (buf)
        {
          if (! bfd_get_section_contents (abfd, sect, buf, 0, size))
          {
            errMsg = "libbfd can't get '" SECTION_TAG_NAME
                     "' section contents";
            delete[] buf; buf = 0;
          }
          else
          {
            buf[size] = 0;
          }
        }
      }
    }
    bfd_close (abfd);
  }
  return buf;
}
