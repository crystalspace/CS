/*
    Crystal Space Windowing System: Windowing System Component
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

#include <stddef.h>
#include <ctype.h>
#include "sysdef.h"
#include "csengine/csspr2d.h"
#include "csinput/csevent.h"
#include "csinput/csinput.h"
#include "qint.h"
#include "igraph2d.h"
#include "csws/cscomp.h"
#include "csws/csmouse.h"
#include "csws/csapp.h"
#include "csws/cswsutil.h"

// The csComponent class itself
csComponent::csComponent (csComponent *iParent) : state (CSS_VISIBLE),
  originalpalette (true), DragStyle (CS_DRAG_MOVEABLE), clipparent (NULL),
  text (NULL), Font (csFontParent), Maximized (false), focused (NULL),
  top (NULL), next (NULL), prev (NULL), parent (NULL), app (NULL), id (0)
{
  SetPalette (NULL, 0);
  if (iParent)
    iParent->Insert (this);
}

csComponent::~csComponent ()
{
  if (text)
    CHKB (delete[] text);
  if (app && (app->MouseOwner == this))
    app->CaptureMouse (NULL);
  if (app && (app->KeyboardOwner == this))
    app->CaptureKeyboard (NULL);
  if (app && (app->FocusOwner == this))
    app->CaptureFocus (NULL);

  DeleteAll ();

  SetPalette (NULL, 0);
  if (parent != clipparent)
    clipparent->DeleteClipChild (this);
  if (parent)
    parent->Delete (this);
}

static bool do_delete (csComponent *child, void *param)
{
  (void)param;
  CHK (delete child);
  return false;
}

void csComponent::DeleteAll ()
{
  ForEach (do_delete);
  focused = NULL;
  top = NULL;
}

void csComponent::SetPalette (int *iPalette, int iPaletteSize)
{
  if (!originalpalette)
    free (palette);
  palette = iPalette; palettesize = iPaletteSize;
  originalpalette = true;
}

void csComponent::SetColor (int Index, int Color)
{
  if (originalpalette)
  {
    void *temp = malloc (palettesize * sizeof (int));
    memcpy (temp, palette, palettesize * sizeof (int));
    palette = (int *)temp;
    originalpalette = false;
  }
  palette[Index] = Color;
}

static bool set_app (csComponent *child, void *param)
{
  child->SetApp ((csApp *)param);
  return false;
}

void csComponent::SetApp (csApp *newapp)
{
  app = newapp;
  ForEach (set_app, (void *)newapp);
}

void csComponent::InsertClipChild (csComponent *clipchild)
{
  if (clipchildren.Find ((void *)clipchild) < 0)
  {
    if (clipchild->clipparent)
      clipchild->clipparent->DeleteClipChild (clipchild);
    clipchildren.Push ((void *)clipchild);
  }
  clipchild->clipparent = this;
}

void csComponent::DeleteClipChild (csComponent *clipchild)
{
  int num = clipchildren.Find ((void *)clipchild);
  if (num >= 0)
    clipchildren.Delete (num);
}

void csComponent::Insert (csComponent *comp)
{
  if (!comp)
    return;

  if (comp->parent)
    comp->parent->Delete (comp);
  comp->SetApp (app);
  comp->clipparent = comp->parent = this;
  if (focused)
  {
    comp->prev = focused->prev;
    comp->next = focused;
    focused->prev->next = comp;
    focused->prev = comp;
    if ((!focused->GetState (CSS_SELECTABLE))
     && (comp->GetState (CSS_SELECTABLE)))
      focused = comp;
  }
  else
  {
    comp->next = comp;
    comp->prev = comp;
    if (GetState (CSS_FOCUSED))
      SetFocused (comp);
    else
      focused = comp;
    top = comp;
  } /* endif */
}

void csComponent::Delete (csComponent *comp)
{
  if (focused)
  {
    csComponent *cur = focused;
    do
    {
      if (cur == comp)
      {
        cur->Hide ();
        if (cur == focused)
        {
          focused = (cur->next == focused ? (csComponent*)NULL : cur->next);
          if (focused && GetState (CSS_FOCUSED))
            focused->SetState (CSS_FOCUSED, true);
        } /* endif */
        if (cur == top)
          top = top->next == top ? (csComponent*)NULL : top->next;
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        cur->next = cur->prev = cur->clipparent = cur->parent = NULL;
        return;
      } /* endif */
    } while ((cur = cur->next) != focused); /* enddo */
  } /* endif */
}

csComponent *csComponent::ForEach (bool (*func) (csComponent *child, void *param),
  void *param, bool Zorder)
{
  if (!func)
    return NULL;

  csComponent *cur = Zorder ? top : focused;
  csComponent *last = NULL;
  if (cur)
    last = Zorder ? cur->next : cur->prev;
  while (cur)
  {
    csComponent *next = Zorder ? cur->prev : cur->next;
    if (func (cur, param))
      return cur;
    if (cur == last)
      break;
    cur = next;
  } /* endwhile */
  return NULL;
}

static bool find_child_by_id (csComponent *child, void *find_id)
{
  return (child->id == (unsigned int)find_id);
}

csComponent *csComponent::GetChild (int find_id)
{
  return ForEach (find_child_by_id, (void *)find_id);
}

bool csComponent::SetFocused (csComponent *comp)
{
  if (!comp || (comp->parent != this))
    return false;
  if (focused != comp)
  {
    csComponent *oldfocused;
    csComponent *olddefault = GetDefault ();
    do
    {
      // if focused child gets changed during following SetState, restart
      oldfocused = focused;
      if (focused)
        focused->SetState (CSS_FOCUSED, false);
      if (focused == comp)
        return true;
    } while (focused != oldfocused);
    if (!comp->GetState (CSS_VISIBLE))
      comp->SetState (CSS_VISIBLE, true);
    if (GetState (CSS_FOCUSED))
      comp->SetState (CSS_FOCUSED, true);
    focused = comp;
    csComponent *newdefault = GetDefault ();
    if (newdefault != olddefault)
    {
      if (olddefault)
        olddefault->Invalidate ();
      if (newdefault)
        newdefault->Invalidate ();
    } /* endif */
  } /* endif */
  return true;
}

