/*
    Crystal Space Windowing System: tree class
    Copyright (C) 2000 by Norman Krämer
    based on the listbox code:
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
#include "csws/cstree.h"
#include "csws/cstimer.h"
#include "csws/csscrbar.h"
#include "csws/csapp.h"
#include "cssys/csinput.h"
#include "csws/cswindow.h"

// Amount of space at left and at right of each listbox item
#define TREEITEM_XSPACE              2
// Amount of space at top and at bottom of each listbox item
#define TREEITEM_YSPACE              2

// Mouse scroll time interval in milliseconds
#define MOUSE_SCROLL_INTERVAL           100

// Horizontal large scrolling step
#define TREE_HORIZONTAL_PAGESTEP     8

csTreeItem::csTreeItem (csComponent *iParent, const char *iText, int iID,
  csTreeItemStyle iStyle) : csComponent (iParent)
{
  state |= CSS_SELECTABLE | CSS_TRANSPARENT;
  ItemStyle = iStyle;
  ItemBitmap = NULL;
  DeleteBitmap = false;
  hOffset = vOffset = 0;
  id = iID;
  deltax = 0;
  SetPalette (CSPAL_TREEITEM);
  SetText (iText);
  if (parent)
  {
    csComponent *pc[2];
    pc[0] = parent;
    pc[1] = this;
    bool succ = (parent->SendCommand (cscmdTreeAddChild, pc) != pc);
    if (!succ)
    {
      // parent probably was a TreeItem itself
      if (parent->parent && (parent->parent->SendCommand (cscmdTreeAddChild, pc)!=pc) )
      {
	parent->parent->Insert (this);
	Show (false); // we changed the parent and removing this from the previous parent makes it hide, so we show it here
      }
    }
  }
  if (parent)
    SetColor (CSPAL_TREEITEM_BACKGROUND, parent->GetColor (0));
}

csTreeItem::~csTreeItem ()
{
  if (ItemBitmap && DeleteBitmap)
    delete ItemBitmap;
}

void csTreeItem::SuggestSize (int &w, int &h)
{
  int minh = 0;
  w = hOffset; h = vOffset;

  if (ItemBitmap)
  {
    w += ItemBitmap->Width () + 2;
    minh = h + ItemBitmap->Height ();
  } /* endif */

  if (text && parent)
  {
    w += TextWidth (text);
    h += TextHeight ();
  } /* endif */

  if (h < minh)
    h = minh;

  // Leave a bit of space at left, right, top and bottom
  w += TREEITEM_XSPACE * 2;
  h += TREEITEM_YSPACE * 2;
}

bool csTreeItem::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseDown:
      if (parent->GetState (CSS_DISABLED))
        return true;
      if (Event.Mouse.Button == 1)
      {
        parent->SendCommand (cscmdTreeStartTracking, this);
        parent->SendCommand (cscmdTreeItemClicked, this);
      }
      return true;
    case csevMouseMove:
      if ((app->MouseOwner == parent)
       && !GetState (CSS_FOCUSED))
        parent->SendCommand (cscmdTreeTrack, this);
      return true;
    case csevMouseDoubleClick:
      if ((Event.Mouse.Button == 1) && parent)
        parent->SendCommand (cscmdTreeItemDoubleClicked, this);
      return true;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdTreeItemCheck:
          Event.Command.Info = GetState (CSS_TREEITEM_SELECTED)
            ? (void *)CS_TREEITEMCHECK_SELECTED
            : (void *)CS_TREEITEMCHECK_UNSELECTED;
          return true;
        case cscmdTreeItemSet:
          if (Event.Command.Info)
            parent->SetFocused (this);
          SetState (CSS_TREEITEM_SELECTED, (bool)Event.Command.Info);
          return true;
        case cscmdTreeItemScrollVertically:
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
        case cscmdTreeItemSetHorizOffset:
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

void csTreeItem::SetState (int mask, bool enable)
{
  
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if ((oldstate ^ state) & CSS_TREEITEM_SELECTED)
  {
    Invalidate ();
    parent->SendCommand (GetState (CSS_TREEITEM_SELECTED) ?
      cscmdTreeItemSelected : cscmdTreeItemDeselected, this);
  } /* endif */
  if ((oldstate ^ state) & CSS_FOCUSED)
  {
    Invalidate ();
    if (GetState (CSS_FOCUSED)){
      parent->SendCommand (cscmdTreeMakeVisible, this);
    }
  } /* endif */
}

