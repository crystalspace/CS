/*
    Crystal Space Windowing System: list box class
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

#include <ctype.h>

#include "csws/cslistbx.h"
#include "csws/cstimer.h"
#include "csws/csscrbar.h"
#include "csws/csapp.h"
#include "csws/csskin.h"
#include "csutil/event.h"

// Amount of space at left and at right of each listbox item
#define LISTBOXITEM_XSPACE              2
// Amount of space at top and at bottom of each listbox item
#define LISTBOXITEM_YSPACE              2

// Mouse scroll time interval in milliseconds
#define MOUSE_SCROLL_INTERVAL           100

// Horizontal large scrolling step
#define LISTBOX_HORIZONTAL_PAGESTEP     8

csListBoxItem::csListBoxItem (csComponent *iParent, const char *iText, ID iID,
  csListBoxItemStyle iStyle) : csComponent (iParent),
    ItemStyle(iStyle), deltax(0),  ItemBitmap(0),  DeleteBitmap(false),
    hOffset(0)
{
  state |= CSS_SELECTABLE | CSS_TRANSPARENT;
  id = iID;
  SetText (iText);
  SetPalette (CSPAL_LISTBOXITEM);
  ApplySkin (GetSkin ());
}

csListBoxItem::~csListBoxItem ()
{
  if (DeleteBitmap)
    delete ItemBitmap;
}

void csListBoxItem::SuggestSize (int &w, int &h)
{
  w = hOffset; h = 0;

  if (ItemBitmap)
  {
    w += ItemBitmap->Width () + LISTBOXITEM_XSPACE;
    int _h = ItemBitmap->Height ();
    if (h < _h) h = _h;
  } /* endif */

  if (text && parent)
  {
    int fh, fw = GetTextSize (text, &fh);
    w += fw;
    if (h < fh) h = fh;
  } /* endif */

  // Leave a bit of space at left, right, top and bottom
  w += LISTBOXITEM_XSPACE * 2;
  h += LISTBOXITEM_YSPACE * 2;
}

bool csListBoxItem::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseDown:
      if (parent->GetState (CSS_DISABLED))
        return true;
      if (Event.Mouse.Button == 1)
      {
        parent->SendCommand (cscmdListBoxStartTracking, (intptr_t)this);
        parent->SendCommand (cscmdListBoxItemClicked, (intptr_t)this);
      }
      return true;
    case csevMouseMove:
      if ((app->MouseOwner == parent)
       && !GetState (CSS_FOCUSED))
        parent->SendCommand (cscmdListBoxTrack, (intptr_t)this);
      return true;
    case csevMouseDoubleClick:
      if ((Event.Mouse.Button == 1) && parent)
        parent->SendCommand (cscmdListBoxItemDoubleClicked, (intptr_t)this);
      return true;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdListBoxItemCheck:
          Event.Command.Info = GetState (CSS_LISTBOXITEM_SELECTED)
            ? CS_LISTBOXITEMCHECK_SELECTED
            : CS_LISTBOXITEMCHECK_UNSELECTED;
          return true;
        case cscmdListBoxItemSet:
          if (Event.Command.Info)
            parent->SetFocused (this);
          SetState (CSS_LISTBOXITEM_SELECTED, Event.Command.Info ? true : false);
          return true;
        case cscmdListBoxItemScrollVertically:
          if (bound.IsEmpty ())
            Event.Command.Info = true;
          else
          {
            int w,h;
            SuggestSize (w, h);
            if (bound.Height () < h)
              Event.Command.Info = true;
          } /* endif */
          return true;
        case cscmdListBoxItemSetHorizOffset:
          if (deltax != (int)Event.Command.Info)
          {
            deltax = (int)Event.Command.Info;
            if (parent->GetState(CSS_TRANSPARENT))
            {
               parent->parent->Invalidate();
               parent->Invalidate(true);
             }
            Invalidate ();
          } /* endif */
          return true;
      } /* endswitch */
      break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

