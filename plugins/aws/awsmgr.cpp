/******************************************************************************************
 * Notes:
 *    1. The redraw process still does not properly support clipping.  This should be revisted
 *  when we figure out why the clipping mechanisms in the sofware and opengl don't work like one
 *  would expect.
 *
 *  Clipping works properly now at least in software mode -- Noah
 *  There are also now 3 new tests in the g2dtest app which can help test
 *  clipping on other canvases.
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
#include "awslstbx.h"
#include "awsscrbr.h"
#include "awsbarct.h"
#include "awsstbar.h"
#include "awsEngineView.h"
#include "awsControlBar.h"
#include "awsMenu.h"
#include "awsimgvw.h"
#include "awsmled.h"

#include "aws3dfrm.h"


#include "awscmpt.h"
#include "awscscr.h"
#include "awslayot.h"

#include "awsntbk.h"
#include "awswin.h"

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
  iAwsComponentFactory *factory,
  const char *name)
{
  awsComponentFactoryMap *cfm = new awsComponentFactoryMap;

  factory->IncRef ();

  cfm->factory = factory;
  cfm->id = prefmgr->NameToId (name);

  component_factories.AddItem (cfm);
}

iAwsComponentFactory *awsManager::FindComponentFactory (const char *name)
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


iAwsComponent *awsManager::CreateEmbeddableComponentFrom(const char *name)

{

	iAwsComponentFactory *factory = FindComponentFactory(name);

    // If we have a factory for this component, then create it and set it up.
    if (factory) 

	{
		factory->DecRef();

		return factory->Create ();

	}

	return NULL;

}


iAwsComponent *awsManager::GetTopComponent ()
{
  return top;
}

void awsManager::SetTopComponent (iAwsComponent *_top)
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

    frame.Set (0, 0, ptG2D->GetWidth (), ptG2D->GetHeight ());

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

void awsManager::CreateTransition(iAwsComponent *win, unsigned transition_type, float step_size)
{
  if (win==NULL) return;

  awsWindowTransition *t = new awsWindowTransition;
  int w = G2D()->GetWidth();
  int h = G2D()->GetHeight();
  
  t->morph=0.0;
  t->morph_step=step_size;
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

void awsManager::CreateTransitionEx(iAwsComponent *win, unsigned transition_type, float step_size, csRect &user)
{
  if (win==NULL) return;

  awsWindowTransition *t = new awsWindowTransition;
  //  int w = G2D()->GetWidth();
  //  int h = G2D()->GetHeight();
  
  t->morph=0.0;
  t->morph_step=step_size;
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

bool 
awsManager::PerformTransition(awsWindowTransition *t)
{
  float dx, dy;
  csRect interp(t->start);

  if (t->morph==0.0)
  {
    t->win->Move(t->start.xmin - t->win->Frame().xmin,
	         t->start.ymin - t->win->Frame().ymin);
    
  }

  dx=t->end.xmin - t->start.xmin;
  dy=t->end.ymin - t->start.ymin;

  dx*=t->morph;
  dy*=t->morph;

  interp.Move((int)dx, (int)dy);
  t->win->Move(interp.xmin - t->win->Frame().xmin,
	       interp.ymin - t->win->Frame().ymin);

  t->win->Invalidate();

  if (t->morph==1.0)
  {
    switch(t->transition_type)
    {
    case AWS_TRANSITION_SLIDE_IN_LEFT:
    case AWS_TRANSITION_SLIDE_IN_RIGHT:
    case AWS_TRANSITION_SLIDE_IN_UP:
    case AWS_TRANSITION_SLIDE_IN_DOWN:
    default:
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

    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
      "returning false from transition.\n");

    return false;
  }
  else
  {

    t->morph+=t->morph_step;
    if (t->morph>1.0)
      t->morph=1.0;
  }

  return true;
}

iAwsComponent* awsManager::ComponentAt(int x, int y)
{
	for(iAwsComponent* cur = GetTopComponent(); cur; cur = cur->ComponentBelow())
	{
		// find the top level component which contains the point
		iAwsComponent* child = cur->ChildAt(x,y);
		if(cur->isHidden()) continue;
		if(child)
		{
			// then iterate down the tree until a child no longer contains
		    // the point
			iAwsComponent* temp;
			while( (temp = child->ChildAt(x,y)))
				child = temp;
			return child;
		}
		else if(cur->Frame().Contains(x,y))
			return cur;
	}
	return NULL;
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
  int i;

  if (win->isHidden ()) return false;

  for (i = 0; i < dirty.Count (); ++i)
    if (win->Overlaps (dirty.RectAt (i))) return true;

  return false;
}

bool awsManager::ComponentIsInTransition(iAwsComponent *win, bool perform_transition)
{
  int i;

  if (win->isHidden ()) return false;

  for(i = 0; i < transitions.Length(); ++i)
  {
    awsWindowTransition *t = (awsWindowTransition *)transitions[i];

    if (t->win==win)
    {
      if (perform_transition) return PerformTransition(t);
      else return true;
    }
  }

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

  int erasefill = GetPrefMgr ()->GetColor (AC_TRANSPARENT);
  int i;

  iAwsComponent *curwin = top, *oldwin = 0;

  redraw_tag++;

  csRect clip (frame);

  CS_ASSERT(frame.xmax <= ptG2D->GetWidth() && frame.ymax <= ptG2D->GetHeight());

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
  if (transitions.Length()==0   
      && dirty.Count () == 0 
      && !(flags & AWSF_AlwaysRedrawWindows)
     )

    return ;


  /******* The following code is only executed if there is something to redraw *************/
  curwin = top;

  // check to see if any part of this window needs redrawn, or if the always draw flag is set
  while (curwin)
  {
    if (
        (   ComponentIsInTransition(curwin, true) /* MUST COME BEFORE OTHERS! */
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
        dr.Intersect(curwin->Frame());
        cr.Union (dr);
      }

      // cr is now the smallest possible rect that
      // contains all the dirty area over the window
      RedrawWindow (curwin, cr);
    }         // end if this window is dirty
    curwin = curwin->ComponentAbove ();
  }           // end iterate all windows

  //int i;

  // Debug code: draw boxes around dirty regions

  /* for(i=0; i<dirty.Count(); ++i)
  {
         csRect dr(dirty.RectAt(i));
         ptG2D->DrawLine(dr.xmin, dr.ymin, dr.xmax, dr.ymin, GetPrefMgr()->GetColor(AC_WHITE));
         ptG2D->DrawLine(dr.xmin, dr.ymin, dr.xmin, dr.ymax, GetPrefMgr()->GetColor(AC_WHITE));
         ptG2D->DrawLine(dr.xmin, dr.ymax, dr.xmax, dr.ymax, GetPrefMgr()->GetColor(AC_WHITE));
         ptG2D->DrawLine(dr.xmax, dr.ymin, dr.xmax, dr.ymax, GetPrefMgr()->GetColor(AC_WHITE));
  } */


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
  ptG3D->FinishDraw ();

  // Reset the dirty region
  dirty.makeEmpty ();

  // done with the redraw!
}

