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

#include <stdarg.h>
#include <stdio.h>

#include "sysdef.h"
#include "cssys/system.h"

// to be called before all printf () calls
void csSystemDriver::console_open ()
{
}

// to be called before shutdown
void csSystemDriver::console_close ()
{
}

// to be called instead of printf (exact same prototype/functionality of printf)
void csSystemDriver::console_out (const char *str)
{
  fputs (str, stdout);
  fflush (stdout);
}
