/*
    Copyright (C) 2000 by Norman Krämer
  
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

#ifndef __CSSLIDER_H__
#define __CSSLIDER_H__

/**
 * This is a slider control that draws vertical and/or horizontal lines in its parent canvas.
 * Look at csGrid to see it in action.
 */

#include "csws/csbutton.h"

enum {
  /**
   * Sent to parent whenever the slider is moved.
   */
  cscmdSliderPosChanged = 0x00000d00, // slider moved
  cscmdSliderPosSet // slider has been set
};

class csSlider : public csComponent
{
 protected:
  bool isSliding;
  bool isHorizontal;
  int mx, my; // last mouse position

 public:
  csSlider (csComponent *pParent);

  virtual void Draw ();
  virtual bool HandleEvent (iEvent &Event);
  bool SetRect (int xmin, int ymin, int xmax, int ymax);
  void GetLastMousePos (int &x, int &y){ x = mx; y = my; }

 public:
  csButton *handle;
};

#endif