void awsManager::RedrawWindow (iAwsComponent *win, csRect clip)
{
  // precondition: clip is contained inside the the window frame and 
  // also inside the canvas drawing area

  if (DEBUG_MANAGER) printf ("aws-debug: start drawing window.\n");

  ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

  // tell window frame to draw
  win->OnDraw (clip); 

  /// Draw the children
  RecursiveDrawChildren (win, clip);

  if (DEBUG_MANAGER) printf ("aws-debug: finished drawing window.\n");
}

void awsManager::RecursiveDrawChildren (iAwsComponent *cmp, csRect clip)
{
  iAwsComponent *child;

  if (DEBUG_MANAGER) printf ("aws-debug: start drawing children.\n");

  if(!cmp->HasChildren()) return;

  for(child = cmp->GetTopChild(); child->ComponentBelow();
      child = child->ComponentBelow());

  for (; child ; child = child->ComponentAbove())
  {

    // Do not draw the child if it's hidden or invisible.
    if (child->isHidden() ||
        child->Flags() & AWSF_CMP_INVISIBLE)
	continue;

    if (DEBUG_MANAGER)
      printf ("aws-debug: entered draw children loop for %p.\n", child);

    csRect child_clip(child->Frame());

	  child_clip.Intersect (clip);

    if(! (child->Flags() & AWSF_CMP_NON_CLIENT))
      child_clip.Intersect (cmp->ClientFrame());

	if(child_clip.IsEmpty()) // there is nothing to draw
		continue;

	// enforce the clipping
    ptG2D->SetClipRect(child_clip.xmin, child_clip.ymin, 
                       child_clip.xmax, child_clip.ymax);

    // Draw the child
    child->OnDraw (child_clip);

	RecursiveDrawChildren (child, child_clip);
  }           // End for
  if (DEBUG_MANAGER) printf ("aws-debug: finished drawing children.\n");
}

iAwsParmList *awsManager::CreateParmList ()
{
  return new awsParmList;
}

