/*
    Copyright (C) 2000 by Norman Kraemer

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

#ifndef __CS_CSSPLIT_H__
#define __CS_CSSPLIT_H__

/**\file
 */

/**
 * \addtogroup csws_comps_splitter
 * @{ */
 
#include "csextern.h"
 
#include "csws/csbutton.h"

/// Splitter messages
enum
{
  /**
   * Sent to parent whenever the splitter is moved.
   * <pre>
   * IN: (csSplitter *)splitter
   * </pre>
   */
  cscmdSplitterPosChanged = 0x00000d00,
  /**
   * Send to parent component when the splitter has finished
   * its motion (e.g. the user releases the mouse button).
   * <pre>
   * IN: (csSplitter *)splitter
   * </pre>
   */
  cscmdSplitterPosSet
};

/**
 * This is a splitter control that draws vertical and/or horizontal lines
 * in its parent canvas. It is used to split some view into parts dynamically.
 * Look at csGrid to see it in action.
 */
class CS_CRYSTALSPACE_EXPORT csSplitter : public csComponent
{
protected:
  /// True if it is currently sliding
  bool isSliding;
  /// True if the splitter is horizontal
  bool isHorizontal;
  /// Mouse delta x and y (when user grabbed the splitter)
  int mdx, mdy;
  /// Current mouse position within slider
  int mousex, mousey;

public:
  /// Create the splitter object
  csSplitter (csComponent *pParent);

  /// Draw the splitter
  virtual void Draw ();
  /// Handle events
  virtual bool HandleEvent (iEvent &Event);
  /// Set splitter size/position
  bool SetRect (int xmin, int ymin, int xmax, int ymax);
  /// Get the position of the splitter
  void GetPos (int &x, int &y);
};

/** @} */

#endif // __CS_CSSPLIT_H__
