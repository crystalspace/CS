/*
    Copyright (C) 2001 by Christopher Nelson

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

/******************************************************************************
 * NOTES
 *
 * The redraw process still does not properly support clipping.  This should be
 * revisted when we figure out why the clipping mechanisms in the sofware and
 * opengl don't work like one would expect.
 *
 * Clipping works properly now at least in software mode -- Noah There are also
 * now 3 new tests in the g2dtest app which can help test clipping on other
 * canvases.
 *****************************************************************************/

#include "cssysdef.h"
#include "csutil/csevent.h"
#include "iengine/engine.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivideo/txtmgr.h"

#include "aws.h"
#include "awsfparm.h"
#include "awsprefs.h"

// includes for registration/embedding
#include "aws3dfrm.h"
#include "awsControlBar.h"
#include "awsEngineView.h"
#include "awsMenu.h"
#include "awsbarct.h"
#include "awschkbx.h"
#include "awscmdbt.h"
#include "awscomp.h"
#include "awscscr.h"
#include "awsgrpfr.h"
#include "awsimgvw.h"
#include "awslabel.h"
#include "awslayot.h"
#include "awslstbx.h"
#include "awsmled.h"
#include "awsntbk.h"
#include "awsradbt.h"
#include "awsscrbr.h"
#include "awsstbar.h"
#include "awsstdsk.h"
#include "awstimer.h"
#include "awstxtbx.h"
#include "awswin.h"

#include <stdio.h>

#undef DEBUG_MANAGER

awsManager::awsManager (iBase *p) :
  updatestore_dirty(true),
  top(0),
  mouse_in(0),
  keyb_focus(0),
  mouse_focus(0),
  focused(0),
  modal_dialog(0),
  mouse_captured(false),
  ptG2D(0),
  ptG3D(0),
  object_reg(0),
  flags(0)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = 0;
}

awsManager::~awsManager ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);

    scfiEventHandler->DecRef ();
  }
  component_factories.DeleteAll ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool awsManager::Initialize (iObjectRegistry *object_reg)
{
  awsManager::object_reg = object_reg;

  prefmgr = SCF_CREATE_INSTANCE("crystalspace.window.preferencemanager",
				iAwsPrefManager);
  if (!prefmgr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
      "AWS could not create an instance of the default PREFERENCE manager. "
      "This is a serious error.");
    return false;
  }
  prefmgr->SetWindowMgr (this);
  if (!prefmgr->Setup (object_reg))
    return false;

  sinkmgr = SCF_CREATE_INSTANCE ("crystalspace.window.sinkmanager",
			        iAwsSinkManager);
  if (!sinkmgr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
      "AWS could not create an instance of the default SINK manager. "
      "This is a serious error.");
    return false;
  }
  if (!sinkmgr->Setup (object_reg))
    return false;

  strset = CS_QUERY_REGISTRY_TAG_INTERFACE(object_reg,
    "crystalspace.shared.stringset", iStringSet);
  if (!strset.IsValid())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
      "AWS could not locate the global shared string set \""
      "crystalspace.shared.stringset\". This is a serious error.");
    return false;
  }

  RegisterCommonComponents ();

  // Now set by default.
  SetFlag(AWSF_AlwaysRedrawWindows);

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
  prefmgr = pmgr;
}

iStringSet* awsManager::GetStringTable ()
{
  return strset;
}

iAwsComponent *awsManager::CreateEmbeddableComponent (
  iAwsComponent *forComponent)
{
  return new awsComponent (forComponent);
}

void awsManager::RegisterComponentFactory (iAwsComponentFactory *factory,
					   const char *name)
{
  awsComponentFactoryMap cfm;
  cfm.factory = factory;
  cfm.id = prefmgr->NameToId (name);

  component_factories.Push (cfm);
}

iAwsComponentFactory *awsManager::FindComponentFactory (const char *name)
{
  unsigned long id = prefmgr->NameToId (name);

  for (size_t i = 0; i < component_factories.Length(); i++)
  {
    if (component_factories[i].id == id)
      return component_factories[i].factory;
  }
  
  return 0;
}

iAwsComponent *awsManager::CreateEmbeddableComponentFrom(const char *name)
{
  iAwsComponentFactory *factory = FindComponentFactory(name);
  if (!factory)
    return 0;
  return factory->Create ();
}

iAwsComponent *awsManager::GetTopComponent ()
{
  return top;
}

void awsManager::SetTopComponent (iAwsComponent *_top)
{
  top = _top;
}

iAwsComponent *awsManager::GetFocusedComponent ()
{
  return focused;
}

void awsManager::SetFocusedComponent (iAwsComponent *_focused)
{
  if (focused == _focused)
    return;

  if (focused)
    focused->UnsetFocus();
  if (_focused)
    _focused->SetFocus();
  /*keyb_focus = */focused = _focused;
}

iAwsComponent *awsManager::GetKeyboardFocusedComponent()
{
  return keyb_focus;
}

