/*
	
	CrystWindow.h
	
	by Pierre Raynaud-Richard.
	
	Copyright 1998 Be Incorporated, All Rights Reserved.
	
*/

#ifndef CRYST_GLIDE_WINDOW_H
#define CRYST_GLIDE_WINDOW_H

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


class CrystGlideView : public BView {

public:
		// standard constructor and destrcutor
				CrystGlideView(BRect frame); 
    virtual	    ~CrystGlideView();

		// standard window member
//virtual	void	MessageReceived(BMessage *message);
    virtual void KeyDown(const char *bytes, int32 numBytes);
    virtual void KeyUp(const char *bytes, int32 numBytes);
   	virtual void MouseDown(BPoint point);
  	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
    virtual void DirectConnected(direct_buffer_info *info);
   	bool IsInMotion(void);

	//	for initialisation
	virtual void AttachedToWindow();
	
	int 	inmotion;
	BPoint 	lastloc;
};

class csGraphics2DBeGlide;

class CrystGlideWindow : public BDirectWindow { // BGLScreen { //BWindow { // BWindowScreen {
friend class csGraphics2DGLBe;
public:
		// standard constructor and destrcutor
			CrystGlideWindow(BRect frame, const char *name, CrystGlideView *v, csGraphics2DBeGlide *p); 
virtual		~CrystGlideWindow();

		// standard window member
virtual	bool		QuitRequested();
virtual status_t	SetFullScreen(bool enable);
virtual	bool		IsFullScreen(void);
virtual	void		MessageReceived(BMessage *message);

virtual void	DirectConnected(direct_buffer_info *info);

		CrystGlideView		*view;
		status_t 		res;
		
		//	stuff to implement BDirectWindow
//		IBeLibGraphicsInfo	*piG2D;// new pointer to 2D driver info method interface.
		csGraphics2DBeGlide	*pi_BeG2D;//local copy of this pointer to csGraphics2DBeLib.
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
