#include "cssysdef.h"
#include "aws.h"
#include "awsprefs.h"
#include "ivideo/txtmgr.h"
#include "iengine/engine.h"
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include <stdio.h>

awsManager::awsManager(iBase *p):prefmgr(NULL),object_reg(NULL), 
               UsingDefaultContext(false), DefaultContextInitialized(false)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);

  canvas.DisableAutoUpdate();
}

awsManager::~awsManager()
{
}

bool 
awsManager::Initialize(iObjectRegistry *object_reg)
{   
  awsManager::object_reg = object_reg;
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
    
  printf("aws-debug: getting preference manager.\n");  
  iAwsPrefs *prefs =  SCF_CREATE_INSTANCE ("crystalspace.window.preferencemanager", iAwsPrefs);
  
  if (!prefs)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
        "AWS could not create an instance of the default preference manager.  This is a serious error.");
    return false;
  }
  else
  {
    printf("aws-debug: initing and setting the internal preference manager.\n");
    
    prefs->Setup(object_reg);

    printf("aws-debug: inited pref manager, now setting.\n");

    SetPrefMgr(prefs);

    printf("aws-debug: decRefing the prefs manager.\n");
    prefs->DecRef();
  }
      
  printf("aws-debug: left aws initialize.\n");
  return true;
}

iAwsPrefs *
awsManager::GetPrefMgr()
{
  return prefmgr;
}
 
void
awsManager::SetPrefMgr(iAwsPrefs *pmgr)
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

awsWindow *
awsManager::GetTopWindow()
{ return top; }
    
void 
awsManager::SetTopWindow(awsWindow *_top)
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
   }
}

void 
awsManager::SetDefaultContext(iEngine* engine, iTextureManager* txtmgr)
{
  if (!DefaultContextInitialized)
  {
    canvas.SetSize(512, 512);
    canvas.SetKeyColor(255,0,255);
    if (!canvas.Initialize(object_reg, engine, txtmgr, "awsCanvas"))
      printf("aws-debug: SetDefaultContext failed to initialize the memory canvas.\n");
    else
      printf("aws-debug: Memory canvas initialized!\n");
    
    if (!canvas.PrepareAnim())
      printf("aws-debug: Prepare anim failed!\n");
    else
      printf("aws-debug: Prepare anim succeeded.\n");
   
    if (engine!=NULL)
    {
      iTextureWrapper *tw = engine->GetTextureList()->NewTexture(canvas.GetTextureWrapper()->GetTextureHandle());
      iMaterialWrapper *canvasMat = engine->CreateMaterial("awsCanvasMat", tw);
    }
    
    DefaultContextInitialized=true;
  }

  if (txtmgr)
    GetPrefMgr()->SetTextureManager(txtmgr);
          
  ptG2D = canvas.G2D();
  ptG3D = canvas.G3D();
  
  printf("aws-debug: G2D=%x G3D=%x\n", ptG2D, ptG3D);
    
  if (ptG2D && ptG3D) 
  {
    ptG2D->DoubleBuffer(false);
    ptG3D->BeginDraw(CSDRAW_2DGRAPHICS);
    ptG2D->Clear(txtmgr->FindRGB(255,0,255));
    ptG3D->FinishDraw();
    ptG3D->Print(NULL);
    
    UsingDefaultContext=true;
  }
}

void
awsManager::Mark(csRect &rect)
{
  int i;
   //  If we have too many rects, we simply assume that a large portion of the
   // screen will be filled, so we agglomerate them all in buffer 0.
   if (all_buckets_full)
   {
     dirty[0].AddAdjanced(rect);
     return;
   }

   for(i=0; i<dirty_lid; ++i)
   {
       if (dirty[i].Intersects(rect))
	 dirty[i].AddAdjanced(rect);
   }

   //  If we get here it's because the rectangle didn't fit anywhere. So,
   // add in a new one, unless we're full, in which case we merge all and
   // set the dirty flag.
   if (dirty_lid>awsNumRectBuckets)
   {
     for(i=1; i<awsNumRectBuckets; ++i)
     {
       dirty[0].AddAdjanced(dirty[i]);
       all_buckets_full=true;
     }
     dirty[0].AddAdjanced(rect);
   }
   else
     dirty[dirty_lid++].Set(rect);
}

bool
awsManager::WindowIsDirty(awsWindow *win)
{
  if (all_buckets_full) 
  {
    // return the result the overlap test with the dirty rect
    return win->Overlaps(dirty[0]);
  }
  else
  {
	int i;
    for(i=0; i<dirty_lid; ++i)
      if (win->Overlaps(dirty[i])) return true;
  }

  return false;
}

void
awsManager::Print(iGraphics3D *g3d)
{
  g3d->DrawPixmap(canvas.GetTextureWrapper()->GetTextureHandle(), 
  		  64,0,512,480, //g3d->GetWidth(),g3d->GetHeight(),
		  0,0,512,480,128);
  		  
  
}

