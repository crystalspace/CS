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
#include <DirectWindow.h>

//#include <OS.h>


class CrystView : public BView {

public:
		// standard constructor and destrcutor
				CrystView(BRect frame); 
virtual			~CrystView();

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
class csGraphics2DBeLib;

class CrystWindow : public BDirectWindow { // BWindowScreen {

public:
		// standard constructor and destructor
				CrystWindow(BRect frame, const char *name, CrystView *v, csGraphics2DBeLib *piBeG2D); 
virtual			~CrystWindow();

		// standard window member
virtual	bool	QuitRequested();
virtual	void	MessageReceived(BMessage *message);

virtual void	DirectConnected(direct_buffer_info *info);

		CrystView		*view;
		//	stuff to implement BDirectWindow
		IBeLibGraphicsInfo	*piG2D;// new pointer to 2D driver info method interface.
		csGraphics2DBeLib	*pi_BeG2D;//local copy of this pointer to csGraphics2DBeLib.
		
		// the drawing thread function.
static	long	StarAnimation(void *data);
};


#endif
