/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_GL_STRINGLISTS_H__
#define __CS_GL_STRINGLISTS_H__

#include "cstool/identstrings.h"

CS_IDENT_STRING_LIST(ClipperTypes)
  CS_IDENT_STRING(CS_CLIPPER_NONE)
  CS_IDENT_STRING(CS_CLIPPER_OPTIONAL)
  CS_IDENT_STRING(CS_CLIPPER_TOPLEVEL)
  CS_IDENT_STRING(CS_CLIPPER_REQUIRED)
  CS_IDENT_STRING(CS_CLIPPER_EMPTY)
CS_IDENT_STRING_LIST_END(ClipperTypes)

CS_BITMASKTOSTR_MASK_TABLE_BEGIN(drawflagNames)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CSDRAW_2DGRAPHICS)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CSDRAW_3DGRAPHICS)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CSDRAW_CLEARZBUFFER)
  CS_BITMASKTOSTR_MASK_TABLE_DEFINE(CSDRAW_CLEARSCREEN)
CS_BITMASKTOSTR_MASK_TABLE_END;

#endif // __CS_GL_STRINGLISTS_H__
