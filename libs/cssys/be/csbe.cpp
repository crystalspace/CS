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
#include <stdarg.h>
#include <time.h>
#include <Beep.h>
#include <sys/param.h>

#include "sysdef.h"
#include "cssys/common/system.h"
#include "csbe.h"
#include "csutil/inifile.h"
//#include "cs3d/software/graph3d.h"	//@@@VERY DOUBTFUL! This should not be needed!!!

#include "cs2d/be/isysg2d.h"		//@@@WHY?
#include "cs2d/be/CrystWindow.h"	//@@@WHY?
static bool LoopStarted = false;// For Print() to know whenever to flush output
#define Gfx2D System->piG2D			//	put in by DH

// The System driver ////////////////

BEGIN_INTERFACE_TABLE(SysSystemDriver)
  IMPLEMENTS_COMPOSITE_INTERFACE(System)
  IMPLEMENTS_COMPOSITE_INTERFACE(BeLibSystemDriver)
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(SysSystemDriver)

/*
 * Signal handler to clean up and give some
 * final debugging information.
 */
extern void debug_dump ();
extern void cleanup ();

/* not too much to be said... */
class CrystApp : public BApplication {
public:
	CrystApp(SysSystemDriver *from) : BApplication("application/x-vnd.xsware-crystal") {
		driver = from;
		mspf = 100 * 1000;
		time = system_time();//dhdebug
	};
	void Pulse(void);
	bool QuitRequested();
	void CrystApp::MessageReceived(BMessage *message);

	void doMouseAction(BPoint point, int16 button, bool shift, bool alt, bool ctrl);
	void doKeyAction(int key, bool down, bool shift, bool alt, bool ctrl);
	bigtime_t mspf;
	bigtime_t time;//dhdebug
private:
	
	SysSystemDriver	*driver;
	BWindow			*aWindow;
	bigtime_t		milisecpf;
};

void handler (int sig)
{
  static bool in_exit = false;
  if (in_exit)
    exit (1);
  in_exit = true;

  if (sig == SIGINT)
    fprintf (stderr, "\n^C\n");
  else
    fprintf (stderr, "SIGNAL %d CAUGHT!!!\n", sig);

  int err;
  err = errno;
  printf ("error %d: [%s]\n", err, strerror (err));

  if (sig != SIGINT)
    debug_dump ();

  cleanup ();
  exit (1);
} 

void init_sig ()
{
  signal (SIGHUP, handler);
  signal (SIGINT, handler);
//  signal (SIGTRAP, handler);
  signal (SIGABRT, handler);
  signal (SIGALRM, handler);
  signal (SIGTERM, handler);
  signal (SIGPIPE, handler);
  signal (SIGSEGV, handler);
  signal (SIGBUS, handler);
  signal (SIGFPE, handler);
  signal (SIGILL, handler);
}

static CrystApp *app = NULL;
static bool firstRun;

bool CrystApp::QuitRequested()
{
	status_t err_code, exit_value;
	
	printf("CrystApp::QuitRequested() entered. \n");
	
	// shutdown rendering thread first.
	driver->Shutdown = true;
	snooze(200000);
	err_code = wait_for_thread(find_thread("LoopThread"), &exit_value);//put in because I have moved drawing into that thread.
	
	printf("LoopThread blown away. err code is %lx \n", err_code);
	return(TRUE);
}

void CrystApp::Pulse() {
//	bigtime_t		time;

//	time = system_time()+milisecpf;
//PostMessage('next');
//    driver->NextFrame();
//	time -= system_time();
#if 0
	if( time < 0) {
		SetPulseRate(1000000/reqfps);
	}
    if(g2d->win->Lock()){
    	if(g2d->view->IsInMotion())
    		Mouse->do_mousemotion (SysGetTime (), g2d->view->lastloc.x, g2d->view->lastloc.y);
    	g2d->win->Unlock();
    }
#endif
}

