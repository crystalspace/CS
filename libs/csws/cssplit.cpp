/*
    Copyright (C) 2000 by Norman Kraemer

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
#include "csws/cssplit.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"

csSplitter::csSplitter (csComponent *pParent) : csComponent (pParent)
{
  SetPalette (CSPAL_SPLITTER);
  state |= CSS_SELECTABLE | CSS_TOPSELECT;
  isSliding = false;
}

void csSplitter::Draw ()
{
  int idx = isSliding ? 3 : 0;
  int dx = 0, dy = 0;
  int w = bound.Width (), h = bound.Height ();
  if (isHorizontal)
  {
    Line (0, 0, 0, h, idx + CSPAL_SPLITTER_ILIGHT3D);
    Line (w - 1, 0, w - 1, h - 1, idx + CSPAL_SPLITTER_IDARK3D);
    dx = 1;
  }
  else
  {
    Line (0, 0, w, 0, idx + CSPAL_SPLITTER_ILIGHT3D);
    Line (0, h - 1, w, h - 1, idx + CSPAL_SPLITTER_IDARK3D);
    dy = 1;
  }
  Box (dx, dy, bound.Width () - dx, bound.Height () - dy,
    idx + CSPAL_SPLITTER_IBACKGROUND);
}

bool csSplitter::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseDown:
      if (app->MouseOwner != this && Event.Mouse.Button == 1)
      {
        // we are going to capture the mouse to follow all movements
        app->CaptureMouse (this);
        isSliding = true;
        Select ();
        // Save the position of the mouse relative to our top-left corner
        mousex = mdx = Event.Mouse.x; mousey = mdy = Event.Mouse.y;
        Invalidate ();
        return true;
      }
      break;
    case csevMouseUp:
      if (isSliding && Event.Mouse.Button == 1)
      {
        app->CaptureMouse (0);
        isSliding = false;
        parent->SendCommand (cscmdSplitterPosSet, (intptr_t)this);
        Invalidate ();
        return true;
      }
      break;
    case csevMouseMove:
      SetMouse (isHorizontal ? csmcSizeEW : csmcSizeNS);
      if (isSliding)
      {
        mousex = Event.Mouse.x;
	mousey = Event.Mouse.y;
        if (isHorizontal)
          SetPos (bound.xmin + mousex - mdx, bound.ymin);
        else
          SetPos (bound.xmin, bound.ymin + mousey - mdy);
        if (parent)
          parent->SendCommand (cscmdSplitterPosChanged, (intptr_t)this);
        return true;
      }
      break;
  }
  return csComponent::HandleEvent (Event);
}

bool csSplitter::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csComponent::SetRect (xmin, ymin, xmax, ymax))
  {
    isHorizontal = bound.Height () >  bound.Width ();
    return true;
  }
  return false;
}

void csSplitter::GetPos (int &x, int &y)
{
  x = bound.xmin + (isHorizontal ? bound.Width () / 2 : mousex);
  y = bound.ymin + (isHorizontal ? mousey : bound.Height () / 2);
}