bool csComponent::SetZorder (csComponent *comp, csComponent *below)
{
  // Handle degenerated cases first
  if (!comp || (comp->parent != this) || (below && (below->parent != this)))
    return false;
  if ((comp == below) || (comp->next == comp))
    return true;

  /* When a window changes its Z-order, all windows which after this
   * operation will rise their Z-order should be invalidated (only
   * the rectangle which intersects the bound of moving window).
   */
  csComponent *last = comp->prev == top ? (csComponent*)NULL : comp->prev;

  // Remove component from parent's child list
  comp->prev->next = comp->next;
  comp->next->prev = comp->prev;
  // If below == top, then comp becomes the top window
  if (below == top)
    top = comp;
  // Now find the insertion point
  if (!below)
    below = top;
  // Insert component in new place
  comp->next = below->next;
  comp->prev = below;
  below->next->prev = comp;
  below->next = comp;

  /* Now invalidate the intersection rectangle between window that changed
   * its Z-order and all windows that rised. If the 'comp' window rised itself,
   * only it should be invalidated, othewise we should invalidate all windows
   * in rising Z-order until 'last' is hit.
   */

  // Decide whenever 'comp' rised or lowered
  csComponent *cur = comp;
  while ((cur != top) && (cur != last))
    cur = cur->next;
  if (cur == top)
  {
    // if we hit the top Z-order window, the window has rised
    csRect inv;
    cur = last ? last->next : top->next;
    while (cur != top)
    {
      inv.Union (cur->bound);
      cur = cur->next;
    } /* endwhile */
    inv.Move (-top->bound.xmin, -top->bound.ymin);
    top->Invalidate (inv, true);
  } else
  {
    // otherwise it has lowered
    csRect inv;
    cur = comp;
    while (cur != last)
    {
      cur = cur->next;
      inv.Set (
        comp->bound.xmin - cur->bound.xmin,
        comp->bound.ymin - cur->bound.ymin,
        comp->bound.xmax - cur->bound.xmin,
        comp->bound.ymax - cur->bound.ymin);
      cur->Invalidate (inv, true);
    } /* endwhile */
  } /* endif */

  return true;
}

int csComponent::dragX;
int csComponent::dragY;
int csComponent::dragMode = 0;
csRect csComponent::dragBound;

bool csComponent::do_handle_event (csComponent *child, void *param)
{
  csEvent *Event = (csEvent *)param;

  switch (Event->Type)
  {
    case csevBroadcast:
      child->HandleEvent (*Event);
      return false;
    case csevMouseMove:
    case csevMouseDoubleClick:
    case csevMouseDown:
    case csevMouseUp:
    {
      // If child is not visible, skip it
      // If child has another clip parent, skip it
      if ((!child->GetState (CSS_VISIBLE))
       || (child->clipparent != child->parent))
        return false;

      bool retc;
      // Bring mouse coordinates to child coordinate system
      int dX = child->bound.xmin, dY = child->bound.ymin;
      Event->Mouse.x -= dX;
      Event->Mouse.y -= dY;
      if (child->bound.ContainsRel (Event->Mouse.x, Event->Mouse.y))
      {
        retc = child->HandleEvent (*Event);
        if (child->GetState (CSS_TRANSPARENT) == 0)
          retc = true;
      } else
        retc = false;
      Event->Mouse.x += dX;
      Event->Mouse.y += dY;
      return retc;
    }
    case csevKeyDown:
    case csevKeyUp:
      if (!child->GetState (CSS_VISIBLE))
        return false;
      // fallback to default behaviour
    default:
      return child->HandleEvent (*Event);
  } /* endswitch */
}

