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

#include <stdio.h>
#include "sysdef.h"
#include "csterr/ddgerror.h"

// ----------------------------------------------------------------------

// Global error object to ensure gError class gets initialized.
ddgError *ddgError::gError = NULL;

ddgError::~ddgError(void)
{
}

ddgError::ddgError(void)
{
	gError = this;
	/// Global Error code info.
	_error = Unknown;
	// The error code as a string.
	_code.assign("-");
	/// Line number at which error occured.
	_line = 0;
	/// File in which error occured.
	_file.assign("-");
	/// Error message.
	_msg.assign("-");
}

void ddgError::exitHandler( void )
{
	if (!gError) gError = new ddgError;
	report();
	exit(gError->_error);
}

void ddgError::report(void)
{
	if (!gError) gError = new ddgError;
	if (gError)
#if 0
		cerr << gError->_file.s << "(" 
			 << gError->_line << ")"
			 << gError->_error << ":"
			 << gError->_code.s << "\n" 
			 << gError->_msg.s << "\n";
#else
	  fprintf (stderr, "Error: %s %d\n",
	  	gError->_file.s, gError->_line);
#endif
}

bool ddgError::set(int error, char *msg, char *file, int line, char *code )
{
	if (!gError) gError = new ddgError;
	if (gError)
	{
		gError->_error = error;
		gError->_line = line;
		gError->_file.assign(file);
		gError->_msg.assign(msg);
		gError->_code.assign(code);
	}
	return true;
}

