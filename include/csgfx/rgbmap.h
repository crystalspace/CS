/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __RGBMAP_H__
#define __RGBMAP_H__

#include "csgfx/rgbpixel.h"

/// This is a smiple RGB pixel map for purposes where iImage is too complex.
class csRGBMap
{
private:
  csRGBpixel *Map;
  int Size;

public:
  /// constructor
  inline csRGBMap (int Size = 0);
  /// destructor
  inline ~csRGBMap ();

  /// return the maximum size of the map
  inline int GetSize () const;
  /// return the RGB map
  inline csRGBpixel *GetMap ();

  /// Clear the whole RGB map
  inline void Clear ();
  /// Allocate a new map
  inline void Alloc (int Size);
  /// Copy all data from another map
  inline void Copy (const csRGBMap *other);
  /**
   * Clear the current map and use the pixel array of another map.
   * The other map will be set to empty.
   */
  inline void TakeOver (csRGBMap *other);
};

inline csRGBMap::csRGBMap (int s)
{
  Map = NULL;
  Size = 0;
  Alloc (s);
}

inline csRGBMap::~csRGBMap ()
{
  Clear ();
}

inline int csRGBMap::GetSize () const
{
  return Size;
}

inline csRGBpixel *csRGBMap::GetMap ()
{
  return Map;
}

inline void csRGBMap::Clear ()
{
  if (Map) delete[] Map;
  Map = NULL;
  Size = 0;
}

inline void csRGBMap::Alloc (int s)
{
  if (Size == s) return;
  Clear ();
  if (s>0)
  {
    Map = new csRGBpixel [s];
    Size = s;
  }
}

inline void csRGBMap::Copy (const csRGBMap *other)
{
  Alloc (other->Size);
  memcpy (Map, other->Map, sizeof(csRGBpixel) * Size);
}

inline void csRGBMap::TakeOver (csRGBMap *other)
{
  Clear ();
  Map = other->Map;
  Size = other->Size;
  other->Map = NULL;
  other->Size = 0;
}

#endif // __RGBMAP_H__
