/******************************************************************************************
 * Notes:
 *    1. The redraw process still does not properly support clipping.  This should be revisted
 *  when we figure out why the clipping mechanisms in the sofware and opengl don't work like one
 *  would expect.
 *
 */
#include "cssysdef.h"
#include "iutil/plugin.h"
#include "iutil/eventq.h"
#include "aws.h"
#include "awsprefs.h"
#include "awsfparm.h"
#include "awsclip.h"
#include "ivideo/txtmgr.h"
#include "iengine/engine.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "ivaria/reporter.h"
#include "csutil/csevent.h"

// includes for registration/embedding
#include "awscomp.h"
#include "awstimer.h"
#include "awsstdsk.h"
#include "awscmdbt.h"
#include "awslabel.h"
#include "awstxtbx.h"
#include "awsradbt.h"
#include "awschkbx.h"
#include "awsgrpfr.h"
#include "awslistbx.h"
#include "awsscrbr.h"

#include "awscmpt.h"
#include "awscscr.h"
#include "awslayot.h"

#include <stdio.h>

const int proctex_width = 512;
const int proctex_height = 512;
const int DEBUG_MANAGER = false;

// Implementation //////////////////////////////////////////////////////
awsManager::awsComponentFactoryMap::~awsComponentFactoryMap ()
{
  factory->DecRef ();
}

awsManager::awsManager (iBase *p) :
  prefmgr(NULL),
  sinkmgr(NULL),
  updatestore_dirty(true),
  top(NULL),
  mouse_in(NULL),
  keyb_focus(NULL),
  mouse_focus(NULL),
  mouse_captured(false),
  ptG2D(NULL),
  ptG3D(NULL),
  object_reg(NULL),
  canvas(NULL),
  flags(0)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = NULL;
}

awsManager::~awsManager ()
{
  if (scfiEventHandler)
  {
    iEventQueue *q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
    {
      q->RemoveListener (scfiEventHandler);
      q->DecRef ();
    }

    scfiEventHandler->DecRef ();
  }

  SCF_DEC_REF (prefmgr);
  SCF_DEC_REF (sinkmgr);
  SCF_DEC_REF (canvas);

  void *p = component_factories.GetFirstItem ();
  while ((p = component_factories.GetCurrentItem ()))
  {
    delete (awsComponentFactoryMap *)p;
    component_factories.RemoveItem ();
  }
}

bool awsManager::Initialize (iObjectRegistry *object_reg)
{
  awsManager::object_reg = object_reg;

  if (DEBUG_MANAGER) printf ("aws-debug: getting preference manager.\n");
  prefmgr = SCF_CREATE_INSTANCE (
      "crystalspace.window.preferencemanager",
      iAwsPrefManager);

  if (DEBUG_MANAGER) printf ("aws-debug: getting sink manager.\n");
  sinkmgr = SCF_CREATE_INSTANCE (
      "crystalspace.window.sinkmanager",
      iAwsSinkManager);

  if (!prefmgr)
  {
    csReport (
      object_reg,
      CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.aws",
      "AWS could not create an instance of the default PREFERENCE manager. This is a serious error.");
    return false;
  }
  else
  {
    if (DEBUG_MANAGER)
      printf (
        "aws-debug: initing and setting the internal preference manager.\n");

    prefmgr->SetWindowMgr (this);
    if (!prefmgr->Setup (object_reg)) return false;
  }

  if (!sinkmgr)
  {
    csReport (
      object_reg,
      CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.aws",
      "AWS could not create an instance of the default SINK manager. This is a serious error.");
    return false;
  }

  if (DEBUG_MANAGER) printf ("aws-debug: registering common components.\n");

  RegisterCommonComponents ();

  if (DEBUG_MANAGER) printf ("aws-debug: left aws initialize.\n");

  return true;
}

iAwsPrefManager *awsManager::GetPrefMgr ()
{
  return prefmgr;
}

iAwsSinkManager *awsManager::GetSinkMgr ()
{
  return sinkmgr;
}

void awsManager::SetPrefMgr (iAwsPrefManager *pmgr)
{
  if (prefmgr && pmgr)
  {
    prefmgr->DecRef ();
    pmgr->IncRef ();

    prefmgr = pmgr;
  }
  else if (pmgr)
  {
    pmgr->IncRef ();
    prefmgr = pmgr;
  }
}

