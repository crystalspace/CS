/*
    Crystal Space Windowing System: tree class
    Copyright (C) 2000 by Norman Kraemer, based on the listbox code:
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

#include "csws/cstree.h"
#include "csws/cstimer.h"
#include "csws/csscrbar.h"
#include "csws/csapp.h"
#include "csws/csskin.h"
#include "csws/cswsutil.h"
#include "csutil/event.h"

#define TREEBOX_TEXTURE_NAME		"csws::TreeBox"

// Amount of space at left and at right of each listbox item
#define TREEITEM_XSPACE			2
// Amount of space at top and at bottom of each listbox item
#define TREEITEM_YSPACE			2

// Mouse scroll time interval in milliseconds
#define MOUSE_SCROLL_INTERVAL		100

// Horizontal large scrolling step (in pixels)
#define TREE_HORIZONTAL_PAGESTEP	8

csTreeItem::csTreeItem (csComponent *iParent, const char *iText, int iID,
  csTreeItemStyle iStyle) : csComponent (iParent)
{
  state |= CSS_SELECTABLE | CSS_TRANSPARENT;
  SetText (iText);
  id = iID;
  ItemStyle = iStyle;
  ItemBitmap [0] = ItemBitmap [1] = 0;
  DeleteBitmap = false;
  hChildrenOffset = 0;
  SetPalette (CSPAL_TREEITEM);

  // Create the expand/collapse button
  button = 0; // see ::Insert
  button = new csButton (this, cscmdTreeItemToggle, CSBS_SELECTABLE, csbfsNone);
  button->SetState (CSS_TRANSPARENT, true);
  button->SetState (CSS_VISIBLE, false);
}

csTreeItem::~csTreeItem ()
{
  // First of all, delete child items (subbranch) before we notify the parent
  button = 0;
  DeleteAll ();
  // Now notify the parent to check if we're not the active item
  treebox->SendCommand (cscmdTreeItemDeleteNotify, (intptr_t)this);
  // Forget user bitmaps, if any
  SetBitmap (0, 0, false);
}

void csTreeItem::SetBitmap (csPixmap *iBitmap, csPixmap *iBitmapOpen, bool iDelete)
{
  if (DeleteBitmap)
  {
    delete ItemBitmap [0];
    delete ItemBitmap [1];
  }
  ItemBitmap [0] = iBitmap;
  ItemBitmap [1] = iBitmapOpen;
  DeleteBitmap = iDelete;
  Invalidate ();
  SetState (CSS_TREEITEM_PLACEITEMS, true);
  parent->SendCommand (cscmdTreeItemSizeChangeNotify, (intptr_t)this);
}

void csTreeItem::SuggestSize (int &w, int &h)
{
  if (!button)
  {
    // Not initialized yet
    w = h = 0;
    return;
  }

  w = button->bound.Width ();
  // If the button is too small, it seems we haven't set the bitmaps yet
  if (w < 4)
  {
    treebox->PrepareButton (button, GetState (CSS_TREEITEM_OPEN));
    w = button->bound.Width ();
  }
  h = button->bound.Height ();

  csPixmap *pmap = GetState (CSS_TREEITEM_OPEN) && ItemBitmap [1] ?
    ItemBitmap [1] : ItemBitmap [0];

  if (pmap)
  {
    w += TREEITEM_XSPACE + pmap->Width ();
    int _h = pmap->Height ();
    if (h < _h) h = _h;
  } /* endif */

  if (text)
  {
    int fh, fw = GetTextSize (text, &fh);
    w += TREEITEM_XSPACE + fw;
    if (h < fh) h = fh;
  } /* endif */

  // Leave a bit of space at left, right, top and bottom
  w += TREEITEM_XSPACE * 2;
  h += TREEITEM_YSPACE * 2;

  // Check if button position haven't changed
  int x = TREEITEM_XSPACE;
  int y = (h - button->bound.Height ()) / 2;
  if ((x != button->bound.xmin)
   || (y != button->bound.ymin))
  {
    button->SetPos (x, y);
    hChildrenOffset = button->bound.xmax;
  }
}

void csTreeItem::SuggestTotalSize (int &w, int &h, int &totw, int &toth)
{
  SuggestSize (w, h);
  totw = w; toth = h;
  if (!GetState (CSS_TREEITEM_OPEN))
    return;

  csComponent *cur = top;
  if (cur)
    do
    {
      if (cur->SendCommand (cscmdTreeItemCheck) == CS_TREEITEM_MAGIC)
      {
        int _w, _h, _totw, _toth;
        ((csTreeItem *)cur)->SuggestTotalSize (_w, _h, _totw, _toth);
        _totw += hChildrenOffset;
        if (totw < _totw) totw = _totw;
        toth += _toth;
      }
      cur = cur->next;
    } while (cur != top);
}

static bool do_show_items (csComponent *child, intptr_t param)
{
  if (child->SendCommand (cscmdTreeItemCheck) == CS_TREEITEM_MAGIC)
    child->SetState (CSS_VISIBLE, !!param);
  return false;
}