void csTreeItem::SetBitmap (csPixmap *iBitmap, bool iDelete)
{
  if (ItemBitmap && DeleteBitmap)
    delete ItemBitmap;
  ItemBitmap = iBitmap;
  DeleteBitmap = iDelete;
  Invalidate ();
}

void csTreeItem::Draw ()
{
  bool enabled = !parent->GetState (CSS_DISABLED);
  bool selected = enabled && GetState (CSS_TREEITEM_SELECTED);
  if (selected)
    Clear (CSPAL_TREEITEM_SELECTION);
  if (GetState (CSS_FOCUSED) && enabled)
  {
    int w,h;
    SuggestSize (w, h);
    int bh = bound.Height ();
    if (bh > h) h = bh;
    Rect3D (0, 0, bound.Width (), h, CSPAL_TREEITEM_SELRECT,
      CSPAL_TREEITEM_SELRECT);
  } /* endif */

  int color;
  if (GetState (CSS_SELECTABLE) && enabled)
  {
    if (ItemStyle == cstisNormal)
      if (selected)
        color = CSPAL_TREEITEM_SNTEXT;
      else
        color = CSPAL_TREEITEM_UNTEXT;
    else
      if (selected)
        color = CSPAL_TREEITEM_SETEXT;
      else
        color = CSPAL_TREEITEM_UETEXT;
  }
  else
    color = CSPAL_TREEITEM_DTEXT;

  int x = TREEITEM_XSPACE - deltax + hOffset;
  if (ItemBitmap)
  {
    Pixmap (ItemBitmap, x, vOffset + (bound.Height () - ItemBitmap->Height ()) / 2);
    x += ItemBitmap->Width () + 2;
  } /* endif */
  if (text)
  {
    Text (x, vOffset + (bound.Height () - TextHeight () + 1) / 2, color, -1, text);
    x += TextWidth (text);
  } /* endif */

  csComponent::Draw ();
}

csTreeCtrl::csTreeCtrl (csComponent *iParent, int iStyle,
  csTreeFrameStyle iFrameStyle) : csComponent (iParent)
{
  state |= CSS_SELECTABLE;
  TreeStyle = iStyle;
  FrameStyle = iFrameStyle;
  SetPalette (CSPAL_TREECTRL);
  deltax = 0;
  fPlaceItems = false;
  csScrollBarFrameStyle sbsty;
  switch (FrameStyle)
  {
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
    default:
      return;
  } /* endswitch */
  firstvisible = first = new csTimer (this, MOUSE_SCROLL_INTERVAL);
  if (iStyle & CSTS_HSCROLL)
    hscroll = new csScrollBar (this, sbsty);
  else
    hscroll = NULL;
  if (iStyle & CSTS_VSCROLL)
    vscroll = new csScrollBar (this, sbsty);
  else
    vscroll = NULL;

  treeroot = new TreeCtrlNode (NULL, NULL, true);
  branchdeltax = 20;
  if (parent)
    parent->SendCommand (cscmdWindowSetClient, this);
}

void csTreeCtrl::Draw ()
{
  if (fPlaceItems)
    PlaceItems ();

  switch (FrameStyle)
  {
    case cstfsNone:
      break;
    case cstfsThinRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_INPUTLINE_LIGHT3D, CSPAL_INPUTLINE_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_INPUTLINE_DARK3D, CSPAL_INPUTLINE_LIGHT3D);
      break;
    case cstfsThickRect:
      Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_INPUTLINE_LIGHT3D, CSPAL_INPUTLINE_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1,
          CSPAL_INPUTLINE_2LIGHT3D, CSPAL_INPUTLINE_2DARK3D);
      break;
  } /* endswitch */

  Box (BorderWidth, BorderHeight, bound.Width () - BorderWidth,
    bound.Height () - BorderHeight, FrameStyle == cstfsThickRect ?
    CSPAL_INPUTLINE_BACKGROUND2 : CSPAL_INPUTLINE_BACKGROUND);

  DrawBranches ();
  
  csComponent::Draw ();
}

