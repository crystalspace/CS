#include "cssysdef.h"
#include "awsprefs.h"
#include "awscomp.h"
#include "awsslot.h"
#include "awsfparm.h"
#include "awslayot.h"
#include "iutil/event.h"
#include "csutil/scfstr.h"
#include "iaws/awsdefs.h"
#include "ivideo/graph2d.h"
#include "awsgbl.h"
#include "awsbl.h"

#include <stdio.h>
#include <string.h>

const bool AWS_COMP_DEBUG = false;

awsComponent::awsComponent () :
  wmgr(NULL),
  parent(NULL),
  top_child(NULL),
  below(NULL),
  above(NULL),
  layout(NULL),
  is_zoomed(false),
  flags(0),
  signalsrc(this)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

awsComponent::~awsComponent ()
{
  /// Let go our references to any children if we have them.
  iAwsComponent* child = GetTopChild();
  iAwsComponent* next;
  while(child)
  {
	next = child->ComponentBelow();
	child->Unlink(); 
	child->DecRef();
	child = next;
  }

}

csRect awsComponent::Frame ()
{
  return frame;
}

csRect awsComponent::ClientFrame ()
{
  csRect insets = getInsets();
  csRect client;
  client.xmin = Frame().xmin + insets.xmin;
  client.ymin = Frame().ymin + insets.ymin;
  client.xmax = Frame().xmax - insets.xmax;
  client.ymax = Frame().ymax - insets.ymax;
  return client;
}

const char *awsComponent::Type ()
{
  return "Component";
}

bool awsComponent::IsMaximized()
{
  return is_zoomed;
}

bool awsComponent::isHidden ()
{
  return Flags () & AWSF_CMP_HIDDEN;
}

bool awsComponent::isDeaf ()
{
  return Flags () & AWSF_CMP_DEAF;
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
  return top_child != NULL;
}

iAws *awsComponent::WindowManager ()
{
  return wmgr;
}

iAwsComponent *awsComponent::Parent ()
{
  return parent;
}

iAwsLayoutManager *awsComponent::Layout ()
{
  return layout;
}

void awsComponent::SetParent (iAwsComponent *_parent)
{
  parent = _parent;
}

void awsComponent::SetLayout (iAwsLayoutManager *l)
{
  layout = l;
}

iAwsComponent *awsComponent::GetComponent ()
{
  return this;
}

bool awsComponent::Create(iAws* wmgr, iAwsComponent* parent, 
                          iAwsComponentNode* settings)
{
  SetID(settings->Name());
  SetParent(parent);

  // set ourself up by querying the settings 
  if(!Setup(wmgr, settings)) return false;

  // if we are a top-level component link in to the top-level list
  if(Parent() == NULL)
  {
    // Link into the current hierarchy, at the top.
    if (wmgr->GetTopComponent () == NULL)
    {
      wmgr->SetTopComponent (this);
    }
    else
    {
      LinkAbove (wmgr->GetTopComponent ());
      wmgr->SetTopComponent (this);
    }
  }

  // unless you have set the non client flag by this point 
  // you get added to the parent's layout
  if(~Flags() & AWSF_CMP_NON_CLIENT && Parent() && Parent()->Layout())
    Parent()->Layout()->AddComponent(this, settings);

  if(Parent())
    Parent()->AddChild(this);
  return true;
}

/**
 *  This function is normally called automatically by Create.  You may call it manually if you wish, but
 * there's little reason to do so.
 **************************************************************************************************************/
bool awsComponent::Setup (iAws *_wmgr, iAwsComponentNode *settings)
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
      {
        awsGridBagLayout* temp = new awsGridBagLayout (this, settings, pm);
        layout = SCF_QUERY_INTERFACE(temp, iAwsLayoutManager);
        temp->DecRef();
      }
      else if (strcmp ("Border", ln->GetData ()) == 0)
      {
        awsBorderLayout* temp = new awsBorderLayout (this, settings, pm);
        layout = SCF_QUERY_INTERFACE(temp, iAwsLayoutManager);
        temp->DecRef();
      }
    }
  }

  return true;
}

bool awsComponent::GetProperty (const char *name, void **parm)
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

bool awsComponent::SetProperty (const char *name, void *parm)
{
  if (strcmp ("Frame", name) == 0)
  {
    csRect *r = (csRect *) (parm);
    ResizeTo(*r);
    return true;
  }

  return false;
}

