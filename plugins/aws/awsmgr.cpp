#include "cssysdef.h"
#include "aws.h"

awsManager::awsManager(iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);

  canvas.DisableAutoUpdate();
}

awsManager::~awsManager()
{
}

bool 
awsManager::Initialize(iSystem *sys)
{
  canvas.Initialize(sys);

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
      prefmgr=pmgr;
      prefmgr->IncRef();
   }
   else if (pmgr)
   {
      prefmgr=pmgr;
      prefmgr->IncRef();
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
   }
}

void 
awsManager::SetDefaultContext()
{
  ptG2D = canvas.G2D();
  ptG3D = canvas.G3D();
}

void
awsManager::Mark(csRect &rect)
{
   //  If we have too many rects, we simply assume that a large portion of the
   // screen will be filled, so we agglomerate them all in buffer 0.
   if (all_buckets_full)
   {
     dirty[0].AddAdjanced(rect);
     return;
   }

   for(int i=0; i<dirty_lid; ++i)
   {
       if (dirty[i].Intersects(rect))
	 dirty[i].AddAdjanced(rect);
   }

   //  If we get here it's because the rectangle didn't fit anywhere. So,
   // add in a new one, unless we're full, in which case we merge all and
   // set the dirty flag.
   if (dirty_lid>awsNumRectBuckets)
   {
     for(int i=1; i<awsNumRectBuckets; ++i)
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
    for(int i=0; i<dirty_lid; ++i)
      if (win->Overlaps(dirty[i])) return true;
  }

  return false;
}

void       
awsManager::Redraw()
{
   static unsigned redraw_tag = 0;

   redraw_tag++;
    
   // check to see if there is anything to redraw.
   if (dirty[0].IsEmpty()) {
      return;
   }

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
            for(int i=0; i<dirty_lid; ++i)
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

 //// Canvas stuff  //////////////////////////////////////////////////////////////////////////////////


awsManager::awsCanvas::awsCanvas ()
{
   
}

awsManager::awsCanvas::~awsCanvas ()
{
}
 
void 
awsManager::awsCanvas::Animate (csTime current_time)
{
}
