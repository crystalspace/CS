#include "cssysdef.h"
#include "aws/awsprefs.h"
#include "aws/awscomp.h"
#include "aws/awsslot.h"
#include "aws/awsfparm.h"
#include "iutil/event.h"
#include "csutil/scfstr.h"

#include <stdio.h>
#include <string.h>

const bool AWS_COMP_DEBUG=false;


awsComponent::awsComponent():wmgr(NULL), win(NULL), parent(NULL), children(NULL), signalsrc(this)
{
  hidden = true; // initially set it hidden to cause the marking of a dirty rectangle upon first Show ()
}

awsComponent::~awsComponent()
{
   /// Let go our references to any children if we have them.
   if (children)
   {
      void *item;
      int   i;

      for(i=0; i<GetChildCount(); ++i)
      {
        item = GetChildAt(i);
          
        awsComponent *cmp = (awsComponent *)item;

        cmp->DecRef();
      }

      delete children;
   }
}

csRect& 
awsComponent::Frame()
{ return frame; }

char *
awsComponent::Type()
{ return "Component"; }

bool 
awsComponent::isHidden()
{ return hidden; }

unsigned long 
awsComponent::GetID()
{ return id; }

void 
awsComponent::SetID(unsigned long _id)
{ id = _id; }

bool 
awsComponent::HasChildren()
{ return children!=NULL; }
   
iAws *
awsComponent::WindowManager()
{ return wmgr; }

iAwsWindow *
awsComponent::Window()
{ return win; }

iAwsComponent *
awsComponent::Parent()
{ return parent; }

void 
awsComponent::SetWindow(iAwsWindow *_win)
{ win = _win; }

void 
awsComponent::SetParent(iAwsComponent *_parent)
{ parent = _parent; }

iAwsComponent *
awsComponent::GetComponent()
{ return this; }

    
/**
 *  This function is normally called automatically by the window manager.  You may call it manually if you wish, but
 * there's little reason to do so.  
 **************************************************************************************************************/
bool 
awsComponent::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  if (wmgr) return false;

  wmgr = _wmgr;
  
  if (AWS_COMP_DEBUG)
    printf("aws-debug: setting up awsComponent (%s).\n", Type());
  
  if (settings) 
  {
  
   iAwsPrefManager *pm=WindowManager()->GetPrefMgr();
     
   pm->GetRect(settings, "Frame", frame);
   
   if (AWS_COMP_DEBUG)
     printf("aws-debug: Frame is: (%d,%d)-(%d,%d)\n", frame.xmin, frame.ymin, frame.xmax, frame.ymax);
   
   // Children are automatically filled in by the windowmanager.
   
  }

  return true;

}

bool 
awsComponent::GetProperty(char *name, void **parm)
{
  if (strcmp("Frame", name)==0)
  {
    csRect *r = new csRect(Frame());
    *parm = (void *)r;
    return true;
  }
  else if (strcmp("Type", name)==0)
  {
    iString *s = new scfString(Type());
    *parm = (void *)s;
    return true;
  }

  return false;
}

bool 
awsComponent::SetProperty(char *name, void *parm)
{  
  if (strcmp("Frame", name)==0)
  {
    csRect *r = (csRect *)(parm);

    Frame().Set(*r);
    return true;
  }
  
  return false;
}

bool 
awsComponent::Execute(char *action, awsParmList &parmlist)
{
  if (strcmp("MoveTo", action)==0)
  {

  }
  else if (strcmp("Hide", action)==0)
  {
    Hide();
    return true;
  }
  else if (strcmp("Show", action)==0)
  {
    Show();
    return true;
  }
  else if (strcmp("Invalidate", action)==0)
  {
    Invalidate();
    return true;
  }
  else if (strcmp("Overlaps", action)==0)
  {
    csRect *r;
    if (parmlist.GetRect("Rect", &r))
    {
      bool result= Overlaps(*r);
      parmlist.AddBool("Result", result); 
    }
    return true;
  } 

  return false;
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
awsComponent::AddChild(iAwsComponent *child, bool owner)
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
     children = new csBasicVector();
   
   children->Push(child);
   
   // Modify the child's rectangle to be inside and relative to the parent's rectangle.
   child->Frame().Move(Frame().xmin, Frame().ymin);
}

void 
awsComponent::RemoveChild(iAwsComponent *child)
{
   int i;

   if (children)
     if ((i=children->Find(child))!=-1)
     {
       children->Delete(i);
       child->DecRef ();
     }
}

int
awsComponent::GetChildCount()
{
  if (children)
    return children->Length();
  else 
    return 0;
}

iAwsComponent *
awsComponent::GetChildAt(int i)
{
  if (children)
    return (iAwsComponent *)((*children)[i]);
  else
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

void 
awsComponent::MoveChildren(int delta_x, int delta_y)
{
  // If we have no children, go away.
  if (!HasChildren()) return;

  int i;
  for(i=0; i<GetChildCount(); ++i)
  {
    iAwsComponent *child = GetChildAt(i);
    
    if (child->HasChildren())
      child->MoveChildren(delta_x, delta_y);

    child->Frame().Move(delta_x, delta_y);
  }

}

bool 
awsComponent::RegisterSlot(iAwsSlot *slot, unsigned long signal)
{
  return signalsrc.RegisterSlot(slot, signal);
}

bool 
awsComponent::UnregisterSlot(iAwsSlot *slot, unsigned long signal)
{
  return signalsrc.UnregisterSlot(slot, signal);
} 

void 
awsComponent::Broadcast(unsigned long signal)
{
  signalsrc.Broadcast(signal);
}

void awsComponent::OnDraw(csRect clip)                           { return; }
bool awsComponent::OnMouseDown(int button, int x, int y)         { return false; }
bool awsComponent::OnMouseUp(int button, int x, int y)           { return false; }
bool awsComponent::OnMouseMove(int button, int x, int y)         { return false; }
bool awsComponent::OnMouseClick(int button, int x, int y)        { return false; }
bool awsComponent::OnMouseDoubleClick(int button, int x, int y)  { return false; }
bool awsComponent::OnMouseExit()                                 { return false; }
bool awsComponent::OnMouseEnter()                                { return false; }
bool awsComponent::OnKeypress(int key, int modifiers)            { return false; }
bool awsComponent::OnLostFocus()                                 { return false; }
bool awsComponent::OnGainFocus()                                 { return false; }

/////////////////////////////////////  awsComponentFactory ////////////////////////////////////////////////////////

/**
  *  A factory is simply a class that knows how to build your component.  Although components aren't required to have
  * a factory, they will not be able to be instantiated through the template functions and window definitions if they
  * don't.  In any case, a factory is remarkably simple to build.  All you need to do is to inherit from 
  * awsComponentFactory and call register with the window manager and the named type of the component.  That's it.
  */
awsComponentFactory::awsComponentFactory(iAws *_wmgr)
{
   // This is where you call register, only you must do it in the derived factory.  Like this:
   // Register(wmgr, "Radio Button");

   wmgr=_wmgr;
}

awsComponentFactory::~awsComponentFactory()
{
   // Do nothing.
}

void
awsComponentFactory::Register(char *name)
{
  wmgr->RegisterComponentFactory(this, name);
}

void 
awsComponentFactory::RegisterConstant(char *name, int value)
{
  wmgr->GetPrefMgr()->RegisterConstant(name, value);
}