void csTreeCtrl::DrawBranches ()
{
  csVector levels, lineseg;
  bool vis = false;
  int nLevel;
  TreeCtrlNode *node, *firstnode = (TreeCtrlNode*)(treeroot->IsLeaf () ? NULL 
                                    : treeroot->children.Get (0));
  TreeCtrlNode *firstVisNode = treeroot->FindItem (firstvisible);

  csRect clipbound (BorderWidth, BorderHeight, 
                    bound.Width () - BorderWidth,  bound.Height () - BorderHeight);
  if (hscroll) clipbound.ymax = hscroll->bound.ymin;
  if (vscroll) clipbound.xmax = vscroll->bound.xmin;

  int y = 0;

  if (!firstVisNode)
    firstVisNode = firstnode;

  node = firstnode;

  levels.Push (treeroot);
  lineseg.Push ((void*)treeroot->children.Length ());

  while (node && y < clipbound.ymax )
  {
    nLevel = levels.Find (node->parent);
    if (nLevel != -1)
    {
      levels.Insert (nLevel+1, node);
      lineseg.Insert (nLevel+1, (void*)node->children.Length ());
    }
    else
      nLevel = 0;

    if (node == firstVisNode)
      vis = true;

    int w, h;
    node->item->SuggestSize (w, h);
    if (vis && w && h){
      for (int i=0; i <= nLevel; i++)
      {
	int nSegments = (int)lineseg.Get (i);
	if (nSegments > 0)
	{
	  int x = clipbound.xmin + i * branchdeltax + branchdeltax/2 - deltax;
	  Line (x, y, x, y+h/(i==nLevel && nSegments==1 ? 2 : 1), CSPAL_TREECTRL_BRANCH);
	  if (i == nLevel)
	  {
	    Line (x, y+h/2, x+branchdeltax/2, y+h/2, CSPAL_TREECTRL_BRANCH);
	    // if this node is not a leaf and is not open, we draw a little info that there are more children
	    if (!node->IsLeaf () && !node->open)
	      Box (x-2, y+h/2 - 2, x+ 3, y+h/2 + 3, CSPAL_TREECTRL_BRANCHKNOB);
	  }
	  
	}
      }
      y+=h;
    }

    lineseg[nLevel] = (void*)((int)lineseg[nLevel] - 1);

    node=node->Next ();
    if (node == firstnode) break;
  }  

}

void csTreeCtrl::PlaceItems (bool setscrollbars)
{
  int cury = BorderHeight;
  bool vis = false;
  TreeCtrlNode *node, *focusedNode, *firstnode, *firstVisNode;

  fPlaceItems = false;

  // if focused item is not selectable, find next selectable item
  if (focused && !focused->GetState (CSS_SELECTABLE))
  {
    focusedNode = node = treeroot->FindItem (focused);
    if (node)
      do
      {
	node = node->Next ();
	if (ULong (node->item->SendCommand (cscmdTreeItemCheck, NULL)) == CS_TREEITEMCHECK_SELECTED)
	  break;
      } while (node != focusedNode);
    if (node == focusedNode)
    {
      SetFocused (NextChild (focused));
      focused->SetState (CSS_TREEITEM_SELECTED, true);
    }
    else
      focused = node->item;
  } /* endif */

  firstnode = node = (TreeCtrlNode*)(treeroot->IsLeaf () ? NULL : treeroot->children.Get (0));
  firstVisNode = treeroot->FindItem (firstvisible);
  if (!firstVisNode)
    firstVisNode = node;

  csRect itembound;
  csRect clipbound (BorderWidth, BorderHeight, bound.Width () - BorderWidth,
    bound.Height () - BorderHeight);
  if (hscroll)
    clipbound.ymax = hscroll->bound.ymin;
  if (vscroll)
    clipbound.xmax = vscroll->bound.xmin;
  vertcount = 0;
  // collect item statistics
  maxdeltax = 0;
  int itemcount = 0;
  int numfirst = 0;
  bool foundfirst = false;
  csVector levels;
  int nLevel;

  levels.Push (treeroot);

  while (node && node->item)
  {
    nLevel = levels.Find (node->parent);
    if (nLevel != -1)
      levels.Insert (nLevel+1, node);
    else
      nLevel = 0;
    nLevel++;
    if (node == firstVisNode)
    {
      foundfirst = true;
      // start reserving space from first visible item
      vis = true;
    } /* endif */

    int w, h;
    node->item->SuggestSize (w, h);
    if (w && h)
    {
      itemcount++;
      if (!foundfirst)
        numfirst++;
      if (w + nLevel * branchdeltax > maxdeltax)
	maxdeltax = w + nLevel * branchdeltax;
    } /* endif */

    if (vis)
    {
      // Query current item width and height
      if (h > 0)
      {
        // Set current item x,y and height
        itembound.Set (BorderWidth+nLevel * branchdeltax, cury, 
	               BorderWidth+nLevel * branchdeltax+w, cury + h);
        itembound.Intersect (clipbound);
        if (!itembound.IsEmpty ())
          vertcount++;
        node->item->SetRect (itembound);
        cury += h;
      } /* endif */
    }else
      if (w && h)
        node->item->SetRect (0, 0, -1, -1);
    node = node->Next ();
    if (node == firstnode)
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
    if (hscroll)
    {
      hsbstatus.size = clipbound.Width ();
      hsbstatus.maxsize = maxdeltax;
      maxdeltax -= clipbound.Width ();
      //      maxdeltax += deltax;
      if (maxdeltax < 0) maxdeltax = 0;
      hsbstatus.value = MIN (maxdeltax, deltax);
      hsbstatus.maxvalue = maxdeltax;
      hsbstatus.step = 1;
      hsbstatus.pagestep = TREE_HORIZONTAL_PAGESTEP;
      hscroll->SendCommand (cscmdScrollBarSet, &hsbstatus);
    } /* endif */
  } /* endif */

  // @@@ Code below "mirrors" what we just done above, except we now know the maxdelta
  // now fine tune the location of the tree elements according to deltax
  deltax = MIN (maxdeltax, deltax);
  levels.DeleteAll ();
  levels.Push (treeroot);
  nLevel = 0;
  node = firstnode;
  while (node)
  {
    nLevel = levels.Find (node->parent);
    if (nLevel != -1)
      levels.Insert (nLevel+1, node);
    else
      nLevel = 0;
    
    int xOff;
    
    int w,h;
    node->item->SuggestSize (w, h);
    node->item->bound.xmin -= deltax;
    node->item->bound.xmax = node->item->bound.xmin + w;
    w = node->item->bound.xmin;
    node->item->bound.Intersect (clipbound);
    //    node->item->SetRect (itembound);
    xOff = MAX (0, clipbound.xmin - w);
    node->item->SendCommand (cscmdTreeItemSetHorizOffset, (void *)xOff);
    node = node->Next ();
    if (node == firstnode)
      break;
  }

}