bool csComponent::HandleEvent (csEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      // If this is a post-process broadcast, redraw and pass it to children
      if (Event.Command.Code == cscmdRedraw)
      {
        Redraw ();                              // redraw invalidated rectangle
        ForEach (do_handle_event, &Event);
        return false;
      }
      break;
    case csevCommand:
      // Handle quit message
      if (Event.Command.Code == cscmdQuit)
        app->ShutDown ();
      break;
  } /* endswitch */

  // If in drag mode, handle mouse events
  if (dragMode)                                 // Component is in drag mode?
  {
    switch (Event.Type)
    {
      case csevKeyDown:
        if (Event.Key.Code != CSKEY_ESC)
          break;
        if (app->MouseOwner != this)
          return (ForEach (do_handle_event, &Event, false) != NULL);
AbortDrag:
        SetRect (dragBound.xmin, dragBound.ymin, dragBound.xmax, dragBound.ymax);
        dragMode = 0;
        if (app->MouseOwner == this)
          app->CaptureMouse (NULL);
        return true;
      case csevMouseDown:
        if (Event.Mouse.Button == 2)
          goto AbortDrag;
        return true;
      case csevMouseUp:
        // since we don't know which mouse button has initiated dragging,
        // we'll abort on first mouse up message
        dragMode = 0;
        if (app->MouseOwner == this)
          app->CaptureMouse (NULL);
        return true;
      case csevMouseMove:
      {
        int dX = Event.Mouse.x, dY = Event.Mouse.y;
        LocalToGlobal (dX, dY);
        dX -= dragX; dY -= dragY;
        int newXmin = dragBound.xmin, newXmax = dragBound.xmax;
        int newYmin = dragBound.ymin, newYmax = dragBound.ymax;

        if (dragMode & CS_DRAG_XMIN) newXmin += dX;
        if (dragMode & CS_DRAG_XMAX) newXmax += dX;
        if (dragMode & CS_DRAG_YMIN) newYmin += dY;
        if (dragMode & CS_DRAG_YMAX) newYmax += dY;

        SetDragRect (newXmin, newYmin, newXmax, newYmax);
        SetSizingCursor (dragMode);
        return true;
      }
    } /* endswitch */
  } /* endif */

  bool ret = false;
  if (IS_KEYBOARD_EVENT (Event))
  {
    if (focused && focused->GetState (CSS_VISIBLE))
      ret = focused->HandleEvent (Event);
    else
      ret = false;
  } else
  {
    if (IS_MOUSE_EVENT (Event))
    {
      // Pass mouse events to 'clip children'
      for (int i = clipchildren.Length () - 1; i >= 0; i--)
      {
        csComponent *child = (csComponent *)clipchildren [i];
        // If child is not visible, skip it
        if (child->GetState (CSS_VISIBLE))
        {
          // Bring mouse coordinates to child coordinate system
          int dX = 0, dY = 0;
          child->LocalToGlobal (dX, dY);
          GlobalToLocal (dX, dY);
          Event.Mouse.x -= dX;
          Event.Mouse.y -= dY;
          if (child->bound.ContainsRel (Event.Mouse.x, Event.Mouse.y))
          {
            child->HandleEvent (Event);
            ret = true;
          } else
            ret = false;
          Event.Mouse.x += dX;
          Event.Mouse.y += dY;
        } /* endif */
        if (ret)
          break;
      } /* endfor */
    } /* endif */
    // Command messages should be not be passed to children
    if (Event.Type != csevCommand)
      // For mouse events, proceed from Z-order top child,
      // for all other events proceed from focused child
      if (!ret)
        ret = (ForEach (do_handle_event, &Event, IS_MOUSE_EVENT (Event)) != NULL);
  } /* endif */

  if (ret)
    return true;

  // Handle some events that works for all components
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdClose:
          Close ();
          return true;
        case cscmdHide:
          if (GetState (CSS_VISIBLE))
            Hide ();
          else
            Show ();
          return true;
        case cscmdMaximize:
          if (!Maximized)
            Maximize ();
          else
            Restore ();
          return true;
      } /* endswitch */
      break;
    case csevMouseDown:
    case csevMouseDoubleClick:
      // If mouse has been clicked on this component, no child eaten
      // the event and the component is selectable - select it
      if ((Event.Mouse.Button == 1)
       && (!GetState (CSS_FOCUSED))
       && (GetState (CSS_SELECTABLE)))
      {
        Select ();
        return true;
      }
      if ((Event.Mouse.Button == 2)
       && (app->FocusOwner == this)
       && (!bound.ContainsRel (Event.Mouse.x, Event.Mouse.y)))
        app->Dismiss (cscmdCancel);
      break;
    case csevKeyDown:
      if ((Event.Key.Code == CSKEY_ESC)
       && (app->FocusOwner == this))
        app->Dismiss (cscmdCancel);
      break;
  } /* endswitch */

  // Event has not been consumed
  return false;
}

static bool do_prehandle_event (csComponent *child, void *param)
{
  csEvent *Event = (csEvent *)param;

  if (IS_MOUSE_EVENT (*Event))
  {
    // Bring mouse coordinates to child coordinate system
    int dX = child->bound.xmin, dY = child->bound.ymin;
    Event->Mouse.x -= dX;
    Event->Mouse.y -= dY;
    bool ret = child->PreHandleEvent (*Event);
    Event->Mouse.x += dX;
    Event->Mouse.y += dY;
    return ret;
  } else
    return child->PreHandleEvent (*Event);
}

bool csComponent::PreHandleEvent (csEvent &Event)
{
  return !!(ForEach (do_prehandle_event, &Event, IS_MOUSE_EVENT (Event)));
}

static bool do_posthandle_event (csComponent *child, void *param)
{
  csEvent *Event = (csEvent *)param;

  if (IS_MOUSE_EVENT (*Event))
  {
    // Bring mouse coordinates to child coordinate system
    int dX = child->bound.xmin, dY = child->bound.ymin;
    Event->Mouse.x -= dX;
    Event->Mouse.y -= dY;
    bool ret = child->PostHandleEvent (*Event);
    Event->Mouse.x += dX;
    Event->Mouse.y += dY;
    return ret;
  } else
    return child->PostHandleEvent (*Event);
}

bool csComponent::PostHandleEvent (csEvent &Event)
{
  return !!(ForEach (do_posthandle_event, &Event, IS_MOUSE_EVENT (Event)));
}

void *csComponent::SendCommand (int CommandCode, void *Info)
{
  if (this)
  {
    csEvent Event (0, csevCommand, CommandCode, Info);
    HandleEvent (Event);
    return Event.Command.Info;
  } else
    return Info;
}

void csComponent::Drag (int x, int y, int DragMode)
{
  if (DragMode == CS_DRAG_ALL)
  {
    if (!(DragStyle & CS_DRAG_MOVEABLE))
      return;
  }
  else if (!(DragStyle & CS_DRAG_SIZEABLE))
    return;

  dragMode = DragMode;
  LocalToGlobal (x, y);
  dragX = x; dragY = y;
  dragBound.Set (bound);
  Select ();
  app->CaptureMouse (this);
}

bool csComponent::Select ()
{
  if (!parent)
    return true;
  if (!GetState (CSS_SELECTABLE))
    return false;
  if (!parent->Select ())
    return false;
  parent->SetFocused (this);
  if (GetState (CSS_TOPSELECT))
    parent->SetZorder (this, parent->top);
  return true;
}

static bool do_find_default (csComponent *child, void *param)
{
  (void)param;
  if (child->GetState (CSS_SELECTABLE))
    return (child->SendCommand (cscmdAreYouDefault, NULL) == child);
  else
    return false;
}

csComponent *csComponent::GetDefault ()
{
  if (GetState (CSS_FOCUSED))
  {
    csComponent *c = ForEach (do_find_default);
    if (c)
      return c;
    else if (focused
          && (focused->GetState (CSS_FOCUSED | CSS_SELECTABLE) ==
             (CSS_FOCUSED | CSS_SELECTABLE)))
      return focused;
  } /* endif */
  return NULL;
}

csComponent *csComponent::NextChild (csComponent *start, bool disabled)
{
  if (!start) start = focused;
  csComponent *cur = start->next;
  while (cur != start
      && !cur->GetState (CSS_SELECTABLE)
      && (!disabled || !cur->GetState (CSS_DISABLED)))
    cur = cur->next;
  return cur;
}

