/*
    Copyright (C) 2000 by Norman Krämer
  
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
#include "csws/csslider.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"

csSlider::csSlider (csComponent *pParent) : csComponent (pParent)
{
  SetPalette (CSPAL_SLIDER);
  SetState (CSS_SELECTABLE, true);
  SetState (CSS_TOPSELECT, true);
  isSliding = false;
  mx = my = 0;
}

void csSlider::Draw ()
{
  if (isSliding){
    Box (0, 0, bound.Width (), bound.Height (), CSPAL_SLIDER_ACTIVE_FILL);
  }else{
    Box (0, 0, bound.Width (), bound.Height (), CSPAL_SLIDER_FILL);
  }
}

bool csSlider::HandleEvent (csEvent &Event)
{
  switch (Event.Type){
  case csevMouseDoubleClick:
  case csevMouseDown:
    if (app->MouseOwner != this && Event.Mouse.Button == 1){
      // we are going to capture the mouse to follow all movements
      app->CaptureMouse (this);
      isSliding = true;
      Select ();
      Invalidate ();
      return true;
    }
    break;
  case csevMouseUp:
    if (isSliding && Event.Mouse.Button == 1){
      app->CaptureMouse (NULL);
      isSliding = false;
      mx = Event.Mouse.x;
      my = Event.Mouse.y;
      parent->SendCommand (cscmdSliderPosSet, (void*)this);
      Invalidate ();
      return true;
    }
    break;
  case csevMouseMove:
    if (isSliding){
      mx = Event.Mouse.x;
      my = Event.Mouse.y;
      if (isHorizontal)
	SetPos (bound.xmin + mx, bound.ymin);
      else
	SetPos (bound.xmin, bound.ymin + my);
      if (parent)
	parent->SendCommand (cscmdSliderPosChanged, (void*)this);
      Invalidate ();
      return true;
    }
    break;
  }
  return csComponent::HandleEvent (Event);
}

bool csSlider::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csComponent::SetRect (xmin, ymin, xmax, ymax)){
    isHorizontal = bound.Height () >  bound.Width ();
    return true;
  }
  return false;
}
