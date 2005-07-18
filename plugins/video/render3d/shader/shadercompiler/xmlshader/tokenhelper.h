/*
  Copyright (C) 2004 by Frank Richter
	    (C) 2004 by Jorrit Tyberghein

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __TOKENHELPER_H__
#define __TOKENHELPER_H__

static inline
bool TokenEquals (const char* token, size_t tokenLen, const char* cmp)
{
  return (tokenLen == strlen (cmp)) && (strncmp (token, cmp, tokenLen) == 0);
}

#endif // __TOKENHELPER_H__