void csListBoxItem::SetState (int mask, bool enable)
{
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if ((oldstate ^ state) & CSS_LISTBOXITEM_SELECTED)
  {
    if (parent->GetState(CSS_TRANSPARENT))
    {
      parent->parent->Invalidate();
      parent->Invalidate(true);
    }
    Invalidate ();
    parent->SendCommand (GetState (CSS_LISTBOXITEM_SELECTED) ?
      cscmdListBoxItemSelected : cscmdListBoxItemDeselected, (intptr_t)this);
  } /* endif */
  if ((oldstate ^ state) & CSS_FOCUSED)
  {
    if (parent->GetState(CSS_TRANSPARENT))
    {
      parent->parent->Invalidate();
      parent->Invalidate(true);
    }
    Invalidate ();
    if (GetState (CSS_FOCUSED))
      parent->SendCommand (cscmdListBoxMakeVisible, (intptr_t)this);
  } /* endif */
}

void csListBoxItem::SetBitmap (csPixmap *iBitmap, bool iDelete)
{
  if (DeleteBitmap)
    delete ItemBitmap;
  ItemBitmap = iBitmap;
  DeleteBitmap = iDelete;

  if (parent->GetState(CSS_TRANSPARENT))
  {
    parent->parent->Invalidate();
    parent->Invalidate(true);
  }
  Invalidate ();
}

//---------------------------------------------------// csListBox //----------//

csListBox::csListBox (csComponent *iParent, int iStyle,
  csListBoxFrameStyle iFrameStyle) : csComponent (iParent),
    ListBoxStyle(iStyle),  FrameStyle(iFrameStyle),
    deltax(0), fPlaceItems(false),
    FrameBitmap(0), fDelFrameBitmap(false), FrameAlpha(0)

{
  state |= CSS_SELECTABLE;
  SetPalette (CSPAL_LISTBOX);
  csScrollBarFrameStyle sbsty;

  switch (FrameStyle)
  {
    case cslfsNone:
      BorderWidth = BorderHeight = 0;
      sbsty = cssfsThinRect;
      break;
    case cslfsThinRect:
      BorderWidth = BorderHeight = 2;
      sbsty = cssfsThinRect;
      break;
    case cslfsThickRect:
      BorderWidth = BorderHeight = 2;
      sbsty = cssfsThickRect;
      break;
     case cslfsTextured:
       BorderWidth=BorderHeight=2;
       sbsty=cssfsThinRect;
       state|=CSS_TRANSPARENT;
       break;
     case cslfsTexturedNoFrame:
     case cslfsBitmap:
       BorderWidth=BorderHeight=0;
       sbsty=cssfsThinRect;
       state|=CSS_TRANSPARENT;
       break;
    default:
      return;
  } /* endswitch */
  firstvisible = first = new csTimer (this, MOUSE_SCROLL_INTERVAL);
  if (iStyle & CSLBS_HSCROLL)
    hscroll = new csScrollBar (this, sbsty);
  else
    hscroll = 0;
  if (iStyle & CSLBS_VSCROLL)
    vscroll = new csScrollBar (this, sbsty);
  else
    vscroll = 0;


   ApplySkin (GetSkin ());
}

csListBox::~csListBox()
{
   if (fDelFrameBitmap && FrameBitmap)
     delete FrameBitmap;
}

