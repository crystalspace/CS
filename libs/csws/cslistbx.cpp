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

#define SYSDEF_CASE
#include "cssysdef.h"
#include "csws/cslistbx.h"
#include "csws/cstimer.h"
#include "csws/csscrbar.h"
#include "csws/csapp.h"

// Amount of space at left and at right of each listbox item
#define LISTBOXITEM_XSPACE              2
// Amount of space at top and at bottom of each listbox item
#define LISTBOXITEM_YSPACE              2

// Mouse scroll time interval in milliseconds
#define MOUSE_SCROLL_INTERVAL           100

// Horizontal large scrolling step
#define LISTBOX_HORIZONTAL_PAGESTEP     8

csListBoxItem::csListBoxItem (csComponent *iParent, const char *iText, int iID,
  csListBoxItemStyle iStyle) : csComponent (iParent)
{
  state |= CSS_SELECTABLE | CSS_TRANSPARENT;
  SetText (iText);
  id = iID;
  ItemStyle = iStyle;
  ItemBitmap = NULL;
  DeleteBitmap = false;
  hOffset = 0;
  deltax = 0;
  SetPalette (CSPAL_LISTBOXITEM);
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
        parent->SendCommand (cscmdListBoxStartTracking, this);
        parent->SendCommand (cscmdListBoxItemClicked, this);
      }
      return true;
    case csevMouseMove:
      if ((app->MouseOwner == parent)
       && !GetState (CSS_FOCUSED))
        parent->SendCommand (cscmdListBoxTrack, this);
      return true;
    case csevMouseDoubleClick:
      if ((Event.Mouse.Button == 1) && parent)
        parent->SendCommand (cscmdListBoxItemDoubleClicked, this);
      return true;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdListBoxItemCheck:
          Event.Command.Info = GetState (CSS_LISTBOXITEM_SELECTED)
            ? (void *)CS_LISTBOXITEMCHECK_SELECTED
            : (void *)CS_LISTBOXITEMCHECK_UNSELECTED;
          return true;
        case cscmdListBoxItemSet:
          if (Event.Command.Info)
            parent->SetFocused (this);
          SetState (CSS_LISTBOXITEM_SELECTED, (bool)Event.Command.Info);
          return true;
        case cscmdListBoxItemScrollVertically:
          if (bound.IsEmpty ())
            Event.Command.Info = (void *)true;
          else
          {
            int w,h;
            SuggestSize (w, h);
            if (bound.Height () < h)
              Event.Command.Info = (void *)true;
          } /* endif */
          return true;
        case cscmdListBoxItemSetHorizOffset:
          if (deltax != (int)Event.Command.Info)
          {
            deltax = (int)Event.Command.Info;
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
    Invalidate ();
    parent->SendCommand (GetState (CSS_LISTBOXITEM_SELECTED) ?
      cscmdListBoxItemSelected : cscmdListBoxItemDeselected, this);
  } /* endif */
  if ((oldstate ^ state) & CSS_FOCUSED)
  {
    Invalidate ();
    if (GetState (CSS_FOCUSED))
      parent->SendCommand (cscmdListBoxMakeVisible, this);
  } /* endif */
}

void csListBoxItem::SetBitmap (csPixmap *iBitmap, bool iDelete)
{
  if (DeleteBitmap)
    delete ItemBitmap;
  ItemBitmap = iBitmap;
  DeleteBitmap = iDelete;
  Invalidate ();
}