csComponent *csComponent::PrevChild (csComponent *start, bool disabled)
{
  if (!start) start = focused;
  csComponent *cur = start->prev;
  while (cur != start
      && !cur->GetState (CSS_SELECTABLE)
      && (!disabled || !cur->GetState (CSS_DISABLED)))
    cur = cur->prev;
  return cur;
}

csComponent *csComponent::NextControl (csComponent *start)
{
  if (!start)
    start = focused;
  csComponent *cur = start;
  do
  {
    cur = NextChild (cur, true);
    if (cur->GetState (CSS_GROUP))
    {
      cur = start;
      while (!cur->GetState (CSS_GROUP))
        cur = PrevChild (cur, true);
      if (cur->GetState (CSS_DISABLED))
        cur = cur->NextChild (cur);
      break;
    }
  } while ((cur != start) && !cur->GetState (CSS_SELECTABLE));
  return cur;
}

csComponent *csComponent::PrevControl (csComponent *start)
{
  if (!start)
    start = focused;
  csComponent *cur = start;
  if (cur->GetState (CSS_GROUP))
  {
    csComponent *prev = start;
    do
    {
      if (cur->GetState (CSS_SELECTABLE))
        prev = cur;
      cur = NextChild (cur, true);
    } while (!cur->GetState (CSS_GROUP));
    return prev;
  }
  do
  {
    cur = PrevChild (cur, true);
    if (cur->GetState (CSS_SELECTABLE))
      return cur;
    if (cur->GetState (CSS_GROUP))
      return PrevControl (cur);
  } while (cur != start);
  return start;
}

csComponent *csComponent::NextGroup (csComponent *start)
{
  if (!start)
    start = focused;
  csComponent *cur = start;
  while (((cur = NextChild (cur, true)) != start)
      && !cur->GetState (CSS_GROUP)) ;
  if (cur == start)
    return NextControl ();
  if (cur->GetState (CSS_DISABLED))
    return NextChild (cur);
  return cur;
}

csComponent *csComponent::PrevGroup (csComponent *start)
{
  if (!start)
    start = focused;
  csComponent *cur = start;
  while (!cur->GetState (CSS_GROUP)
      && ((cur = PrevChild (cur, true)) != start)) ;
  while (((cur = PrevChild (cur, true)) != start)
      && !cur->GetState (CSS_GROUP)) ;
  if (cur == start)
    return PrevControl ();
  if (cur->GetState (CSS_DISABLED))
    return NextChild (cur);
  return cur;
}

bool csComponent::FixFocused ()
{
  csComponent *start = focused;
  csComponent *cur = start;

  do
  {
    if (cur->GetState (CSS_SELECTABLE))
      break;
  } while ((cur = cur->next) != start);

  if (cur == focused)
    return true;
  else
  {
    SetFocused (cur);
    return false;
  }
}

void csComponent::Redraw ()
{
  if (!dirty.IsEmpty ())
  {
    SetClipRect ();
    Draw ();
    dirty.MakeEmpty ();
  }
}

void csComponent::Draw ()
{
  // no operation
}

void csComponent::Show (bool focused)
{
  if (!GetState (CSS_VISIBLE))
    SetState (CSS_VISIBLE, true);
  if (focused && !GetState (CSS_FOCUSED))
    Select ();
}

void csComponent::Hide ()
{
  if (!GetState (CSS_VISIBLE))
    return;
  if (parent && (parent->focused == this))
    parent->SetFocused (prev);
  if (parent && (parent->top == this))
    parent->top = prev;

  SetState (CSS_VISIBLE, false);
  dirty.MakeEmpty ();

  if (parent != clipparent)
  {
    csRect clipbound (bound);
    if (parent)
    {
      parent->LocalToGlobal (clipbound.xmin, clipbound.ymin);
      parent->LocalToGlobal (clipbound.xmax, clipbound.ymax);
    } /* endif */
    if (clipparent)
    {
      clipparent->GlobalToLocal (clipbound.xmin, clipbound.ymin);
      clipparent->GlobalToLocal (clipbound.xmax, clipbound.ymax);
      clipparent->Invalidate (clipbound, true);
    } /* endif */
  } else
    parent->Invalidate (bound, true);
}

bool csComponent::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (bound.Equal (xmin, ymin, xmax, ymax))
    return false;
  csRect inv (bound);
  bound.Set (xmin, ymin, xmax, ymax);
  if (parent)
  {
    inv.Exclude (xmin, ymin, xmax, ymax);
    parent->Invalidate (inv, true);
  }
  Invalidate (true);
  return true;
}

bool csComponent::SetDragRect (int xmin, int ymin, int xmax, int ymax)
{
  // Allow component to fix up size and position according to its needs
  int w = xmax - xmin;
  int h = ymax - ymin;
  FixSize (w, h);
  if (xmax == bound.xmax)
    xmin = xmax - w;
  else
    xmax = xmin + w;
  if (ymax == bound.ymax)
    ymin = ymax - h;
  else
    ymax = ymin + h;
  FixPosition (xmin, ymin);
  return SetRect (xmin, ymin, xmax, ymax);
}

static bool do_invalidate (csComponent *child, void *param)
{
  csRect dr (*((csRect *)param));
  dr.Move (-child->bound.xmin, -child->bound.ymin);
  child->Invalidate (dr, true);
  return false;
}

void csComponent::Invalidate (csRect &area, bool IncludeChildren)
{
  if (!GetState (CSS_VISIBLE))
    return;

  csRect inv (0, 0, bound.Width (), bound.Height ());
  inv.Intersect (area);
  if (inv.IsEmpty ())
    return;

  dirty.Union (inv);
  if (app)
    app->RedrawFlag = true;

  if (parent && GetState (CSS_TRANSPARENT))
  {
    inv.Move (bound.xmin, bound.ymin);
    parent->Invalidate (inv);
  } /* endif */

  if (IncludeChildren)
  {
    ForEach (do_invalidate, &dirty);
    csRect dr;
    for (int i = clipchildren.Length () - 1; i >= 0; i--)
    {
      csComponent *child = (csComponent *)clipchildren [i];
      if (child->GetState (CSS_VISIBLE))
      {
        int dX = 0, dY = 0;
        LocalToGlobal (dX, dY);
        child->GlobalToLocal (dX, dY);
        dr.Set (dirty);
        dr.Move (dX, dY);
        child->Invalidate (dr, true);
      } /* endif */
    } /* endfor */
  } /* endif */
}

