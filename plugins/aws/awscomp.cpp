#include "cssysdef.h"
#include "awsprefs.h"
#include "awscomp.h"
#include "awsslot.h"
#include "iutil/event.h"

#include <stdio.h>


awsComponent::awsComponent():children(NULL)
{
}

awsComponent::~awsComponent()
{
   /// Let go our references to any children if we have them.
   if (children)
   {
      void *item = children->GetFirstItem();

      while(item)
      {
         awsComponent *cmp = (awsComponent *)item;

         cmp->DecRef();

         item = children->GetNextItem();
      }

      delete children;
   }
}
    
    
/**
 *  This function is normally called automatically by the window manager.  You may call it manually if you wish, but
 * there's little reason to do so.  
 **************************************************************************************************************/
bool 
awsComponent::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  if (!wmgr) return false;

  wmgr = _wmgr;
  
  printf("aws-debug: setting up awsComponent.\n");
  
  if (settings) 
  {
  
   iAwsPrefs *pm=WindowManager()->GetPrefMgr();
   iString *id_str=NULL;
  
   pm->GetRect(settings, "Frame", frame);
   //pm->GetString(settings, "Id", id_str);

   if (id_str!=NULL) id = pm->NameToId(id_str->GetData());
   
   printf("aws-debug: Frame is: (%d,%d)-(%d,%d)\n", frame.xmin, frame.ymin, frame.xmax, frame.ymax);
   

   // Children are automatically filled in by the windowmanager.
   
  }

  return true;

}

void
awsComponent::Invalidate()
{
  WindowManager()->Mark(frame);
}

void 
awsComponent::Invalidate(csRect area)
{
  WindowManager()->Mark(area);
}

bool 
awsComponent::HandleEvent(iEvent& Event)
{
 
  switch(Event.Type)
  {
  case csevMouseMove:
    return OnMouseMove(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseUp:
    return OnMouseUp(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseDown:
    return OnMouseDown(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseClick:
    return OnMouseClick(Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

  case csevMouseEnter:
    return OnMouseEnter();

  case csevMouseExit:
    return OnMouseExit();

  case csevKeyDown:
    return OnKeypress(Event.Key.Char, Event.Key.Modifiers);
    
  }

  return false;
}

bool 
awsComponent::Overlaps(csRect &r)
{
  return frame.Intersects(r);
}

void 
awsComponent::AddChild(awsComponent *child, bool owner)
{
  (void)owner;
  /* @@@: we cannot incref for non-owned only if we generally decrefing upon destruction
          We either incref them all or we store an additional mark for owned children
   // Only grab a reference if we are not the owner.
   if (owner==false)
     child->IncRef();
  */
   // Create a new child list if the current one does not exist.
   if (children==NULL)
     children = new csDLinkList();
   
   children->AddItem(child);
   
   // Modify the child's rectangle to be inside and relative to the parent's rectangle.
   child->frame.Move(frame.xmin, frame.ymin);
}

void 
awsComponent::RemoveChild(awsComponent *child)
{
   if (children)
     if (children->SetCurrentItem (child))
     {
       children->RemoveItem();
       child->DecRef ();
     }
}

awsComponent *
awsComponent::GetFirstChild()
{
   if (children)
     return (awsComponent *)children->GetFirstItem();

   return NULL;
}

    
awsComponent *
awsComponent::GetNextChild()
{
   if (children)
    return (awsComponent *)children->GetNextItem();

  return NULL;
}


void 
awsComponent::Hide()
{
  if (hidden) return;
  else 
  {
    hidden=true;
    WindowManager()->Mark(Frame());
  }
}

void 
awsComponent::Show()
{
  if (!hidden) return;
  else 
  {
    hidden=false;
    WindowManager()->Mark(Frame());
  }
}

/////////////////////////////////////  awsComponentFactory ////////////////////////////////////////////////////////

/**
  *  A factory is simply a class that knows how to build your component.  Although components aren't required to have
  * a factory, they will not be able to be instantiated through the template functions and window definitions if they
  * don't.  In any case, a factory is remarkably simple to build.  All you need to do is to inherit from 
  * awsComponentFactory and call register with the window manager and the named type of the component.  That's it.
  */
awsComponentFactory::awsComponentFactory(iAws *wmgr)
{
   // This is where you call register, only you must do it in the derived factory.  Like this:
   // Register(wmgr, "Radio Button");
}

awsComponentFactory::~awsComponentFactory()
{
   // Do nothing.
}

void
awsComponentFactory::Register(iAws *wmgr, char *name)
{
    wmgr->RegisterComponentFactory(this, name);
}



