/*
    Copyright (C) 1998 by Jorrit Tyberghein and K. Robert Bate.
  
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

/*----------------------------------------------------------------
	Written by K. Robert Bate 1999.
----------------------------------------------------------------*/

#include <Devices.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Events.h>
#include <Fonts.h>
#include <LowMem.h>
#include <Menus.h>
#include <Quickdraw.h>
#include <StandardFile.h>
#include <TextUtils.h>
#include <ToolUtils.h>
#include <Windows.h>
#include <stdio.h>
#include <SIOUX.h>
#include "sysdef.h"
#include "cscom/com.h"

#define kAppleMenuID			128
#define kFileMenuID				129
#define kEditMenuID				130

#define kMenuBarID				128

#define kAlertOrWarningDialog	1024
#define kCommandLineDialog		1025
#define kDepthFailureDialog		1026
#define kChangeDepthDialog		1027
#define kAboutDialog			1028

#define kCommandLineString		1024

Str255	gAppName;
FSSpec	gAppLocation;
bool Shutdown = false;

extern CS_HLIBRARY csLoadLibrary (char *szLibName);
extern PROC csGetProcAddress (CS_HLIBRARY, char *);

OSErr AppleEventHandler( AppleEvent *event, AppleEvent *reply, long refCon );
AEEventHandlerUPP AppleEventHandlerUPP = NULL;
void Initialize( void );
void denitialize( void );
void HandleEvents( void );
void HandleMouseEvent( EventRecord *theEvent );
void DoAboutDialog( void );
void HandleMenuSelection( const short menuNum, const short itemNum );
void HandleKey( const char key, const char keycode, const short modifiers, bool down );
void HandleOSEvent( EventRecord *theEvent );
OSErr HandleAppleEvent( AppleEvent *theEvent );
void SendODOCToSelf( FSSpec *theFileSpec );
void SendRDOCToSelf( FSSpec *theFileSpec );

void main ( ) 
{
	Initialize();

	while ( ! Shutdown )
		HandleEvents();

	denitialize();
}

void Initialize( void )
{
	unsigned int		i;
	ProcessSerialNumber	theCurrentProcess;
	ProcessInfoRec		theInfo;
	Handle				theMenuBar;
	MenuHandle			theMenu;
	Str255				theText;

 	/*
 	 *	Initialize all the needed managers.
 	 */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	FlushEvents ( everyEvent, 0 );

	/*
	 *	Initialize sioux (console).
	 */
	SIOUXSettings.standalone = FALSE;
	SIOUXSettings.setupmenus = FALSE;
	SIOUXSettings.initializeTB = FALSE;
	SIOUXSettings.asktosaveonclose = FALSE;
	SIOUXSettings.autocloseonquit = FALSE;

	/*
	 *	Get the application name and location on disk.
	 */
	GetCurrentProcess( &theCurrentProcess );
	theInfo.processInfoLength = sizeof( ProcessInfoRec );
	theInfo.processName = gAppName;
	theInfo.processAppSpec = &gAppLocation;
	GetProcessInformation( &theCurrentProcess, &theInfo );
	gAppName[ gAppName[0] + 1 ] = '\0';

	/*
	 *	Setup the handler for the apple events.
	 */
	AppleEventHandlerUPP = NewAEEventHandlerProc( AppleEventHandler );
	if ( AppleEventHandlerUPP )
		AEInstallEventHandler( typeWildCard, typeWildCard, 
							  (AEEventHandlerUPP)AppleEventHandlerUPP, 0, FALSE );

	/*
	 *	Get the menu bar for this application
	 */

	theMenuBar = GetNewMBar( kMenuBarID );
	if ( theMenuBar ) {
		SetMenuBar( theMenuBar );

		/*
		 *	Add the items in the apple menu
		 */
		theMenu = GetMenuHandle( kAppleMenuID );
		if ( theMenu ) {
			AppendResMenu( theMenu, 'DRVR' );
		}
		strcpy( (char *)&theText[1], "About " );
		strcat(  (char *)&theText[1], (char *)&gAppName[1] );
		strcat(  (char *)&theText[1], "É" );
		theText[0] = strlen( (char *)&theText[1] );
		SetMenuItemText( theMenu, 1, theText );

		/*
		 *	Disable the edit menu
		 */
		theMenu = GetMenuHandle( kEditMenuID );
		if ( theMenu ) {
			DisableItem( theMenu, 0 );
		}

		/*
		 *	Draw the menu bar onto the screen
		 */
		DrawMenuBar();
	}
}


