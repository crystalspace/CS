/*
  Copyright (C) 2003 by Mårten Svanfeldt
                        Anders Stenberg

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

#include "null_render3d.h"
#include "null_renderbuffer.h"

SCF_IMPLEMENT_IBASE (csNullRenderBuffer)
  SCF_IMPLEMENTS_INTERFACE (iRenderBuffer)
SCF_IMPLEMENT_IBASE_END

csNullRenderBuffer::csNullRenderBuffer (int size, csRenderBufferType type,
	csRenderBufferComponentType comptype, int compcount)
{
  static const int compSizes[] = 
  {
    sizeof (char), sizeof (unsigned char), 
    sizeof (short), sizeof (unsigned short),
    sizeof (int), sizeof (unsigned int),
    sizeof (float),
    sizeof (double)
  };

  SCF_CONSTRUCT_IBASE (0)
  csNullRenderBuffer::size = size;
  csNullRenderBuffer::type = type;
  csNullRenderBuffer::comptype = comptype;
  csNullRenderBuffer::compcount = compcount;

  compSize = compSizes [comptype];
}

csNullRenderBuffer::~csNullRenderBuffer()
{
  SCF_DESTRUCT_IBASE()
}

