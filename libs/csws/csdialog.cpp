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

#include "sysdef.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "csws/cswindow.h"
#include "csws/csdialog.h"

csDialog::csDialog (csComponent *iParent, csDialogFrameStyle iFrameStyle)
  : csComponent (iParent)
{
  SetPalette (CSPAL_DIALOG);
  state |= CSS_SELECTABLE;
  FrameStyle = iFrameStyle;
  DragStyle = 0;
  first = NULL;
  GridX = GridY = -1;
  SnapSizeToGrid = false;
  if (parent)
    parent->SendCommand (cscmdWindowSetClient, (void *)this);
  switch (FrameStyle)
  {
    case csdfsNone:
      BorderWidth = 0; BorderHeight = 0;
      break;
    case csdfsHorizontal:
      BorderWidth = 0; BorderHeight = 2;
      break;
    case csdfsVertical:
      BorderWidth = 2; BorderHeight = 0;
      break;
    case csdfsAround:
      BorderWidth = 2; BorderHeight = 2;
      break;
  } /* endswitch */
}

void csDialog::Draw ()
{
  switch (FrameStyle)
  {
    case csdfsNone:
      break;
    case csdfsHorizontal:
      Line (0, 0, bound.Width () - 1, 0, CSPAL_DIALOG_2LIGHT3D);
      Line (0, 1, bound.Width () - 1, 1, CSPAL_DIALOG_LIGHT3D);
      Line (0, bound.Height () - 2, bound.Width () - 1, bound.Height () - 2,
        CSPAL_DIALOG_DARK3D);
      Line (0, bound.Height () - 1, bound.Width () - 1, bound.Height () - 1,
        CSPAL_DIALOG_2DARK3D);
      break;
    case csdfsVertical:
      Line (0, 0, 0, bound.Height () - 1, CSPAL_DIALOG_2LIGHT3D);
      Line (1, 0, 1, bound.Height () - 1, CSPAL_DIALOG_LIGHT3D);
      Line (bound.Width () - 2, 0, bound.Width () - 2, bound.Height () - 1,
        CSPAL_DIALOG_DARK3D);
      Line (bound.Width () - 1, 0, bound.Width () - 1, bound.Height () - 1,
        CSPAL_DIALOG_2DARK3D);
      break;
    case csdfsAround:
      Rect3D (0, 0, bound.Width (), bound.Height (),
        CSPAL_DIALOG_2DARK3D, CSPAL_DIALOG_2LIGHT3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
        CSPAL_DIALOG_DARK3D, CSPAL_DIALOG_LIGHT3D);
      break;
  } /* endswitch */
  Box (BorderWidth, BorderHeight, bound.Width () - BorderWidth,
    bound.Height () - BorderHeight, CSPAL_DIALOG_BACKGROUND);
  csComponent::Draw ();
}

bool csDialog::HandleEvent (csEvent &Event)
{
  if (csComponent::HandleEvent (Event))
    return true;

  switch (Event.Type)
  {
    case csevMouseMove:
    case csevMouseDown:
      if (HandleDragEvent (Event, BorderWidth, BorderHeight))
        return true;
      break;
    case csevKeyDown:
      switch (Event.Key.Code)
      {
        case CSKEY_TAB:
          if ((Event.Key.ShiftKeys & CSMASK_ALLSHIFTS) == CSMASK_SHIFT)
          {
            SetFocused (PrevGroup ());
            return true;
          }
          else if ((Event.Key.ShiftKeys & CSMASK_ALLSHIFTS) == 0)
          {
            SetFocused (NextGroup ());
            return true;
          } /* endif */
          break;
        case CSKEY_LEFT:
        case CSKEY_UP:
          if ((Event.Key.ShiftKeys & CSMASK_ALLSHIFTS) == 0)
          {
            SetFocused (PrevControl ());
            return true;
          }
          break;
        case CSKEY_RIGHT:
        case CSKEY_DOWN:
          if ((Event.Key.ShiftKeys & CSMASK_ALLSHIFTS) == 0)
          {
            SetFocused (NextControl ());
            return true;
          }
          break;
        case CSKEY_ENTER:
          if ((Event.Key.ShiftKeys & CSMASK_ALLSHIFTS) == 0)
          {
            csComponent *def = GetDefault ();
            if (def->SendCommand (cscmdActivate, NULL) != def)
              if (def != focused)
                focused->SendCommand (cscmdActivate, NULL);
            return true;
          }
          break;
      } /* endswitch */
      break;
    case csevCommand:
      if (parent)
        return parent->HandleEvent (Event);
      break;
  } /* endswitch */
  return false;
}

