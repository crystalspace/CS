/*
    Crystal Space Windowing System: title bar class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#include <string.h>
#include "sysdef.h"
#include "csws/cscomp.h"
#include "csws/csttlbar.h"
#include "csinput/csevent.h"

csTitleBar::csTitleBar (csComponent *iParent, char *iText)
  : csComponent (iParent)
{
  CHK (text = new char [strlen (iText) + 1]);
  strcpy (text, iText);
  SetPalette (CSPAL_TITLEBAR);
}

void csTitleBar::Draw ()
{
  int indx = parent->GetState (CSS_FOCUSED) ? CSPAL_TITLEBAR_ABACKGROUND :
    CSPAL_TITLEBAR_PBACKGROUND;

  // Draw the 3D frame around titlebar
  Rect3D (0, 0, bound.Width (), bound.Height (), CSPAL_TITLEBAR_LIGHT3D,
    CSPAL_TITLEBAR_DARK3D);

  // Draw title bar background
  Box (1, 1, bound.Width () - 1, bound.Height () - 1, indx);

  // Draw title bar text
  int tw = TextWidth (text);
  int th = TextHeight ();
  int tx = (bound.Width () - tw) / 2;
  int ty = (bound.Height () - th) / 2;
  Text (tx, ty, indx + 1, -1, text);

  // Hash title bar (a-la Macintosh)
  int bdy = bound.Height () / 4;
  if ((bdy > 2) && (tx > 16))
    for (int by = 1 + bdy / 2; by < bound.Height () - 2; by += 4)
    {
      Line (8, by, tx - 8, by, indx + 2);
      Line (8, by + 1, tx - 8, by + 1, indx + 3);
      Line (tx + tw + 7, by, bound.Width () - 9, by, indx + 2);
      Line (tx + tw + 7, by + 1, bound.Width () - 9, by + 1, indx + 3);
    }

  csComponent::Draw ();
}

bool csTitleBar::HandleEvent (csEvent &Event)
{
  bool retc = csComponent::HandleEvent (Event);
  switch (Event.Type)
  {
    case csevMouseDown:
      if (parent && ((Event.Mouse.Button == 1) || (Event.Mouse.Button == 2)))
      {
        parent->Select ();
        parent->Drag (Event.Mouse.x + bound.xmin, Event.Mouse.y + bound.ymin, CS_DRAG_ALL);
        return true;
      }
      return true;
    case csevMouseDoubleClick:
      if (parent && (Event.Mouse.Button == 1))
        parent->SendCommand (cscmdMaximize);
      return true;
    case csevMouseMove:
      if (bound.ContainsRel (Event.Mouse.x, Event.Mouse.y))
        SetMouse (csmcMove);
      return true;
  } /* endswitch */
  return retc;
}
