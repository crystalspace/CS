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
#include "csws/csapp.h"
#include "csws/csttlbar.h"
#include "csws/sdefault.h"

void csDefaultTitlebarSkin::Initialize (csApp *iApp, csSkin *Parent)
{
  (void)iApp;
  Parent->Load (ABack, "Titlebar", "ActiveBackground");
  Parent->Load (IBack, "Titlebar", "InactiveBackground");
  Hash = Parent->GetConfigYesNo ("Titlebar", "EnableHash", true);
}

void csDefaultTitlebarSkin::Deinitialize ()
{
  ABack.Free ();
  IBack.Free ();
}

void csDefaultTitlebarSkin::Draw (csComponent &iComp)
{
  csTitleBar &This = (csTitleBar &)iComp;

  bool focused = This.parent->GetState (CSS_FOCUSED) ? true : false;
  int indx = focused ? CSPAL_TITLEBAR_ABACKGROUND :
    CSPAL_TITLEBAR_PBACKGROUND;

  // Draw the 3D frame around titlebar
  This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
    CSPAL_TITLEBAR_LIGHT3D, CSPAL_TITLEBAR_DARK3D);

  // Draw title bar background
  csBackground &b = focused ? ABack : IBack;
  if (b.GetType () != csbgNone)
    b.Draw (This, 1, 1, This.bound.Width () - 2, This.bound.Height () - 2, 0, 0, 0);
  else
    This.Box (1, 1, This.bound.Width () - 1, This.bound.Height () - 1, indx);

  // Draw title bar text
  int th, tw = This.GetTextSize (This.GetText (), &th);
  int tx = (This.bound.Width () - tw) / 2;
  int ty = (This.bound.Height () - th) / 2;
  This.Text (tx, ty, indx + 1, -1, This.GetText ());

  // Hash title bar (a-la Macintosh)
  if (Hash)
  {
    int bdy = This.bound.Height () / 4;
    if ((bdy > 2) && (tx > 8 + This.bound.Height ()))
	{
	  int by;
      for (by = 1 + bdy / 2; by < This.bound.Height () - 2; by += 4)
      {
        This.Line (8, by, tx - 7, by, indx + 2);
        This.Line (8, by + 1, tx - 7, by + 1, indx + 3);
        This.Line (tx + tw + 7, by, This.bound.Width () - 8, by, indx + 2);
        This.Line (tx + tw + 7, by + 1, This.bound.Width () - 8, by + 1, indx + 3);
      }
	}
  }
}