iAwsComponent *awsManager::CreateWindowFrom (const char* defname)
{
  if (DEBUG_MANAGER)
    printf ("aws-debug: Searching for window def \"%s\"\n", defname);

  // Find the window definition
  iAwsComponentNode *cmpnode = GetPrefMgr ()->FindWindowDef (defname);

  if (DEBUG_MANAGER)
    printf (
      "aws-debug: Window definition was %s\n",
      (cmpnode ? "found." : "not found."));

  // If we couldn't find it, abort
  if (cmpnode == NULL) return NULL;

  // Create a new component
  iAwsComponentFactory *factory = FindComponentFactory (
          cmpnode->ComponentTypeName ()->GetData ());

  // If we do not have a factory, abort construction
  if (!factory) return NULL;
  iAwsComponent *comp = factory->Create ();

  // Setup the component
  if(!comp->Create(this, NULL, cmpnode))
    return NULL;

  /* Now recurse through all of the child nodes, creating them and setting them
  up.  Nodes are created via their factory functions.  If a factory cannot be
  found, then that node and all of it's children are ignored. */
  CreateChildrenFromDef (this, comp, cmpnode);

  return comp;
}

void awsManager::CreateChildrenFromDef (
  iAws *wmgr,
  iAwsComponent *parent,
  iAwsComponentNode *settings)
{
  int i;
  for (i = 0; i < settings->Length (); ++i)
  {
    iAwsKey *key = settings->GetAt (i);

    if (key == NULL) continue;

    if (key->Type () == KEY_COMPONENT)
    {
      iAwsComponentNode *comp_node = 
         SCF_QUERY_INTERFACE(key, iAwsComponentNode);
      CS_ASSERT(comp_node);
      iAwsComponentFactory *factory = FindComponentFactory (
          comp_node->ComponentTypeName ()->GetData ());

      // If we have a factory for this component, then create it and set it up.
      if (factory)
      {
        iAwsComponent *comp = factory->Create ();

        // sets up the component for use
        // returns true if all went well
        if(comp->Create(wmgr, parent, comp_node))

          // Process all subcomponents of this component.
          CreateChildrenFromDef (wmgr, comp, comp_node);
      }
    
      comp_node->DecRef();

    }
    else if (key->Type () == KEY_CONNECTIONMAP)
    {
      int j;
      iAwsKeyContainer *conmap = 
        SCF_QUERY_INTERFACE(key, iAwsKeyContainer);
      CS_ASSERT(conmap);
      awsSlot *slot = new awsSlot ();

      for (j = 0; j < conmap->Length (); ++j)
      {
        iAwsConnectionKey *con = 
          SCF_QUERY_INTERFACE(conmap->GetAt (j), iAwsConnectionKey);
        CS_ASSERT(con);

        slot->Connect (parent, con->Signal (), con->Sink (), con->Trigger ());
        con->DecRef();
      }       // end for count of connections

      //  Now that we've processed the connection map, we use a trick and send out
      // a creation signal for the component.  Note that we can't do this until the
      // connection map has been created, or the signal won't go anywhere!
      parent->Broadcast (0xefffffff);

      conmap->DecRef();

    }         // end else
  }           // end for count of keys

  parent->LayoutChildren();

}


void awsManager::CaptureMouse (iAwsComponent *comp)
{
  if (DEBUG_MANAGER) printf("aws-debug: Mouse captured\n");


  mouse_captured = true;
  if (comp == NULL) comp = GetTopComponent ();

  mouse_focus = comp;
}

void awsManager::ReleaseMouse ()
{
  if (DEBUG_MANAGER) printf("aws-debug: Mouse released\n");
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
  case csevMouseClick:
  case csevMouseDown:
    {
      // If the mouse is locked keep it there
      if (mouse_captured && mouse_focus)
        if(mouse_focus->HandleEvent (Event)) return true;

      // Find out which component contains the pointer.
      iAwsComponent* comp = ComponentAt(Event.Mouse.x, Event.Mouse.y);
      
      // if the mouse is still captured just stop
      if (mouse_captured && mouse_focus)
        return false;
      
      // check to see if focus needs updating
      // if that succeeds then the keyboard might need focusing too
      if(ChangeMouseFocus(comp, Event))
        ChangeKeyboardFocus(comp, Event);
      
      // its possible that some component captured the mouse
      // in response to losing mouse focus. If that occured then we
      // give that component a chance to handle the event
      // rather than the component curently containing the mouse
      if(mouse_captured && mouse_focus)
        return mouse_focus->HandleEvent(Event);
      
      // move up the chain of components to find the first one that can handle
      // the event. 
      while(comp && !(comp->Flags() & AWSF_CMP_DEAF) &&  !comp->HandleEvent(Event))
        comp = comp->Parent();
      
      // if we haven't reached the top then some component handled it
      if(comp) return true;
    }
    break;
  
  case csevKeyDown:
    if (keyb_focus) keyb_focus->HandleEvent (Event);
    
    break;
}

  return false;
}