void csListBoxItem::Draw ()
{
  bool enabled = !parent->GetState (CSS_DISABLED);
  bool selected = enabled && GetState (CSS_LISTBOXITEM_SELECTED);
  if (selected)
  {
    if (parent->GetState (CSS_FOCUSED) && enabled)
      Clear (CSPAL_LISTBOXITEM_SELECTION);
    else
      Rect3D (0, 0, bound.Width (), bound.Height (),
        CSPAL_LISTBOXITEM_SELECTION, CSPAL_LISTBOXITEM_SELECTION);
  }

  int color;
  if (GetState (CSS_SELECTABLE) && enabled)
  {
    if (ItemStyle == cslisNormal)
      if (selected && parent->GetState (CSS_FOCUSED))
        color = CSPAL_LISTBOXITEM_SNTEXT;
      else
        color = CSPAL_LISTBOXITEM_UNTEXT;
    else
      if (selected && parent->GetState (CSS_FOCUSED))
        color = CSPAL_LISTBOXITEM_SETEXT;
      else
        color = CSPAL_LISTBOXITEM_UETEXT;
  }
  else
    color = CSPAL_LISTBOXITEM_DTEXT;

  int x = LISTBOXITEM_XSPACE - deltax + hOffset;
  if (ItemBitmap)
  {
    Pixmap (ItemBitmap, x, (bound.Height () - ItemBitmap->Height ()) / 2);
    x += ItemBitmap->Width () + LISTBOXITEM_XSPACE;
  } /* endif */
  if (text)
  {
    int fh;
    GetTextSize (text, &fh);
    Text (x, (bound.Height () - fh + 1) / 2, color, -1, text);
  } /* endif */
  csComponent::Draw ();
}

//---------------------------------------------------// csListBox //----------//

csListBox::csListBox (csComponent *iParent, int iStyle,
  csListBoxFrameStyle iFrameStyle) : csComponent (iParent)
{
  state |= CSS_SELECTABLE;
  ListBoxStyle = iStyle;
  FrameStyle = iFrameStyle;
  SetPalette (CSPAL_LISTBOX);
  deltax = 0;
  fPlaceItems = false;
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
    default:
      return;
  } /* endswitch */
  firstvisible = first = new csTimer (this, MOUSE_SCROLL_INTERVAL);
  if (iStyle & CSLBS_HSCROLL)
    hscroll = new csScrollBar (this, sbsty);
  else
    hscroll = NULL;
  if (iStyle & CSLBS_VSCROLL)
    vscroll = new csScrollBar (this, sbsty);
  else
    vscroll = NULL;
}

void csListBox::Draw ()
{
  if (fPlaceItems)
    PlaceItems ();

  switch (FrameStyle)
  {
    case cslfsNone:
      break;
    case cslfsThinRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_LISTBOX_LIGHT3D, CSPAL_LISTBOX_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_LISTBOX_DARK3D, CSPAL_LISTBOX_LIGHT3D);
      break;
    case cslfsThickRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_LISTBOX_LIGHT3D, CSPAL_LISTBOX_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_LISTBOX_2LIGHT3D, CSPAL_LISTBOX_2DARK3D);
      break;
  } /* endswitch */
  Box (BorderWidth, BorderHeight, bound.Width () - BorderWidth,
    bound.Height () - BorderHeight, FrameStyle == cslfsThickRect ?
    CSPAL_LISTBOX_BACKGROUND2 : CSPAL_LISTBOX_BACKGROUND);

  csComponent::Draw ();
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
      if (unsigned (cur->SendCommand (cscmdListBoxItemCheck, NULL)) == CS_LISTBOXITEMCHECK_SELECTED)
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
        cur->SendCommand (cscmdListBoxItemSetHorizOffset, (void *)deltax);
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
      vscroll->SendCommand (cscmdScrollBarSet, &vsbstatus);
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
      hscroll->SendCommand (cscmdScrollBarSet, &hsbstatus);
    } /* endif */
  } /* endif */
}

