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

#ifndef __CS_CRYSTWINDOW_H__
#define __CS_CRYSTWINDOW_H__

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Bitmap.h>
#include <DirectWindow.h>
struct iObjectRegistry;
struct iGraphics2D;

class CrystView : public BView
{
public:
  CrystView(BRect, iObjectRegistry*, BBitmap*); 
  virtual ~CrystView();

  virtual void KeyDown(char const* bytes, int32 numBytes);
  virtual void KeyUp(char const* bytes, int32 numBytes);
  virtual void MouseDown(BPoint);
  virtual void MouseUp(BPoint);
  virtual void MouseMoved(BPoint, uint32 transit, BMessage const*);
  virtual void Draw(BRect);

protected:
  iObjectRegistry* object_reg;
  BBitmap* bitmap;
  void UserAction() const;
};

class CrystWindow : public BDirectWindow
{
public:
  CrystWindow (BRect, char const*, CrystView*, iObjectRegistry*, iGraphics2D*);
  virtual ~CrystWindow();
  virtual bool QuitRequested();

protected:
  CrystView* view;
  iObjectRegistry* object_reg;
  iGraphics2D* g2d;
};

#endif __CS_CRYSTWINDOW_H__