bool csTreeCtrl::SetRect (int xmin, int ymin, int xmax, int ymax)
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
    child->SetState (CSS_TREEITEM_SELECTED, true);
  return false;
}

static bool do_deselect (csComponent *child, void *param)
{
  if (child != (csComponent *)param)
    child->SetState (CSS_TREEITEM_SELECTED, false);
  return false;
}

static bool do_findtext (csComponent *child, void *param)
{
  return (strcmp (child->GetText (), (char *)param) == 0);
}

bool csTreeCtrl::HandleEvent (iEvent &Event)
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
        if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
        {
          TreeCtrlNode *node = treeroot->FindItem (focused);
          node = node->Prev ();
          SendCommand (cscmdTreeTrack, (node ? (void *)node->item : NULL));
          return true;
        } /* endif */
        return false;
      case CSKEY_DOWN:
        if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0) 
        {
          TreeCtrlNode *node = treeroot->FindItem (focused);
          node = node->Next ();
          SendCommand (cscmdTreeTrack, (void *)node->item);
          return true;
        } /* endif */
        return false;
      case CSKEY_LEFT:
        if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL) 
        {
          if (deltax > TREE_HORIZONTAL_PAGESTEP)
            deltax -= TREE_HORIZONTAL_PAGESTEP;
          else
            deltax = 0;
          PlaceItems ();
          return true;
        }
        else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0) 
        {
          if (deltax > 0)
            deltax--;
          else
            return true;
          PlaceItems ();
          return true;
        } /* endif */
        return false;
      case CSKEY_RIGHT:
        if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL) 
        {
          if (deltax + TREE_HORIZONTAL_PAGESTEP <= maxdeltax)
            deltax += TREE_HORIZONTAL_PAGESTEP;
          else
            deltax = maxdeltax;
          PlaceItems ();
          return true;
        }
        else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0) 
        {
          if (deltax  < maxdeltax)
            deltax++;
          else
            return true;
          PlaceItems ();
          return true;
        } /* endif */
        return false;
      case CSKEY_PGUP:
        if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0) 
        {
          TreeCtrlNode *node = treeroot->FindItem (focused);
          for (int i = 0; node && i < vertcount; i++)
          {
            node = node->Prev ();
            SendCommand (cscmdTreeTrack, (node ? (void *)node->item : NULL));
          }
          return true;
        }
        else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL) 
        {
          TreeCtrlNode *node = (TreeCtrlNode *)(treeroot->IsLeaf () ? NULL : 
                                        ((TreeCtrlNode *)treeroot->children.Get (0))->Next ());
          SendCommand (cscmdTreeTrack, (node? NULL : (void *)node->item) );
          return true;
        }
        return false;
      case CSKEY_PGDN:
        if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
        {
          TreeCtrlNode *node = treeroot->FindItem (focused);
          for (int i = 0; node && i < vertcount; i++)
          {
            node = node->Next ();
            SendCommand (cscmdTreeTrack, (void *)node->item);
          }
          return true;
        }
        else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL)
        {
          SendCommand (cscmdTreeTrack, (void *)GetLast ());
          return true;
        }
        return false;
      case CSKEY_HOME:
        if ((Event.Key.Modifiers & CSMASK_CTRL) && (deltax != 0))
        {
          deltax = 0;
          PlaceItems ();
          return true;
        }
        else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
        {
          TreeCtrlNode *node = (TreeCtrlNode *)(treeroot->IsLeaf () ? NULL : 
                                                treeroot->children.Get (0));
          SendCommand (cscmdTreeTrack, (node ? (void *)node->item : NULL));
          return true;
        }
        return false;
      case CSKEY_END:
        if ((Event.Key.Modifiers & CSMASK_CTRL) && (deltax != maxdeltax)) 
        {
          deltax = maxdeltax;
          PlaceItems ();
          return true;
        }
        else if ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == 0)
        {
          SendCommand (cscmdTreeTrack, (void *)GetLast ());
          return true;
        }
        return false;
      case '/':
        if ((TreeStyle & CSTS_MULTIPLESEL) 
           && ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL)) 
        {
          ForEachItem (do_select, NULL, false);
          return true;
        } /* endif */
        return false;
      case '\\':
        if ((TreeStyle & CSTS_MULTIPLESEL) 
           && ((Event.Key.Modifiers & CSMASK_ALLSHIFTS) == CSMASK_CTRL)) 
        {
          ForEachItem (do_deselect, NULL);
          return true;
        } /* endif */
        return false;
      default:
        if ((Event.Key.Char >= ' ') && (Event.Key.Char <= 255) 
           && !(Event.Key.Modifiers & (CSMASK_CTRL | CSMASK_ALT))) 
        {
          // Find first next item that starts with this letter
          TreeCtrlNode *node, *focNode = treeroot->FindItem (focused);
          if (focNode)
          {
            node = focNode->Next ();
            while (node != focNode)
              if (node->item->SendCommand (cscmdTreeItemCheck, NULL) 
                 && (UPPERCASE (node->item->GetText () [0]) == UPPERCASE (Event.Key.Char)))
              {
                SendCommand (cscmdTreeTrack, (void *)node->item);
                return true;
              }
              else
                node = node->Next ();
          }
          return true;
        }
    } /* endswitch */
    break;
  case csevCommand:
    switch (Event.Command.Code)
    {
    case cscmdTreeClear:
      if (app->MouseOwner == this)
	app->CaptureMouse (NULL);
      delete treeroot; treeroot = new TreeCtrlNode (NULL, NULL, true);
      firstvisible = first = NULL;
      return true;
    case cscmdTreeItemSelected:
      if ((TreeStyle & CSTS_MULTIPLESEL) == 0)
	ForEachItem (do_deselect, Event.Command.Info);
      // fallback to resend
    case cscmdTreeItemDeselected:
    case cscmdTreeItemClicked:
      // resend command to parent
      if (parent)
	parent->HandleEvent (Event);
      return true;
    case cscmdTreeItemDoubleClicked:
      SwitchOpenState ((csComponent*)Event.Command.Info);
      // resend command to parent
      if (parent)
	parent->HandleEvent (Event);
      return true;
    case cscmdTreeStartTracking:
      {
	csComponent *item = (csComponent *)Event.Command.Info;
	selstate = item->GetState (CSS_TREEITEM_SELECTED) != 0;
	app->CaptureMouse (this);
	
	SetFocused (item);
	Select ();
	if (TreeStyle & CSTS_MULTIPLESEL)
	{
	  if (app->GetKeyState (CSKEY_CTRL))
	    item->SetState (CSS_TREEITEM_SELECTED, selstate = !selstate);
	  else
	  {
	    ForEachItem (do_deselect, (void *)item);
	    item->SetState (CSS_TREEITEM_SELECTED, selstate = true);
	  } /* endif */
	}
	else
	  item->SetState (CSS_TREEITEM_SELECTED, selstate = true);
	break;
      }
    case cscmdTreeTrack:
      {
	csComponent *item = (csComponent *)Event.Command.Info;
	if (item)
	{
	  if (app->MouseOwner != this)
	    selstate = true;
	  if (item->GetState (CSS_SELECTABLE) && item->SendCommand (cscmdTreeItemCheck)) 
	  {
	    if (app->MouseOwner != this)
	      ForEachItem (do_deselect, (void *)item);
	    SetFocused (item);
	    Select ();
	    item->SetState (CSS_TREEITEM_SELECTED, (TreeStyle & CSTS_MULTIPLESEL) ? selstate : true);
	  } /* endif */
	}
	return true;
      }
    case cscmdTreeMakeVisible:
      MakeItemVisible ((csComponent *)Event.Command.Info);
      return true;
    case cscmdTreeQueryFirstSelected:
      Event.Command.Info = FindFirstSelected ();
      return true;
    case cscmdTimerPulse:
      if (app && app->MouseOwner == this)
      {
        GetMousePosition (Event.Mouse.x, Event.Mouse.y);
	if (app->MouseOwner == this)
	{
	  TreeCtrlNode *node = treeroot->FindItem (focused);
	  if (Event.Mouse.y < BorderHeight)
	  {
	    node = (node ? node->Prev () : NULL );
	    SendCommand (cscmdTreeTrack, (node ? (void *)node->item : NULL));
	  }
	  else if ((Event.Mouse.y > bound.Height () - BorderHeight)
		    || (hscroll  && (Event.Mouse.y >= hscroll->bound.ymin)))
	  {
	    node = (node ? node->Next () : NULL);
	    SendCommand (cscmdTreeTrack, (node ? (void *)node->item : NULL));
	  }
	} /* endif */
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
	} 
	else if (bar == vscroll)
	{
	  vsbstatus = sbs;
	  TreeCtrlNode *cur, *firstNode;
	  cur = firstNode = (TreeCtrlNode*)treeroot->children.Get (0);
	  do
	  {
	    if (cur->item->SendCommand (cscmdTreeItemCheck, NULL)) 
	    {
	      if (sbs.value == 0) 
	      {
		if (firstvisible != cur->item) 
		{
		  firstvisible = cur->item;
		  PlaceItems (false);
		} /* endif */
		break;
	      } /* endif */
	      sbs.value--;
	    } /* endif */
	    cur = cur->Next ();
	  } while (cur != firstNode); /* enddo */
	} /* endif */
	Invalidate ();
	return true;
      }
      break;
    case cscmdTreeSelectItem:
      Event.Command.Info = ForEachItem (do_findtext, (char *)Event.Command.Info);
      return true;
    case cscmdTreeAddChild:
      Event.Command.Info = (void*)AddChild (((csComponent**)Event.Command.Info)[0],
                                            ((csComponent**)Event.Command.Info)[1]);
      return true;
    case cscmdTreeRemoveChild:
      RemoveChild ((csComponent*)Event.Command.Info);
      Event.Command.Info = NULL;
      return true;
    case cscmdTreeRemoveAll:
      RemoveAll ();
      return true;
    } /* endswitch */
    break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

