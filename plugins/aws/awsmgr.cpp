#include "cssysdef.h"
#include "iutil/plugin.h"
#include "aws.h"
#include "awsprefs.h"
#include "ivideo/txtmgr.h"
#include "iengine/engine.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "ivaria/reporter.h"

#include "awscmdbt.h"


#include <stdio.h>

const int proctex_width=512;
const int proctex_height=512; 
const int DEBUG_MANAGER = false;

awsManager::awsComponentFactoryMap::~awsComponentFactoryMap ()
{
  factory->DecRef ();
}

awsManager::awsManager(iBase *p):prefmgr(NULL), sinkmgr(NULL),
               updatestore_dirty(true), 
               top(NULL), mouse_in(NULL), mouse_captured(false),
               ptG2D(NULL), ptG3D(NULL), object_reg(NULL), 
               UsingDefaultContext(false), DefaultContextInitialized(false)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);

  canvas.DisableAutoUpdate();
}

awsManager::~awsManager()
{
  SCF_DEC_REF (prefmgr);
  SCF_DEC_REF (sinkmgr);

  void *p = component_factories.GetFirstItem();
  while ((p=component_factories.GetCurrentItem ()))
  {
    delete (awsComponentFactoryMap *)p;
    component_factories.RemoveItem ();
  }

}

bool 
awsManager::Initialize(iObjectRegistry *object_reg)
{   
  awsManager::object_reg = object_reg;
    
  if (DEBUG_MANAGER) printf("aws-debug: getting preference manager.\n");  
  prefmgr =  SCF_CREATE_INSTANCE ("crystalspace.window.preferencemanager", iAwsPrefManager);

  if (DEBUG_MANAGER) printf("aws-debug: getting sink manager.\n");  
  sinkmgr =  SCF_CREATE_INSTANCE ("crystalspace.window.sinkmanager", iAwsSinkManager);
  
  if (!prefmgr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
        "AWS could not create an instance of the default PREFERENCE manager. This is a serious error.");
    return false;
  }
  else
  {
    if (DEBUG_MANAGER) printf("aws-debug: initing and setting the internal preference manager.\n");
    
    prefmgr->SetWindowMgr(this);
    prefmgr->Setup(object_reg);
  }

  if (!sinkmgr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
        "AWS could not create an instance of the default SINK manager. This is a serious error.");
    return false;
  }

  if (DEBUG_MANAGER) printf("aws-debug: registering common components.\n");

  RegisterCommonComponents();
  
  if (DEBUG_MANAGER) printf("aws-debug: left aws initialize.\n");
 
  return true;
}

iAwsPrefManager *
awsManager::GetPrefMgr()
{
  return prefmgr;
}

iAwsSinkManager *
awsManager::GetSinkMgr()
{
  return sinkmgr;
}
 
void
awsManager::SetPrefMgr(iAwsPrefManager *pmgr)
{
   if (prefmgr && pmgr)
   {
      prefmgr->DecRef();
      pmgr->IncRef();

      prefmgr=pmgr;
   }
   else if (pmgr)
   {
      pmgr->IncRef();
      prefmgr=pmgr;
   }
}

void
awsManager::RegisterComponentFactory(awsComponentFactory *factory, char *name)
{
   awsComponentFactoryMap *cfm = new awsComponentFactoryMap;

   factory->IncRef();

   cfm->factory= factory;
   cfm->id=prefmgr->NameToId(name);

   component_factories.AddItem(cfm);
}

awsComponentFactory *
awsManager::FindComponentFactory(char *name)
{
  void *p = component_factories.GetFirstItem();
  unsigned long id = prefmgr->NameToId(name);
  
  do 
  {
    awsComponentFactoryMap *cfm = (awsComponentFactoryMap *)p;
    
    if (cfm->id == id)
      return cfm->factory;
      
    p = component_factories.GetNextItem();
  } while(p!=component_factories.PeekFirstItem());
  
  return NULL;
}