void csTreeItem::SetState (int mask, bool enable)
{
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if ((oldstate ^ state) & CSS_TREEITEM_OPEN)
  {
    treebox->PrepareButton (button, GetState (CSS_TREEITEM_OPEN));
    // Toggle visibility of all child items
    ForEach (do_show_items, (intptr_t)GetState (CSS_TREEITEM_OPEN));
    // Tell parent to reposition all the items
    parent->SendCommand (cscmdTreeItemSizeChangeNotify, (intptr_t)this);
  } /* endif */
}

void csTreeItem::Insert (csComponent *comp)
{
  csComponent::Insert (comp);
  if (button)
  {
    button->SetState (CSS_VISIBLE, (button->next != button));
    SetState (CSS_TREEITEM_PLACEITEMS, true);
    ((csTreeItem *)comp)->treebox = treebox;
    parent->SendCommand (cscmdTreeItemSizeChangeNotify, (intptr_t)this);
  }
}

void csTreeItem::Delete (csComponent *comp)
{
  csComponent::Delete (comp);
  if ((comp != button) && button)
  {
    button->SetState (CSS_VISIBLE, (button->next != button));
    SetState (CSS_TREEITEM_PLACEITEMS, true);
    parent->SendCommand (cscmdTreeItemSizeChangeNotify, (intptr_t)this);
    if (button->next == button)
      Toggle (0);
  }
}

bool csTreeItem::SetFocused (csComponent *comp)
{
  bool s1 = (focused == button);
  if (!csComponent::SetFocused (comp))
    return false;
  bool s2 = (focused == button);
  if (s1 != s2)
    Invalidate ();
  return true;
}

void csTreeItem::PlaceItems ()
{
  // If we aren't created yet, or are going to disappear, ignore request
  if (!button)
    return;

  SetState (CSS_TREEITEM_PLACEITEMS, false);

  int w, h;
  SuggestSize (w, h);

  int curx = hChildrenOffset;
  int cury = h;
  for (csComponent *cur = button->next; cur != button; cur = cur->next)
  {
    if (cur->SendCommand (cscmdTreeItemCheck) != CS_TREEITEM_MAGIC)
      continue;

    csTreeItem *item = (csTreeItem *)cur;
    int w, h, totw, toth;
    item->SuggestTotalSize (w, h, totw, toth);
    csRect itembound (curx, cury, curx + totw, cury + toth);
    item->SetRect (itembound);
    cury += toth;
  }
}

void csTreeItem::Draw ()
{
  csComponent::Draw ();

  bool enabled = !treebox->GetState (CSS_DISABLED);
  bool selected = enabled && (treebox->active == this);

  int w, h;
  SuggestSize (w, h);

  // First of all, draw lines connecting current item with all his children
  if ((button->next != button) && GetState (CSS_TREEITEM_OPEN))
  {
    int x = (button->bound.xmin + button->bound.xmax) / 2;
    int sy = button->bound.ymax, ly = sy;
    csTreeItem *cur = (csTreeItem *)button->next;
    while ((void *)cur != (void *)button)
    {
      int cw, ch;
      cur->SuggestSize (cw, ch);
      ly = cur->bound.ymin + ch / 2;
      Line (x, ly, cur->bound.xmin + cur->button->bound.xmax, ly, CSPAL_TREEITEM_LINES);
      cur = (csTreeItem *)cur->next;
    }
    Line (x, sy, x, ly, CSPAL_TREEITEM_LINES);
  }

  if (selected)
  {
    int x = button->GetState (CSS_VISIBLE) ? 0 : button->bound.xmax;
    if (treebox->GetState (CSS_FOCUSED) && enabled)
      Box (x, 0, w, h, CSPAL_TREEITEM_SELECTION);
    else
      Rect3D (x, 0, w, h, CSPAL_TREEITEM_SELECTION, CSPAL_TREEITEM_SELECTION);
  }

  int color;
  if (GetState (CSS_SELECTABLE) && enabled)
    if (ItemStyle == cstisNormal)
      color = selected && treebox->GetState (CSS_FOCUSED) ?
        CSPAL_TREEITEM_SNTEXT : CSPAL_TREEITEM_UNTEXT;
    else
      color = selected && treebox->GetState (CSS_FOCUSED) ?
        CSPAL_TREEITEM_SETEXT : CSPAL_TREEITEM_UETEXT;
  else
    color = CSPAL_TREEITEM_DTEXT;

  int x = button->bound.xmax + TREEITEM_XSPACE;

  csPixmap *pmap = GetState (CSS_TREEITEM_OPEN) && ItemBitmap [1] ?
    ItemBitmap [1] : ItemBitmap [0];
  if (pmap)
  {
    Pixmap (pmap, x, (h - pmap->Height ()) / 2);
    x += TREEITEM_XSPACE + pmap->Width ();
  } /* endif */
  if (text)
  {
    int fh;
    GetTextSize (text, &fh);
    Text (x, (h - fh + 1) / 2, color, -1, text);
  } /* endif */
}