void csComponent::ClipAlienChildren (csObjVector &rect, csComponent *child)
{
  int i;

  for (int c = clipchildren.Length () - 1; c >= 0; c--)
  {
    csComponent *nb = (csComponent *)clipchildren [c];

    if (nb == child)
      break;

    if (nb->GetState (CSS_TRANSPARENT))
    {
      // if given children is transparent, invalidate
      // its areas covered by rect[]
      csRect inv;
      for (i = rect.Length () - 1; i >= 0; i--)
      {
        inv.Set (*((csRect *)rect[i]));
        int dX = 0, dY = 0;
        LocalToGlobal (dX, dY);
        nb->GlobalToLocal (dX, dY);
        inv.Move (dX, dY);
        nb->Invalidate (inv, true);
      } /* endfor */
    } else if (nb->GetState (CSS_VISIBLE))
      for (i = rect.Length () - 1; i >= 0; i--)
      {
        csRect *cur = (csRect *)rect[i];
        csRect childbound (nb->bound);
        if (nb->parent)
        {
          nb->parent->LocalToGlobal (childbound.xmin, childbound.ymin);
          nb->parent->LocalToGlobal (childbound.xmax, childbound.ymax);
        } /* endif */
        GlobalToLocal (childbound.xmin, childbound.ymin);
        GlobalToLocal (childbound.xmax, childbound.ymax);
        if (childbound.Intersects (*cur))
        {
          csRect r (cur->xmin, cur->ymin, cur->xmax, childbound.ymin);
          r.Intersect (*cur);
          if (!r.IsEmpty ())
            CHKB (rect.Push (new csRect (r)));

          r.Set (cur->xmin, childbound.ymax, cur->xmax, cur->ymax);
          r.Intersect (*cur);
          if (!r.IsEmpty ())
            CHKB (rect.Push (new csRect (r)));

          r.Set (cur->xmin, childbound.ymin, childbound.xmin, childbound.ymax);
          r.Intersect (*cur);
          if (!r.IsEmpty ())
            CHKB (rect.Push (new csRect (r)));

          r.Set (childbound.xmax, childbound.ymin, cur->xmax, childbound.ymax);
          r.Intersect (*cur);
          if (!r.IsEmpty ())
            CHKB (rect.Push (new csRect (r)));

          rect.Delete (i);
        } /* endif */
      } /* endfor */
  } /* endif */
}

void csComponent::Clip (csObjVector &rect, csComponent *last)
{
  int i; // for dumb compilers that doesn't understand ANSI C++ "for" scoping

  if (GetState (CSS_VISIBLE) == 0)
  {
    rect.DeleteAll ();
    return;
  }
  // Clip all rectangles against this window
  csRect relbound (0, 0, bound.xmax - bound.xmin, bound.ymax - bound.ymin);
  for (i = rect.Length () - 1; i >= 0; i--)
  {
    ((csRect *)rect[i])->Intersect (relbound);
    if (((csRect *)rect[i])->IsEmpty ())
      rect.Delete (i);
  } /* endfor */

  // Clip against children if this component is child's real parent
  if ((last == this) || (last->parent == this))
    if (top && top != last)             // If component has children
    {
      csComponent *nb = top;
      do
      {
        if (nb->GetState (CSS_TRANSPARENT))
        {
          // if given children is transparent, invalidate
          // its areas covered by rect[]
          csRect inv;
          // Trick: to avoid being invalidated by invalidated children,
          // we'll drop the visibility flag, so that Invalidate () will exit.
          int oldstate = state;
          state &= ~CSS_VISIBLE;
          for (i = rect.Length () - 1; i >= 0; i--)
          {
            inv.Set (*((csRect *)rect[i]));
            inv.Move (-nb->bound.xmin, -nb->bound.ymin);
            nb->Invalidate (inv, true);
          } /* endfor */
          state = oldstate;
        } else if ((nb->GetState (CSS_VISIBLE)) && (nb->clipparent == this))
          for (i = rect.Length () - 1; i >= 0; i--)
          {
            csRect *cur = (csRect *)rect[i];
            if (nb->bound.Intersects (*cur))
            {
              csRect r (cur->xmin, cur->ymin, cur->xmax, nb->bound.ymin);
              r.Intersect (*cur);
              if (!r.IsEmpty ())
                CHKB (rect.Push (new csRect (r)));

              r.Set (cur->xmin, nb->bound.ymax, cur->xmax, cur->ymax);
              r.Intersect (*cur);
              if (!r.IsEmpty ())
                CHKB (rect.Push (new csRect (r)));

              r.Set (cur->xmin, nb->bound.ymin, nb->bound.xmin, nb->bound.ymax);
              r.Intersect (*cur);
              if (!r.IsEmpty ())
                CHKB (rect.Push (new csRect (r)));

              r.Set (nb->bound.xmax, nb->bound.ymin, cur->xmax, nb->bound.ymax);
              r.Intersect (*cur);
              if (!r.IsEmpty ())
                CHKB (rect.Push (new csRect (r)));

              rect.Delete (i);
            } /* endif */
          } /* endfor */
        nb = nb->prev;
      } while ((nb != top) && (nb != last)); /* enddo */
    } /* endif */

  // Search for 'clip children' and clip rectangls against their bounds
  // (see the remark upon definition of csComponent::parentclip)
  ClipAlienChildren (rect, last);

  if (clipparent != parent)
  {
    // transfer rectangles from parent's coordinates to clip parent's
    int dX = bound.xmin, dY = bound.ymin;
    if (parent)
      parent->LocalToGlobal (dX, dY);
    if (clipparent)
      clipparent->GlobalToLocal (dX, dY);
    for (i = (int)rect.Length () - 1; i >= 0; i--)
      ((csRect *)rect[i])->Move (dX, dY);
  } else
  {
    // transform all rectangles from local window coordinates
    // to parent window coordinates
    for (i = (int)rect.Length () - 1; i >= 0; i--)
      ((csRect *)rect[i])->Move (bound.xmin, bound.ymin);
  } /* endif */

  if (clipparent)
    clipparent->Clip (rect, this);
}

