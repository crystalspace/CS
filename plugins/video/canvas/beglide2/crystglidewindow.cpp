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
#include "isys/system.h"
#include "crystglidewindow.h"
#include "glidebe2d.h"

CrystGlideView::CrystGlideView(BRect frame, iSystem* isys, BBitmap* ibmap) :
  BView(frame, "", B_FOLLOW_NONE, B_WILL_DRAW), system(isys), bitmap(ibmap)
{
  system->IncRef();
}

CrystGlideView::~CrystGlideView()
{
  system->DecRef();
}

void CrystGlideView::UserAction() const
{
  system->SystemExtension("UserAction", Looper()->CurrentMessage());
}

void CrystGlideView::KeyDown(char const* bytes, int32 numBytes)
{
  UserAction();
}

void CrystGlideView::KeyUp(char const* bytes, int32 numBytes)
{
  UserAction();
}

void CrystGlideView::MouseMoved(BPoint, uint32 transit, BMessage const*) 
{
  UserAction();
}

void CrystGlideView::MouseDown(BPoint)
{
  UserAction();
  if (!IsFocus())
    MakeFocus();
}

void CrystGlideView::MouseUp(BPoint point)
{
  UserAction();
}

void CrystGlideView::Draw(BRect r)
{
  DrawBitmap(bitmap, r, r);
}

void CrystGlideView::AttachedToWindow()
{
  BView::AttachedToWindow(); 
}

void CrystGlideView::DirectConnected(direct_buffer_info* info)
{
  // Hopefully this will call Glide2x.so's DirectConnected(). [David Heuen]
  DirectConnected(info);
}

CrystGlideWindow::CrystGlideWindow(BRect frame, char const* name,
  CrystGlideView* v, iSystem* isys, csGraphics2DBeGlide* ig2d) :
  BDirectWindow(frame, name, B_TITLED_WINDOW, B_NOT_RESIZABLE, 0),
  view(v), system(isys), g2d(ig2d)
{
  system->IncRef();
  g2d->IncRef();
  view->SetViewColor(B_TRANSPARENT_32_BIT);

  AddChild(view);
  AddShortcut('f', B_COMMAND_KEY, new BMessage('full'));
  SetSizeLimits(40.0, 2000.0, 40.0, 2000.0);
}

CrystGlideWindow::~CrystGlideWindow()
{
  Hide();
  Flush();
  g2d->DecRef();
  system->DecRef();
}

void CrystGlideWindow::DirectConnected(direct_buffer_info* info)
{
  BDirectWindow::DirectConnected(info);
}

bool CrystGlideWindow::QuitRequested()
{
  system->SystemExtension("ContextClose", g2d);
  system->SystemExtension("Quit");
  return false; // Allow Crystal Space to close window itself.
}

void CrystGlideWindow::MessageReceived(BMessage* m)
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

status_t CrystGlideWindow::SetFullScreen(bool enable)
{
  status_t rc = BDirectWindow::SetFullScreen(enable);
  if (rc == B_OK)
    g2d->SetFullScreen(enable);
  return rc;
}

bool CrystGlideWindow::IsFullScreen()
{
  return BDirectWindow::IsFullScreen();
}
