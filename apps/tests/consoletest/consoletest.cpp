/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csutil/ansicolor.h"
#include "csutil/sysfunc.h"

#include "teststrings.h"

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  {
    const char** qbf = quickBrownFox;
    while (*qbf != 0)
    {
      csPrintf ("%-10s %s\n", qbf[1], qbf[0]);
      qbf += 2;
    }
  }
  csPrintf ("\n");
  {
    const char** iceg = iCanEatGlass;
    while (*iceg != 0)
    {
      csPrintf ("%-10s %s\n", iceg[1], iceg[0]);
      iceg += 2;
    }
  }
  csPrintf ("\n");
  csPrintf ("Adding some " 
    CS_ANSI_FI CS_ANSI_FC "c" CS_ANSI_FY "o" CS_ANSI_FM "l" CS_ANSI_FY "o" CS_ANSI_FW "r" CS_ANSI_RST 
    " for " CS_ANSI_BW CS_ANSI_FR "f" CS_ANSI_FG "u" CS_ANSI_FB "n" CS_ANSI_RST "\n");
  printf ("For change, a printf()\n");
  csPrintfErr ("csPrintfErr()\n");
  csPrintf ("csPrintf() again\n");

  return 0;
}