bool awsManager::SetupCanvas (iAwsCanvas *_newCanvas,
			      iGraphics2D *g2d,
			      iGraphics3D *g3d)
{
  iAwsCanvas *newCanvas = _newCanvas;

  if (newCanvas == 0)
  {
	if (g2d == 0 || g3d == 0)
		return false;

    newCanvas = new awsScreenCanvas (g2d, g3d);
  }
  
  canvas = csPtr<iAwsCanvas>(newCanvas);

  ptG2D = canvas->G2D ();
  ptG3D = canvas->G3D ();

  ptG2D->DoubleBuffer (false);

  prefmgr->SetTextureManager (ptG3D->GetTextureManager ());
  prefmgr->SetFontServer (ptG2D->GetFontServer ());

  frame.Set (0, 0, ptG2D->GetWidth (), ptG2D->GetHeight ());

  Mark (frame);

  return true;
}

iAwsCanvas *awsManager::GetCanvas ()
{
  return canvas;
}

void awsManager::CreateTransition(iAwsComponent *win,
				  unsigned transition_type,
				  csTicks duration)
{
  if (win==0) return;

  awsWindowTransition *t = new awsWindowTransition;
  int w = G2D()->GetWidth();
  int h = G2D()->GetHeight();
  
  t->start_time = 0;
  t->morph_duration=duration;
  t->transition_type=transition_type;
  t->win=win;
  
  switch(transition_type)
  {
  case AWS_TRANSITION_SLIDE_IN_LEFT:
    t->end=win->Frame();
    t->start=csRect(w+1, t->end.ymin, w+1+t->end.Width(), t->end.ymax);
    break;
    
  case AWS_TRANSITION_SLIDE_IN_RIGHT:
    t->end=win->Frame();
    t->start=csRect(0-t->end.Width()-1, t->end.ymin, -1, t->end.ymax);
    break;

  case AWS_TRANSITION_SLIDE_IN_UP:
    t->end=win->Frame();
    t->start=csRect(t->end.xmin, h+1, t->end.xmax, h+1+t->end.Height());
    break;

  case AWS_TRANSITION_SLIDE_IN_DOWN:
    t->end=win->Frame();
    t->start=csRect(t->end.xmin, 0-t->end.Height()-1, t->end.xmax, -1);
    break;

  case AWS_TRANSITION_SLIDE_OUT_LEFT:
    t->start=win->Frame();
    t->end=csRect(w+1, t->start.ymin, w+1+t->start.Width(), t->start.ymax);
    break;
    
  case AWS_TRANSITION_SLIDE_OUT_RIGHT:
    t->start=win->Frame();
    t->end=csRect(0-t->start.Width()-1, t->start.ymin, -1, t->start.ymax);
    break;

  case AWS_TRANSITION_SLIDE_OUT_UP:
    t->start=win->Frame();
    t->end=csRect(t->start.xmin, h+1, t->start.xmax, h+1+t->start.Height());
    break;

  case AWS_TRANSITION_SLIDE_OUT_DOWN:
    t->start=win->Frame();
    t->end=csRect(t->start.xmin, 0-t->start.Height()-1, t->start.xmax, -1);
    break;

  default:
    delete t;
    return;
    break;
  }

  transitions.Push(t);
}

void awsManager::CreateTransitionEx(iAwsComponent *win,
				    unsigned transition_type,
				    csTicks duration, csRect &user)
{
  if (win==0) return;

  awsWindowTransition *t = new awsWindowTransition;
  
  t->start_time = 0;
  t->morph_duration=duration;
  t->transition_type=transition_type;
  t->win=win;
  
  switch(transition_type)
  {
  case AWS_TRANSITION_SLIDE_IN_LEFT:
  case AWS_TRANSITION_SLIDE_IN_RIGHT:
  case AWS_TRANSITION_SLIDE_IN_UP:
  case AWS_TRANSITION_SLIDE_IN_DOWN:
    t->end=win->Frame();
    t->start=user;
    break;

  case AWS_TRANSITION_SLIDE_LEFT:    
  case AWS_TRANSITION_SLIDE_RIGHT:  
  case AWS_TRANSITION_SLIDE_DOWN:
  case AWS_TRANSITION_SLIDE_UP:
  case AWS_TRANSITION_SLIDE_OUT_LEFT:    
  case AWS_TRANSITION_SLIDE_OUT_RIGHT:  
  case AWS_TRANSITION_SLIDE_OUT_DOWN:
  case AWS_TRANSITION_SLIDE_OUT_UP:
    t->start=win->Frame();
    t->end=user;
    break;

  default:
    delete t;
    return;
    break;
  }

  transitions.Push(t);
}

awsWindowTransition* awsManager::FindTransition(iAwsComponent *win)
{
  awsWindowTransition *t;
  
  for(size_t i = 0; i < transitions.Length(); ++i)
  {
    t = (awsWindowTransition *)transitions[i];

    if (t->win==win)
    {
      return t;
    }
  }
  
  // if we get here, we didn't find a transition for the window
  return 0;
}