void csComponent::LocalToGlobal (int &x, int &y)
{
  x += bound.xmin;
  y += bound.ymin;
  if (parent)
    parent->LocalToGlobal (x, y);
}

void csComponent::GlobalToLocal (int &x, int &y)
{
  x -= bound.xmin;
  y -= bound.ymin;
  if (parent)
    parent->GlobalToLocal (x, y);
}

int csComponent::GetFont ()
{
  if (Font != csFontParent)
    return Font;
  else if (parent)
    return parent->GetFont ();
  else
    return csFontCourier;
}

int csComponent::TextWidth (const char *text)
{
  return app->TextWidth (text, GetFont ());
}

int csComponent::TextHeight ()
{
  return app->TextHeight (GetFont ());
}

void csComponent::Box (int xmin, int ymin, int xmax, int ymax, int colindx)
{
  if ((xmin >= xmax) || (ymin >= ymax))
    return;
  csObjVector rect (8, 4);
  CHK (csRect *bb = new csRect (xmin, ymin, xmax, ymax));
  bb->Intersect (dirty);
  if (!clip.IsEmpty ())
    bb->Intersect (clip);
  rect.Push (bb);
  Clip (rect, this);
  for (int i = rect.Length () - 1; i >= 0; i--)
  {
    csRect *cur = (csRect *)rect[i];
    app->pplBox (cur->xmin, cur->ymin, cur->xmax, cur->ymax,
                 GetColor (colindx));
  } /* endfor */
}

void csComponent::Line (float x1, float y1, float x2, float y2, int colindx)
{
  // First clip the line against dirty rectangle
  if (app->ClipLine (x1, y1, x2, y2,
        dirty.xmin, dirty.ymin, dirty.xmax, dirty.ymax))
    return;

 /* Do clipping as follows: create a minimal rectangle which fits the line,
  * clip the rectangle against children & parents, then clip the line against
  * all resulting rectangles.
  */
  csObjVector rect (8, 4);
  CHK (csRect *lb = new csRect (QInt (x1), QInt (y1), QInt (x2), QInt (y2)));
  lb->Normalize ();
  lb->xmax += 2;
  lb->ymax += 2;
  lb->Intersect (dirty);
  if (!clip.IsEmpty ())
    lb->Intersect (clip);
  rect.Push (lb);
  Clip (rect, this);

  int dx = 0; int dy = 0;
  LocalToGlobal (dx, dy);
  x1 += dx; y1 += dy;
  x2 += dx; y2 += dy;

  int color = GetColor (colindx);
  for (int i = rect.Length () - 1; i >= 0; i--)
  {
    csRect *cur = (csRect *)rect[i];
    float xx1 = x1, xx2 = x2, yy1 = y1, yy2 = y2;

    if (!app->ClipLine (xx1, yy1, xx2, yy2,
        cur->xmin, cur->ymin, cur->xmax, cur->ymax))
      app->pplLine (xx1, yy1, xx2, yy2, color);
  }
}

void csComponent::Pixel (int x, int y, int colindx)
{
  // First clip the pixel against dirty rectangle
  if (!dirty.Contains (x, y))
    return;

  csObjVector rect (8, 4);
  CHK (csRect *lb = new csRect (x, y, x + 1, y + 1));
  lb->Intersect (dirty);
  if (!clip.IsEmpty ())
    lb->Intersect (clip);
  rect.Push (lb);
  Clip (rect, this);

  LocalToGlobal (x, y);

  int color = GetColor (colindx);
  for (int i = rect.Length () - 1; i >= 0; i--)
  {
    csRect *cur = (csRect *)rect[i];
    app->pplPixel (cur->xmin, cur->ymin, color);
  }
}

void csComponent::Text (int x, int y, int fgindx, int bgindx, const char *s)
{
  if (!s)
    return;

 /* Do clipping as follows: create a minimal rectangle which fits the string,
  * clip the rectangle against children & parents, then clip the string against
  * all resulting rectangles.
  */
  csObjVector rect (8, 4);
  csRect tb (x, y, x + TextWidth (s), y + TextHeight ());
  tb.Intersect (dirty);
  if (!clip.IsEmpty ())
    tb.Intersect (clip);
  CHK (rect.Push (new csRect (tb)));
  Clip (rect, this);

  int ox = x, oy = y;
  LocalToGlobal (x, y);
  tb.Move (x - ox, y - oy);
  bool restoreclip = false;
  for (int i = rect.Length () - 1; i >= 0; i--)
  {
    csRect *cur = (csRect *)rect[i];
    if (cur->Intersects (tb))
    {
      app->pplSetClipRect (*cur); restoreclip = true;
      app->pplText (x, y, GetColor (fgindx), bgindx >= 0 ? GetColor (bgindx) : -1,
        GetFont (), s);
    } /* endif */
  } /* endfor */
  if (restoreclip)
    app->pplRestoreClipRect ();
}

void csComponent::Sprite2D (csSprite2D *s2d, int x, int y, int w, int h)
{
  if (!s2d)
    return;

 /* Do clipping as follows: create a minimal rectangle which fits the sprite,
  * clip the rectangle against children & parents, then clip the sprite against
  * all resulting rectangles.
  */
  csObjVector rect (8, 4);
  CHK (csRect *sb = new csRect (x, y, x + w, y + h));
  sb->Intersect (dirty);
  if (!clip.IsEmpty ())
    sb->Intersect (clip);
  rect.Push (sb);
  Clip (rect, this);

  LocalToGlobal (x, y);
  bool restoreclip = false;
  for (int i = rect.Length () - 1; i >= 0; i--)
  {
    csRect *cur = (csRect *)rect[i];
    app->pplSetClipRect (*cur); restoreclip = true;
    app->pplSprite2D (s2d, x, y, w, h);
  } /* endfor */
  if (restoreclip)
    app->pplRestoreClipRect ();
}

