/*
    Copyright (C) 1999 by Eric Sunshine <sunshine@sunshineco.com>
    Writen by Eric Sunshine <sunshine@sunshineco.com>

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

#include "sysdef.h"
#include "csengine/cspmeter.h"
#include "csengine/sysitf.h"

csProgressMeter::csProgressMeter(int n) :
  type(MSG_INITIALIZATION), total(n), current(0), anchor(0) {}

void csProgressMeter::Step()
{
  if (current < total)
  {
    // In actuality meter displays only 50 units, but prints 0% - 100%.
    current++;
    int const extent = 50 * current / total;
    if (anchor <= extent)
    {
      char buff [256]; // Batch the update here before sending to CsPrintf().
      char* p = buff;
      for (int i = anchor; i <= extent; i++)
      {
        if (i % 5 != 0)
	  *p++ = '.';
	else
	{
          int n;
	  sprintf (p, "%d%%%n", i * 2, &n );
	  p += n;
	}
      }
      *p = '\0';
      CsPrintf (type, "%s", buff);
      anchor = extent + 1;
    }
  }
}