bool awsManager::PerformTransition(iAwsComponent *win)
{
  awsWindowTransition *t = FindTransition(win);

  float dx, dy;
  csRect interp(t->start);
  csTicks current_time = csGetTicks();

  // check if this is the start of transitioning
  if (t->start_time == 0)
  {
    t->win->Move(t->start.xmin - t->win->Frame().xmin,
	    t->start.ymin - t->win->Frame().ymin);
	    
	  t->start_time = current_time;
  }
  // else calculate the new position and move the window
  else
  {
    dx = t->end.xmin - t->start.xmin;
    dy = t->end.ymin - t->start.ymin;

    // calculate the morph amount as a percentage
    csTicks elapsed_time = current_time - t->start_time;
    float morph_amount = (float)elapsed_time / t->morph_duration;
    if (morph_amount > 1.0)
      morph_amount = 1.0;

    // calculate the morph amount in pixels
    dx *= morph_amount;
    dy *= morph_amount;
  
    interp.Move((int)dx, (int)dy);
    t->win->Move(interp.xmin - t->win->Frame().xmin,
  	       interp.ymin - t->win->Frame().ymin);
  
    t->win->Invalidate();
  }

  // check if we are finished with the transition
  if (current_time - t->start_time >= t->morph_duration)
  {
    switch(t->transition_type)
    {
    case AWS_TRANSITION_SLIDE_IN_LEFT:
    case AWS_TRANSITION_SLIDE_IN_RIGHT:
    case AWS_TRANSITION_SLIDE_IN_UP:
    case AWS_TRANSITION_SLIDE_IN_DOWN:
    default:
      // do nothing for these transition types
      break;

    case AWS_TRANSITION_SLIDE_OUT_LEFT:    
    case AWS_TRANSITION_SLIDE_OUT_RIGHT:  
    case AWS_TRANSITION_SLIDE_OUT_UP:
    case AWS_TRANSITION_SLIDE_OUT_DOWN:
      // Hide window (an out transition means out of view)
      t->win->Hide();

      // Fix frame back to start
      t->win->Move(t->start.xmin-t->win->Frame().xmin,
        t->start.ymin-t->win->Frame().ymin);
      break;
    }

    transitions.Delete(t);
    delete t;

    return false;
  }

  return true;
}

iAwsComponent* awsManager::ComponentAt(int x, int y)
{
  for(iAwsComponent* cur = GetTopComponent(); cur; cur = cur->ComponentBelow())
  {
    if (cur->isHidden())
      continue;
    
    // find the top level component which contains the point
    iAwsComponent* child = cur->ChildAt(x,y);
    if (child)
    {
      // then iterate down the tree until a child no longer contains
      // the point
      iAwsComponent* temp;
      while ((temp = child->ChildAt(x,y)) != 0)
	child = temp;
      
      return child;
    }
    if (cur->Frame().Contains(x,y))
      return cur;
  }
  
  return 0;
}

bool awsManager::MouseInComponent(int x, int y)
{
  for(iAwsComponent* cur = GetTopComponent(); cur; cur = cur->ComponentBelow())
  {
    if (cur->isHidden())
      continue;
    if (cur->Frame().Contains(x,y))
      return true;
  }
  return false;
}

void awsManager::Mark (const csRect &rect)
{
  dirty.Include (rect);
}

void awsManager::Unmark (const csRect &rect)
{
  dirty.Exclude (rect);
}

void awsManager::Erase (const csRect &rect)
{
  erase.Include (rect);
}

void awsManager::MaskEraser (const csRect &rect)
{
  erase.Exclude (rect);
}

void awsManager::InvalidateUpdateStore ()
{
  updatestore_dirty = true;
}

bool awsManager::ComponentIsDirty (iAwsComponent *win)
{
  if (!win->isHidden ())
    for (int i = 0; i < dirty.Count (); ++i)
      if (win->Overlaps (dirty.RectAt (i)))
	return true;
  return false;
}

bool awsManager::ComponentIsInTransition(iAwsComponent *win)
{
  if (win->isHidden ())
    return false;

  if (FindTransition(win))
    return true;
  else
    return false;
}