void csComponent::Rect3D (int xmin, int ymin, int xmax, int ymax, int darkindx,
  int lightindx)
{
  if ((xmax <= xmin) || (ymax <= ymin))
    return;

  Line (xmin + 1, ymax - 1, xmax - 2, ymax - 1, darkindx);
  Line (xmax - 1, ymax - 1, xmax - 1, ymin, darkindx);
  Line (xmax - 2, ymin, xmin, ymin, lightindx);
  Line (xmin, ymin + 1, xmin, ymax - 1, lightindx);
}

void csComponent::ObliqueRect3D (int xmin, int ymin, int xmax, int ymax,
  int cornersize, int darkindx, int lightindx)
{
  if ((xmax <= xmin) || (ymax <= ymin))
    return;
  Line (xmax - 1, ymin + 1, xmax - 1, ymax - cornersize - 1, darkindx);
  Line (xmax - 1, ymax - cornersize, xmax - cornersize, ymax - 1, darkindx);
  Line (xmax - cornersize - 1, ymax - 1, xmin + 1, ymax - 1, darkindx);
  Line (xmin, ymax - 1, xmin, ymin + cornersize, lightindx);
  Line (xmin, ymin + cornersize - 1, xmin + cornersize - 1, ymin, lightindx);
  Line (xmin + cornersize, ymin, xmax - 1, ymin, lightindx);
}

void csComponent::SetState (int mask, bool enable)
{
  int oldstate = state;

  // If visibility changes, invalidate/hide the window
  if ((mask & CSS_VISIBLE)
   && (!!(state & CSS_VISIBLE) != enable))
  {
    if (enable)
    {
      state |= CSS_VISIBLE;
      Invalidate (true);
    } else
    {
      state &= ~CSS_VISIBLE;
      Hide ();
    } /* endif */
  } /* endif */

  // If disabled status changed, change selectable status too
  if ((mask & CSS_DISABLED)
   && !!(state & CSS_DISABLED) != enable)
  {
    if (!(mask & CSS_SELECTABLE))
      SetState (CSS_SELECTABLE, !enable);
    Invalidate ();
  } /* endif */

  if (enable)
    state |= mask;
  else
    state &= ~mask;

  // Propagate focused flag through all child windows
  if (((oldstate ^ state) & CSS_FOCUSED)
   && (focused))
    focused->SetState (CSS_FOCUSED, enable);

  // If selectable state changed to false, select next parent's child
  // If it changes to true, select us if parent's focused is not selectable
  if ((mask & CSS_SELECTABLE)
   && (parent))
    if (state & CSS_SELECTABLE == 0)
      parent->SetFocused (parent->PrevChild ());
    else if (!parent->focused->GetState (CSS_SELECTABLE))
      parent->focused = this;
}

void csComponent::SetMouse (csMouseCursorID Cursor)
{
  if (app)
    app->SetMouseCursor (Cursor);
}

void csComponent::SetSizingCursor (int dragtype)
{
  if ((dragtype & CS_DRAG_ALL) == CS_DRAG_ALL)
    SetMouse (csmcMove);
  else if (((dragtype & (CS_DRAG_XMIN | CS_DRAG_YMIN)) == (CS_DRAG_XMIN | CS_DRAG_YMIN))
        || ((dragtype & (CS_DRAG_XMAX | CS_DRAG_YMAX)) == (CS_DRAG_XMAX | CS_DRAG_YMAX)))
    SetMouse (csmcSizeNWSE);
  else if (((dragtype & (CS_DRAG_XMAX | CS_DRAG_YMIN)) == (CS_DRAG_XMAX | CS_DRAG_YMIN))
        || ((dragtype & (CS_DRAG_XMIN | CS_DRAG_YMAX)) == (CS_DRAG_XMIN | CS_DRAG_YMAX)))
    SetMouse (csmcSizeNESW);
  else if (((dragtype & CS_DRAG_XMIN) == CS_DRAG_XMIN)
        || ((dragtype & CS_DRAG_XMAX) == CS_DRAG_XMAX))
    SetMouse (csmcSizeEW);
  else if (((dragtype & CS_DRAG_YMIN) == CS_DRAG_YMIN)
        || ((dragtype & CS_DRAG_YMAX) == CS_DRAG_YMAX))
    SetMouse (csmcSizeNS);
}

bool csComponent::GetMousePosition (int &x, int &y)
{
  app->GetMouse ()->GetPosition (x, y);
  GlobalToLocal (x, y);
  return bound.ContainsRel (x, y);
}

bool csComponent::HandleDragEvent (csEvent &Event, int BorderW, int BorderH)
{
  switch (Event.Type)
  {
    case csevMouseDown:
      if (bound.ContainsRel (Event.Mouse.x, Event.Mouse.y))
      {
        if (Event.Mouse.Button == 1)
        {
          int dragmode = 0;
          if (Event.Mouse.x < BorderW)
            dragmode |= CS_DRAG_XMIN;
          if (Event.Mouse.x >= bound.Width () - BorderW)
            dragmode |= CS_DRAG_XMAX;
          if (Event.Mouse.y < BorderH)
            dragmode |= CS_DRAG_YMIN;
          if (Event.Mouse.y >= bound.Height () - BorderH)
            dragmode |= CS_DRAG_YMAX;
          if (dragmode)
          {
            Drag (Event.Mouse.x, Event.Mouse.y, dragmode);
            return true;
          } /* endif */
        }
        else if (Event.Mouse.Button == 2)
        {
          Drag (Event.Mouse.x, Event.Mouse.y, CS_DRAG_ALL);
          return true;
        } /* endif */
      } /* endif */
    case csevMouseMove:
      if (bound.ContainsRel (Event.Mouse.x, Event.Mouse.y))
      {
        int cursortype = 0;
        if (DragStyle & CS_DRAG_SIZEABLE)
        {
          if (Event.Mouse.x < BorderW)
            cursortype |= CS_DRAG_XMIN;
          if (Event.Mouse.x >= bound.Width () - BorderW)
            cursortype |= CS_DRAG_XMAX;
          if (Event.Mouse.y < BorderH)
            cursortype |= CS_DRAG_YMIN;
          if (Event.Mouse.y >= bound.Height () - BorderH)
            cursortype |= CS_DRAG_YMAX;
	}
	if ((cursortype == 0) && (DragStyle & CS_DRAG_MOVEABLE))
          cursortype = CS_DRAG_ALL;
        if (cursortype)
          SetSizingCursor (cursortype);
      }
      break;
  } /* endswitch */
  return false;
}