void csTreeCtrl::MakeItemVisible (csComponent *item)
{
  TreeCtrlNode *cur=treeroot->FindItem (item);
  if (!cur) 
    return;

  // are the item parent nodes all open ?
  bool bOpen = true;
  while (bOpen && cur)
  {
    cur = (TreeCtrlNode*)cur->parent;
    if (cur) bOpen = cur->open;
  }
  if (!bOpen) 
    return;

  if (!item->SendCommand (cscmdTreeItemScrollVertically, (void *)false))
  {
    // item is already visible
    return;
  }

  TreeCtrlNode *firstNode = (TreeCtrlNode*)treeroot->children.Get (0);
  TreeCtrlNode *firstVisNode = treeroot->FindItem (firstvisible);
  if (!firstVisNode) 
    firstVisNode = firstNode;

  cur = firstVisNode;
  while (cur && (cur != firstNode) && (cur->item != item))
    cur = cur->Prev ();

  if (cur && cur->item == item)
    firstvisible = cur->item;
  else
  {
    cur = treeroot->FindItem (item);
    int cy = bound.Height () - BorderHeight;
    if (hscroll)
      cy = hscroll->bound.ymin;
    firstvisible = item;
    firstVisNode = cur;
    while (firstVisNode != firstNode && cur && cur->item)
    {
      int w, h;
      cur->item->SuggestSize (w, h);
      cy -= h;
      if (cy < BorderHeight)
        break;
      firstvisible = cur->item;
      firstVisNode = cur;
      cur = cur->Prev ();
    } /* endwhile */
  } /* endif */
  PlaceItems ();
}