void awsManager::UpdateStore ()
{
  if (updatestore_dirty)
  {
    iAwsComponent *cur = top;

    updatestore.makeEmpty ();

    // Get all frames into the store.
    while (cur)
    {
      if (!cur->isHidden ())
      {
        csRect r (cur->Frame ());
        updatestore.Include (r);
      }

      cur = cur->ComponentBelow ();
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

#if 0
  // Debug code
  iGraphics2D *g2d = g3d->GetDriver2D();
  for(i=0; i<updatestore.Count(); ++i)
  {
    csRect r(updatestore.RectAt(i));
    g2d->DrawLine(r.xmin, r.ymin, r.xmax, r.ymin,
		  GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmin, r.ymin, r.xmin, r.ymax,
		  GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmin, r.ymax, r.xmax, r.ymax,
		  GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmax, r.ymin, r.xmax, r.ymax,
		  GetPrefMgr()->GetColor(AC_WHITE));

  }
#endif
}

void awsManager::Redraw ()
{
  static unsigned redraw_tag = 1;

  int erasefill = GetPrefMgr ()->GetColor (AC_TRANSPARENT);
  int i;
  iAwsComponent *curwin = top, *oldwin = 0;

  redraw_tag++;

  csRect clip (frame);

  CS_ASSERT(frame.xmax <= ptG2D->GetWidth() &&
	    frame.ymax <= ptG2D->GetHeight());

  ptG3D->BeginDraw (CSDRAW_2DGRAPHICS);
  ptG2D->SetClipRect (frame.xmin, frame.ymin,
                      frame.xmax, frame.ymax);

  // Broadcast frame events.
  while (curwin)
  {
    if (!curwin->isHidden ())
    {
      csEvent Event;
      Event.Type = csevFrameStart;
      curwin->HandleEvent (Event);
    }
    curwin = curwin->ComponentBelow ();
  }

  // check to see if there is anything to redraw.
  if (transitions.Length() == 0 && dirty.Count () == 0 &&
      !(flags & AWSF_AlwaysRedrawWindows))
    return;

  // The following code is only executed if there is something to redraw.
  curwin = top;

  // check to see if any part of this window needs redrawn, or if the always
  // draw flag is set
  while (curwin)
  {
    bool transition_performed = false;
    if (ComponentIsInTransition(curwin))
    {
      transition_performed = PerformTransition(curwin);
    }
    if (
        (   transition_performed               /* MUST COME BEFORE OTHERS! */
         || ComponentIsDirty (curwin) 
         || (flags & AWSF_AlwaysRedrawWindows) 
        )
        && (!curwin->isHidden ())              /* MUST COME LAST! */
       )
    {
      curwin->SetRedrawTag (redraw_tag);
      if (flags & AWSF_AlwaysRedrawWindows) curwin->Invalidate();
    }

    oldwin = curwin;
    curwin = curwin->ComponentBelow ();
  }
  
  dirty.ClipTo (clip);
  erase.ClipTo (clip);

  // At this point in time, oldwin points to the bottom most window.  That
  // means that we take curwin, set it equal to oldwin, and then follow the
  // chain up to the top, redrawing on the way.  This makes sure that we only
  // redraw each window once.
  curwin = oldwin;
  while (curwin)
  {
#ifdef DEBUG_MANAGER
    printf ("aws-debug: consider window: %p\n", curwin);
    printf (
	"aws-debug: redraw tag: %d/%d\n",
	curwin->RedrawTag (),
	redraw_tag);
#endif
    
    if (redraw_tag == curwin->RedrawTag ())
    {
#ifdef DEBUG_MANAGER
      printf ("aws-debug: window is dirty, redraw.\n");
#endif
      // Setup our dirty gathering rect.
      csRect cr;
      cr.MakeEmpty ();
      
      for (i = 0; i < dirty.Count (); ++i)
      {
	csRect dr (dirty.RectAt (i));
	dr.Intersect(curwin->Frame());
	cr.Union (dr);
      }
      
      // cr is now the smallest possible rect that
      // contains all the dirty area over the window
      RedrawWindow (curwin, cr);
    }         // end if this window is dirty
    curwin = curwin->ComponentAbove ();
  }           // end iterate all windows
  
  // Debug code: draw boxes around dirty regions
#if 0
  for(i=0; i<dirty.Count(); ++i)
  {
    csRect dr(dirty.RectAt(i));
    ptG2D->DrawLine(dr.xmin, dr.ymin, dr.xmax, dr.ymin,
		    GetPrefMgr()->GetColor(AC_WHITE));
    ptG2D->DrawLine(dr.xmin, dr.ymin, dr.xmin, dr.ymax,
		    GetPrefMgr()->GetColor(AC_WHITE));
    ptG2D->DrawLine(dr.xmin, dr.ymax, dr.xmax, dr.ymax,
		    GetPrefMgr()->GetColor(AC_WHITE));
    ptG2D->DrawLine(dr.xmax, dr.ymin, dr.xmax, dr.ymax,
		    GetPrefMgr()->GetColor(AC_WHITE));
     }
#endif
  
  // Clear clipping bounds when done.
  ptG2D->SetClipRect (0, 0, ptG2D->GetWidth (), ptG2D->GetHeight ());
  
  // This draws all of the erasure areas.
  if (flags & AWSF_AlwaysEraseWindows)
  {
    UpdateStore();
    for(i = 0; i < updatestore.Count(); i++)
      MaskEraser(updatestore.RectAt(i));
    
    for (i = 0; i < erase.Count (); ++i)
    {
      csRect r (erase.RectAt (i));
      G2D()->DrawBox (r.xmin, r.ymin, r.Width (), r.Height (), erasefill);
    }
  }
  
  // This only needs to happen when drawing to the default context.
  // XXX: Matze: Applications do this, so is shouldn't be needed here
  //ptG3D->FinishDraw ();

  // Reset the dirty region
  dirty.makeEmpty ();
  // done with the redraw!
}

void awsManager::RedrawWindow (iAwsComponent *win, csRect clip)
{
  // precondition: clip is contained inside the the window frame and 
  // also inside the canvas drawing area

#ifdef DEBUG_MANAGER
  printf ("aws-debug: start drawing window.\n");
#endif

  ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

  // tell window frame to draw
  win->OnDraw (clip); 

  /// Draw the children
  RecursiveDrawChildren (win, clip);

#ifdef DEBUG_MANAGER
  printf ("aws-debug: finished drawing window.\n");
#endif
}

void awsManager::RecursiveDrawChildren (iAwsComponent *cmp, csRect clip)
{
  iAwsComponent *child;

#ifdef DEBUG_MANAGER
  printf ("aws-debug: start drawing children.\n");
#endif

  if (!cmp->HasChildren()) return;

  for(child = cmp->GetTopChild();
      child->ComponentBelow();
      child = child->ComponentBelow()) {}

  for ( ; child; child = child->ComponentAbove())
  {
    // Do not draw the child if it's hidden or invisible.
    if (child->isHidden() || child->Flags() & AWSF_CMP_INVISIBLE)
	continue;

#ifdef DEBUG_MANAGER
    printf ("aws-debug: entered draw children loop for %p.\n", child);
#endif

    csRect child_clip(child->Frame());
    child_clip.Intersect (clip);

    if (! (child->Flags() & AWSF_CMP_NON_CLIENT))
      child_clip.Intersect (cmp->ClientFrame());

    if (child_clip.IsEmpty()) // there is nothing to draw
      continue;

    // enforce the clipping
    ptG2D->SetClipRect(child_clip.xmin, child_clip.ymin, 
                       child_clip.xmax, child_clip.ymax);

    // Draw the child
    child->OnDraw (child_clip);
    RecursiveDrawChildren (child, child_clip);
  } // End for
#ifdef DEBUG_MANAGER
  printf ("aws-debug: finished drawing children.\n");
#endif
}

iAwsParmList *awsManager::CreateParmList ()
{
  return new awsParmList(this);
}

iAwsComponent *awsManager::CreateWindowFrom (const char* defname)
{
  // Find the window definition
  iAwsComponentNode *cmpnode = GetPrefMgr ()->FindWindowDef (defname);
  if (cmpnode == 0)
    return 0;

  // Create a new component
  iAwsComponentFactory *factory =
      FindComponentFactory (cmpnode->ComponentTypeName ()->GetData ());
  if (!factory)
    return 0;
  
  iAwsComponent *comp = factory->Create ();

  // Setup the component
  if (!comp->Create(this, 0, cmpnode))
    return 0;

  // Now recurse through all of the child nodes, creating them and setting them
  // up.  Nodes are created via their factory functions.  If a factory cannot
  // be found, then that node and all of it's children are ignored.
  CreateChildrenFromDef (this, comp, cmpnode);

  return comp;
}

void awsManager::CreateChildrenFromDef (
  iAws *wmgr,
  iAwsComponent *parent,
  iAwsComponentNode *settings)
{
  for (int i = 0; i < settings->Length (); ++i)
  {
    iAwsKey *key = settings->GetAt (i);

    if (key == 0)
      continue;

    if (key->Type () == KEY_COMPONENT)
    {
      csRef<iAwsComponentNode> comp_node (
      	SCF_QUERY_INTERFACE(key, iAwsComponentNode));
      CS_ASSERT(comp_node);
      iAwsComponentFactory *factory = FindComponentFactory (
          comp_node->ComponentTypeName ()->GetData ());

      // If we have a factory for this component, then create it and set it up.
      if (factory)
      {
        iAwsComponent *comp = factory->Create ();
        // sets up the component for use returns true if all went well
        if (comp->Create(wmgr, parent, comp_node))
	{
          // Process all subcomponents of this component.
          CreateChildrenFromDef (wmgr, comp, comp_node);
	}
	comp->DecRef();
      }
    }
    else if (key->Type () == KEY_CONNECTIONMAP)
    {
      int j;
      csRef<iAwsKeyContainer> conmap (
      	SCF_QUERY_INTERFACE(key, iAwsKeyContainer));
      CS_ASSERT(conmap);
      awsSlot *slot = new awsSlot ();

      for (j = 0; j < conmap->Length (); ++j)
      {
        csRef<iAwsConnectionKey> con (
		SCF_QUERY_INTERFACE(conmap->GetAt (j), iAwsConnectionKey));
        CS_ASSERT(con);

        slot->Connect (parent, con->Signal (), con->Sink (), con->Trigger ());
      } // end for count of connections

      // Now that we've processed the connection map, we use a trick and send
      // out a creation signal for the component.  Note that we can't do this
      // until the connection map has been created, or the signal won't go
      // anywhere!
      parent->Broadcast (0xefffffff);
    }         // end else
  }           // end for count of keys

  parent->LayoutChildren();
}

void awsManager::CaptureMouse (iAwsComponent *comp)
{
#ifdef DEBUG_MANAGER
  printf("aws-debug: Mouse captured\n");
#endif
  mouse_captured = true;
  if (comp == 0)
    comp = GetTopComponent ();
  mouse_focus = comp;
}

void awsManager::ReleaseMouse ()
{
#ifdef DEBUG_MANAGER
  printf("aws-debug: Mouse released\n");
#endif
  mouse_captured = false;
  mouse_focus = 0;
}

void awsManager::SetModal (iAwsComponent *comp)
{
#ifdef DEBUG_MANAGER
  printf("aws-debug: Modal Set: %p\n", comp);
#endif
  // return out if the new modal window is null or there is already a
  // modal_dialog
  if (comp == 0 || modal_dialog)
	return;
  modal_dialog = comp;
}

void awsManager::UnSetModal()
{
#ifdef DEBUG_MANAGER
  printf("aws-debug: Modal Unset: %p\n", modal_dialog);
#endif
  modal_dialog = 0;
}

bool awsManager::HandleEvent (iEvent &Event)
{  
  // Find out what kind of event it is
  switch (Event.Type)
  {
  case csevMouseMove:
  case csevMouseUp:
  case csevMouseClick:
  case csevMouseDown:
    {
      //  If there is a modal_dialog check to see if we are
      //  it or a child of it.  If not return out.
      if (modal_dialog)
      {
        iAwsComponent *comp = ComponentAt(Event.Mouse.x, Event.Mouse.y);

        while (comp)
        {
          if (comp == modal_dialog)
            break;

          comp = comp->Parent();
        }

        if (comp == 0)
          return true;
      }

      // If the mouse is locked keep it there
      if (mouse_captured && mouse_focus)
        if (mouse_focus->HandleEvent (Event))
          return true;

      // Find out which component contains the pointer.
      iAwsComponent* comp = ComponentAt(Event.Mouse.x, Event.Mouse.y);

      // if the mouse is still captured just stop
      if (mouse_captured && mouse_focus)
        return false;

      // check to see if focus needs updating
      // if that succeeds then the keyboard might need focusing too
      if (comp && ChangeMouseFocus(comp, Event))
        ChangeKeyboardFocus(comp, Event);

      // its possible that some component captured the mouse
      // in response to losing mouse focus. If that occured then we
      // give that component a chance to handle the event
      // rather than the component curently containing the mouse
      if (mouse_captured && mouse_focus)
        return mouse_focus->HandleEvent(Event);

      // move up the chain of components to find the first one that can handle
      // the event. 
      while (comp && !(comp->Flags() & AWSF_CMP_DEAF) &&
	    !comp->HandleEvent(Event))
        comp = comp->Parent();

      // if we haven't reached the top then some component handled it
      if (comp)
        return true;
    }
    break;

  case csevKeyboard:
    if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
    {
      iAwsComponent *cmp = 0;

      if (flags & AWSF_KeyboardControl)	
      {
        //  If there is a modal_dialog check to see if we are
        //  it or a child of it.  If not return out.
        if (modal_dialog)
        {
          iAwsComponent *comp = GetFocusedComponent();

          while (comp)
          {
            if (comp == modal_dialog)
              break;

            comp = comp->Parent();
          }

          if (comp == 0)
            return true;
        }

        cmp = GetFocusedComponent();

        csKeyModifiers m;
        csKeyEventHelper::GetModifiers (&Event, m);
        utf32_char code = csKeyEventHelper::GetCookedCode (&Event);

        if (code == CSKEY_TAB)
        {
          bool found = false;
          while (cmp && !found) 
          {
            if (m.modifiers[csKeyModifierTypeCtrl] != 0)
            {
              if (cmp->Parent() && 
                cmp == cmp->Parent()->GetTabComponent(0) && 
                cmp->Parent()->Parent())
              {
                cmp = cmp->Parent();
              }

              cmp = cmp->Parent()->TabPrev(cmp);
            }
            else
            {
              if (cmp->Parent() 
                && cmp == cmp->Parent()->GetTabComponent(
                cmp->Parent()->GetTabLength() - 1) && 
                cmp->Parent()->Parent())
              {
                cmp = cmp->Parent();
              }

              cmp = cmp->Parent()->TabNext(cmp);
            }

            if (cmp && cmp->Focusable() && !cmp->isHidden())
              found = true;
            else 
            {
              if (cmp && cmp->HasChildren())
              {
                if (m.modifiers[csKeyModifierTypeCtrl] != 0)
                  cmp = cmp->GetTabComponent(cmp->GetTabLength() - 1);
                else cmp = cmp->GetTabComponent(0);
                if (cmp && cmp->Focusable() && !cmp->isHidden())
                  found = true;
              }
            }
          }

          if (cmp)
            SetFocusedComponent(cmp);
          
          return true;
        }
        if (cmp) ChangeKeyboardFocus(cmp, Event);
      }

      //  If there is a modal_dialog check to see if we are
      //  it or a child of it.  If not return out.
      if (modal_dialog)
      {
        iAwsComponent *comp = keyb_focus;

        while (comp)
        {
          if (comp == modal_dialog)
            break;

          comp = comp->Parent();
        }

        if (comp == 0)
          return true;
      }
      
      //Cycle till the parent of the window the component belongs to.
      cmp = keyb_focus;
      while (cmp)
        if (cmp->Parent ())
          cmp = cmp->Parent ();
        else break;

      //If the component is not visible, do not pass it the keyboard event!
      if (keyb_focus && !cmp->isHidden ())
        return keyb_focus->HandleEvent (Event);
        
    }
    break;

  case csevBroadcast:
    if (Event.Command.Code == cscmdPreProcess)
    {
      DispatchEventRecursively(GetTopComponent(), Event);
    }
    break;
  }
  
  return false;
}

void awsManager::DispatchEventRecursively(iAwsComponent *c, iEvent &ev)
{
  while (c != 0)
  {
    if (!c->isHidden())
    {
      c->HandleEvent(ev);
      DispatchEventRecursively(c->GetTopChild(), ev);
    }
    c = c->ComponentBelow();
  }
}

iAwsComponent* awsManager::FindCommonParent(iAwsComponent* cmp1,
					    iAwsComponent* cmp2)
{
  iAwsComponent* testParent1 = cmp1;
  iAwsComponent* testParent2 = cmp2;

  while (testParent1)
  {
    while (testParent2)
    {
      if (testParent1 == testParent2)
        return testParent1;
      testParent2 = testParent2->Parent();
    }
    testParent2 = cmp2;
    testParent1 = testParent1->Parent();
  }

  return 0;
}

// Note: If this is too slow common_parent could be calculated once and then
// passed and updated as necessary. Unless component trees get really deep
// though I don't think it matters so I left it this way because its slightly
// clearer.
bool awsManager::ChangeMouseFocus(iAwsComponent *cmp, iEvent &Event)
{
  iAwsComponent* common_parent = FindCommonParent(mouse_in, cmp);
  
  if (mouse_in == cmp)
    return ChangeMouseFocusHelper(cmp, Event);
  else if (common_parent == mouse_in)
  {
    if (ChangeMouseFocus(cmp->Parent(), Event))  // get focus to the parent
      return ChangeMouseFocusHelper(cmp, Event); // then get it to cmp
  }
  else
  {
    // get focus to mouse_in's parent
    if (ChangeMouseFocusHelper(mouse_in->Parent(),Event))
      return ChangeMouseFocus(cmp, Event); // get it the rest of the way
    else
      return false;
  }
  // can't get here but compiler seems to complain if I don't have it
  return false;
}

bool awsManager::ChangeMouseFocusHelper(iAwsComponent *cmp, iEvent &Event)
{
  // Reusing this event, save the orignal type
  uint8 et = Event.Type;
  if (mouse_in != cmp)
  {
    if (mouse_in)
    {
      Event.Type = csevMouseExit;
      mouse_in->HandleEvent (Event);
    }
    
    // A component might respond to a mouseExit event by 
    // capturing the mouse. If so we leave mouse_in with its
    // current ref. Then when the component releases the mouse
    // it will again receive the mouseExit as if the mouse never
    // left.
    if (mouse_captured && mouse_focus)
    {
      Event.Type = et;
      return false;
    }
    
    mouse_in = cmp;
    
    if (mouse_in)
    {
      Event.Type = csevMouseEnter;
      mouse_in->HandleEvent (Event);
    }
    
    Event.Type = et;
  }
  
  // do we need to raise the focused component?  
  if (et == csevMouseDown)
    RaiseComponents(cmp);
  else if (flags & AWSF_RaiseOnMouseOver &&
    (et == csevMouseMove || et == csevMouseUp || et == csevMouseClick))
  {
    RaiseComponents(cmp);

    // if component is focusable then focus it
    if (cmp && cmp->Focusable())
      SetFocusedComponent(cmp);
  }
  
  return true;
}

void awsManager::ChangeKeyboardFocus(iAwsComponent *cmp, iEvent &Event)
{
  // Reusing this event, save the orignal type
  uint8 et = Event.Type;

  if (et == csevMouseDown 
    ||(et == csevMouseMove && (flags & AWSF_RaiseOnMouseOver))
    || ((et == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&   
    (flags & AWSF_KeyboardControl)))
  {
    if (keyb_focus != cmp)
    {
      // Create a new event for Got/Lost Focus messages
      if (keyb_focus)
      {
        Event.Type = csevLostFocus;
        keyb_focus->HandleEvent (Event);
      }

      keyb_focus = cmp;
      if (keyb_focus)
      {
        Event.Type = csevGainFocus;
        keyb_focus->HandleEvent (Event);
      }
      Event.Type = et;
    }
  }
}

void awsManager::RaiseComponents(iAwsComponent* comp)
{
  while (comp)
  {
    if (comp->Flags() & AWSF_CMP_TOP_SELECT)
      comp->Raise();

    comp = comp->Parent();
  }
}

#define RegisterFactory(factoryclass)	\
  factory = new factoryclass(this);  \
  factory->DecRef ();

void awsManager::RegisterCommonComponents ()
{
  // Components register themselves into the window manager.  Just creating a
  // factory takes care of all the implementation details.  There's nothing
  // else you need to do.
  iAwsComponentFactory* factory;
  RegisterFactory (awsCmdButtonFactory);
  RegisterFactory (awsLabelFactory);
  RegisterFactory (awsTextBoxFactory);
  RegisterFactory (awsRadButtonFactory);
  RegisterFactory (awsCheckBoxFactory);
  RegisterFactory (awsGroupFrameFactory);
  RegisterFactory (awsListBoxFactory);
  RegisterFactory (awsScrollBarFactory);
  RegisterFactory (awsBarChartFactory);
  RegisterFactory (awsStatusBarFactory);
  RegisterFactory (awsNotebookFactory);
  RegisterFactory (awsNotebookPageFactory);
  RegisterFactory (awsNotebookButtonFactory);
  RegisterFactory (awsWindowFactory);
  RegisterFactory (awsEngineViewFactory);
  RegisterFactory (awsImageViewFactory);
  RegisterFactory (awsMultiLineEditFactory);
  RegisterFactory (awsControlBarFactory);
  RegisterFactory (awsPopupMenuFactory);
  RegisterFactory (awsMenuEntryFactory);
  RegisterFactory (awsMenuBarFactory);
  RegisterFactory (awsMenuBarEntryFactory);
  factory = new awsComponentFactory (this);
  RegisterComponentFactory (factory, "awsComponent");
  factory->DecRef ();
  
  // Standard sink
  awsStandardSink* temp_sink = new awsStandardSink (this);
  GetSinkMgr ()->RegisterSink ("awsStandardSink", temp_sink);
  temp_sink->DecRef ();

  // Global constants
  GetPrefMgr ()->RegisterConstant ("True", 1);
  GetPrefMgr ()->RegisterConstant ("False", 0);
  GetPrefMgr ()->RegisterConstant ("Yes", 1);
  GetPrefMgr ()->RegisterConstant ("No", 0);

  GetPrefMgr ()->RegisterConstant ("signalComponentCreated", 0xefffffff);

  GetPrefMgr ()->RegisterConstant ("mouseOver", 1);
  GetPrefMgr ()->RegisterConstant ("mouseClick", 2);

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

  GetPrefMgr ()->RegisterConstant ("blCenter", 0);
  GetPrefMgr ()->RegisterConstant ("blNorth", 1);
  GetPrefMgr ()->RegisterConstant ("blEast", 2);
  GetPrefMgr ()->RegisterConstant ("blSouth", 3);
  GetPrefMgr ()->RegisterConstant ("blWest", 4);

  GetPrefMgr ()->RegisterConstant ("fsBump", _3dfsBump);
  GetPrefMgr ()->RegisterConstant ("fsSimple", _3dfsSimple);
  GetPrefMgr ()->RegisterConstant ("fsRaised", _3dfsRaised);
  GetPrefMgr ()->RegisterConstant ("fsSunken", _3dfsSunken);
  GetPrefMgr ()->RegisterConstant ("fsFlat", _3dfsFlat);
  GetPrefMgr ()->RegisterConstant ("fsNone", _3dfsNone);
  GetPrefMgr ()->RegisterConstant ("fsBevel", _3dfsBevel);
  GetPrefMgr ()->RegisterConstant ("fsThick", _3dfsThick);
  GetPrefMgr ()->RegisterConstant ("fsBitmap", _3dfsBitmap);
  GetPrefMgr ()->RegisterConstant ("fsSmallRaised", _3dfsSmallRaised);
  GetPrefMgr ()->RegisterConstant ("fsSmallSunken", _3dfsSmallSunken);
}

bool awsManager::AllWindowsHidden ()
{
  iAwsComponent *curwin = top;

  while (curwin)
  {
    if (!curwin->isHidden ())
      return false;

    curwin = curwin->ComponentBelow ();
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

void awsManager::ComponentDestroyed(iAwsComponent *comp)
{
  if (mouse_in == comp)
  {
    mouse_in = 0;
  }
  if (keyb_focus == comp)
  {
    keyb_focus = 0;
  }
}