int csTreeItem::Toggle (int iAction)
{
  // 0 (collapsed) or 1 (expanded)
  int curstate = int (!!GetState (CSS_TREEITEM_OPEN));
  // We can open if we have any children except the button
  bool canopen = (button->next != button);

  if (curstate != iAction)
  {
    if (iAction > 1)
      curstate = curstate ^ 1;
    else
      curstate = iAction;

    // If we cannot open, return
    if (curstate && !canopen)
      return 0;

    if (treebox->SendCommand (cscmdTreeItemToggleNotify, (intptr_t)this) != CS_TREEITEM_MAGIC)
    {
      SetState (CSS_TREEITEM_OPEN, curstate);
      if (curstate
       && !treebox->GetState (CSS_DISABLED)
       && (treebox->active == this))
        treebox->SendCommand (cscmdTreeMakeBranchVisible, (intptr_t)this);
    }
  }

  return curstate;
}

static bool do_toggle (csTreeItem *iItem, intptr_t iParam)
{
  iItem->Toggle (*((int*)iParam));
  return false;
}

bool csTreeItem::HandleEvent (iEvent &Event)
{
  // First of all, give a chance to our children to handle the event
  bool ourmev = false;
  bool passdown = false;
  if (CS_IS_MOUSE_EVENT (Event))
  {
    int w, h;
    SuggestSize (w, h);
    ourmev = !GetState (CSS_DISABLED) && (Event.Mouse.y < h);
    if (button->bound.Contains (Event.Mouse.x, Event.Mouse.y)
      || ((Event.Mouse.y > h) && (Event.Mouse.x > hChildrenOffset)))
      passdown = true;
  }
  else if (CS_IS_KEYBOARD_EVENT (Event))
    passdown = true;
  if (passdown && csComponent::HandleEvent (Event))
    return true;

  switch (Event.Type)
  {
    case csevMouseDown:
      if (ourmev && (Event.Mouse.Button == 1))
        treebox->SendCommand (cscmdTreeStartTracking, (intptr_t)this);
      return true;
    case csevMouseMove:
      if (ourmev && app->MouseOwner == treebox)
        treebox->SendCommand (cscmdTreeTrack, (intptr_t)this);
      return true;
    case csevMouseClick:
      if (Event.Mouse.Button == 2)
        treebox->parent->SendCommand (cscmdTreeItemRightClick, (intptr_t)this);
      return true;
    case csevMouseDoubleClick:
      if (ourmev)
        Toggle (2);
      return true;
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	if (treebox->active != this)
	  return false;
	switch (csKeyEventHelper::GetRawCode (&Event))
	{
	  case CSKEY_PADPLUS:
	    switch (csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS)
	    {
	      case 0:
		Toggle (1);
		return true;
	      case CSMASK_SHIFT:
		SendCommand (cscmdTreeItemToggleAll, 1);
		return true;
	      case CSMASK_CTRL:
		treebox->ExpandAll ();
		return true;
	    }
	    break;
	  case CSKEY_PADMINUS:
	    switch (csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS)
	    {
	      case 0:
		Toggle (0);
		return true;
	      case CSMASK_SHIFT:
		SendCommand (cscmdTreeItemToggleAll, 0);
		return true;
	      case CSMASK_CTRL:
		treebox->CollapseAll ();
		return true;
	    }
	    break;
	  case CSKEY_PADMULT:
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	    {
	      Toggle (2);
	      return true;
	    }
	    break;
	}
	return false;
      }
      break;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdTreeItemCheck:
          Event.Command.Info = CS_TREEITEM_MAGIC;
          return true;
        case cscmdTreeItemToggleAll:
          ForEachItem (do_toggle, Event.Command.Info, false);
          // Fallback to cscmdTreeItemToggle
        case cscmdTreeItemToggle:
          Event.Command.Info = (intptr_t)Toggle ((int)Event.Command.Info);
          return true;
        case cscmdTreeItemSizeChangeNotify:
          SetState (CSS_TREEITEM_PLACEITEMS, true);
          return parent->HandleEvent (Event);
        case cscmdTreeItemGetPrev:
          Event.Command.Info = (intptr_t)PrevItem ();
          return true;
        case cscmdTreeItemGetNext:
          Event.Command.Info = (intptr_t)NextItem ();
          return true;
        case cscmdTreeItemGetFirst:
          Event.Command.Info = button && (button->next != button) ? (intptr_t)button->next : (intptr_t)0;
          return true;
        case cscmdTreeItemGetLast:
          Event.Command.Info = button && (button->prev != button) ? (intptr_t)button->prev : (intptr_t)0;
          return true;
      } /* endswitch */
      break;
    case csevBroadcast:
      switch (Event.Command.Code)
      {
        case cscmdPostProcess:
          if (GetState (CSS_TREEITEM_PLACEITEMS))
            PlaceItems ();
          break;
      }
      break;
  } /* endswitch */
  return passdown ? false : csComponent::HandleEvent (Event);
}

csTreeItem *csTreeItem::NextItem ()
{
  if (GetState (CSS_TREEITEM_OPEN))
    return (csTreeItem *)button->next;
  if (parent->SendCommand (cscmdTreeItemGetLast) != (intptr_t)this)
    return (csTreeItem *)next;

  // Trick: temporarily fool parent to think it's closed, so that
  // it will skip his first child and will proceed directly to its
  // next neightbour.
  int oldst = ((csTreeItem *)parent)->state & CSS_TREEITEM_OPEN;
  ((csTreeItem *)parent)->state &= ~CSS_TREEITEM_OPEN;
  csTreeItem *item = (csTreeItem *)parent->SendCommand (cscmdTreeItemGetNext);
  ((csTreeItem *)parent)->state |= oldst;
  return item;
}