iAwsWindow *
awsManager::GetTopWindow()
{ return top; }
    
void 
awsManager::SetTopWindow(iAwsWindow *_top)
{ top = _top; }

void 
awsManager::SetContext(iGraphics2D *g2d, iGraphics3D *g3d)
{
   if (g2d && g3d)
   {
       ptG2D = g2d;
       ptG3D = g3d;
       
       frame.Set(0,0,ptG2D->GetWidth(), ptG2D->GetHeight());
       
       UsingDefaultContext=false;

       Mark(frame);
   }
}

void 
awsManager::SetDefaultContext(iEngine* engine, iTextureManager* txtmgr)
{
  if (!DefaultContextInitialized)
  {
    canvas.SetSize(proctex_width, proctex_height);
    canvas.SetKeyColor(255,0,255);
    if (!canvas.Initialize(object_reg, engine, txtmgr, "awsCanvas"))
      printf("aws-debug: SetDefaultContext failed to initialize the memory canvas.\n");
    else
      printf("aws-debug: Memory canvas initialized!\n");
    
    if (!canvas.PrepareAnim())
      printf("aws-debug: Prepare anim failed!\n");
    else
      printf("aws-debug: Prepare anim succeeded.\n");
   
//    if (engine!=NULL)
//    {
//       iTextureWrapper *tw = engine->GetTextureList()->NewTexture(canvas.GetTextureWrapper()->GetTextureHandle());
//       iMaterialWrapper *canvasMat = engine->CreateMaterial("awsCanvasMat", tw);
//    }
    
    DefaultContextInitialized=true;
  }
            
  ptG2D = canvas.G2D();
  ptG3D = canvas.G3D();
  
  printf("aws-debug: G2D=%p G3D=%p\n", ptG2D, ptG3D);

  if (txtmgr)
    GetPrefMgr()->SetTextureManager(txtmgr);

  if (ptG2D)
    GetPrefMgr()->SetFontServer(ptG2D->GetFontServer());
    
  if (ptG2D && ptG3D) 
  {
    ptG2D->DoubleBuffer(false);
    ptG3D->BeginDraw(CSDRAW_2DGRAPHICS);
    ptG2D->Clear(txtmgr->FindRGB(255,0,255));
    ptG3D->FinishDraw();
    ptG3D->Print(NULL);
    
    frame.Set(0,0,ptG2D->GetWidth(), ptG2D->GetHeight());
    
    Mark(frame);
    UsingDefaultContext=true;
  }
}

void
awsManager::Mark(csRect &rect)
{
  dirty.Include(rect);
}

void
awsManager::Unmark(csRect &rect)
{
  dirty.Exclude(rect);  
}

void
awsManager::InvalidateUpdateStore()
{
  updatestore_dirty=true;
}


bool
awsManager::WindowIsDirty(iAwsWindow *win)
{
  int i;
   
  for(i=0; i<dirty.Count(); ++i)
    if (win->Overlaps(dirty.RectAt(i))) return true;
  
  return false;
}

void 
awsManager::UpdateStore()
{
 if (updatestore_dirty)
  {
 
   iAwsWindow *curwin=top;
   
   updatestore.makeEmpty();

   // Get all frames into the store.
   while(curwin)
   {
      csRect r(curwin->Frame());
      //printf("\t%d,%d,%d,%d\n", r.xmin, r.ymin, r.xmax, r.ymax);
      updatestore.Include(r);
      curwin = curwin->WindowBelow();
   }

   updatestore_dirty=false;

  }
}

