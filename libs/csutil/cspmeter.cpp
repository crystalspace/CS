/*
    Copyright (C) 1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
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

#include "cssysdef.h"
#include "csutil/cspmeter.h"

csProgressMeter::csProgressMeter(iSystem* s, int n, int t) : sys(s), type(t),
  granularity(10), tick_scale(2), total(n), current(0), anchor(0) {}

void csProgressMeter::Step()
{
  if (current < total)
  {
    current++;
    int const units = (current == total ? 100 :
      (((100 * current) / total) / granularity) * granularity);
    int const extent = units / tick_scale;
    if (anchor < extent)
    {
      char buff [256]; // Batch the update here before emitting it.
      char const* safety_margin = buff + sizeof(buff) - 5;
      char* p = buff;
      for (int i = anchor + 1; i <= extent && p < safety_margin; i++)
      {
        if (i % (10 / tick_scale) != 0)
	  *p++ = '.';
	else
	{
          int n;
	  sprintf(p, "%d%%%n", i * tick_scale, &n );
	  p += n;
	}
      }
      *p = '\0';
      sys->Printf(type, "%s", buff);
      anchor = extent;
    }
  }
}

void csProgressMeter::Restart()
{
  Reset();
  sys->Printf(type, "0%%");
}

void csProgressMeter::SetGranularity(int n)
{
  if (n >= 1 && n <= 100)
    granularity = n;
}

void csProgressMeter::SetTickScale(int n)
{
  if (n >= 1 && n <= 100)
    tick_scale = n;
}