csComponent *csTreeCtrl::ForEachItem (bool (*func) (csComponent *child,
  void *param), void *param, bool iSelected)
{
  if (!func)
    return NULL;

  csComponent *start = first;
  csComponent *cur = start;
  while (cur)
  {
    csComponent *next = cur->next;

    ULong reply = (long)cur->SendCommand (cscmdTreeItemCheck, NULL);
    bool ok;
    if (iSelected)
      ok = (reply == CS_TREEITEMCHECK_SELECTED);
    else
      ok = (reply != 0);
    if (ok && func (cur, param))
      return cur;
    if ((cur == next) || ((cur = next) == start))
      break;
  } /* endwhile */
  return NULL;
}

void csTreeCtrl::SetState (int mask, bool enable)
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

bool csTreeCtrl::SetFocused (csComponent *comp)
{
  if (!csComponent::SetFocused (comp))
    return false;
  if (parent)
    parent->SendCommand (cscmdTreeItemFocused, comp);
  return true;
}

void csTreeCtrl::Insert (csComponent *comp)
{
  fPlaceItems = true;
  Invalidate ();
  csComponent::Insert (comp);
}

void csTreeCtrl::Delete (csComponent *comp)
{
  fPlaceItems = true;
  Invalidate ();
  csComponent::Delete (comp);
}

