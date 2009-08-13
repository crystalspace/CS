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

#include "cssysdef.h"
#include <ctype.h>
#include "csgfx/textureformatstrings.h"

namespace CS
{

csString TextureFormatStrings::ConvertCanonical (const char* in)
{
  StructuredTextureFormat fmt = CS::TextureFormatStrings::ConvertStructured (in);
  return fmt.GetCanonical ();
}

//--------------------------------------------------------------------------------

StructuredTextureFormat::StructuredTextureFormat ()
{
  coded_components = CONST_UINT64 (0);
  format = Invalid;
}

bool StructuredTextureFormat::AddComponent (char cmp, int size)
{
  uint64 shifted = coded_components << 16;
  if ((shifted >> 16) != coded_components)
    return false;
  coded_components = shifted + (CONST_UINT64 (256) * cmp) + size;
  return true;
}

void StructuredTextureFormat::FixSizes (int size)
{
  uint16 p1 = (coded_components >> 48) & 65535;
  uint16 p2 = (coded_components >> 32) & 65535;
  uint16 p3 = (coded_components >> 16) & 65535;
  uint16 p4 = coded_components & 65535;
  if (p1 != 0 && (p1 & 255) == 0) p1 += size;
  if (p2 != 0 && (p2 & 255) == 0) p2 += size;
  if (p3 != 0 && (p3 & 255) == 0) p3 += size;
  if (p4 != 0 && (p4 & 255) == 0) p4 += size;
  coded_components = (uint64 (p1) << 48) + (uint64 (p2) << 32) + (p3 << 16) + p4;
}

csString StructuredTextureFormat::GetCanonical ()
{
  if (format == Invalid) return csString ();
  if (format == Special) return special;
  csString out;
  uint16 p1 = (coded_components >> 48) & 65535;
  uint16 p2 = (coded_components >> 32) & 65535;
  uint16 p3 = (coded_components >> 16) & 65535;
  uint16 p4 = coded_components & 65535;
  if (p1 != 0) { out += char ((p1>>8)&255); out += p1 & 255; }
  if (p2 != 0) { out += char ((p2>>8)&255); out += p2 & 255; }
  if (p3 != 0) { out += char ((p3>>8)&255); out += p3 & 255; }
  if (p4 != 0) { out += char ((p4>>8)&255); out += p4 & 255; }
  out += '_';
  out += (char)format;
  return out;
}

uint StructuredTextureFormat::GetComponentMask () const
{
  if ((format == Special) || (format == Invalid)) return 0;

  uint mask = 0;
  for (int n = 0; n < 4; n++)
  {
    char c =  (coded_components >> (16 * n + 8)) & 255;
    switch (c)
    {
      case 0:   break;
      case 'r': mask |= compR; break;
      case 'g': mask |= compG; break;
      case 'b': mask |= compB; break;
      case 'a': mask |= compA; break;
      case 'x': mask |= compX; break;
      case 'l': mask |= compL; break;
      case 'd': mask |= compD; break;
      case 's': mask |= compS; break;
      default:  mask |= compUnknown; break;
    }
  }
  return mask;
}

//--------------------------------------------------------------------------------

StructuredTextureFormat TextureFormatStrings::ConvertStructured (const char* in)
{
  // Scan the initial part for components.
  if (!in || !*in) return StructuredTextureFormat ();

  StructuredTextureFormat out;

  // If we have a string starting with '*' then we just return the
  // string unchanged.
  if (*in == '*')
  {
    out.SetSpecial (in);
    return out;
  }

  int lastsize = 8;
  do
  {
    char cmp = *in++;
    if (strchr ("rgbalds", cmp) == 0) return StructuredTextureFormat ();

    int s = 0;
    while (isdigit (*in))
    {
      s = s*10 + (*in-'0');
      in++;
    }
    if (!out.AddComponent (cmp, s))
      return StructuredTextureFormat ();
    if (s != 0) lastsize = s;
  }
  while (*in && *in != '_');

  StructuredTextureFormat::TextureFormat format = 
    StructuredTextureFormat::Integer;
  if (*in == '_')
  {
    // There is a format that follows.
    in++;
    if ((*in == StructuredTextureFormat::Float 
        || *in == StructuredTextureFormat::Integer) 
      && *(in+1) == 0)
      format = (StructuredTextureFormat::TextureFormat)*in;
    else
      return StructuredTextureFormat ();
  }
  out.SetFormat (format);
  out.FixSizes (lastsize);
  return out;
}

} // namespace CS

