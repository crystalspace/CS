/*
    Copyright (C) 2002 by Norman Krämer

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
#include <time.h>

#define CS_DJGPP_UCLOCKS_PER_MILLISEC (UCLOCKS_PER_SEC/1000)
csTicks csGetTicks ()
{
  static uclock_t Remainder = 0;
  uclock_t Now = 0;


  Now = uclock () + Remainder;
  Remainder = Now%CS_DJGPP_UCLOCKS_PER_MILLISEC;
  
  return (csTicks)(Now / CS_DJGPP_UCLOCKS_PER_MILLISEC);
}
 
void csSleep (int SleepTime)
{
  uclock_t endtime;

  /* Wait for SleepTime milliseconds.  */
  endtime = uclock () + SleepTime * (UCLOCKS_PER_SEC / 1000);
  while (uclock () < endtime);
}
 

