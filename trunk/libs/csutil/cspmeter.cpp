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
#include "csutil/sysfunc.h"
#include "ivaria/conout.h"

csTextProgressMeter::csTextProgressMeter (iConsoleOutput* cons, int n)
  : scfImplementationType (this),
  console (cons), granularity(10), tick_scale(2),  total(n), current(0), anchor(0)
{
}

csTextProgressMeter::~csTextProgressMeter()
{
}

void csTextProgressMeter::Step(unsigned int n)
{
  if (current < total)
  {
    bool doFlush = false;
    current += n;
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
      if (console)
        console->PutText ("%s", buff.GetData());
      else
      {
        doFlush = true;
        csPrintf ("%s", buff.GetData());
      }
      anchor = extent;
    }
    if (current == total)
    {
      if (console)
        console->PutText ("\n");
      else
      {
        doFlush = true;
        csPrintf ("\n");
      }
    }
    if (doFlush) fflush (stdout);
  }
}

void csTextProgressMeter::Restart()
{
  Reset();
  if (console)
    console->PutText ("0%%");
  else
    csPrintf ("0%%");
}

void csTextProgressMeter::Abort ()
{
  current = total;
  if (console)
    console->PutText ("\n");
  else
    csPrintf ("\n");
}

void csTextProgressMeter::Finalize ()
{
  current = total;
  if (console)
    console->PutText ("\n");
  else
    csPrintf ("\n");
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
