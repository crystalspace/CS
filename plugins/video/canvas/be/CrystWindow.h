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
class ISystem;
class csGraphics2DBeLib;
class IBeLibSystemDriver;

class CrystView : public BView {

public:
		CrystView(BRect frame, IBeLibSystemDriver*); 
virtual	~CrystView();

virtual void KeyDown(char const* bytes, int32 numBytes);
virtual void KeyUp(char const* bytes, int32 numBytes);
virtual void MouseDown(BPoint);
virtual void MouseUp(BPoint);
virtual void MouseMoved(BPoint, uint32 transit, BMessage const*);

protected:
		IBeLibSystemDriver* be_system;
		void ProcessUserEvent() const;
};

class CrystWindow : public BDirectWindow { // BWindowScreen {

public:
		CrystWindow(BRect, char const*, CrystView*,
			csGraphics2DBeLib*, ISystem*, IBeLibSystemDriver*);
virtual	~CrystWindow();

virtual	bool QuitRequested();
virtual	void MessageReceived(BMessage*);

virtual void DirectConnected(direct_buffer_info*);

protected:
		CrystView* view;
		ISystem* cs_system;
		IBeLibSystemDriver* be_system;
		// Stuff to implement BDirectWindow
		// FIXME: Why keep pi_BeG2D when piG2D is around?
		IBeLibGraphicsInfo* piG2D;   // New pointer to 2D driver info method interface.
		csGraphics2DBeLib* pi_BeG2D; // Local copy of this pointer to csGraphics2DBeLib.
};

#endif CRYST_WINDOW_H
