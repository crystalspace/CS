/*
  Crystal Space Windowing System: dialog window class
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSDIALOG_H__
#define __CSDIALOG_H__

#include "csutil/csbase.h"
#include "cscomp.h"

/**
 * Possible dialog frame styles<p>
 * Dialog can have no frame (default) or can have a horizontal or vertical
 * "frame" which is used for horizontal and vertical toolbars.
 */
enum csDialogFrameStyle
{
  /// Dialog has no frame (default)
  csdfsNone,
  /// Dialog has two 3D horizontal lines at top and bottom (like menu bar)
  csdfsHorizontal,
  /// Dialog has two 3D vertical lines at left and right
  csdfsVertical,
  /// Dialog has a 3D frame around perimeter
  csdfsAround
};

/**
 * The Dialog class is a single-colored canvas which contains a
 * number of child controls. The dialog can perform a number of
 * operations on its childs such as switching between them using
 * Tab/ShiftTab key, activating the default button when Enter is
 * pressed etc.
 * <p>
 * Other uses for csDialog class are for floating toolbars. They
 * can be even resizeable; to create a floating toolbar you should
 * create a stand-alone dialog object, setting his DragStyle to
 * CS_DRAG_MOVEABLE and, possibly, CS_DRAG_SIZEABLE. In this case
 * dialog will act as a standalone window; it would be good if you
 * specify its frame style to csdfsAround: in this case it will
 * look like a usual window but without titlebar.
 */
class csDialog : public csComponent
{
  /// Dialog frame style
  csDialogFrameStyle FrameStyle;
  /// Automatical grid placement parameters
  int GridX, GridY;
  /// Automatically snap dialog size to grid?
  bool SnapSizeToGrid;
  /// First component
  csComponent *first;

protected:
  /// Border width and height
  int BorderWidth, BorderHeight;

public:
  /// Create dialog object
  csDialog (csComponent *iParent, csDialogFrameStyle iFrameStyle = csdfsNone);

  /// Draw the button
  virtual void Draw ();

  /// Handle input events
  virtual bool HandleEvent (csEvent &Event);

  /**
   * Enable/disable(dx<0||dy<0) automatic control placement in a grid fashion.
   * DeltaX and DeltaY is the horizontal and vertical distance between controls;
   * SnapSize tells dialog object whenever dialog size should snap when it
   * is resized to the maximal x/y coordinates of all controls.
   */
  void SetAutoGrid (int iDeltaX, int iDeltaY, bool iSnapSize)
  { GridX = iDeltaX; GridY = iDeltaY; SnapSizeToGrid = iSnapSize; }

  /// Do auto-placement work if enabled
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /// Return the recommended minimal size of dialog
  virtual void SuggestSize (int &w, int &h);

  /// Return border width and height
  void GetBorderSize (int &w, int &h)
  { w = BorderWidth; h = BorderHeight; }

  /// Fix dialog size when resizing
  virtual void FixSize (int &newW, int &newH);

protected:
  /// Used by SuggestSize
  static bool do_topleft (csComponent *comp, void *param);
};

#endif // __CSDIALOG_H__