void csListBox::PlaceItems (bool setscrollbars)
{
  int cury = BorderHeight;
  bool vis = false;

  fPlaceItems = false;

  // if focused item is not selectable, find next selectable item
  if (focused && !focused->GetState (CSS_SELECTABLE))
  {
    csComponent *cur = focused;
    do
    {
      cur = cur->next;
      if (unsigned (cur->SendCommand (cscmdListBoxItemCheck, 0)) == CS_LISTBOXITEMCHECK_SELECTED)
        break;
    } while (cur != focused);
    if (cur == focused)
    {
      SetFocused (NextChild (focused));
      focused->SetState (CSS_LISTBOXITEM_SELECTED, true);
    }
    else
      focused = cur;
  } /* endif */

  csComponent *cur = first;
  csRect itembound;
  csRect clipbound (BorderWidth, BorderHeight, bound.Width () - BorderWidth,
    bound.Height () - BorderHeight);
  if (hscroll)
    clipbound.ymax = hscroll->bound.ymin;
  if (vscroll)
    clipbound.xmax = vscroll->bound.xmin;
  vertcount = 0;
  // collect listbox statistics
  maxdeltax = 0;
  int itemcount = 0;
  int numfirst = 0;
  bool foundfirst = false;
  cur = first;
  while (cur)
  {
    if (cur == firstvisible)
    {
      foundfirst = true;
      // start reserving space from first visible item
      vis = true;
    } /* endif */

    int w, h;
    cur->SuggestSize (w, h);
    if (w && h)
    {
      itemcount++;
      if (!foundfirst)
        numfirst++;
      if (w > maxdeltax)
        maxdeltax = w;
    } /* endif */

    if (vis)
    {
      // Query current item width and height
      if (h > 0)
      {
        // Set current item x,y and height
        itembound.Set (BorderWidth, cury, bound.Width (), cury + h);
        itembound.Intersect (clipbound);
        if (!itembound.IsEmpty ())
          vertcount++;
        cur->SetRect (itembound);
        cur->SetState (CSS_VISIBLE, true);
        cur->SendCommand (cscmdListBoxItemSetHorizOffset, (intptr_t)deltax);
        cury += h;
      } /* endif */
    }
    else if (w && h)
    {
      cur->SetState (CSS_VISIBLE, false);
      // Also set the rectangle to invalid
      cur->SetRect (0, 0, -1, -1);
    }
    cur = cur->next;
    if (cur == first)
      break;
  } /* endwhile */

  if (setscrollbars)
  {
    if (vscroll)
    {
      vsbstatus.value = numfirst;
      vsbstatus.maxvalue = itemcount - vertcount;
      vsbstatus.size = vertcount;
      vsbstatus.maxsize = itemcount;
      vsbstatus.step = 1;
      vsbstatus.pagestep = vertcount;
      vscroll->SendCommand (cscmdScrollBarSet, (intptr_t)&vsbstatus);
    } /* endif */

    int maxw = maxdeltax;
    maxdeltax -= clipbound.Width ();
    if (maxdeltax < 0) maxdeltax = 0;
    if (hscroll)
    {
      hsbstatus.size = clipbound.Width ();
      hsbstatus.maxsize = maxw;
      hsbstatus.value = deltax;
      hsbstatus.maxvalue = maxdeltax;
      hsbstatus.step = 1;
      hsbstatus.pagestep = LISTBOX_HORIZONTAL_PAGESTEP;
      hscroll->SendCommand (cscmdScrollBarSet, (intptr_t)&hsbstatus);
    } /* endif */
  } /* endif */
}


bool csListBox::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csComponent::SetRect (xmin, ymin, xmax, ymax))
  {
    if (hscroll)
      hscroll->SetRect (0, bound.Height () - CSSB_DEFAULTSIZE,
        bound.Width () - (vscroll ? CSSB_DEFAULTSIZE - 1 : 0),
        bound.Height ());
    if (vscroll)
      vscroll->SetRect (bound.Width () - CSSB_DEFAULTSIZE, 0,
        bound.Width (),
        bound.Height () - (hscroll ? CSSB_DEFAULTSIZE - 1 : 0));
    fPlaceItems = true;
    return true;
  } else
    return false;
}

static bool do_select (csComponent *child, intptr_t param)
{
  if (child != (csComponent *)param)
    child->SetState (CSS_LISTBOXITEM_SELECTED, true);
  return false;
}

static bool do_deselect (csComponent *child, intptr_t param)
{
  if (child != (csComponent *)param)
    child->SetState (CSS_LISTBOXITEM_SELECTED, false);
  return false;
}

static bool do_deleteitem (csComponent *child, intptr_t param)
{
  (void)param;
  delete child;
  return false;
}

static bool do_true (csComponent *child, intptr_t param)
{
  (void)child; (void)param;
  return true;
}

static bool do_findtext (csComponent *child, intptr_t param)
{
  return (strcmp (child->GetText (), (char *)param) == 0);
}

