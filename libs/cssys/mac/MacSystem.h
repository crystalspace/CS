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

class iMacGraphicsInfo;

class SysSystemDriver : public csSystemDriver
{
public:
				 SysSystemDriver();
	virtual		~SysSystemDriver();

	virtual void Close();

	virtual bool Initialize ( int argc, char *argv[], const char *iConfigName );
	virtual void Loop();

	virtual void Alert( const char* s );
	virtual void Warn( const char* s );

	OSErr		HandleAppleEvent( AppleEvent *theEvent );

	DECLARE_IBASE;

private:
	void		DispatchEvent( time_t current_time, EventRecord *theEvent, iMacGraphicsInfo* piG2D );
	void		HandleMouseEvent( time_t current_time, EventRecord *theEvent, iMacGraphicsInfo* piG2D );
	void		HandleMenuUpdate( void );
	void		HandleMenuSelection( const short menuNum, const short itemNum );
	void		HandleKey( time_t current_time, const char key, const char keycode, const short modifiers, bool down );
	void		HandleHLEvent( time_t current_time, EventRecord *theEvent );
	void		HandleOSEvent( time_t current_time, EventRecord *theEvent, iMacGraphicsInfo* piG2D );
	void		ScanKeyboard( time_t current_time );
	int			GetCommandLine( char ***arg );
	int			ParseCommandLine( char *s );

	void		DoAboutDialog( void);

	Str255		mAppName;
	FSSpec		mAppLocation;

	char		CommandLine[256];
	char		argStr[256];
	char 		*argv[MAX_ARGS + 1];

	long		mKeyboardState[4];
};

#endif /* MACSYSTEM_H */