bool csTreeCtrl::CompareTreeCtrlNode (csTreeNode *node, csSome param, bool stopOnSuccess)
{
  (void)stopOnSuccess;
  return ((csTreeCtrl::TreeCtrlNode*)node)->item == (csComponent*)param;
}

csTreeCtrl::TreeCtrlNode *csTreeCtrl::TreeCtrlNode::FindItem (csComponent *theItem)
{ 
  return (TreeCtrlNode *)DSF (CompareTreeCtrlNode, NULL, theItem, true); 
}

csTreeCtrl::TreeCtrlNode *csTreeCtrl::TreeCtrlNode::Next (TreeCtrlNode* after)
{
  /**
   * "this" node is somewhere in the tree, we now determine which node is the next below this node if we 
   * would draw the entire tree.
   */
  TreeCtrlNode *foundNode=NULL;

  if (!open || (!after && IsLeaf ()) || (after && children.Find (after) == children.Length ()-1))
  {
    if (parent)
      return ((TreeCtrlNode *)parent)->Next (this);
    else
      return (IsLeaf () ? NULL : (TreeCtrlNode*)children.Get (0) );
  }
  else
  {
    if (!after)
      foundNode = (TreeCtrlNode*)children.Get (0);
    else
    {
      int idx = children.Find (after);
      if (idx != -1)
	foundNode = (TreeCtrlNode*)children.Get (idx+1);
    }
  }
  return foundNode;
}

csTreeCtrl::TreeCtrlNode *csTreeCtrl::TreeCtrlNode::Prev (TreeCtrlNode* before)
{
  /**
   * "this" node is somewhere in the tree, we now determine which node is the next below this node if we 
   * would draw the entire tree.
   */
  TreeCtrlNode *foundNode=NULL, *node;
  if (!before || !open )
  {
    if (parent)
      return ((TreeCtrlNode *)parent)->Prev (this);
    else
      return NULL;
  }
  else
  {
    int idx = children.Find (before)-1;
    if (idx >= 0)
    {
      node = (TreeCtrlNode*)children.Get (idx);
      if (!node->open || node->IsLeaf ())
	foundNode = node;
      else
      {
	while (!node->IsLeaf () && node->open)
	  node = (TreeCtrlNode*)node->children.Get (node->children.Length ()-1);
	foundNode = node;
      }
    }
    else if(idx == -1) 
           return this;
  }
  return foundNode;
}

bool csTreeCtrl::BranchOpen (csTreeNode *node)
{
  return ((csTreeCtrl::TreeCtrlNode*)node)->open || node->IsLeaf ();
}

bool csTreeCtrl::TreeItemSelected (csTreeNode *node, csSome param, bool stopOnSuccess)
{
  (void)param;
  (void)stopOnSuccess;
  csComponent *c = ((csTreeCtrl::TreeCtrlNode*)node)->item;
  bool isSel = c && c->GetState (CSS_TREEITEM_SELECTED);
  if (param && isSel)
  {
    ((csVector*)param)->Push (c);
  }
  return isSel;
}

