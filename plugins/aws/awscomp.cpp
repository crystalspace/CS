/*
    Copyright (C) 2000-2001 by Christopher Nelson.

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

#include "cssysdef.h"
#include <stdio.h>
#include <string.h>

#include "awsprefs.h"
#include "awscomp.h"
#include "awsslot.h"
#include "awsfparm.h"
#include "awslayot.h"
#include "iutil/event.h"
#include "csutil/scfstr.h"
#include "iaws/awsdefs.h"
#include "ivideo/graph2d.h"
#include "csutil/event.h"
#include "awsgbl.h"
#include "awsbl.h"

//#define AWS_COMP_DEBUG

awsComponent::awsComponent ()
  : wmgr (0),
    parent (0),
    top_child (0),
    below (0),
    above (0),
    is_zoomed (false),
    flags (0),
    signalsrc (),
    redraw_tag (0),
    focusable (false),
    self(0),
    _destructionMark( false ),
	CompType(false),
	CompFrame(frame)
{
  self = this;
  signalsrc.SetOwner (self);
  SCF_CONSTRUCT_IBASE (0);
  set_preferred_size = false;
}

awsComponent::awsComponent (iAwsComponent* wrapper)
  : wmgr (0),
    parent (0),
    top_child (0),
    below (0),
    above (0),
    is_zoomed (false),
    flags (0),
    signalsrc (),
    redraw_tag (0),
    focusable (false),
    self(wrapper),
    _destructionMark( false ),
	CompType(false),
	CompFrame(frame)

{
  signalsrc.SetOwner (self);
  SCF_CONSTRUCT_IBASE (0);
  set_preferred_size = false;
}

awsComponent::~awsComponent ()
{
  /// Remove all children from TabOrder, but don't free them.
  TabOrder.DeleteAll ();

  /// Let go our references to any children if we have them.
  iAwsComponent* child = self->GetTopChild ();
  iAwsComponent* next;
  while (child)
  {
    next = child->ComponentBelow ();
    self->RemoveChild (child);
    child = next;
  }

  self->Unlink ();
  self->WindowManager()->ComponentDestroyed(self);
  SCF_DESTRUCT_IBASE ();
}

csRect awsComponent::Frame ()
{
  return frame;
}

csRect awsComponent::ClientFrame ()
{
  csRect insets = self->getInsets ();
  csRect client;
  client.xmin = self->Frame().xmin + insets.xmin;
  client.ymin = self->Frame().ymin + insets.ymin;
  client.xmax = self->Frame().xmax - insets.xmax;
  client.ymax = self->Frame().ymax - insets.ymax;

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
  return self->Flags () & AWSF_CMP_HIDDEN;
}

void awsComponent::SetFocusable (bool _focusable)
{
  focusable = _focusable;
}

bool awsComponent::Focusable ()
{
  return focusable;
}

bool awsComponent::isFocused ()
{
  return self->Flags () & AWSF_CMP_FOCUSED;
}

bool awsComponent::isDeaf ()
{
  return self->Flags () & AWSF_CMP_DEAF;
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
  return top_child != 0;
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
  return self;
}

bool awsComponent::Create (
  iAws* wmgr,
  iAwsComponent* parent, 
  iAwsComponentNode* settings)
{
  self->SetID (settings->Name ());
  self->SetParent (parent);

  /// Bind properties.
  CompType.Bind("Type", properties);
  CompFrame.Bind("Frame", properties);

  /// Set ourself up by querying the settings.
  if (!self->Setup (wmgr, settings))
    return false;

  /// If we are a top-level component link in to the top-level list.
  if (self->Parent () == 0)
  {
    // Link into the current hierarchy, at the top.
    if (wmgr->GetTopComponent ())
      self->LinkAbove (wmgr->GetTopComponent ());
    
    wmgr->SetTopComponent (self);
  }
  else
  {
    /**
     * Unless you have set the non client flag by this point 
     * you get added to the parent's layout.
     */
    if (~self->Flags() & AWSF_CMP_NON_CLIENT &&
         self->Parent ()->Layout ())
      self->Parent ()->Layout ()->AddComponent (self, settings);

    self->Parent ()->AddChild (self);
    self->Parent ()->AddToTabOrder (self);
  }
  return true;
}

