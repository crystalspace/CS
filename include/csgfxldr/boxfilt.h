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

#ifndef BOXFILT_H
#define BOXFILT_H

/**
 * A matrix for a 3x3 box filter.
 */
struct Filter3x3
{
  int f11, f12, f13;
  int f21, f22, f23;
  int f31, f32, f33;
  int tot;
};

/**
 * A matrix for a 5x5 box filter.
 */
struct Filter5x5
{
  int f00, f01, f02, f03, f04;
  int f10, f11, f12, f13, f14;
  int f20, f21, f22, f23, f24;
  int f30, f31, f32, f33, f34;
  int f40, f41, f42, f43, f44;
  int tot;
};

#endif //BOXFILT_H