iAwsComponent *awsManager::CreateEmbeddableComponent ()
{
  return new awsComponent ();
}

void awsManager::RegisterComponentFactory (
  awsComponentFactory *factory,
  char *name)
{
  awsComponentFactoryMap *cfm = new awsComponentFactoryMap;

  factory->IncRef ();

  cfm->factory = factory;
  cfm->id = prefmgr->NameToId (name);

  component_factories.AddItem (cfm);
}

awsComponentFactory *awsManager::FindComponentFactory (char *name)
{
  void *p = component_factories.GetFirstItem ();
  unsigned long id = prefmgr->NameToId (name);

  do
  {
    awsComponentFactoryMap *cfm = (awsComponentFactoryMap *)p;

    if (cfm->id == id) return cfm->factory;

    p = component_factories.GetNextItem ();
  } while (p != component_factories.PeekFirstItem ());

  return NULL;
}

iAwsWindow *awsManager::GetTopWindow ()
{
  return top;
}

void awsManager::SetTopWindow (iAwsWindow *_top)
{
  top = _top;
}

void awsManager::SetCanvas (iAwsCanvas *newCanvas)
{
  if (newCanvas)
  {
    if (canvas) canvas->DecRef ();
    canvas = newCanvas;
    canvas->IncRef ();

    ptG2D = canvas->G2D ();
    ptG3D = canvas->G3D ();

    ptG2D->DoubleBuffer (false);

    /*ptG3D->BeginDraw(CSDRAW_2DGRAPHICS);
    ptG2D->Clear(ptG3D->GetTextureManager()->FindRGB(255,0,255));
    ptG3D->FinishDraw();
    ptG3D->Print(NULL);*/
    prefmgr->SetTextureManager (ptG3D->GetTextureManager ());
    prefmgr->SetFontServer (ptG2D->GetFontServer ());

    frame.Set (0, 0, ptG2D->GetWidth () - 1, ptG2D->GetHeight () - 1);

    Mark (frame);
  }
}

iAwsCanvas *awsManager::GetCanvas ()
{
  //@@@ Jorrit: I think this is wrong, isn't it?
  // A getter should never increase ref count:
  // if (canvas) canvas->IncRef ();
  return canvas;
}

iAwsCanvas *awsManager::CreateDefaultCanvas (
  iEngine *engine,
  iTextureManager *txtmgr)
{
  iAwsCanvas *canvas = new awsMultiProctexCanvas (
      engine->GetContext ()->GetDriver2D ()->GetWidth (),
      engine->GetContext ()->GetDriver2D ()->GetHeight (),
      object_reg,
      engine,
      txtmgr);
  //@@@ Jorrit: When a canvas is created it already has
  // ref count 1. So doing another incref is not good.
  //SCF_INC_REF (canvas);

  return canvas;
}

iAwsCanvas *awsManager::CreateDefaultCanvas (
  iEngine *engine,
  iTextureManager *txtmgr,
  int width,
  int height,
  const char *name)
{
  iAwsCanvas *canvas = new awsSingleProctexCanvas (
      width,
      height,
      object_reg,
      engine,
      txtmgr,
      name);
  //@@@ Jorrit: When a canvas is created it already has
  // ref count 1. So doing another incref is not good.
  //SCF_INC_REF (canvas);

  return canvas;
}

iAwsCanvas *awsManager::CreateCustomCanvas (
  iGraphics2D *g2d,
  iGraphics3D *g3d)
{
  iAwsCanvas *canvas = new awsScreenCanvas (g2d, g3d);

  //@@@ Jorrit: When a canvas is created it already has
  // ref count 1. So doing another incref is not good.
  //SCF_INC_REF (canvas);

  return canvas;
}

void awsManager::Mark (csRect &rect)
{
  dirty.Include (rect);
}

void awsManager::Unmark (csRect &rect)
{
  dirty.Exclude (rect);
}

void awsManager::Erase (csRect &rect)
{
  erase.Include (rect);
}

void awsManager::MaskEraser (csRect &rect)
{
  erase.Exclude (rect);
}

