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
#include "CrystGLWindow.h"

CrystGLView::CrystGLView(BRect frame, iSystem* isys) :
  BGLView(frame, "", B_FOLLOW_NONE, 0, BGL_RGB | BGL_DEPTH | BGL_DOUBLE),
  system(isys)
{
  system->IncRef();
}

CrystGLView::~CrystGLView()
{
  system->DecRef();
}

void CrystGLView::UserAction() const
{
  system->SystemExtension("UserAction", Looper()->CurrentMessage());
}

void CrystGLView::KeyDown(char const* bytes, int32 numBytes)
{
  UserAction();
}

void CrystGLView::KeyUp(char const* bytes, int32 numBytes)
{
  UserAction();
}

void CrystGLView::MouseMoved(BPoint, uint32 transit, BMessage const*)
{
  UserAction();
}

void CrystGLView::MouseDown(BPoint)
{
  UserAction();
  if (!IsFocus())
    MakeFocus();
}

void CrystGLView::MouseUp(BPoint)
{
  UserAction();
}

void CrystGLView::AttachedToWindow()
{
  LockGL(); 
  BGLView::AttachedToWindow(); 
  UnlockGL();
}

CrystGLWindow::CrystGLWindow(BRect frame, char const* name, CrystGLView *v,
	iSystem* isys, iGraphics2D* ig2d) :
	BDirectWindow(frame, name, B_TITLED_WINDOW, B_NOT_RESIZABLE, 0),
	view(v), system(isys), g2d(ig2d)
{
  system->IncRef();
  g2d->IncRef();
  view->SetViewColor(0, 0, 0);

  AddChild(view);
  AddShortcut('f', B_COMMAND_KEY, new BMessage('full'));
  SetSizeLimits(40.0, 2000.0, 40.0, 2000.0);
}

CrystGLWindow::~CrystGLWindow()
{
  Hide();
  Flush();
  g2d->DecRef();
  system->DecRef();
}

void CrystGLWindow::DirectConnected(direct_buffer_info* info)
{
  if (view)
  {
    view->DirectConnected(info);
    view->EnableDirectMode(true);
  }
}

bool CrystGLWindow::QuitRequested()
{
  system->SystemExtension("ContextClose", g2d);
  system->SystemExtension("Quit");
  return false; // Allow Crystal Space to close window itself.
}

void CrystGLWindow::MessageReceived(BMessage* m)
{
  switch (m->what)
  {
    case 'full':
      SetFullScreen(!IsFullScreen());
      break;
    default:
      BWindow::MessageReceived(m);
      break;
  }
}
