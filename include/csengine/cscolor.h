/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef CSCOLOR_H
#define CSCOLOR_H

/**
 * A class used to represent a color in RGB space.
 */
class csColor
{
public:
  /// Red (0..1)
  float red;
  /// Green (0..1)
  float green;
  /// Blue (0..1)
  float blue;

public:
  /// Initialize a color object (contents undefined)
  csColor () { }
  /// Initialize a color object with given R,G,B components
  csColor (float r, float g, float b)
  { red = r; green = g; blue = b; }
  /// Set color to given R,G,B components
  void Set (float r, float g, float b)
  { red = r; green = g; blue = b; }
  /// Clamp color to given R,G,B values
  void Clamp (float r, float g, float b)
  { if (red > r) red = r; if (green > g) green = g; if (blue > b) blue = b; }
  /// Assign one color object to another
  inline void operator= (const csColor &Copy)
  { red = Copy.red; green = Copy.green; blue = Copy.blue; }
};

#endif /*CSCOLOR_H*/
