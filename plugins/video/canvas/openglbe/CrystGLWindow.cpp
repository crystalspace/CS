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
#include "iutil/event.h"
#include "ivideo/graph2d.h"
#include "CrystGLWindow.h"
#include "cssys/be/behelp.h"
#include "iutil/objreg.h"

CrystGLView::CrystGLView(BRect frame, iObjectRegistry* objreg) :
  BGLView(frame, "", B_FOLLOW_NONE, 0, BGL_RGB | BGL_DEPTH | BGL_DOUBLE),
  object_reg(objreg)
{
}

CrystGLView::~CrystGLView()
{
}

void CrystGLView::UserAction() const
{
  iBeHelper* behelper = CS_QUERY_REGISTRY (object_reg, iBeHelper);
  CS_ASSERT (behelper != NULL);
  behelper->UserAction (Looper()->CurrentMessage());
  behelper->DecRef ();
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
	iObjectRegistry* objreg, iGraphics2D* ig2d) :
	BDirectWindow(frame, name, B_TITLED_WINDOW, B_NOT_RESIZABLE, 0),
	view(v), object_reg(objreg), g2d(ig2d)
{
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
  iBeHelper* behelper = CS_QUERY_REGISTRY (object_reg, iBeHelper);
  CS_ASSERT (behelper != NULL);
  behelper->ContextClose(g2d);
  behelper->Quit ();
  behelper->DecRef ();
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
