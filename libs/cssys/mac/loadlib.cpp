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
#include "cssysdef.h"
#include "cssys/csshlib.h"

csLibraryHandle csFindLoadLibrary (const char *iName)
{
  return csFindLoadLibrary (NULL, iName, ".shlb");
}

csLibraryHandle csLoadLibrary (const char *iName)
{
  CFragConnectionID libID;
  OSErr theError;
  Str63 theLibName;
  Str255 errorString;
  void (*pfnDllInitialize)(void) = NULL;

  strcpy ((char *)&theLibName [1], iName);
  theLibName[0] = strlen ((char *)&theLibName [1]);

  theError = GetSharedLibrary (theLibName, kPowerPCCFragArch, kReferenceCFrag,
    &libID, (Ptr*)&pfnDllInitialize, errorString);

  if ((theError == noErr) && pfnDllInitialize)
    pfnDllInitialize ();

  return (csLibraryHandle)libID;
}

void *csGetLibrarySymbol( csLibraryHandle Handle, const char* iName )
{
	Str63				theSymbolName;
	CFragSymbolClass	theSymbolClass;
	void *				theSymbolAddress;
	OSErr				theError;

	strcpy( (char *)&theSymbolName[1], iName );
	theSymbolName[0] = strlen( iName );
    theError = FindSymbol( (CFragConnectionID)Handle, theSymbolName, (Ptr*)&theSymbolAddress, &theSymbolClass );

	if ( theError != noErr ) {
		theSymbolAddress = NULL;
	}

    return theSymbolAddress;
}

bool csUnloadLibrary(csLibraryHandle Handle)
{
	OSErr	theError;

	theError = CloseConnection( (CFragConnectionID *)&Handle );

	return ( theError == noErr );
}

void csPrintLibraryError (const char *iModule)
{
}
