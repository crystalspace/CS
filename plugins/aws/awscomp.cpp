#include "cssysdef.h"
#include "awsprefs.h"
#include "awscomp.h"
#include "awsslot.h"
#include "awsfparm.h"
#include "awslayot.h"
#include "iutil/event.h"
#include "csutil/scfstr.h"
#include "iaws/awsdefs.h"
#include "awsgbl.h"
#include "awsbl.h"

#include <stdio.h>
#include <string.h>

const bool AWS_COMP_DEBUG = false;

awsComponent::awsComponent () :
  wmgr(NULL),
  win(NULL),
  parent(NULL),
  layout(NULL),
  children(NULL),
  flags(0),
  signalsrc(this)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

awsComponent::~awsComponent ()
{
  /// Let go our references to any children if we have them.
  if (children)
  {
    void *item;
    int i;

    for (i = 0; i < GetChildCount (); ++i)
    {
      item = GetChildAt (i);

      awsComponent *cmp = (awsComponent *)item;

      cmp->DecRef ();
    }

    delete children;
  }
}

csRect &awsComponent::Frame ()
{
  return frame;
}

char *awsComponent::Type ()
{
  return "Component";
}

bool awsComponent::isHidden ()
{
  return Flags () & AWSF_CMP_HIDDEN;
}

void awsComponent::SetFlag (unsigned int flag)
{
  flags |= flag;
}

void awsComponent::ClearFlag (unsigned int flag)
{
  flags &= (~flag);
}

unsigned int awsComponent::Flags ()
{
  return flags;
}

unsigned long awsComponent::GetID ()
{
  return id;
}

void awsComponent::SetID (unsigned long _id)
{
  id = _id;
}

bool awsComponent::HasChildren ()
{
  return children != NULL;
}

iAws *awsComponent::WindowManager ()
{
  return wmgr;
}

iAwsWindow *awsComponent::Window ()
{
  return win;
}

iAwsComponent *awsComponent::Parent ()
{
  return parent;
}

awsLayoutManager *awsComponent::Layout ()
{
  return layout;
}

void awsComponent::SetWindow (iAwsWindow *_win)
{
  win = _win;
}

void awsComponent::SetParent (iAwsComponent *_parent)
{
  parent = _parent;
}

void awsComponent::SetLayout (awsLayoutManager *l)
{
  layout = l;
}

iAwsComponent *awsComponent::GetComponent ()
{
  return this;
}

/**
 *  This function is normally called automatically by the window manager.  You may call it manually if you wish, but
 * there's little reason to do so.
 **************************************************************************************************************/
bool awsComponent::Setup (iAws *_wmgr, awsComponentNode *settings)
{
  if (wmgr) return false;

  wmgr = _wmgr;

  if (AWS_COMP_DEBUG)
    printf ("aws-debug: setting up awsComponent (%s).\n", Type ());

  if (settings)
  {
    iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

    pm->GetRect (settings, "Frame", frame);

    if (AWS_COMP_DEBUG)
      printf (
        "aws-debug: Frame is: (%d,%d)-(%d,%d)\n",
        frame.xmin,
        frame.ymin,
        frame.xmax,
        frame.ymax);

    // Children are automatically filled in by the windowmanager.

    // Do layout check
    iString *ln = NULL;

    pm->GetString (settings, "Layout", ln);

    if (ln)
    {
      if (strcmp ("GridBag", ln->GetData ()) == 0)
        layout = new awsGridBagLayout (this);
      else if (strcmp ("Border", ln->GetData ()) == 0)
        layout = new awsBorderLayout (this, pm, settings);
    }
  }

  return true;
}

bool awsComponent::GetProperty (char *name, void **parm)
{
  if (strcmp ("Frame", name) == 0)
  {
    csRect *r = new csRect (Frame ());
    *parm = (void *)r;
    return true;
  }
  else if (strcmp ("Type", name) == 0)
  {
    iString *s = new scfString (Type ());
    *parm = (void *)s;
    return true;
  }

  return false;
}

bool awsComponent::SetProperty (char *name, void *parm)
{
  if (strcmp ("Frame", name) == 0)
  {
    csRect *r = (csRect *) (parm);

    Frame ().Set (*r);
    return true;
  }

  return false;
}

bool awsComponent::Execute (char *action, iAwsParmList &parmlist)
{
  if (strcmp ("MoveTo", action) == 0)
  {
  }
  else if (strcmp ("Hide", action) == 0)
  {
    Hide ();
    return true;
  }
  else if (strcmp ("Show", action) == 0)
  {
    Show ();
    return true;
  }
  else if (strcmp ("Invalidate", action) == 0)
  {
    Invalidate ();
    return true;
  }
  else if (strcmp ("HideWindow", action) == 0)
  {
    if (Window ())
    {
      Window ()->Hide ();
      WindowManager ()->InvalidateUpdateStore ();
    }

    return true;
  }
  else if (strcmp ("Overlaps", action) == 0)
  {
    csRect *r;
    if (parmlist.GetRect ("Rect", &r))
    {
      bool result = Overlaps (*r);
      parmlist.AddBool ("Result", result);
    }

    return true;
  }

  return false;
}

void awsComponent::Invalidate ()
{
  WindowManager ()->Mark (frame);
}

void awsComponent::Invalidate (csRect area)
{
  WindowManager ()->Mark (area);
}