void csComponent::FixPosition (int &newX, int &newY)
{
  // do nothing
  (void)newX; (void)newY;
}

void csComponent::FixSize (int &newW, int &newH)
{
  // do nothing
  (void)newW; (void)newH;
}

void csComponent::Close ()
{
  if (GetState (CSS_MODAL))
    app->Dismiss (cscmdCancel);
  else
    CHKB (delete this);
}

void csComponent::PrepareLabel (const char *iLabel, char * &oLabel,
  int &oUnderlinePos)
{
  if (oLabel)
    CHKB (delete[] oLabel);

  oUnderlinePos = -1;
  oLabel = NULL;

  if (iLabel)
  {
    int sl = strlen (iLabel);
    int cc = 0;
    int i;

    for (i = 0; i < sl; i++)
      if (iLabel [i] != '~')
        cc++;
    CHK (oLabel = new char [cc + 1]);

    cc = 0;
    for (i = 0; i < sl; i++)
      if (iLabel [i] != '~')
        oLabel[cc++] = iLabel[i];
      else
        oUnderlinePos = cc;
    oLabel [cc] = 0;
  } /* endif */
}

void csComponent::DrawUnderline (int iX, int iY, const char *iText, int iUnderlinePos,
  int iColor)
{
  if ((iUnderlinePos >= 0) && (iUnderlinePos < (int)strlen (iText)))
  {
    char tmp[256];
    strcpy (tmp, iText);
    tmp [iUnderlinePos + 1] = 0;
    int fx = TextWidth (tmp);
    tmp [iUnderlinePos] = 0;
    int sx = TextWidth (tmp);
    int sy = TextHeight ();
    sx += iX;
    fx += iX;
    sy += iY;
    Line (sx, sy, fx, sy, iColor);
  } /* endif */
}

int csComponent::WordLeft (const char *iText, int StartPos)
{
  while ((StartPos > 0) && (!isalnum (iText [StartPos - 1])))
    StartPos--;
  while ((StartPos > 0) && (isalnum (iText [StartPos - 1])))
    StartPos--;
  return StartPos;
}

int csComponent::WordRight (const char *iText, int StartPos)
{
  int sl = strlen (iText);

  if (isalnum (iText [StartPos]))
    while ((StartPos < sl) && (isalnum (iText [StartPos])))
      StartPos++;
  while ((StartPos < sl) && (!isalnum (iText [StartPos])))
    StartPos++;
  return StartPos;
}

void csComponent::SetText (const char *iText)
{
  if (!iText)
    iText = "";
  CHKB (delete [] text);
  text = strnew (iText);
  Invalidate ();
}

void csComponent::GetText (char *oText, int iTextSize)
{
  if (text)
  {
    int sl = strlen (text);
    if (sl >= iTextSize)
      sl = iTextSize - 1;
    memcpy (oText, text, sl);
    oText [sl] = 0;
  } else if (iTextSize)
   *oText = 0;
}

static bool do_setfont (csComponent *child, void *param)
{
  child->SetFont ((int)param, true);
  return false;
}

void csComponent::SetFont (int iFont, bool IncludeChildren)
{
  Font = iFont;
  if (IncludeChildren)
    ForEach (do_setfont, (void *)iFont);
}

void csComponent::SuggestSize (int &w, int &h)
{
  w = 0; h = 0;
  FixSize (w, h);
}

void csComponent::SetSuggestedSize (int dw, int dh)
{
  int w, h;
  SuggestSize (w, h);
  SetSize (w + dw, h + dh);
}

void csComponent::Center (bool iHoriz, bool iVert)
{
  if (parent)
  {
    if (iHoriz)
      SetPos ((parent->bound.Width () - bound.Width ()) / 2, bound.ymin);
    if (iVert)
      SetPos (bound.xmin, (parent->bound.Height () - bound.Height ()) / 2);
  } /* endif */
}

bool csComponent::Maximize ()
{
  if (!Maximized && (DragStyle & CS_DRAG_SIZEABLE) && parent)
  {
    OrgBound.Set (bound);
    csRect newbound (0, 0, parent->bound.Width (), parent->bound.Height ());
    // give a chance to parent window to limit "maximize" bounds
    parent->SendCommand (cscmdLimitMaximize, (void *)&newbound);
    SetRect (newbound);
    Maximized = true;
    return true;
  } /* endif */
  return false;
}

bool csComponent::Restore ()
{
  if (Maximized && (DragStyle & CS_DRAG_SIZEABLE))
  {
    csComponent::SetRect (OrgBound);
    Maximized = false;
    return true;
  } /* endif */
  return false;
}

void csComponent::FindMaxFreeRect (csRect &area)
{
  // Now compute maximal uncovered area of desktop
  csObjVector rect (8, 4);
  CHK (rect.Push (new csRect (bound)));
  Clip (rect, this);

  // Show the "wait" mouse cursor if there are too many rectangles
  csMouseCursorID oldMouse = app->GetMouseCursor ();
  if (rect.Length () > 6)
  {
    SetMouse (csmcWait);
    app->FlushEvents ();
  } /* endif */

  // Search the one with maximal area
  RectUnion (rect, area);

  // Restore mouse cursor
  SetMouse (oldMouse);
}
