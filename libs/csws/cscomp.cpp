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
#include "cssysdef.h"

#include <stddef.h>
#include <ctype.h>

#include "cstool/cspixmap.h"
#include "csutil/csinput.h"
#include "csqint.h"
#include "ivideo/graph2d.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"
#include "csws/csmouse.h"
#include "csws/cswsutil.h"
#include "csws/csskin.h"
#include "csws/cswsaux.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "csutil/event.h"

//--//--//--//--//--//--//--//--//--//--//--//--/ The csComponent class --//--//

CS_IMPLEMENT_STATIC_VAR (GetDragBoundRect, csRect, ())
csRect *csComponent::dragBound = 0;
// The visible region cache
CS_IMPLEMENT_STATIC_VAR (GetVisRegion, cswsRectVector, (8, 8))
cswsRectVector *csComponent::visregion = 0;

csComponent::csComponent (csComponent *iParent) : state (CSS_VISIBLE),
  palette (0), originalpalette (0), DragStyle (CS_DRAG_MOVEABLE),
  clipparent (0), text (0), Font (0), FontSize (0),
  focused (0), top (0), next (0), prev (0), parent (0),
  app (0), skinslice (0), skindata (0), id (0)
{
  dragBound = GetDragBoundRect ();
  visregion = GetVisRegion ();

  SetPalette (0, 0);
  if (iParent)
    iParent->Insert (this);
  // The skin slice is set as soon as PreHandleEvent() is called
  // for the first time. We cannot query skin slice since for now the
  // GetSkinName() method (which is virtual) won't work due to C++ design.
}

csComponent::~csComponent ()
{
  // Notify the application that the component is being destroyed
  if (app)
    app->NotifyDelete (this);
  if (parent != clipparent)
    clipparent->DeleteClipChild (this);
  if (parent)
    parent->Delete (this);
  if (Font)
    Font->DecRef ();

  delete [] text;
  DeleteAll ();
  SetPalette (0, 0);

  // Tell skin slice to free its private data (if any)
  // We'll reset the VISIBLE bit to avoid extra work in Reset()
  state &= ~CSS_VISIBLE;
  if (skinslice)
    skinslice->Reset (*this);
}

char *csComponent::GetSkinName ()
{
  return 0;
}

csSkin *csComponent::GetSkin ()
{
  return parent ? parent->GetSkin () : (csSkin*)0;
}

bool csComponent::ApplySkin (csSkin *Skin)
{
  if (!Skin)
    return false;

  const char *skinname = GetSkinName ();
  if (!skinname)
    return true;	// we don't need a skin slice

  size_t sliceidx = Skin->FindSortedKey (Skin->KeyCmp(skinname));
  if ((sliceidx == csArrayItemNotFound) && !skinslice)
    return false;

  if (sliceidx != csArrayItemNotFound)
    Skin->Get (sliceidx)->Apply (*this);
  return true;
}

static bool do_delete (csComponent *child, intptr_t param)
{
  (void)param;
  delete child;
  return false;
}

void csComponent::DeleteAll ()
{
  ForEach (do_delete);
  focused = 0;
  top = 0;
}

void csComponent::SetPalette (int *iPalette, int iPaletteSize)
{
  if (originalpalette != palette)
    delete [] palette;
  palette = iPalette; palettesize = iPaletteSize;
  originalpalette = iPalette;
}

void csComponent::SetColor (int Index, int Color)
{
  if (palette == originalpalette)
  {
    int *temp = new int [palettesize];
    memcpy (temp, palette, palettesize * sizeof (int));
    palette = temp;
  }
  palette [Index] = Color;
}

void csComponent::ResetPalette ()
{
  if (palette != originalpalette)
  {
    delete [] palette;
    palette = originalpalette;
  }
}

static bool set_app (csComponent *child, intptr_t param)
{
  child->SetApp ((csApp *)param);
  return false;
}

void csComponent::SetApp (csApp *newapp)
{
  app = newapp;
  ForEach (set_app, (intptr_t)newapp);
}

void csComponent::InsertClipChild (csComponent *clipchild)
{
  if (clipchildren.Find (clipchild) == csArrayItemNotFound)
  {
    if (clipchild->clipparent)
      clipchild->clipparent->DeleteClipChild (clipchild);
    clipchildren.Push (clipchild);
  }
  clipchild->clipparent = this;
}

void csComponent::DeleteClipChild (csComponent *clipchild)
{
  size_t num = clipchildren.Find (clipchild);
  if (num != csArrayItemNotFound)
    clipchildren.DeleteIndex (num);
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
          focused = (cur->next == focused) ? (csComponent *)0 : NextChild (focused);
          if (cur == focused) // Whoa, delo pahnet kerosinom
            focused = cur->next;
          else if (focused && GetState (CSS_FOCUSED))
            focused->SetState (CSS_FOCUSED, true);
        } /* endif */
        if (cur == top)
          top = top->next == top ? (csComponent*)0 : top->next;
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        cur->next = cur->prev = cur->clipparent = cur->parent = 0;
        return;
      } /* endif */
    } while ((cur = cur->next) != focused); /* enddo */
  } /* endif */
}

csComponent *csComponent::ForEach (bool (*func) (csComponent *child, intptr_t param),
  intptr_t param, bool Zorder)
{
  if (!func)
    return 0;

  csComponent *cur = Zorder ? top : focused;
  csComponent *last = 0;
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
  return 0;
}

static bool find_child_by_id (csComponent *child, intptr_t find_id)
{
  return (child->id == (csComponent::ID)find_id);
}

csComponent *csComponent::GetChild (ID find_id) const
{
  return ((csComponent *)this)->ForEach (find_child_by_id, (intptr_t)find_id);
}

