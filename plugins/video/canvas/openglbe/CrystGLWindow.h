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

#ifndef __CS_CRYSTGLWINDOW_H__
#define __CS_CRYSTGLWINDOW_H__

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Bitmap.h>
#include <DirectWindow.h>
#include <GLView.h>
struct iObjectRegistry;
struct iGraphics2D;

class CrystGLView : public BGLView
{
public:
  CrystGLView(BRect frame, iObjectRegistry*);
  virtual ~CrystGLView();
  virtual void AttachedToWindow();

  virtual void KeyDown(char const* bytes, int32 numBytes);
  virtual void KeyUp(char const* bytes, int32 numBytes);
  virtual void MouseDown(BPoint);
  virtual void MouseUp(BPoint);
  virtual void MouseMoved(BPoint, uint32 transit, BMessage const*);

protected:
  iObjectRegistry* object_reg;
  void UserAction() const;
};

class CrystGLWindow : public BDirectWindow
{
public:
  CrystGLWindow(BRect, char const*, CrystGLView*, iObjectRegistry*,
  	iGraphics2D*);
  virtual ~CrystGLWindow();
  virtual bool QuitRequested();
  virtual void MessageReceived(BMessage*);
  virtual void DirectConnected(direct_buffer_info*);

protected:
  CrystGLView* view;
  iObjectRegistry* object_reg;
  iGraphics2D* g2d;
};

#endif // __CS_CRYSTGLWINDOW_H__
