/*
    Crystal Space Windowing System: color wheel class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSCWHEEL_H__
#define __CS_CSCWHEEL_H__

/**\file
 * Crystal Space Windowing System: color wheel class
 */

/**
 * \addtogroup csws_comps_cwheel
 * @{ */
 
#include "csextern.h"
 
#include "csstatic.h"

/// Color wheel messages
enum
{
  /**
   * Color wheel changed notification<p>
   * This notification is sent to parent when H & L wheel components change.
   * <pre>
   * IN: (csColorWheel *)source
   * </pre>
   */
  cscmdColorWheelChanged = 0x00000B00
};

/// Color wheel static control
class CS_CRYSTALSPACE_EXPORT csColorWheel : public csStatic
{
  // Current H & S values (L is separate)
  float h,s;
  // true if mouse events are captured by this control
  bool trackmouse;
public:
  /// Create a object of ColorWheel class
  csColorWheel (csComponent *iParent);
  /// Destroy the color wheel object
  virtual ~csColorWheel ();
  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);
  /// Draw the wheel
  virtual void Draw ();
  /// Set H and S components
  void SetHS (float iH, float iS);
  /// Query the H and S components
  void GetHS (float &oH, float &oS)
  { oH = h; oS = s; }
};

/** @} */

#endif // __CS_CSCWHEEL_H__