bool csComponent::SetFocused (csComponent *comp)
{
  if (!comp || (comp->parent != this))
    return false;
  if (focused != comp)
  {
    // Ask parent if it agrees to move focus away from currently focused child
    if (focused && !SendCommand (cscmdLoseFocus, (intptr_t)focused))
      return false;

    // Now ask parent if it agrees to focus given child component
    if (comp && !SendCommand (cscmdReceiveFocus, (intptr_t)comp))
      return false;

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
      // csComponent::SetState will set our `focused' field
      comp->SetState (CSS_FOCUSED, true);
    else
      // Directly set the focused component
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
  csComponent *last = comp->prev == top ? (csComponent*)0 : comp->prev;

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
  }
  else
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

// These variables are used while dragging.
// Since there can be just one dragged window at any time,
// we can use static variables rather than member.
int csComponent::dragX;
int csComponent::dragY;
int csComponent::dragMode = 0;

bool csComponent::do_handle_event (csComponent *child, intptr_t param)
{
  iEvent *Event = (iEvent *)param;

  switch (Event->Type)
  {
    case csevBroadcast:
      child->HandleEvent (*Event);
      return false;
    case csevMouseMove:
    case csevMouseClick:
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
	// Check to see if the mouse was last over this same component
        if (child->app->LastMouseContainer != child)
        {
       	  // No, send a csevMouseExit to the old component, and a csevMouseEnter to the new one.
       	  if (child->app->LastMouseContainer)
	  {
	    csEvent *mouseExitEvent = new csEvent();
	    mouseExitEvent->Type = csevMouseExit;
	    mouseExitEvent->Time = Event->Time;
	    mouseExitEvent->Mouse.x = Event->Mouse.x;
	    mouseExitEvent->Mouse.y = Event->Mouse.y;
	    mouseExitEvent->Mouse.Button = Event->Mouse.Button;
	    mouseExitEvent->Mouse.Modifiers = Event->Mouse.Modifiers;
	    child->app->LastMouseContainer->HandleEvent (*mouseExitEvent);
	    mouseExitEvent->DecRef ();
       	  }

       	  if (child)
	  {
	    csEvent *mouseEnterEvent = new csEvent();
	    mouseEnterEvent->Type = csevMouseEnter;
	    mouseEnterEvent->Time = Event->Time;
	    mouseEnterEvent->Mouse.x = Event->Mouse.x;
	    mouseEnterEvent->Mouse.y = Event->Mouse.y;
	    mouseEnterEvent->Mouse.Button = Event->Mouse.Button;
	    mouseEnterEvent->Mouse.Modifiers = Event->Mouse.Modifiers;
      	    child->HandleEvent(*mouseEnterEvent);
	    mouseEnterEvent->DecRef();
	  }
	  // Save the current container.
	  child->app->LastMouseContainer = child;
        }

	// after HandleEvent() returned child might've been freed,
	// so check the state for the transparent flag in advance
	retc = (child->GetState (CSS_TRANSPARENT) == 0);
	retc = child->HandleEvent (*Event) || retc;
      }
      else
        retc = false;
      Event->Mouse.x += dX;
      Event->Mouse.y += dY;
      return retc;
    }
    case csevKeyboard:
      if (!child->GetState (CSS_VISIBLE))
        return false;
      // fallback to default behaviour
    default:
      return child->HandleEvent (*Event);
  } /* endswitch */
}

bool csComponent::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevBroadcast:
      switch (Event.Command.Code)
      {
        case cscmdSkinChanged:
        {
          const char *name = GetSkinName ();
          if (name)
            if (!ApplySkin ((csSkin *)Event.Command.Info))
            {
              app->Printf (CS_REPORTER_SEVERITY_WARNING,
                "The skin does not contain a slice for component `%s'\n", name);
              abort ();
            }
          break;
        }
        case cscmdMoveClipChildren:
          if (top)
          {
            csComponent *cur = top;
            int *delta = (int *)Event.Command.Info;
            do
            {
              if (cur->GetState (CSS_VISIBLE) && (cur->clipparent != this))
              {
                csRect r (cur->bound);
                if (delta [0] > 0)
                  cur->bound.xmax += delta [0];
                else
                  cur->bound.xmin += delta [0];
                if (delta [1] > 0)
                  cur->bound.ymax += delta [1];
                else
                  cur->bound.ymin += delta [1];
                cur->SetRect (r);
              }
              cur = cur->prev;
            } while (cur != top);
          }
          break;
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
      case csevKeyboard:
	if ((csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
	  (csKeyEventHelper::GetCookedCode (&Event) != CSKEY_ESC))
          break;
        if (app->MouseOwner != this)
          return (ForEach (do_handle_event, (intptr_t)&Event, true) != 0);
AbortDrag:
        SetRect (dragBound->xmin, dragBound->ymin, dragBound->xmax, dragBound->ymax);
        dragMode = 0;
        if (app->MouseOwner == this)
          app->CaptureMouse (0);
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
          app->CaptureMouse (0);
        return true;
      case csevMouseMove:
        if (app->MouseOwner == this)
        {
          int dX = Event.Mouse.x, dY = Event.Mouse.y;
          LocalToGlobal (dX, dY);
          dX -= dragX; dY -= dragY;
          int newXmin = dragBound->xmin, newXmax = dragBound->xmax;
          int newYmin = dragBound->ymin, newYmax = dragBound->ymax;

          if (dragMode & CS_DRAG_XMIN) newXmin += dX;
          if (dragMode & CS_DRAG_XMAX) newXmax += dX;
          if (dragMode & CS_DRAG_YMIN) newYmin += dY;
          if (dragMode & CS_DRAG_YMAX) newYmax += dY;

          SetDragRect (newXmin, newYmin, newXmax, newYmax);
          SetSizingCursor (dragMode);
          return true;
        }
        break;
    } /* endswitch */
  } /* endif */

  bool ret = false;
  if (CS_IS_KEYBOARD_EVENT (Event))
  {
    if (focused && focused->GetState (CSS_VISIBLE))
      ret = focused->HandleEvent (Event);
    else
      ret = false;
  }
  else
  {
    if (CS_IS_MOUSE_EVENT (Event))
    {
      // Pass mouse events to 'clip children'
      size_t i;
      for (i = clipchildren.Length (); i-- > 0;)
      {
        csComponent *child = clipchildren[i];
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
          }
	  else
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
        ret = (ForEach (do_handle_event, (intptr_t)&Event, CS_IS_MOUSE_EVENT (Event)) != 0);
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
          if (!GetState (CSS_MAXIMIZED))
            Maximize ();
          else
            Restore ();
          return true;
      } /* endswitch */
      break;
    case csevMouseDown:
      // If mouse has been clicked on this component, no child eaten
      // the event and the component is selectable - select it
      if ((Event.Mouse.Button == 1)
       && (!GetState (CSS_FOCUSED))
       && (GetState (CSS_SELECTABLE)))
        Select ();
      else if ((Event.Mouse.Button == 2)
       && (app->FocusOwner == this)
       && (!bound.ContainsRel (Event.Mouse.x, Event.Mouse.y)))
        app->Dismiss (cscmdCancel);
      break;
    case csevKeyboard:
      if (((csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
	  (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_ESC))
       && (app->FocusOwner == this))
        app->Dismiss (cscmdCancel);
      break;
  } /* endswitch */

  // Event has not been consumed
  return false;
}