csTreeItem *csTreeItem::PrevItem ()
{
  csTreeItem *first = (csTreeItem *)parent->SendCommand (cscmdTreeItemGetFirst);
  if (!first || (first == this))
    return ((parent->SendCommand (cscmdTreeItemCheck) == CS_TREEITEM_MAGIC)) ?
      (csTreeItem *)parent : 0;
  csTreeItem *last = (csTreeItem *)prev;
  while (last->GetState (CSS_TREEITEM_OPEN))
    last = (csTreeItem *)last->SendCommand (cscmdTreeItemGetLast);
  return last;
}

csTreeItem *csTreeItem::ForEachItem (bool (*func) (csTreeItem *child,
  intptr_t param), intptr_t param, bool iOnlyOpen)
{
  csTreeItem *item = (csTreeItem *)button->next;
  if ((csComponent *)item == button)
    return 0;
  do
  {
    if (func (item, param))
      return item;
    if (!iOnlyOpen || item->GetState (CSS_TREEITEM_OPEN))
    {
      csTreeItem *tmp = item->ForEachItem (func, param, iOnlyOpen);
      if (tmp) return tmp;
    }
    item = (csTreeItem *)item->next;
  } while ((csComponent *)item != button);
  return 0;
}

//--------------------------------------------------------// csTreeBox //-----//

static int treeref = 0;
static csPixmap *treepix [8];

csTreeBox::csTreeBox (csComponent *iParent, int iStyle,
  csTreeFrameStyle iFrameStyle) : csComponent (iParent)
{
  state |= CSS_SELECTABLE;
  SetPalette (CSPAL_TREEBOX);
  deltax = deltay = maxdeltax = maxdeltay = 0;
  active = 0;
  hscroll = vscroll = 0;
  clipview = 0; // See ::Insert
  timer = new csTimer (this, MOUSE_SCROLL_INTERVAL);
  clipview = new csTreeView (this);
  SetStyle (TreeStyle = iStyle, FrameStyle = iFrameStyle);

  if (!treeref)
  {
    static char *pixnam [8] =
    { "TBCN", "TBCP", "TBON", "TBOP", "TBSCN", "TBSCP", "TBSON", "TBSOP" };
    iTextureHandle *tex = app->GetTexture (TREEBOX_TEXTURE_NAME);
	int i;
    for (i = 0; i < 8; i++)
    {
      int tx,ty,tw,th;
      ParseConfigBitmap (app, app->skin->Prefix, "Dialog", pixnam [i], tx, ty, tw, th);
      treepix [i] = new csSimplePixmap (tex, tx, ty, tw, th);
    }
  }
  treeref++;
}

csTreeBox::~csTreeBox ()
{
  treeref--;
  if (!treeref)
  {
	int i;
    for (i = 0; i < 8; i++)
      delete treepix [i];
  }
}

static bool do_hide_buttons (csTreeItem *iItem, intptr_t iParam)
{
  (void)iParam;
  iItem->ResetButton ();
  return false;
}

void csTreeBox::SetStyle (int iStyle, csTreeFrameStyle iFrameStyle)
{
  int oldTreeStyle = TreeStyle;
  TreeStyle = iStyle;
  FrameStyle = iFrameStyle;
  csScrollBarFrameStyle sbsty;
  switch (FrameStyle)
  {
    default:
    case cstfsNone:
      BorderWidth = BorderHeight = 0;
      sbsty = cssfsThinRect;
      break;
    case cstfsThinRect:
      BorderWidth = BorderHeight = 2;
      sbsty = cssfsThinRect;
      break;
    case cstfsThickRect:
      BorderWidth = BorderHeight = 2;
      sbsty = cssfsThickRect;
      break;
  } /* endswitch */

  delete hscroll;
  delete vscroll;
  csTreeView *oldclip = clipview;
  clipview = 0;
  if (iStyle & CSTS_HSCROLL)
    hscroll = new csScrollBar (this, sbsty);
  else
    hscroll = 0;
  if (iStyle & CSTS_VSCROLL)
    vscroll = new csScrollBar (this, sbsty);
  else
    vscroll = 0;
  clipview = oldclip;

  PlaceScrollbars ();
  Invalidate ();

  // Make expand/collapse buttons very small if 'small buttons' style
  // has been changed to force buttons to resize properly.
  if ((oldTreeStyle ^ TreeStyle) & CSTS_SMALLBUTTONS)
    ForEachItem (do_hide_buttons, 0, false);
}

void csTreeBox::Draw ()
{
  csComponent::Draw ();

  switch (FrameStyle)
  {
    case cstfsNone:
      break;
    case cstfsThinRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_TREEBOX_LIGHT3D, CSPAL_TREEBOX_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_TREEBOX_DARK3D, CSPAL_TREEBOX_LIGHT3D);
      break;
    case cstfsThickRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_TREEBOX_LIGHT3D, CSPAL_TREEBOX_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_TREEBOX_2LIGHT3D, CSPAL_TREEBOX_2DARK3D);
      break;
  } /* endswitch */

  Box (BorderWidth, BorderHeight, bound.Width () - BorderWidth,
    bound.Height () - BorderHeight, FrameStyle == cstfsThickRect ?
    CSPAL_TREEBOX_BACKGROUND2 : CSPAL_TREEBOX_BACKGROUND);
}