void
awsManager::Print(iGraphics3D *g3d)
{
  UpdateStore();
  
  int i;
  for(i=0; i<updatestore.Count(); ++i)
  {
    csRect r(updatestore.RectAt(i));
    g3d->DrawPixmap(canvas.GetTextureWrapper()->GetTextureHandle(), 
      		  r.xmin,r.ymin,r.xmax-r.xmin,r.ymax-r.ymin,
		  r.xmin,r.ymin,r.xmax-r.xmin,r.ymax-r.ymin,
                  0);
  }

/*
  // Debug code
  iGraphics2D *g2d = g3d->GetDriver2D();
  for(i=0; i<updatestore.Count(); ++i)
  {
    csRect r(updatestore.RectAt(i));
     
    g2d->DrawLine(r.xmin, r.ymin, r.xmax, r.ymin, GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmin, r.ymin, r.xmin, r.ymax, GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmin, r.ymax, r.xmax, r.ymax, GetPrefMgr()->GetColor(AC_WHITE));
    g2d->DrawLine(r.xmax, r.ymin, r.xmax, r.ymax, GetPrefMgr()->GetColor(AC_WHITE));

  }
*/
}

void       
awsManager::Redraw()
{
   static unsigned redraw_tag = 1;
   static csRect bounds(0,0,proctex_width,proctex_height);
   //int    erasefill = GetPrefMgr()->GetColor(AC_TRANSPARENT);
   int    i;
        

   redraw_tag++;
   
   ptG3D->BeginDraw(CSDRAW_2DGRAPHICS);
   
   //ptG2D->SetClipRect(0,0,proctex_width, proctex_width);

   //if (redraw_tag%2) ptG2D->DrawBox( 0,  0,25, 25, GetPrefMgr()->GetColor(AC_SHADOW));
   //else              ptG2D->DrawBox( 0,  0,25, 25, GetPrefMgr()->GetColor(AC_HIGHLIGHT));
       
   // check to see if there is anything to redraw.
   if (dirty.Count() == 0) 
      return;
   
   /******* The following code is only executed if there is something to redraw *************/
   
   //if (updatestore_dirty && UsingDefaultContext)
     //ptG2D->DrawBox(0,0, proctex_width,proctex_height,erasefill);


   iAwsWindow *curwin=top, *oldwin = 0;
   
   // check to see if any part of this window needs redrawn
   while(curwin)
   {
      if (WindowIsDirty(curwin)) {
        curwin->SetRedrawTag(redraw_tag);
      }

      oldwin=curwin;
      curwin = curwin->WindowBelow();
   }

   /*  At this point in time, oldwin points to the bottom most window.  That means that we take curwin, set it
    * equal to oldwin, and then follow the chain up to the top, redrawing on the way.  This makes sure that we 
    * only redraw each window once.
    */
   
   curwin=oldwin;
   while(curwin)
   {
     if (DEBUG_MANAGER)
     {
      printf("aws-debug: consider window: %p\n", curwin); 
      printf("aws-debug: redraw tag: %d/%d\n", curwin->RedrawTag(), redraw_tag);
     }

      if (redraw_tag == curwin->RedrawTag()) 
      { 
        if (DEBUG_MANAGER) printf("aws-debug: window is dirty, redraw.\n");

        for(i=0; i<dirty.Count(); ++i)
        {
          
          if(DEBUG_MANAGER) printf("aws-debug: consider rect:%d of %d\n", i, dirty.Count()); 

          csRect dr(dirty.RectAt(i));

          // Find out if we need to erase.
         /* if (!UsingDefaultContext || updatestore_dirty)
          {
            csRect lo(dr);
            lo.Subtract(curwin->Frame());

            if (!lo.IsEmpty())            
              ptG2D->DrawBox(dr.xmin-1, dr.ymin-1, dr.xmax+1, dr.ymax+1, erasefill);
          }*/
          
          
          RedrawWindow(curwin, dr);
        }
      }

      curwin=curwin->WindowAbove();
   }
  
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
   
   // This only needs to happen when drawing to the default context.
   if (UsingDefaultContext)
   {     
     ptG3D->FinishDraw ();
     ptG3D->Print(&bounds);

     //UpdateStore();

     //for(i=0; i<updatestore.Count(); ++i)
       //ptG3D->Print(&(updatestore.RectAt(i)));
   }

   // Reset the dirty region
   dirty.makeEmpty();

   // done with the redraw!
}