void CrystApp::MessageReceived(BMessage *msg)
{	
//	bigtime_t	time = system_time();
//	bigtime_t pre_draw_microsecs, post_draw_microsecs;//dhdebug
//	static long prev_frame_time = csSystemDriver::Time();//dhdebug
//	long curr_time;//dhdebug
//	printf("got something %4s\n",&msg->what);
	switch(msg->what) {
	// Switch between full-screen mode and windowed mode.
/*	case 'next' :
		{
//		driver->NextFrame(system_time () - time, system_time ());
		pre_draw_microsecs = system_time();
		curr_time = pre_draw_microsecs / 1000;
		driver->NextFrame(curr_time - prev_frame_time, curr_time);//dhdebug
		post_draw_microsecs = system_time();
		printf("curr_time, prev_frame_time: %u  %u \n", curr_time, prev_frame_time);//dhdebug
//		time = system_time() - time;
		prev_frame_time = curr_time;//dhdebug
//		mspf = time;
		mspf = post_draw_microsecs - pre_draw_microsecs;//dhdebug
		BMessageQueue *queu = MessageQueue();
		printf("queue size is %d \n", queu->CountMessages());
		BMessage *mmsg = NULL;
		int32 i,nxt = 'next';
		i=0;
		while((mmsg = queu->FindMessage(nxt, 1))){
			queu->RemoveMessage(mmsg);
			printf("removed a next message");
			i++;
		}
//			printf("%4d was queed\n",i);
		}
		break;*/
	case 'keys' :
		doKeyAction(msg->FindInt16("key"), msg->FindBool("down"),
			msg->FindBool("shift"),msg->FindBool("alt"),msg->FindBool("ctrl"));
		break;
	case 'mous' :
		doMouseAction(msg->FindPoint("loc"),msg->FindInt16("butn"),
			msg->FindBool("shift"),msg->FindBool("alt"),msg->FindBool("ctrl"));
		break;
	default :
		BApplication::MessageReceived(msg);
		break;
	}
}

void CrystApp::doMouseAction(BPoint point, int16 button, bool shift, bool alt, bool ctrl) 
{
//  		Mouse->do_mousemotion (g2d->view->lastloc.x, g2d->view->lastloc.y);
	if(button)
		driver->Mouse->do_buttonpress (SysGetTime (), button, point.x, point.y, shift, alt, ctrl);
	else
		driver->Mouse->do_buttonrelease (SysGetTime (), button, point.x, point.y);
}

void CrystApp::doKeyAction(int key, bool down, bool shift, bool alt, bool ctrl)
{
	switch (key) {
//		case B_Alt_L:
//		case B_Alt_R:      key = CSKEY_ALT; break;
//		case B_Control_L:
//		case B_Control_R:  key = CSKEY_CTRL; break;
//		case B_Shift_L:
//		case B_Shift_R:    key = CSKEY_SHIFT; break;
		case B_UP_ARROW:         	key = CSKEY_UP; break;
		case B_DOWN_ARROW:       	key = CSKEY_DOWN; break;
		case B_LEFT_ARROW:       	key = CSKEY_LEFT; break;
	    case B_RIGHT_ARROW:      	key = CSKEY_RIGHT; break;
	    case B_BACKSPACE:  			key = CSKEY_BACKSPACE; break;
	    case B_INSERT:     			key = CSKEY_INS; break;
	    case B_DELETE:     			key = CSKEY_DEL; break;
	    case B_PAGE_UP:    			key = CSKEY_PGUP; break;
	    case B_PAGE_DOWN:  			key = CSKEY_PGDN; break;
	    case B_HOME:       			key = CSKEY_HOME; break;
	    case B_END:        			key = CSKEY_END; break;
	    case B_ESCAPE:     			key = CSKEY_ESC; break;
	    case B_TAB:        			key = CSKEY_TAB; break;
	    case B_RETURN:     			key = CSKEY_ENTER; break;
//	    case B_F1_KEY:				key = CSKEY_F1; break;
//	    case B_F2_KEY:				key = CSKEY_F2; break;
//	    case B_F3_KEY:				key = CSKEY_F3; break;
//	    case B_F4_KEY:				key = CSKEY_F4; break;
//	    case B_F5_KEY:				key = CSKEY_F5; break;
//	    case B_F6_KEY:				key = CSKEY_F6; break;
//	    case B_F7_KEY:				key = CSKEY_F7; break;
//	    case B_F8_KEY:				key = CSKEY_F8; break;
//	    case B_F9_KEY:				key = CSKEY_F9; break;
//	    case B_F10_KEY:				key = CSKEY_F10; break;
//	    case B_F11_KEY:				key = CSKEY_F11; break;
//	    case B_F12_KEY:				key = CSKEY_F12; break;
        default:            		break;
	}
	if (down)
		driver->Keyboard->do_keypress (SysGetTime (), key); //, shift, alt, ctrl);
	else
		driver->Keyboard->do_keyrelease (SysGetTime (), key);
}


SysSystemDriver::SysSystemDriver () : csSystemDriver()
{
  CHK (app = new CrystApp(this));
  firstRun = true;
//  void init_sig ();
//  init_sig ();
};

long csSystemDriver::Time()
{
  return system_time()/1000;
}

