/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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
#include "sysdef.h"
#include "cs2d/be/belibg2d.h"
#ifndef CRYST_WINDOW_H
#include "cs2d/be/CrystWindow.h"
#endif

static void doKey(int key, bool down) ;

CrystView::CrystView(BRect frame) : BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	inmotion = false;
}

CrystView::~CrystView(){}

void CrystView::KeyDown(const char *bytes, int32 numBytes)
{
	if(numBytes == 1) {
		doKey((int)*bytes, true);
	} else if(numBytes == 2 && bytes[0]==B_FUNCTION_KEY) {
		printf("got a function key=%x\n",bytes[1]);
	}
}

void CrystView::KeyUp(const char *bytes, int32 numBytes)
{
	if(numBytes == 1) {
		doKey((int)*bytes, false);
	} else if(numBytes == 2 && bytes[0]==B_FUNCTION_KEY) {
	}
}

static void doKey(int key, bool down) 
{
	uint32 kinfo = modifiers();
	BMessage	msg('keys');
	msg.AddInt16("key",key);
	msg.AddBool ("down", down);
	msg.AddBool ("shift", kinfo & B_SHIFT_KEY);
	msg.AddBool ("alt", kinfo & B_COMMAND_KEY);
	msg.AddBool ("ctrl", kinfo & B_CONTROL_KEY);

	be_app->PostMessage(&msg);
}

void CrystView::MouseMoved(BPoint point, uint32 transit, const BMessage *message) {
#if 0
	System->Mouse->do_mousemotion (point.x, point.y);
#endif
}

void CrystView::MouseDown(BPoint point)
{
//printf("got a mouse\n");
	uint32 buttons;
	BPoint p;
	uint32 kinfo = modifiers();
	GetMouse(&p,&buttons);

	if(buttons & B_PRIMARY_MOUSE_BUTTON )			buttons = 1;
	else if(buttons & B_SECONDARY_MOUSE_BUTTON)		buttons = 2;
	else if(buttons & B_TERTIARY_MOUSE_BUTTON)		buttons = 3;

	BMessage	msg('mous');
	msg.AddInt16("butn",buttons);
	msg.AddPoint("loc",p);
	msg.AddBool ("shift", kinfo & B_SHIFT_KEY);
	msg.AddBool ("alt", kinfo & B_COMMAND_KEY);
	msg.AddBool ("ctrl", kinfo & B_CONTROL_KEY);

	be_app->PostMessage(&msg);

	if((buttons!=0) && (!IsFocus()))
		MakeFocus();
	inmotion = buttons;
}

bool CrystView::IsInMotion(void) {
	uint32 buttons;
	BPoint p;
	GetMouse(&p,&buttons);

	if(buttons) {
		lastloc = p;
	} else if(inmotion) { // the button was just released
		MouseDown(p);
		inmotion = false;
	}
	return inmotion;
}

CrystWindow::CrystWindow(BRect frame, const char *name, CrystView *theview, csGraphics2DBeLib *piBeG2D)
//			: BWindow(frame,name, B_TITLED_WINDOW, 0, 0)
//			: BDirectWindow(frame,name, B_TITLED_WINDOW, 0, 0)
			: BDirectWindow(frame,name, B_TITLED_WINDOW, B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
//			: BWindowScreen(name, B_8_BIT_640x480, res, 0)
{
	view = theview;
	
	//	initialise local flags
	pi_BeG2D = piBeG2D;// cache a pointer to the csGraphics2DBeLib object for later use.
	pi_BeG2D->fConnected = false;
	pi_BeG2D->fConnectionDisabled = false;
	pi_BeG2D->locker = new BLocker();
	
	HRESULT hRes = piBeG2D->QueryInterface ((REFIID)IID_IBeLibGraphicsInfo, (void**)&piG2D);
	if (SUCCEEDED(hRes))	{
		printf("piBeG2D->QueryInterface succeeded \n");
	}

	view->SetViewColor(0, 0, 0);// remove for direct framebuffer access
//	view->SetViewColor(B_TRANSPARENT_32_BIT);// uncomment for direct framebuffer access
	AddChild(view);

	// Add a shortcut to switch in and out of fullscreen mode.
	AddShortcut('f', B_COMMAND_KEY, new BMessage('full'));
	
	// As we said before, the window shouldn't get wider than 2048 in any
	// direction, so those limits will do.
	SetSizeLimits(40.0, 2000.0, 40.0, 2000.0);//this isn't needed in BDirectWindow
}

CrystWindow::~CrystWindow()
{
//	long		result;
	pi_BeG2D->fConnectionDisabled = true;
	Hide();
	Flush();
	delete pi_BeG2D->locker;
	piG2D->Release();
}

bool CrystWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(TRUE);
}

void CrystWindow::MessageReceived(BMessage *message)
{
	switch(message->what) {
	// Switch between full-screen mode and windowed mode.
	case 'full' :
//		SetFullScreen(!IsFullScreen());
		break;
	default :
		BWindow::MessageReceived(message);
		break;
	}
}

void CrystWindow::DirectConnected(direct_buffer_info *info)
{
printf("Entered CrystWindow::DirectConnected \n");
//HRESULT hRes = piG2D->DirectConnect(info);//dh:uncomment this to get direct framebuffer access

//	this bit just keeps conventional window behaviour until I've sorted out DirectConnected
BDirectWindow::DirectConnected(info);
}

long CrystWindow::StarAnimation(void *data)
{
	return 0;
}