void awsManager::InvalidateUpdateStore ()
{
  updatestore_dirty = true;
}

bool awsManager::WindowIsDirty (iAwsWindow *win)
{
  int i;

  for (i = 0; i < dirty.Count (); ++i)
    if (win->Overlaps (dirty.RectAt (i))) return true;

  return false;
}

void awsManager::UpdateStore ()
{
  if (updatestore_dirty)
  {
    iAwsWindow *curwin = top;

    updatestore.makeEmpty ();

    // Get all frames into the store.
    while (curwin)
    {
      if (!curwin->isHidden ())
      {
        csRect r (curwin->Frame ());
        updatestore.Include (r);
      }

      curwin = curwin->WindowBelow ();
    }

    updatestore_dirty = false;
  }
}

void awsManager::Print (iGraphics3D *g3d, uint8 Alpha)
{
  UpdateStore ();

  int i;
  csRect clip (0, 0, g3d->GetWidth () - 1, g3d->GetHeight () - 1);

  updatestore.ClipTo (clip);

  // Merge erase areas if we have to do both. Otherwise, just update normally.
  if (erase.Count () > 0)
  {
    for (i = 0; i < updatestore.Count (); ++i)
      erase.Include (updatestore.RectAt (i));

    for (i = 0; i < erase.Count (); ++i)
    {
      csRect r (erase.RectAt (i));
      canvas->Show (&r, g3d, Alpha);
    }

    erase.makeEmpty ();
  }
  else
  {
    for (i = 0; i < updatestore.Count (); ++i)
    {
      csRect r (updatestore.RectAt (i));
      canvas->Show (&r, g3d, Alpha);
    }
  }

  // Debug code

  /*iGraphics2D *g2d = g3d->GetDriver2D();
  for(i=0; i<updatestore.Count(); ++i)
  {
    csRect r(updatestore.RectAt(i));

    g2d->DrawLine(r.xmin, r.ymin, r.xmax, r.ymin, GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmin, r.ymin, r.xmin, r.ymax, GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmin, r.ymax, r.xmax, r.ymax, GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmax, r.ymin, r.xmax, r.ymax, GetPrefMgr()->GetColor(AC_WHITE));

  } */
}