void
awsManager::RedrawWindow(iAwsWindow *win, csRect &dirtyarea)
{
     if (DEBUG_MANAGER) printf("aws-debug: start drawing window.\n");

     /// See if this window intersects with this dirty area
     if (!dirtyarea.Intersects(win->Frame()))
       return;

     /// Draw the window first.
     //csRect clip(win->Frame());

     /// Clip the window to it's intersection with the dirty rectangle
     //clip.Intersect(dirtyarea);
     //ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

     /// Tell the window to draw
     win->OnDraw(win->Frame());

     /// Now draw all of it's children
     RecursiveDrawChildren(win, dirtyarea);

     if (DEBUG_MANAGER) printf("aws-debug: finished drawing window.\n");
}

void
awsManager::RecursiveDrawChildren(iAwsComponent *cmp, csRect &dirtyarea)
{
   int i; 
   iAwsComponent *child;

   if (DEBUG_MANAGER) printf("aws-debug: start drawing children.\n");

   for(i=0; i<cmp->GetChildCount(); ++i)
   {
     child = cmp->GetChildAt(i);

     if (DEBUG_MANAGER) printf("aws-debug: entered draw children loop for %p.\n", child);

     // Check to see if this component even needs redrawing.
     //if (!dirtyarea.Intersects(child->Frame()))
       //continue;                                            

     csRect clip(child->Frame());
     clip.Intersect(dirtyarea);
     //ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

     // Draw the child
     child->OnDraw(clip);

     // If it has children, draw them
     if (child->HasChildren())
       RecursiveDrawChildren(child, dirtyarea);
     
   } // End for

   if (DEBUG_MANAGER) printf("aws-debug: finished drawing children.\n");

}

iAwsWindow *
awsManager::CreateWindowFrom(char *defname)
{
   printf("aws-debug: Searching for window def \"%s\"\n", defname);
   
   // Find the window definition
   awsComponentNode *winnode = GetPrefMgr()->FindWindowDef(defname);
   
   printf("aws-debug: Window definition was %s\n", (winnode ? "found." : "not found."));
   
   // If we couldn't find it, abort
   if (winnode==NULL) return NULL;
   
   // Create a new window
   iAwsWindow *win = new awsWindow();

   // Setup the name of the window
   win->SetID(winnode->Name());
   
   // Tell the window to set itself up
   win->Setup(this, winnode);
   
   /* Now recurse through all of the child nodes, creating them and setting them
   up.  Nodes are created via their factory functions.  If a factory cannot be 
   found, then that node and all of it's children are ignored. */
   
   CreateChildrenFromDef(this, win, winnode);
     
   return win;
}

void
awsManager::CreateChildrenFromDef(iAws *wmgr, iAwsComponent *parent, awsComponentNode *settings)
{
  int i;
  for(i=0; i<settings->GetLength(); ++i)
  {
    awsKey *key = settings->GetItemAt(i); 
    
    if (key != NULL && key->Type() == KEY_COMPONENT)
    {
      awsComponentNode *comp_node = (awsComponentNode *)key;
      awsComponentFactory *factory = FindComponentFactory(comp_node->ComponentTypeName()->GetData());
      
      // If we have a factory for this component, then create it and set it up.
      if (factory)
      {
	iAwsComponent *comp = factory->Create();

        // Setup the name of the component
        comp->SetID(comp_node->Name());
		
	// Prepare the component, and add it into it's parent
	comp->Setup(wmgr, comp_node);
	parent->AddChild(comp);
	
	// Process all subcomponents of this component.
	CreateChildrenFromDef(wmgr, comp, comp_node);
      }
      
    }
      
  }
  
  
}

void
awsManager::CaptureMouse()
{
  mouse_captured=true;
}

void   
awsManager::ReleaseMouse()
{
  mouse_captured=false;
}


