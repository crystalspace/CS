/*
    Copyright (C) 2005 by Frank Richter

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


#include "cssysdef.h"
#include "csutil/csstring.h"
#include <stdlib.h>

#ifndef CS_HAVE_SETENV

int setenv (const char* name, const char* value, bool overwrite)
{
  if (overwrite || (getenv (name) == 0))
  {
    csString envStr;
    envStr.Format ("%s=%s", name, value);
    return _putenv (envStr);
  }
  return 0;
}

#endif // CS_HAVE_SETENV