void awsManager::Redraw ()
{
  static unsigned redraw_tag = 1;
  static csRect bounds (frame);
  int erasefill = GetPrefMgr ()->GetColor (AC_TRANSPARENT);
  int i;

  iAwsWindow *curwin = top, *oldwin = 0;

  redraw_tag++;

  csRect clip (frame);

  ptG3D->BeginDraw (CSDRAW_2DGRAPHICS);
  ptG2D->SetClipRect (0, 0, ptG2D->GetWidth () - 1, ptG2D->GetHeight () - 1);

  // Broadcast frame events.
  while (curwin)
  {
    if (!curwin->isHidden ())
    {
      csEvent Event;
      Event.Type = csevFrameStart;
      curwin->HandleEvent (Event);
    }

    curwin = curwin->WindowBelow ();
  }

  // check to see if there is anything to redraw.
  if (dirty.Count () == 0 && !(flags & AWSF_AlwaysRedrawWindows))
    return ;
  else
  {
    dirty.ClipTo (clip);
    erase.ClipTo (clip);
  }

  /******* The following code is only executed if there is something to redraw *************/
  curwin = top;

  // check to see if any part of this window needs redrawn, or if the always draw flag is set
  while (curwin)
  {
    if (
      (!curwin->isHidden ()) &&
      (WindowIsDirty (curwin) || (flags & AWSF_AlwaysRedrawWindows)))
    {
      curwin->SetRedrawTag (redraw_tag);
      if (flags & AWSF_AlwaysRedrawWindows) Mark (curwin->Frame ());
    }

    oldwin = curwin;
    curwin = curwin->WindowBelow ();
  }

  /*  At this point in time, oldwin points to the bottom most window.  That means that we take curwin, set it
   * equal to oldwin, and then follow the chain up to the top, redrawing on the way.  This makes sure that we
   * only redraw each window once.
   */
  curwin = oldwin;
  while (curwin)
  {
    if (DEBUG_MANAGER)
    {
      printf ("aws-debug: consider window: %p\n", curwin);
      printf (
        "aws-debug: redraw tag: %d/%d\n",
        curwin->RedrawTag (),
        redraw_tag);
    }

    if (redraw_tag == curwin->RedrawTag ())
    {
      if (DEBUG_MANAGER) printf ("aws-debug: window is dirty, redraw.\n");

      // Setup our dirty gathering rect.
      csRect cr;
      cr.MakeEmpty ();

      for (i = 0; i < dirty.Count (); ++i)
      {
        csRect dr (dirty.RectAt (i));

        if (DEBUG_MANAGER)
          printf ("aws-debug: consider rect:%d of %d\n", i, dirty.Count ());

        if (dr.Intersects (curwin->Frame ())) cr.Union (dr);
      }       // end gather all dirty rects that touch this window.

      // Get the intersection between the window and the clip rect.
      RedrawWindow (curwin, cr);
    }         // end if this window is dirty
    curwin = curwin->WindowAbove ();
  }           // end iterate all windows

  //int i;

  // Debug code: draw boxes around dirty regions

  /*for(i=0; i<dirty.Count(); ++i)
  {
         csRect dr(dirty.RectAt(i));
         ptG2D->DrawLine(dr.xmin, dr.ymin, dr.xmax, dr.ymin, GetPrefMgr()->GetColor(AC_WHITE));
         ptG2D->DrawLine(dr.xmin, dr.ymin, dr.xmin, dr.ymax, GetPrefMgr()->GetColor(AC_WHITE));
         ptG2D->DrawLine(dr.xmin, dr.ymax, dr.xmax, dr.ymax, GetPrefMgr()->GetColor(AC_WHITE));
         ptG2D->DrawLine(dr.xmax, dr.ymin, dr.xmax, dr.ymax, GetPrefMgr()->GetColor(AC_WHITE));
  }*/

  // This draws all of the erasure areas.
  if (flags & AWSF_AlwaysEraseWindows)
  {
    awsClipper clipper (ptG3D, ptG2D);
    clipper.SetClipRect (clip);

    for (i = 0; i < dirty.Count (); ++i) erase.Exclude (dirty.RectAt (i));

    for (i = 0; i < erase.Count (); ++i)
    {
      csRect r (erase.RectAt (i));
      clipper.DrawBox (r.xmin, r.ymin, r.Width (), r.Height (), erasefill);
    }
  }

  // Clear clipping bounds when done.
  ptG2D->SetClipRect (0, 0, ptG2D->GetWidth (), ptG2D->GetHeight ());

  // This only needs to happen when drawing to the default context.
  ptG3D->FinishDraw ();

  // Reset the dirty region
  dirty.makeEmpty ();

  // done with the redraw!
}

void awsManager::RedrawWindow (iAwsWindow *win, csRect &dirtyarea)
{
  if (DEBUG_MANAGER) printf ("aws-debug: start drawing window.\n");

  /// See if this window intersects with this dirty area
  if (!dirtyarea.Intersects (win->Frame ())) return ;

  /// Draw the window first.

  //csRect clip(win->Frame());
  //
  //  /// Clip the window to it's intersection with the dirty rectangle

  //clip.Intersect(csRect(0,0,G2D()->GetWidth(), G2D()->GetHeight()));

  //ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);
  //
  //  /// Tell the window to draw
  win->OnDraw (win->Frame ());

  /// Now draw all of it's children
  RecursiveDrawChildren (win, dirtyarea);

  if (DEBUG_MANAGER) printf ("aws-debug: finished drawing window.\n");
}

void awsManager::RecursiveDrawChildren (iAwsComponent *cmp, csRect &dirtyarea)
{
  int i;
  iAwsComponent *child;

  if (DEBUG_MANAGER) printf ("aws-debug: start drawing children.\n");

  for (i = 0; i < cmp->GetChildCount (); ++i)
  {
    child = cmp->GetChildAt (i);

    if (DEBUG_MANAGER)
      printf ("aws-debug: entered draw children loop for %p.\n", child);

    // Check to see if this component even needs redrawing.

    //if (!dirtyarea.Intersects(child->Frame()))

    //continue;
    csRect clip (child->Frame ());
    clip.Intersect (dirtyarea);

    //ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

    // Draw the child
    child->OnDraw (clip);

    // If it has children, draw them
    if (child->HasChildren ()) RecursiveDrawChildren (child, dirtyarea);
  }           // End for
  if (DEBUG_MANAGER) printf ("aws-debug: finished drawing children.\n");
}