bool csListBox::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseUp:
      if ((Event.Mouse.Button == 1) && (app->MouseOwner == this))
      {
        app->CaptureMouse (0);
        return true;
      } /* endif */
      break;
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case CSKEY_UP:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      (CSMASK_SHIFT | CSMASK_ALT)) == 0)
	    {
	      bool mc = (csKeyEventHelper::GetModifiersBits (&Event) & 
		CSMASK_CTRL) && !app->MouseOwner;
	      if (mc) app->CaptureMouse (this);
	      csComponent *comp = focused->prev;
	      while (mc && comp->GetState (CSS_LISTBOXITEM_SELECTED))
		comp = comp->prev;
	      SendCommand (cscmdListBoxTrack, (intptr_t)comp);
	      if (mc) app->CaptureMouse (0);
	    } /* endif */
	    return true;
	  case CSKEY_DOWN:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      (CSMASK_SHIFT | CSMASK_ALT)) == 0)
	    {
	      bool mc = (csKeyEventHelper::GetModifiersBits (&Event) & 
		CSMASK_CTRL) && !app->MouseOwner;
	      if (mc) app->CaptureMouse (this);
	      csComponent *comp = focused->next;
	      while (mc && comp->GetState (CSS_LISTBOXITEM_SELECTED))
		comp = comp->next;
	      SendCommand (cscmdListBoxTrack, (intptr_t)comp);
	      if (mc) app->CaptureMouse (0);
	    } /* endif */
	    return true;
	  case CSKEY_LEFT:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	    {
	      if (deltax > LISTBOX_HORIZONTAL_PAGESTEP)
		deltax -= LISTBOX_HORIZONTAL_PAGESTEP;
	      else
		deltax = 0;
	      PlaceItems ();
	    }
	    else if (((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0) && (deltax > 0))
	    {
	      deltax--;
	      PlaceItems ();
	    } /* endif */
	    return true;
	  case CSKEY_RIGHT:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	    {
	      if (deltax + LISTBOX_HORIZONTAL_PAGESTEP <= maxdeltax)
		deltax += LISTBOX_HORIZONTAL_PAGESTEP;
	      else
		deltax = maxdeltax;
	      PlaceItems ();
	    }
	    else if (((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0) && (deltax < maxdeltax))
	    {
	      deltax++;
	      PlaceItems ();
	    } /* endif */
	    return true;
	  case CSKEY_PGUP:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	    {
	      for (int i = 0; i < vertcount; i++)
		SendCommand (cscmdListBoxTrack, (intptr_t)focused->prev);
	    }
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
		      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	      SendCommand (cscmdListBoxTrack, (intptr_t)NextChild (first));
	    return true;
	  case CSKEY_PGDN:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
		 CSMASK_ALLSHIFTS) == 0)
	    {
	      for (int i = 0; i < vertcount; i++)
		SendCommand (cscmdListBoxTrack, (intptr_t)focused->next);
	    }
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	      SendCommand (cscmdListBoxTrack, (intptr_t)PrevChild (first));
	    return true;
	  case CSKEY_HOME:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_CTRL) && (deltax != 0))
	    {
	      deltax = 0;
	      PlaceItems ();
	    }
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	      SendCommand (cscmdListBoxTrack, (intptr_t)NextChild (first));
	    return true;
	  case CSKEY_END:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_CTRL) && (deltax != maxdeltax))
	    {
	      deltax = maxdeltax;
	      PlaceItems ();
	    }
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	      SendCommand (cscmdListBoxTrack, (intptr_t)PrevChild (first));
	    return true;
	  case '/':
	    if ((ListBoxStyle & CSLBS_MULTIPLESEL)
	    && ((csKeyEventHelper::GetModifiersBits (&Event) & 
	    CSMASK_ALLSHIFTS) == CSMASK_CTRL))
	      ForEachItem (do_select, 0, false);
	    return true;
	  case '\\':
	    if ((ListBoxStyle & CSLBS_MULTIPLESEL)
	    && ((csKeyEventHelper::GetModifiersBits (&Event) & 
	    CSMASK_ALLSHIFTS) == CSMASK_CTRL))
	      ForEachItem (do_deselect, 0);
	    return true;
	  default:
	    if ((csKeyEventHelper::GetCookedCode (&Event) >= ' ')
	    && (csKeyEventHelper::GetCookedCode (&Event) <= 255)
	    && !(csKeyEventHelper::GetModifiersBits (&Event) & 
	    (CSMASK_CTRL | CSMASK_ALT)))
	    {
	      // Find first next item that starts with this letter
	      csComponent *cur = focused->next;
	      while (cur != focused)
		if (cur->SendCommand (cscmdListBoxItemCheck, 0)
		&& (toupper (cur->GetText () [0]) == 
		toupper (csKeyEventHelper::GetCookedCode (&Event))))
		{
		  SendCommand (cscmdListBoxTrack, (intptr_t)cur);
		  return true;
		}
		else
		  cur = cur->next;
	      return true;
	    }
	} /* endswitch */
      }
      break;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdListBoxClear:
          if (app->MouseOwner == this)
            app->CaptureMouse (0);
          ForEachItem (do_deleteitem, 0, false);
          firstvisible = first;
          return true;
        case cscmdListBoxItemSelected:
          if ((ListBoxStyle & CSLBS_MULTIPLESEL) == 0)
            ForEachItem (do_deselect, Event.Command.Info);
          // fallback to resend
        case cscmdListBoxItemDeselected:
        case cscmdListBoxItemClicked:
        case cscmdListBoxItemDoubleClicked:
          // resend command to parent
          if (parent)
            parent->HandleEvent (Event);
          return true;
        case cscmdListBoxStartTracking:
        {
          csComponent *item = (csComponent *)Event.Command.Info;
          selstate = item->GetState (CSS_LISTBOXITEM_SELECTED) != 0;
          app->CaptureMouse (this);

          SetFocused (item);
          Select ();
          if (ListBoxStyle & CSLBS_MULTIPLESEL)
          {
            if (app->GetKeyState (CSKEY_CTRL))
              item->SetState (CSS_LISTBOXITEM_SELECTED, selstate = !selstate);
            else
            {
              ForEachItem (do_deselect, (intptr_t)item);
              item->SetState (CSS_LISTBOXITEM_SELECTED, selstate = true);
            } /* endif */
          } else
            item->SetState (CSS_LISTBOXITEM_SELECTED, selstate = true);
          break;
        }
        case cscmdListBoxTrack:
        {
          csComponent *item = (csComponent *)Event.Command.Info;
          if (app->MouseOwner != this)
            selstate = true;
          if (item->GetState (CSS_SELECTABLE)
           && item->SendCommand (cscmdListBoxItemCheck))
          {
            if (app->MouseOwner != this)
              ForEachItem (do_deselect, (intptr_t)item);
            SetFocused (item);
            Select ();
            item->SetState (CSS_LISTBOXITEM_SELECTED, (ListBoxStyle & CSLBS_MULTIPLESEL)
             ? selstate : true);
          } /* endif */
          return true;
        }
        case cscmdListBoxMakeVisible:
          MakeItemVisible ((csComponent *)Event.Command.Info);
          return true;
        case cscmdListBoxQueryFirstSelected:
          Event.Command.Info = (intptr_t)ForEachItem (do_true, 0, true);
          return true;
        case cscmdTimerPulse:
          if (app && app->MouseOwner == this)
          {
            GetMousePosition (Event.Mouse.x, Event.Mouse.y);
            if (Event.Mouse.y < BorderHeight)
              SendCommand (cscmdListBoxTrack, (intptr_t)focused->prev);
            else if ((Event.Mouse.y > bound.Height () - BorderHeight)
                  || (hscroll
                   && (Event.Mouse.y >= hscroll->bound.ymin)))
                   SendCommand (cscmdListBoxTrack, (intptr_t)focused->next);
          } /* endif */
          return true;
        case cscmdScrollBarValueChanged:
        {
          csScrollBar *bar = (csScrollBar *)Event.Command.Info;
          csScrollBarStatus sbs;
          if (!bar || bar->SendCommand (cscmdScrollBarGetStatus, (intptr_t)&sbs))
            return true;

          if (sbs.maxvalue <= 0)
            return true;

          if (bar == hscroll)
          {
            hsbstatus = sbs;
            if (deltax != hsbstatus.value)
            {
              deltax = hsbstatus.value;
              PlaceItems (false);
            } /* endif */
          } else if (bar == vscroll)
          {
            vsbstatus = sbs;
            csComponent *cur = first;
            do
            {
              if (cur->SendCommand (cscmdListBoxItemCheck, 0))
              {
                if (sbs.value == 0)
                {
                  if (firstvisible != cur)
                  {
                    firstvisible = cur;
                    PlaceItems (false);
                  } /* endif */
                  break;
                } /* endif */
                sbs.value--;
              } /* endif */
              cur = cur->next;
            } while (cur != first); /* enddo */
          } /* endif */
          return true;
        }
        case cscmdListBoxSelectItem:
          Event.Command.Info =
	    (intptr_t)ForEachItem (do_findtext, (intptr_t)Event.Command.Info);
          return true;
      } /* endswitch */
      break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