void csTreeBox::PlaceItems (int sbFlags)
{
  SetState (CSS_TREEBOX_PLACEITEMS, false);

  // collect item statistics
  maxdeltax = maxdeltay = 0;
  // Current y offset for the current item
  int curx = -deltax, cury = -deltay;
  // Current stage: 0 - above top margin; 1 - inside the box; 2 - below bottom
  int stage = 0;
  // Current tree item
  csComponent *cur = clipview->top;
  // The middle height of one item
  int midh = 0, numh = 0;

  while (cur)
  {
    if (cur->SendCommand (cscmdTreeItemCheck) == CS_TREEITEM_MAGIC)
    {
      csTreeItem *item = (csTreeItem *)cur;

      int w, h, totw, toth;
      item->SuggestTotalSize (w, h, totw, toth);
      maxdeltay += toth;

      if (totw > maxdeltax)
        maxdeltax = totw;

      switch (stage)
      {
        case 0:
          if (cury + toth > 0)
            stage++;
          break;
        case 1:
          if (cury > clipview->bound.ymax)
            stage++;
          break;
      } /* endswitch */

      // Set current item x,y and height for *all* items
      // (so that we can use coordinates in MakeVisible)
      csRect itembound (curx, cury, curx + totw, cury + toth);
      item->SetRect (itembound);
      // Broadcast a pseudo-postprocess event to force the item
      // to position his children if needed.
      item->SendBroadcast (cscmdPostProcess);

      if (stage == 1)
      {
        item->SetState (CSS_VISIBLE, true);
        midh += h;
        numh++;
      }
      else
        cur->SetState (CSS_VISIBLE, false);
      cury += toth;
    } /* endif */
    cur = cur->next;
    if (cur == clipview->top)
      break;
  } /* endwhile */

  // If we have free space at bottom, scroll down
  if (deltay > 0 && cury < clipview->bound.Height ())
  {
    deltay = maxdeltay - clipview->bound.Height ();
    if (deltay < 0) deltay = 0;
    PlaceItems (sbFlags);
  }
  else
  {
    bool placesb = false;
    if ((sbFlags & CSTS_VSCROLL) && vscroll)
    {
      if (numh) midh /= numh; else midh = 1;
      vsbstatus.size = clipview->bound.Height ();
      vsbstatus.maxsize = maxdeltay;
      vsbstatus.value = deltay;
      vsbstatus.maxvalue = maxdeltay - vsbstatus.size;
      vsbstatus.step = midh;
      vsbstatus.pagestep = vsbstatus.size;
      vscroll->SendCommand (cscmdScrollBarSet, (intptr_t)&vsbstatus);
      if (TreeStyle & CSTS_AUTOSCROLLBAR)
      {
        bool state = vsbstatus.size < vsbstatus.maxsize;
        if (state != !!vscroll->GetState (CSS_VISIBLE))
        {
          vscroll->SetState (CSS_VISIBLE, state);
          placesb = true;
        }
      }
    } /* endif */

    int maxw = maxdeltax;
    maxdeltax -= clipview->bound.Width ();
    if (maxdeltax < 0) maxdeltax = 0;
    if ((sbFlags & CSTS_HSCROLL) && hscroll)
    {
      hsbstatus.size = clipview->bound.Width ();
      hsbstatus.maxsize = maxw;
      hsbstatus.value = deltax;
      hsbstatus.maxvalue = maxdeltax;
      hsbstatus.step = 1;
      hsbstatus.pagestep = TREE_HORIZONTAL_PAGESTEP;
      hscroll->SendCommand (cscmdScrollBarSet, (intptr_t)&hsbstatus);
      if (TreeStyle & CSTS_AUTOSCROLLBAR)
      {
        bool state = hsbstatus.size < hsbstatus.maxsize;
        if (state != !!hscroll->GetState (CSS_VISIBLE))
        {
          hscroll->SetState (CSS_VISIBLE, state);
          placesb = true;
        }
      }
    } /* endif */

    if (placesb)
      PlaceScrollbars ();
  } /* endif */
}

bool csTreeBox::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (!csComponent::SetRect (xmin, ymin, xmax, ymax))
    return false;

  PlaceScrollbars ();
  return true;
}

void csTreeBox::PlaceScrollbars ()
{
  csRect clip (BorderWidth, BorderHeight,
    bound.Width () - BorderWidth, bound.Height () - BorderHeight);

  bool hs = hscroll && hscroll->GetState (CSS_VISIBLE);
  bool vs = vscroll && vscroll->GetState (CSS_VISIBLE);

  if (hs)
  {
    hscroll->SetRect (0, bound.Height () - CSSB_DEFAULTSIZE,
      bound.Width () - (vs ? CSSB_DEFAULTSIZE - 1 : 0),
      bound.Height ());
    clip.ymax = hscroll->bound.ymin;
  }
  if (vs)
  {
    vscroll->SetRect (bound.Width () - CSSB_DEFAULTSIZE, 0,
      bound.Width (),
      bound.Height () - (hs ? CSSB_DEFAULTSIZE - 1 : 0));
    clip.xmax = vscroll->bound.xmin;
  }

  clipview->SetRect (clip);
  SetState (CSS_TREEBOX_PLACEITEMS, true);
}

