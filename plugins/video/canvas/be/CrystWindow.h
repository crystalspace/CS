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

#ifndef CRYST_WINDOW_H
#define CRYST_WINDOW_H

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Bitmap.h>
#include <DirectWindow.h>

class iSystem;
class csGraphics2DBeLib;
class iBeLibSystemDriver;

class CrystView : public BView
{
public:
	CrystView(BRect frame, iBeLibSystemDriver*); 
	virtual	~CrystView();

	virtual void KeyDown(char const* bytes, int32 numBytes);
	virtual void KeyUp(char const* bytes, int32 numBytes);
	virtual void MouseDown(BPoint);
	virtual void MouseUp(BPoint);
	virtual void MouseMoved(BPoint, uint32 transit, BMessage const*);

protected:
	iBeLibSystemDriver* be_system;
	void ProcessUserEvent() const;
};

class CrystWindow : public BDirectWindow
{

public:
	CrystWindow (BRect, const char*, CrystView*,
		csGraphics2DBeLib*, iSystem*, iBeLibSystemDriver*);
	virtual	~CrystWindow();

	virtual	bool QuitRequested();
	virtual	void MessageReceived(BMessage*);

	virtual bool DirectConnected(direct_buffer_info*);

protected:
	CrystView* view;
	iSystem* cs_system;
	iBeLibSystemDriver* be_system;
	csGraphics2DBeLib* pG2D;
};

#endif CRYST_WINDOW_H
