/*
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __ILIGHT_H__
#define __ILIGHT_H__

#include "csutil/scf.h"

class csLight;
class csColor;

SCF_VERSION (iLight, 0, 0, 2);

/**
 * The iLight interface is the SCF interface for the csLight class. 
 */
struct iLight : public iBase
{
  /// Get the position of this light.
  virtual csVector3& GetCenter () = 0;
  /// Get the squared radius.
  virtual float GetSquaredRadius () const = 0;
  /// Get the color of this light.
  virtual csColor& GetColor () = 0;
  /// Get the brightness of a light at a given distance.
  virtual float GetBrightnessAtDistance (float d) = 0;
};

#endif // __ILIGHT_H__
