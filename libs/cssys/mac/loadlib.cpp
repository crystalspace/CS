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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <CodeFragments.h>
#include "sysdef.h"
#include "comdefs.h"

extern CS_HLIBRARY SysLoadLibrary (char *szLibName);
extern PROC SysGetProcAddress (CS_HLIBRARY Handle, char *szProcName);
extern bool SysFreeLibrary (CS_HLIBRARY Handle);

CS_HLIBRARY SysLoadLibrary( char* szLibName )
{
	CFragConnectionID libID;
	OSErr	theError;
	Str63	theLibName;
	Str255	errorString;
    HRESULT (*pfnDllInitialize)(void) = NULL;

	strcpy( (char *)&theLibName[1], szLibName );
	theLibName[0] = strlen( szLibName );

    theError = GetSharedLibrary( theLibName, kPowerPCCFragArch, kReferenceCFrag, &libID, (Ptr*)&pfnDllInitialize, errorString );

    if ( theError == noErr )
    {
		if (!pfnDllInitialize) {
			return 0;
		}

		(*pfnDllInitialize)();
	}

	return (CS_HLIBRARY)libID;
}

PROC SysGetProcAddress( CS_HLIBRARY hLib, char* szProcName )
{
	Str63				theSymbolName;
	CFragSymbolClass	theSymbolClass;
	PROC				theSymbolAddress;
	OSErr				theError;

	strcpy( (char *)&theSymbolName[1], szProcName );
	theSymbolName[0] = strlen( szProcName );
    theError = FindSymbol( (CFragConnectionID)hLib, theSymbolName, (Ptr*)&theSymbolAddress, &theSymbolClass );

	if ( theError != noErr ) {
		theSymbolAddress = NULL;
	}

    return theSymbolAddress;
}

bool SysFreeLibrary(CS_HLIBRARY hLib)
{
	OSErr	theError;

	theError = CloseConnection( (CFragConnectionID *)&hLib );

	return ( theError == noErr );
}