bool awsComponent::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseMove:
      return OnMouseMove (Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

    case csevMouseUp:
      return OnMouseUp (Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

    case csevMouseDown:
      return OnMouseDown (Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

    case csevMouseClick:
      return OnMouseClick (Event.Mouse.Button, Event.Mouse.x, Event.Mouse.y);

    case csevMouseEnter:
      return OnMouseEnter ();

    case csevMouseExit:
      return OnMouseExit ();

    case csevKeyDown:
      return OnKeypress (Event.Key.Char, Event.Key.Modifiers);

    case csevGainFocus:
      return OnGainFocus ();

    case csevLostFocus:
      return OnLostFocus ();

    case csevFrameStart:
      return OnFrame ();
  }

  return false;
}

bool awsComponent::Overlaps (csRect &r)
{
  return frame.Intersects (r);
}

csRect awsComponent::getPreferredSize ()
{
  return getMinimumSize ();
}

csRect awsComponent::getMinimumSize ()
{
  return csRect (0, 0, 30, 15);
}

csRect awsComponent::getInsets ()
{
  return csRect (0, 0, 0, 0);
}

void awsComponent::AddChild (iAwsComponent *child, bool has_layout)
{
  // Create a new child list if the current one does not exist.
  if (children == NULL) children = new csBasicVector ();

  children->Push (child);

  if (!has_layout)
  {
    // Modify the child's rectangle to be inside and relative to the parent's rectangle.
    child->Frame ().Move (Frame ().xmin, Frame ().ymin);
  }

  // Fire off the event so that the child can do something if it needs to.
  child->OnAdded ();
}

void awsComponent::RemoveChild (iAwsComponent *child)
{
  int i;

  if (children)
    if ((i = children->Find (child)) != -1)
    {
      children->Delete (i);
      child->DecRef ();
    }
}

int awsComponent::GetChildCount ()
{
  if (children)
    return children->Length ();
  else
    return 0;
}

iAwsComponent *awsComponent::GetChildAt (int i)
{
  if (children)
    return (iAwsComponent *) ((*children)[i]);
  else
    return NULL;
}

void awsComponent::Hide ()
{
  if (Flags () & AWSF_CMP_HIDDEN)
    return ;
  else
  {
    SetFlag (AWSF_CMP_HIDDEN);
    WindowManager ()->Mark (Frame ());
  }
}

void awsComponent::Show ()
{
  if (!(Flags () & AWSF_CMP_HIDDEN))
    return ;
  else
  {
    ClearFlag (AWSF_CMP_HIDDEN);
    WindowManager ()->Mark (Frame ());
  }
}

void awsComponent::MoveChildren (int delta_x, int delta_y)
{
  // If we have no children, go away.
  if (!HasChildren ()) return ;

  int i;
  for (i = 0; i < GetChildCount (); ++i)
  {
    iAwsComponent *child = GetChildAt (i);

    if (child->HasChildren ()) child->MoveChildren (delta_x, delta_y);

    child->Frame ().Move (delta_x, delta_y);
  }
}

void awsComponent::ResizeChildren ()
{
  if (Layout ()) Layout ()->LayoutComponents ();
}

bool awsComponent::RegisterSlot (iAwsSlot *slot, unsigned long signal)
{
  return signalsrc.RegisterSlot (slot, signal);
}

bool awsComponent::UnregisterSlot (iAwsSlot *slot, unsigned long signal)
{
  return signalsrc.UnregisterSlot (slot, signal);
}

void awsComponent::Broadcast (unsigned long signal)
{
  signalsrc.Broadcast (signal);
}

void awsComponent::OnDraw (csRect)
{
  return ;
}

bool awsComponent::OnMouseDown (int, int, int)
{
  return false;
}

bool awsComponent::OnMouseUp (int, int, int)
{
  return false;
}

bool awsComponent::OnMouseMove (int, int, int)
{
  return false;
}

bool awsComponent::OnMouseClick (int, int, int)
{
  return false;
}

bool awsComponent::OnMouseDoubleClick (int, int, int)
{
  return false;
}

bool awsComponent::OnMouseExit ()
{
  return false;
}

bool awsComponent::OnMouseEnter ()
{
  return false;
}

bool awsComponent::OnKeypress (int, int)
{
  return false;
}

bool awsComponent::OnLostFocus ()
{
  return false;
}

bool awsComponent::OnGainFocus ()
{
  return false;
}

bool awsComponent::OnFrame ()
{
  return false;
}

void awsComponent::OnAdded ()
{
  return ;
}

void awsComponent::OnResized ()
{
  return ;
}

/////////////////////////////////////  awsComponentFactory ////////////////////////////////////////////////////////

/**
  *  A factory is simply a class that knows how to build your component.  Although components aren't required to have
  * a factory, they will not be able to be instantiated through the template functions and window definitions if they
  * don't.  In any case, a factory is remarkably simple to build.  All you need to do is to inherit from
  * awsComponentFactory and call register with the window manager and the named type of the component.  That's it.
  */

SCF_IMPLEMENT_IBASE (awsComponentFactory)
  SCF_IMPLEMENTS_INTERFACE (iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsComponentFactory::awsComponentFactory (iAws *_wmgr)
{
  SCF_CONSTRUCT_IBASE (NULL);
  // This is where you call register, only you must do it in the derived factory.  Like this:

  // Register(wmgr, "Radio Button");
  wmgr = _wmgr;
}

awsComponentFactory::~awsComponentFactory ()
{
  // Do nothing.
}

iAwsComponent *awsComponentFactory::Create ()
{
  return NULL;
}

void awsComponentFactory::Register (char *name)
{
  wmgr->RegisterComponentFactory (this, name);
}

void awsComponentFactory::RegisterConstant (char *name, int value)
{
  wmgr->GetPrefMgr ()->RegisterConstant (name, value);
}