void csListBox::SuggestSize (int &w, int &h)
{
  w = h = 0;
  if (hscroll)
    h = hscroll->bound.Height ();
  if (vscroll)
    w = vscroll->bound.Width ();

  h = MAX (bound.Height (), h);
  w = MAX (bound.Width (), w);
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

static bool do_select (csComponent *child, void *param)
{
  if (child != (csComponent *)param)
    child->SetState (CSS_LISTBOXITEM_SELECTED, true);
  return false;
}

static bool do_deselect (csComponent *child, void *param)
{
  if (child != (csComponent *)param)
    child->SetState (CSS_LISTBOXITEM_SELECTED, false);
  return false;
}

static bool do_deleteitem (csComponent *child, void *param)
{
  (void)param;
  delete child;
  return false;
}

static bool do_true (csComponent *child, void *param)
{
  (void)child; (void)param;
  return true;
}

static bool do_findtext (csComponent *child, void *param)
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
        app->CaptureMouse (NULL);
        return true;
      } /* endif */
      break;
    case csevKeyDown:
      switch (Event.Key.Code)
      {
        case CSKEY_UP:
          if ((Event.Key.Modifiers & (CSMASK_SHIFT | CSMASK_ALT)) == 0)
          {
            bool mc = (Event.Key.Modifiers & CSMASK_CTRL) && !app->MouseOwner;
            if (mc) app->CaptureMouse (this);
            csComponent *comp = focused->prev;
            while (mc && comp->GetState (CSS_LISTBOXITEM_SELECTED))
              comp = comp->prev;
            SendCommand (cscmdListBoxTrack, (void *)comp);
            if (mc) app->CaptureMouse (NULL);
          } /* endif */
          return true;
        case CSKEY_DOWN:
          if ((Event.Key.Modifiers & (CSMASK_SHIFT | CSMASK_ALT)) == 0)
          {
            bool mc = (Event.Key.Modifiers & CSMASK_CTRL) && !app->MouseOwner;
            if (mc) app->CaptureMouse (this);
            csComponent *comp = focused->next;
            while (mc && comp->GetState (CSS_LISTBOXITEM_SELECTED))
              comp = comp->next;
            SendCommand (cscmdListBoxTrack, (void *)comp);
            if (mc) app->CaptureMouse (NULL);
          } /* endif */
          return true;
        case CSKEY_LEFT:
          if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL)
          {
            if (deltax > LISTBOX_HORIZONTAL_PAGESTEP)
              deltax -= LISTBOX_HORIZONTAL_PAGESTEP;
            else
              deltax = 0;
            PlaceItems ();
          }
          else if (((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0) && (deltax > 0))
          {
            deltax--;
            PlaceItems ();
          } /* endif */
          return true;
        case CSKEY_RIGHT:
          if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL)
          {
            if (deltax + LISTBOX_HORIZONTAL_PAGESTEP <= maxdeltax)
              deltax += LISTBOX_HORIZONTAL_PAGESTEP;
            else
              deltax = maxdeltax;
            PlaceItems ();
          }
          else if (((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0) && (deltax < maxdeltax))
          {
            deltax++;
            PlaceItems ();
          } /* endif */
          return true;
        case CSKEY_PGUP:
          if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
            for (int i = 0; i < vertcount; i++)
              SendCommand (cscmdListBoxTrack, (void *)focused->prev);
          else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL)
            SendCommand (cscmdListBoxTrack, (void *)NextChild (first));
          return true;
        case CSKEY_PGDN:
          if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
            for (int i = 0; i < vertcount; i++)
              SendCommand (cscmdListBoxTrack, (void *)focused->next);
          else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL)
            SendCommand (cscmdListBoxTrack, (void *)PrevChild (first));
          return true;
        case CSKEY_HOME:
          if ((Event.Key.Modifiers & CSMASK_CTRL) && (deltax != 0))
          {
            deltax = 0;
            PlaceItems ();
          }
          else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
            SendCommand (cscmdListBoxTrack, (void *)NextChild (first));
          return true;
        case CSKEY_END:
          if ((Event.Key.Modifiers & CSMASK_CTRL) && (deltax != maxdeltax))
          {
            deltax = maxdeltax;
            PlaceItems ();
          }
          else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
            SendCommand (cscmdListBoxTrack, (void *)PrevChild (first));
          return true;
        case '/':
          if ((ListBoxStyle & CSLBS_MULTIPLESEL)
           && ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL))
            ForEachItem (do_select, NULL, false);
          return true;
        case '\\':
          if ((ListBoxStyle & CSLBS_MULTIPLESEL)
           && ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL))
            ForEachItem (do_deselect, NULL);
          return true;
        default:
          if ((Event.Key.Char >= ' ')
           && (Event.Key.Char <= 255)
           && !(Event.Key.Modifiers & (CSMASK_CTRL | CSMASK_ALT)))
          {
            // Find first next item that starts with this letter
            csComponent *cur = focused->next;
            while (cur != focused)
              if (cur->SendCommand (cscmdListBoxItemCheck, NULL)
               && (UPPERCASE (cur->GetText () [0]) == UPPERCASE (Event.Key.Char)))
              {
                SendCommand (cscmdListBoxTrack, (void *)cur);
                return true;
              }
              else
                cur = cur->next;
            return true;
          }
      } /* endswitch */
      break;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdListBoxClear:
          if (app->MouseOwner == this)
            app->CaptureMouse (NULL);
          ForEachItem (do_deleteitem, NULL, false);
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
              ForEachItem (do_deselect, (void *)item);
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
              ForEachItem (do_deselect, (void *)item);
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
          Event.Command.Info = ForEachItem (do_true, NULL, true);
          return true;
        case cscmdTimerPulse:
          if (app && app->MouseOwner == this)
          {
            GetMousePosition (Event.Mouse.x, Event.Mouse.y);
            if (Event.Mouse.y < BorderHeight)
              SendCommand (cscmdListBoxTrack, (void *)focused->prev);
            else if ((Event.Mouse.y > bound.Height () - BorderHeight)
                  || (hscroll
                   && (Event.Mouse.y >= hscroll->bound.ymin)))
                   SendCommand (cscmdListBoxTrack, (void *)focused->next);
          } /* endif */
          return true;
        case cscmdScrollBarValueChanged:
        {
          csScrollBar *bar = (csScrollBar *)Event.Command.Info;
          csScrollBarStatus sbs;
          if (!bar || bar->SendCommand (cscmdScrollBarGetStatus, &sbs))
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
              if (cur->SendCommand (cscmdListBoxItemCheck, NULL))
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
          Event.Command.Info = ForEachItem (do_findtext, (char *)Event.Command.Info);
          return true;
      } /* endswitch */
      break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