/**
 * This function is normally called automatically by Create. You may
 * call it manually if you wish, but there's little reason to do so.
 */
bool awsComponent::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  if (wmgr) return false;

  wmgr = _wmgr;

#ifdef AWS_COMP_DEBUG  
  csPrintf ("aws-debug: setting up awsComponent (%s).\n", self->Type ());
#endif

  if (settings)
  {
    iAwsPrefManager *pm = self->WindowManager ()->GetPrefMgr ();

    pm->GetRect (settings, "Frame", frame);

#ifdef AWS_COMP_DEBUG
    csPrintf (
      "aws-debug: Frame is: (%d,%d)-(%d,%d)\n",
      frame.xmin,
      frame.ymin,
      frame.xmax,
      frame.ymax);
#endif

    /// Children are automatically filled in by the windowmanager.

    /// Do layout check.
    iString *ln = 0;

    pm->GetString (settings, "Layout", ln);

    if (ln)
    {
      if (strcmp ("GridBag", ln->GetData ()) == 0)
      {
        awsGridBagLayout* temp = new awsGridBagLayout (self, settings, pm);
        layout = SCF_QUERY_INTERFACE (temp, iAwsLayoutManager);
        temp->DecRef ();
      }
      else if (strcmp ("Border", ln->GetData ()) == 0)
      {
        awsBorderLayout* temp = new awsBorderLayout (self, settings, pm);
        layout = SCF_QUERY_INTERFACE (temp, iAwsLayoutManager);
        temp->DecRef ();
      }
    }

    iString *setStr = NULL;
    const csStringArray &cusProps = pm->GetCustomStringProperties();
    for( size_t i = 0; i < cusProps.Length(); ++i )
    {
      const char *tp = cusProps[ i ];
      pm->GetString( settings, tp, setStr );
      if( NULL != setStr )
      {
        csRef< iString > tr( setStr );
        _customStringProps.Put( pm->NameToId( tp ), tr );
      }
    }
  }
  return true;
}

bool awsComponent::GetProperty (const char *name, intptr_t *parm)
{
  if (strcmp ("Frame", name) == 0)
  {
    csRect rect = self->Frame ();
    csRect *r = new csRect (rect);
    *parm = (intptr_t)r;
    return true;
  }
  else if (strcmp ("Type", name) == 0)
  {
    iString *s = new scfString (self->Type ());
    *parm = (intptr_t)s;
    return true;
  }
  
  unsigned long nameId = wmgr->GetPrefMgr()->NameToId( name );
  csRef< iString > nullStr;
  csRef< iString > cusPropVal = _customStringProps.Get( nameId, nullStr );
  if( cusPropVal.IsValid() )
  {
    iString *s = new scfString (*cusPropVal);
    *parm = (intptr_t)s;
    return true;
  }
  return false;
}

bool awsComponent::SetProperty (const char *name, intptr_t parm)
{
  if (strcmp ("Frame", name) == 0)
  {
    csRect *r = (csRect *) (parm);
    self->ResizeTo (*r);
    return true;
  }
  
  if( csArrayItemNotFound != wmgr->GetPrefMgr()->GetCustomStringProperties().Find( name ) )
  {
    csRef< iString > ts( ( iString* ) parm );
    _customStringProps.PutUnique( wmgr->GetPrefMgr()->NameToId( name ), ts );
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
    self->Hide ();
    return true;
  }
  else if (strcmp ("Show", action) == 0)
  {
    self->Show ();
    return true;
  }
  else if (strcmp ("Invalidate", action) == 0)
  {
    self->Invalidate ();
    return true;
  }
  else if (strcmp ("HideWindow", action) == 0)
  {
    if (self->Window ())
    {
      self->Window ()->GetComponent ()->Hide ();
      self->WindowManager ()->InvalidateUpdateStore ();
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
      bool result = self->Overlaps (*r);
      parmlist->AddBool ("Result", result);
    }
    return true;
  }
  return false;
}