bool awsComponent::Execute (const char* action, iAwsParmList* parmlist)
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
      Window ()->GetComponent()->Hide ();
      WindowManager ()->InvalidateUpdateStore ();
    }

    return true;
  }
  else if (strcmp ("Overlaps", action) == 0)
  {
    if (!parmlist)
      return false;

    csRect *r;
    if (parmlist->GetRect ("Rect", &r))
    {
      bool result = Overlaps (*r);
      parmlist->AddBool ("Result", result);
    }

    return true;
  }

  return false;
}

void awsComponent::Invalidate ()
{
  Invalidate(frame);
}

void awsComponent::Invalidate (csRect area)
{
  if(WindowManager())
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

void awsComponent::AddChild (iAwsComponent *child)
{
  child->IncRef();

  // Create a new child list if the current one does not exist.
  if (top_child)
      child->LinkAbove(top_child);
  top_child = child;

  if (!Layout())
  {
    // Modify the child's rectangle to be inside and relative to the parent's rectangle.
    if(child->Flags() & AWSF_CMP_NON_CLIENT)
      child->Move(Frame().xmin, Frame().ymin);
    else
      child->Move (ClientFrame ().xmin, ClientFrame ().ymin);
  }

  // Fire off the event so that the child can do something if it needs to.
  child->OnAdded ();
}

void awsComponent::SetTopChild(iAwsComponent* child)
{
	top_child = child;
}

iAwsComponent* awsComponent::GetTopChild()
{
	return top_child;
}

void awsComponent::RemoveChild (iAwsComponent *child)
{
   if(child == top_child)
	   top_child = child->ComponentBelow();
   child->Unlink();

   child->DecRef();
}

int awsComponent::GetChildCount ()
{
  int count = 0;
  iAwsComponent* cur = GetTopChild();
  while(cur)
  {
	  count++;
	  cur = cur->ComponentBelow();
  }
  return count;
}

iAwsComponent *awsComponent::FindChild(const char* name)
{
  unsigned id = WindowManager()->GetPrefMgr()->NameToId(name);

  return DoFindChild(id);
}

iAwsComponent *awsComponent::DoFindChild(unsigned id)
{
  if (!HasChildren ()) return NULL;
  iAwsComponent* result;

  for (iAwsComponent *child = GetTopChild(); child ; child = child->ComponentBelow())
  {

    // if this child matches, good.
    if (child->GetID() == id)
      return child;

    // otherwise, check this child
    if ((result = child->DoFindChild(id))!=NULL)
      return result;
  }

  return NULL;
}

iAwsComponent *awsComponent::ChildAt(int x, int y)
{
  // if the point is not inside the client area then return NULL
  if(!Frame().Contains(x,y))
	  return NULL;

  for(iAwsComponent* cmp = GetTopChild(); cmp; cmp = cmp->ComponentBelow())
  {
	 if(cmp->isHidden()) continue;
	 if(!cmp->Frame().Contains(x,y)) continue;
	 if(cmp->Flags() & AWSF_CMP_NON_CLIENT)
		 return cmp;
	 else if(ClientFrame().Contains(x,y))
		 return cmp;
  }

  return NULL;
}

void awsComponent::Hide ()
{
  if (Flags () & AWSF_CMP_HIDDEN)
    return ;
  else
  {
    SetFlag (AWSF_CMP_HIDDEN);
    Invalidate();
  }
  if(!Parent()) WindowManager()->InvalidateUpdateStore();
  else Parent()->OnChildHide();
}

void awsComponent::Show ()
{
  if (!(Flags () & AWSF_CMP_HIDDEN))
    return ;
  else
  {
    ClearFlag (AWSF_CMP_HIDDEN);
    Invalidate();
  }
  if(!Parent()) WindowManager()->InvalidateUpdateStore();
  else Parent()->OnChildShow();
}

void awsComponent::SetDeaf (bool bDeaf)
{
  if ((Flags () & AWSF_CMP_DEAF) ^ bDeaf)
    if (bDeaf)
      SetFlag (AWSF_CMP_DEAF);
    else
      ClearFlag (AWSF_CMP_DEAF);
}

void awsComponent::Move(int delta_x, int delta_y)
{
  // remove frivilous calls
  if(delta_x == 0 && delta_y == 0) return; 

  csRect dirty1 (Frame ());

  Invalidate();
  frame.Move(delta_x, delta_y);
  Invalidate();

  MoveChildren(delta_x, delta_y);

  if(Parent()) Parent()->OnChildMoved();
  else
  {
    if (WindowManager ()->GetFlags () & AWSF_AlwaysEraseWindows)
      WindowManager ()->Erase (dirty1);
    WindowManager()->InvalidateUpdateStore();
  }
}

void awsComponent::MoveChildren (int delta_x, int delta_y)
{
  iAwsComponent* child = GetTopChild();
  while(child)
  {
    child->Move (delta_x, delta_y);
	child = child->ComponentBelow();
  }
}

void awsComponent::Resize(int w, int h)
{
  // remove frivilous calls
  if(w == Frame().Width() && h == Frame().Height())
    return;

  if( (!Parent()) &&
    (w < Frame ().Width() || h < Frame ().Height()))
  {
    if (WindowManager ()->GetFlags () & AWSF_AlwaysEraseWindows)
    {
      csRect f(Frame());
      f.xmax++;
      f.ymax++;
      WindowManager ()->Erase (f);
    }
  }

  Invalidate();
	frame.SetSize(w,h);
  Invalidate();

	LayoutChildren();
	
	if(Parent()) Parent()->OnChildMoved();
	else if(WindowManager())
    WindowManager()->InvalidateUpdateStore();

	OnResized();
}

void awsComponent::MoveTo(int x, int y)
{
	Move(x-Frame().xmin, y-Frame().ymin);
}

void awsComponent::ResizeTo(csRect newFrame)
{

	MoveTo(newFrame.xmin, newFrame.ymin);
	Resize(newFrame.Width(), newFrame.Height());
}

void awsComponent::LayoutChildren ()
{
  if (Layout ()) Layout ()->LayoutComponents ();
}



void awsComponent::AddToLayout(iAwsComponent* cmp, iAwsComponentNode* settings)
{
   if (Layout())
     Layout()->AddComponent(cmp, settings);
}


iAwsComponent* awsComponent::Window()
{
	iAwsComponent* cur = this;
	while(cur->Parent() && !(cur->Flags() & AWSF_CMP_WINDOW))
		cur = cur->Parent();
	return cur;
}


void awsComponent::LinkAbove (iAwsComponent *comp)
{
  if (comp)
  {
    above = comp->ComponentAbove ();
	below = comp;
    comp->SetComponentAbove (this);
    if(above) above->SetComponentBelow(this);
  }

  CS_ASSERT(LinkedListCheck());
}

void awsComponent::LinkBelow (iAwsComponent *comp)
{
  if (comp)
  {
    above = comp;
    below = comp->ComponentBelow ();
    comp->SetComponentBelow (this);
	if(below) below->SetComponentAbove(this);
  }

  CS_ASSERT(LinkedListCheck());
}

void awsComponent::Unlink()
{
  if(ComponentAbove()) 
	  ComponentAbove()->SetComponentBelow(ComponentBelow());
  if(ComponentBelow())
	  ComponentBelow()->SetComponentAbove(ComponentAbove());

  CS_ASSERT(LinkedListCheck());
}

iAwsComponent* awsComponent::ComponentAbove()
{
	return above;
}

iAwsComponent* awsComponent::ComponentBelow()
{
	return below;
}

void awsComponent::SetComponentAbove(iAwsComponent* comp)
{
	above = comp;
  CS_ASSERT(LinkedListCheck());
}

void awsComponent::SetComponentBelow(iAwsComponent* comp)
{
	below = comp;
  CS_ASSERT(LinkedListCheck());
}

void awsComponent::SetAbove(iAwsComponent* comp)
{
	
  // Get us out of the hierarchy
  Unlink();

  // go back in the hierarchy at the top
  LinkAbove(comp);

  // child components tell there parent window to raise them
  if(Parent())
  {
	// if we just took the head position fix up the head pointer
	if(Parent()->GetTopChild() == comp)
	  Parent()->SetTopChild(this);
  }
  else
  {
	if(WindowManager()->GetTopComponent() == comp)
		WindowManager()->SetTopComponent(this);
  }
  // make sure we get redrawn now
  Invalidate();
  
  return;
}

void awsComponent::SetBelow(iAwsComponent* comp)
{

	// Get us out of the hierarchy
	Unlink();

	// go back in the hierarchy at the top
	LinkBelow(comp);

	// make sure we get redrawn now
	Invalidate();

	return;
}

void awsComponent::Raise()
{
  if(Parent())
  {
    if(Parent()->GetTopChild() != this)
	{
	  OnRaise();
      SetAbove(Parent()->GetTopChild());
	}
  }
  else
  {
	if(WindowManager()->GetTopComponent() != this)
	{
	  OnRaise();
	  SetAbove(WindowManager()->GetTopComponent());
	}
  }
}

void awsComponent::Lower()
{
  iAwsComponent* temp;
  if(Parent())
    temp = Parent()->GetTopChild();
  else
    temp = WindowManager()->GetTopComponent();

  while(temp->ComponentBelow())
    temp = temp->ComponentBelow();

  if(temp != this)
  {
	OnLower();
    SetBelow(temp);
  }
}

unsigned int awsComponent::RedrawTag()
{
  return redraw_tag;
}

void awsComponent::SetRedrawTag(unsigned int tag)
{
  redraw_tag = tag;
}

void awsComponent::Maximize()
{
  if (!is_zoomed)
  {
      is_zoomed = true;
      unzoomed_frame.Set (Frame ());

	  if(!Parent())
	  {
        Move(-Frame().xmin, -Frame().ymin);
		Resize(WindowManager ()->G2D()->GetWidth()-1,
			   WindowManager()->G2D ()->GetHeight()-1);
	  }
	  else
	  {
		Move(Parent()->ClientFrame().xmin - Frame().xmin,
			 Parent()->ClientFrame().ymin - Frame().ymin);
		Resize(Parent()->ClientFrame().Width(),
			   Parent()->ClientFrame().Height());
	  }
  }
}

void awsComponent::UnMaximize()
{
if (is_zoomed)
    {
	   is_zoomed = false;
	   Move(unzoomed_frame.xmin - Frame().xmin, 
		    unzoomed_frame.ymin - Frame().ymin);
	   Resize(unzoomed_frame.Width(), unzoomed_frame.Height());
    }
	
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
  if (AWS_COMP_DEBUG) printf("aws-debug: mouse down  : %s\n", Type());
  return false;
}

bool awsComponent::OnMouseUp (int, int, int)
{
  if (AWS_COMP_DEBUG) printf("aws-debug: mouse up    : %s\n", Type());
  return false;
}

bool awsComponent::OnMouseMove (int, int, int)
{
  return false;
}

bool awsComponent::OnMouseClick (int, int, int)
{
  if (AWS_COMP_DEBUG) printf("aws-debug: mouse click : %s\n", Type());
  return false;
}

bool awsComponent::OnMouseDoubleClick (int, int, int)
{
  return false;
}

bool awsComponent::OnMouseExit ()
{
  if (AWS_COMP_DEBUG) printf("aws-debug: mouse exit  : %s\n", Type());
  return false;
}

bool awsComponent::OnMouseEnter ()
{
  if (AWS_COMP_DEBUG) printf("aws-debug: mouse enter : %s\n", Type());
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

void awsComponent::OnChildMoved ()
{
  return ;
}

void awsComponent::OnRaise()
{
  return ;
}

void awsComponent::OnLower()
{
  return ;
}

void awsComponent::OnChildHide()
{
  return ;
}

void awsComponent::OnChildShow()
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

  wmgr = _wmgr;
}

awsComponentFactory::~awsComponentFactory ()
{
  // Do nothing.
}

iAwsComponent *awsComponentFactory::Create ()
{
  return new awsComponent;
}

void awsComponentFactory::Register (const char *name)
{
  wmgr->RegisterComponentFactory (this, name);
}

void awsComponentFactory::RegisterConstant (const char *name, int value)
{
  wmgr->GetPrefMgr ()->RegisterConstant (name, value);
}


bool awsComponent::LinkedListCheck()
{
  iAwsComponent* cmp;
  for(cmp = ComponentBelow(); cmp; cmp = cmp->ComponentBelow())
    if(cmp == this) return false;
  for(cmp = ComponentAbove(); cmp; cmp = cmp->ComponentAbove())
    if(cmp == this) return false;
  for(cmp = Parent(); cmp; cmp = cmp->Parent())
    if(cmp == this) return false;

  return true;
}


//////////////////////////////////// awsComponentVector ///////////////////////////////
  
iAwsComponent *awsComponentVector::Get(int idx) const 
{
    return (iAwsComponent*)csVector::Get (idx);
}
int awsComponentVector::Push (iAwsComponent* comp) 
{ 
    return csVector::Push ((csSome)comp);
}
