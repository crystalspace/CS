/*
    Copyright (C) 2002 by Eric Sunshine <sunshine@sunshineco.com>

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
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include <windows.h>
#include <lmcons.h>

#include "csutil/win32/wintools.h"

csString csGetUsername()
{
  csString username;
  wchar_t* wname = 0;
  if (cswinIsWinNT ())
  {
    WCHAR buff[UNLEN + 1];
    DWORD sz = sizeof(buff) / sizeof (WCHAR);
    if (GetUserNameW (buff, &sz))
    {
      wname = csStrNewW (buff);
    }
  }
  else
  {
    char buff[UNLEN + 1];
    DWORD sz = sizeof(buff);
    if (GetUserNameA (buff, &sz))
    {
      wname = cswinAnsiToWide (buff);
    }
  }
  char* name = csStrNew (wname);
  username.Replace (name);
  delete[] name;
  delete[] wname;
  username.Trim();
  return username;
}
