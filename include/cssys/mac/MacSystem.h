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
	Written by K. Robert Bate 1998.
----------------------------------------------------------------*/

#ifndef MACSYSTEM_H
#define MACSYSTEM_H

#include <AppleEvents.h>
#include <Events.h>
#include <Files.h>
#include "cssys/system.h"

#define	MAX_ARGS	25

class iMacGraphics;
struct iObjectRegistry;

class SysSystemDriver : public csSystemDriver, public iEventPlug
{
public:
			SysSystemDriver(iObjectRegistry* object_reg);
	virtual		~SysSystemDriver();

	virtual void Close();

	virtual bool Initialize ( int argc, const char* const argv[]);
	virtual void Loop ();

	virtual void Alert( const char* s );
	virtual void Warn( const char* s );

	OSErr		HandleAppleEvent( const AppleEvent *theEvent );

	SCF_DECLARE_IBASE_EXT (csSystemDriver);

	//------------------------- iEventPlug interface ---------------------------//

	virtual unsigned GetPotentiallyConflictingEvents ()
	{ return CSEVTYPE_Keyboard | CSEVTYPE_Mouse; }
	virtual unsigned QueryEventPriority (unsigned /*iType*/)
	{ return 100; }

private:
	void		DispatchEvent( EventRecord *theEvent, iMacGraphics* piG2D );
	void		HandleMouseEvent( EventRecord *theEvent, iMacGraphics* piG2D );
	void		HandleMenuUpdate( void );
	void		HandleMenuSelection( const short menuNum, const short itemNum );
	void		HandleKey( const unsigned char key, const char keycode, const short modifiers, bool down );
	void		HandleHLEvent( EventRecord *theEvent );
	void		HandleOSEvent( EventRecord *theEvent, iMacGraphics* piG2D );
	void		ScanKeyboard( );
	int			GetCommandLine( char ***arg );
	int			ParseCommandLine( char *s );

	void		DoAboutDialog( void);

	Str255		mAppName;
	FSSpec		mAppLocation;

	bool		mInputSprocketsAvailable;
	bool		mInputSprocketsRunning;

	bool			mDriverNeedsEvent;
	iGraphics2D*	mG2D;
	iMacGraphics*	mIG2D;

	char		CommandLine[256];
	char		argStr[256];
	char 		*argv[MAX_ARGS + 1];

	long		mKeyboardState[4];

	iEventOutlet	*EventOutlet;
};

#endif /* MACSYSTEM_H */