csComponent *csTreeCtrl::FindFirstSelected ()
{
  csTreeNode *node = treeroot->DSF (TreeItemSelected, BranchOpen, NULL, true);
  return (node? NULL : ((TreeCtrlNode*)node)->item);
}

void csTreeCtrl::FindAllSelected (csVector *list)
{
  treeroot->DSF (TreeItemSelected, BranchOpen, (csSome)list, false);
}

csComponent *csTreeCtrl::GetLast ()
{
  TreeCtrlNode *node = treeroot;
  while (!node->IsLeaf () && node->open)
    node = (TreeCtrlNode *)node->children.Get (node->children.Length () -1);
  return (node == treeroot ? NULL : node->item);
}

bool csTreeCtrl::AddChild (csComponent *item1, csComponent *item2)
{
  bool succ = false;
  // first check if item2 is already i the tree
  TreeCtrlNode *node2 = treeroot->FindItem (item2);
  TreeCtrlNode *node1;
  if (item1 == this)
    node1 = treeroot;
  else
    node1 = treeroot->FindItem (item1);
  if (node1)
  {
    if (node2)
    {
      if (node2->parent != node1)
      {
	// move item2 to new parent
	if (node2->parent) 
	  node2->parent->RemoveChild (node2);
	node1->AddChild (node2);
      }
    }
    else
    {
      (void)new TreeCtrlNode (item2, node1, false);
    }
    succ = true;
  }
  return succ;
}

void csTreeCtrl::RemoveChild (csComponent *item)
{
  TreeCtrlNode *node = treeroot->FindItem (item);
  if (node)
    delete node;
}

void csTreeCtrl::RemoveAll ()
{
  delete treeroot;
  treeroot = new TreeCtrlNode (NULL, NULL, true);
  PlaceItems ();
  Invalidate ();
}

bool csTreeCtrl::ZipTreeItemCanvas (csTreeNode *node, csSome param, bool stopOnSuccess)
{
  (void)param;
  (void)stopOnSuccess;
  csComponent *c = ((csTreeCtrl::TreeCtrlNode*)node)->item;
  if (c) c->bound.Set (-1, -1, -1, -1);
  return false;
}

void csTreeCtrl::SwitchOpenState (csComponent *item)
{
  TreeCtrlNode *node = treeroot->FindItem (item);
  if (node)
  {
    node->open = !node->open;
    treeroot->DSF (ZipTreeItemCanvas, NULL, NULL, false);
    PlaceItems ();
    Invalidate ();
  }
}

void csTreeCtrl::OpenItem (csComponent *item)
{
  TreeCtrlNode *node = treeroot->FindItem (item);
  if (node && !node->open)
  {
    node->open = true;
    treeroot->DSF (ZipTreeItemCanvas, NULL, NULL, false);
    PlaceItems ();
    Invalidate ();
  }
}

bool csTreeCtrl::OpenAllItems (csTreeNode *node, csSome param, bool stopOnSuccess)
{
  (void)param;
  (void)stopOnSuccess;
  ((csTreeCtrl::TreeCtrlNode*)node)->open = true;
  return false;
}

void csTreeCtrl::OpenAll ()
{
  treeroot->DSF (OpenAllItems, NULL, NULL, false);
  treeroot->DSF (ZipTreeItemCanvas, NULL, NULL, false);
  PlaceItems ();
  Invalidate ();
}

void csTreeCtrl::CollapseItem (csComponent *item)
{
  TreeCtrlNode *node = treeroot->FindItem (item);
  if (node && node->open)
  {
    node->open = false;
    treeroot->DSF (ZipTreeItemCanvas, NULL, NULL, false);
    PlaceItems ();
    Invalidate ();
  }
}

bool csTreeCtrl::CollapsAllItems (csTreeNode *node, csSome param, bool stopOnSuccess)
{
  (void)param;
  (void)stopOnSuccess;
  ((csTreeCtrl::TreeCtrlNode*)node)->open = false;
  return false;
}

void csTreeCtrl::CollapseAll ()
{
  treeroot->DSF (CollapsAllItems, NULL, NULL, false);
  treeroot->DSF (ZipTreeItemCanvas, NULL, NULL, false);
  PlaceItems ();
  Invalidate ();
}

void csTreeCtrl::SetBranchIndent (int x)
{
  if (x != branchdeltax)
  {
    branchdeltax = x;
    PlaceItems ();
    Invalidate ();
  }
}