void awsComponent::Invalidate ()
{
  self->Invalidate (self->Frame());
}

void awsComponent::Invalidate (csRect area)
{
  if (self->WindowManager ())
    self->WindowManager ()->Mark (area);
}

bool awsComponent::HandleEvent (iEvent &Event)
{
#define SAVE_COMP(xcomp) \
  {\
    iAwsComponent *tc = xcomp;\
    while (tc != 0)\
    {\
      tc->IncRef();\
      tc = tc->Parent();\
    }\
  }

#define UNSAVE_COMP(xcomp) \
  {\
    iAwsComponent *tc = xcomp;\
    while (tc != 0)\
    {\
      iAwsComponent *tn = tc->Parent();\
      tc->DecRef();\
      tc = tn;\
    }\
  }

  switch (Event.Type)
  {
  case csevMouseMove:
    {
      SAVE_COMP(self)
      bool r = self->OnMouseMove (Event.Mouse.Button,
        Event.Mouse.x, Event.Mouse.y);
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevMouseUp:
    {
      SAVE_COMP(self)
      bool r = self->OnMouseUp (Event.Mouse.Button, Event.Mouse.x,
        Event.Mouse.y);
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevMouseDown:
    {
      SAVE_COMP(self)
      bool r = self->OnMouseDown (Event.Mouse.Button, Event.Mouse.x,
        Event.Mouse.y);
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevMouseClick:
    {
      SAVE_COMP(self)
      bool r = self->OnMouseClick (Event.Mouse.Button, Event.Mouse.x,
        Event.Mouse.y);
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevMouseEnter:
    {
      SAVE_COMP(self)
      bool r = self->OnMouseEnter ();
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevMouseExit:
    {
      SAVE_COMP(self)
      bool r = self->OnMouseExit ();
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevKeyboard:
    if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
    {
      csKeyEventData eventData;
      csKeyEventHelper::GetEventData (&Event, eventData);
      {
        SAVE_COMP(self)
        bool r = self->OnKeyboard (eventData);
        UNSAVE_COMP(self)
        return r;
      }
    }
    else
      return false;
    break;
  case csevGainFocus:
    {
      SAVE_COMP(self)
      bool r = self->OnGainFocus ();
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevLostFocus:
    {
      SAVE_COMP(self)
      bool r = self->OnLostFocus ();
      UNSAVE_COMP(self)
      return r;
    }
    break;
  case csevFrameStart:
    {
      SAVE_COMP(self)
      bool r = self->OnFrame ();
      UNSAVE_COMP(self)
      return r;
    }
    break;
  }
  return false;

#undef SAVE_COMP
#undef UNSAVE_COMP
}

bool awsComponent::Overlaps (csRect &r)
{
  return self->Frame().Intersects (r);
}

csRect awsComponent::getPreferredSize ()
{
  if (set_preferred_size)
    return preferred_size;
  else
    return self->getMinimumSize ();
}

void awsComponent::setPreferredSize (const csRect& size)
{
  preferred_size = size;
  set_preferred_size = true;
}

void awsComponent::clearPreferredSize ()
{
  set_preferred_size = false;
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

  /// Create a new child list if the current one does not exist.
  if (top_child)
    child->LinkAbove (top_child);
  top_child = child;

  if (!self->Layout ())
  {
    /**
     * Modify the child's rectangle to be inside and relative to the
     * parent's rectangle.
     */
    if (child->Flags () & AWSF_CMP_NON_CLIENT)
      child->Move (self->Frame ().xmin, self->Frame ().ymin);
    else
      child->Move (self->ClientFrame ().xmin,
                   self->ClientFrame ().ymin);
  }

  /// Fire off the event so that the child can do something if it needs to.
  child->OnAdded ();
}

void awsComponent::SetTopChild (iAwsComponent* child)
{
  top_child = child;
}

iAwsComponent* awsComponent::GetTopChild ()
{
  return top_child;
}

void awsComponent::RemoveChild (iAwsComponent *child)
{
  if (child == top_child)
    top_child = child->ComponentBelow ();
  
  child->Unlink ();
  child->DecRef ();
}

int awsComponent::GetChildCount ()
{
  int count = 0;
  iAwsComponent* cur = self->GetTopChild ();
  while (cur)
  {
    count++;
    cur = cur->ComponentBelow ();
  }
  return count;
}

iAwsComponent *awsComponent::FindChild (const char* name)
{
  unsigned id = self->WindowManager ()->GetPrefMgr ()->NameToId (name);

  return self->DoFindChild (id);
}

iAwsComponent *awsComponent::DoFindChild (unsigned id)
{
  if (!self->HasChildren ())
    return 0;

  iAwsComponent* result;
  iAwsComponent* child;
  for (child = self->GetTopChild ();
       child;
       child = child->ComponentBelow ())
  {
    /// If this child matches, good.
    if (child->GetID () == id)
      return child;

    /// Otherwise, check this child.
    if ((result = child->DoFindChild (id)) != 0)
      return result;
  }
  return 0;
}

iAwsComponent *awsComponent::ChildAt (int x, int y)
{
  /// If the point is not inside the client area then return 0.
  if (!self->Frame ().Contains (x, y))
    return 0;

  iAwsComponent* cmp;
  for (cmp = self->GetTopChild (); cmp; cmp = cmp->ComponentBelow ())
  {
    if (cmp->isHidden ())
      continue;
    if (!cmp->Frame ().Contains (x, y))
      continue;
    if (cmp->Flags () & AWSF_CMP_NON_CLIENT)
      return cmp;
    if (cmp->ClientFrame ().Contains (x, y))
      return cmp;
  }
  return 0;
}

void awsComponent::Hide ()
{
  if (self->Flags () & AWSF_CMP_HIDDEN)
    return;
  
  self->SetFlag (AWSF_CMP_HIDDEN);
  self->Invalidate ();
  if (!self->Parent ())
    self->WindowManager ()->InvalidateUpdateStore ();
  else
    self->Parent ()->OnChildHide ();
}

void awsComponent::Show ()
{
  if (!(self->Flags () & AWSF_CMP_HIDDEN))
    return;
  
  self->ClearFlag (AWSF_CMP_HIDDEN);
  self->Invalidate ();
  if (!self->Parent ())
    self->WindowManager ()->InvalidateUpdateStore ();
  else
    self->Parent ()->OnChildShow ();
}

void awsComponent::SetFocus ()
{
  if (self->Flags () & AWSF_CMP_FOCUSED)
    return;

  self->SetFlag (AWSF_CMP_FOCUSED);
  self->Invalidate ();
  if (!self->Parent ())
    self->WindowManager ()->InvalidateUpdateStore ();
  else
    self->OnSetFocus ();
}

void awsComponent::UnsetFocus ()
{
  if (!(self->Flags () & AWSF_CMP_FOCUSED))
    return;
  
  self->ClearFlag (AWSF_CMP_FOCUSED);
  self->Invalidate ();
  if (!self->Parent ())
    self->WindowManager ()->InvalidateUpdateStore ();
  else
    self->OnUnsetFocus ();
}

void awsComponent::SetDeaf (bool bDeaf)
{
  if (!((self->Flags () & AWSF_CMP_DEAF) ^ bDeaf))
    return;
  
  if (bDeaf)
    self->SetFlag (AWSF_CMP_DEAF);
  else
    self->ClearFlag (AWSF_CMP_DEAF);
}

void awsComponent::Move (int delta_x, int delta_y)
{
  /// Remove frivilous calls.
  if (delta_x == 0 && delta_y == 0)
    return; 

  csRect dirty1 (self->Frame ());

  self->Invalidate ();
  frame.Move (delta_x, delta_y);
  self->Invalidate ();

  MoveChildren (delta_x, delta_y);

  if (self->Parent ())
    self->Parent ()->OnChildMoved ();
  else
  {
    if (self->WindowManager ()->GetFlags () & AWSF_AlwaysEraseWindows)
      self->WindowManager ()->Erase (dirty1);
    self->WindowManager ()->InvalidateUpdateStore ();
  }
}

void awsComponent::MoveChildren (int delta_x, int delta_y)
{
  iAwsComponent* child = self->GetTopChild ();
  while (child)
  {
    child->Move (delta_x, delta_y);
    child = child->ComponentBelow ();
  }
}

void awsComponent::Resize (int w, int h)
{
  /// Remove frivilous calls.
  if (w == self->Frame ().Width () && h == self->Frame ().Height ())
    return;

  if ((!self->Parent ()) &&
      (w < self->Frame ().Width () || h < self->Frame ().Height ()))
  {
    if (self->WindowManager ()->GetFlags () & AWSF_AlwaysEraseWindows)
    {
      csRect f (self->Frame ());
      f.xmax++;
      f.ymax++;
      self->WindowManager ()->Erase (f);
    }
  }

  self->Invalidate ();
  frame.SetSize (w, h);
  self->Invalidate ();

  self->LayoutChildren ();
	
  if (self->Parent ())
    self->Parent ()->OnChildMoved ();
  else if (self->WindowManager ())
    self->WindowManager ()->InvalidateUpdateStore ();

  self->OnResized ();
}

void awsComponent::MoveTo (int x, int y)
{
  self->Move (x - self->Frame ().xmin, y - self->Frame ().ymin);
}

void awsComponent::ResizeTo (csRect newFrame)
{
  self->MoveTo (newFrame.xmin, newFrame.ymin);
  self->Resize (newFrame.Width (), newFrame.Height ());
}

void awsComponent::LayoutChildren ()
{
  if (self->Layout ()) self->Layout ()->LayoutComponents ();
}

void awsComponent::AddToLayout(iAwsComponent* cmp, iAwsComponentNode* settings)
{
  if (self->Layout ())
    self->Layout ()->AddComponent (cmp, settings);
}

iAwsComponent* awsComponent::Window ()
{
  iAwsComponent* cur = self;
  while (cur->Parent () && !(cur->Flags() & AWSF_CMP_WINDOW))
  {
    cur = cur->Parent ();
  }
  return cur;
}

void awsComponent::LinkAbove (iAwsComponent *comp)
{
  if (comp)
  {
    self->SetComponentAbove(comp->ComponentAbove ());
    self->SetComponentBelow(comp);
    comp->SetComponentAbove (self);
    if (self->ComponentAbove())
      self->ComponentAbove()->SetComponentBelow (self);
  }
  CS_ASSERT (LinkedListCheck ());
}

void awsComponent::LinkBelow (iAwsComponent *comp)
{
  if (comp)
  {
    self->SetComponentAbove(comp);
    self->SetComponentBelow(comp->ComponentBelow ());
    comp->SetComponentBelow (self);
    if (self->ComponentBelow())
      self->ComponentBelow()->SetComponentAbove (self);
  }
  CS_ASSERT(LinkedListCheck ());
}

void awsComponent::Unlink ()
{
  if (self->Parent () && self->Parent ()->GetTopChild () == self)
    parent->SetTopChild (self->ComponentBelow ());
  if (!self->Parent () && wmgr->GetTopComponent () == self)
    wmgr->SetTopComponent (self->ComponentBelow ());
  if (self->ComponentAbove ()) 
    self->ComponentAbove ()->SetComponentBelow (self->ComponentBelow ());
  if (self->ComponentBelow ())
    self->ComponentBelow ()->SetComponentAbove (self->ComponentAbove ());

  above = below = 0;
  CS_ASSERT(LinkedListCheck ());
}

iAwsComponent* awsComponent::ComponentAbove ()
{
  return above;
}

iAwsComponent* awsComponent::ComponentBelow ()
{
  return below;
}

void awsComponent::SetComponentAbove (iAwsComponent* comp)
{
  above = comp;
  CS_ASSERT (LinkedListCheck ());
}

void awsComponent::SetComponentBelow (iAwsComponent* comp)
{
  below = comp;
  CS_ASSERT (LinkedListCheck ());
}

bool awsComponent::AddToTabOrder (iAwsComponent *child)
{
  if (child->Parent () != self)
    return false;

  TabOrder.PushSmart (child);
  return true;
}

iAwsComponent *awsComponent::TabNext (iAwsComponent *child)
{
  size_t n = TabOrder.Find (child);

  if (n == csArrayItemNotFound)
    return 0;
  else if (n == TabOrder.Length () - 1)
    return ((iAwsComponent *)TabOrder[0]);
  else
    return ((iAwsComponent *)TabOrder[n + 1]);
}

iAwsComponent *awsComponent::TabPrev (iAwsComponent *child)
{
  int n = (int)TabOrder.Find(child);

  if (n == -1)
    return 0;
  else
  {
    if (n == 0)
      return ((iAwsComponent *)TabOrder[TabOrder.Length() - 1]);
    else
      return ((iAwsComponent *)TabOrder[n - 1]);
  }
}

int awsComponent::GetTabLength ()
{
  return (int)TabOrder.Length ();
}

iAwsComponent *awsComponent::GetTabComponent (int index)
{
  if ((size_t)index < TabOrder.Length ())
    return TabOrder[index];
  else
    return 0;
}

iAwsComponent *awsComponent::GetFirstFocusableChild (iAwsComponent *comp)
{
  int i;
  for (i = 0; i < comp->GetTabLength (); i++)
  {
    if (comp->GetTabComponent (i)->Focusable ())
      return comp->GetTabComponent (i);
    else
    {
      if (comp->GetTabComponent (i)->HasChildren ())
      {
        iAwsComponent *c = GetFirstFocusableChild (comp->GetTabComponent (i));
        if (c) return c;
      }
    }
  }
  return 0;
}

void awsComponent::SetAbove (iAwsComponent* comp)
{
  /// Get us out of the hierarchy.
  self->Unlink ();

  /// Go back in the hierarchy at the top.
  self->LinkAbove (comp);

  /// Child components tell there parent window to raise them.
  if (self->Parent ())
  {
    /// If we just took the head position fix up the head pointer.
    if (self->Parent ()->GetTopChild () == comp)
      self->Parent ()->SetTopChild (self);
  }
  else
  {
    if (self->WindowManager ()->GetTopComponent () == comp)
      self->WindowManager ()->SetTopComponent (self);
  }
  /// Make sure we get redrawn now.
  self->Invalidate ();

  return;
}

void awsComponent::SetBelow (iAwsComponent* comp)
{
  /// Get us out of the hierarchy.
  self->Unlink ();

  /// Go back in the hierarchy at the top.
  self->LinkBelow(comp);

  /// Make sure we get redrawn now.
  self->Invalidate();

  return;
}

void awsComponent::Raise ()
{
  if (self->Parent ())
  {
    if (self->Parent ()->GetTopChild () != self)
    {
      self->OnRaise ();
      SetAbove (self->Parent ()->GetTopChild ());
    }
  }
  else
  {
    if (self->WindowManager ()->GetTopComponent () != self)
    {
      self->OnRaise ();
      SetAbove (self->WindowManager ()->GetTopComponent ());
    }
  }
}

void awsComponent::Lower ()
{
  iAwsComponent* temp;
  if (self->Parent ())
    temp = self->Parent ()->GetTopChild ();
  else
    temp = self->WindowManager ()->GetTopComponent ();

  while (temp->ComponentBelow ())
    temp = temp->ComponentBelow ();

  if (temp != self)
  {
    self->OnLower ();
    SetBelow (temp);
  }
}

unsigned int awsComponent::RedrawTag ()
{
  return redraw_tag;
}

void awsComponent::SetRedrawTag (unsigned int tag)
{
  redraw_tag = tag;
}

void awsComponent::Maximize ()
{
  if (!is_zoomed)
  {
    is_zoomed = true;
    unzoomed_frame.Set (Frame ());

    if (!self->Parent ())
    {
      self->Move (-self->Frame ().xmin, -self->Frame ().ymin);
      self->Resize (self->WindowManager ()->G2D ()->GetWidth () - 1,
        self->WindowManager ()->G2D ()->GetHeight () - 1);
    }
    else
    {
      self->Move (
        self->Parent ()->ClientFrame ().xmin - self->Frame ().xmin,
        self->Parent ()->ClientFrame ().ymin - self->Frame ().ymin);
      self->Resize (
        self->Parent ()->ClientFrame ().Width (),
        self->Parent ()->ClientFrame ().Height ());
    }
  }
}

void awsComponent::UnMaximize ()
{
  if (is_zoomed)
  {
    is_zoomed = false;
    self->Move (unzoomed_frame.xmin - self->Frame ().xmin, 
      unzoomed_frame.ymin - self->Frame ().ymin);
    self->Resize (unzoomed_frame.Width (), unzoomed_frame.Height ());
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

bool awsComponent::OnMouseDown (int button, int x, int y)
{
#ifdef AWS_COMP_DEBUG
  csPrintf("aws-debug: mouse down  : %s\n", Type ());
#endif
  return false;
}

bool awsComponent::OnMouseUp (int button, int x, int y)
{
#ifdef AWS_COMP_DEBUG
  csPrintf("aws-debug: mouse up    : %s\n", Type ());
#endif
  return false;
}

bool awsComponent::OnMouseMove (int button, int x, int y)
{
  return false;
}

bool awsComponent::OnMouseClick (int button, int x, int y)
{
#ifdef AWS_COMP_DEBUG
  csPrintf("aws-debug: mouse click : %s\n", Type ());
#endif
  return false;
}

bool awsComponent::OnMouseDoubleClick (int button, int x, int y)
{
  return false;
}

bool awsComponent::OnMouseExit ()
{
#ifdef AWS_COMP_DEBUG
  csPrintf("aws-debug: mouse exit  : %s\n", Type ());
#endif
  return false;
}

bool awsComponent::OnMouseEnter ()
{
#ifdef AWS_COMP_DEBUG
  csPrintf("aws-debug: mouse enter : %s\n", Type ());
#endif
  return false;
}

bool awsComponent::OnKeyboard (const csKeyEventData&)
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

void awsComponent::OnRaise ()
{
  return ;
}

void awsComponent::OnLower ()
{
  return ;
}

void awsComponent::OnChildHide ()
{
  return ;
}

void awsComponent::OnChildShow ()
{
  return ;
}

void awsComponent::OnSetFocus ()
{
  return ;
}

void awsComponent::OnUnsetFocus ()
{
  return ;
}

/**
 * A factory is simply a class that knows how to build your component. 
 * Although components aren't required to have a factory, they will not
 * be able to be instantiated through the template functions and window
 * definitions if they don't.  In any case, a factory is remarkably
 * simple to build.  All you need to do is to inherit from
 * awsComponentFactory and call register with the window manager and
 * the named type of the component. That's it.
 */

SCF_IMPLEMENT_IBASE (awsComponentFactory)
  SCF_IMPLEMENTS_INTERFACE (iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsComponentFactory::awsComponentFactory (iAws *_wmgr)
{
  SCF_CONSTRUCT_IBASE (0);

  /**
   * This is where you call register, only you must do it in
   * the derived factory.  Like this.
   */

  wmgr = _wmgr;
}

awsComponentFactory::~awsComponentFactory ()
{
  SCF_DESTRUCT_IBASE ();
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
  for (cmp = self->ComponentBelow (); cmp; cmp = cmp->ComponentBelow ())
  {
    if (cmp == self)
      return false;
  }
  for (cmp = self->ComponentAbove (); cmp; cmp = cmp->ComponentAbove ())
  {
    if (cmp == self)
      return false;
  }
  for (cmp = self->Parent (); cmp; cmp = cmp->Parent ())
  {
    if (cmp == self)
      return false;
  }
  return true;
}
