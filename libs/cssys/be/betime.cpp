/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
  
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

#include <OS.h>
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "cssys/be/csbe.h"

//-----------------------------------------------------------------------------
// The function csGetTicks() is maintained in its own source file in order to
// allow console applications to make use of the system-specific time function
// without having to link with all of the other facilities referenced by the
// csSystemDriver class.  Do *not* incorporate the contents of this file into
// the main source file for the csSystemDriver class (csbe.cpp).  If it is
// incorporated, then it will be much more difficult to link a simple console
// application.
//-----------------------------------------------------------------------------
csTicks csGetTicks ()
{
  return system_time() / 1000;
}


//-----------------------------------------------------------------------------
// A dummy implementation of csSleep()
//-----------------------------------------------------------------------------
void csSleep( int msec )
{
  (void)msec;
}
