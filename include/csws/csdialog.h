/*
    Crystal Space Windowing System: dialog window class
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

#ifndef __CS_CSDIALOG_H__
#define __CS_CSDIALOG_H__

/**\file
 * Crystal Space Windowing System: dialog window class
 */

/**
 * \addtogroup csws_comps_dialog
 * @{ */
 
#include "csextern.h"
 
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
  csdfsAround,
  /// Dialog's frame and background are defined entirely by a bitmap
  csdfsBitmap
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
class CS_CRYSTALSPACE_EXPORT csDialog : public csComponent
{
protected:
  /// Dialog frame style
  csDialogFrameStyle FrameStyle;
  /// Automatical grid placement parameters
  int GridX, GridY;
  /// Automatically snap dialog size to grid?
  bool SnapSizeToGrid;
  /// First component
  csComponent *first;
  /// Border width and height
  int BorderWidth, BorderHeight;
  /// Dialog transparency (if CSS_TRANSPARENT is set)
  uint8 Alpha, OverlayAlpha;
  /// Frame bitmap, if there is one
  csPixmap *FrameBitmap, *OverlayBitmap;
  /// Set if this component should delete the frame bitmap when it is done
  bool delFrameBitmap, delOverlayBitmap;

public:
  /// Create dialog object
  csDialog (csComponent *iParent, csDialogFrameStyle iFrameStyle = csdfsNone);

  /// Destroy a dialog object
  virtual ~csDialog();

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

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
  /// Set border width and height
  void SetBorderSize (int w, int h);

  /// Fix dialog size when resizing
  virtual void FixSize (int &newW, int &newH);

  /// Query dialog border style
  inline csDialogFrameStyle GetFrameStyle ()
  { return FrameStyle; }
  /// Change dialog border style
  void SetFrameStyle (csDialogFrameStyle iFrameStyle);

  /// Get the name of the skip slice for this component
  virtual char *GetSkinName ()
  { return "Dialog"; }

  /// Set dialog transparency level (0 - opaque, 255 - fully transparent)
  void SetAlpha (uint8 iAlpha);

  /// Set dialog overlay transparency level (0 - opaque, 255 - fully transparent)
  void SetOverlayAlpha (uint8 iAlpha);

  /// Query dialog transparency level
  uint8 GetAlpha ()
  { return GetState (CSS_TRANSPARENT) ? Alpha : 0; }

  /// Query dialog overlay transparency level
  uint8 GetOverlayAlpha ()
  { return GetState (CSS_TRANSPARENT) ? OverlayAlpha : 0; }


  /// Set the bitmap for the frame (only useful if the framestyle is csdfsBitmap)
  void SetFrameBitmap(csPixmap *iFrameBitmap, bool iDelFrameBitmap);

  /// Set the bitmap for the overlay (only useful if the framestyle is csdfsBitmap)
  void SetOverlayBitmap(csPixmap *iOverlayBitmap, bool iDelOverlayBitmap);

  /// Get the frame bitmap
  csPixmap *GetFrameBitmap()
  { return FrameBitmap; }

  /// Get the overlay bitmap
  csPixmap *GetOverlayBitmap()
  { return OverlayBitmap; }

protected:
  /// Adjust focused control by switching back or forth if it is disabled
  void AdjustFocused (bool forward);
  /// Used by SuggestSize
  static bool do_topleft (csComponent *comp, intptr_t param);
  /// Place all dialog items in correspondence to GridX, GridY and SnapSizeToGrid
  bool PlaceItems ();
};

/** @} */

#endif // __CS_CSDIALOG_H__