void csListBox::MakeItemVisible (csComponent *item)
{
  if (!item->SendCommand (cscmdListBoxItemScrollVertically, (void *)false))
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
  void *param), void *param, bool iSelected)
{
  if (!func)
    return NULL;

  csComponent *start = first;
  csComponent *cur = start;
  while (cur)
  {
    csComponent *next = cur->next;

    unsigned reply = (long)cur->SendCommand (cscmdListBoxItemCheck, NULL);
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
  return NULL;
}

void csListBox::SetState (int mask, bool enable)
{
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if ((oldstate ^ state) & CSS_DISABLED)
  {
    bool dis = GetState (CSS_DISABLED);
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
    unsigned rc = (unsigned)comp->SendCommand (cscmdListBoxItemCheck, NULL);
    if (rc == CS_LISTBOXITEMCHECK_SELECTED
     || rc == CS_LISTBOXITEMCHECK_UNSELECTED)
      parent->SendCommand (cscmdListBoxItemFocused, comp);
  }
  return true;
}

void csListBox::Insert (csComponent *comp)
{
  fPlaceItems = true;
  Invalidate ();
  csComponent::Insert (comp);
}

void csListBox::Delete (csComponent *comp)
{
  fPlaceItems = true;
  Invalidate ();
  csComponent::Delete (comp);
}
