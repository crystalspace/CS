/*
    Copyright (C) Aleksandras Gluchovas
      
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

#ifndef __CSPOINT_H__
#define __CSPOINT_H__

/**
 * A 2D point object
 */
struct csPoint
{
  /// The coordinates of the point
  int x, y;

  /// Constructor: initialize the object with zeros
  csPoint ()
  { x = y = 0; }
  /// Constructor: initialize the object with given values
  csPoint (int iX, int iY)
  { x = iX; y = iY; }
  /// Set the coordinates of this point
  void Set (int iX, int iY)
  { x = iX; y = iY; }
};

#endif