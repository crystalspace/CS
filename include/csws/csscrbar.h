/*
    Crystal Space Windowing System: scroll bar class
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

#ifndef __CS_CSSCRBAR_H__
#define __CS_CSSCRBAR_H__

/**\file
 * Crystal Space Windowing System: scroll bar class
 */

/**
 * \addtogroup csws_comps_scrbar
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"
#include "csbutton.h"
#include "cstimer.h"

/// Recommended scroll bar size (width or height)
#define CSSB_DEFAULTSIZE	(3+3+9)

/// Possible scrollbar frame styles
enum csScrollBarFrameStyle
{
  /// Scroll bar has a thick rectangular frame
  cssfsThickRect,
  /// Scroll bar has a thin rectangular frame
  cssfsThinRect
};

/// Scroll bar range structure
struct csScrollBarStatus
{
  /// Knob position and max position
  int value, maxvalue;
  /// Knob size and max size
  int size, maxsize;
  /// Scroll bar jump steps
  int step, pagestep;
};

/// csScrollBar class messages
enum
{
  /**
   * Set scroll bar range, value etc.
   * <pre>
   * IN:  (csScrollBarStatus *)status
   * OUT: 0 if successful
   * </pre>
   */
  cscmdScrollBarSet = 0x00000600,
  /**
   * Query scroll bar status
   * <pre>
   * IN:  (csScrollBarStatus *)status
   * OUT: 0 if successful
   * </pre>
   */
  cscmdScrollBarGetStatus,
  /**
   * Scroll bar value changed (parent notification)
   * <pre>
   * IN: (csScrollBar *)source
   * </pre>
   */
  cscmdScrollBarValueChanged,
  /**
   * Query just the scroll bar value
   * <pre>
   * OUT: (int)value
   * </pre>
   */
  cscmdScrollBarQueryValue,
  /**
   * Set just the scroll bar value
   * <pre>
   * IN: (int)value
   * </pre>
   */
  cscmdScrollBarSetValue
};

/**
 * The ScrollBar component class is used to scroll left/right or
 * up/down windows whose content does not fit into their size.
 * If scroll bar's width is bigger than its height, it becomes a
 * horizontal scroll bar, otherwise it is a vertical scroll bar.
 * A scroll bar notifies its parent with a command event when
 * user requests (using mouse) to scroll window contents.
 * Parent can set scroll bar's position/range using command
 * messages.
 */
class CS_CRYSTALSPACE_EXPORT csScrollBar : public csComponent
{
  /// Scroll bar frame style
  csScrollBarFrameStyle FrameStyle;
  /// The scroll buttons and the scroller
  csButton *topleft, *botright, *scroller;
  /// The repeat timer
  csTimer *timer;
  /// Active button ID
  int active_button;
  /// true if scroll bar is horizontal
  bool IsHorizontal;
  /// true if tracking scroller
  bool TrackScroller;
  /// Scroller tracking initial position
  int scrollerdx, scrollerdy;
  /// Length of active portion of scrollbar
  int activepixlen;
  /// Images of arrows
  static csPixmap *sprarrows[12];
  /// Image of scroller
  static csPixmap *sprscroller[2];
  /// Current scroll bar status
  csScrollBarStatus status;

public:
  /// Create static component object (by default - a label) linked to another
  csScrollBar (csComponent *iParent, csScrollBarFrameStyle iFrameStyle = cssfsThickRect);

  /// Destroy the scroll bar
  virtual ~csScrollBar ();

  /// Handle scroll button notifications
  virtual bool HandleEvent (iEvent &Event);

  /// Set scroll button positions on resize
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /// Override SetState method to disable scroll buttons as well
  virtual void SetState (int mask, bool enable);

  /**
   * Accessors
   */

  /// Get frame style
  csScrollBarFrameStyle GetFrameStyle()
  { return FrameStyle; }

  /// Get active button
  int GetActiveButton()
  { return active_button; }

  /// Returns true if scrollbar is horizontal
  bool GetIsHorizontal()
  { return IsHorizontal; }

  /// Get the scroller button
  csButton *GetScroller()
  { return scroller; }

  /// Get the top or left arrow button
  csButton *GetTopLeft()
  { return topleft; }

  /// Get the bottom or right arrow button
  csButton *GetBotRight()
  { return botright; }

  /// Get the name of the skip slice for this component
  virtual char *GetSkinName ()
  { return "ScrollBar"; }

protected:
  /// Set scroll bar value
  void SetValue (int iValue);
};

/** @} */

#endif // __CS_CSSCRBAR_H__