void       
awsManager::Redraw()
{
   static unsigned redraw_tag = 0;
   static csRect bounds(0,0,512,512);

   redraw_tag++;
   
   ptG3D->BeginDraw(CSDRAW_2DGRAPHICS);
   
   ptG2D->SetClipRect(0,0,512,512);
   ptG2D->DrawBox( 2,  202,510, 18, GetPrefMgr()->GetColor(AC_FILL));
   ptG2D->DrawLine(0,  200,512,200, GetPrefMgr()->GetColor(AC_HIGHLIGHT));
   ptG2D->DrawLine(1,  201,511,201, GetPrefMgr()->GetColor(AC_HIGHLIGHT2));
   ptG2D->DrawLine(0,  200,0,  220, GetPrefMgr()->GetColor(AC_HIGHLIGHT));
   ptG2D->DrawLine(1,  201,1,  219, GetPrefMgr()->GetColor(AC_HIGHLIGHT2));
   ptG2D->DrawLine(0,  220,512,220, GetPrefMgr()->GetColor(AC_SHADOW));
   ptG2D->DrawLine(1,  219,511,219, GetPrefMgr()->GetColor(AC_SHADOW2));
   ptG2D->DrawLine(512,200,512,220, GetPrefMgr()->GetColor(AC_SHADOW));
   ptG2D->DrawLine(511,201,511,219, GetPrefMgr()->GetColor(AC_SHADOW2));
   
   // This only needs to happen when drawing to the default context.
   if (UsingDefaultContext)
   {
     ptG3D->FinishDraw ();
     ptG3D->Print (&bounds);
   }
     
   // check to see if there is anything to redraw.
   if (dirty[0].IsEmpty()) {
      return;
   }		
   
   return;
   
   awsWindow *curwin=top, *oldwin;
   
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
      if (curwin->RedrawTag() == redraw_tag) 
      {
         if (all_buckets_full) 
           RedrawWindow(curwin, dirty[0]);
         else
         {
			int i;
            for(i=0; i<dirty_lid; ++i)
              RedrawWindow(curwin, dirty[i]);
         }
      }
      curwin=curwin->WindowAbove();
   }

   // done with the redraw!
}

void
awsManager::RedrawWindow(awsWindow *win, csRect &dirtyarea)
{
     /// See if this window intersects with this dirty area
     if (!dirtyarea.Intersects(win->Frame()))
       return;

     /// Draw the window first.
     csRect clip(win->Frame());

     /// Clip the window to it's intersection with the dirty rectangle
     clip.Intersect(dirtyarea);
     ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

     /// Tell the window to draw
     win->OnDraw(clip);

     /// Now draw all of it's children
     RecursiveDrawChildren(win, dirtyarea);
}

void
awsManager::RecursiveDrawChildren(awsComponent *cmp, csRect &dirtyarea)
{
   awsComponent *child = cmp->GetFirstChild();

   while (child) 
   {
     // Check to see if this component even needs redrawing.
     if (!dirtyarea.Intersects(child->Frame()))
       continue;                                            

     csRect clip(child->Frame());
     clip.Intersect(dirtyarea);
     ptG2D->SetClipRect(clip.xmin, clip.ymin, clip.xmax, clip.ymax);

     // Draw the child
     child->OnDraw(clip);

     // If it has children, draw them
     if (child->HasChildren())
       RecursiveDrawChildren(child, dirtyarea);

    child = cmp->GetNextChild();
   }

}

awsWindow *
awsManager::CreateWindowFrom(char *defname)
{
   printf("aws-debug: Searching for window def \"%s\"\n", defname);
   
   // Find the window definition
   awsComponentNode *winnode = GetPrefMgr()->FindWindowDef(defname);
   
   printf("aws-debug: Window definition was %s\n", (winnode ? "found." : "not found."));
   
   // If we couldn't find it, abort
   if (winnode==NULL) return NULL;
   
   // Create a new window
   awsWindow *win = new awsWindow();
   
   // Tell the window to set itself up
   win->Setup(this, winnode);
   
   /* Now recurse through all of the child nodes, creating them and setting them
   up.  Nodes are created via their factory functions.  If a factory cannot be 
   found, then that node and all of it's children are ignored. */
   
   CreateChildrenFromDef(this, win, winnode);
     
   return win;
}

void
awsManager::CreateChildrenFromDef(iAws *wmgr, awsComponent *parent, awsComponentNode *settings)
{
  int i;
  for(i=0; i<settings->GetLength(); ++i)
  {
    awsKey *key = settings->GetItemAt(i); 
    
    if (key->Type() == KEY_COMPONENT)
    {
      awsComponentNode *comp_node = (awsComponentNode *)key;
      awsComponentFactory *factory = FindComponentFactory(comp_node->ComponentTypeName()->GetData());
      
      // If we have a factory for this component, then create it and set it up.
      if (factory)
      {
	awsComponent *comp = factory->Create();
		
	// Prepare the component, and add it into it's parent
	comp->Setup(wmgr, comp_node);
	parent->AddChild(comp);
	
	// Process all subcomponents of this component.
	CreateChildrenFromDef(wmgr, comp, comp_node);
      }
      
    }
      
  }
  
  
}


 //// Canvas stuff  //////////////////////////////////////////////////////////////////////////////////


awsManager::awsCanvas::awsCanvas ()
{
  mat_w=512;
  mat_h=512;
  
  texFlags = CS_TEXTURE_2D | CS_TEXTURE_PROC;
   
}

awsManager::awsCanvas::~awsCanvas ()
{
}
 
void 
awsManager::awsCanvas::Animate (csTicks current_time)
{
}

void 
awsManager::awsCanvas::SetSize(int w, int h)
{  mat_w=w; mat_h=h; }

