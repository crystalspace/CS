/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
  
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cssysdef.h"
#include "isys/event.h"
#include "ivideo/graph2d.h"
#include "isys/system.h"
#include "CrystWindow.h"

CrystView::CrystView(BRect frame, iSystem* isys, BBitmap* ibmap) :
  BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW), system(isys), bitmap(ibmap)
{
  system->IncRef();
}

CrystView::~CrystView()
{
  system->DecRef();
}

void CrystView::UserAction() const
{
  system->SystemExtension("UserAction", Looper()->CurrentMessage());
}

void CrystView::KeyDown(char const *bytes, int32 numBytes)
{
  UserAction();
}

void CrystView::KeyUp(char const *bytes, int32 numBytes)
{
  UserAction();
}

void CrystView::MouseMoved(BPoint, uint32 transit, BMessage const*)
{
  UserAction();
}

void CrystView::MouseDown(BPoint)
{
  UserAction();
  if (!IsFocus())
    MakeFocus();
}

void CrystView::MouseUp(BPoint)
{
  UserAction();
}

void CrystView::Draw(BRect r)
{
  DrawBitmap(bitmap, r, r);
}

CrystWindow::CrystWindow(BRect frame, char const* name, CrystView* v,
  iSystem* isys, iGraphics2D* ig2d) :
  BDirectWindow(frame, name, B_TITLED_WINDOW, B_NOT_RESIZABLE|B_NOT_ZOOMABLE),
  view(v), system(isys), g2d(ig2d)
{
  system->IncRef();
  g2d->IncRef();
  view->SetViewColor(0, 0, 0);
  AddChild(view);
  SetSizeLimits(40, 2000, 40, 2000);
}

CrystWindow::~CrystWindow()
{
  Hide();
  Flush();
  g2d->DecRef();
  system->DecRef();
}

bool CrystWindow::QuitRequested()
{
  system->SystemExtension("ContextClose", g2d);
  system->SystemExtension("Quit");
  return false; // Allow Crystal Space to close window itself.
}