static bool do_findtext (csTreeItem *child, intptr_t param)
{
  return (strcmp (child->GetText (), (char *)param) == 0);
}

bool csTreeBox::HandleEvent (iEvent &Event)
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
	      CSMASK_ALLSHIFTS) == 0)
	      FocusItem ((csTreeItem *)active->SendCommand (cscmdTreeItemGetPrev));
	    return true;
	  case CSKEY_DOWN:
	    if (csKeyEventHelper::GetModifiersBits (&Event) == 0)
	      FocusItem ((csTreeItem *)active->SendCommand (cscmdTreeItemGetNext));
	    return true;
	  case CSKEY_LEFT:
	  {
	    int odx = deltax;
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	      if (deltax > TREE_HORIZONTAL_PAGESTEP)
		deltax -= TREE_HORIZONTAL_PAGESTEP;
	      else
		deltax = 0;
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	      if (deltax > 0)
		deltax--;
	    if (deltax != odx)
	      PlaceItems ();
	    return true;
	  }
	  case CSKEY_RIGHT:
	  {
	    int odx = deltax;
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	      if (deltax + TREE_HORIZONTAL_PAGESTEP <= maxdeltax)
		deltax += TREE_HORIZONTAL_PAGESTEP;
	      else
		deltax = maxdeltax;
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	      if (deltax < maxdeltax)
		deltax++;
	    if (deltax != odx)
	      PlaceItems ();
	    return true;
	  }
	  case CSKEY_PGUP:
	  case CSKEY_PGDN:
	  {
	    if (csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_CTRL)
	    {
	      csTreeItem *f = (csTreeItem *)clipview->SendCommand (
		csKeyEventHelper::GetCookedCode (&Event) == CSKEY_PGUP ? 
		  cscmdTreeItemGetFirst : cscmdTreeItemGetLast);
	      if (f && (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_PGDN))
		while (f->GetState (CSS_TREEITEM_OPEN))
		  f = (csTreeItem *)f->SendCommand (cscmdTreeItemGetLast);
	      FocusItem (f);
	    }
	    else
	    {
	      bool setcaret = !(csKeyEventHelper::GetModifiersBits (&Event) & 
		CSMASK_SHIFT);
	      int delta = clipview->bound.Height ();
	      if (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_PGUP)
		delta = -delta;
	      VScroll (delta, setcaret);
	    }
	    return true;
	  }
	  case CSKEY_HOME:
	  {
	    int odx = deltax, ody = deltay;
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	      deltax = 0;
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	      deltay = 0;
	    if (odx != deltax || ody != deltay)
	      PlaceItems ();
	    return true;
	  }
	  case CSKEY_END:
	  {
	    int odx = deltax, ody = deltay;
	    if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == 0)
	      deltax = maxdeltax;
	    else if ((csKeyEventHelper::GetModifiersBits (&Event) & 
	      CSMASK_ALLSHIFTS) == CSMASK_CTRL)
	      deltay = maxdeltay;
	    if (odx != deltax || ody != deltay)
	      PlaceItems ();
	    return true;
	  }
	  default:
	    int keyChar = csKeyEventHelper::GetCookedCode (&Event);
	    if (active && (keyChar > ' ') && (keyChar < 128))
	    {
	      csTreeItem *cur = active;
	      do
	      {
		cur = (csTreeItem *)cur->SendCommand (cscmdTreeItemGetNext);
		if (!cur) cur = (csTreeItem *)clipview->top;
	      } while ((cur != active) && (cur->GetText () [0] != toupper (keyChar)));
	      if (cur != active)
	      {
		FocusItem (cur);
		return true;
	      }
	    }
	    break;
	} /* endswitch */
      }
      break;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdTreeClear:
          if (app->MouseOwner == this)
            app->CaptureMouse (0);
          clipview->DeleteAll ();
          active = 0;
          return true;
        case cscmdTreeSetHorizOffset:
          if (deltax != (int)Event.Command.Info)
          {
            deltax = (int)Event.Command.Info;
            SetState (CSS_TREEBOX_PLACEITEMS, true);
          } /* endif */
          return true;
        case cscmdTreeItemSizeChangeNotify:
          SetState (CSS_TREEBOX_PLACEITEMS, true);
          return true;
        case cscmdTreeStartTracking:
          if (!app->MouseOwner)
            app->CaptureMouse (this);
          // Fallback to cscmdTreeTrack
        case cscmdTreeTrack:
          FocusItem ((csTreeItem *)Event.Command.Info);
          return true;
        case cscmdTreeMakeVisible:
          if (!GetState (CSS_TREEBOX_LOCKVISIBLE))
            MakeItemVisible ((csComponent *)Event.Command.Info, false);
          return true;
        case cscmdTreeMakeBranchVisible:
          if (!GetState (CSS_TREEBOX_LOCKVISIBLE))
            MakeItemVisible ((csComponent *)Event.Command.Info, true);
          return true;
        case cscmdTreeQuerySelected:
          Event.Command.Info = (intptr_t)active;
          return true;
        case cscmdTimerPulse:
          if (app && active && app->MouseOwner == this)
          {
            int x = 0, y = 0, w, h;
            OtherToThis (active, x, y);
            active->SuggestSize (w, h);
            GetMousePosition (Event.Mouse.x, Event.Mouse.y);
            if (Event.Mouse.y < y)
              FocusItem ((csTreeItem *)active->SendCommand (cscmdTreeItemGetPrev));
            else if (Event.Mouse.y >= y + h)
              FocusItem ((csTreeItem *)active->SendCommand (cscmdTreeItemGetNext));
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
              PlaceItems (0);
            } /* endif */
          }
          else if (bar == vscroll)
          {
            vsbstatus = sbs;
            if (deltay != vsbstatus.value)
            {
              deltay = vsbstatus.value;
              PlaceItems (CSTS_HSCROLL);
            } /* endif */
          } /* endif */
          Invalidate ();
          return true;
        }
        case cscmdTreeSelectItem:
          Event.Command.Info = (intptr_t)ForEachItem (do_findtext,
            Event.Command.Info, false);
          return true;
        case cscmdTreeItemToggleNotify:
          parent->HandleEvent (Event);
          if (Event.Command.Info != CS_TREEITEM_MAGIC)
          {
            // Check if active item is not the child of the item
            // that is going to collapse
            csTreeItem *item = (csTreeItem *)Event.Command.Info;
            if (item->GetState (CSS_TREEITEM_OPEN))
            {
              csTreeItem *cur = active;
              do
              {
                if (cur == item)
                {
                  FocusItem (item);
                  break;
                }
                cur = (csTreeItem *)cur->parent;
              } while (cur->SendCommand (cscmdTreeItemCheck) == CS_TREEITEM_MAGIC);
            }
          }
          return true;
        case cscmdTreeItemGetFirst:
          Event.Command.Info = (intptr_t)clipview->top;
          return true;
        case cscmdTreeItemGetLast:
          Event.Command.Info = clipview->top ? (intptr_t)clipview->top->prev : (intptr_t)0;
          return true;
        case cscmdTreeItemDeleteNotify:
          if ((intptr_t)active == Event.Command.Info)
          {
            csTreeItem *item = (csTreeItem *)active->SendCommand (cscmdTreeItemGetPrev);
            if (item && item->button)
              FocusItem (item);
            else
              active = 0;
          }
          SetState (CSS_TREEBOX_PLACEITEMS, true);
          return true;
      } /* endswitch */
      break;
    case csevBroadcast:
      switch (Event.Command.Code)
      {
        case cscmdPostProcess:
          if (GetState (CSS_TREEBOX_PLACEITEMS))
            PlaceItems ();
          break;
      }
      break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

