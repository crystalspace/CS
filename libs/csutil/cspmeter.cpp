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
#include "csutil/csstring.h"
#include "ivaria/conout.h"

SCF_IMPLEMENT_IBASE (csTextProgressMeter)
  SCF_IMPLEMENTS_INTERFACE (iProgressMeter)
SCF_IMPLEMENT_IBASE_END

csTextProgressMeter::csTextProgressMeter (iConsoleOutput* cons, int n)
	: console (cons), granularity(10), tick_scale(2),
	  total(n), current(0), anchor(0)
{
  SCF_CONSTRUCT_IBASE (0);
}

csTextProgressMeter::~csTextProgressMeter()
{
  SCF_DESTRUCT_IBASE ();
}

void csTextProgressMeter::Step()
{
  if (current < total)
  {
    current++;
    int const units = (current == total ? 100 :
      (((100 * current) / total) / granularity) * granularity);
    int const extent = units / tick_scale;
    if (anchor < extent)
    {
      csString buff; // Batch the update here before emitting it.
      int i;
      for (i = anchor + 1; i <= extent; i++)
      {
        if (i % (10 / tick_scale) != 0)
	   buff << '.';
	else
	  buff.AppendFmt ("%d%%", i * tick_scale);
      }
      console->PutText ("%s", buff.GetData());
      anchor = extent;
    }
    if (current == total)
      console->PutText ("\n");
  }
}

void csTextProgressMeter::Restart()
{
  Reset();
  console->PutText ("0%%");
}

void csTextProgressMeter::Abort ()
{
  current = total;
  console->PutText ("\n");
}

void csTextProgressMeter::Finalize ()
{
  current = total;
  console->PutText ("\n");
}

void csTextProgressMeter::SetGranularity(int n)
{
  if (n >= 1 && n <= 100)
    granularity = n;
}

void csTextProgressMeter::SetTickScale(int n)
{
  if (n >= 1 && n <= 100)
    tick_scale = n;
}