iAwsComponent* awsManager::FindCommonParent(iAwsComponent* cmp1, iAwsComponent* cmp2)
{
  iAwsComponent* testParent1 = cmp1;
  iAwsComponent* testParent2 = cmp2;

  while(testParent1)
  {
    while(testParent2)
    {
      if(testParent1 == testParent2)
        return testParent1;
      testParent2 = testParent2->Parent();
    }
    testParent2 = cmp2;
    testParent1 = testParent1->Parent();
  }

  return NULL;
}

// note, if this is too slow common_parent could be calculated once and then
// passed and updated as necessary. Unless component trees get really deep though
// I don't think it matters so I left it this way because its slightly clearer.

bool awsManager::ChangeMouseFocus(iAwsComponent *cmp, iEvent &Event)
{
  iAwsComponent* common_parent = FindCommonParent(mouse_in, cmp);
  
  if(mouse_in == cmp)
    return ChangeMouseFocusHelper(cmp, Event);
  else if(common_parent == mouse_in)
  {
    if(ChangeMouseFocus(cmp->Parent(), Event))   // get focus to the parent
      return ChangeMouseFocusHelper(cmp, Event); // then get it to cmp
  }
  else
  {
    if(ChangeMouseFocusHelper(mouse_in->Parent(),Event)) // get focus to mouse_in's parent
      return ChangeMouseFocus(cmp, Event);               // get it the rest of the way
    else
      return false;
  }
  // can't get here but compiler seems to complain
  // if I don't have it
  return false;
}



bool awsManager::ChangeMouseFocusHelper(iAwsComponent *cmp, iEvent &Event)
{
  // Reusing this event, save the orignal type
  uint8 et = Event.Type;
  if(mouse_in != cmp)
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
    if(mouse_captured && mouse_focus)
    {
      Event.Type = et;
      return false;
    }
    
    mouse_in = cmp;
    
    if(mouse_in)
    {
      Event.Type = csevMouseEnter;
      mouse_in->HandleEvent (Event);
    }
    
    Event.Type = et;
  }
  
  // do we need to raise the focused component?
  
  if(et == csevMouseDown)
    RaiseComponents(cmp);
  else if( flags & AWSF_RaiseOnMouseOver &&
    (et == csevMouseMove || et == csevMouseUp || et == csevMouseClick))
    RaiseComponents(cmp);
  return true;
}


void awsManager::ChangeKeyboardFocus(iAwsComponent *cmp, iEvent &Event)
{
  // Reusing this event, save the orignal type
	uint8 et = Event.Type;

  if (et == csevMouseDown)
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
      if(keyb_focus)
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
  while(comp)
  {
	 if(comp->Flags() & AWSF_CMP_TOP_SELECT) comp->Raise();
	 comp = comp->Parent();
  }
}

void awsManager::RegisterCommonComponents ()
{
  //   Components register themselves into the window manager.  Just creating a factory
  // takes care of all the implementation details.  There's nothing else you need to do.
  (void)new awsCmdButtonFactory (this);
  (void)new awsLabelFactory (this);
  (void)new awsTextBoxFactory (this);
  (void)new awsRadButtonFactory (this);
  (void)new awsCheckBoxFactory (this);
  (void)new awsGroupFrameFactory (this);
  (void)new awsListBoxFactory (this);
  (void)new awsScrollBarFactory (this);
  (void)new awsBarChartFactory (this);
  (void)new awsStatusBarFactory (this);
  (void)new awsNotebookFactory (this);
  (void)new awsNotebookPageFactory (this);
  (void)new awsNotebookButtonFactory (this);
  (void)new awsWindowFactory (this);
  (void)new awsEngineViewFactory(this);
  (void)new awsImageViewFactory (this);
  (void)new awsMultiLineEditFactory (this);
  (void)new awsControlBarFactory(this);
  (void)new awsPopupMenuFactory(this);
  (void)new awsMenuEntryFactory(this);
  (void)new awsMenuBarFactory(this);
  (void)new awsMenuBarEntryFactory(this);
  
  RegisterComponentFactory (new awsComponentFactory (this), "awsComponent");

  // Standard sink
  GetSinkMgr ()->RegisterSink ("awsStandardSink", new awsStandardSink (this));

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
    if (!curwin->isHidden ()) return false;

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
