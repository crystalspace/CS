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
#include "cssys/common/system.h"

#define	MAX_ARGS	25

class IMacGraphicsInfo;

class SysSystemDriver : public csSystemDriver
{
public:
				 SysSystemDriver();
	virtual		~SysSystemDriver();

	virtual void Close();

	virtual bool Initialize (int argc, char *argv[], IConfig* cfg_engine);
	virtual void SetSystemDefaults(void);
	virtual bool ParseArg(int argc, char* argv[], int &i);
	virtual void SystemHelp(void);
	virtual void Loop();

	virtual void Alert(const char* s);
	virtual void Warn(const char* s);

	OSErr		HandleAppleEvent( AppleEvent *theEvent );

private:
	void		DispatchEvent( long current_time, EventRecord *theEvent, IMacGraphicsInfo* piG2D );
	void		HandleMouseEvent( long current_time, EventRecord *theEvent, IMacGraphicsInfo* piG2D );
	void		HandleMenuUpdate( void );
	void		HandleMenuSelection( const short menuNum, const short itemNum );
	void		HandleKey( long current_time, const char key, const char keycode, const short modifiers, bool down );
	void		HandleHLEvent( long current_time, EventRecord *theEvent );
	void		HandleOSEvent( long current_time, EventRecord *theEvent, IMacGraphicsInfo* piG2D );
	void		ScanKeyboard( long current_time );
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
