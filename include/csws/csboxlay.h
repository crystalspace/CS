/*
    Copyright (C) Aleksandras Gluchovas
    CS port by Norman Kraemer <norman@users.sourceforge.net>

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

#ifndef __CS_CSBOXLAYOUT_H__
#define __CS_CSBOXLAYOUT_H__

/**
 * \addtogroup csws_layout
 * @{ */
 
#include "csextern.h"
 
#include "cslayout.h"

/**
 * Components are scaled to fit in one row or one column of the canvas.
 */

class CS_CRYSTALSPACE_EXPORT csBoxLayout : public csLayout
{
 public:
  /// Set axis to 0 to align the components along the x-axis in the middle
  /// of this layout component.
  /// To align along y axis set axis to 1.
  csBoxLayout (csComponent* pParent, int axis);

  virtual void SuggestSize (int &sugw, int& sugh);
  virtual void LayoutContainer ();

  enum AXIS_ORIENTATIONS
  {
    X_AXIS = 0,
    Y_AXIS = 1
  };

 protected:
  int mAxis;
};

/** @} */

#endif // __CS_CSBOXLAYOUT_H__
