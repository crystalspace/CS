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

#include "cssysdef.h"
#include <string.h>
#include "csws/cscomp.h"
#include "csws/csttlbar.h"
#include "iutil/event.h"

csTitleBar::csTitleBar (csComponent *iParent, const char *iText)
  : csComponent (iParent)
{
  text = new char [strlen (iText) + 1];
  strcpy (text, iText);
  SetPalette (CSPAL_TITLEBAR);
  ApplySkin (GetSkin ());
}

bool csTitleBar::HandleEvent (iEvent &Event)
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
