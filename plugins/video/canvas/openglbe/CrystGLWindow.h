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

	//	for intialisation
	virtual void AttachedToWindow();
	
	int 	inmotion;
	BPoint 	lastloc;
};

class csGraphics2DGLBe;

class CrystGLWindow : public BDirectWindow { // BGLScreen { //BWindow { // BWindowScreen {
friend class csGraphics2DGLBe;
public:
		// standard constructor and destrcutor
				CrystGLWindow(BRect frame, const char *name, CrystGLView *v, csGraphics2DGLBe *p); 
virtual			~CrystGLWindow();

		// standard window member
virtual	bool	QuitRequested();
virtual	void	MessageReceived(BMessage *message);

virtual void	DirectConnected(direct_buffer_info *info);

		CrystGLView		*view;
		status_t 		res;
		
		//	stuff to implement BDirectWindow
//		IBeLibGraphicsInfo	*piG2D;// new pointer to 2D driver info method interface.
		csGraphics2DGLBe	*pi_BeG2D;//local copy of this pointer to csGraphics2DBeLib.
protected:
  BLocker		*locker;
  bool			fDirty;
  bool			fConnected;
  bool			fConnectionDisabled;
  bool			fDrawingThreadSuspended;
		
		// the drawing thread function.
public:
static	long	StarAnimation(void *data);
};


#endif
