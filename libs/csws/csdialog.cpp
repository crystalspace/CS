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

#include "cssysdef.h"
#include "csutil/csinput.h"
#include "csws/cswindow.h"
#include "csws/csdialog.h"
#include "csws/csskin.h"
#include "iutil/event.h"
#include "csutil/event.h"

#define SKIN ((csDialogSkin *)skinslice)

csDialog::csDialog (csComponent *iParent, csDialogFrameStyle iFrameStyle)
  : csComponent (iParent), FrameBitmap(0),  OverlayBitmap(0),
    delFrameBitmap(false), delOverlayBitmap(false)
{
  SetPalette (CSPAL_DIALOG);
  state |= CSS_SELECTABLE;
  DragStyle = 0;
  OverlayAlpha=0;
  first = 0;
  GridX = GridY = -1;
  SnapSizeToGrid = false;
  if (parent)
    parent->SendCommand (cscmdWindowSetClient, (intptr_t)this);

  // If our parent is a dialog as well, mark ourselves as transparent
  // to avoid untiled textures as backgrounds.
  if (parent)
  {
    char *ps = parent->GetSkinName ();
    if (ps && !strcmp (ps, "Dialog"))
      SetAlpha (255);
  }

  if (FrameStyle == csdfsBitmap)
    SetState (CSS_TRANSPARENT, true);

  ApplySkin (GetSkin ());
  SetFrameStyle (iFrameStyle);
}

csDialog::~csDialog()
{
 if (delFrameBitmap && FrameBitmap) delete FrameBitmap;
 if (delOverlayBitmap && OverlayBitmap) delete OverlayBitmap;
}

bool csDialog::HandleEvent (iEvent &Event)
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
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case CSKEY_TAB:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_SHIFT)
	    {
	      SetFocused (PrevGroup ());
	      AdjustFocused (false);
	      return true;
	    }
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	    {
	      SetFocused (NextGroup ());
	      AdjustFocused (true);
	      return true;
	    } /* endif */
	    break;
	  case CSKEY_LEFT:
	  case CSKEY_UP:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	    {
	      SetFocused (PrevControl ());
	      AdjustFocused (false);
	      return true;
	    }
	    break;
	  case CSKEY_RIGHT:
	  case CSKEY_DOWN:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	    {
	      SetFocused (NextControl ());
	      AdjustFocused (true);
	      return true;
	    }
	    break;
	  case CSKEY_ENTER:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	    {
	      csComponent *def = GetDefault ();
	      if (def->SendCommand (cscmdActivate, 0) != (intptr_t)def)
		if (def != focused)
		  focused->SendCommand (cscmdActivate, 0);
	      return true;
	    }
	    break;
	} /* endswitch */
      }
      break;
    case csevCommand:
      if (parent)
        return parent->HandleEvent (Event);
      break;
  } /* endswitch */
  return false;
}

void csDialog::AdjustFocused (bool forward)
{
  int i = 10;
  while (i--)
  {
    if (!focused->GetState (CSS_DISABLED))
      break;
    SetFocused (forward ? NextControl () : PrevControl ());
  }
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

static bool do_checksize (csComponent *comp, intptr_t param)
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

bool csDialog::do_topleft (csComponent *comp, intptr_t param)
{
  csDialog *self = (csDialog *)param;
  comp->SetPos (self->BorderWidth + self->GridX,
    self->BorderHeight + self->GridY);
  return false;
}

void csDialog::SuggestSize (int &w, int &h)
{
  // If we have no children, we have some default size
  if (!top)
  {
    w = h = 8;
    return;
  }

  /// If we never auto-placed the items, place them at top-left corner
  if ((GridX >= 0) && (GridY >= 0) && !first)
    ForEach (do_topleft, (intptr_t)this);
  csRect rect (999999, 999999, 0, 0);
  ForEach (do_checksize, (intptr_t)&rect);
  if (rect.xmin == 999999)
    rect.xmin = BorderWidth;
  if (rect.ymin == 999999)
    rect.ymin = BorderHeight;
  w = rect.xmin + rect.xmax;
  h = rect.ymin + rect.ymax;
}

void csDialog::FixSize (int &newW, int &newH)
{
  if ((GridX >= 0) && (GridY >= 0))
  {
    int minw,minh;
    minw = (BorderWidth + GridX) * 2;
    minh = (BorderHeight + GridY) * 2;
    if (first)
    {
      minw += first->bound.Width ();
      minh += first->bound.Height ();
    }
    // Don't allow too small windows
    if (newW < minw) newW = minw;
    if (newH < minh) newH = minh;
  }
}

void csDialog::SetBorderSize (int w, int h)
{
  BorderWidth = w;
  BorderHeight = h;
  csComponent::SetRect (bound);
}

void csDialog::SetFrameStyle (csDialogFrameStyle iFrameStyle)
{
  FrameStyle = iFrameStyle;
  SKIN->SetBorderSize (*this);
  csComponent::SetRect (bound);
  Invalidate ();
}

void csDialog::SetAlpha (uint8 iAlpha)
{
  Alpha = iAlpha;

  if (Alpha || FrameStyle == csdfsBitmap)
    SetState (CSS_TRANSPARENT, true);
  else
    SetState (CSS_TRANSPARENT, false);
}

void csDialog::SetOverlayAlpha (uint8 iAlpha)
{
  OverlayAlpha = iAlpha;

  if (Alpha || FrameStyle == csdfsBitmap)
    SetState (CSS_TRANSPARENT, true);
  else
    SetState (CSS_TRANSPARENT, false);
}

void
csDialog::SetFrameBitmap(csPixmap *iFrameBitmap, bool iDelFrameBitmap)
{
	// Delete the previous bitmap, if it needs to be
	if (delFrameBitmap && FrameBitmap)
	{
  	  delete FrameBitmap;
	  delFrameBitmap=false;
	  FrameBitmap=0;
	}

	// Set the new one only	if there's something to set
	if (iFrameBitmap) {
	  FrameBitmap = iFrameBitmap;
	  delFrameBitmap = iDelFrameBitmap;
	}
}

void
csDialog::SetOverlayBitmap(csPixmap *iOverlayBitmap, bool iDelOverlayBitmap)
{
	// Delete the previous bitmap, if it needs to be
	if (delOverlayBitmap && OverlayBitmap)
	{
  	  delete OverlayBitmap;
	  delOverlayBitmap=false;
	  OverlayBitmap=0;
	}

	// Set the new one only if there's something to set
	if (iOverlayBitmap) {
	  OverlayBitmap = iOverlayBitmap;
	  delOverlayBitmap = iDelOverlayBitmap;
	}
}