bool 
awsManager::HandleEvent(iEvent& Event)
{
  
  // Find out what kind of event it is
  switch(Event.Type)
  {
  case csevMouseMove:
  case csevMouseUp:
  case csevMouseDown:
  case csevMouseClick:

    // Find out which top most window contains the pointer.
    if (GetTopWindow())
    {
      // If the mouse is locked into the top window, keep it there
      if (mouse_captured) 
      {
        if (RecursiveBroadcastToChildren(GetTopWindow(), Event)) return true;
        else return GetTopWindow()->HandleEvent(Event);
        
        break;
      }
      
      // If the top window still contains the mouse, it stays on top
      if (GetTopWindow()->Frame().Contains(Event.Mouse.x, Event.Mouse.y))
      {
        if (RecursiveBroadcastToChildren(GetTopWindow(), Event)) return true;
        else return GetTopWindow()->HandleEvent(Event);
        
        break;
      }
      
      else
      {
        // Find the window that DOES contain the mouse.
      
        iAwsWindow *win=GetTopWindow();
      
        // Skip the top 'cause we already checked it.
        if (win) win=win->WindowBelow();

        while(win)
        {
          // If the window contains the mouse, it becomes new top.
          if (win->Frame().Contains(Event.Mouse.x, Event.Mouse.y))
          {
            win->Raise();
            if (RecursiveBroadcastToChildren(win, Event)) return true;
            else return win->HandleEvent(Event);
            break;
          }
          else
            win = win->WindowBelow();
        }
      }
    }

  break;

  case csevKeyDown:
    if (GetTopWindow()) 
    {
      if (RecursiveBroadcastToChildren(GetTopWindow(), Event)) return true;
      else return GetTopWindow()->HandleEvent(Event);
    }

  break;
  }
    
  return false;
}

bool 
awsManager::RecursiveBroadcastToChildren(iAwsComponent *cmp, iEvent &Event)
{
  int i;
  iAwsComponent *child;

  for(i=0; i<cmp->GetChildCount(); ++i)
   {

    child = cmp->GetChildAt(i);

    // If it has children, broadcast to them (depth-first recursion)
    if (child->HasChildren()) 
      if (RecursiveBroadcastToChildren(child, Event))
        return true;

   
    switch(Event.Type)
    {
      case csevMouseMove:
      case csevMouseUp:
      case csevMouseDown:
      case csevMouseClick:

        // Only give to child if it contains the mouse.
        if (child->Frame().Contains(Event.Mouse.x, Event.Mouse.y))
        {

          if (mouse_in != child)
          {
            // Create a new event for MouseExit and MouseEnter
            uchar et = Event.Type;

            if (mouse_in)
            {
              Event.Type = csevMouseExit;
              mouse_in->HandleEvent(Event);
            }

            mouse_in=child;
            Event.Type = csevMouseEnter;
            mouse_in->HandleEvent(Event);

            Event.Type = et;
          }

          return child->HandleEvent(Event);
        } 
      break;


      case csevKeyDown:
        
        if (child->HandleEvent(Event)) return true;
      
      break;
    }  // End switch
   
   } // End for

  return false;

}

void 
awsManager::RegisterCommonComponents()
{
  // Components register themselves into the window manager.  Just creating a factory
  //  takes care of all the implementation details.  There's nothing else you need to do.
  (void)new awsCmdButtonFactory(this);
}
    

//// Canvas stuff  //////////////////////////////////////////////////////////////////////////////////


awsManager::awsCanvas::awsCanvas ()
{
  mat_w=proctex_width;
  mat_h=proctex_height;
  
  texFlags = CS_TEXTURE_2D | CS_TEXTURE_PROC;
   
}

void 
awsManager::awsCanvas::Animate (csTicks current_time)
{
  (void)current_time;
}

void 
awsManager::awsCanvas::SetSize(int w, int h)
{  mat_w=w; mat_h=h; }

