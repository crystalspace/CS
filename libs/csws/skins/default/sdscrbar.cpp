/*
    Crystal Space Windowing System: Default scrollbar skin
    Copyright (C) 2001 Christopher Nelson

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

#include "cssysdef.h"
#include "csws/csscrbar.h"
#include "csws/sdefault.h"

/**
 *
 *  These are defined also in csscrbar.cpp.  If you make modifications, be sure to update BOTH places.
 *
 */

// Minimal scroll button size
#define CSSB_MINIMAL_KNOBSIZE	(3+3+8)
/// Minimal scroll bar size
#define CSSB_MINIMAL_SIZE	(2+2+7)

// Scrolling state
#define SCROLL_UL			1001	// scroll up or left (h/v scrollbars)
#define SCROLL_DR			1002	// scroll down or right
#define SCROLL_PAGE_UL		1003	// scroll up or left by pages
#define SCROLL_PAGE_DR		1004	// scroll down or right by pages

// Period of time to wait before scroll autorepeat
#define SCROLL_START_INTERVAL   500
// Scroll time interval in milliseconds
#define SCROLL_REPEAT_INTERVAL  100

#define SCROLLBAR_TEXTURE_NAME  "csws::ScrollBar"


void csDefaultScrollBarSkin::Draw(csComponent &This)
{

#define This ((csScrollBar &)This)

  int dx, dy;
  switch (This.GetFrameStyle())
  {
    case cssfsThickRect:
      This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
        CSPAL_SCROLLBAR_LIGHT3D, CSPAL_SCROLLBAR_DARK3D);
      This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1,
        CSPAL_SCROLLBAR_LIGHT3D, CSPAL_SCROLLBAR_DARK3D);
      dx = dy = 2;
      break;
    case cssfsThinRect:
      This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
        CSPAL_SCROLLBAR_LIGHT3D, CSPAL_SCROLLBAR_DARK3D);
      dx = dy = 1;
      break;
    default:
      return;
  } /* endswitch */

  int c1 = CSPAL_SCROLLBAR_BACKGROUND;
  int c2 = CSPAL_SCROLLBAR_BACKGROUND;

  if (This.GetActiveButton() == SCROLL_PAGE_UL)
    c1 = CSPAL_SCROLLBAR_SELBACKGROUND;
  else if (This.GetActiveButton() == SCROLL_PAGE_DR)
    c2 = CSPAL_SCROLLBAR_SELBACKGROUND;

  if (This.GetIsHorizontal())
  {
    This.Box (dx, dy, This.GetScroller()->bound.xmin, This.bound.Height () - dy, c1);
    This.Box (This.GetScroller()->bound.xmax, dy, This.bound.Width () - 1, This.bound.Height () - dy, c2);
  } else
  {
    This.Box (dx, dy, This.bound.Width () - dx, This.GetScroller()->bound.ymin, c1);
    This.Box (dx, This.GetScroller()->bound.ymax, This.bound.Width () - dx, This.bound.Height () - 1, c2);
  } /* endif */

#undef This
}