iAwsParmList *awsManager::CreateParmList ()
{
  return new awsParmList;
}

iAwsWindow *awsManager::CreateWindowFrom (char *defname)
{
  if (DEBUG_MANAGER)
    printf ("aws-debug: Searching for window def \"%s\"\n", defname);

  // Find the window definition
  awsComponentNode *winnode = GetPrefMgr ()->FindWindowDef (defname);

  if (DEBUG_MANAGER)
    printf (
      "aws-debug: Window definition was %s\n",
      (winnode ? "found." : "not found."));

  // If we couldn't find it, abort
  if (winnode == NULL) return NULL;

  // Create a new window
  iAwsWindow *win = new awsWindow ();

  // Setup the name of the window
  win->SetID (winnode->Name ());

  // Tell the window to set itself up
  win->Setup (this, winnode);

  /* Now recurse through all of the child nodes, creating them and setting them
  up.  Nodes are created via their factory functions.  If a factory cannot be
  found, then that node and all of it's children are ignored. */
  CreateChildrenFromDef (this, win, win, winnode);

  // If window has layout, then layout children.
  if (win->Layout ())
  {
    csRect r = win->getInsets ();
    r.xmin += win->Frame ().xmin;
    r.ymin += win->Frame ().ymin;
    RecursiveLayoutChildren (win);
    win->MoveChildren (r.xmin, r.ymin);
  }

  return win;
}

void awsManager::CreateChildrenFromDef (
  iAws *wmgr,
  iAwsWindow *win,
  iAwsComponent *parent,
  awsComponentNode *settings)
{
  int i;
  for (i = 0; i < settings->GetLength (); ++i)
  {
    awsKey *key = settings->GetItemAt (i);

    if (key == NULL) continue;

    if (key->Type () == KEY_COMPONENT)
    {
      awsComponentNode *comp_node = (awsComponentNode *)key;
      awsComponentFactory *factory = FindComponentFactory (
          comp_node->ComponentTypeName ()->GetData ());

      // If we have a factory for this component, then create it and set it up.
      if (factory)
      {
        iAwsComponent *comp = factory->Create ();

        // Setup the name of the component
        comp->SetID (comp_node->Name ());

        // Setup window and parent of component
        comp->SetWindow (win);
        comp->SetParent (parent);

        // Prepare the component, and add it into it's parent
        comp->Setup (wmgr, comp_node);
        parent->AddChild (comp, (parent->Layout () != NULL));

        // Set it up in the parent's layout manager, if there is one.
        if (parent->Layout ())
          parent->Layout ()->AddComponent (
              wmgr->GetPrefMgr (),
              comp_node,
              comp);

        // Process all subcomponents of this component.
        CreateChildrenFromDef (wmgr, win, comp, comp_node);
      }
    }
    else if (key->Type () == KEY_CONNECTIONMAP)
    {
      int j;
      awsConnectionNode *conmap = (awsConnectionNode *)key;
      awsSlot *slot = new awsSlot ();

      for (j = 0; j < conmap->GetLength (); ++j)
      {
        awsConnectionKey *con = (awsConnectionKey *)conmap->GetItemAt (j);

        slot->Connect (parent, con->Signal (), con->Sink (), con->Trigger ());
      }       // end for count of connections

      //  Now that we've processed the connection map, we use a trick and send out

      // a creation signal for the component.  Note that we can't do this until the

      // connection map has been created, or the signal won't go anywhere!
      parent->Broadcast (0xefffffff);
    }         // end else
  }           // end for count of keys
}

void awsManager::RecursiveLayoutChildren (iAwsComponent *comp)
{
  if (comp->Layout ()) comp->Layout ()->LayoutComponents ();
  if (!comp->HasChildren ()) return ;

  int i;
  for (i = 0; i < comp->GetChildCount (); ++i)
  {
    iAwsComponent *child = comp->GetChildAt (i);

    RecursiveLayoutChildren (child);
  }
}