void csListBox::MakeItemVisible (csComponent *item)
{
  if (!item->SendCommand (cscmdListBoxItemScrollVertically, false))
  {
    // item is already visible
    return;
  }

  csComponent *cur = firstvisible;
  while ((cur != first) && (cur != item))
    cur = cur->prev;

  if (cur == item)
    firstvisible = cur;
  else
  {
    cur = item;
    int cy = bound.Height () - BorderHeight;
    if (hscroll)
      cy = hscroll->bound.ymin;
    firstvisible = item;
    while (firstvisible != first)
    {
      int w, h;
      cur->SuggestSize (w, h);
      cy -= h;
      if (cy < BorderHeight)
        break;
      firstvisible = cur;
      cur = cur->prev;
    } /* endwhile */
  } /* endif */
  PlaceItems ();
}

csComponent *csListBox::ForEachItem (bool (*func) (csComponent *child,
  intptr_t param), intptr_t param, bool iSelected)
{
  if (!func)
    return 0;

  csComponent *start = first;
  csComponent *cur = start;
  while (cur)
  {
    csComponent *next = cur->next;

    unsigned reply = (long)cur->SendCommand (cscmdListBoxItemCheck, 0);
    bool ok;
    if (iSelected)
      ok = (reply == CS_LISTBOXITEMCHECK_SELECTED);
    else
      ok = (reply != 0);
    if (ok && func (cur, param))
      return cur;
    if ((cur == next) || ((cur = next) == start))
      break;
  } /* endwhile */
  return 0;
}