static bool do_prehandle_event (csComponent *child, intptr_t param)
{
  iEvent *Event = (iEvent *)param;

  if (CS_IS_MOUSE_EVENT (*Event))
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

bool csComponent::PreHandleEvent (iEvent &Event)
{
  return !!(ForEach (do_prehandle_event, (intptr_t)&Event, CS_IS_MOUSE_EVENT (Event)));
}

static bool do_posthandle_event (csComponent *child, intptr_t param)
{
  iEvent *Event = (iEvent *)param;

  if (CS_IS_MOUSE_EVENT (*Event))
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

bool csComponent::PostHandleEvent (iEvent &Event)
{
  return !!(ForEach (do_posthandle_event, (intptr_t)&Event, CS_IS_MOUSE_EVENT (Event)));
}

intptr_t csComponent::SendCommand (int CommandCode, intptr_t Info)
{
  if (this)
  {
    csEvent Event (0, csevCommand, CommandCode, Info);
    HandleEvent (Event);
    return Event.Command.Info;
  }
  else
    return Info;
}

intptr_t csComponent::SendBroadcast (int CommandCode, intptr_t Info)
{
  if (this)
  {
    csEvent Event (0, csevBroadcast, CommandCode, Info);
    HandleEvent (Event);
    return Event.Command.Info;
  }
  else
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
  dragBound->Set (bound);
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

static bool do_find_default (csComponent *child, intptr_t param)
{
  (void)param;
  if (child->GetState (CSS_SELECTABLE))
    return (child->SendCommand (cscmdAreYouDefault, 0) == (intptr_t)child);
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
  return 0;
}

csComponent *csComponent::NextChild (csComponent *start, bool disabled)
{
  if (!start) start = focused;
  csComponent *cur = start->next;
  while (cur != start
      && (cur->GetState (CSS_SELECTABLE | CSS_VISIBLE) != (CSS_SELECTABLE | CSS_VISIBLE))
      && (!disabled || !cur->GetState (CSS_DISABLED)))
    cur = cur->next;
  return cur;
}

csComponent *csComponent::PrevChild (csComponent *start, bool disabled)
{
  if (!start) start = focused;
  csComponent *cur = start->prev;
  while (cur != start
      && (cur->GetState (CSS_SELECTABLE | CSS_VISIBLE) != (CSS_SELECTABLE | CSS_VISIBLE))
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

void csComponent::CheckDirtyTD (csRect &ioR)
{
  if (!GetState (CSS_VISIBLE))
    return;

  // Transform rectangle from parent's coordinate system to ours
  csRect r (ioR);
  if (!r.IsEmpty ())
  {
    // Clip to our bounds
    r.Intersect (bound);
    r.Move (-bound.xmin, -bound.ymin);
  }

  if (!r.IsEmpty () || GetState (CSS_DIRTY))
  {
    // Loop through all child components from top - down in Z order,
    // and see if any of the dirty chids are transparent; if so, we
    // should mark as dirty the respective area on their neightbours.

    // "Clip children" are always topmost, so start from them
    size_t i;
    for (i = clipchildren.Length (); i-- > 0;)
      clipchildren[i]->CheckDirtyTD (r);

    // Loop through all "direct" children top-down
    csComponent *c = top;
    if (c)
      do
      {
        if (c->clipparent == this)
          c->CheckDirtyTD (r);
        c = c->prev;
      } while (c != top);

    // Add the rectangle - after processing by all childs - to our dirty rectangle
    csRect old_dirty (dirty);
    dirty.Union (r);
    if (!dirty.Equal (old_dirty))
    {
      SetState (CSS_DIRTY, true);
      app->SetState (CSS_RESTART_DIRTY_CHECK, true);
    }
  }

  // If we are transparent, add our dirty rectangle to output rectangle.
  if (GetState (CSS_TRANSPARENT))
  {
    // Transfer rectangle to parent's coordinate system
    r.Set (dirty);
    r.Move (bound.xmin, bound.ymin);
    ioR.Union (r);
  }
  else
    // If we are not transparent subtract our bounds from output rectangle.
    ioR.Exclude (bound);
}

void csComponent::CheckDirtyBU (csRect &ioR)
{
  if (!GetState (CSS_VISIBLE))
    return;

  // Add the rectangle to our dirty area if we're transparent
  if (!ioR.IsEmpty ())
  {
    if (GetState (CSS_TRANSPARENT))
    {
      csRect r (ioR);
      r.Intersect (bound);
      // Transform rectangle from parent's coordinate system to ours
      r.Move (-bound.xmin, -bound.ymin);
      csRect old_dirty (dirty);
      dirty.Union (r);
      if (!dirty.Equal (old_dirty))
      {
        SetState (CSS_DIRTY, true);
        app->SetState (CSS_RESTART_DIRTY_CHECK, true);
      }
    }
    else
      ioR.Exclude (bound);
  }

  if (!GetState (CSS_DIRTY))
    return;

  // Loop through all child components from bottom - up in Z order,
  // and see if any of the dirty chids are transparent; if so, we
  // should propagate our dirty area to them.
  csRect r (dirty);

  // Loop through all "direct" children bottom-up
  if (top)
  {
    csComponent *bottom = top->next;
    csComponent *c = bottom;
    do
    {
      if (c->clipparent == this)
        c->CheckDirtyBU (r);
      c = c->next;
    } while (c != bottom);
  }

  // Continue with "clip children" bottom-up
  size_t i;
  for (i = 0; i < clipchildren.Length (); i++)
    clipchildren[i]->CheckDirtyBU (r);

  // Add the dirty rectangle into output rectangle
  r.Move (bound.xmin, bound.ymin);
  ioR.Union (r);
}

void csComponent::Redraw ()
{
  if (!GetState (CSS_VISIBLE))
    return;

  if (GetState (CSS_DIRTY))
  {
    if (!dirty.IsEmpty ())
    {
#if 0
// for debugging: type the title of the component, the bounds and the dirty area
printf ("%s: %d,%d (%d,%d) -- dirty: %d,%d (%d,%d)\n", text,
  bound.xmin, bound.ymin, bound.Width(), bound.Height(),
  dirty.xmin, dirty.ymin, dirty.Width(), dirty.Height());
#endif
      // Disable any additional clipping
      SetClipRect ();
      // Compute the visible region
      visregion->Push (new csRect (dirty));
      Clip (*visregion, this);
      // Perform drawing, if it makes sense
      if (visregion->Length ())
      {
        Draw ();
        // Free the visible region
        visregion->DeleteAll ();
      }
    }
    // Okay, now clear the dirty flag
    SetState (CSS_DIRTY, false);
  }

  // Now redraw all child components, from bottom to top in Z-order
  if (top)
  {
    csComponent *cur = top->next;
    csComponent *last = cur;
    do
    {
      if (cur->GetState (CSS_DIRTY))
        cur->Redraw ();
      cur = cur->next;
    } while (cur != last);
  } /* endif */
}

void csComponent::Draw ()
{
  if (skinslice)
    skinslice->Draw (*this);
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
  csRect r (bound);
  if (parent != clipparent)
  {
    int dX = 0, dY = 0;
    if (parent)
      parent->LocalToGlobal (dX, dY);
    if (clipparent)
      clipparent->GlobalToLocal (dX, dY);
    r.Move (dX, dY);
  }
  clipparent->Invalidate (r, true, this);

  if (parent && (parent->focused == this))
    parent->SetFocused (prev);
  if (parent && (parent->top == this))
    parent->top = prev;

  SetState (CSS_VISIBLE, false);
  dirty.MakeEmpty ();
}

bool csComponent::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (bound.Equal (xmin, ymin, xmax, ymax))
    return false;
  csRect inv (bound);
  bound.Set (xmin, ymin, xmax, ymax);

  // If we are invisible, return now
  if (!GetState (CSS_VISIBLE))
    return true;

  // Invalidate all children if we were moved
  bool moved = (inv.xmin != bound.xmin) || (inv.ymin != bound.ymin);
  Invalidate (moved);

  // If we were not moved, but were resized we should invalidate all the
  // child components who were perhaps exposed a little more (because of
  // clipping some components may be shown partially; when the window grows,
  // the exposed part of those components has to be invalidated).
  if (!moved && focused)
  {
    csRect br (0, inv.Height (), inv.Width (), bound.Height ());
    csRect rr (inv.Width (), 0, bound.Width (), bound.Height ());
    if (!br.IsEmpty () || !rr.IsEmpty ())
    {
      csComponent *cur = focused;
      do
      {
        if (cur->clipparent == this)
        {
          if (br.Intersects (cur->bound))
            cur->Invalidate (br.xmin - cur->bound.xmin, br.ymin - cur->bound.ymin,
              br.xmax - cur->bound.xmin, br.ymax - cur->bound.ymin, true);
          if (rr.Intersects (cur->bound))
            cur->Invalidate (rr.xmin - cur->bound.xmin, rr.ymin - cur->bound.ymin,
              rr.xmax - cur->bound.xmin, rr.ymax - cur->bound.ymin, true);
        }
        cur = cur->next;
      } while (cur != focused);

      // Now invalidate all clip children that intersects these two areas
      size_t i;
      for (i = clipchildren.Length (); i-- > 0;)
      {
        csComponent *cur = clipchildren[i];
        int dX = 0, dY = 0;
        LocalToGlobal (dX, dY);
        cur->GlobalToLocal (dX, dY);
        csRect brc (br.xmin + dX, br.ymin + dY, br.xmax + dX, br.ymax + dY);
        csRect rrc (rr.xmin + dX, rr.ymin + dY, rr.xmax + dX, rr.ymax + dY);
        if (brc.Intersects (cur->bound))
          cur->Invalidate (brc.xmin - cur->bound.xmin, brc.ymin - cur->bound.ymin,
            brc.xmax - cur->bound.xmin, brc.ymax - cur->bound.ymin, true);
        if (rrc.Intersects (cur->bound))
          cur->Invalidate (rrc.xmin - cur->bound.xmin, rrc.ymin - cur->bound.ymin,
            rrc.xmax - cur->bound.xmin, rrc.ymax - cur->bound.ymin, true);
      }
    }
  }

  // if we were moved, all our children that have another clip parent
  // were moved as well. Since they may overlap on top of other components
  // (non-our-children) we have to invalidate every of them apart.
  if (top)
  {
    int delta [2];
    delta [0] = inv.xmin - xmin;
    delta [1] = inv.ymin - ymin;
    if (delta [0] || delta [1])
      SendBroadcast (cscmdMoveClipChildren, (intptr_t)&delta);
  }

  if (parent)
  {
    csRect r (inv);
    if (!GetState (CSS_TRANSPARENT))
      r.Exclude (xmin, ymin, xmax, ymax);
    if (!r.IsEmpty ())
      parent->Invalidate (r, true, this);
  }
  if (clipparent && clipparent != parent)
  {
    csRect r (inv);
    if (!GetState (CSS_TRANSPARENT))
      r.Exclude (xmin, ymin, xmax, ymax);
    if (!r.IsEmpty ())
    {
      int dX = 0, dY = 0;
      parent->LocalToGlobal (dX, dY);
      clipparent->GlobalToLocal (dX, dY);
      r.Move (dX, dY);
      clipparent->Invalidate (r, true, this);
    }
  }

  return true;
}

bool csComponent::SetDragRect (int xmin, int ymin, int xmax, int ymax)
{
  // Allow component to fix up size and position according to its needs
  int w = xmax - xmin;
  int h = ymax - ymin;
  FixSize (w, h);
  if ((xmax == bound.xmax) && (xmin != bound.xmin))
    xmin = xmax - w;
  else
    xmax = xmin + w;
  if ((ymax == bound.ymax) && (ymin != bound.ymin))
    ymin = ymax - h;
  else
    ymax = ymin + h;
  FixPosition (xmin, ymin);
  return SetRect (xmin, ymin, xmax, ymax);
}

void csComponent::Invalidate (csRect &area, bool fIncludeChildren,
  csComponent *below)
{
  if (!GetState (CSS_VISIBLE))
    return;

  csRect inv (0, 0, bound.Width (), bound.Height ());
  inv.Intersect (area);
  if (inv.IsEmpty ())
    return;

  dirty.Union (inv);
  SetState (CSS_DIRTY, true);

  if (fIncludeChildren)
  {
    // Now invalidate all direct chidren
    csComponent *child = (below && (below->parent == this)) ? below : top;
    if (child)
      do
      {
        if (child->GetState (CSS_VISIBLE))
        {
          csRect &r = (child->clipparent == this) ? inv : area;
          if ((child != below)
           && r.Intersects (child->bound))
          {
            csRect dr (area);
            dr.Move (-child->bound.xmin, -child->bound.ymin);
            child->Invalidate (dr, true);
          }
        }
        child = child->prev;
      } while (child != top);

    // And now all indirect children
    size_t i = clipchildren.Length ();
    if (below && (below->parent != this))
    {
      while ((i > 0) && (clipchildren.Get (i - 1) != below))
        i--;
      if (i > 0)
        i--;
    }
    while (i > 0)
    {
      child = clipchildren[i - 1];
      csRect dr (area);
      int dX = 0, dY = 0;
      LocalToGlobal (dX, dY);
      child->GlobalToLocal (dX, dY);
      dr.Move (dX, dY);
      if (dr.Intersects (child->bound))
        child->Invalidate (dr, true);
      i--;
    }
  } /* endif */
}

void csComponent::ClipChild (cswsRectVector &rect, csComponent *child)
{
  if (!child->GetState (CSS_VISIBLE))
    return;

  if (child->GetState (CSS_TRANSPARENT))
  {
    // If this child is transparent, clip against all non-transparent
    // children of `child' component.

    // transform all rectangles to child coordinate system
    int dX = 0, dY = 0;
    if (child->parent != this)
    {
      child->LocalToGlobal (dX, dY);
      GlobalToLocal (dX, dY);
    }
    else
    {
      dX = child->bound.xmin;
      dY = child->bound.ymin;
    }
    int i;
    for (i = (int)rect.Length () - 1; i >= 0; i--)
      ((csRect *)rect[i])->Move (-dX, -dY);

    child->Clip (rect, child, true);

    return;
  }

  csRect childbound (child->bound);
  if (child->parent != this)
  {
    int dX = 0, dY = 0;
    if (child->parent)
      child->parent->LocalToGlobal (dX, dY);
    GlobalToLocal (dX, dY);
    // Transform child's bound to our coordinate system
    childbound.Move (dX, dY);
  } /* endif */

  // Clip child's bound against our bound (in the case child exceeds us)
  csRect relbound (0, 0, bound.Width (), bound.Height ());
  childbound.Intersect (relbound);

  // Now clip all the rectangles against this child's bound
  size_t i;
  for (i = rect.Length (); i-- > 0; )
  {
    csRect *cur = (csRect *)rect[i];
    if (childbound.Intersects (*cur))
    {
      csRect r (cur->xmin, cur->ymin, cur->xmax, childbound.ymin);
      r.Intersect (*cur);
      if (!r.IsEmpty ())
        rect.Push (new csRect (r));

      r.Set (cur->xmin, childbound.ymax, cur->xmax, cur->ymax);
      r.Intersect (*cur);
      if (!r.IsEmpty ())
        rect.Push (new csRect (r));

      r.Set (cur->xmin, childbound.ymin, childbound.xmin, childbound.ymax);
      r.Intersect (*cur);
      if (!r.IsEmpty ())
        rect.Push (new csRect (r));

      r.Set (childbound.xmax, childbound.ymin, cur->xmax, childbound.ymax);
      r.Intersect (*cur);
      if (!r.IsEmpty ())
        rect.Push (new csRect (r));

      rect.DeleteIndex (i);
    } /* endif */
  } /* endfor */
}

void csComponent::Clip (cswsRectVector &rect, csComponent *last, bool forchild)
{
  size_t i; // for dumb compilers that don't understand ANSI C++ "for" scoping

  if (!GetState (CSS_VISIBLE))
  {
    rect.DeleteAll ();
    return;
  }

  if (!forchild)
  {
    // Clip all rectangles against this window
    csRect relbound (0, 0, bound.xmax - bound.xmin, bound.ymax - bound.ymin);
    for (i = rect.Length (); i-- > 0;)
    {
      ((csRect *)rect[i])->Intersect (relbound);
      if (((csRect *)rect[i])->IsEmpty ())
        rect.DeleteIndex (i);
    } /* endfor */
  }

  // Clip against children if this component is child's clip parent
  if ((last == this) || (last->parent == this))
    if (top && top != last)             // If component has children
    {
      csComponent *nb = top;
      do
      {
        if (nb->clipparent == this)
          ClipChild (rect, nb);
        nb = nb->prev;
      } while ((nb != top) && (nb != last)); /* enddo */
    } /* endif */

  // Clip rectangles against 'clip children' bounds.
  // (see the remark at definition of csComponent::parentclip)
  size_t c;
  for (c = clipchildren.Length (); c-- > 0;)
  {
    csComponent *nb = clipchildren[c];
    if (nb == last)
      break;
    ClipChild (rect, nb);
  } /* endif */

  int dX = bound.xmin, dY = bound.ymin;
  if (clipparent != parent)
  {
    // transfer rectangles from parent's coordinates to clip parent's
    if (parent)
      parent->LocalToGlobal (dX, dY);
    if (clipparent)
      clipparent->GlobalToLocal (dX, dY);
  }

  // transform all rectangles from local window coordinates
  // to parent window coordinates
  for (i = rect.Length (); i-- > 0;)
    ((csRect *)rect[i])->Move (dX, dY);

  // Now tell our clip parent to perform further clipping
  if (!forchild && clipparent)
    clipparent->Clip (rect, this);
}

void csComponent::FastClip (cswsRectVector &rect)
{
  int dX = 0, dY = 0;
  LocalToGlobal (dX, dY);
  // Clip all the rectangles in "rect" against all the rectangles in "visregion"
  size_t i, j;
  for (i = rect.Length (); i-- > 0;)
  {
    csRect *r = (csRect *)rect.Get (i);
    // Transform rectangle to global coordinates
    csRect cr (*r);
    cr.Move (dX, dY);
    bool used = false;
    for (j = visregion->Length (); j-- > 0;)
    {
      csRect vis (*(csRect *)visregion->Get (j));
      vis.Intersect (cr);
      if (!vis.IsEmpty ())
        if (used)
          rect.Push (new csRect (vis));
        else
        {
          r->Set (vis);
          used = true;
        }
    }
    if (!used)
      rect.DeleteIndex (i);
  }
}

void csComponent::LocalToGlobal (int &x, int &y)
{
  // Recursive looks more elegant, but non-recursive is faster :-)
  register csComponent *cur = this;
  while (cur)
  {
    x += cur->bound.xmin;
    y += cur->bound.ymin;
    cur = cur->parent;
  }
}

void csComponent::GlobalToLocal (int &x, int &y)
{
  register csComponent *cur = this;
  while (cur)
  {
    x -= cur->bound.xmin;
    y -= cur->bound.ymin;
    cur = cur->parent;
  }
}

void csComponent::OtherToThis (csComponent *from, int &x, int &y)
{
  register csComponent *cur1 = from;
  while (cur1 && cur1 != this)
  {
    x += cur1->bound.xmin;
    y += cur1->bound.ymin;
    cur1 = cur1->parent;
  }
  register csComponent *cur2 = this;
  while (cur2 && cur2 != cur1)
  {
    x -= cur2->bound.xmin;
    y -= cur2->bound.ymin;
    cur2 = cur2->parent;
  }
}

void csComponent::Box (int xmin, int ymin, int xmax, int ymax, int colindx)
{
  if ((xmin >= xmax) || (ymin >= ymax))
    return;
  cswsRectVector rect (8, 4);
  csRect *bb = new csRect (xmin, ymin, xmax, ymax);
  if (!clip.IsEmpty ())
    bb->Intersect (clip);
  rect.Push (bb);
  FastClip (rect);
  size_t i;
  for (i = rect.Length (); i-- > 0;)
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
  cswsRectVector rect (8, 4);
  csRect *lb = new csRect (int (x1), int (y1), int (x2), int (y2));
  lb->Normalize ();
  lb->xmax += 1;
  lb->ymax += 1;
  if (!clip.IsEmpty ())
    lb->Intersect (clip);
  rect.Push (lb);
  FastClip (rect);

  int dx = 0; int dy = 0;
  LocalToGlobal (dx, dy);
  x1 += dx; y1 += dy;
  x2 += dx; y2 += dy;

  int color = GetColor (colindx);
  size_t i;
  for (i = rect.Length (); i-- > 0;)
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

  cswsRectVector rect (8, 4);
  csRect *lb = new csRect (x, y, x + 1, y + 1);
  if (!clip.IsEmpty ())
    lb->Intersect (clip);
  rect.Push (lb);
  FastClip (rect);

  LocalToGlobal (x, y);

  int color = GetColor (colindx);
  size_t i;
  for (i = rect.Length (); i-- > 0;)
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
  cswsRectVector rect (8, 4);
  int fh, fw = GetTextSize (s, &fh);
  csRect tb (x, y, x + fw, y + fh);
  if (!clip.IsEmpty ())
    tb.Intersect (clip);
  rect.Push (new csRect (tb));
  FastClip (rect);

  int ox = x, oy = y;
  LocalToGlobal (x, y);
  tb.Move (x - ox, y - oy);
  bool restoreclip = false;
  iFont *font;
  GetFont (font);
  size_t i;
  for (i = rect.Length (); i-- > 0;)
  {
    csRect *cur = (csRect *)rect[i];
    if (cur->Intersects (tb))
    {
      app->pplSetClipRect (*cur); restoreclip = true;
      app->pplText (x, y, GetColor (fgindx), bgindx >= 0 ? GetColor (bgindx) : -1,
        font, s);
    } /* endif */
  } /* endfor */
  if (restoreclip)
    app->pplRestoreClipRect ();
}

void csComponent::Pixmap (csPixmap *s2d, int x, int y, int w, int h, uint8 Alpha)
{
  if (!s2d)
    return;

 /* Do clipping as follows: create a minimal rectangle which fits the pixmap,
  * clip the rectangle against children & parents, then clip the pixmap against
  * all resulting rectangles.
  */
  cswsRectVector rect (8, 4);
  csRect *sb = new csRect (x, y, x + w, y + h);
  if (!clip.IsEmpty ())
    sb->Intersect (clip);
  rect.Push (sb);
  FastClip (rect);

  LocalToGlobal (x, y);
  bool restoreclip = false;
  size_t i;
  for (i = rect.Length (); i-- > 0;)
  {
    csRect *cur = (csRect *)rect[i];
    app->pplSetClipRect (*cur); restoreclip = true;
    app->pplPixmap (s2d, x, y, w, h, Alpha);
  } /* endfor */
  if (restoreclip)
    app->pplRestoreClipRect ();
}

void csComponent::Pixmap (csPixmap *s2d, int x, int y, int w, int h,
  int orgx, int orgy, uint8 Alpha)
{
  if (!s2d)
    return;

 /* Do clipping as follows: create a minimal rectangle which fits the pixmap,
  * clip the rectangle against children & parents, then clip the pixmap against
  * all resulting rectangles.
  */
  cswsRectVector rect (8, 4);
  csRect *sb = new csRect (x, y, x + w, y + h);
  if (!clip.IsEmpty ())
    sb->Intersect (clip);
  rect.Push (sb);
  FastClip (rect);

  size_t i;
  for (i = rect.Length (); i-- > 0;)
  {
    csRect *cur = (csRect *)rect [i];
    app->pplTiledPixmap (s2d, cur->xmin, cur->ymin, cur->Width (), cur->Height (),
      orgx, orgy, Alpha);
  } /* endfor */
}

void csComponent::Texture (iTextureHandle *tex, int x, int y, int w, int h,
  int orgx, int orgy, uint8 Alpha)
{
  if (!tex)
    return;

  cswsRectVector rect (8, 4);
  csRect *sb = new csRect (x, y, x + w, y + h);
  if (!clip.IsEmpty ())
    sb->Intersect (clip);
  rect.Push (sb);
  FastClip (rect);

  LocalToGlobal (orgx, orgy);
  size_t i;
  for (i = rect.Length (); i-- > 0;)
  {
    csRect *cur = (csRect *)rect [i];
    app->pplTexture (tex, cur->xmin, cur->ymin, cur->Width (), cur->Height (),
      cur->xmin - orgx, cur->ymin - orgy, cur->Width (), cur->Height (), Alpha);
  } /* endfor */
}

void csComponent::Rect3D (int xmin, int ymin, int xmax, int ymax, int darkindx,
  int lightindx)
{
  if ((xmax <= xmin) || (ymax <= ymin))
    return;

  Line (xmin + 1, ymax - 1, xmax - 1, ymax - 1, darkindx);
  Line (xmax - 1, ymax, xmax - 1, ymin + 1, darkindx);
  Line (xmax, ymin, xmin, ymin, lightindx);
  Line (xmin, ymin + 1, xmin, ymax, lightindx);
}

void csComponent::ObliqueRect3D (int xmin, int ymin, int xmax, int ymax,
  int cornersize, int darkindx, int lightindx)
{
  if ((xmax <= xmin) || (ymax <= ymin))
    return;

  Line (xmax - 1, ymin + 1, xmax - 1, ymax - cornersize, darkindx);
  Line (xmax, ymax - cornersize, xmax - cornersize, ymax, darkindx);
  Line (xmax - cornersize, ymax - 1, xmin + 1, ymax - 1, darkindx);
  Line (xmin, ymax, xmin, ymin + cornersize, lightindx);
  Line (xmin, ymin + cornersize, xmin + cornersize, ymin, lightindx);
  Line (xmin + cornersize, ymin, xmax, ymin, lightindx);
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

  // If our focused status is set, update parent `focused' field
  if ((mask & CSS_FOCUSED)
   && enable
   && parent)
    parent->focused = this;

  // Propagate focused flag through all child windows
  if (((oldstate ^ state) & CSS_FOCUSED)
   && (focused))
    focused->SetState (CSS_FOCUSED, enable);

  // If selectable state changed to false, select next parent's child
  // If it changes to true, select us if parent's focused is not selectable
  if ((mask & CSS_SELECTABLE)
   && (parent))
    if ((state & CSS_SELECTABLE) == 0)
      parent->SetFocused (parent->PrevChild ());
    else if (!parent->focused->GetState (CSS_SELECTABLE))
      parent->focused = this;

  // If dirty flag is set, propagate to parent component
  if (mask & CSS_DIRTY)
  {
    if (enable)
    {
      if (parent && !parent->GetState (CSS_DIRTY))
        parent->SetState (CSS_DIRTY, true);
    }
    else
      dirty.MakeEmpty ();
  }
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
  app->GetMouse ().GetPosition (x, y);
  GlobalToLocal (x, y);
  return bound.ContainsRel (x, y);
}

bool csComponent::HandleDragEvent (iEvent &Event, int BorderW, int BorderH)
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
        {
          SetSizingCursor (cursortype);
          return true;
        }
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
  {
    app->Dismiss (cscmdCancel);
  }
  else
    delete this;
}

csComponent *csComponent::GetChildAt (int x, int y,
  bool (*func) (csComponent *, void *), void *data)
{
  // Check clip children first
  size_t i;
  for (i = clipchildren.Length (); i-- > 0;)
  {
    csComponent *child = clipchildren[i];
    if (child->GetState (CSS_VISIBLE))
    {
      int nx = x, ny = y;
      LocalToGlobal (nx, ny);
      child->parent->GlobalToLocal (nx, ny);
      if (child->bound.Contains (nx, ny))
      {
        csComponent *r = child->GetChildAt (nx, ny, func, data);
        if (r) return r;
      }
    }
  }
  // Now check all "normal" children, starting from topmost
  csComponent *child = top;
  if (child)
  {
    int nx = x - bound.xmin, ny = y - bound.ymin;
    do
    {
      if (child->clipparent == this
       && child->GetState (CSS_VISIBLE)
       && child->bound.Contains (nx, ny))
      {
        csComponent *r = child->GetChildAt (nx, ny, func, data);
        if (r) return r;
      }
      child = child->prev;
    } while (child != top);
  }

  if (!GetState (CSS_TRANSPARENT)
   || !func
   || func (this, data))
    return this;

  return 0;
}

void csComponent::PrepareLabel (const char *iLabel, char * &oLabel,
  size_t &oUnderlinePos)
{
  if (oLabel)
    delete[] oLabel;

  oUnderlinePos = (size_t)-1;
  oLabel = 0;

  if (iLabel)
  {
    size_t sl = strlen (iLabel);
    size_t cc = 0;
    size_t i;

    for (i = 0; i < sl; i++)
      if (iLabel [i] != '~')
        cc++;
    oLabel = new char [cc + 1];

    cc = 0;
    for (i = 0; i < sl; i++)
      if (iLabel [i] != '~')
        oLabel[cc++] = iLabel[i];
      else
        oUnderlinePos = cc;
    oLabel [cc] = 0;
  } /* endif */
}

void csComponent::DrawUnderline (int iX, int iY, const char *iText, size_t iUnderlinePos,
  int iColor)
{
  if ((iUnderlinePos != (size_t)-1) && (iUnderlinePos < strlen (iText)))
  {
    size_t sl = strlen (iText) + 1;
    CS_ALLOC_STACK_ARRAY (char, tmp, sl);
    memcpy (tmp, iText, sl);
    tmp [iUnderlinePos + 1] = 0;
    int fx = GetTextSize (tmp);
    tmp [iUnderlinePos] = 0;
    int sy, sx = GetTextSize (tmp, &sy);
    sx += iX;
    fx += iX;
    sy += iY;
    Line (sx, sy, fx, sy, iColor);
  } /* endif */
}

size_t csComponent::WordLeft (const char *iText, size_t StartPos)
{
  while ((StartPos > 0) && (!isalnum (iText [StartPos - 1])))
    StartPos--;
  while ((StartPos > 0) && (isalnum (iText [StartPos - 1])))
    StartPos--;
  return StartPos;
}

size_t csComponent::WordRight (const char *iText, size_t StartPos)
{
  size_t sl = strlen (iText);

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
  delete [] text;
  text = csStrNew (iText);
  Invalidate ();
}

void csComponent::GetText (char *oText, size_t iTextSize) const
{
  if (text)
  {
    size_t sl = strlen (text);
    if (sl >= iTextSize)
      sl = iTextSize - 1;
    memcpy (oText, text, sl);
    oText [sl] = 0;
  }
  else if (iTextSize)
   *oText = 0;
}

void csComponent::SetFont (iFont *iNewFont)
{
  if (Font)
    Font->DecRef ();
  Font = iNewFont;
  if (Font)
    Font->IncRef ();
}

void csComponent::GetFont (iFont *&oFont)
{
  if (parent && (!Font))
    parent->GetFont (oFont);

  if (Font) oFont = Font;
}

int csComponent::GetTextSize (const char *text, int *oHeight)
{
  iFont *font;
  GetFont (font);
  int w, h;
  font->GetDimensions (text, w, h);
  if (oHeight) *oHeight = h;
  return w;
}

int csComponent::GetTextChars (const char *text, int iWidth)
{
  iFont *font;
  GetFont (font);
  return font->GetLength (text, iWidth);
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
  if (!GetState (CSS_MAXIMIZED) && (DragStyle & CS_DRAG_SIZEABLE) && parent)
  {
    OrgBound.Set (bound);
    csRect newbound (0, 0, parent->bound.Width (), parent->bound.Height ());
    // give a chance to parent window to limit "maximize" bounds
    parent->SendCommand (cscmdLimitMaximize, (intptr_t)&newbound);
    SetRect (newbound);
    SetState (CSS_MAXIMIZED, true);
    return true;
  } /* endif */
  return false;
}

bool csComponent::Restore ()
{
  if (GetState (CSS_MAXIMIZED) && (DragStyle & CS_DRAG_SIZEABLE))
  {
    csComponent::SetRect (OrgBound);
    SetState (CSS_MAXIMIZED, false);
    return true;
  } /* endif */
  return false;
}

void csComponent::FindMaxFreeRect (csRect &area)
{
  // Now compute maximal uncovered area of desktop
  cswsRectVector rect (8, 4);
  rect.Push (new csRect (bound));
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

bool csComponent::CheckHotKey (iEvent &iEvent, char iHotKey)
{
  csKeyModifiers m;
  csKeyEventHelper::GetModifiers (&iEvent, m);
  if ((m.modifiers[csKeyModifierTypeCtrl] != 0) ||
    (csKeyEventHelper::GetAutoRepeat (&iEvent)))
    return false;

  iHotKey = toupper (iHotKey);
  char Key;
  if (m.modifiers[csKeyModifierTypeAlt] != 0)
    Key = toupper (csKeyEventHelper::GetRawCode (&iEvent));
  else
    Key = toupper (csKeyEventHelper::GetCookedCode (&iEvent));
  return Key == iHotKey;
}
