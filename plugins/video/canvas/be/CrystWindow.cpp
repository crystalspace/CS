/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Written by Xavier Planet.
    Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>
  
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
#include <sys/param.h>
#include "sysdef.h"
#include "isystem.h"
#include "cs2d/be/CrystWindow.h"
#include "cssys/be/icsbe.h"

CrystView::CrystView(BRect frame, iBeLibSystemDriver* isys) :
  BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW), be_system(isys)
{
  be_system->IncRef();
}

CrystView::~CrystView()
{
  be_system->DecRef();
}

void CrystView::ProcessUserEvent() const
{
  be_system->ProcessUserEvent(Looper()->CurrentMessage());
}

void CrystView::KeyDown(const char *bytes, int32 numBytes)
{
  ProcessUserEvent();
}

void CrystView::KeyUp(const char *bytes, int32 numBytes)
{
  ProcessUserEvent();
}

void CrystView::MouseMoved(BPoint point, uint32 transit, BMessage const* m)
{
  ProcessUserEvent();
}

void CrystView::MouseDown(BPoint point)
{
  ProcessUserEvent();
  if (!IsFocus())
    MakeFocus();
}

void CrystView::MouseUp(BPoint point)
{
  ProcessUserEvent();
}

CrystWindow::CrystWindow(BRect frame, const char* name, CrystView* v,
  iSystem* isys, iBeLibSystemDriver* bsys) :
  BDirectWindow(frame,name, B_TITLED_WINDOW, B_NOT_RESIZABLE|B_NOT_ZOOMABLE),
  view(v), cs_system(isys), be_system(bsys)
{
  be_system->IncRef();
  view->SetViewColor(0, 0, 0);
  AddChild(view);
  SetSizeLimits(40, 2000, 40, 2000);
}

CrystWindow::~CrystWindow()
{
  Hide();
  Flush();
  be_system->DecRef();
}

bool CrystWindow::QuitRequested()
{
  cs_system->StartShutdown();
  // @@@FIXME: Don't destroy window before "LoopThread" has finished.
  return true;
}