void SysSystemDriver::SetSystemDefaults ()
{
  csSystemDriver::SetSystemDefaults ();

  SimDepth = 0;
  //FULL_SCREEN = false;
  if (config)
  {
    SimDepth = config->GetInt ("VideoDriver", "SIMULATE_DEPTH", SimDepth);
//  UseDW = config->GetYesNo ("VideoDriver", "DIRECT_WINDOW", 0);
//  HardwareCursor = config->GetYesNo ("VideoDriver", "SYS_MOUSE_CURSOR", 1);
  }
}

bool SysSystemDriver::ParseArg (int argc, char* argv[], int &i)
{
  if (strcasecmp ("-sdepth", argv[i]) == 0)
  {
    i++;
    sscanf (argv[i], "%d", &SimDepth);
    if (SimDepth != 8 && SimDepth != 15 && SimDepth != 16 && SimDepth != 32)
    {
      Printf (MSG_FATAL_ERROR, "Crystal Space can't run in this simulated depth! (use 8, 15, 16, or 32)!\n");
      return false;
    }
  }
  else
    return csSystemDriver::ParseArg (argc, argv, i);
  return true;
}

void SysSystemDriver::Help ()
{
  csSystemDriver::Help ();
  Printf (MSG_STDOUT, "  -sdepth <depth>    set simulated depth (8, 15, 16, or 32) (default=none)\n");
}

void SysSystemDriver::SysFocusChange (void *Self, int Enable)
{
  if (((SysSystemDriver *)Self)->EventQueue)
    ((SysSystemDriver *)Self)->do_focus (Enable);
}

static int32 begin_loop(void* data)
{
  return ((SysSystemDriver*)data)->LoopThread();
}

// System loop !
void SysSystemDriver::Loop(void)
{
	LoopStarted = true;
	bigtime_t		time;
	BView *dpy;
	int	reqfps = 8;
	thread_id	my_thread;

	IBeLibGraphicsInfo* piG2D = NULL;
	HRESULT hRes = Gfx2D->QueryInterface ((REFIID)IID_IBeLibGraphicsInfo, (void**)&piG2D);
	if (SUCCEEDED(hRes))
	{
    hRes = piG2D->GetDisplay (&dpy);
		piG2D->Release ();
		piG2D = NULL;
	}

	if(firstRun) {
		firstRun = false;
		my_thread = spawn_thread(begin_loop, "LoopThread",
							 B_DISPLAY_PRIORITY, (void*)this);
		resume_thread(my_thread);
//		app->SetPulseRate(1000000/reqfps);//dhdebug
		app->Run();
	} else while (!Shutdown && !ExitLoop) {
		// we want a frame to take at least 16 ms.
		time = system_time()+1000000/reqfps;
//		NextFrame();
#if 0
	    if(g2d->win->Lock()){
	    	if(g2d->view->IsInMotion())
	    		Mouse->do_mousemotion (SysGetTime (), g2d->view->lastloc.x, g2d->view->lastloc.y);
	    	g2d->win->Unlock();
	    }
#endif
		time -= system_time();
		if (time > 0)
			snooze(time);
	}
}


// COM Implementation

IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED( SysSystemDriver, BeLibSystemDriver )