void awsManager::CaptureMouse (iAwsComponent *comp)
{
  mouse_captured = true;
  if (comp == NULL) comp = GetTopWindow ();

  mouse_focus = comp;
}

void awsManager::ReleaseMouse ()
{
  mouse_captured = false;
  mouse_focus = NULL;
}

bool awsManager::HandleEvent (iEvent &Event)
{
  // Find out what kind of event it is
  switch (Event.Type)
  {
    case csevMouseMove:
    case csevMouseUp:
    case csevMouseDown:
    case csevMouseClick:
      // Find out which top most window contains the pointer.
      if (GetTopWindow ())
      {
        // If the mouse is locked into the top window, keep it there
        if (mouse_captured && mouse_focus)
        {
          //        if (RecursiveBroadcastToChildren(mouse_focus, Event)) return true;

          //        else return mouse_focus->HandleEvent(Event);
          return mouse_focus->HandleEvent (Event);
        }     // end mouse captured
        else
        {
          // If the top window still contains the mouse, it stays on top
          if (
            !GetTopWindow ()->isHidden () &&
            GetTopWindow ()->Frame ().Contains (Event.Mouse.x, Event.Mouse.y))
          {
            if (RecursiveBroadcastToChildren (GetTopWindow (), Event))
              return true;
            else
            {
              PerformFocusChange (GetTopWindow (), Event);
              return GetTopWindow ()->HandleEvent (Event);
            }
          }   // end if topmost window contains it
          else
          {
            // Find the window that DOES contain the mouse.
            iAwsWindow *win = GetTopWindow ();

            // Skip the top 'cause we already checked it.
            if (win) win = win->WindowBelow ();

            while (win)
            {
              // If the window contains the mouse, it becomes new top.
              if (
                !win->isHidden () &&
                win->Frame ().Contains (Event.Mouse.x, Event.Mouse.y))
              {
                win->Raise ();
                if (RecursiveBroadcastToChildren (win, Event))
                  return true;
                else
                {
                  PerformFocusChange (win, Event);
                  return win->HandleEvent (Event);
                }
              }
              else
                win = win->WindowBelow ();
            } // end while iterating windows
          }   // end else check all other windows
        }     // end else mouse is not captured
      }       // end if there is a top window
      break;

    case csevKeyDown:
      // if (GetTopWindow())

      // {

      //   if (RecursiveBroadcastToChildren(GetTopWindow(), Event)) return true;

      //   else return GetTopWindow()->HandleEvent(Event);

      // }
      if (keyb_focus) keyb_focus->HandleEvent (Event);

      break;
  }

  return false;
}

bool awsManager::RecursiveBroadcastToChildren (
  iAwsComponent *cmp,
  iEvent &Event)
{
  int i;
  iAwsComponent *child;

  for (i = 0; i < cmp->GetChildCount (); ++i)
  {
    child = cmp->GetChildAt (i);

    // If it has children, broadcast to them (depth-first recursion)
    if (child->HasChildren ())
      if (RecursiveBroadcastToChildren (child, Event)) return true;
    if (CheckFocus (child, Event)) return true;
  }           // End for
  return false;
}

bool awsManager::CheckFocus (iAwsComponent *cmp, iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseMove:
    case csevMouseUp:
    case csevMouseDown:
    case csevMouseClick:
      // Only give to child if it contains the mouse.
      if (cmp->Frame ().Contains (Event.Mouse.x, Event.Mouse.y))
      {
        PerformFocusChange (cmp, Event);
        return cmp->HandleEvent (Event);
      }
      break;

    case csevKeyDown:
      if (cmp->HandleEvent (Event)) return true;

      break;
  }           // End switch
  return false;
}