void denitialize( void )
{
	/*
	 *	If the handler for the apple events were setup,
	 *	remove them.
	 */
	if ( AppleEventHandlerUPP ) {
		AERemoveEventHandler( typeWildCard, typeWildCard, 
							  (AEEventHandlerUPP)AppleEventHandlerUPP, FALSE );
		DisposeRoutineDescriptor( AppleEventHandlerUPP );
		AppleEventHandlerUPP = NULL;
	}
}


void HandleEvents( void )
{
	EventRecord theEvent;
	Boolean outEventWasProcessed;

	if ( WaitNextEvent( everyEvent, &theEvent, 1, NULL ) ) {
		switch ( theEvent.what )
		{
			case mouseDown:
			case mouseUp:
				if ( ! SIOUXHandleOneEvent( &theEvent ))
					HandleMouseEvent( &theEvent );
				break;

			case activateEvt:
				SIOUXHandleOneEvent( &theEvent );
				break;

			case keyDown:
			case autoKey:
			case keyUp:
				{
					char theKey = theEvent.message & charCodeMask;
					char theCode = theEvent.message & keyCodeMask;
					HandleKey( theKey, theCode, theEvent.modifiers, theEvent.what != keyUp );
				}
				break;

			case updateEvt:
				SIOUXHandleOneEvent( &theEvent );
				break;

			case diskEvt:
				Point dPt = {100,100};					
				if( HiWord( theEvent.message ) != 0)
					DIBadMount( dPt, theEvent.message );
				break;

			case kHighLevelEvent:
				AEProcessAppleEvent( &theEvent );
				break;

			case osEvt:
				HandleOSEvent( &theEvent );
				break;

			default:
				break;
		}
	}
}


void HandleMouseEvent( EventRecord *theEvent )
{
	WindowPtr	targetWindow;
	short		partCode;
	long		menucode;
	Point		theMouse;
	
	// find out what part of the screen the mouse was clicked on
	
	partCode = FindWindow( theEvent->where, &targetWindow );
	
	switch (partCode)
	{	
		case inDesk:
			break;
		
		case inMenuBar:
			// pull down the menu and track it
			menucode = MenuSelect( theEvent->where );
			
			// handle the user's choice	
			if ( HiWord( menucode ))
				HandleMenuSelection( HiWord(menucode), LoWord(menucode) );
			HiliteMenu(0);

			break;
		
		case inSysWindow:
			SystemClick( theEvent, targetWindow );	// system will handle it (DA window)
			break;
	
		case inContent:
			if ( targetWindow )						// the user clicked in a window
			{
				if ( targetWindow != (WindowPtr)FrontWindow() ) {
					SelectWindow( targetWindow );
				}
			}
			break;

		case inDrag:
			// Drag the window.
			// Again, disallow this if the front window is modal.
			bool	dblBfrState;
			Rect r = (*GetGrayRgn())->rgnBBox;
			InsetRect( &r, 4, 4 );
			DragWindow( targetWindow, theEvent->where, &r );
			break;
		
		case inGrow:
		{
			// the user wants to resize the window
			// first, get min and max bounds
			Rect r;	// minimum and maximum window size
			SetRect( &r, 100, 50, 640, 800 );	// arbitrary bounds for windows!
			long newSize = GrowWindow( targetWindow, theEvent->where, &r );
			if (newSize) {
				short newHeight = HiWord(newSize);
				short newWidth = LoWord(newSize);
				SizeWindow( targetWindow, newWidth, newHeight, true );
				Rect invalr = {0,0,newHeight,newWidth};
				InvalRect( &invalr );
			}
			break;
		}

		case inGoAway:
			if( TrackGoAway( targetWindow, theEvent->where ))
			{
				// if the user clicks in the go-away box, the application will be shutdown.
				
				Shutdown = true;
			}
			break;
		
		case inZoomIn:
		case inZoomOut:
			if( TrackBox( targetWindow, theEvent->where, partCode ))
			{
				// if the user clicked in the zoom box, the window will be zoomed				
			}
			break;
	}

}


void DoAboutDialog( void )
{
	ParamText( gAppName, "\p", "\p", "\p" );
	Alert( kAboutDialog, NULL );
}


