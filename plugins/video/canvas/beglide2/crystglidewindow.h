/*
    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
  
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

#ifndef __CS_CRYSTGLIDEWINDOW_H__
#define __CS_CRYSTGLIDEWINDOW_H__

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Bitmap.h>
#include <DirectWindow.h>
class csGraphics2DBeGlide;

class CrystGlideView : public BView
{
public:
  CrystGlideView(BRect frame, iSystem*, BBitmap*);
  virtual ~CrystGlideView();
  virtual void AttachedToWindow();

  virtual void KeyDown(char const* bytes, int32 numBytes);
  virtual void KeyUp(char const* bytes, int32 numBytes);
  virtual void MouseDown(BPoint);
  virtual void MouseUp(BPoint);
  virtual void MouseMoved(BPoint, uint32 transit, BMessage const*);
  virtual void DirectConnected(direct_buffer_info*);
  virtual void Draw(BRect);

protected:
  iSystem* system;
  BBitmap* bitmap;
  void UserAction() const;
};

class CrystGlideWindow : public BDirectWindow
{
public:
  CrystGlideWindow(BRect, char const*, CrystGlideView*, iSystem*,
    csGraphics2DBeGlide*);
  virtual ~CrystGlideWindow();
  virtual bool QuitRequested();
  virtual void MessageReceived(BMessage*);
  virtual void DirectConnected(direct_buffer_info*);

  virtual status_t SetFullScreen(bool enable);
  virtual bool IsFullScreen();

protected:
  CrystGlideView* view;
  iSystem* system;
  csGraphics2DBeGlide* g2d;
};

#endif // __CS_CRYSTGLIDEWINDOW_H__
