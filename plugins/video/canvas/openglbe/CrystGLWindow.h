/*
	
	CrystWindow.h
	
	by Pierre Raynaud-Richard.
	
	Copyright 1998 Be Incorporated, All Rights Reserved.
	
*/

#ifndef CRYST_WINDOW_H
#define CRYST_WINDOW_H

#ifndef _APPLICATION_H
#include <Application.h>
#endif

#ifndef _WINDOW_H
#include <Window.h>
#endif
#ifndef _VIEW_H
#include <View.h>
#endif
#ifndef _BITMAP_H
#include <Bitmap.h>
#endif
//#include <WindowScreen.h>
//#include <DirectWindow.h>
#include <GLView.h>

//#include <OS.h>


class CrystGLView : public BGLView {

public:
		// standard constructor and destrcutor
				CrystGLView(BRect frame); 
virtual			~CrystGLView();

		// standard window member
//virtual	void	MessageReceived(BMessage *message);
    virtual void KeyDown(const char *bytes, int32 numBytes);
    virtual void KeyUp(const char *bytes, int32 numBytes);
   	virtual void MouseDown(BPoint point);
  	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);

   	bool IsInMotion(void);

	int 	inmotion;
	BPoint 	lastloc;
};

class CrystGLWindow : public BWindow { // BGLScreen { //BDirectWindow { // BWindowScreen {

public:
		// standard constructor and destrcutor
				CrystGLWindow(BRect frame, const char *name, CrystGLView *v); 
virtual			~CrystGLWindow();

		// standard window member
virtual	bool	QuitRequested();
virtual	void	MessageReceived(BMessage *message);

		CrystGLView		*view;
		status_t 		res;
		// the drawing thread function.
static	long	StarAnimation(void *data);
};


#endif