bool csDialog::PlaceItems ()
{
  if ((GridX >= 0) && (GridY >= 0))
  {
    if (!first)
      first = focused;
    if (!first)
      return true;

    csComponent *cur = first;
    int curX = BorderWidth + GridX, curY = BorderHeight + GridY;
    int maxX = -1, maxY = -1;
    do
    {
      int lastX, lastY;
      for ( ; ; )
      {
        lastX = curX + cur->bound.Width ();
        lastY = curY + cur->bound.Height ();
        if (lastX > bound.Width () - BorderWidth)
        {
          if (curX == BorderWidth + GridX)
            break;
          curX = BorderWidth + GridX;
          curY = maxY + GridY;
        }
        else
          break;
      } /* endfor */
      if (!cur->bound.IsEmpty ())
      {
        cur->SetRect (curX, curY, lastX, lastY);
        if (lastX > maxX)
          maxX = lastX;
        if (lastY > maxY)
          maxY = lastY;
        curX = lastX + GridX;
      } /* endif */
      cur = cur->next;
    } while (cur != first); /* enddo */
    if (SnapSizeToGrid)
      return csComponent::SetRect (bound.xmin, bound.ymin,
        bound.xmin + maxX + GridX + BorderWidth,
        bound.ymin + maxY + GridY + BorderHeight);
  } /* endif */
  return true;
}

bool csDialog::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csComponent::SetRect (xmin, ymin, xmax, ymax))
    return PlaceItems ();
  return false;
}

static bool do_checksize (csComponent *comp, void *param)
{
  csRect *r = (csRect *)param;
  if (comp->bound.xmin < r->xmin)
    r->xmin = comp->bound.xmin;
  if (comp->bound.ymin < r->ymin)
    r->ymin = comp->bound.ymin;
  if (comp->bound.xmax > r->xmax)
    r->xmax = comp->bound.xmax;
  if (comp->bound.ymax > r->ymax)
    r->ymax = comp->bound.ymax;
  return false;
}

bool csDialog::do_topleft (csComponent *comp, void *param)
{
  csDialog *self = (csDialog *)param;
  comp->SetPos (self->BorderWidth + self->GridX,
    self->BorderHeight + self->GridY);
  return false;
}

void csDialog::SuggestSize (int &w, int &h)
{
  /// If we never auto-placed the items, place them at top-left corner
  if ((GridX >= 0) && (GridY >= 0) && !first)
    ForEach (do_topleft, this);
  csRect rect (999999, 999999, 0, 0);
  ForEach (do_checksize, &rect);
  if (rect.xmin == 999999)
    rect.xmin = BorderWidth;
  if (rect.ymin == 999999)
    rect.ymin = BorderHeight;
  w = rect.xmin + rect.xmax;
  h = rect.ymin + rect.ymax;
}

void csDialog::FixSize (int &newW, int &newH)
{
  int minw,minh;
  if ((GridX >= 0) && (GridY >= 0))
  {
    minw = (BorderWidth + GridX) * 2;
    minh = (BorderHeight + GridY) * 2;
    if (first)
    {
      minw += first->bound.Width ();
      minh += first->bound.Height ();
    }
  }
  else
    SuggestSize (minw, minh);

  // Don't allow too small windows
  if (newW < minw) newW = minw;
  if (newH < minh) newH = minh;
}