STDMETHODIMP SysSystemDriver::XBeLibSystemDriver::GetSettings (int &SimDepth,
  bool &UseSHM, bool &HardwareCursor)
{
  METHOD_PROLOGUE (SysSystemDriver, BeLibSystemDriver)
  SimDepth = pThis->SimDepth;
  UseSHM = pThis->UseSHM;
  HardwareCursor = pThis->HardwareCursor;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XBeLibSystemDriver::GetKeyboardHandler (
  BeKeyboardHandler &Handler, void *&Parm)
{
  METHOD_PROLOGUE (SysSystemDriver, BeLibSystemDriver)
  Handler = SysKeyboardDriver::Handler;
  Parm = pThis->Keyboard;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XBeLibSystemDriver::GetMouseHandler (
  BeMouseHandler &Handler, void *&Parm)
{
  METHOD_PROLOGUE (SysSystemDriver, BeLibSystemDriver)
  Handler = SysMouseDriver::Handler;
  Parm = pThis->Mouse;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XBeLibSystemDriver::GetFocusHandler (
  BeFocusHandler &Handler, void *&Parm)
{
  METHOD_PROLOGUE(SysSystemDriver, BeLibSystemDriver)
  Handler = SysSystemDriver::SysFocusChange;
  Parm = pThis;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XBeLibSystemDriver::SetLoopCallback (
  LoopCallback Callback, void *Param)
{
  METHOD_PROLOGUE(SysSystemDriver, BeLibSystemDriver)
  pThis->Callback = Callback;
  pThis->CallbackParam = Param;
  return S_OK;
}

/*********************************************************/

// Keyboard fonctions
SysKeyboardDriver::SysKeyboardDriver() : csKeyboardDriver ()
{
  // Create your keyboard interface
}

void SysKeyboardDriver::Handler(void *Self, int Key, bool Down)
{
  csKeyboardDriver *Keyboard = (csKeyboardDriver *)Self;
  if (!Keyboard->Ready ())
    return;
    
  if (Key)
  {
    if (Down)
      Keyboard->do_keypress (System->Time (), Key);
    else
      Keyboard->do_keyrelease (System->Time (), Key);
  } /* endif */
}

//bool SysKeyboardDriver::Open(csEventQueue *EvQueue)
//{
//  csKeyboardDriver::Open (EvQueue);
  // Open your keyboard interface
//  return true;
//}

//void SysKeyboardDriver::Close ()
//{
//  csKeyboardDriver::Close();
//  // Close your keyboard interface
//}

// Mouse fonctions
SysMouseDriver::SysMouseDriver() : csMouseDriver ()
{
// Initialize mouse system
}

void SysMouseDriver::Handler (void *Self, int Button, int Down, int x, int y,
  int ShiftFlags)
{
  csMouseDriver *Mouse = (csMouseDriver *)Self;
  if (!Mouse->Ready ())
    return;
    
  if (Button == 0)
    Mouse->do_mousemotion (System->Time (), x, y);
  else if (Down)
    Mouse->do_buttonpress (System->Time (), Button, x, y, ShiftFlags & CSMASK_SHIFT,
      ShiftFlags & CSMASK_ALT, ShiftFlags & CSMASK_CTRL);
  else
    Mouse->do_buttonrelease (System->Time (), Button, x, y);
}

//bool SysMouseDriver::Open(csEventQueue *EvQueue)
//{
//  csMouseDriver::Open (EvQueue);
// Open mouse system
//  return 1;
//}

//void SysMouseDriver::Close()
//{
// Close mouse system
//}


//@@@ JORRIT: this is a bad place but I don't know where to define it otherwise
bool ModuleIsStopping ()
{
  return false;
}


// This is the thread doing the loop itself.
long SysSystemDriver::LoopThread()
{
//	bigtime_t			delay;
	static bigtime_t	prev_frame = Time();
	bigtime_t			curr_time;
//	SysSystemDriver		*sys = this;
//	bool				*fConnected, *fConnectionDisabled;
//	bool	shutdown = false;
	
	snooze(100000);
	
	// initialise pointer-pointers
/*
	IBeLibGraphicsInfo* piG2D = NULL;
	HRESULT hRes = Gfx2D->QueryInterface ((REFIID)IID_IBeLibGraphicsInfo, (void**)&piG2D);
	if (FAILED(hRes))	{
		printf("Loopthread: can't get access to 2d graphics driver.\n");
		exit(1);
	}*/
//	piG2D->GetfConnected(&fConnected);
//	piG2D->GetfConnectionDisabled(&fConnectionDisabled);
		
	// loop, frame after frame, until asked to quit.
	while (!Shutdown /* && !(*fConnectionDisabled)*/) {
	long render_time;
	bigtime_t before,after;
//		printf("LoopThread: loop executing\n");
//		piG2D->SetFrameBufferLock(true);// change to implement BeginDraw/FinishDraw
//		if (*fConnected)	{		// change to implement BeginDraw/FinishDraw
		// get the right to do direct screen access.
		//acquire_sem(w->drawing_lock);
//		app->PostMessage('next');//dhdebug
//		printf("LoopThread: NextFrame executing\n");
		curr_time = Time();
		before = curr_time;
		/*driver->*/NextFrame(curr_time - prev_frame, curr_time);
		after = Time();
		render_time = (after - before);
//		printf ("render time is %d milliseconds.\n", render_time);
		prev_frame=curr_time;
//		if(be_app->Lock()) {
//			shutdown = sys->Shutdown;
//			delay = sys->mspf;
//			be_app->Unlock();
//		}
//		if(delay>0)
//			snooze(delay);
//	snooze(app->mspf);//dhdebug

		// release the direct screen access
		//release_sem(w->drawing_lock);
//		}// change to implement BeginDraw/FinishDraw
//		piG2D->SetFrameBufferLock(false);// change to implement BeginDraw/FinishDraw
	}	
	printf("LoopThread: Shutdown detected.\n");//dh
	return 0;
}