void csListBox::SetState (int mask, bool enable)
{
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if ((oldstate ^ state) & CSS_DISABLED)
  {
    bool dis = GetState (CSS_DISABLED) ? true : false;
    if (hscroll)
      hscroll->SetState (CSS_DISABLED, dis);
    if (vscroll)
      vscroll->SetState (CSS_DISABLED, dis);
  }
}

bool csListBox::SetFocused (csComponent *comp)
{
  if (!csComponent::SetFocused (comp))
    return false;
  if (parent)
  {
    // Check if it is for real a list box item
    unsigned rc = (unsigned)comp->SendCommand (cscmdListBoxItemCheck, 0);
    if (rc == CS_LISTBOXITEMCHECK_SELECTED
     || rc == CS_LISTBOXITEMCHECK_UNSELECTED)
      parent->SendCommand (cscmdListBoxItemFocused, (intptr_t)comp);
  }
  return true;
}

void csListBox::Insert (csComponent *comp)
{
  fPlaceItems = true;
  if (GetState(CSS_TRANSPARENT))
  {
    parent->Invalidate();
    //if (hscroll) hscroll->Invalidate();
    //if (vscroll) vscroll->Invalidate();
  }
  Invalidate (true, this);
  csComponent::Insert (comp);
}

void csListBox::Delete (csComponent *comp)
{
  fPlaceItems = true;
  if (GetState(CSS_TRANSPARENT))
  {
    parent->Invalidate();
    //if (hscroll) hscroll->Invalidate();
    //if (vscroll) vscroll->Invalidate();
  }

  Invalidate (true, this);
  csComponent::Delete (comp);
}

void csListBox::SuggestSize (int &w, int &h)
{
#define SKIN ((csListBoxSkin *)skinslice)

  SKIN->SuggestSize (*this, w, h);

#undef SKIN
}

void csListBox::GetBorderSize(int *iBorderWidth,  int *iBorderHeight)
{
   if (iBorderWidth)  *iBorderWidth =BorderWidth;
   if (iBorderHeight) *iBorderHeight=BorderHeight;
}

void csListBox::SetFrameBitmap(csPixmap *iFrameBitmap, bool iDelFrameBitmap)
{
  if (iFrameBitmap)
  {
    FrameBitmap=iFrameBitmap;
    fDelFrameBitmap=iDelFrameBitmap;
  }
}

void csListBox::SetTexture(csPixmap *iTexture, bool iDelFrameBitmap)
{
  if (iTexture)
  {
    FrameBitmap=iTexture;
    fDelFrameBitmap=iDelFrameBitmap;
  }
}

void csListBox::SetAlpha(uint8 iAlpha)
{
 FrameAlpha = iAlpha;
}

