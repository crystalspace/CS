/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddgError_Class_
#define _ddgError_Class_

#include "csterr/ddgstr.h"
#undef assert
/**
 *  A class to handle error codes and exiting on error.
 *  In general methods should return false if no error occured
 *  and true if there was an error.
 *  When an error occurs, the function should call Error::set
 *  and leave the message of what happened.
 *  At some point in the call stack the decision can be made to
 *  act on this error by calling Error::report or Error::exitHandler.
 */
class WEXP ddgError
{
	/// Global Error object.
	static ddgError *gError;
	/// Error code info.
	int _error;
	/// Error code info as a string.
	ddgStr _code;
	/// Line number at which error occured.
	int _line;
	/// File in which error occured.
	ddgStr _file;
	/// Error message.
	ddgStr _msg;
public:
	/// Constructor.
	ddgError(void);
	/// Destructor.
	~ddgError(void);
	/// Define some useful error codes.
	enum codes { Ok = 0,
		Failure = 1,
		Memory = 2,
		FileAccess = 3, FileRead = 4, FileWrite = 5, FileContent = 6,
		Render = 98,
		Unknown = 998,
		Warning = 999 };
	/// A function which will be called by the assert macros before exiting.
	static void exitHandler( void );
	/// Set error.
	static bool set(int code, char *msg, char* file, int line, char *codeMsg = NULL);
	/// Print out the error.
	static void report(void);
	/// Convienent macro to set error information.
	#define ddgErrorSet(a,b) ddgError::set(ddgError##::a,b,__FILE__,__LINE__, #a)
	#ifdef _DEBUG
	/// Generate warning only.
	#define assertw(a)    if (!(a)) {ddgErrorSet(Warning,"Assert failed!\n" #a);ddgError::report(); }
	/// Generate warning with string.
	#define assertws(a,s) if (!(a)) {ddgErrorSet(Warning,"Assert failed!\n" s "\n" #a);ddgError::report();}
	/// Generate error and exit with error 1.
	#define assert(a)     if (!(a)) {ddgErrorSet(Failure,"Assert failed!\n" #a);ddgError::exitHandler();}
	/// Generate error with string and exit with error 1.
	#define asserts(a,s)  if (!(a)) {ddgErrorSet(Failure,"Assert failed!\n" s "\n" #a);ddgError::exitHandler();}
	#else
	#define assertw(a)
	#define assert(a)
	#define asserts(a,s)
	#define assertws(a,s)
	#endif
};

#endif
