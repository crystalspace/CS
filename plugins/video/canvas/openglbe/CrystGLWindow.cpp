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

/*
		
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "sysdef.h"
#include "cs2d/openglbe/glbe2d.h"
#include "isystem.h"

#ifndef CRYST_WINDOW_H
#include "CrystGLWindow.h"
#endif

static void doKey(int key, bool down) ;

CrystGLView::CrystGLView(BRect frame)
	: BGLView(frame, "", B_FOLLOW_NONE, 0, BGL_RGB | BGL_DEPTH | BGL_DOUBLE)
{
	inmotion = false;
}

CrystGLView::~CrystGLView(){}

void CrystGLView::KeyDown(const char *bytes, int32 numBytes)
{
// printf("got a key\n");
	if(numBytes == 1) {
		doKey((int)*bytes, true);
	} else if(numBytes == 2 && bytes[0]==B_FUNCTION_KEY) {
//		printf("got a function key=%x\n",bytes[1]);
	}
}

void CrystGLView::KeyUp(const char *bytes, int32 numBytes)
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

void CrystGLView::MouseMoved(BPoint point, uint32 transit, const BMessage *message) {
#if 0
	System->Mouse->do_mousemotion (point.x, point.y);
#endif
}

void CrystGLView::MouseDown(BPoint point)
{
// printf("got a mouse\n");
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

bool CrystGLView::IsInMotion(void) {
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

void	CrystGLView::AttachedToWindow()
{
      LockGL(); 
      BGLView::AttachedToWindow(); 
      UnlockGL();
}

CrystGLWindow::CrystGLWindow(BRect frame, const char *name, CrystGLView *theview, csGraphics2DGLBe *piBeG2D)
//			: BWindow(frame,name, B_TITLED_WINDOW, B_NOT_RESIZABLE,0)
//			: BGLScreen(name, B_16_BIT_640x480, 0, &res, 0)
			: BDirectWindow(frame,name, B_TITLED_WINDOW, B_NOT_RESIZABLE, 0)
//			: BWindowScreen(name, B_8_BIT_640x480, res, 0)
{
	view = theview;
/*
	//	initialise local flags
	fConnected = false;
	fConnectionDisabled = false;
	fDrawingThreadSuspended=false;
	locker = new BLocker();*/ //dh:remove for conventional DirectConnected
	
	// cache the pointer to the 2D graphics driver object
	pi_BeG2D = piBeG2D;
	
	view->SetViewColor(0, 0, 0);
	AddChild(view);

	// Add a shortcut to switch in and out of fullscreen mode.
	AddShortcut('f', B_COMMAND_KEY, new BMessage('full'));
	
	// As we said before, the window shouldn't get wider than 2048 in any
	// direction, so those limits will do.
	SetSizeLimits(40.0, 2000.0, 40.0, 2000.0);
}

CrystGLWindow::~CrystGLWindow()
{
//	long		result;
}

bool CrystGLWindow::QuitRequested()
{
	bool sys_shutdown;
	
	pi_BeG2D->system->GetShutdown(sys_shutdown);
	printf ("entering CrystGLWindow::QuitRequested(): Shutdown is %d \n", sys_shutdown);
	
	pi_BeG2D->dpy->EnableDirectMode( false );
	be_app->PostMessage(B_QUIT_REQUESTED);
	
	// allow quit only when done from application itself
	// there's a problem in that while in this routine, the window is locked.
	// the shutdown flag can't work unless LoopThread gets to read it which it can't till this exits.
	pi_BeG2D->system->GetShutdown(sys_shutdown);
	
	printf ("exitting CrystGLWindow::QuitRequested(): Shutdown is %d \n", sys_shutdown);
	return sys_shutdown;
	
/*	
	// this window is destroyed, prevent the csGraphics2DGLBe::Close() method from trying to close it again!
	pi_BeG2D->window = NULL;
	
	// but don't destroy the window till LoopThread has had a chance to finish!
	pi_BeG2D->system->Shutdown();
	Unlock();//dh: will this allow things to finish?
	status_t exit_value;
	wait_for_thread(find_thread("LoopThread"), &exit_value);
	Lock();//dh: BWindows insist that the window is locked before blowing it away.
	
	return(TRUE);*/
}

void CrystGLWindow::MessageReceived(BMessage *message)
{
	switch(message->what) {
	// Switch between full-screen mode and windowed mode.
	case 'full' :
		SetFullScreen(!IsFullScreen());
		break;
	default :
		BWindow::MessageReceived(message);
		break;
	}
}

void CrystGLWindow::DirectConnected(direct_buffer_info *info)
{/*
printf("Entered CrystWindow::DirectConnected \n");
	if (!fConnected && fConnectionDisabled) {
		return S_OK;
	}
	locker->Lock();

	switch (info->buffer_state & B_DIRECT_MODE_MASK) {
		case B_DIRECT_START:
//			printf("DirectConnected: B_DIRECT_START \n");
			fConnected = true;
			if (fDrawingThreadSuspended)	{
				status_t retval;
				bool notdone=true;
				while (resume_thread(find_thread("LoopThread")) == B_BAD_THREAD_STATE)	{
					//	this is done to cope with thread setting fDrawingThreadSuspended then getting
					//	rescheduled before it can suspend itself.  It just makes repeated attempts to
					//	resume that thread.
					snooze(1000);
				}
				fDrawingThreadSuspended = false;
			}
				
		case B_DIRECT_MODIFY:
		break;
		
		case B_DIRECT_STOP:
//			printf("DirectConnected: B_DIRECT_STOP \n");
			fConnected = false;
		break;
	}
	
	locker->Unlock();
//    printf("leaving IXBeLibGraphicsInfo::DirectConnected \n");
	*/
//	let's try doing it with BGLView's DirectConnected method
	if (pi_BeG2D->dpy) {
		pi_BeG2D->dpy->DirectConnected(info);
		}
	pi_BeG2D->dpy->EnableDirectMode( true );// is this necessary
	
//	this bit just keeps conventional window behaviour until I've sorted out DirectConnected
//BDirectWindow::DirectConnected(info);
}

long CrystGLWindow::StarAnimation(void *data)
{
	return 0;
}