void csTreeBox::VScroll (int iDelta, bool iMoveCaret)
{
  int cx = 0, cy = 0;
  OtherToThis (active, cx, cy);
  cy += iDelta;
  csTreeItem *best = 0;
  if (iMoveCaret)
  {
    // Find the nearest item with y == newcarety
    int nearesty = +9999999;
    csTreeItem *cur = (csTreeItem *)clipview->top;
    do
    {
      int _x = 0, _y = 0;
      OtherToThis (cur, _x, _y);
      int delta = abs (_y - cy);
      if (delta < nearesty)
      {
        nearesty = delta;
        best = cur;
      }
      cur = (csTreeItem *)cur->SendCommand (cscmdTreeItemGetNext);
    } while (cur && cur->GetState (CSS_VISIBLE));
  }
  deltay += iDelta;
  if (deltay < 0) deltay = 0;
  if (deltay > maxdeltay) deltay = maxdeltay;
  PlaceItems ();
  if (best)
    FocusItem (best);
}

csTreeItem *csTreeBox::ForEachItem (bool (*func) (csTreeItem *child,
  intptr_t param), intptr_t param, bool iOnlyOpen)
{
  csTreeItem *item = (csTreeItem *)clipview->top;
  if (!item)
    return 0;
  do
  {
    if (func (item, param))
      return item;
    if (!iOnlyOpen || item->GetState (CSS_TREEITEM_OPEN))
    {
      csTreeItem *tmp = item->ForEachItem (func, param, iOnlyOpen);
      if (tmp) return tmp;
    }
    item = (csTreeItem *)item->next;
  } while ((csComponent *)item != clipview->top);
  return 0;
}

void csTreeBox::SetState (int mask, bool enable)
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
  if (active && ((state ^ oldstate) & CSS_FOCUSED))
    active->Invalidate ();
  if ((mask & CSS_TREEBOX_PLACEITEMS) && enable)
    Invalidate ();
}

