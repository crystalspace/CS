/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
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
#include "cs2d/openglbe/CrystGLWindow.h"
#include "cssys/be/icsbe.h"

CrystGLView::CrystGLView(BRect frame, iBeLibSystemDriver* isys) :
  BGLView(frame, "", B_FOLLOW_NONE, 0, BGL_RGB | BGL_DEPTH | BGL_DOUBLE),
  be_system(isys)
{
  be_system->IncRef();
}

CrystGLView::~CrystGLView()
{
  be_system->DecRef();
}

void CrystGLView::ProcessUserEvent() const
{
  be_system->ProcessUserEvent(Looper()->CurrentMessage());
}

void CrystGLView::KeyDown(char const* bytes, int32 numBytes)
{
  ProcessUserEvent();
}

void CrystGLView::KeyUp(char const* bytes, int32 numBytes)
{
  ProcessUserEvent();
}

void CrystGLView::MouseMoved(BPoint point, uint32 transit, const BMessage* m)
{
  ProcessUserEvent();
}

void CrystGLView::MouseDown(BPoint point)
{
  ProcessUserEvent();
  if (!IsFocus())
    MakeFocus();
}

void CrystGLView::MouseUp(BPoint point)
{
  ProcessUserEvent();
}

void CrystGLView::AttachedToWindow()
{
  LockGL(); 
  BGLView::AttachedToWindow(); 
  UnlockGL();
}

CrystGLWindow::CrystGLWindow(BRect frame, const char* name, CrystGLView *v,
	iSystem* isys, iBeLibSystemDriver* bsys) :
	BDirectWindow(frame,name, B_TITLED_WINDOW, B_NOT_RESIZABLE, 0),
	view(v), cs_system(isys), be_system(bsys)
{
  be_system->IncRef();
  view->SetViewColor(0, 0, 0);
  AddChild(view);
  AddShortcut('f', B_COMMAND_KEY, new BMessage('full'));
  SetSizeLimits(40.0, 2000.0, 40.0, 2000.0);
}

CrystGLWindow::~CrystGLWindow()
{
  Hide();
  Flush();
  be_system->DecRef();
}

bool CrystGLWindow::QuitRequested()
{
  cs_system->StartShutdown();
  // @@@FIXME: Don't destroy window before "LoopThread" has finished.
  return true;
}

void CrystGLWindow::MessageReceived(BMessage* m)
{
  switch(m->what)
  {
    case 'full':
      SetFullScreen(!IsFullScreen());
      break;
    default:
      BWindow::MessageReceived(m);
      break;
  }
}