void HandleMenuSelection( const short menuNum, const short itemNum )
{
	// User has selected a menu command.
	// Here, we'll handle only a few default cases.
	if (menuNum == kAppleMenuID) {
		if (itemNum == 1) {
			// show the about box
			DoAboutDialog();
		} else {
			// launch Apple Menu item
			Str255 appleItemName;
			GetMenuItemText( GetMenuHandle(menuNum), itemNum, appleItemName );
			GrafPtr savePort;
			GetPort( &savePort );
			OpenDeskAcc( appleItemName );
			SetPort( savePort );
		}
		return;
	}
	
	if (menuNum == kFileMenuID) {
		if (itemNum == 1) {
			StandardFileReply	stdReply;
			SFTypeList			theTypeList;

			theTypeList[0] = 'shlb';
			StandardGetFile( NULL, 1, theTypeList, &stdReply );
			if ( stdReply.sfGood )
				SendODOCToSelf( &stdReply.sfFile );
		} else if (itemNum == 2) {
			StandardFileReply	stdReply;
			SFTypeList			theTypeList;

			theTypeList[0] = 'shlb';
			StandardGetFile( NULL, 1, theTypeList, &stdReply );
			if ( stdReply.sfGood )
				SendRDOCToSelf( &stdReply.sfFile );
		} else {
			// We'll assume that's the last item in the File menu.
			if (itemNum == CountMenuItems( GetMenuHandle(menuNum) )) {
				// QUIT!
				Shutdown = true;
			}
		}
	}
}


void HandleKey( const char key, const char /* keycode */, const short modifiers, bool /* down */ )
{
	// A key has been pressed -- handle typical cases.
	if (modifiers & cmdKey) {
		// command key was pressed -- check for menu shortcut.
		long menucode = MenuKey( key );		
		// handle the user's choice	
		if ( HiWord( menucode )) {
			HandleMenuSelection( HiWord(menucode), LoWord(menucode) );
			HiliteMenu(0);
			return;
		}
	}
}


void HandleOSEvent( EventRecord *theEvent )
{
	unsigned char	osEvtFlag;

	osEvtFlag = (unsigned char) (((unsigned long) theEvent->message) >> 24);
	if (osEvtFlag == suspendResumeMessage) {
		if (theEvent->message & resumeFlag) {
		} else {
			::HiliteMenu(0);				// Unhighlight menu titles
		}
	}
}


/************************************************************************************************
 *	AppleEventHandler
 *
 *		Callback for handling Apple Events
 */
OSErr AppleEventHandler( AppleEvent *event, AppleEvent *reply, long refCon )
{
#pragma unused( reply, refCon )

	return HandleAppleEvent( event );
}


/************************************************************************************************
 *	DoAppleEvent
 *
 */
OSErr HandleAppleEvent( AppleEvent *theEvent )
	{
	DescType	eventClass;
	DescType	eventID;
	DescType	actualType;
	long		actualSize;
	AEDesc		fileListDesc;
	OSErr		err;
	long		numFiles;
	int			i;
	AEKeyword	actualKeyword;
	FSSpec		filespec;
	FInfo		theFileInfo;
	short		refNum;
	long		count;
	
	// Get the event class		                      
	err = AEGetAttributePtr( theEvent, keyEventClassAttr,
				typeType, &actualType, (Ptr)&eventClass, 
				sizeof(eventClass), &actualSize );
	if ( err != noErr )
		return err;

	// Get the event ID
	err = AEGetAttributePtr( theEvent, keyEventIDAttr,
				typeType, &actualType, (Ptr)&eventID, 
				sizeof(eventID), &actualSize );
	if ( err != noErr )
		return err;
		                      
	if ( eventClass == kCoreEventClass ) {	
		switch (eventID) {
			case kAEOpenApplication:
				break;

			case kAEOpenDocuments:
			case kAEPrintDocuments:
				err = AEGetKeyDesc( theEvent, keyDirectObject, typeAEList, &fileListDesc );
				if (err == noErr) {					
					err = AECountItems( &fileListDesc, &numFiles );
					if (err == noErr) {
						for (i = 1; i <= numFiles; i++) {
							err = AEGetNthPtr( &fileListDesc, i, typeFSS, &actualKeyword,
												&actualType, (Ptr)&filespec, sizeof(filespec), &actualSize );
							if ( err == noErr ) {
								HRESULT hRes;
								PROC pfnDllRegisterServer = NULL;
								char	library_name[256];

								/*
								 *	Convert the name of the library to a c string from a pascal string.
								 */
								strncpy( library_name, (char *)&(filespec.name[1]), filespec.name[0] );
								library_name[ filespec.name[0] ] = '\0';

								printf ("%s.......",library_name);
								CS_HLIBRARY hLib = csLoadLibrary (library_name);

								if (!hLib)
								{
									printf ("Couldn't open library!\n");
									continue;
								}

								pfnDllRegisterServer = csGetProcAddress (hLib, "DllRegisterServer");
								if (!pfnDllRegisterServer)
								{
									printf ("Unable to find DllRegisterServer.\n");
									continue;
								}

								hRes = pfnDllRegisterServer ();
								if (FAILED (hRes))
								{
									printf ("DllRegisterServer failed.\n");
									continue;
								}

								printf ("DllRegisterServer succeeded.\n");
							}
						}
						AEDisposeDesc( &fileListDesc );
					}
				}
				break;

			case kAEQuitApplication:
				Shutdown = true;
				break;

			default:
				err = errAEEventNotHandled;		// We got an event we don't understand
				break;
		}
	} else if ( eventClass == 'CSrg' ) {
		if ( eventID == 'RDOC' ) {
			err = AEGetKeyDesc( theEvent, keyDirectObject, typeAEList, &fileListDesc );
			if (err == noErr) {					
				err = AECountItems( &fileListDesc, &numFiles );
				if (err == noErr) {
					for (i = 1; i <= numFiles; i++) {
						err = AEGetNthPtr( &fileListDesc, i, typeFSS, &actualKeyword,
											&actualType, (Ptr)&filespec, sizeof(filespec), &actualSize );
						if ( err == noErr ) {
							HRESULT hRes;
							PROC pfnDllRegisterServer = NULL;
							char	library_name[256];

							/*
							 *	Convert the name of the library to a c string from a pascal string.
							 */
							strncpy( library_name, (char *)&(filespec.name[1]), filespec.name[0] );
							library_name[ filespec.name[0] ] = '\0';

							printf ("%s.......",library_name);
							CS_HLIBRARY hLib = csLoadLibrary (library_name);

							if (!hLib)
							{
								printf ("Couldn't open library!\n");
								continue;
							}

							pfnDllRegisterServer = csGetProcAddress (hLib, "DllUnregisterServer");
							if (!pfnDllRegisterServer)
							{
								printf ("Unable to find DllUnregisterServer.\n");
								continue;
							}

							hRes = pfnDllRegisterServer ();
							if (FAILED (hRes))
							{
								printf ("DllUnregisterServer failed.\n");
								continue;
							}

							printf ("DllUnregisterServer succeeded.\n");
						}
						AEDisposeDesc( &fileListDesc );
					}
				}
			}
		} else {
			err = errAEEventNotHandled;		// We got an event we don't understand
		}
	} else {
		err = errAEEventNotHandled;		// We got an event we don't understand
	}
	
	return err;
}


