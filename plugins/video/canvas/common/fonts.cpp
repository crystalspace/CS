/*
    System fonts
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

#include "sysdef.h"
#include "graph2d.h"			// FontDef definition
#include "cs2d/common/police.fnt"
#include "cs2d/common/courier.fnt"	// font (C) Andrew Zabolotny
#include "cs2d/common/tiny.fnt"		// font (C) Andrew Zabolotny
#include "cs2d/common/italic.fnt"	// font (C) Andrew Zabolotny

FontDef FontList[] =
{
  {8, 8, 8,	width_Police,	(unsigned char *)font_Police},
  {8, 8, 8,	NULL,		(unsigned char *)font_Police},
  {8, 8, 8,	width_Italic,	(unsigned char *)font_Italic},
  {8, 8, 8,	NULL,		(unsigned char *)font_Italic},
  {7, 8, 8,	width_Courier,	(unsigned char *)font_Courier},
  {8, 8, 8,	NULL,		(unsigned char *)font_Courier},
  {4, 6, 8,	width_Tiny,	(unsigned char *)font_Tiny},
  {6, 6, 8,	NULL,		(unsigned char *)font_Tiny}
};
