/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#include <ctype.h>
#include "cssysdef.h"
#include "csgfx/textureformatstrings.h"

namespace CS
{

struct comp
{
  char component;
  int size;
};

csString TextureFormatStrings::CanonicalTextureFormat (const char* in)
{
  csString out;

  // Scan the initial part for components.
  if (!*in) { out = '-'; return out; }	// Error.

  // If we have a string starting with '*' then we just return the
  // string unchanged.
  if (*in == '*')
  {
    out = in;
    return out;
  }

  comp components[10];
  int idx = 0;
  int lastsize = 8;
  do
  {
    if (idx >= 10) { out = '-'; return out; }	// Error. Too many components.
    char cmp = *in++;
    if (strchr ("rgbalds", cmp) == 0) { out = '-'; return out; } // Error.
    components[idx].component = cmp;
    int s = 0;
    while (isdigit (*in))
    {
      s = s*10 + (*in-'0');
      in++;
    }
    components[idx].size = s;
    if (s != 0) lastsize = s;
    idx++;
  }
  while (*in && *in != '_');

  char format = 'i';
  if (*in == '_')
  {
    // There is a format that follows.
    in++;
    if ((*in == 'f' || *in == 'i') && *(in+1) == 0)
      format = *in;
    else { out = '-'; return out; }	// Error.
  }

  int i;
  for (i = 0 ; i < idx ; i++)
  {
    out += components[i].component;
    int s = components[i].size;
    if (s) out += s;
    else out += lastsize;
  }
  out += '_';
  out += format;

  return out;
}

} // namespace CS