void SendODOCToSelf( FSSpec *theFileSpec )
{

	OSErr			err;
	AEAddressDesc	theTarget;
	AppleEvent		openEvent, replyEvent;
	ProcessSerialNumber	psn;

	/*
	 *	First we create the target for the event.
	 */

	psn.highLongOfPSN 	= 0;
	psn.lowLongOfPSN 	= kCurrentProcess;
	err = AECreateDesc( typeProcessSerialNumber, (Ptr)&psn, sizeof(ProcessSerialNumber), &theTarget);

	if (err == noErr) {
		/* Next we create the Apple event that will later get sent. */
		err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, &theTarget, kAutoGenerateReturnID, kAnyTransactionID, &openEvent);

		if (err == noErr) {
			err = AEPutParamPtr(&openEvent, keyDirectObject, typeFSS,
							theFileSpec, sizeof(FSSpec));

			if (err == noErr) {
				/*
				 *	and finally send the event
				 *	Since we are sending to ourselves, no need for reply.
				 */
				err = AESend(&openEvent, &replyEvent, kAENoReply + kAECanInteract, kAENormalPriority, 3600, NULL, NULL);
			}

			/*and of course dispose of the openDoc AEVT itself*/
			err = AEDisposeDesc(&openEvent);
		}
	}
}


void SendRDOCToSelf( FSSpec *theFileSpec )
{

	OSErr			err;
	AEAddressDesc	theTarget;
	AppleEvent		openEvent, replyEvent;
	ProcessSerialNumber	psn;

	/*
	 *	First we create the target for the event.
	 */

	psn.highLongOfPSN 	= 0;
	psn.lowLongOfPSN 	= kCurrentProcess;
	err = AECreateDesc( typeProcessSerialNumber, (Ptr)&psn, sizeof(ProcessSerialNumber), &theTarget);

	if (err == noErr) {
		/* Next we create the Apple event that will later get sent. */
		err = AECreateAppleEvent('CSrg', 'RDOC', &theTarget, kAutoGenerateReturnID, kAnyTransactionID, &openEvent);

		if (err == noErr) {
			err = AEPutParamPtr(&openEvent, keyDirectObject, typeFSS,
							theFileSpec, sizeof(FSSpec));

			if (err == noErr) {
				/*
				 *	and finally send the event
				 *	Since we are sending to ourselves, no need for reply.
				 */
				err = AESend(&openEvent, &replyEvent, kAENoReply + kAECanInteract, kAENormalPriority, 3600, NULL, NULL);
			}

			/*and of course dispose of the openDoc AEVT itself*/
			err = AEDisposeDesc(&openEvent);
		}
	}
}