void csTreeBox::Insert (csComponent *comp)
{
  // In reality, insert tree items in our children rather than into this component
  if (clipview)
  {
    // Alas, we can't do "SendCommand (cscmdTreeItemCheck)" because the
    // tree item may not be finished yet (it may call Insert() from csComponent
    // constructor) and thus csComponent's HandleEvent will be called.
    // Thus we have to rely on the fact that if the clipview is non-0,
    // any further components that are inserted here are tree items.
    ((csTreeItem *)comp)->treebox = this;
    if (!active)
      active = (csTreeItem *)comp;
    clipview->Insert (comp);
    SetState (CSS_TREEBOX_PLACEITEMS, true);
  }
  else
    csComponent::Insert (comp);
}

void csTreeBox::MakeItemVisible (csComponent *iItem, bool iChildren)
{
  if (GetState (CSS_TREEBOX_PLACEITEMS))
    PlaceItems ();

  csRect ir;
  if (iChildren)
    ir.Set (iItem->bound);
  else
  {
    ir.xmin = iItem->bound.xmin;
    ir.ymin = iItem->bound.ymin;
    iItem->SuggestSize (ir.xmax, ir.ymax);
    ir.xmax += ir.xmin;
    ir.ymax += ir.ymin;
  }
  OtherToThis (iItem->parent, ir.xmin, ir.ymin);
  OtherToThis (iItem->parent, ir.xmax, ir.ymax);

  // Vertical scrolling
  int ody = deltay;
  if (ir.ymax > clipview->bound.ymax)
  {
    deltay += ir.ymax - clipview->bound.ymax;
    ir.Move (0, clipview->bound.ymax - ir.ymax);
  }
  if (ir.ymin < clipview->bound.ymin)
    deltay += ir.ymin - clipview->bound.ymin;

  // Horizontal scrolling: the behaviour is different: scroll only if
  // the item is completely invisible.
  int odx = deltax;
  if (ir.xmax <= clipview->bound.xmin + TREEITEM_XSPACE)
    deltax += ir.xmin - clipview->bound.xmin;
  else if (ir.xmin >= clipview->bound.xmax - TREEITEM_XSPACE)
    deltax += ir.xmax - clipview->bound.xmax;

  if (odx != deltax
   || ody != deltay)
    SetState (CSS_TREEBOX_PLACEITEMS, true);
}

void csTreeBox::FocusItem (csTreeItem *iItem)
{
  if (!iItem)
    return;
  if (!GetState (CSS_FOCUSED))
    Select ();
  if (active != iItem)
  {
    active->Invalidate ();
    active = iItem;

    // Expand all branches until `active'
    for (csTreeItem *cur = active; ; )
    {
      cur = (csTreeItem *)cur->parent;
      if (cur->SendCommand (cscmdTreeItemCheck) != CS_TREEITEM_MAGIC)
        break;
      if (!cur->GetState (CSS_TREEITEM_OPEN))
        cur->Toggle (1);
    }

    active->button->Select ();
    active->Invalidate ();
    active->Select ();

    parent->SendCommand (cscmdTreeItemFocused, (intptr_t)active);
  }
  MakeItemVisible (active);
}

void csTreeBox::ExpandAll ()
{
  int oldstate = GetState (CSS_TREEBOX_LOCKVISIBLE);
  SetState (CSS_TREEBOX_LOCKVISIBLE, true);
  ForEachItem (do_toggle, 1, false);
  if (!oldstate)
    SetState (CSS_TREEBOX_LOCKVISIBLE, false);

  MakeItemVisible (active, true);
}

void csTreeBox::CollapseAll ()
{
  int oldstate = GetState (CSS_TREEBOX_LOCKVISIBLE);
  SetState (CSS_TREEBOX_LOCKVISIBLE, true);
  ForEachItem (do_toggle, 0, false);
  if (!oldstate)
    SetState (CSS_TREEBOX_LOCKVISIBLE, false);

  csTreeItem *cur = active, *last = active;
  for (;;)
  {
    cur = (csTreeItem *)cur->parent;
    if (cur->SendCommand (cscmdTreeItemCheck) != CS_TREEITEM_MAGIC)
      break;
    if (!cur->parent->GetState (CSS_TREEITEM_OPEN))
      last = cur;
  }
  FocusItem (last);
}

void csTreeBox::PrepareButton (csButton *iButton, bool iOpen)
{
  int dp = iOpen ? 2 : 0;
  if (TreeStyle & CSTS_SMALLBUTTONS)
    dp += 4;
  iButton->SetBitmap (treepix [dp], treepix [dp + 1], false);
  int w, h;
  iButton->SuggestSize (w, h);
  iButton->SetRect (0, 0, w, h);
}

//--------------------------------------------// csTreeBox::csTreeView //-----//

csTreeBox::csTreeView::csTreeView (csComponent *iParent) : csComponent (iParent)
{
  SetState (CSS_TRANSPARENT | CSS_SELECTABLE, true);
}

bool csTreeBox::csTreeView::HandleEvent (iEvent &Event)
{
  if (Event.Type == csevCommand)
    if (parent->HandleEvent (Event))
      return true;
  return csComponent::HandleEvent (Event);
}

void csTreeBox::csTreeView::Delete (csComponent *comp)
{
  parent->SetState (CSS_TREEBOX_PLACEITEMS, true);
  csComponent::Delete (comp);
}