void awsManager::PerformFocusChange (iAwsComponent *cmp, iEvent &Event)
{
  // Create a new event for MouseExit and MouseEnter
  uint8 et = Event.Type;

  if (mouse_in == cmp) return ;

  //printf("focus %x (%s)-> %x (%s)\n", mouse_in, (mouse_in ? mouse_in->Type() : "None"), cmp, cmp->Type());
  if (mouse_in)
  {
    Event.Type = csevMouseExit;
    mouse_in->HandleEvent (Event);
  }

  mouse_in = cmp;

  Event.Type = csevMouseEnter;
  mouse_in->HandleEvent (Event);

  Event.Type = et;

  if (et == csevMouseDown)
  {
    if (keyb_focus != cmp)
    {
      // Create a new event for Got/Lost Focus messages
      et = Event.Type;

      if (keyb_focus)
      {
        Event.Type = csevLostFocus;
        keyb_focus->HandleEvent (Event);
      }

      keyb_focus = cmp;
      Event.Type = csevGainFocus;
      keyb_focus->HandleEvent (Event);

      Event.Type = et;
    }
  }
}

void awsManager::RegisterCommonComponents ()
{
  // Components register themselves into the window manager.  Just creating a factory

  //  takes care of all the implementation details.  There's nothing else you need to do.
  (void)new awsCmdButtonFactory (this);
  (void)new awsLabelFactory (this);
  (void)new awsTextBoxFactory (this);
  (void)new awsRadButtonFactory (this);
  (void)new awsCheckBoxFactory (this);
  (void)new awsGroupFrameFactory (this);
  (void)new awsListBoxFactory (this);
  (void)new awsScrollBarFactory (this);

  // Standard sink
  GetSinkMgr ()->RegisterSink ("awsStandardSink", new awsStandardSink ());

  // Global constants
  GetPrefMgr ()->RegisterConstant ("True", 1);
  GetPrefMgr ()->RegisterConstant ("False", 0);
  GetPrefMgr ()->RegisterConstant ("Yes", 1);
  GetPrefMgr ()->RegisterConstant ("No", 0);

  GetPrefMgr ()->RegisterConstant ("signalComponentCreated", 0xefffffff);

  // Layout constants
  GetPrefMgr ()->RegisterConstant ("gbcRelative", -1);
  GetPrefMgr ()->RegisterConstant ("gbcRemainder", 0);
  GetPrefMgr ()->RegisterConstant ("gbcNone", 1);
  GetPrefMgr ()->RegisterConstant ("gbcBoth", 2);
  GetPrefMgr ()->RegisterConstant ("gbcHorizontal", 3);
  GetPrefMgr ()->RegisterConstant ("gbcVertical", 4);
  GetPrefMgr ()->RegisterConstant ("gbcCenter", 10);
  GetPrefMgr ()->RegisterConstant ("gbcNorth", 11);
  GetPrefMgr ()->RegisterConstant ("gbcNorthEast", 12);
  GetPrefMgr ()->RegisterConstant ("gbcEast", 13);
  GetPrefMgr ()->RegisterConstant ("gbcSouthEast", 14);
  GetPrefMgr ()->RegisterConstant ("gbcSouth", 15);
  GetPrefMgr ()->RegisterConstant ("gbcSouthWest", 16);
  GetPrefMgr ()->RegisterConstant ("gbcWest", 17);
  GetPrefMgr ()->RegisterConstant ("gbcNorthWest", 18);

  GetPregMgr ()->RegisterConstant ("blCenter", 0);
  GetPregMgr ()->RegisterConstant ("blNorth", 1);
  GetPregMgr ()->RegisterConstant ("blEast", 2);
  GetPregMgr ()->RegisterConstant ("blSouth", 3);
  GetPregMgr ()->RegisterConstant ("blWest", 4);

}

bool awsManager::AllWindowsHidden ()
{
  iAwsWindow *curwin = top, *oldwin = 0;

  while (curwin)
  {
    if (!curwin->isHidden ()) return false;

    curwin = curwin->WindowBelow ();
  }

  return true;
}

iGraphics2D *awsManager::G2D ()
{
  return ptG2D;
}

iGraphics3D *awsManager::G3D ()
{
  return ptG3D;
}

iObjectRegistry *awsManager::GetObjectRegistry ()
{
  return object_reg;
}

void awsManager::SetFlag (unsigned int _flags)
{
  flags |= _flags;
}

void awsManager::ClearFlag (unsigned int _flags)
{
  flags &= (~_flags);
}

unsigned int awsManager::GetFlags ()
{
  return flags;
}
